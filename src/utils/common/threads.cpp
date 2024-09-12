//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//
#if defined( PLATFORM_WINDOWS )
	#include <sysinfoapi.h>
	#include <processthreadsapi.h>
	#include <synchapi.h>
	#include <handleapi.h>
	//#include <WinBase.h>
#elif defined( PLATFORM_POSIX )
	#include <pthread.h>
#else
	#error "threads.h: Don't know how to handle threads here!"
#endif
#include "cmdlib.h"
#define NO_THREAD_NAMES
#include "pacifier.h"
#include "threads.h"

#define MAX_THREADS 16


class CRunThreadsData {
public:
	int m_iThread;
	void* m_pUserData;
	RunThreadsFn m_Fn;
};

CRunThreadsData g_RunThreadsData[ MAX_THREADS ];


int dispatch;
int workcount;
bool pacifier;

bool threaded;
bool g_bLowPriorityThreads = false;

#if IsWindows()
	HANDLE g_ThreadHandles[ MAX_THREADS ];
#elif IsPosix()
	pthread_t g_ThreadHandles[ MAX_THREADS ];
#endif

/*
=============
GetThreadWork
=============
*/
int GetThreadWork() {
	int r;

	ThreadLock();

	if ( dispatch == workcount ) {
		ThreadUnlock();
		return -1;
	}

	UpdatePacifier( static_cast<float>( dispatch ) / static_cast<float>( workcount ) );

	r = dispatch;
	dispatch++;
	ThreadUnlock();

	return r;
}


ThreadWorkerFn workfunction;

void ThreadWorkerFunction( int iThread, void* ) {
	int work;

	while ( true ) {
		work = GetThreadWork();
		if ( work == -1 ) {
			break;
		}

		workfunction( iThread, work );
	}
}

void RunThreadsOnIndividual( int workcnt, bool showpacifier, ThreadWorkerFn func ) {
	if ( numthreads == -1 ) {
		ThreadSetDefault();
	}

	workfunction = func;
	RunThreadsOn( workcnt, showpacifier, ThreadWorkerFunction );
}

int numthreads = -1;
CThreadMutex mutex;
static int enter;

void SetLowPriority() {
	#if IsWindows()
		#define IDLE_PRIORITY_CLASS 0x00000040 // WinBase.h
		SetPriorityClass( GetCurrentProcess(), IDLE_PRIORITY_CLASS );
	#elif IsPosix()
		nice(19);
	#endif
}


void ThreadSetDefault() {
	if ( numthreads == -1 ) { // not set manually
		#if IsWindows()
			SYSTEM_INFO info;
			GetSystemInfo( &info );
			numthreads = info.dwNumberOfProcessors;
		#elif IsLinux()
			numthreads = sysconf( _SC_NPROCESSORS_ONLN );
		#endif
		if ( numthreads < 1 || numthreads > 32 ) {
				numthreads = 1;
		}
	}

	Msg( "%i threads\n", numthreads );
}


void ThreadLock() {
	if (! threaded ) {
		return;
	}
	mutex.Lock();
	if ( enter ) {
		Error( "Recursive ThreadLock\n" );
	}
	enter = 1;
}

void ThreadUnlock() {
	if (! threaded ) {
		return;
	}
	if (! enter ) {
		Error( "ThreadUnlock without lock\n" );
	}
	enter = 0;
	mutex.Unlock();
}

// This runs in the thread and dispatches a RunThreadsFn call.
#if IsWindows()
    DWORD WINAPI InternalRunThreadsFn( LPVOID pParameter ) {
        auto pData = static_cast<CRunThreadsData*>( pParameter );
        pData->m_Fn( pData->m_iThread, pData->m_pUserData );
        return {};
    }
#elif IsPosix()
    void* InternalRunThreadsFn( void* pParameter ) {
        auto pData = static_cast<CRunThreadsData*>( pParameter );
        pData->m_Fn( pData->m_iThread, pData->m_pUserData );
        return nullptr;
    }
#endif


void RunThreads_Start( RunThreadsFn fn, void* pUserData, ERunThreadsPriority ePriority ) {
	Assert( numthreads > 0 );
	threaded = true;

	if ( numthreads > MAX_TOOL_THREADS ) {
		numthreads = MAX_TOOL_THREADS;
	}

	for ( int i = 0; i < numthreads; i++ ) {
		g_RunThreadsData[ i ].m_iThread = i;
		g_RunThreadsData[ i ].m_pUserData = pUserData;
		g_RunThreadsData[ i ].m_Fn = fn;
		#if IsWindows()
			g_ThreadHandles[ i ] = CreateThread(
				nullptr,                 // [inp]  LPSECURITY_ATTRIBUTES lpThreadAttributes,
				0,                       // [inp]                  DWORD dwStackSize,
				InternalRunThreadsFn,    // [inp] LPTHREAD_START_ROUTINE lpStartAddress,
				&g_RunThreadsData[ i ],  // [inp]                 LPVOID lpParameter,
				0,                       // [inp]                  DWORD dwCreationFlags,
				nullptr                  // [out]                LPDWORD lpThreadId
			);

			#define THREAD_PRIORITY_LOWEST THREAD_BASE_PRIORITY_MIN
			#define THREAD_PRIORITY_BELOW_NORMAL ( THREAD_PRIORITY_LOWEST + 1 )
			#define THREAD_PRIORITY_NORMAL 0
			#define THREAD_PRIORITY_HIGHEST THREAD_BASE_PRIORITY_MAX
			#define THREAD_PRIORITY_ABOVE_NORMAL ( THREAD_PRIORITY_HIGHEST - 1 )
			#define THREAD_PRIORITY_ERROR_RETURN ( MAXLONG )

			#define THREAD_PRIORITY_TIME_CRITICAL THREAD_BASE_PRIORITY_LOWRT
			#define THREAD_PRIORITY_IDLE THREAD_BASE_PRIORITY_IDLE

			#define THREAD_MODE_BACKGROUND_BEGIN 0x00010000
			#define THREAD_MODE_BACKGROUND_END 0x00020000
		#elif IsPosix()
			pthread_create( &g_ThreadHandles[ i ], nullptr, InternalRunThreadsFn, &g_RunThreadsData[i] );
			#define SetThreadPriority      pthread_setschedprio
			#define THREAD_PRIORITY_LOWEST 19
			#define THREAD_PRIORITY_IDLE   0
		#endif

		if ( ePriority == k_eRunThreadsPriority_UseGlobalState ) {
			if ( g_bLowPriorityThreads ) {
				SetThreadPriority( g_ThreadHandles[ i ], THREAD_PRIORITY_LOWEST );
			}
		} else if ( ePriority == k_eRunThreadsPriority_Idle ) {
			SetThreadPriority( g_ThreadHandles[ i ], THREAD_PRIORITY_IDLE );
		}
	}
}


void RunThreads_End() {
	#if IsWindows()
		#define INFINITE 0xFFFFFFFF // WinBase.h
		WaitForMultipleObjects( numthreads, g_ThreadHandles, true, INFINITE );
	#endif
	for ( auto handle : g_ThreadHandles ) {
		#if IsWindows()
			CloseHandle( handle );
		#elif IsPosix()
			if ( handle ) {
				pthread_join( handle, nullptr );
				pthread_cancel( handle );
			}
		#endif
	}

	threaded = false;
}


/*
=============
RunThreadsOn
=============
*/
void RunThreadsOn( int workcnt, bool showpacifier, RunThreadsFn fn, void* pUserData ) {
	auto start = static_cast<int>( Plat_FloatTime() );
	dispatch = 0;
	workcount = workcnt;
	StartPacifier( "" );
	pacifier = showpacifier;

	#if defined( _PROFILE )
		threaded = false;
		( *func )( 0 );
		return;
	#endif

	RunThreads_Start( fn, pUserData );
	RunThreads_End();

	auto end = static_cast<int>( Plat_FloatTime() );
	if ( pacifier ) {
		EndPacifier( false );
		printf( " (%i)\n", end - start );
	}
}
