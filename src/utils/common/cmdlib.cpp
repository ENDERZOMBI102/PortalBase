//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//
//=============================================================================//
// -----------------------
// cmdlib.cpp
// -----------------------
#include "cmdlib.h"
#include "tier0/platform.h"
#include "tier1/strtools.h"
#if IsWindows()
	#if IsPC()
		#include <processthreadsapi.h>
	#endif
	#include <io.h>
	#include <conio.h>
	#include <direct.h>
#elif IsPosix()
	#include <sys/types.h>
	#include <sys/stat.h>
#endif
#include "KeyValues.h"
#include "filesystem_helpers.h"
#include "filesystem_tools.h"
#include "tier0/icommandline.h"
#include "utllinkedlist.h"
#include "utlvector.h"

// set these before calling CheckParm
int myargc;
char** myargv;

char com_token[ 1024 ];

qboolean archive;
char archivedir[ 1024 ];

FileHandle_t g_pLogFile = nullptr;

CUtlLinkedList<CleanupFn, unsigned short> g_CleanupFunctions;
CUtlLinkedList<SpewHookFn, unsigned short> g_ExtraSpewHooks;

bool g_bStopOnExit = false;
void ( *g_ExtraSpewHook )( const char* ) = nullptr;

void CmdLib_FPrintf( FileHandle_t hFile, const char* pFormat, ... ) {
	static CUtlVector<char> buf;
	if ( buf.Count() == 0 ) {
		buf.SetCount( 1024 );
	}

	va_list marker;
	va_start( marker, pFormat );

	while ( true ) {
		int ret = Q_vsnprintf( buf.Base(), buf.Count(), pFormat, marker );
		if ( ret >= 0 ) {
			// Write the string.
			g_pFileSystem->Write( buf.Base(), ret, hFile );

			break;
		} else {
			// Make the buffer larger.
			int newSize = buf.Count() * 2;
			buf.SetCount( newSize );
			if ( buf.Count() != newSize ) {
				Error( "CmdLib_FPrintf: can't allocate space for text." );
			}
		}
	}

	va_end( marker );
}

char* CmdLib_FGets( char* pOut, int outSize, FileHandle_t hFile ) {
	int iCur = 0;
	for ( ; iCur < ( outSize - 1 ); iCur++ ) {
		char c;
		if ( !g_pFileSystem->Read( &c, 1, hFile ) ) {
			if ( iCur == 0 ) {
				return nullptr;
			} else {
				break;
			}
		}

		pOut[ iCur ] = c;
		if ( c == '\n' ) {
			break;
		}

		if ( c == EOF ) {
			if ( iCur == 0 ) {
				return nullptr;
			} else {
				break;
			}
		}
	}

	pOut[ iCur ] = 0;
	return pOut;
}

// This pauses before exiting if they use -StopOnExit. Useful for debugging.
class CExitStopper {
public:
	~CExitStopper() {
		if ( g_bStopOnExit ) {
			Warning( "\nPress any key to quit.\n" );
			getchar();
		}
	}
} g_ExitStopper;


// FIXME: This is dumbly fixed
static unsigned char g_InitialColor{ 0xFF };
static unsigned char g_LastColor{ 0xFF };
static unsigned char g_BadColor{ 0xFF };
static WORD g_BackgroundFlags{ 0xFFFF };
static void GetInitialColors() {
	// Get the old background attributes.
	//	CONSOLE_SCREEN_BUFFER_INFO oldInfo;
	//	GetConsoleScreenBufferInfo( GetStdHandle( STD_OUTPUT_HANDLE ), &oldInfo );
	//	g_InitialColor = g_LastColor = oldInfo.wAttributes & (FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY);
	//	g_BackgroundFlags = oldInfo.wAttributes & (BACKGROUND_RED|BACKGROUND_GREEN|BACKGROUND_BLUE|BACKGROUND_INTENSITY);
	//
	//	g_BadColor = 0;
	//	if (g_BackgroundFlags & BACKGROUND_RED)
	//		g_BadColor |= FOREGROUND_RED;
	//	if (g_BackgroundFlags & BACKGROUND_GREEN)
	//		g_BadColor |= FOREGROUND_GREEN;
	//	if (g_BackgroundFlags & BACKGROUND_BLUE)
	//		g_BadColor |= FOREGROUND_BLUE;
	//	if (g_BackgroundFlags & BACKGROUND_INTENSITY)
	//		g_BadColor |= FOREGROUND_INTENSITY;
}

WORD SetConsoleTextColor( int red, int green, int blue, int intensity ) {
	WORD ret = g_LastColor;

	// TODO: Just use the Truecolor output codes
	if ( !red && !green && !blue ) {  // 000
		g_LastColor = intensity ? 90 : 30;
	} else if ( !red && !green ) {    // 001
		g_LastColor = intensity ? 94 : 34;
	} else if ( !red && !blue ) {     // 010
		g_LastColor = intensity ? 92 : 32;
	} else if ( !red ) {              // 011
		g_LastColor = intensity ? 96 : 36;
	} else if ( !green && !blue ) {   // 100
		g_LastColor = intensity ? 91 : 31;
	} else if ( !green ) {            // 101
		g_LastColor = intensity ? 95 : 35;
	} else if ( !blue ) {             // 110
		g_LastColor = intensity ? 93 : 33;
	} else {                          // 111
		g_LastColor = intensity ? 97 : 37;
	}

	printf( "\x1b[2;%im", g_LastColor );
	return ret;
}

void RestoreConsoleTextColor( WORD color ) {
	// SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), color | g_BackgroundFlags );
	printf( "\x1b[2;39m" );// reset text color to default
	g_LastColor = color;
}


#if defined( CMDLIB_NODBGLIB )
	// This can go away when everything is in bin.
	void Error( char const* pMsg, ... ) {
		va_list marker;
		va_start( marker, pMsg );
		vprintf( pMsg, marker );
		va_end( marker );

		exit( -1 );
	}
#else
	CThreadMutex g_SpewMutex;
	bool g_bSuppressPrintfOutput{ false };

	SpewRetval_t CmdLib_SpewOutputFunc( SpewType_t type, char const* pMsg ) {
		// Hopefully two threads won't call this simultaneously right at the start!
		WORD old;
		SpewRetval_t retVal;

		g_SpewMutex.Lock();
		{
			if ( ( type == SPEW_MESSAGE ) || ( type == SPEW_LOG ) ) {
				Color c = *GetSpewOutputColor();
				if ( c.r() != 255 || c.g() != 255 || c.b() != 255 ) {
					// custom color
					old = SetConsoleTextColor( c.r(), c.g(), c.b(), c.a() );
				} else {
					old = SetConsoleTextColor( 1, 1, 1, 0 );
				}
				retVal = SPEW_CONTINUE;
			} else if ( type == SPEW_WARNING ) {
				old = SetConsoleTextColor( 1, 1, 0, 1 );
				retVal = SPEW_CONTINUE;
			} else if ( type == SPEW_ASSERT ) {
				old = SetConsoleTextColor( 1, 0, 0, 1 );
				retVal = SPEW_DEBUGGER;
			} else if ( type == SPEW_ERROR ) {
				old = SetConsoleTextColor( 1, 0, 0, 1 );
				retVal = SPEW_ABORT;// doesn't matter... we exit below, so we can return an errorlevel (which dbg.dll doesn't do).
			} else {
				old = SetConsoleTextColor( 1, 1, 1, 1 );
				retVal = SPEW_CONTINUE;
			}

			if ( !g_bSuppressPrintfOutput || type == SPEW_ERROR ) {
				printf( "%s", pMsg );
			}

			if ( type == SPEW_ERROR ) {
				printf( "\n" );
			}

			if ( g_pLogFile ) {
				CmdLib_FPrintf( g_pLogFile, "%s", pMsg );
				g_pFileSystem->Flush( g_pLogFile );
			}

			// Dispatch to other spew hooks.
			FOR_EACH_LL( g_ExtraSpewHooks, i ) {
				g_ExtraSpewHooks[ i ]( pMsg );
			}

			RestoreConsoleTextColor( old );
		}
		g_SpewMutex.Unlock();

		if ( type == SPEW_ERROR ) {
			CmdLib_Exit( 1 );
		}

		return retVal;
	}


	void InstallSpewFunction() {
		setvbuf( stdout, nullptr, _IONBF, 0 );
		setvbuf( stderr, nullptr, _IONBF, 0 );

		SpewOutputFunc( CmdLib_SpewOutputFunc );
		GetInitialColors();
	}

	void InstallExtraSpewHook( SpewHookFn pFn ) {
		g_ExtraSpewHooks.AddToTail( pFn );
	}

	#if 0
		void CmdLib_AllocError( unsigned long size ) {
			Error( "Error trying to allocate %d bytes.\n", size );
		}

		int CmdLib_NewHandler( size_t size ) {
			CmdLib_AllocError( size );
			return 0;
		}
	#endif

	void InstallAllocationFunctions() {
		//	_set_new_mode( 1 ); // so if malloc() fails, we exit.
		//	_set_new_handler( CmdLib_NewHandler );
	}

	void SetSpewFunctionLogFile( char const* pFilename ) {
		Assert( not g_pLogFile );
		g_pLogFile = g_pFileSystem->Open( pFilename, "a" );

		Assert( g_pLogFile );
		if ( not g_pLogFile ) {
			Error( "Can't create LogFile:\"%s\"\n", pFilename );
		}

		CmdLib_FPrintf( g_pLogFile, "\n\n\n" );
	}


	void CloseSpewFunctionLogFile() {
		if ( g_pFileSystem && g_pLogFile ) {
			g_pFileSystem->Close( g_pLogFile );
			g_pLogFile = FILESYSTEM_INVALID_HANDLE;
		}
	}


	void CmdLib_AtCleanup( CleanupFn pFn ) {
		g_CleanupFunctions.AddToTail( pFn );
	}


	void CmdLib_Cleanup() {
		CloseSpewFunctionLogFile();

		CmdLib_TermFileSystem();

		FOR_EACH_LL( g_CleanupFunctions, i )
		g_CleanupFunctions[ i ]();
	}


	void CmdLib_Exit( int exitCode ) {
		#if IsWindows()
			TerminateProcess( GetCurrentProcess(), 1 );
		#elif IsPosix()
			exit( exitCode );
		#endif
	}
#endif


/*
===================
ExpandWildcards

Mimic unix command line expansion
===================
*/
#define MAX_EX_ARGC 1024
int ex_argc;
char* ex_argv[ MAX_EX_ARGC ];
#if IsWindows()
	void ExpandWildcards( int* argc, char*** argv ) {
		struct _finddata_t fileinfo;
		int handle;
		int i;
		char filename[ 1024 ];
		char filebase[ 1024 ];
		char* path;

		ex_argc = 0;
		for ( i = 0; i < *argc; i++ ) {
			path = ( *argv )[ i ];
			if ( path[ 0 ] == '-' || ( !strstr( path, "*" ) && !strstr( path, "?" ) ) ) {
				ex_argv[ ex_argc++ ] = path;
				continue;
			}

			handle = _findfirst( path, &fileinfo );
			if ( handle == -1 )
				return;

			Q_ExtractFilePath( path, filebase, sizeof( filebase ) );

			do {
				sprintf( filename, "%s%s", filebase, fileinfo.name );
				ex_argv[ ex_argc++ ] = copystring( filename );
			} while ( _findnext( handle, &fileinfo ) != -1 );

			_findclose( handle );
		}

		*argc = ex_argc;
		*argv = ex_argv;
	}
#else
	void ExpandWildcards( int* argc, char*** argv ) { }
#endif


// only printf if in verbose mode
qboolean verbose = false;
void qprintf( const char* format, ... ) {
	if (! verbose ) {
		return;
	}

	va_list argptr;
	va_start( argptr, format );

	char str[ 2048 ];
	Q_vsnprintf( str, sizeof( str ), format, argptr );

	#if defined( CMDLIB_NODBGLIB )
		printf( "%s", str );
	#else
		Msg( "%s", str );
	#endif

	va_end( argptr );
}


// ---------------------------------------------------------------------------------------------------- //
// Helpers.
// ---------------------------------------------------------------------------------------------------- //

static void CmdLib_getwd( char* out, int outSize ) {
	#if IsWindows() || IsWindows()
		_getcwd( out, outSize );
		Q_strncat( out, "\\", outSize, COPY_ALL_CHARACTERS );
	#else
		getcwd( out, outSize );
		strcat( out, "/" );
	#endif
	Q_FixSlashes( out );
}

char* ExpandArg( char* path ) {
	static char full[ 1024 ];

	if ( path[ 0 ] != '/' && path[ 0 ] != '\\' && path[ 1 ] != ':' ) {
		CmdLib_getwd( full, sizeof( full ) );
		Q_strncat( full, path, sizeof( full ), COPY_ALL_CHARACTERS );
	} else {
		Q_strncpy( full, path, sizeof( full ) );
	}
	return full;
}


char* ExpandPath( char* path ) {
	static char full[ 1024 ];
	if ( path[ 0 ] == '/' || path[ 0 ] == '\\' || path[ 1 ] == ':' ) {
		return path;
	}
	sprintf( full, "%s%s", qdir, path );
	return full;
}


char* copystring( const char* s ) {
	char* b;
	b = (char*) malloc( strlen( s ) + 1 );
	strcpy( b, s );
	return b;
}


void GetHourMinuteSeconds( int nInputSeconds, int& nHours, int& nMinutes, int& nSeconds ) { }


void GetHourMinuteSecondsString( int nInputSeconds, char* pOut, int outLen ) {
	int nMinutes = nInputSeconds / 60;
	int nSeconds = nInputSeconds - nMinutes * 60;
	int nHours = nMinutes / 60;
	nMinutes -= nHours * 60;

	const char* extra[2] { "", "s" };

	if ( nHours > 0 ) {
		Q_snprintf( pOut, outLen, "%d hour%s, %d minute%s, %d second%s", nHours, extra[ nHours != 1 ], nMinutes, extra[ nMinutes != 1 ], nSeconds, extra[ nSeconds != 1 ] );
	} else if ( nMinutes > 0 ) {
		Q_snprintf( pOut, outLen, "%d minute%s, %d second%s", nMinutes, extra[ nMinutes != 1 ], nSeconds, extra[ nSeconds != 1 ] );
	} else {
		Q_snprintf( pOut, outLen, "%d second%s", nSeconds, extra[ nSeconds != 1 ] );
	}
}


void Q_mkdir( char* path ) {
	#if IsWindows()
		if ( _mkdir( path ) != -1 ) {
			return;
		}
	#else
		if ( mkdir( path, 0777 ) != -1 ) {
			return;
		}
	#endif
	//	if (errno != EEXIST)
	Error( "mkdir failed %s\n", path );
}

void CmdLib_InitFileSystem( const char* pFilename, int maxMemoryUsage ) {
	FileSystem_Init( pFilename, maxMemoryUsage );
	if ( !g_pFileSystem ) {
		Error( "CmdLib_InitFileSystem failed." );
	}
}

void CmdLib_TermFileSystem() {
	FileSystem_Term();
}

CreateInterfaceFn CmdLib_GetFileSystemFactory() {
	return FileSystem_GetFactory();
}


/*
============
FileTime

returns -1 if not present
============
*/
int FileTime( char* path ) {
	struct stat buf{};

	if ( stat( path, &buf ) == -1 ) {
		return -1;
	}

	return buf.st_mtime;
}


/*
==============
COM_Parse

Parse a token out of a string
==============
*/
char* COM_Parse( char* data ) {
	return ParseFile( data, com_token, nullptr );
}


/*
=============================================================================

						MISC FUNCTIONS

=============================================================================
*/


/*
=================
CheckParm

Checks for the given parameter in the program's command line arguments
Returns the argument number (1 to argc-1) or 0 if not present
=================
*/
int CheckParm( char* check ) {
	for ( int i = 1; i < myargc; i++ ) {
		if ( !Q_strcasecmp( check, myargv[ i ] ) ) {
			return i;
		}
	}

	return 0;
}


/*
================
Q_filelength
================
*/
int Q_filelength( FileHandle_t f ) {
	return g_pFileSystem->Size( f );
}


FileHandle_t SafeOpenWrite( const char* filename ) {
	FileHandle_t f = g_pFileSystem->Open( filename, "wb" );

	if ( !f ) {
		//Error ("Error opening %s: %s",filename,strerror(errno));
		// BUGBUG: No way to get equivalent of errno from IFileSystem!
		Error( "Error opening %s! (Check for write enable)\n", filename );
	}

	return f;
}

#define MAX_CMDLIB_BASE_PATHS 10
static char g_pBasePaths[ MAX_CMDLIB_BASE_PATHS ][ MAX_PATH ];
static int g_NumBasePaths = 0;

void CmdLib_AddBasePath( const char* pPath ) {
	//	printf( "CmdLib_AddBasePath( \"%s\" )\n", pPath );
	if ( g_NumBasePaths < MAX_CMDLIB_BASE_PATHS ) {
		Q_strncpy( g_pBasePaths[ g_NumBasePaths ], pPath, MAX_PATH );
		Q_FixSlashes( g_pBasePaths[ g_NumBasePaths ] );
		g_NumBasePaths++;
	} else {
		Assert( 0 );
	}
}

bool CmdLib_HasBasePath( const char* pFileName_, int& pathLength ) {
	char* pFileName = (char*) _alloca( strlen( pFileName_ ) + 1 );
	strcpy( pFileName, pFileName_ );
	Q_FixSlashes( pFileName );
	pathLength = 0;
	int i;
	for ( i = 0; i < g_NumBasePaths; i++ ) {
		// see if we can rip the base off of the filename.
		if ( Q_strncasecmp( g_pBasePaths[ i ], pFileName, strlen( g_pBasePaths[ i ] ) ) == 0 ) {
			pathLength = strlen( g_pBasePaths[ i ] );
			return true;
		}
	}
	return false;
}

int CmdLib_GetNumBasePaths() {
	return g_NumBasePaths;
}

const char* CmdLib_GetBasePath( int i ) {
	Assert( i >= 0 && i < g_NumBasePaths );
	return g_pBasePaths[ i ];
}


//-----------------------------------------------------------------------------
// Like ExpandPath but expands the path for each base path like SafeOpenRead
//-----------------------------------------------------------------------------
int CmdLib_ExpandWithBasePaths( CUtlVector<CUtlString>& expandedPathList, const char* pszPath ) {
	int nPathLength = 0;

	pszPath = ExpandPath( const_cast<char*>( pszPath ) );// Kind of redundant but it's how CmdLib_HasBasePath needs things

	if ( CmdLib_HasBasePath( pszPath, nPathLength ) ) {
		pszPath = pszPath + nPathLength;
		for ( int i = 0; i < CmdLib_GetNumBasePaths(); ++i ) {
			CUtlString& expandedPath = expandedPathList[ expandedPathList.AddToTail( CmdLib_GetBasePath( i ) ) ];
			expandedPath += pszPath;
		}
	} else {
		expandedPathList.AddToTail( pszPath );
	}

	return expandedPathList.Count();
}


FileHandle_t SafeOpenRead( const char* filename ) {
	int pathLength;
	if ( CmdLib_HasBasePath( filename, pathLength ) ) {
		filename = filename + pathLength;
		for ( int i = 0; i < g_NumBasePaths; i++ ) {
			char tmp[ MAX_PATH ];
			strcpy( tmp, g_pBasePaths[ i ] );
			strcat( tmp, filename );
			if ( auto f = g_pFileSystem->Open( tmp, "rb" ) ) {
				return f;
			}
		}
		Error( "Error opening %s\n", filename );
		return nullptr;
	} else {
		auto f = g_pFileSystem->Open( filename, "rb" );
		if (! f ) {
			Error( "Error opening %s", filename );
		}

		return f;
	}
}

void SafeRead( FileHandle_t f, void* buffer, int count ) {
	if ( g_pFileSystem->Read( buffer, count, f ) != (size_t) count ) {
		Error( "File read failure" );
	}
}


void SafeWrite( FileHandle_t f, void* buffer, int count ) {
	if ( g_pFileSystem->Write( buffer, count, f ) != (size_t) count ) {
		Error( "File write failure" );
	}
}


/*
==============
FileExists
==============
*/
qboolean FileExists( const char* filename ) {
	FileHandle_t hFile = g_pFileSystem->Open( filename, "rb" );
	if ( hFile == FILESYSTEM_INVALID_HANDLE ) {
		return false;
	} else {
		g_pFileSystem->Close( hFile );
		return true;
	}
}

/*
==============
LoadFile
==============
*/
int LoadFile( const char* filename, void** bufferptr ) {
	auto file{ SafeOpenRead( filename ) };
	if ( FILESYSTEM_INVALID_HANDLE == file ) {
		*bufferptr = nullptr;
		return 0;
	}

	int length{ Q_filelength( file ) };
	char* buffer{ new char[ length + 1 ] };
	buffer[ length ] = 0;
	SafeRead( file, buffer, length );
	g_pFileSystem->Close( file );
	*bufferptr = buffer;
	return length;
}


/*
==============
SaveFile
==============
*/
void SaveFile( const char* filename, void* buffer, int count ) {
	FileHandle_t f = SafeOpenWrite( filename );
	SafeWrite( f, buffer, count );
	g_pFileSystem->Close( f );
}

/*
====================
Extract file parts
====================
*/
// FIXME: should include the slash, otherwise
// backing to an empty path will be wrong when appending a slash


/*
==============
ParseNum / ParseHex
==============
*/
int ParseHex( char* hex ) {
	char* str;
	int num;

	num = 0;
	str = hex;

	while ( *str ) {
		num <<= 4;
		if ( *str >= '0' && *str <= '9' ) {
			num += *str - '0';
		} else if ( *str >= 'a' && *str <= 'f' ) {
			num += 10 + *str - 'a';
		} else if ( *str >= 'A' && *str <= 'F' ) {
			num += 10 + *str - 'A';
		} else {
			Error( "Bad hex number: %s", hex );
		}
		str++;
	}

	return num;
}


int ParseNum( char* str ) {
	if ( str[ 0 ] == '$' ) {
		return ParseHex( str + 1 );
	}
	if ( str[ 0 ] == '0' && str[ 1 ] == 'x' ) {
		return ParseHex( str + 2 );
	}
	return atol( str );
}

/*
============
CreatePath
============
*/
void CreatePath( char* path ) {
	char *ofs, c;

	// strip the drive
	if ( path[ 1 ] == ':' ) {
		path += 2;
	}

	for ( ofs = path + 1; *ofs; ofs++ ) {
		c = *ofs;
		if ( c == '/' || c == '\\' ) {// create the directory
			*ofs = 0;
			Q_mkdir( path );
			*ofs = c;
		}
	}
}

//-----------------------------------------------------------------------------
// Creates a path, path may already exist
//-----------------------------------------------------------------------------
#if IsWindows()
	void SafeCreatePath( char* path ) {
		char* ptr;

		// skip past the drive path, but don't strip
		if ( path[ 1 ] == ':' ) {
			ptr = strchr( path, '\\' );
		} else {
			ptr = path;
		}
		while ( ptr ) {
			ptr = strchr( ptr + 1, '\\' );
			if ( ptr ) {
				*ptr = '\0';
				_mkdir( path );
				*ptr = '\\';
			}
		}
	}
#endif

/*
============
QCopyFile

  Used to archive source files
============
*/
void QCopyFile( char* from, char* to ) {
	void* buffer;
	int length;

	length = LoadFile( from, &buffer );
	CreatePath( to );
	SaveFile( to, buffer, length );
	free( buffer );
}
