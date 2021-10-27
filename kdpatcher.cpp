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

bool execute( const std::string & command, bool wait_for_end = true, const char * environment = NULL ) {
	return execute( const_cast< char * >( command.c_str() ), wait_for_end, environment );
}

bool execute( char * command, bool wait_for_end = true, const char * const_environment = NULL ) {
	std::cout << command << std::endl;
	
	char * environment = const_cast< char * >( const_environment );
	
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	
	ZeroMemory( &si, sizeof( si ) );
	si.cb = sizeof( si );
	ZeroMemory( &pi, sizeof( pi ) );
	
	int result = CreateProcessA( NULL, command, NULL, NULL, FALSE, 0, environment, NULL, &si, &pi);

	if ( wait_for_end ) {
		WaitForSingleObject( pi.hProcess, INFINITE );
	}
	
	CloseHandle( pi.hProcess );
	CloseHandle( pi.hThread );
	
	return result != 0;
}

bool update( const char * versionlist_address ) {
	std::string local_version = get_local_version();
	
	std::cout << "Local version is " << local_version << std::endl;
	
	auto [ all_versions, initial_release_url ] = get_all_versions( versionlist_address );
		
	std::cout << "Newest version is " << all_versions.front().first << std::endl;
	
	patch_all( all_versions, initial_release_url, local_version );
	
	std::cout << "Launching program" << std::endl;
	
	execute( GREPOBOT_EXECUTABLE, false );
	
	return true;
}

std::string get_local_version() {
	if ( GetFileAttributes( CHANGELOG_FILENAME ) == INVALID_FILE_ATTRIBUTES ) {
		std::cout << "Cannot find local version" << std::endl;
		return "none";
	}
	
	std::ifstream changelog( CHANGELOG_FILENAME );
	
	std::string line;
	std::getline( changelog, line );
	
	int start = line.find( "[" );
	int end = line.find( "]" );
	std::string local_version = line.substr( start + 2, end - start - 3 );
	
	int index = 0;
	while ( ( index = local_version.find( " " ) ) != -1 ) {
		local_version.at( index ) = '_';
	}
	
	return local_version;
}

auto get_all_versions( const char * versionlist_address )
	-> std::pair< std::vector< std::pair< std::string, std::string > >, std::pair< std::string, std::string > >
{
	std::cout << "Downloading version list" << std::endl;
	
	bool result = execute( std::string() + "curl " + versionlist_address  + " -o " + VERSIONLIST_FILENAME );
	std::cout << "curl result " << result << std::endl;
	
	std::vector< std::pair< std::string, std::string > > all_versions;	
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
		
		all_versions.push_back( { name, path } );
	} while ( ! version_list.eof() );
	
	std::pair< std::string, std::string > initial_release_url = std::move( all_versions.back() );
	all_versions.pop_back();
	
	return { all_versions, initial_release_url };
}

void patch_all( const std::vector< std::pair< std::string, std::string > > & all_versions, const std::pair< std::string, std::string > & initial_release_url, const std::string & local_version ) {
	int i = all_versions.size() - 1;
	bool revert_to_init = false;
	
	// Move to actual version
	for ( ; i >= 0 && all_versions.at( i ).first != local_version; i-- );
	
	if ( i == 0 ) {
		// Up to date
		return;
	} else if ( i < 0 ) {
		// Something went wrong, revert to init and patch to newest
		i = all_versions.size() - 1;
		revert_to_init = true;
	}
	
	extract_bspatch();
	
	download_init( initial_release_url.second, all_versions.back().second, revert_to_init );

	for ( ; i > 0; i-- ) {
		const std::string & current_version = all_versions.at( i ).first;
		const auto & [ name, address ] = all_versions.at( i - 1 );
		
		std::cout << "Patching from " << current_version << " to " << name << std::endl;
		
		patch_one( name, address );
	}
	
	std::cout << "Extracting patched version" << std::endl;
	
	bool result = execute( std::string() + "tar --extract --file=" + ZIP_FILENAME );
	std::cout << "tar result " << result << std::endl;
	
	std::cout << "Local version is up to date" << std::endl;
}

void extract_bspatch() {
	if ( GetFileAttributes( BSPATCH_EXECUTABLE ) != INVALID_FILE_ATTRIBUTES ) {
		std::cout << "File bspatch.exe exists" << std::endl;
		return;
	}
	
	std::cout << "Extracting file bspatch.exe" << std::endl;
	

	char kdpatcher_executable[ MAX_PATH ] = { 0 };
	GetModuleFileNameA( NULL, kdpatcher_executable, MAX_PATH );

	std::ifstream kdpatcher( kdpatcher_executable, std::ios::in | std::ios::binary);
	std::ofstream bspatch( BSPATCH_EXECUTABLE, std::ios::out | std::ios::binary );
	
	char buffer[ BUFFER_SIZE ] = { 0 };
	
	printf( "sizeof(int64_t) = %llu\n", sizeof( int64_t ) );
	printf( "sizeof(int64_t *) = %llu\n", sizeof( int64_t * ) );
	printf( "sizeof(void *) = %llu\n", sizeof( void * ) );
	
	puts( "Before reading" );
	for ( int i = 0; i < 8; i++ ) {
		printf( "%d ", buffer[ i ] );
	}
	printf( "\n" );
	
	// Read file size
	kdpatcher.seekg( -WORD, std::ios::end );
	kdpatcher.read( buffer, WORD );
	
	puts( "After reading" );
	for ( int i = 0; i < 8; i++ ) {
		printf( "%d ", buffer[ i ] );
	}
	printf( "\n" );
	
	printf( "%" PRId64 "\n", * static_cast< int64_t * >( static_cast< void * >( buffer ) ) );
	
	int64_t blocks = * static_cast< int64_t * >( static_cast< void * >( buffer ) );
	std::cout << "blocks " << blocks << std::endl;
	
	// Copy file content
	kdpatcher.seekg( -( blocks * BUFFER_SIZE + 8 ), std::ios::end );
	
	for ( int i = 0; i < blocks; i++ ) {
		ZeroMemory( buffer, BUFFER_SIZE );
		
		kdpatcher.read( buffer, BUFFER_SIZE );
		bspatch.write( buffer, BUFFER_SIZE );
	}
	
	std::cout << "File bspatch.exe extracted" << std::endl;
}

void download_init( const std::string & initial_release_url, const std::string & first_version_url, bool revert_to_init ) {
	if ( GetFileAttributes( ZIP_FILENAME ) != INVALID_FILE_ATTRIBUTES and ! revert_to_init ) {
		std::cout << "Initial release found" << std::endl;
		return;
	}
	
	std::cout << "Initial release not found" << std::endl;
	std::cout << "Downloading initial release" << std::endl;
	
	bool result = execute( std::string() + "curl -L " + initial_release_url + " -o " + INITIAL_RELEASE_FILENAME );
	std::cout << "curl result " << result << std::endl;
	
	result = execute( std::string() + "tar --extract --file=" + INITIAL_RELEASE_FILENAME );
	std::cout << "tar result " << result << std::endl;
	
	// Maybe delete?

	result = execute( std::string() + "curl -L " + first_version_url + " -o " + ZIP_FILENAME );
	std::cout << "curl result " << result << std::endl;
}

void patch_one( const std::string & name, const std::string & address ) {
	std::cout << "Downloading patch " << name << std::endl;
	
	bool result = execute( std::string() + "curl -L " + address + " -o " + name );
	std::cout << "curl result " << result << std::endl;
	
	std::cout << "Applying patch " << name << std::endl;
	int i;
	std::cin >> i;
	
	result = execute( std::string() + BSPATCH_EXECUTABLE + " " + ZIP_FILENAME + " " + ZIP_FILENAME + " " + name, true, "__COMPAT_LAYER=RUNASINVOKER\0" );
	std::cout << "bspatch result " << result << std::endl;
	
	std::cout << "Patch applied" << std::endl;
}

void write_help() {
	std::cout << "Usage:" << std::endl;
	std::cout << "kdpatcher update [version_list_url]" << std::endl;
	std::cout << "NOT IMPLEMENTED -- kdpatcher publish [version_list_url]" << std::endl;
}