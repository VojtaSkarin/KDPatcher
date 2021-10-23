#include <iostream>
#include <cstring>
#include <fstream>
#include <Windows.h>
#include <stdio.h>
#include <tchar.h>
#include <vector>
#include <tuple>
#include <unistd.h>

#define execute( string ) \
char command[] = string; \
execute_function( command );

#define BUFFER_SIZE 1024

const char * CHANGELOG_NAME = "C:\\GrepoBot\\ChangeLog.txt";
const char * VERSIONLIST_ADDRESS = "zemechvaly.cz/version_list.txt";
const char * VERSIONLIST_FILENAME = "version_list.txt";
const char * ZIP = "GrepoBotInstaller.zip";

bool update();

bool execute_string( const std::string & command );
bool execute_function( char * command );

void patch_all( const std::vector< std::pair< std::string, std::string > > & newer_versions );
void patch_one( const std::string & name, const std::string & address );
void extract_bspatch();

std::string get_current_version();
auto get_newer_versions( const std::string & current_version )
	-> std::vector< std::pair< std::string, std::string > >;
