#include <iostream>
#include <cstring>
#include <fstream>
#include <Windows.h>
#include <stdio.h>
#include <tchar.h>
#include <vector>
#include <tuple>
#include <unistd.h>
#include <inttypes.h>

#define BUFFER_SIZE 1024
#define WORD 8

const char * GREPOBOT_EXECUTABLE = "GrepoBot.exe";
const char * BSPATCH_EXECUTABLE = "bspatch.exe";

const char * CHANGELOG_FILENAME = "ChangeLog.txt";
const char * VERSIONLIST_FILENAME = "VersionList.dat";
const char * ZIP_FILENAME = "GrepoBotData.ada";
const char * LIBCEF_DLL = "libcef.dll";

bool update( const char * versionlist_address );
void write_help();

bool execute( const std::string & command, bool wait_for_end, const char * environment );
bool execute( char * command, bool wait_for_end, const char * environment );

void patch_all( const std::vector< std::pair< std::string, std::string > > & all_versions, const std::pair< std::string, std::string > & dll_url, const std::string & local_version );
void patch_one( const std::string & name, const std::string & address );
void extract_bspatch();
bool download_init( const std::string & dll_url, const std::string & first_version_url );

std::string get_local_version();
auto get_all_versions( const char * versionlist_address )
	-> std::pair< std::vector< std::pair< std::string, std::string > >, std::pair< std::string, std::string > >;
