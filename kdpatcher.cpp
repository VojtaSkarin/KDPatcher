#include "kdpatcher.hpp"

int main( int argc, char ** argv ) {
	if ( argc != 3 ) {
		write_help();
		return EXIT_FAILURE;
	}
	
	if ( strcmp( argv[ 1 ], "update" ) == 0 ) {
		// Update
		std::cout << "Updating current directory" << std::endl;
		update( argv[ 2 ] );
	} else if ( strcmp( argv[ 1 ], "publish" ) == 0 ) {
		// Publish
		std::cout << "Publishing build directory to server" << std::endl;
		// publish();
	} else {
		write_help();
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
}

bool execute( const std::string & command, bool wait_for_end = true ) {
	return execute( const_cast< char * >( command.c_str() ), wait_for_end );
}

bool execute( char * command, bool wait_for_end = true ) {
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	
	ZeroMemory( &si, sizeof( si ) );
	si.cb = sizeof( si );
	ZeroMemory( &pi, sizeof( pi ) );
	
	int result = CreateProcessA( NULL, command, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

	if ( wait_for_end ) {
		WaitForSingleObject( pi.hProcess, INFINITE );
	}
	
	CloseHandle( pi.hProcess );
	CloseHandle( pi.hThread );
	
	return result != 0;
}

bool update( const char * versionlist_address ) {
	std::string current_version = get_current_version();
	
	std::cout << "Local version is " << current_version << std::endl;
	
	std::vector< std::pair< std::string, std::string > > newer_versions =
		get_newer_versions( current_version, versionlist_address );
		
	std::cout << "Actual version is " << newer_versions.front().first << std::endl;
	
	patch_all( newer_versions );
	
	std::cout << "Launching program" << std::endl;
	
	execute( EXECUTABLE_FILENAME, false );
	
	return true;
}

std::string get_current_version() {
	if ( GetFileAttributes( CHANGELOG_FILENAME ) == INVALID_FILE_ATTRIBUTES ) {
		std::cout << "Cannot find local version" << std::endl;
		return "none";
	}
	
	std::ifstream changelog( CHANGELOG_FILENAME );
	
	std::string line;
	std::getline( changelog, line );
	
	int start = line.find( "[" );
	int end = line.find( "]" );
	std::string version = line.substr( start + 2, end - start - 3 );
	
	int index = 0;
	while ( ( index = version.find( " " ) ) != -1 ) {
		version.at( index ) = '_';
	}
	
	return version;
}

auto get_newer_versions( const std::string & current_version, const char * versionlist_address )
	-> std::vector< std::pair< std::string, std::string > >
{
	std::cout << "Downloading version list" << std::endl;
	
	bool result = execute( std::string() + "curl " + versionlist_address  + " -o " + VERSIONLIST_FILENAME );
	std::cout << "curl result " << result << std::endl;
	
	std::vector< std::pair< std::string, std::string > > newer_versions;	
	std::ifstream version_list( VERSIONLIST_FILENAME );
	
	do {
		std::string current;
		std::getline( version_list, current );
		
		size_t start = current.find( ", " );
		
		if ( start == std::string::npos ) {
			continue;
		}
		
		std::string name = current.substr( 0, start );
		std::string path = current.substr( start + 2 );
		
		newer_versions.push_back( { name, path } );
	} while ( newer_versions.back().first != current_version && ! version_list.eof() );
	
	return newer_versions;
}

void patch_all( const std::vector< std::pair< std::string, std::string > > & newer_versions ) {
	if ( newer_versions.size() == 1 ) {
		std::cout << "Local version is up to date" << std::endl;
		return;
	}
	
	extract_bspatch();
	
	if ( GetFileAttributes( ZIP_FILENAME ) == INVALID_FILE_ATTRIBUTES ) {
		std::cout << "Initial release not found" << std::endl;
		std::cout << "Downloading initial release" << std::endl;
		
		bool result = execute( std::string() + "curl -L " + newer_versions.back().second + " -o " + ZIP_FILENAME );
		std::cout << "curl result " << result << std::endl;
	} else {
		std::cout << "Initial release found" << std::endl;
	}
	
	for ( int i = newer_versions.size() - 1; i > 0; i-- ) {
		const std::string & local_version = newer_versions.at( i ).first;
		const auto & [ name, address ] = newer_versions.at( i - 1 );
		
		std::cout << "Patching from " << local_version << " to " << name << std::endl;
		
		patch_one( name, address );
	}
	
	std::cout << "Extracting patched version" << std::endl;
	
	bool result = execute( std::string() + "tar --extract --file=" + ZIP_FILENAME );
	std::cout << "tar result " << result << std::endl;
	
	std::cout << "Local version is up to date" << std::endl;
}

void extract_bspatch() {
	if ( GetFileAttributes( "bspatch.exe" ) != INVALID_FILE_ATTRIBUTES ) {
		std::cout << "File bspatch.exe exists" << std::endl;
		return;
	}
	
	std::cout << "Extracting file bspatch.exe" << std::endl;
	
	std::ifstream kdpatcher( "kdpatcher.exe", std::ios::in | std::ios::binary);
	std::ofstream bspatch( "bspatch.exe", std::ios::out | std::ios::binary );
	
	char buffer[ BUFFER_SIZE ];
	
	// Read file size
	kdpatcher.seekg( -8, std::ios::end );
	kdpatcher.read( buffer, 8 );
	int64_t blocks = * static_cast< int64_t * >( static_cast< void * >( buffer ) );
	
	// Copy file content
	kdpatcher.seekg( -( blocks * BUFFER_SIZE + 8 ), std::ios::end );
	
	for ( int i = 0; i < blocks; i++ ) {
		ZeroMemory( buffer, BUFFER_SIZE );
		
		kdpatcher.read( buffer, BUFFER_SIZE );
		bspatch.write( buffer, BUFFER_SIZE );
	}
	
	std::cout << "File bspatch.exe extracted" << std::endl;
}

void patch_one( const std::string & name, const std::string & address ) {
	std::cout << "Downloading patch " << name << std::endl;
	
	bool result = execute( std::string() + "curl -L " + address + " -o " + name );
	std::cout << "curl result " << result << std::endl;
	
	std::cout << "Applying patch " << name << std::endl;
	
	result = execute( std::string() + "bspatch " + ZIP_FILENAME + " " + ZIP_FILENAME + " " + name );
	std::cout << "bspatch result " << result << std::endl;
	
	std::cout << "Patch applied" << std::endl;
}

void write_help() {
	std::cout << "Usage:" << std::endl;
	std::cout << "kdpatcher update [version_list_url]" << std::endl;
	std::cout << "NOT IMPLEMENTED -- kdpatcher publish [version_list_url]" << std::endl;
}