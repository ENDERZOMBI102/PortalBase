//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:  Defines the entry point for the console application.
//
// $NoKeywords: $
//
//=============================================================================//
#include "interface.h"
#include "ilaunchabledll.h"
#include "tier0/icommandline.h"
#include "tier1/strtools.h"


void MakeFullPath( const char* pIn, char* pOut, int outLen ) {
	if ( pIn[ 0 ] == '/' or pIn[ 0 ] == '\\' or pIn[ 1 ] == ':' ) {
		// It's already a full path.
		Q_strncpy( pOut, pIn, outLen );
	} else {
		_getcwd( pOut, outLen );
		Q_strncat( pOut, "\\", outLen, COPY_ALL_CHARACTERS );
		Q_strncat( pOut, pIn, outLen, COPY_ALL_CHARACTERS );
	}
}

int main( const int argc, char* argv[] ) {
	char dllName[ 512 ];

	CommandLine()->CreateCmdLine( argc, argv );

	// check whether they used the -both switch. If this is specified, vrad will be run
	// twice, once with -hdr and once without
	int both_arg = 0;
	for ( int arg = 1; arg < argc; arg += 1 ) {
		if ( Q_stricmp( argv[ arg ], "-both" ) == 0 ) {
			both_arg = arg;
		}
	}

	char fullPath[ 512 ];
	char redirectFilename[ 512 ];
	MakeFullPath( argv[ 0 ], fullPath, sizeof( fullPath ) );
	Q_StripFilename( fullPath );
	Q_snprintf( redirectFilename, sizeof( redirectFilename ), "%s\\%s", fullPath, "vrad.redirect" );

	// First, look for vrad.redirect and load the dll specified in there if possible.
	CSysModule* pModule = nullptr;
	FILE* fp = fopen( redirectFilename, "rt" );
	if ( fp ) {
		if ( fgets( dllName, sizeof( dllName ), fp ) ) {
			char* pEnd = strstr( dllName, "\n" );
			if ( pEnd ) {
				*pEnd = 0;
			}

			pModule = Sys_LoadModule( dllName );
			if ( pModule ) {
				printf( "Loaded alternate VRAD DLL (%s) specified in vrad.redirect.\n", dllName );
			} else {
				printf( "Can't find '%s' specified in vrad.redirect.\n", dllName );
			}
		}

		fclose( fp );
	}

	int returnValue = 0;

	for ( int mode = 0; mode < 2; mode += 1 ) {
		if ( mode and not both_arg ) {
			continue;
		}

		// If it didn't load the module above, then use the default one
		if ( not pModule ) {
			strcpy( dllName, "vrad_dll" DLL_EXT_STRING );
			pModule = Sys_LoadModule( dllName );
		}

		if ( not pModule ) {
			printf( "vrad_launcher error: can't load %s\n%s", dllName, Sys_LastErrorString() );
			return 1;
		}

		CreateInterfaceFn fn = Sys_GetFactory( pModule );
		if ( not fn ) {
			printf( "vrad_launcher error: can't get factory from %s\n", dllName );
			Sys_UnloadModule( pModule );
			return 2;
		}

		int retCode = 0;
		auto* pDLL = static_cast<ILaunchableDLL*>( fn( LAUNCHABLE_DLL_INTERFACE_VERSION, &retCode ) );
		if ( not pDLL ) {
			printf( "vrad_launcher error: can't get ILaunchableDLL interface from %s\n", dllName );
			Sys_UnloadModule( pModule );
			return 3;
		}

		if ( both_arg ) {
			strcpy( argv[ both_arg ], mode ? "-hdr" : "-ldr" );
		}
		returnValue = pDLL->main( argc, argv );
		Sys_UnloadModule( pModule );
		pModule = nullptr;
	}
	return returnValue;
}
