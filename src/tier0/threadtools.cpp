//
// Created by ENDERZOMBI102 on 10/02/2024.
//
#include "threadtools.h"
#if IsWindows()
	#include <synchapi.h>
	#include <processthreadsapi.h>
    #include <handleapi.h>
#elif IsPosix()
	#include <sys/time.h>
	#include <csignal>
#endif


static ThreadedLoadLibraryFunc_t g_pThrLoadLibFunc{ nullptr };
static uint g_MainThreadId{ 0 };

// ----- SimpleThread_t -----
//
ThreadHandle_t CreateSimpleThread( ThreadFunc_t pHandle, void* pParam, ThreadId_t* pID, unsigned stackSize ) {
	#if IsWindows()
	    AssertUnreachable();
	    return {};
	#elif IsPosix()
		pthread_t handle;
		pthread_attr_t attrs;
		if (! pthread_attr_init( &attrs ) ) {
			return nullptr;
		}
		if (! pthread_attr_setstacksize( &attrs, std::max( static_cast<long int>( stackSize ), PTHREAD_STACK_MIN ) ) ) {
			pthread_attr_destroy( &attrs );
			return nullptr;
		}
		pthread_create( &handle, &attrs, reinterpret_cast<void*(*)(void*)>( pHandle ), pParam );
		pthread_attr_destroy( &attrs );

		return reinterpret_cast<ThreadHandle_t>( handle );
	#endif
}
ThreadHandle_t CreateSimpleThread( ThreadFunc_t pHandle, void* pParam, unsigned stackSize ) {
	#if IsWindows()
	    AssertUnreachable();
	    return {};
	#elif IsPosix()
		pthread_t handle;
		pthread_attr_t attrs;
		if (! pthread_attr_init( &attrs ) ) {
			return nullptr;
		}
		if (! pthread_attr_setstacksize( &attrs, std::max( static_cast<long int>( stackSize ), PTHREAD_STACK_MIN ) ) ) {
			pthread_attr_destroy( &attrs );
			return nullptr;
		}
		pthread_create( &handle, &attrs, reinterpret_cast<void*(*)(void*)>( pHandle ), pParam );
		pthread_attr_destroy( &attrs );

		return reinterpret_cast<ThreadHandle_t>( handle );
	#endif
}
bool ReleaseThreadHandle( ThreadHandle_t pHandle ) {
	AssertUnreachable();
	return {};
}

void ThreadSleep( unsigned pDurationMs ) {
	#if IsWindows()
		Sleep( pDurationMs );
	#elif IsLinux()
		usleep( pDurationMs );
	#elif IsPosix()
		timespec spec{ .tv_sec = 0, .tv_nsec = static_cast<long>( pDurationMs * 1000 ) };
		nanosleep( &spec, nullptr );
	#else
		#error "ThreadSleep: Missing implementation!"
	#endif
}
uint ThreadGetCurrentId() {
	#if IsWindows()
		return static_cast<uint>( GetCurrentThreadId() );
	#elif IsPosix()
		// NOTE: This might be wrong, as the code might expect a SYSTEM UNIQUE id
		return static_cast<uint>( pthread_self() );
	#else
		#error "ThreadSleep: Missing implementation!"
	#endif
}
ThreadHandle_t ThreadGetCurrentHandle() {
	#if IsWindows()
        return static_cast<ThreadHandle_t>( GetCurrentThread() );
	#elif IsPosix()
		return reinterpret_cast<ThreadHandle_t>( pthread_self() );
	#endif
}
int ThreadGetPriority( ThreadHandle_t hThread ) {
	AssertUnreachable();
	return {};
}
bool ThreadSetPriority( ThreadHandle_t hThread, int priority ) {
	AssertUnreachable();
	return {};
}
bool ThreadInMainThread() {
    return g_MainThreadId == ThreadGetCurrentId();
}
void DeclareCurrentThreadIsMainThread() {
	g_MainThreadId = ThreadGetCurrentId();
}

void SetThreadedLoadLibraryFunc( ThreadedLoadLibraryFunc_t func ) {
	g_pThrLoadLibFunc = func;
}
ThreadedLoadLibraryFunc_t GetThreadedLoadLibraryFunc() {
	return g_pThrLoadLibFunc;
}

bool ThreadJoin( ThreadHandle_t, unsigned timeout ) {
	AssertUnreachable();
	return {};
}
void ThreadDetach( ThreadHandle_t ) {
	AssertUnreachable();
}

void ThreadSetDebugName( ThreadId_t id, const char* pszName ) {
	#if IsWindows()
		SetThreadDescription( id, pszName );
	#elif IsPosix()
		pthread_setname_np( id, pszName );
	#else
		#error
	#endif
}

void ThreadSetAffinity( ThreadHandle_t hThread, int nAffinityMask ) {
	AssertUnreachable();
}

#if IsWindows()
	int ThreadWaitForObjects( int nEvents, const HANDLE* pHandles, bool bWaitAll, unsigned timeout );
#endif

// ----- ThreadInterlocked* -----
//
#if !defined( USE_INTRINSIC_INTERLOCKED )
	long ThreadInterlockedIncrement( long volatile* pIt ) { // NOLINT(*-non-const-parameter)
		// type __atomic_add_fetch(type *ptr, type val, int memorder)
		return __atomic_add_fetch( pIt, 1, __ATOMIC_ACQ_REL );
	}
	long ThreadInterlockedDecrement( long volatile* pIt ) { // NOLINT(*-non-const-parameter)
		// type __atomic_sub_fetch(type *ptr, type val, int memorder)
		return __atomic_sub_fetch( pIt, 1, __ATOMIC_ACQ_REL );
	}
	long ThreadInterlockedExchange( long volatile* pIt, long pValue ) { // NOLINT(*-non-const-parameter)
		// type __atomic_exchange_n(type *ptr, type val, int memorder)
		return __atomic_exchange_n( pIt, pValue, __ATOMIC_ACQ_REL );
	}
	long ThreadInterlockedExchangeAdd( long volatile* pIt, long pValue ) { // NOLINT(*-non-const-parameter)
		// type __atomic_fetch_add(type *ptr, type val, int memorder)
		return __atomic_fetch_add( pIt, pValue, __ATOMIC_ACQ_REL );
	}
	long ThreadInterlockedCompareExchange( long volatile* pIt, long pValue, long comperand ) { // NOLINT(*-non-const-parameter)
		long last;
		long expected = comperand;
		do {
			// void __atomic_load (type *ptr, type *ret, int memorder)
			__atomic_load( pIt, &last, __ATOMIC_RELAXED );
			// bool __atomic_compare_exchange_n(type *ptr, type *expected, type desired, bool weak, int success_memorder, int failure_memorder)
		} while (! __atomic_compare_exchange_n( pIt, &expected, pValue, false, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED ) );
		return last;
	}
	bool ThreadInterlockedAssignIf( long volatile* pIt, long value, long comperand ) { // NOLINT(*-non-const-parameter)
		// bool __atomic_compare_exchange_n(type *ptr, type *expected, type desired, bool weak, int success_memorder, int failure_memorder)
		return __atomic_compare_exchange_n( pIt, &comperand, value, false, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED );
	}
#endif
int64 ThreadInterlockedIncrement64( int64 volatile* pIt ) {// NOLINT(*-non-const-parameter)
    #if IsWindows()
        return _InlineInterlockedIncrement64(pIt);
    #elif IsPosix()
        return __atomic_add_fetch( pIt, 1, __ATOMIC_ACQ_REL );
    #endif
}
int64 ThreadInterlockedDecrement64( int64 volatile* pIt ) {// NOLINT(*-non-const-parameter)
    #if IsWindows()
        return _InlineInterlockedDecrement64(pIt);
    #elif IsPosix()
	    return __atomic_sub_fetch( pIt, 1, __ATOMIC_ACQ_REL );
    #endif
}
int64 ThreadInterlockedCompareExchange64( int64 volatile* pIt, int64 pValue, int64 comperand ) {// NOLINT(*-non-const-parameter)
    #if IsWindows()
        return _InterlockedCompareExchange64(pIt, pValue, comperand);
    #elif IsPosix()
	    int64 last;
        int64 expected = comperand;
        do __atomic_load( pIt, &last, __ATOMIC_RELAXED );
        while (! __atomic_compare_exchange_n( pIt, &expected, pValue, false, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED ) );
        return last;
    #endif
}
int64 ThreadInterlockedExchange64( int64 volatile* pIt, int64 pValue ) {                        // NOLINT(*-non-const-parameter)
    #if IsWindows()
        return _InlineInterlockedExchange64(pIt, pValue);
    #elif IsPosix()
	    return __atomic_exchange_n( pIt, pValue, __ATOMIC_ACQ_REL );
    #endif
}
int64 ThreadInterlockedExchangeAdd64( int64 volatile* pIt, int64 pValue ) {// NOLINT(*-non-const-parameter)
    #if IsWindows()
        return _InlineInterlockedExchangeAdd64(pIt, pValue);
    #elif IsPosix()
	    return __atomic_fetch_add( pIt, pValue, __ATOMIC_ACQ_REL );
    #endif
}
bool ThreadInterlockedAssignIf64( volatile int64* pDest, int64 pValue, int64 comperand ) { // NOLINT(*-non-const-parameter)
    #if IsWindows()
        return _InterlockedCompareExchange64(pDest, pValue, comperand);
    #elif IsPosix()
	    return __atomic_compare_exchange_n( pDest, &comperand, pValue, false, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED );
    #endif
}

// ----- CThreadRWLock -----
//
void CThreadRWLock::LockForWrite() {
	this->m_mutex.Lock();
	if ( this->m_nActiveReaders ) {
		this->m_CanWrite.Wait();
	}
	this->m_nWriters += 1;
	this->m_mutex.Unlock();
}
void CThreadRWLock::UnlockWrite() {
	this->m_mutex.Lock();
	this->m_nWriters -= 1;
	if ( this->m_nWriters == 0 && this->m_nPendingReaders != 0 ) {
		this->m_CanRead.Set();
	}
	this->m_mutex.Unlock();
}
void CThreadRWLock::WaitForRead() {
	this->m_nPendingReaders += 1;
	this->m_CanRead.Wait();
}

// ----- CThreadSpinRWLock -----
//
void CThreadSpinRWLock::LockForRead() {
	this->m_lockInfo.m_nReaders += 1;
}
void CThreadSpinRWLock::SpinLockForWrite( const uint32 pThreadId ) {
	while ( this->m_lockInfo.m_nReaders ) ;
	while (! this->m_nWriters.AssignIf( 0, 1 ) ) ;
	this->m_lockInfo.m_writerId = pThreadId;
}
void CThreadSpinRWLock::UnlockRead() {
	this->m_lockInfo.m_nReaders -= 1;
}
void CThreadSpinRWLock::UnlockWrite() {
	this->m_nWriters.AssignIf( 1, 0 );
}

// ----- CThreadFastMutex -----
//
void CThreadFastMutex::Lock( const uint32 pThreadId, unsigned nSpinSleepTime ) volatile {
	while (! this->TryLockInline( pThreadId ) )
		ThreadSleep( nSpinSleepTime );
}

// ----- CThreadSyncObject -----
//
CThreadSyncObject::CThreadSyncObject() {
    #if IsPosix()
        pthread_mutex_init( &this->m_Mutex, nullptr );
        pthread_cond_init( &this->m_Condition, nullptr );
        this->m_bInitalized = true;
    #endif
}
CThreadSyncObject::~CThreadSyncObject() {
    #if IsWindows()
        if (this->m_hSyncObject != nullptr)
            CloseHandle(this->m_hSyncObject);
    #elif IsPosix()
        if ( this->m_bInitalized ) {
            pthread_cond_destroy( &this->m_Condition );
            pthread_mutex_destroy( &this->m_Mutex );
            this->m_bInitalized = false;
        }
    #endif
}
bool CThreadSyncObject::operator!() const {
    #if IsWindows()
        return ! this->m_bCreatedHandle;
    #elif IsPosix()
        return ! this->m_bInitalized;
    #endif
}
// TODO: Verify that this should return `true` when _not_ signaled
bool CThreadSyncObject::Wait( uint32 dwTimeoutMs ) {
    #if IsWindows()
        return WaitForSingleObject(this->m_hSyncObject, dwTimeoutMs);
    #elif IsPosix()
        pthread_mutex_lock( &this->m_Mutex );
        timeval val{};
        gettimeofday( &val, nullptr );
        timespec spec{
            .tv_sec = val.tv_sec,
            .tv_nsec = static_cast<int32>( dwTimeoutMs * 1000 ) + val.tv_usec / 1000
        };
        auto res{ pthread_cond_timedwait( &this->m_Condition, &this->m_Mutex, &spec ) };
        pthread_mutex_unlock( &this->m_Mutex );
		return res;
    #endif
}
void CThreadSyncObject::AssertUseable() {
	#if IsDebug()
        #if IsWindows()
            Assert ( this->m_bCreatedHandle );
        #elif IsPosix()
            Assert( this->m_bInitalized ); // TODO: ????
        #endif
	#endif
}
// ----- CThreadEvent -----
//
CThreadEvent::CThreadEvent( bool fManualReset ) {
	this->m_bManualReset = fManualReset;
    #if IsWindows()
        this->m_hSyncObject = CreateEventA(nullptr, fManualReset, 0, nullptr);
        this->m_bCreatedHandle = true;
    #endif
}
bool CThreadEvent::Set() {
    #if IsWindows()
        return SetEvent(this->m_hSyncObject);
    #elif IsPosix()
        pthread_mutex_lock( &this->m_Mutex );
        this->m_cSet = 1;
        // wake up waiting threads
        auto err{ pthread_cond_signal( &this->m_Condition ) };
        pthread_mutex_unlock( &this->m_Mutex );
        // true if no error occurred, false otherwise. TODO?: return the error code instead of just bool
        return err == 0;
    #endif
}
bool CThreadEvent::Reset() {
#if IsWindows()
    return ResetEvent(this->m_hSyncObject);
#elif IsPosix()
	pthread_mutex_lock( &this->m_Mutex );
	auto wasSet{ this->m_cSet == 1 };
	this->m_cSet = 0;
	pthread_mutex_unlock( &this->m_Mutex );
	return wasSet;
#endif
}
bool CThreadEvent::Check() {
#if IsWindows()
    return this->Wait(0);
#elif IsPosix()
    pthread_mutex_lock( &this->m_Mutex );
	auto isSet{ this->m_cSet == 1 };
	pthread_mutex_unlock( &this->m_Mutex );

	return isSet;
#endif
}
bool CThreadEvent::Wait( uint32 dwTimeout ) {
	return CThreadSyncObject::Wait( dwTimeout );
}

// ----- CThread -----
//
static thread_local CThread* g_CurrentThread{ nullptr };
CThread::CThread() = default;
CThread::~CThread() = default;
unsigned int CThread::ThreadProc( void* pv ) {
	// load up data
	const auto init{ static_cast<ThreadInit_t*>( pv ) };
	// set current thread object
	g_CurrentThread = init->pThread;
	// try to initialize
	const bool success{ g_CurrentThread->Init() };
	*init->pfInitSuccess = success;
	init->pInitCompleteEvent->Set();
	// start thread
	int res{};
	if ( success ) {
		res = g_CurrentThread->Run();
		g_CurrentThread->m_result = res;
		g_CurrentThread->OnExit();
		g_CurrentThread->Cleanup();
	}

	return res;
}

const char* CThread::GetName() {
	return this->m_szName;
}
void CThread::SetName( const char* pName ) {
	#if IsWindows()
        size_t size { strlen(pName) + 1 };
        wchar_t* pNameW { new wchar_t[size] };
        mbstowcs(pNameW,pName,size);
		SetThreadDescription( this->m_hThread, pNameW );
        delete[] pNameW;
	#elif IsPosix()
		pthread_setname_np( this->m_threadId, pName );
	#endif
	strncpy( this->m_szName, pName, 31 );
	this->m_szName[31] = '\0';
}

bool CThread::Start( unsigned nBytesStack ) {
	CThreadEvent initCompleted{ true };
	bool success{ false };
	// init facility data
	ThreadInit_t init{
		.pThread = this,
		.pInitCompleteEvent = &initCompleted,
		.pfInitSuccess = &success,
	};

	#if IsWindows()
		m_hThread = CreateThread( nullptr, nBytesStack, CThread::ThreadProc, &init, 0, &m_threadId );
		if (! m_hThread ) {
			return false;
		}
	#elif IsPosix()
		pthread_attr_t attrs;
		if (! pthread_attr_init( &attrs ) ) {
			return false;
		}
		if (! pthread_attr_setstacksize( &attrs, nBytesStack ) ) {
			pthread_attr_destroy( &attrs );
			return false;
		}
		// TODO: Will this need to be changed when porting to x64?
		if (! pthread_create( &m_threadId, &attrs, reinterpret_cast<void*(*)( void* )>( ThreadProc ), &init ) ) {
			return false;
		}
		pthread_attr_destroy( &attrs );
	#endif

	// wait for `Init()` call to complete
	initCompleted.Wait();
	return success;
}

bool CThread::IsAlive() {
	return this->m_result != -1;
}

bool CThread::Join( uint32 timeout ) {
	#if IsWindows()
		auto res{ WaitForSingleObject( m_hThread, timeout ) };
		if ( res == /*WAIT_TIMEOUT*/0x00000102L || res == /*WAIT_FAILED*/-1 ) {
			return false;
		}
		GetExitCodeThread( m_hThread, m_result );
		return true;
	#elif IsPosix()
		// TODO: Is this correct?
		timespec spec{};
		clock_gettime( CLOCK_MONOTONIC, &spec );
		spec.tv_sec += static_cast<int32>( timeout ) / 1000;
		spec.tv_nsec += static_cast<int32>( timeout ) % 1000 * 1000 * 1000;
		// TODO: Will this need to be changed when porting to x64?
		return pthread_timedjoin_np( m_threadId, reinterpret_cast<void**>( &m_result ), &spec ) == 0;
	#endif
}

#if IsWindows()
	HANDLE CThread::GetThreadHandle() {
		return this->m_hThread;
	}
#endif
uint CThread::GetThreadId() const {
	return this->m_threadId;
}
int CThread::GetResult() const {
	return this->m_result;
}
void CThread::Stop( int exitCode ) {
	AssertUnreachable();
}
int CThread::GetPriority() const {
	#if IsWindows()
		return GetThreadPriority( this->m_hThread );
	#elif IsPosix()
		int unused;
		sched_param params{};
		pthread_getschedparam( this->m_threadId, &unused, &params );
		return params.sched_priority;
	#endif
}
bool CThread::SetPriority( int pValue ) {
	#if IsWindows()
		return SetThreadPriority( this->m_hThread, pValue );
	#elif IsPosix()
		return pthread_setschedprio( this->m_threadId, pValue ) == 0;
	#endif
}
void CThread::SuspendCooperative() {
	AssertUnreachable();
}
void CThread::ResumeCooperative() {
	AssertUnreachable();
}
void CThread::BWaitForThreadSuspendCooperative() {
	AssertUnreachable();
}
#if !IsLinux()
	unsigned int CThread::Suspend() {
		return SuspendThread( this->m_hThread );
	}

	unsigned int CThread::Resume() {
		return ResumeThread( this->m_hThread );
	}
#endif
bool CThread::Terminate( int exitCode ) {
	this->m_result = exitCode;
	#if IsWindows()
		return TerminateThread( this->m_hThread, exitCode );
	#elif IsPosix()
		return pthread_kill( this->m_threadId, SIGKILL ) == 0;
	#endif
}
CThread* CThread::GetCurrentCThread() {
	return g_CurrentThread;
}
void CThread::Yield() {
	#if IsWindows()
		Sleep( 0 );
	#elif IsPosix()
		sched_yield();
	#endif
}
void CThread::Sleep( unsigned duration ) {
	ThreadSleep( duration );
}
bool CThread::Init() { return true; }  // empty default
void CThread::OnExit() { }   // empty default
void CThread::Cleanup() { }  // empty default
bool CThread::WaitForCreateComplete( CThreadEvent* pEvent ) {
	AssertUnreachable();
	return {};
}
CThread::ThreadProc_t CThread::GetThreadProc() {
    return ThreadProc;
}
bool CThread::IsThreadRunning() {
	// TODO: Figure out how to check that the thread is not suspended
	return this->IsAlive();
}

