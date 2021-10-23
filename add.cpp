#include <iostream>
#include <fstream>
#include <Windows.h>

#define BUFFER_SIZE 1024

int main( void ) {
	std::ifstream bspatch( "bspatch.exe", std::ios::in | std::ios::binary );
	std::ofstream kdpatcher( "kdpatcher.exe", std::ios::out | std::ios::binary | std::ios::app );
	
	int64_t blocks = 0;
	char buffer[ BUFFER_SIZE ];
	
	do {
		ZeroMemory( buffer, BUFFER_SIZE );
		bspatch.read( buffer, BUFFER_SIZE );
		kdpatcher.write( buffer, BUFFER_SIZE );
		
		blocks++;
	} while ( bspatch.gcount() > 0 );
	
	std::cout << blocks << " blocks written" << std::endl;
	
	kdpatcher.write( static_cast< char * >( static_cast< void * >( & blocks ) ), 8 );
	
	return 0;
}