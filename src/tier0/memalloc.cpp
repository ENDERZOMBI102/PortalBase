#include "memalloc.hpp"
#include "dbg.h"

#if IsWindows()
	IMemAlloc *g_pMemAlloc = new CMemAlloc();

	// Release versions
	void *CMemAlloc::Alloc( size_t nSize ) { AssertUnreachable(); return {}; }
	void *CMemAlloc::Realloc( void *pMem, size_t nSize ) { AssertUnreachable(); return {}; }
	void CMemAlloc::Free( void *pMem ) { AssertUnreachable(); }
	void *CMemAlloc::Expand_NoLongerSupported( void *pMem, size_t nSize ) { AssertUnreachable(); return {}; }

	// Debug versions
	void *CMemAlloc::Alloc( size_t nSize, const char *pFileName, int nLine ) { AssertUnreachable(); return {}; }
	void *CMemAlloc::Realloc( void *pMem, size_t nSize, const char *pFileName, int nLine ) { AssertUnreachable(); return {}; }
	void  CMemAlloc::Free( void *pMem, const char *pFileName, int nLine ) { AssertUnreachable(); }
	void *CMemAlloc::Expand_NoLongerSupported( void *pMem, size_t nSize, const char *pFileName, int nLine ) { AssertUnreachable(); return {}; }

	// Returns size of a particular allocation
	size_t CMemAlloc::GetSize( void *pMem ) { AssertUnreachable(); return {}; }

	// Force file + line information for an allocation
	void CMemAlloc::PushAllocDbgInfo( const char *pFileName, int nLine ) { AssertUnreachable(); }
	void CMemAlloc::PopAllocDbgInfo() { AssertUnreachable(); }

	// FIXME: Remove when we have our own allocator
	// these methods of the Crt debug code is used in our codebase currently
	long CMemAlloc::CrtSetBreakAlloc( long lNewBreakAlloc ) { AssertUnreachable(); return {}; }
	int CMemAlloc::CrtSetReportMode( int nReportType, int nReportMode ) { AssertUnreachable(); return {}; }
	int CMemAlloc::CrtIsValidHeapPointer( const void *pMem ) { AssertUnreachable(); return {}; }
	int CMemAlloc::CrtIsValidPointer( const void *pMem, unsigned int size, int access ) { AssertUnreachable(); return {}; }
	int CMemAlloc::CrtCheckMemory( void ) { AssertUnreachable(); return {}; }
	int CMemAlloc::CrtSetDbgFlag( int nNewFlag ) { AssertUnreachable(); return {}; }
	void CMemAlloc::CrtMemCheckpoint( _CrtMemState *pState ) { AssertUnreachable(); }

	// FIXME: Make a better stats interface
	void CMemAlloc::DumpStats() { AssertUnreachable(); }
	void CMemAlloc::DumpStatsFileBase( char const *pchFileBase ) { AssertUnreachable(); }

	// FIXME: Remove when we have our own allocator
	void* CMemAlloc::CrtSetReportFile( int nRptType, void* hFile ) { AssertUnreachable(); return {}; }
	void* CMemAlloc::CrtSetReportHook( void* pfnNewHook ) { AssertUnreachable(); return {}; }
	int CMemAlloc::CrtDbgReport( int nRptType, const char * szFile,
					  int nLine, const char * szModule, const char * pMsg ) { AssertUnreachable(); return {}; }

	int CMemAlloc::heapchk() { AssertUnreachable(); return {}; }

	bool CMemAlloc::IsDebugHeap() { AssertUnreachable(); return {}; }

	void CMemAlloc::GetActualDbgInfo( const char *&pFileName, int &nLine ) { AssertUnreachable(); }
	void CMemAlloc::RegisterAllocation( const char *pFileName, int nLine, int nLogicalSize, int nActualSize, unsigned nTime ) { AssertUnreachable(); }
	void CMemAlloc::RegisterDeallocation( const char *pFileName, int nLine, int nLogicalSize, int nActualSize, unsigned nTime ) { AssertUnreachable(); }

	int CMemAlloc::GetVersion() { AssertUnreachable(); return {}; }

	void CMemAlloc::CompactHeap() { AssertUnreachable(); }

	// Function called when malloc fails or memory limits hit to attempt to free up memory (can come in any thread)
	MemAllocFailHandler_t CMemAlloc::SetAllocFailHandler( MemAllocFailHandler_t pfnMemAllocFailHandler ) { AssertUnreachable(); return {}; }

	void CMemAlloc::DumpBlockStats( void * ) { AssertUnreachable(); }

	#if defined( _MEMTEST )
	void CMemAlloc::SetStatsExtraInfo( const char *pMapName, const char *pComment ) { AssertUnreachable(); }
	#endif

	// Returns 0 if no failure, otherwise the size_t of the last requested chunk
	//  "I'm sure this is completely thread safe!" Brian Deen 7/19/2012.
	size_t CMemAlloc::MemoryAllocFailed() { AssertUnreachable(); return {}; }

	// handles storing allocation info for coroutines
	uint32 CMemAlloc::GetDebugInfoSize() { AssertUnreachable(); return {}; }
	void CMemAlloc::SaveDebugInfo( void *pvDebugInfo ) { AssertUnreachable(); }
	void CMemAlloc::RestoreDebugInfo( const void *pvDebugInfo ) { AssertUnreachable(); }
	void CMemAlloc::InitDebugInfo( void *pvDebugInfo, const char *pchRootFileName, int nLine ) { AssertUnreachable(); }

	// Replacement for ::GlobalMemoryStatus which accounts for unused memory in our system
	void CMemAlloc::GlobalMemoryStatus( size_t *pUsedMemory, size_t *pFreeMemory ) { AssertUnreachable(); }
#endif

#if IsPosix()
	size_t ApproximateProcessMemoryUsage() {
		uint32 usage{};

		const auto file{ std::fopen( "/proc/self/stat", "r" ) };
		std::fscanf( file, "%*d (%*[^)]%*[)] %*c %*d %*d %*d %*d %*d %*u %*lu %*lu %*lu %*lu %*lu %*lu %*ld %*ld %*ld %*ld %*ld %*ld %*llu %lu", &usage );
		std::fclose( file );

		return usage;
	}
#endif
