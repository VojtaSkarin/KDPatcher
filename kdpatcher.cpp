#include "kdpatcher.hpp"

int main( int argc, char ** argv ) {
	if ( argc != 2 ) {
		std::cout << "Expecting exactly one argument" << std::endl;
		return EXIT_FAILURE;
	}
	
	if ( strcmp( argv[ 1 ], "update" ) == 0 ) {
		// Update
		std::cout << "Updating current directory" << std::endl;
		update();
	} else if ( strcmp( argv[ 1 ], "publish" ) == 0 ) {
		// Publish
		std::cout << "Publishing build directory to server" << std::endl;
	} else {
		std::cout << "Unsupported switch" << std::endl;
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
}

bool execute_string( const std::string & command ) {
	return execute_function( const_cast< char * >( command.c_str() ) );
}

bool execute_function( char * command ) {
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	
	ZeroMemory( &si, sizeof( si ) );
	si.cb = sizeof( si );
	ZeroMemory( &pi, sizeof( pi ) );
	
	int result = CreateProcessA( NULL, command, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

	WaitForSingleObject( pi.hProcess, INFINITE );
	
	CloseHandle( pi.hProcess );
	CloseHandle( pi.hThread );
	
	return result != 0;
}

void patch( const std::vector< std::pair< std::string, std::string > > & newer_versions ) {
	if ( newer_versions.size() == 1 ) {
		std::cout << "Local version is up to date" << std::endl;
	}
	
	for ( int i = newer_versions.size() - 1; i > 0; i-- ) {
		const std::string & local_version = newer_versions.at( i ).first;
		const auto & [ name, address ] = newer_versions.at( i - 1 );
		
		std::cout << "Patching from " << local_version << " to " << name << address << std::endl;
	}
}

bool update() {
	std::string current_version = get_current_version();
	
	std::cout << "Local version is " << current_version << std::endl;
	
	std::vector< std::pair< std::string, std::string > > newer_versions =
		get_newer_versions( current_version );
		
	std::cout << "Actual version is " << newer_versions.front().first << std::endl;
	
	patch( newer_versions );
	
	return true;
}

std::string get_current_version() {
	std::ifstream changelog( CHANGELOG_NAME );
	
	std::string line;
	std::getline( changelog, line );
	
	int start = line.find( "[" );
	int end = line.find( "]" );
	std::string version = line.substr( start + 2, end - start - 3 );
	
	return version;
}

auto get_newer_versions( const std::string & current_version )
	-> std::vector< std::pair< std::string, std::string > >
{
	execute_string( std::string() + "curl " + VERSIONLIST_ADDRESS  + " -o " + VERSIONLIST_FILENAME );
	
	std::vector< std::pair< std::string, std::string > > newer_versions;	
	std::ifstream version_list( VERSIONLIST_FILENAME );
	
	do {
		std::string current;
		std::getline( version_list, current );
		
		int start = current.find( ", " );
		
		std::string name = current.substr( 0, start );
		std::string path = current.substr( start + 2 );
		
		newer_versions.push_back( { name, path } );
	} while ( newer_versions.back().first != current_version );
	
	return newer_versions;
}