//
// Created by ENDERZOMBI102 on 09/02/2024.
//
#include "tier0/dbg.h"
#include <cassert>
#include <string>
#include <unordered_map>
#include "icommandline.h"
#include "Color.h"

static SpewOutputFunc_t g_pSpewOutFunction{ DefaultSpewFunc };
static AssertFailedNotifyFunc_t g_pAssertFailedListener{ nullptr };
static bool g_bAssertionsDisabled{ false };
static SDL_Window* g_pDialogParent{ nullptr };
static std::unordered_map<const char*, int> g_GroupsData{};
static struct {
	SpewType_t m_eType{ SpewType_t::SPEW_MESSAGE };
	const tchar* m_sFile{ nullptr };
	int m_iLine{ 0 };
} g_sSpewInfo;


void SpewOutputFunc( SpewOutputFunc_t func ) {
	if ( func ) {
		g_pSpewOutFunction = func;
	} else {
		g_pSpewOutFunction = DefaultSpewFunc;
	}
}

SpewOutputFunc_t GetSpewOutputFunc() {
	return g_pSpewOutFunction;
}

SpewRetval_t DefaultSpewFunc( SpewType_t pSpewType, const tchar* pMsg ) {
	switch ( pSpewType ) {
		case SPEW_MESSAGE:
			printf( "[I] %s", pMsg );
			break;
		case SPEW_WARNING:
			printf( "[W] %s", pMsg );
			break;
		case SPEW_ASSERT:
			printf( "[A] %s", pMsg );
			return SpewRetval_t::SPEW_DEBUGGER;
		case SPEW_ERROR:
			printf( "[E] %s", pMsg );
			break;
		case SPEW_LOG:
			printf( "[D] %s", pMsg );
			break;
		default:
			printf( "Invalid spew type: %d (msg=`%s`)", pSpewType, pMsg );
			return SpewRetval_t::SPEW_DEBUGGER;
	}
	return SpewRetval_t::SPEW_CONTINUE;
}

SpewRetval_t DefaultSpewFuncAbortOnAsserts( SpewType_t pSpewType, const tchar* pMsg ) {
	auto res{ DefaultSpewFunc( pSpewType, pMsg ) };
	if ( pSpewType == SpewType_t::SPEW_ASSERT ) {
		return SpewRetval_t::SPEW_ABORT;
	}

	return res;
}

// TODO: Implement these
const tchar* GetSpewOutputGroup() { AssertUnreachable(); return nullptr; }
int GetSpewOutputLevel() { AssertUnreachable(); return 0; }
const Color* GetSpewOutputColor() {
	static Color spewColors[SpewType_t::SPEW_TYPE_COUNT] {
		{ 0xB5, 0xB6, 0xE3, 0 },  // SPEW_MESSAGE
		{ 0xC6, 0xAF, 0x35, 0 },  // SPEW_WARNING
		{ 0xE6, 0xA0, 0x29, 0 },  // SPEW_ASSERT
		{ 0xC6, 0x4D, 0x3F, 0 },  // SPEW_ERROR
		{ 0x71, 0x58, 0x3E, 0 }   // SPEW_LOG
	};
	return &spewColors[g_sSpewInfo.m_eType];
}

// TODO: Implement these
void SpewActivate( const tchar* pGroupName, int level ) {
	g_GroupsData[pGroupName] = level;
}
bool IsSpewActive( const tchar* pGroupName, int level ) {
	return g_GroupsData[pGroupName] >= level;
}

// TODO: Implement these
void _SpewInfo( SpewType_t pType, const tchar* pFile, int pLine ) {
	g_sSpewInfo.m_eType = pType;
	g_sSpewInfo.m_sFile = pFile;
	g_sSpewInfo.m_iLine = pLine;
}
SpewRetval_t _SpewMessage( const tchar* pMsg, ... ) {
	char buffer[MAX_PATH] { 0 };

	va_list args;
	va_start( args, pMsg );
	vsnprintf( buffer, sizeof( buffer ), pMsg, args );
	va_end( args );

	return g_pSpewOutFunction( g_sSpewInfo.m_eType, buffer );
}
SpewRetval_t _DSpewMessage( const tchar* pGroupName, int level, const tchar* pMsg, ... ) { assert( false ); return {}; }
SpewRetval_t ColorSpewMessage( SpewType_t type, const Color* pColor, const tchar* pMsg, ... ) { assert( false ); return {}; }
void _ExitOnFatalAssert( const tchar* pFile, int line ) { exit( 1 ); }
bool ShouldUseNewAssertDialog() { return true; }

bool DoNewAssertDialog( const tchar* pFile, int line, const tchar* pExpression ) {
	using namespace std::string_literals;
	auto message{
		"\n---- Assertion Failed ----"
		"\nWhere: "s + pFile + ":" + std::to_string( line ) +
		"\nAssert: " + pExpression +
		"\n--------------------------"
	};

	puts( message.c_str() );

	return true;
}

// TODO: Implement these
bool AreAllAssertsDisabled() {
	return g_bAssertionsDisabled;
}
void SetAllAssertsDisabled( bool bAssertsEnabled ) {
	g_bAssertionsDisabled = bAssertsEnabled;
}

void SetAssertFailedNotifyFunc( AssertFailedNotifyFunc_t pFunc ) {
	g_pAssertFailedListener = pFunc;
}
void CallAssertFailedNotifyFunc( const char* pchFile, int nLine, const char* pchMessage ) {
	if ( g_pAssertFailedListener ) {
		g_pAssertFailedListener( pchFile, nLine, pchMessage );
	}
}

bool HushAsserts() {
	return CommandLine()->FindParm( "-hushasserts" ) != 0;
}

void SetAssertDialogParent( struct SDL_Window* pWindow ) {
	g_pDialogParent = pWindow;
}
struct SDL_Window* GetAssertDialogParent() {
	return g_pDialogParent;
}


static void SpewInternal( SpewType_t pType, const tchar* pMsg, const va_list& args ) {
	char buffer[MAX_PATH] { 0 };
	vsnprintf( buffer, sizeof( buffer ), pMsg, args );
	auto res{ g_pSpewOutFunction( pType, buffer ) };

	if ( res == SpewRetval_t::SPEW_CONTINUE ) {
		return;
	}

	if ( res == SpewRetval_t::SPEW_ABORT ) {
		puts( "Fatal spew! Aborting execution." );
		exit( 1 );
	}

	DebuggerBreak();
}

void Msg( const tchar* pMsg, ... ) {
	va_list args;
	va_start( args, pMsg );
	SpewInternal( SpewType_t::SPEW_MESSAGE, pMsg, args );
	va_end( args );
}

void Warning( const tchar* pMsg, ... ) {
	va_list args;
	va_start( args, pMsg );
	SpewInternal( SpewType_t::SPEW_WARNING, pMsg, args );
	va_end( args );
}

void Log( const tchar* pMsg, ... ) {
	va_list args;
	va_start( args, pMsg );
	SpewInternal( SpewType_t::SPEW_LOG, pMsg, args );
	va_end( args );
}

void Error( const tchar* pMsg, ... ) {
	va_list args;
	va_start( args, pMsg );
	SpewInternal( SpewType_t::SPEW_ERROR, pMsg, args );
	va_end( args );
	// for some reason, all errors are fatal...
	exit(1);
}

// ---- Dev*
// TODO: Actually implement these
/* These locked at the "developer" group */
void DevMsg( int level, const tchar* pMsg, ... ) {
	va_list args;
	va_start( args, pMsg );
	putchar( 'M' );
	putchar( '|' );
	vprintf( pMsg, args );
	va_end( args );
}
void DevWarning( int level, const tchar* pMsg, ... ) {
	va_list args;
	va_start( args, pMsg );
	putchar( 'W' );
	putchar( '|' );
	vprintf( pMsg, args );
	va_end( args );
}
void DevLog( int level, const tchar* pMsg, ... ) {
	va_list args;
	va_start( args, pMsg );
	putchar( 'L' );
	putchar( '|' );
	vprintf( pMsg, args );
	va_end( args );
}

/* default level versions (level 1) */
void DevMsg( const tchar* pMsg, ... ) {
	va_list args;
	va_start( args, pMsg );
	putchar( 'M' );
	putchar( '|' );
	vprintf( pMsg, args );
	va_end( args );
}
void DevWarning( const tchar* pMsg, ... ) {
	va_list args;
	va_start( args, pMsg );
	putchar( 'W' );
	putchar( '|' );
	vprintf( pMsg, args );
	va_end( args );
}
void DevLog( const tchar* pMsg, ... ) {
	va_list args;
	va_start( args, pMsg );
	putchar( 'L' );
	putchar( '|' );
	vprintf( pMsg, args );
	va_end( args );
}

/* These locked at the "console" group */
void ConColorMsg( int level, const Color& clr, const tchar* pMsg, ... ) {
	va_list args;
	va_start( args, pMsg );
	putchar( 'M' );
	putchar( '|' );
	vprintf( pMsg, args );
	va_end( args );
}
void ConMsg( int level, const tchar* pMsg, ... ) {
	va_list args;
	va_start( args, pMsg );
	putchar( 'M' );
	putchar( '|' );
	vprintf( pMsg, args );
	va_end( args );
}
void ConWarning( int level, const tchar* pMsg, ... ) {
	va_list args;
	va_start( args, pMsg );
	putchar( 'W' );
	putchar( '|' );
	vprintf( pMsg, args );
	va_end( args );
}
void ConLog( int level, const tchar* pMsg, ... ) {
	va_list args;
	va_start( args, pMsg );
	putchar( 'L' );
	putchar( '|' );
	vprintf( pMsg, args );
	va_end( args );
}

/* default console version (level 1) */
void ConColorMsg( const Color& clr, const tchar* pMsg, ... ) {
	va_list args;
	va_start( args, pMsg );
	putchar( 'M' );
	putchar( '|' );
	vprintf( pMsg, args );
	va_end( args );
}
void ConMsg( const tchar* pMsg, ... ) {
	va_list args;
	va_start( args, pMsg );
	putchar( 'M' );
	putchar( '|' );
	vprintf( pMsg, args );
	va_end( args );
}
void ConWarning( const tchar* pMsg, ... ) {
	va_list args;
	va_start( args, pMsg );
	putchar( 'W' );
	putchar( '|' );
	vprintf( pMsg, args );
	va_end( args );
}
void ConLog( const tchar* pMsg, ... ) {
	va_list args;
	va_start( args, pMsg );
	putchar( 'L' );
	putchar( '|' );
	vprintf( pMsg, args );
	va_end( args );
}

/* developer console version (level 2) */
void ConDColorMsg( const Color& clr, const tchar* pMsg, ... );
void ConDMsg( const tchar* pMsg, ... );
void ConDWarning( const tchar* pMsg, ... );
void ConDLog( const tchar* pMsg, ... );

/* These locked at the "network" group */
void NetMsg( int level, const tchar* pMsg, ... );
void NetWarning( int level, const tchar* pMsg, ... );
void NetLog( int level, const tchar* pMsg, ... );

// ---- ....
//
void ValidateSpew( class CValidator& validator );

void COM_TimestampedLog( const char* fmt, ... ) {
	char buffer[1024] { };

	va_list args;
	va_start( args, fmt );
	vsnprintf( buffer, std::size( buffer ), fmt, args );
	va_end( args );
	Msg( "[TSMP LOG] %s\n", buffer );  // FIXME: Actually impl this or remove it
}

// ---- Validity asserts
// TODO: Figure out how to do these without crashing the engine
void _AssertValidReadPtr( void* ptr, int count ) { }
void _AssertValidWritePtr( void* ptr, int count ) { }
void _AssertValidReadWritePtr( void* ptr, int count ) { }
void AssertValidStringPtr( const tchar* ptr, int maxchar ) { }


