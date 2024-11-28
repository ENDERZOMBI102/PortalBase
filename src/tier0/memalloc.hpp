#pragma once
#include "tier0/memalloc.h"
#include "tier0/platform.h"


#if IsWindows()
	class CMemAlloc : public IMemAlloc {
	public:
		// Release versions
		void* Alloc( size_t nSize ) override;
		void* Realloc( void* pMem, size_t nSize ) override;
		void Free( void* pMem ) override;
		void* Expand_NoLongerSupported( void* pMem, size_t nSize ) override;

		// Debug versions
		void* Alloc( size_t nSize, const char* pFileName, int nLine ) override;
		void* Realloc( void* pMem, size_t nSize, const char* pFileName, int nLine ) override;
		void Free( void* pMem, const char* pFileName, int nLine ) override;
		void* Expand_NoLongerSupported( void* pMem, size_t nSize, const char* pFileName, int nLine ) override;

		// Returns size of a particular allocation
		size_t GetSize( void* pMem ) override;

		// Force file + line information for an allocation
		void PushAllocDbgInfo( const char* pFileName, int nLine ) override;
		void PopAllocDbgInfo() override;

		// FIXME: Remove when we have our own allocator
		// these methods of the Crt debug code is used in our codebase currently
		long CrtSetBreakAlloc( long lNewBreakAlloc ) override;
		int CrtSetReportMode( int nReportType, int nReportMode ) override;
		int CrtIsValidHeapPointer( const void* pMem ) override;
		int CrtIsValidPointer( const void* pMem, unsigned int size, int access ) override;
		int CrtCheckMemory() override;
		int CrtSetDbgFlag( int nNewFlag ) override;
		void CrtMemCheckpoint( _CrtMemState* pState ) override;

		// FIXME: Make a better stats interface
		void DumpStats() override;
		void DumpStatsFileBase( char const* pchFileBase ) override;

		// FIXME: Remove when we have our own allocator
		void* CrtSetReportFile( int nRptType, void* hFile ) override;
		void* CrtSetReportHook( void* pfnNewHook ) override;
		int CrtDbgReport( int nRptType, const char* szFile, int nLine, const char* szModule, const char* pMsg ) override;

		int heapchk() override;

		bool IsDebugHeap() override;

		void GetActualDbgInfo( const char*& pFileName, int& nLine ) override;
		void RegisterAllocation( const char* pFileName, int nLine, int nLogicalSize, int nActualSize, unsigned nTime ) override;
		void RegisterDeallocation( const char* pFileName, int nLine, int nLogicalSize, int nActualSize, unsigned nTime ) override;

		int GetVersion() override;

		void CompactHeap() override;

		// Function called when malloc fails or memory limits hit to attempt to free up memory (can come in any thread)
		MemAllocFailHandler_t SetAllocFailHandler( MemAllocFailHandler_t pfnMemAllocFailHandler ) override;

		void DumpBlockStats( void* ) override;

		#if defined( _MEMTEST )
			void SetStatsExtraInfo( const char* pMapName, const char* pComment ) override;
		#endif

		// Returns 0 if no failure, otherwise the size_t of the last requested chunk
		//  "I'm sure this is completely thread safe!" Brian Deen 7/19/2012.
		size_t MemoryAllocFailed() override;

		// handles storing allocation info for coroutines
		uint32 GetDebugInfoSize() override;
		void SaveDebugInfo( void* pvDebugInfo ) override;
		void RestoreDebugInfo( const void* pvDebugInfo ) override;
		void InitDebugInfo( void* pvDebugInfo, const char* pchRootFileName, int nLine ) override;

		// Replacement for ::GlobalMemoryStatus which accounts for unused memory in our system
		void GlobalMemoryStatus( size_t* pUsedMemory, size_t* pFreeMemory ) override;
	};
#endif


