//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Abstract away and help with platform-specific code.
//
// $NoKeywords: $
// TODO: Move as much as possible to CMake, less moving parts are good
//
//===========================================================================//
#pragma once
#include "basetypes.h"
#include "tier0/valve_off.h"
#include "wchartypes.h"

#if defined( PLATFORM_POSIX )
	#include <alloca.h> // need this for _alloca
	#include <unistd.h>
	#include <ctime>
#endif

#include <malloc.h>
#include <new>

// need this for memset
#include <cstring>

//-----------------------------------------------------------------------------
// Set up platform type defines.
//-----------------------------------------------------------------------------
#if defined( PLATFORM_64BITS )
	#define IsPlatform64Bits() true
#else
	#define IsPlatform64Bits() false
#endif
#if defined( PLATFORM_DEBUG )
	#define IsDebug() true
#else
	#define IsDebug() false
#endif
#if defined( PLATFORM_RETAIL )
	#define IsRetail() true
#else
	#define IsRetail() false
#endif
#if defined( PLATFORM_RELEASE )
	#define IsRelease() true
#else
	#define IsRelease() false
#endif

#define IsPC() true
#define IsConsole() false
#define IsWindows() false
#define IsPlatformWindowsPC32() false
#define IsPlatformWindowsPC64() false
#define IsPlatformOpenGL() false
#define IsPosix() false
#define IsLinux() false

#if defined( PLATFORM_WINDOWS )
	#undef IsWindows
	#define IsWindows() true

	#if defined( PLATFORM_64BITS )
		#undef IsPlatformWindowsPC64
		#define IsPlatformWindowsPC64() true
	#else
		#undef IsPlatformWindowsPC32
		#define IsPlatformWindowsPC32() true
	#endif
	// Adding IsPlatformOpenGL() to help fix a bunch of code that was using IsPosix() to infer if the DX->GL translation layer was being used.
	#if defined( DX_TO_GL_ABSTRACTION )
		#udnef IsPlatformOpenGL
		#define IsPlatformOpenGL() true
	#endif
#elif defined( PLATFORM_POSIX )
	#undef IsPosix
	#define IsPosix() true

	#undef IsPlatformOpenGL
	#define IsPlatformOpenGL() true
	#define __cdecl  // override __cdecl to be nothing, used in some places (for now >:3) NOLINT(*-reserved-identifier)

	#if defined( PLATFORM_LINUX )
		#undef IsLinux
		#define IsLinux() true
	#endif
#else
	#error "Unsupported platform, please implement!"
#endif

typedef unsigned char uint8;
typedef signed char int8;

#if IsWindows()
	typedef   signed __int16  int16;
	typedef unsigned __int16 uint16;
	typedef   signed __int32  int32;
	typedef unsigned __int32 uint32;
	typedef   signed __int64  int64;
	typedef unsigned __int64 uint64;
	#if IsPlatform64Bits()
		typedef   signed __int64  intp;
		typedef unsigned __int64 uintp;
	#else
		typedef   signed __int32  intp;
		typedef unsigned __int32 uintp;
	#endif
#else
	typedef   signed short     int16;
	typedef unsigned short     uint16;
	typedef   signed int       int32;
	typedef unsigned int       uint32;
	typedef   signed long long int64;
	typedef unsigned long long uint64;
	#if IsPlatform64Bits()
		typedef long long intp;
		typedef unsigned long long uintp;
	#else
		typedef int intp;
		typedef unsigned int uintp;
	#endif
	typedef void* HWND;
#endif

// From `steam/steamtypes.h`: RTime32
// We use this 32 bit time representing real world time.
// It offers 1 second resolution beginning on January 1, 1970 (Unix time)
typedef uint32 RTime32;

typedef float float32;
typedef double float64;

// for when we don't care about how many bits we use
typedef unsigned int uint;

// Check that the various sized types have the size we expect
static_assert( sizeof(  int16 ) == 2 );
static_assert( sizeof( uint16 ) == 2 );
static_assert( sizeof(  int32 ) == 4 );
static_assert( sizeof( uint32 ) == 4 );
static_assert( sizeof(  int64 ) == 8 );
static_assert( sizeof( uint64 ) == 8 );
static_assert( sizeof(  intp ) == sizeof(void*) ); // intp is an integer that can accommodate a pointer
static_assert( sizeof( uintp ) == sizeof(void*) ); //  ( ie, sizeof(intp) >= sizeof(int) && sizeof(intp) >= sizeof(void *) )

// TODO: Verify if this is still needed, with most probability not.
#if defined( COMPILER_MSVC )
	#pragma once
	// Ensure that everybody has the right compiler version installed. The version
	// number can be obtained by looking at the compiler output when you type 'cl'
	// and removing the last two digits and the periods: 16.00.40219.01 becomes 160040219
	#if _MSC_FULL_VER > 180000000
		#if _MSC_FULL_VER < 180030723
			#error You must install VS 2013 Update 3
		#endif
	#elif _MSC_FULL_VER > 160000000
		#if _MSC_FULL_VER < 160040219
			#error You must install VS 2010 SP1
		#endif
	#else
		#if _MSC_FULL_VER < 140050727
			#error You must install VS 2005 SP1
		#endif
	#endif
#endif

// This can be used to ensure the size of pointers to members when declaring
// a pointer type for a class that has only been forward declared
#if defined( COMPILER_MSVC ) || defined( COMPILER_CLANG )
	#define SINGLE_INHERITANCE __single_inheritance
	#define MULTIPLE_INHERITANCE __multiple_inheritance
#else
	#define SINGLE_INHERITANCE
	#define MULTIPLE_INHERITANCE
#endif

#if defined( COMPILER_MSVC ) || defined( COMPILER_CLANG )
	#define NO_VTABLE __declspec( novtable )
#else
	#define NO_VTABLE
#endif

// This indicates that a function never returns, which helps with
// generating accurate compiler warnings
#define NORETURN [[noreturn]]

// This can be used to declare an abstract (interface only) class.
// Classes marked abstract should not be instantiated; If they are, an access violation will occur.
//
// Example of use:
//
// abstract_class CFoo {
//      ...
// }
//
// MSDN __declspec(novtable) documentation: http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vclang/html/_langref_novtable.asp
//
// Note: NJS: This is not enabled for regular PC, due to not knowing the implications of exporting a class with no vtable.
//       It's probable that this shouldn't be an issue, but an experiment should be done to verify this.
//
#define abstract_class /*__declspec(novtable)*/ class


// MSVC CRT uses 0x7fff while gcc uses MAX_INT, leading to mismatches between platforms
// As a result, we pick the least common denominator here.  This should be used anywhere
// you might typically want to use RAND_MAX
#define VALVE_RAND_MAX 0x7fff


/*
FIXME: Enable this when we no longer fear change =)

// need these for the limits
#include <float.h>
#include <limits.h>

// Maximum and minimum representable values
#define  INT8_MAX			SCHAR_MAX
#define  INT16_MAX			SHRT_MAX
#define  INT32_MAX			LONG_MAX
#define  INT64_MAX			(((int64)~0) >> 1)

#define  INT8_MIN			SCHAR_MIN
#define  INT16_MIN			SHRT_MIN
#define  INT32_MIN			LONG_MIN
#define  INT64_MIN			(((int64)1) << 63)

#define  UINT8_MAX			((uint8)~0)
#define  UINT16_MAX			((uint16)~0)
#define  UINT32_MAX			((uint32)~0)
#define  UINT64_MAX			((uint64)~0)

#define  UINT8_MIN			0
#define  UINT16_MIN			0
#define  UINT32_MIN			0
#define  UINT64_MIN			0

#ifndef  UINT_MIN
#define  UINT_MIN			UINT32_MIN
#endif

#define  FLOAT32_MAX		FLT_MAX
#define  FLOAT64_MAX		DBL_MAX

#define  FLOAT32_MIN FLT_MIN
#define  FLOAT64_MIN DBL_MIN
*/

// portability / compiler settings
#if IsWindows() && !defined( WINDED )
	#if defined( _M_IX86 )
		#define __i386__ 1
	#endif
#elif IsPosix()
	typedef unsigned int DWORD;
	typedef unsigned short WORD;
	typedef void* HINSTANCE;
	#define _MAX_PATH PATH_MAX // NOLINT(*-reserved-identifier)
	#define __cdecl            // NOLINT(*-reserved-identifier)
	#define __stdcall          // NOLINT(*-reserved-identifier)
	#define __declspec         // NOLINT(*-reserved-identifier)
#endif


// Defines MAX_PATH
#if !defined( MAX_PATH )
	#define MAX_PATH 260
#endif

#if IsWindows()
	#define MAX_UNICODE_PATH 32767
#else
	#define MAX_UNICODE_PATH MAX_PATH
#endif

#define MAX_UNICODE_PATH_IN_UTF8 (MAX_UNICODE_PATH * 4)

#undef offsetof
#if defined( COMPILER_GCC )
	//#define offsetof( type, var ) __builtin_offsetof( type, var )
	#define offsetof( s, m ) ( size_t ) & ( ( (s*) 0 )->m )
#else
	#define offsetof( s, m ) ( size_t ) & ( ( (s*) 0 )->m )
#endif


#define ALIGN_VALUE( val, alignment ) ( ( val + alignment - 1 ) & ~( alignment - 1 ) )//  need macro for constant expression

// Used to step into the debugger
#if defined( COMPILER_MSVC )
	#define DebuggerBreak() __debugbreak()
#elif defined( COMPILER_GCC )
	#define DebuggerBreak() __asm__ volatile( "int $0x03" )
#elif defined( COMPILER_CLANG )
	#define DebuggerBreak() __builtin_debugtrap()
#endif
#define DebuggerBreakIfDebugging()  \
	if ( !Plat_IsInDebugSession() ) \
		;                           \
	else                            \
		DebuggerBreak()

#if defined( STAGING_ONLY )
	#define DebuggerBreakIfDebugging_StagingOnly() \
		if ( !Plat_IsInDebugSession() )            \
			;                                      \
		else                                       \
			DebuggerBreak()
#else
	#define DebuggerBreakIfDebugging_StagingOnly()
#endif

// Allows you to specify code that should only execute if we are in a staging build. Otherwise the code noops.
#if defined( STAGING_ONLY )
	#define STAGING_ONLY_EXEC( _exec ) \
		do { _exec; } while ( false )
#else
	#define STAGING_ONLY_EXEC( _exec ) \
		do ; while ( false )
#endif

// C functions for external declarations that call the appropriate C++ methods
#if !defined( EXPORT )
	#if IsWindows()
		#define EXPORT _declspec( dllexport )
	#else
		#define EXPORT /* */
	#endif
#endif

#if defined( __i386__ ) && !IsLinux()
	#define id386 1
#else
	#define id386 0
#endif

// decls for aligning data
#if IsWindows()
	#define DECL_ALIGN( x ) __declspec( align( x ) )
#elif defined( COMPILER_GCC )
	#define DECL_ALIGN( x ) __attribute__( ( aligned( x ) ) )
#else
	#define DECL_ALIGN( x ) /* */
#endif

#if defined( COMPILER_MSVC ) || defined( COMPILER_CLANG )
	// MSVC has the align at the start of the struct
	#define ALIGN4 DECL_ALIGN( 4 )
	#define ALIGN8 DECL_ALIGN( 8 )
	#define ALIGN16 DECL_ALIGN( 16 )
	#define ALIGN32 DECL_ALIGN( 32 )
	#define ALIGN128 DECL_ALIGN( 128 )

	#define ALIGN4_POST
	#define ALIGN8_POST
	#define ALIGN16_POST
	#define ALIGN32_POST
	#define ALIGN128_POST
#elif defined( COMPILER_GCC )
	// gnuc has the align decoration at the end
	#define ALIGN4
	#define ALIGN8
	#define ALIGN16
	#define ALIGN32
	#define ALIGN128

	#define ALIGN4_POST DECL_ALIGN( 4 )
	#define ALIGN8_POST DECL_ALIGN( 8 )
	#define ALIGN16_POST DECL_ALIGN( 16 )
	#define ALIGN32_POST DECL_ALIGN( 32 )
	#define ALIGN128_POST DECL_ALIGN( 128 )
#else
	#error "ALIGN*: Unsupported platform, please implement!"
#endif

// Pull in the /analyze code annotations.
#include "annotations.h"

#if IsWindows()
	#include <direct.h>
#endif

//-----------------------------------------------------------------------------
// Convert int<-->pointer, avoiding 32/64-bit compiler warnings:
//-----------------------------------------------------------------------------
#define INT_TO_POINTER( i ) ((void*) ( ( i ) + (char*) NULL ))
#define POINTER_TO_INT( p ) ( (int) (uintp) ( p ) )


//-----------------------------------------------------------------------------
// Stack-based allocation related helpers
//-----------------------------------------------------------------------------
#if defined( COMPILER_GCC ) || defined( COMPILER_CLANG )
	#define stackalloc( _size ) alloca( ALIGN_VALUE( _size, 16 ) )
	#if IsLinux()
		#define mallocsize( _p ) ( malloc_usable_size( _p ) )
	#elif IsWindows()
		#define mallocsize( _p ) ( _msize( _p ) )
	#else
		#error "mallocsize: Unsupported platform, please implement!"
	#endif
#elif defined( COMPILER_MSVC )
	#define stackalloc( _size ) _alloca( ALIGN_VALUE( _size, 16 ) )
	#define mallocsize( _p ) ( _msize( _p ) )
#else
	#error "stackalloc: Mising implementation"
#endif

#define stackfree( _p ) 0

// Linux had a few areas where it didn't construct objects in the same order that Windows does.
// So when CVProfile::CVProfile() would access g_pMemAlloc, it would crash because the allocator wasn't initalized yet.
#if IsPosix()
	#define CONSTRUCT_EARLY __attribute__( ( init_priority( 101 ) ) )
#else
	#define CONSTRUCT_EARLY
#endif

#if defined( COMPILER_MSVC )
	#define SELECTANY __declspec( selectany )
	#define RESTRICT __restrict
	#define RESTRICT_FUNC __declspec( restrict )
	#define FMTFUNCTION( a, b )
#elif defined( COMPILER_GCC )
	#define SELECTANY __attribute__( ( weak ) )
	#if IsLinux() && !defined( DEDICATED )
		#define RESTRICT
	#else
		#define RESTRICT __restrict
	#endif
	#define RESTRICT_FUNC
	// squirrel.h does a #define printf DevMsg which leads to warnings when we try
	// to use printf as the prototype format function. Using __printf__ instead.
	#define FMTFUNCTION( fmtargnumber, firstvarargnumber ) __attribute__( ( format( __printf__, fmtargnumber, firstvarargnumber ) ) )
#else
	#define SELECTANY static
	#define RESTRICT
	#define RESTRICT_FUNC
	#define FMTFUNCTION( a, b )
#endif

#if defined( COMPILER_MSVC )
	// Used for dll exporting and importing
	#define DLL_EXPORT extern "C" __declspec( dllexport )
	#define DLL_EXPORT_NORET extern "C" [[noreturn]]
	#define DLL_IMPORT extern "C" __declspec( dllimport )
	#define DLL_IMPORT_NORET extern "C" [[noreturn]]

	// Can't use extern "C" when DLL exporting a class
	#define DLL_CLASS_EXPORT __declspec( dllexport )
	#define DLL_CLASS_IMPORT __declspec( dllimport )

	// Can't use extern "C" when DLL exporting a global
	#define DLL_GLOBAL_EXPORT extern __declspec( dllexport )
	#define DLL_GLOBAL_IMPORT extern __declspec( dllimport )

	#define DLL_LOCAL
#elif defined( COMPILER_GCC ) || defined( COMPILER_CLANG )
	// Used for dll exporting and importing
	#define DLL_EXPORT extern "C" [[gnu::visibility( "default" )]]
	#define DLL_EXPORT_NORET extern "C" [[gnu::visibility( "default" ), noreturn]]
	#define DLL_IMPORT extern "C"
	#define DLL_IMPORT_NORET extern "C" [[noreturn]]

	// Can't use extern "C" when DLL exporting a class
	#define DLL_CLASS_EXPORT [[gnu::visibility( "default" )]]
	#define DLL_CLASS_IMPORT

	// Can't use extern "C" when DLL exporting a global
	#define DLL_GLOBAL_EXPORT [[gnu::visibility( "default" )]] extern
	#define DLL_GLOBAL_IMPORT extern

	#define DLL_LOCAL// is now the default
#else
	#error "Unsupported compiler."
#endif

// Used for standard calling conventions
#if defined( COMPILER_MSVC )
	#define STDCALL __stdcall
	#define FASTCALL __fastcall
	#define ALWAYS_INLINE __forceinline
	// GCC 3.4.1 has a bug in supporting forced inline of templated functions
	// this macro lets us not force inlining in that case
	#define FORCEINLINE_TEMPLATE __forceinline
#elif defined( COMPILER_GCC ) || defined( COMPILER_CLANG )
	#define STDCALL
	#define FASTCALL __attribute__( ( fastcall ) )
	#define ALWAYS_INLINE [[gnu::always_inline]] inline
	// GCC 3.4.1 has a bug in supporting forced inline of templated functions
	// this macro lets us not force inlining in that case
	#define FORCEINLINE_TEMPLATE inline
#endif

// Force a function call site -not- to inlined. (useful for profiling)
#define DONT_INLINE( a ) ( ( (int) ( a ) + 1 ) ? ( a ) : ( a ) )

// Pass hints to the compiler to prevent it from generating unnecessary / stupid code
// in certain situations.  Several compilers other than MSVC also have an equivalent
// construct.
//
// Essentially the 'Hint' is that the condition specified is assumed to be true at
// that point in the compilation.  If '0' is passed, then the compiler assumes that
// any subsequent code in the same 'basic block' is unreachable, and thus usually
// removed.
#if defined( COMPILER_MSVC )
	#define HINT( THE_HINT ) __assume( ( THE_HINT ) )
#else
	#define HINT( THE_HINT ) 0
#endif


// In cases where no default is present or appropriate, this causes MSVC to generate
// as little code as possible, and throw an assertion in debug.
// REQUIRES: `<utility>`
#define NO_DEFAULT \
	default:       \
		std::unreachable();


#if defined( COMPILER_MSVC )
	// Remove warnings from warning level 4.
	#pragma warning( disable : 4514 )  // warning C4514: 'acosl' : unreferenced inline function has been removed
	#pragma warning( disable : 4100 )  // warning C4100: 'hwnd' : unreferenced formal parameter
	#pragma warning( disable : 4127 )  // warning C4127: conditional expression is constant
	#pragma warning( disable : 4512 )  // warning C4512: 'InFileRIFF' : assignment operator could not be generated
	#pragma warning( disable : 4611 )  // warning C4611: interaction between '_setjmp' and C++ object destruction is non-portable
	#pragma warning( disable : 4710 )  // warning C4710: function 'x' not inlined
	#pragma warning( disable : 4702 )  // warning C4702: unreachable code
	#pragma warning( disable : 4505 )  // unreferenced local function has been removed
	#pragma warning( disable : 4239 )  // nonstandard extension used : 'argument' ( conversion from class Vector to class Vector& )
	#pragma warning( disable : 4097 )  // typedef-name 'BaseClass' used as synonym for class-name 'CFlexCycler::CBaseFlex'
	#pragma warning( disable : 4324 )  // Padding was added at the end of a structure
	#pragma warning( disable : 4244 )  // type conversion warning.
	#pragma warning( disable : 4305 )  // truncation from 'const double ' to 'float '
	#pragma warning( disable : 4786 )  // Disable warnings about long symbol names
	#pragma warning( disable : 4250 )  // 'X' : inherits 'Y::Z' via dominance
	#pragma warning( disable : 4201 )  // nonstandard extension used : nameless struct/union
	#pragma warning( disable : 4481 )  // warning C4481: nonstandard extension used: override specifier 'override'
	#pragma warning( disable : 4748 )  // warning C4748: /GS can not protect parameters and local variables from local buffer overrun because optimizations are disabled in function

	#if _MSC_VER >= 1300
		#pragma warning( disable : 4511 )  // Disable warnings about private copy constructors
		#pragma warning( disable : 4121 )  // warning C4121: 'symbol' : alignment of a member was sensitive to packing
		#pragma warning( disable : 4530 )  // warning C4530: C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc (disabled due to std headers having exception syntax)
	#endif

	#if _MSC_VER >= 1400
		#pragma warning( disable : 4996 )  // functions declared deprecated
	#endif
#endif

#if IsLinux() && defined( COMPILER_GCC ) && ( ( __GNUC__ * 100 ) + __GNUC_MINOR__ ) >= 406
	// based on some Jonathan Wakely macros on the net...
	#define GCC_DIAG_STR( s ) #s
	#define GCC_DIAG_JOINSTR( x, y ) GCC_DIAG_STR( x##y )
	#define GCC_DIAG_DO_PRAGMA( x ) _Pragma( #x )
	#define GCC_DIAG_PRAGMA( x ) GCC_DIAG_DO_PRAGMA( GCC diagnostic x )

	#define GCC_DIAG_PUSH_OFF( x ) GCC_DIAG_PRAGMA( push ) GCC_DIAG_PRAGMA( ignored GCC_DIAG_JOINSTR( -W, x ) )
	#define GCC_DIAG_POP() GCC_DIAG_PRAGMA( pop )
#else
	#define GCC_DIAG_PUSH_OFF( x )
	#define GCC_DIAG_POP()
#endif

#if IsLinux()
	#pragma GCC diagnostic ignored "-Wconversion-null"  // passing NULL to non-pointer argument 1
	#pragma GCC diagnostic ignored "-Wpointer-arith"    // NULL used in arithmetic. Ie, vpanel == NULL where VPANEL is uint.
	#pragma GCC diagnostic ignored "-Wswitch"           // enumeration values not handled in switch
#endif


// When we port to 64 bit, we'll have to resolve the int, ptr vs size_t 32/64 bit problems...
#if defined( COMPILER_MSVC ) && !IsPlatform64Bits()
	#pragma warning( disable : 4267 )  // conversion from 'size_t' to 'int', possible loss of data
	#pragma warning( disable : 4311 )  // pointer truncation from 'char *' to 'int'
	#pragma warning( disable : 4312 )  // conversion from 'unsigned int' to 'memhandle_t' of greater size
#endif


#if IsPosix()
	#define _stricmp stricmp
	#define strcmpi stricmp
	#define stricmp strcasecmp
	#define _vsnprintf vsnprintf
	#define _alloca alloca
	#if defined( _snprintf )
		#undef _snprintf
	#endif
	#define _snprintf snprintf
	#define GetProcAddress( ptr, name ) ({                                 \
		static_assert( std::is_same_v<decltype( ptr ), HMODULE> == true ); \
		dlsym( reinterpret_cast<void*>( ptr ), name );                     \
	})
	#define _chdir chdir
	#define _strnicmp strnicmp
	#define strnicmp strncasecmp
	#define _getcwd getcwd
	#define _snwprintf swprintf
	#define swprintf_s swprintf
	#define wcsicmp _wcsicmp
	#define _wcsicmp wcscmp
	#define _tempnam tempnam
	#define _finite finite
	#define _unlink unlink
	#define _access access
	#define _mkdir( dir ) mkdir( dir, S_IRWXU | S_IRWXG | S_IRWXO )
	#define _wtoi( arg ) wcstol( arg, NULL, 10 )
	#define _wtoi64( arg ) wcstoll( arg, NULL, 10 )

	typedef uint32 HMODULE;
	typedef void* HANDLE;
#endif

//-----------------------------------------------------------------------------
// fsel
//-----------------------------------------------------------------------------
ALWAYS_INLINE
static float fsel( float fComparand, float fValGE, float fLT ) {
	return fComparand >= 0 ? fValGE : fLT;
}
ALWAYS_INLINE
static double fsel( double fComparand, double fValGE, double fLT ) {
	return fComparand >= 0 ? fValGE : fLT;
}


//-----------------------------------------------------------------------------
// FP exception handling
//-----------------------------------------------------------------------------
//#define CHECK_FLOAT_EXCEPTIONS		1

#if defined( COMPILER_MSVC )
	#if IsWindows() && IsPC() && IsPlatform64Bits()
		inline void SetupFPUControlWord() { }
	#else
		inline void SetupFPUControlWordForceExceptions() {
			// use local to get and store control word
			uint16 tmpCtrlW;
			__asm {
				fnclex /* clear all current exceptions */
				fnstcw word ptr [tmpCtrlW] /* get current control word */
				and [tmpCtrlW], 0FCC0h /* Keep infinity control + rounding control */
				or [tmpCtrlW], 0230h /* set to 53-bit, mask only inexact, underflow */
				fldcw word ptr [tmpCtrlW] /* put new control word in FPU */
			}
		}

		#if defined( CHECK_FLOAT_EXCEPTIONS )
			inline void SetupFPUControlWord() {
				SetupFPUControlWordForceExceptions();
			}
		#else
			inline void SetupFPUControlWord() {
				// use local to get and store control word
				uint16 tmpCtrlW;
				__asm {
					fnstcw word ptr [tmpCtrlW] /* get current control word */
					and [tmpCtrlW], 0FCC0h /* Keep infinity control + rounding control */
					or [tmpCtrlW], 023Fh /* set to 53-bit, mask only inexact, underflow */
					fldcw word ptr [tmpCtrlW] /* put new control word in FPU */
				}
			}
		#endif
	#endif
#else
	inline void SetupFPUControlWord() {
		__volatile unsigned short int __cw;
		__asm __volatile( "fnstcw %0" : "=m"( __cw ) );
		__cw = __cw & 0x0FCC0;// keep infinity control, keep rounding mode
		__cw = __cw | 0x023F; // set 53-bit, no exceptions
		__asm __volatile( "fldcw %0" : : "m"( __cw ) );
	}
#endif// COMPILER_MSVC


//-----------------------------------------------------------------------------
// Purpose: Standard functions for handling endian-ness
//-----------------------------------------------------------------------------

//-------------------------------------
// Basic swaps
//-------------------------------------

template<typename T>
inline T WordSwapC( T w ) {
	uint16 temp;

	temp = ( ( *( (uint16*) &w ) & 0xff00 ) >> 8 );
	temp |= ( ( *( (uint16*) &w ) & 0x00ff ) << 8 );

	return *( (T*) &temp );
}

template<typename T>
inline T DWordSwapC( T dw ) {
	uint32 temp;

	temp = *( (uint32*) &dw ) >> 24;
	temp |= ( ( *( (uint32*) &dw ) & 0x00FF0000 ) >> 8 );
	temp |= ( ( *( (uint32*) &dw ) & 0x0000FF00 ) << 8 );
	temp |= ( ( *( (uint32*) &dw ) & 0x000000FF ) << 24 );

	return *( (T*) &temp );
}

template<typename T>
inline T QWordSwapC( T dw ) {
	// Assert sizes passed to this are already correct, otherwise
	// the cast to uint64 * below is unsafe and may have wrong results
	// or even crash.
	static_assert( sizeof( dw ) == sizeof( uint64 ) );

	uint64 temp;

	temp = *( (uint64*) &dw ) >> 56;
	temp |= ( ( *( (uint64*) &dw ) & 0x00FF000000000000ull ) >> 40 );
	temp |= ( ( *( (uint64*) &dw ) & 0x0000FF0000000000ull ) >> 24 );
	temp |= ( ( *( (uint64*) &dw ) & 0x000000FF00000000ull ) >> 8 );
	temp |= ( ( *( (uint64*) &dw ) & 0x00000000FF000000ull ) << 8 );
	temp |= ( ( *( (uint64*) &dw ) & 0x0000000000FF0000ull ) << 24 );
	temp |= ( ( *( (uint64*) &dw ) & 0x000000000000FF00ull ) << 40 );
	temp |= ( ( *( (uint64*) &dw ) & 0x00000000000000FFull ) << 56 );

	return *( (T*) &temp );
}

//-------------------------------------
// Fast swaps
//-------------------------------------
#if defined( COMPILER_MSVC ) && !IsPlatform64Bits()
	#define WordSwap WordSwapAsm
	#define DWordSwap DWordSwapAsm

	#pragma warning( push )
	#pragma warning( disable : 4035 )// no return value

	template<typename T>
	inline T WordSwapAsm( T w ) {
		__asm {
			  mov ax, w
			  xchg al, ah
		}
	}

	template<typename T>
	inline T DWordSwapAsm( T dw ) {
		__asm {
			  mov eax, dw
			  bswap eax
		}
	}

	#pragma warning( pop )
#else
	#define WordSwap WordSwapC
	#define DWordSwap DWordSwapC
#endif

// No ASM implementation for this yet
#define QWordSwap QWordSwapC

//-------------------------------------
// The typically used methods.
//-------------------------------------

#if defined( __i386__ ) && !defined( VALVE_LITTLE_ENDIAN )
	#define VALVE_LITTLE_ENDIAN 1
#endif

#if defined( _SGI_SOURCE )
	#define VALVE_BIG_ENDIAN 1
#endif

// If a swapped float passes through the fpu, the bytes may get changed.
// Prevent this by swapping floats as DWORDs.
#define SafeSwapFloat( pOut, pIn ) ( *( (uint*) pOut ) = DWordSwap( *( (uint*) pIn ) ) )

#if defined( VALVE_LITTLE_ENDIAN )
	#define BigShort( val ) WordSwap( val )
	#define BigWord( val ) WordSwap( val )
	#define BigLong( val ) DWordSwap( val )
	#define BigDWord( val ) DWordSwap( val )
	#define LittleShort( val ) ( val )
	#define LittleWord( val ) ( val )
	#define LittleLong( val ) ( val )
	#define LittleDWord( val ) ( val )
	#define LittleQWord( val ) ( val )
	#define SwapShort( val ) BigShort( val )
	#define SwapWord( val ) BigWord( val )
	#define SwapLong( val ) BigLong( val )
	#define SwapDWord( val ) BigDWord( val )

	// Pass floats by pointer for swapping to avoid truncation in the fpu
	#define BigFloat( pOut, pIn ) SafeSwapFloat( pOut, pIn )
	#define LittleFloat( pOut, pIn ) ( *pOut = *pIn )
	#define SwapFloat( pOut, pIn ) BigFloat( pOut, pIn )
#elif defined( VALVE_BIG_ENDIAN )
	#define BigShort( val ) ( val )
	#define BigWord( val ) ( val )
	#define BigLong( val ) ( val )
	#define BigDWord( val ) ( val )
	#define LittleShort( val ) WordSwap( val )
	#define LittleWord( val ) WordSwap( val )
	#define LittleLong( val ) DWordSwap( val )
	#define LittleDWord( val ) DWordSwap( val )
	#define LittleQWord( val ) QWordSwap( val )
	#define SwapShort( val ) LittleShort( val )
	#define SwapWord( val ) LittleWord( val )
	#define SwapLong( val ) LittleLong( val )
	#define SwapDWord( val ) LittleDWord( val )

	// Pass floats by pointer for swapping to avoid truncation in the fpu
	#define BigFloat( pOut, pIn ) ( *pOut = *pIn )
	#define LittleFloat( pOut, pIn ) SafeSwapFloat( pOut, pIn )
	#define SwapFloat( pOut, pIn ) LittleFloat( pOut, pIn )
#else
	// @Note (toml 05-02-02): this technique expects the compiler to
	//     optimize the expression and eliminate the other path.
	//     On any new platform/compiler this should be tested.
	inline short BigShort( short val ) {
		int test = 1;
		return ( *(char*) &test == 1 ) ? WordSwap( val ) : val;
	}
	inline uint16 BigWord( uint16 val ) {
		int test = 1;
		return ( *(char*) &test == 1 ) ? WordSwap( val ) : val;
	}
	inline long BigLong( long val ) {
		int test = 1;
		return ( *(char*) &test == 1 ) ? DWordSwap( val ) : val;
	}
	inline uint32 BigDWord( uint32 val ) {
		int test = 1;
		return ( *(char*) &test == 1 ) ? DWordSwap( val ) : val;
	}
	inline short LittleShort( short val ) {
		int test = 1;
		return ( *(char*) &test == 1 ) ? val : WordSwap( val );
	}
	inline uint16 LittleWord( uint16 val ) {
		int test = 1;
		return ( *(char*) &test == 1 ) ? val : WordSwap( val );
	}
	inline long LittleLong( long val ) {
		int test = 1;
		return ( *(char*) &test == 1 ) ? val : DWordSwap( val );
	}
	inline uint32 LittleDWord( uint32 val ) {
		int test = 1;
		return ( *(char*) &test == 1 ) ? val : DWordSwap( val );
	}
	inline uint64 LittleQWord( uint64 val ) {
		int test = 1;
		return ( *(char*) &test == 1 ) ? val : QWordSwap( val );
	}
	inline short SwapShort( short val ) { return WordSwap( val ); }
	inline uint16 SwapWord( uint16 val ) { return WordSwap( val ); }
	inline long SwapLong( long val ) { return DWordSwap( val ); }
	inline uint32 SwapDWord( uint32 val ) { return DWordSwap( val ); }

	// Pass floats by pointer for swapping to avoid truncation in the fpu
	inline void BigFloat( float* pOut, const float* pIn ) {
		int test = 1;
		( *(char*) &test == 1 ) ? SafeSwapFloat( pOut, pIn ) : ( *pOut = *pIn );
	}
	inline void LittleFloat( float* pOut, const float* pIn ) {
		int test = 1;
		( *(char*) &test == 1 ) ? ( *pOut = *pIn ) : SafeSwapFloat( pOut, pIn );
	}
	inline void SwapFloat( float* pOut, const float* pIn ) { SafeSwapFloat( pOut, pIn ); }
#endif

ALWAYS_INLINE unsigned long LoadLittleDWord( const unsigned long* base, unsigned int dwordIndex ) {
	return LittleDWord( base[ dwordIndex ] );
}

ALWAYS_INLINE void StoreLittleDWord( unsigned long* base, unsigned int dwordIndex, unsigned long dword ) {
	base[ dwordIndex ] = LittleDWord( dword );
}


//-----------------------------------------------------------------------------
// DLL export for platform utilities
//-----------------------------------------------------------------------------
#if !defined( STATIC_TIER0 )
	#if defined( TIER0_DLL_EXPORT )
		#define PLATFORM_INTERFACE DLL_EXPORT
		#define PLATFORM_OVERLOAD DLL_GLOBAL_EXPORT
		#define PLATFORM_CLASS DLL_CLASS_EXPORT
	#else
		#define PLATFORM_INTERFACE DLL_IMPORT
		#define PLATFORM_OVERLOAD DLL_GLOBAL_IMPORT
		#define PLATFORM_CLASS DLL_CLASS_IMPORT
	#endif
#else
	#define PLATFORM_INTERFACE extern
	#define PLATFORM_OVERLOAD
	#define PLATFORM_CLASS
#endif


// When in benchmark mode, the timer returns a simple incremented value each time you call it.
//
// It should not be changed after startup unless you really know what you're doing. The only place
// that should do this is the benchmark code itself so it can output a legit duration.
PLATFORM_INTERFACE void Plat_SetBenchmarkMode( bool bBenchmarkMode );
PLATFORM_INTERFACE bool Plat_IsInBenchmarkMode();


PLATFORM_INTERFACE double Plat_FloatTime();   // Returns time in seconds since the module was loaded.
PLATFORM_INTERFACE unsigned int Plat_MSTime();// Time in milliseconds.
PLATFORM_INTERFACE char* Plat_ctime( const time_t* timep, char* buf, size_t bufsize );
PLATFORM_INTERFACE struct tm* Plat_gmtime( const time_t* timep, struct tm* result );
PLATFORM_INTERFACE time_t Plat_timegm( struct tm* timeptr );
PLATFORM_INTERFACE struct tm* Plat_localtime( const time_t* timep, struct tm* result );

#if IsWindows() && defined( COMPILER_MSVC ) && ( _MSC_VER >= 1400 )
	extern "C" unsigned __int64 __rdtsc();
	#pragma intrinsic( __rdtsc )
#endif

inline uint64 Plat_Rdtsc() {
	#if IsWindows() && IsPlatform64Bits()
		return (uint64) __rdtsc();
	#elif IsWindows()
		#if defined( COMPILER_MSVC ) && ( _MSC_VER >= 1400 )
			return static_cast<uint64>( __rdtsc() );
		#else
			__asm rdtsc;
			__asm ret;
		#endif
	#elif defined( __i386__ )
		uint64 val;
		__asm__ __volatile__( "rdtsc" : "=A"( val ) );
		return val;
	#elif defined( __x86_64__ )
		uint32 lo, hi;
		__asm__ __volatile__( "rdtsc" : "=a"( lo ), "=d"( hi ) );
		return ( ( static_cast<uint64>( hi ) ) << 32 ) | lo;
	#else
		#error "Plat_Rdtsc: Unsupported platorm, please implement"
	#endif
}

// b/w compatibility
#define Sys_FloatTime Plat_FloatTime

// Protect against bad auto operator=
#define DISALLOW_OPERATOR_EQUAL( _classname )   \
private:                                        \
	_classname& operator=( const _classname& ); \
                                                \
public:

// Define a reasonable operator=
#define IMPLEMENT_OPERATOR_EQUAL( _classname )       \
public:                                              \
	_classname& operator=( const _classname& src ) { \
		memcpy( this, &src, sizeof( _classname ) );  \
		return *this;                                \
	}

// Processor Information:
struct CPUInformation {
	int m_Size;        // Size of this structure, for forward compatability.

	bool m_bRDTSC : 1, // Is RDTSC supported?
		m_bCMOV   : 1, // Is CMOV supported?
		m_bFCMOV  : 1, // Is FCMOV supported?
		m_bSSE    : 1, // Is SSE supported?
		m_bSSE2   : 1, // Is SSE2 Supported?
		m_b3DNow  : 1, // Is 3DNow! Supported?
		m_bMMX    : 1, // Is MMX supported?
		m_bHT     : 1; // Is HyperThreading supported?

	uint8 m_nLogicalProcessors; // Number op logical processors.
	uint8 m_nPhysicalProcessors;// Number of physical processors

	bool m_bSSE3 : 1,
		m_bSSSE3 : 1,
		m_bSSE4a : 1,
		m_bSSE41 : 1,
		m_bSSE42 : 1;

	int64 m_Speed;    // In cycles per second.

	tchar* m_szProcessorID;// Processor vendor Identification.

	uint32 m_nModel;
	uint32 m_nFeatures[ 3 ];
};

// Have to return a pointer, not a reference, because references are not compatible with the
// extern "C" implied by PLATFORM_INTERFACE.
PLATFORM_INTERFACE const CPUInformation* GetCPUInformation();

PLATFORM_INTERFACE float GetCPUUsage();

PLATFORM_INTERFACE void GetCurrentDate( int* pDay, int* pMonth, int* pYear );

// ---------------------------------------------------------------------------------- //
// Performance Monitoring Events - L2 stats etc...
// ---------------------------------------------------------------------------------- //
PLATFORM_INTERFACE void InitPME();
PLATFORM_INTERFACE void ShutdownPME();

//-----------------------------------------------------------------------------
// Thread related functions
//-----------------------------------------------------------------------------

// Sets a hardware data breakpoint on the given address. Currently Win32-only.
// Specify 1, 2, or 4 bytes for nWatchBytes; pass 0 to unregister the address.
PLATFORM_INTERFACE void Plat_SetHardwareDataBreakpoint( const void* pAddress, int nWatchBytes, bool bBreakOnRead );

// Apply current hardware data breakpoints to a newly created thread.
PLATFORM_INTERFACE void Plat_ApplyHardwareDataBreakpointsToNewThread( unsigned long dwThreadID );

//-----------------------------------------------------------------------------
// Process related functions
//-----------------------------------------------------------------------------
PLATFORM_INTERFACE const tchar* Plat_GetCommandLine();
#if !IsWindows()
	// helper function for OS's that don't have a ::GetCommandLine() call
	PLATFORM_INTERFACE void Plat_SetCommandLine( const char* cmdLine );
#endif
PLATFORM_INTERFACE const char* Plat_GetCommandLineA();

//-----------------------------------------------------------------------------
// Security related functions
//-----------------------------------------------------------------------------
// Ensure that the hardware key's drivers have been installed.
PLATFORM_INTERFACE bool Plat_VerifyHardwareKeyDriver();

// Ok, so this isn't a very secure way to verify the hardware key for now.  It
// is primarially depending on the fact that all the binaries have been wrapped
// with the secure wrapper provided by the hardware keys vendor.
PLATFORM_INTERFACE bool Plat_VerifyHardwareKey();

// The same as above, but notifies user with a message box when the key isn't in
// and gives him an opportunity to correct the situation.
PLATFORM_INTERFACE bool Plat_VerifyHardwareKeyPrompt();

// Can be called in real time, doesn't perform the verify every frame.  Mainly just
// here to allow the game to drop out quickly when the key is removed, rather than
// allowing the wrapper to pop up it's own blocking dialog, which the engine doesn't
// like much.
PLATFORM_INTERFACE bool Plat_FastVerifyHardwareKey();

//-----------------------------------------------------------------------------
// Just logs file and line to simple.log
//-----------------------------------------------------------------------------
PLATFORM_INTERFACE void* Plat_SimpleLog( const tchar* file, int line );

//-----------------------------------------------------------------------------
// Returns true if debugger attached, false otherwise
//-----------------------------------------------------------------------------
#if IsWindows() || IsLinux()
	PLATFORM_INTERFACE bool Plat_IsInDebugSession();
	PLATFORM_INTERFACE void Plat_DebugString( const char* );
#else
	inline bool Plat_IsInDebugSession( bool bForceRecheck = false ) { return false; }
	#define Plat_DebugString( s ) ( (void) 0 )
#endif

//-----------------------------------------------------------------------------
// Returns true if running on a 64 bit (windows) OS
//-----------------------------------------------------------------------------
PLATFORM_INTERFACE bool Is64BitOS();

//-----------------------------------------------------------------------------
// Methods to invoke the constructor, copy constructor, and destructor
//-----------------------------------------------------------------------------

template<class T>
inline T* Construct( T* pMemory ) {
	return ::new ( pMemory ) T;
}

template<class T, typename ARG1>
inline T* Construct( T* pMemory, ARG1 a1 ) {
	return ::new ( pMemory ) T( a1 );
}

template<class T, typename ARG1, typename ARG2>
inline T* Construct( T* pMemory, ARG1 a1, ARG2 a2 ) {
	return ::new ( pMemory ) T( a1, a2 );
}

template<class T, typename ARG1, typename ARG2, typename ARG3>
inline T* Construct( T* pMemory, ARG1 a1, ARG2 a2, ARG3 a3 ) {
	return ::new ( pMemory ) T( a1, a2, a3 );
}

template<class T, typename ARG1, typename ARG2, typename ARG3, typename ARG4>
inline T* Construct( T* pMemory, ARG1 a1, ARG2 a2, ARG3 a3, ARG4 a4 ) {
	return ::new ( pMemory ) T( a1, a2, a3, a4 );
}

template<class T, typename ARG1, typename ARG2, typename ARG3, typename ARG4, typename ARG5>
inline T* Construct( T* pMemory, ARG1 a1, ARG2 a2, ARG3 a3, ARG4 a4, ARG5 a5 ) {
	return ::new ( pMemory ) T( a1, a2, a3, a4, a5 );
}

template<class T, class P>
inline void ConstructOneArg( T* pMemory, P const& arg ) {
	::new ( pMemory ) T( arg );
}

template<class T, class P1, class P2>
inline void ConstructTwoArg( T* pMemory, P1 const& arg1, P2 const& arg2 ) {
	::new ( pMemory ) T( arg1, arg2 );
}

template<class T, class P1, class P2, class P3>
inline void ConstructThreeArg( T* pMemory, P1 const& arg1, P2 const& arg2, P3 const& arg3 ) {
	::new ( pMemory ) T( arg1, arg2, arg3 );
}

template<class T>
inline T* CopyConstruct( T* pMemory, T const& src ) {
	return ::new ( pMemory ) T( src );
}

template<class T>
inline void Destruct( T* pMemory ) {
	pMemory->~T();

	#if IsDebug()
		memset( reinterpret_cast<void*>( pMemory ), 0xDD, sizeof( T ) );
	#endif
}


//
// GET_OUTER()
//
// A platform-independent way for a contained class to get a pointer to its
// owner. If you know a class is exclusively used in the context of some
// "outer" class, this is a much more space efficient way to get at the outer
// class than having the inner class store a pointer to it.
//
//	class COuter {
//		class CInner { // Note: this does not need to be a nested class to work
//			void PrintAddressOfOuter() {
//				printf( "Outer is at 0x%x\n", GET_OUTER( COuter, m_Inner ) );
//			}
//		};
//
//		CInner m_Inner;
//		friend class CInner;
//	};

#define GET_OUTER( OuterType, OuterMember ) \
	( (OuterType*) ( (uint8*) this - offsetof( OuterType, OuterMember ) ) )


/*	TEMPLATE_FUNCTION_TABLE()

    (Note added to platform.h so platforms that correctly support templated
	 functions can handle portions as templated functions rather than wrapped
	 functions)

	Helps automate the process of creating an array of function
	templates that are all specialized by a single integer.
	This sort of thing is often useful in optimization work.

	For example, using TEMPLATE_FUNCTION_TABLE, this:

	TEMPLATE_FUNCTION_TABLE(int, Function, ( int blah, int blah ), 10) {
		return argument * argument;
	}

	is equivilent to the following:

	(NOTE: the function has to be wrapped in a class due to code
	generation bugs involved with directly specializing a function
	based on a constant.)

	template<int argument>
	class FunctionWrapper {
	public:
		int Function( int blah, int blah ) {
			return argument*argument;
		}
	}

	typedef int (*FunctionType)( int blah, int blah );

	class FunctionName {
	public:
		enum { count = 10 };
		FunctionType functions[10];
	};

	FunctionType FunctionName::functions[] = {
		FunctionWrapper<0>::Function,
		FunctionWrapper<1>::Function,
		FunctionWrapper<2>::Function,
		FunctionWrapper<3>::Function,
		FunctionWrapper<4>::Function,
		FunctionWrapper<5>::Function,
		FunctionWrapper<6>::Function,
		FunctionWrapper<7>::Function,
		FunctionWrapper<8>::Function,
		FunctionWrapper<9>::Function
	};
*/

PLATFORM_INTERFACE bool vtune( bool resume );


#define TEMPLATE_FUNCTION_TABLE( RETURN_TYPE, NAME, ARGS, COUNT )          \
	typedef RETURN_TYPE( FASTCALL* __Type_##NAME ) ARGS;                   \
                                                                           \
	template<const int nArgument>                                          \
	struct __Function_##NAME {                                             \
		static RETURN_TYPE FASTCALL Run ARGS;                              \
	};                                                                     \
                                                                           \
	template<const int i>                                                  \
	struct __MetaLooper_##NAME : __MetaLooper_##NAME<i - 1> {              \
		__Type_##NAME func;                                                \
		inline __MetaLooper_##NAME() { func = __Function_##NAME<i>::Run; } \
	};                                                                     \
                                                                           \
	template<>                                                             \
	struct __MetaLooper_##NAME<0> {                                        \
		__Type_##NAME func;                                                \
		inline __MetaLooper_##NAME() { func = __Function_##NAME<0>::Run; } \
	};                                                                     \
                                                                           \
	class NAME {                                                           \
	private:                                                               \
		static const __MetaLooper_##NAME<COUNT> m;                         \
                                                                           \
	public:                                                                \
		enum { count = COUNT };                                            \
		static const __Type_##NAME* functions;                             \
	};                                                                     \
	const __MetaLooper_##NAME<COUNT> NAME::m;                              \
	const __Type_##NAME* NAME::functions = (__Type_##NAME*) &m;            \
	template<const int nArgument>                                          \
	RETURN_TYPE FASTCALL __Function_##NAME<nArgument>::Run ARGS


#define LOOP_INTERCHANGE( BOOLEAN, CODE ) \
	if ( ( BOOLEAN ) ) {                  \
		CODE;                             \
	} else {                              \
		CODE;                             \
	}

//-----------------------------------------------------------------------------
// Dynamic libs support
//-----------------------------------------------------------------------------
#if 0// IsWindows() && IsPC()
	PLATFORM_INTERFACE void* Plat_GetProcAddress( const char* pszModule, const char* pszName );

	template <typename FUNCPTR_TYPE>
	class CDynamicFunction {
	public:
		CDynamicFunction( const char* pszModule, const char* pszName, FUNCPTR_TYPE pfnFallback = NULL ) {
			m_pfn = pfnFallback;
			void* pAddr = Plat_GetProcAddress( pszModule, pszName );
			if ( pAddr ) {
				m_pfn = (FUNCPTR_TYPE)pAddr;
			}
		}

		operator bool()			{ return m_pfn != NULL;	}
		bool operator !()		{ return !m_pfn;	}
		operator FUNCPTR_TYPE()	{ return m_pfn; }

	private:
		FUNCPTR_TYPE m_pfn;
	};
#endif


// Watchdog timer support. Call Plat_BeginWatchdogTimer( nn ) to kick the timer off.
// If you don't call Plat_EndWatchdogTimer within nn seconds, the program will kick off an exception.
// This is for making sure that hung dedicated servers abort (and restart) instead of staying hung.
// Calling Plat_EndWatchdogTimer more than once or when there is no active watchdog is fine.
// Only does anything under linux right now.
// It should be possible to implement this functionality in windows via a thread, if desired.
PLATFORM_INTERFACE void Plat_BeginWatchdogTimer( int nSecs );
PLATFORM_INTERFACE void Plat_EndWatchdogTimer();
PLATFORM_INTERFACE int Plat_GetWatchdogTime();

using Plat_WatchDogHandlerFunction_t  = void(*)();
PLATFORM_INTERFACE void Plat_SetWatchdogHandlerFunction( Plat_WatchDogHandlerFunction_t function );


//-----------------------------------------------------------------------------

#include "tier0/valve_on.h"

#if defined( TIER0_DLL_EXPORT )
	extern "C" int V_tier0_stricmp( const char* s1, const char* s2 );
	#undef stricmp
	#undef strcmpi
	#define stricmp( s1, s2 ) V_tier0_stricmp( s1, s2 )
	#define strcmpi( s1, s2 ) V_tier0_stricmp( s1, s2 )
#endif
