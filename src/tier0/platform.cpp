//
// Created by ENDERZOMBI102 on 10/02/2024.
//
#include "tier0/platform.h"
#include "tier0/dbg.h"
#include <SDL3/SDL_cpuinfo.h>
#include <cstdio>
#include <ctime>
#include <fstream>

#if IsWindows()
	#include <sysinfoapi.h>
	#include <libloaderapi.h>
	#include <profileapi.h>
	#include <processenv.h>
    #include <intrin.h>
    #include <wow64apiset.h>
    #include <processthreadsapi.h>
#elif IsPosix()
	#include <cpuid.h>
	#include <sys/utsname.h>
#endif

double MonotonicTime() {
	#if IsWindows()
		static LARGE_INTEGER freq{}; // cache the value
		if ( freq.QuadPart == 0 ) {
			if (! QueryPerformanceFrequency( &freq ) )
				freq.QuadPart = 0;
		}

		LARGE_INTEGER time;
		// do we have a valid frequency? if so, use the performance counter
		if ( freq.QuadPart > 0 && QueryPerformanceCounter( &time ) ) {
			return static_cast<double>( time.QuadPart ) * 1000 / freq.QuadPart;
		}

		// last resort
		return static_cast<double>( GetTickCount64() ) / 1000;
	#elif IsLinux()
		std::timespec ts{};
		std::timespec_get( &ts, CLOCK_MONOTONIC ); // `CLOCK_MONOTONIC` is a linux extension to C's time library
		return static_cast<double>( ts.tv_sec ) + static_cast<double>( ts.tv_nsec ) / 1e9;
	#endif
}

static bool g_bBenchmarkMode{ false };
static struct ModuleData {
	double m_LoadTime;

	ModuleData() : m_LoadTime( MonotonicTime() ) { };
} g_ModuleData{};


void Plat_SetBenchmarkMode( bool bBenchmarkMode ) {
	g_bBenchmarkMode = bBenchmarkMode;
}
bool Plat_IsInBenchmarkMode() {
	return g_bBenchmarkMode;
}


double Plat_FloatTime() {
	return static_cast<double>( MonotonicTime() - g_ModuleData.m_LoadTime );
}
unsigned int Plat_MSTime() {
	return static_cast<unsigned int>( ( MonotonicTime() - g_ModuleData.m_LoadTime ) * 1000 );
}
char* Plat_ctime( const time_t* timep, char* buf, size_t bufsize ) {
	#if IsWindows()
		ctime_s( buf, bufsize, timep );
		return buf;
	#elif IsLinux()
		return ctime_r( timep, buf );  // TODO: Verify this doesn't exceed the buffer
	#endif
}
struct tm* Plat_gmtime( const time_t* timep, struct tm* result ) {
	#if IsWindows()
		gmtime_s( result, timep );
		return result;
	#elif IsLinux()
		return gmtime_r( timep, result );
	#endif
}
time_t Plat_timegm( struct tm* timeptr ) {
	#if IsWindows()
		return _mkgmtime( timeptr );
	#elif IsLinux()
		return timegm( timeptr );
	#endif
}
struct tm* Plat_localtime( const time_t* timep, struct tm* result ) {
	#if IsWindows()
		localtime_s( result, timep );
		return result;
	#elif IsLinux()
		return localtime_r( timep, result );
	#endif
}

const CPUInformation* GetCPUInformation() {
	static char vendor[13] { 0 };
	// NOTE: All x86 processors nowadays support the following: SSE, SSE2, HT, SSSE3
	static CPUInformation info{
		.m_Size   = sizeof( CPUInformation ),
		.m_bRDTSC = false,
		.m_bCMOV  = true,
		.m_bFCMOV = false,
		.m_bSSE   = IsPC() || SDL_HasSSE(),
		.m_bSSE2  = IsPC() || SDL_HasSSE2(),
		.m_b3DNow = false,
		.m_bMMX   = SDL_HasMMX() != 0,
		.m_bHT    = true,

		.m_nLogicalProcessors = static_cast<uint8>( SDL_GetCPUCount() ),
		.m_nPhysicalProcessors = static_cast<uint8>( SDL_GetCPUCount() ),

		.m_bSSE3  = SDL_HasSSE3() != 0,
		.m_bSSSE3 = IsPC(),
		.m_bSSE4a = false,
		.m_bSSE41 = SDL_HasSSE41() != 0,
		.m_bSSE42 = SDL_HasSSE42() != 0,

		.m_Speed = 3'000'000'000,

		.m_szProcessorID = vendor,

		.m_nModel = 0,
		.m_nFeatures = { 0, 0, 0 },
	};

	if (! info.m_szProcessorID[0] ) {
		int regs[4] { 0 };
		// get vendor
		#if IsPosix()
			__cpuid( 0, regs[0], regs[1], regs[2], regs[3] );
		#elif IsWindows()
			__cpuid( regs, 0 );
		#endif
		memcpy( vendor + 0, &regs[1], 4 );
		memcpy( vendor + 4, &regs[3], 4 );
		memcpy( vendor + 8, &regs[2], 4 );
		// TODO?: get clock speed ( linux: https://gist.github.com/stevedoyle/1319053, win: ??? )
	}

	return &info;
}

float GetCPUUsage();

void GetCurrentDate( int* pDay, int* pMonth, int* pYear ) {
	auto lTime{ time( nullptr ) };
	auto lDate{ localtime( &lTime ) };

	*pDay =lDate->tm_mday;
	*pMonth = lDate->tm_mon;
	*pYear = lDate->tm_year;
}

void InitPME();
void ShutdownPME();

void Plat_SetHardwareDataBreakpoint( const void* pAddress, int nWatchBytes, bool bBreakOnRead );
void Plat_ApplyHardwareDataBreakpointsToNewThread( unsigned long dwThreadID );

const tchar* Plat_GetCommandLine() {
	#if IsWindows()
		return GetCommandLine();
	#elif IsPosix()
		static tchar cmdline[2048] { 0 };
		if ( cmdline[0] != '\0' ) {
			return cmdline;
		}

		const auto file{ std::fopen( "/proc/self/cmdline", "r" ) };
		std::fread( cmdline, 1, sizeof( cmdline ) - 1, file );
		std::fclose( file );

		return cmdline;
	#else
		#error "Plat_GetCommandLine: Missing implementation"
	#endif
}
#if !IsWindows()
	void Plat_SetCommandLine( const char* ) {
		AssertUnreachable();
	}
#endif
const char* Plat_GetCommandLineA() {
	#if IsWindows()
		return GetCommandLineA();
	#elif IsPosix()
		return Plat_GetCommandLine();
	#else
		#error "Plat_GetCommandLineA: Missing implementation"
	#endif
}

bool Plat_VerifyHardwareKeyDriver();

// FIXME: Implement this
bool Plat_VerifyHardwareKey() { return true; }

bool Plat_VerifyHardwareKeyPrompt();

bool Plat_FastVerifyHardwareKey();

void* Plat_SimpleLog( const tchar* file, int line );

#if IsLinux()
	PLATFORM_INTERFACE bool Plat_IsInDebugSession() {
		static char buffer[32] { };
		std::ifstream file{ "/proc/self/status" };

		int tracer;
		while ( file.getline( buffer, sizeof( buffer )  - 1 ) ) {
			if ( sscanf( buffer, "TracerPid: %d", &tracer ) == 1 ) { // NOLINT(*-err34-c), using it to pattern-match the input string...
				return tracer != 0;
			}
		}

		return false;
	}
	PLATFORM_INTERFACE void Plat_DebugString( const char* ) { /* do nothing */ }
#elif IsWindows()
	PLATFORM_INTERFACE bool Plat_IsInDebugSession() {
		return IsDebuggerPresent();
	}
	PLATFORM_INTERFACE void Plat_DebugString( const char* pString ) {
		OutputDebugStringW( pString );
	}
#endif

bool Is64BitOS() {
	#if IsPlatform64Bits()
		return true;
    #elif IsWindows()
        using LPFN_ISWOW64PROCESS = BOOL (WINAPI *) (HANDLE, PBOOL);
		auto isWow64{ false };
		static auto fnIsWow64Process{ reinterpret_cast<LPFN_ISWOW64PROCESS>( GetProcAddress( GetModuleHandle( "kernel32" ), "IsWow64Process" ) ) };

		return fnIsWow64Process && fnIsWow64Process( GetCurrentProcess(), &isWow64 ) && isWow64;
	#elif IsPosix()
		utsname data{};
		uname( &data );
		// TODO: Check if this is correct for all posix impls
		return strcmp( data.machine, "x86_64" ) == 0;
	#else
		#error "Is64BitOS: Not implemented"
	#endif
}