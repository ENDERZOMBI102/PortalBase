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
#pragma once


// Arrays that are indexed by thread idx should always be MAX_TOOL_THREADS+1
// large, so THREADINDEX_MAIN can be used from the main thread.
#define MAX_TOOL_THREADS 16
#define THREADINDEX_MAIN (MAX_TOOL_THREADS)


extern int numthreads;

// If set to true, then all the threads that are created are low priority.
extern bool g_bLowPriorityThreads;

using ThreadWorkerFn = void (*)( int iThread, int iWorkItem );
using RunThreadsFn = void (*)( int iThread, void* pUserData );


enum ERunThreadsPriority {
	k_eRunThreadsPriority_UseGlobalState = 0,  // Default... uses g_bLowPriorityThreads to decide what to set the priority to.
	k_eRunThreadsPriority_Normal,              // Doesn't touch thread priorities.
	k_eRunThreadsPriority_Idle                 // Sets threads to idle priority.
};


// Put the process into an idle priority class, so it doesn't hog the UI.
void SetLowPriority();

void ThreadSetDefault();
int GetThreadWork();

void RunThreadsOnIndividual( int workcnt, bool showpacifier, ThreadWorkerFn fn );

void RunThreadsOn( int workcnt, bool showpacifier, RunThreadsFn fn, void* pUserData = nullptr );

// This version doesn't track work items - it just runs your function and waits for it to finish.
void RunThreads_Start( RunThreadsFn fn, void* pUserData, ERunThreadsPriority ePriority = k_eRunThreadsPriority_UseGlobalState );
void RunThreads_End();

void ThreadLock();
void ThreadUnlock();


#ifndef NO_THREAD_NAMES
	#define RunThreadsOn( n, p, f ) {            \
			if ( p ) printf( "%-20s ", #f ":" ); \
			RunThreadsOn( n, p, f );             \
		}
	#define RunThreadsOnIndividual( n, p, f ) {  \
			if ( p ) printf( "%-20s ", #f ":" ); \
			RunThreadsOnIndividual( n, p, f );   \
		}
#endif
