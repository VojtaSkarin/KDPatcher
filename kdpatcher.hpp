#include <iostream>
#include <cstring>
#include <fstream>
#include <Windows.h>
#include <stdio.h>
#include <tchar.h>
#include <vector>
#include <tuple>
#include <unistd.h>

#define BUFFER_SIZE 1024

const char * CHANGELOG_FILENAME = "ChangeLog.txt";
const char * VERSIONLIST_FILENAME = "version_list.dat";
const char * ZIP_FILENAME = "GrepoBotInstaller.zip";
const char * EXECUTABLE_FILENAME = "GrepoBot.exe";

bool update( const char * versionlist_address );
void write_help();

bool execute( const std::string & command, bool wait_for_end );
bool execute( char * command, bool wait_for_end );

void patch_all( const std::vector< std::pair< std::string, std::string > > & newer_versions );
void patch_one( const std::string & name, const std::string & address );
void extract_bspatch();

std::string get_current_version();
auto get_newer_versions( const std::string & current_version, const char * versionlist_address )
	-> std::vector< std::pair< std::string, std::string > >;
