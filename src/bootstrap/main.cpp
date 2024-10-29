//
// Created by ENDERZOMBI102 on 14/10/2024.
//
#include "basetypes.h"


#include <SDL3/SDL_loadso.h>
#include <cstdlib>
#include <cstring>
#include <format>


int main( const int argc, const char** argv ) {
	// engine bin directory
	const char* binDir{ nullptr };
	{
		const auto path{ argv[0] };
		auto length{ std::strlen( path ) - 1 };
		while ( length > 0 && !( path[length] == '\\' || path[length] == '/' ) ) {
			length -= 1;
		}

		const auto res = new char[ length + 2 ];
		std::memcpy( res, path, length );
		res[length+0] = '/';
		res[length+1] = '\0';
		binDir = res;
	}

	// on posix-like, set the LD_LIBRARY_PATH such as the other binaries can be found
	#if defined( PLATFORM_POSIX )
		const auto old{ std::getenv( "LD_LIBRARY_PATH" ) };
		// the value is copied, so no need to worry about the tmp object
		std:setenv( "LD_LIBRARY_PATH", old ? std::format( "{}:{}", binDir, old ).c_str() : binDir, true );
	#endif

	// create absolute launcher dll path
	const auto launcherDll{ std::format( "{}launcher{}", binDir, DLL_EXT_STRING ) };

	// open launcher dll
	const auto lib{ SDL_LoadObject( launcherDll.c_str() ) };
	if ( not lib ) {
		fputs( SDL_GetError(), stderr );
		return 1;
	}

	// get hold of the main function
	using LauncherMainType = int(*)(int, const char**);
	const auto launcherMain{ reinterpret_cast<LauncherMainType>( SDL_LoadFunction( lib, "LauncherMain" ) ) };

	// call it
	return launcherMain( argc, argv );
}

