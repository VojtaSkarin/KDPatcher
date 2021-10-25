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
const char * GREPOBOT_EXECUTABLE = "GrepoBot.exe";

const char * VERSIONLIST_FILENAME = "VersionList.dat";
const char * ZIP_FILENAME = "GrepoBotData.ada";
const char * BSPATCH_EXECUTABLE = "bspatch.exe";

bool update( const char * versionlist_address );
void write_help();

bool execute( const std::string & command, bool wait_for_end, const char * environment );
bool execute( char * command, bool wait_for_end, const char * environment );

void patch_all( const std::vector< std::pair< std::string, std::string > > & newer_versions );
void patch_one( const std::string & name, const std::string & address );
void extract_bspatch();

std::string get_current_version();
auto get_newer_versions( const std::string & current_version, const char * versionlist_address )
	-> std::vector< std::pair< std::string, std::string > >;
