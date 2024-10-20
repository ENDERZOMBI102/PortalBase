//
// Created by ENDERZOMBI102 on 28/03/2024.
//   Heavily modified version of the SDK 2006 version of `jobthread.cpp`
//
#include "platform.h"
#if IsWindows()
	#include <synchapi.h>
#endif
#include "jobthread.hpp"

// FIXME: This file might not be completely thread-safe, do a check
CThreadPool::CThreadPool() = default;
CThreadPool::~CThreadPool() = default;

bool CThreadPool::Start( const ThreadPoolStartParams_t& startParams ) {
	m_Threads.EnsureCapacity( startParams.nThreads );

	for ( auto i{0}; i < startParams.nThreads; i += 1 ) {
		m_Threads.AddToTail();
		m_Threads[i] = CreateSimpleThread( PoolThreadFunc, this, startParams.nStackSize );
		while ( m_IdleCount == i ) { }  // wait for thread to start
	}

	if ( startParams.fDistribute == ThreeState_t::TRS_TRUE ) {
		// TODO
	}

	return true;
}
bool CThreadPool::Stop( int timeout ) {
	// tell the threads to stop
	this->m_Exit.Set();
	// wait for them to finish
	#if IsWindows()
		return WaitForMultipleObjects( m_Threads.Count(), (HANDLE*) m_Threads.Base(), true, timeout );
	#else
		auto flag{ true };
		// FIXME: Timeout is not comulative!
		for ( const auto& thread : this->m_Threads ) {
			flag = ThreadJoin( thread, timeout ) && flag;
		}
		return flag;
	#endif
}

uint32 CThreadPool::GetJobCount() {
	return m_Queue.Count();
}
int CThreadPool::NumThreads() {
	return this->m_Threads.Count();
}
int CThreadPool::NumIdleThreads() {
	return this->m_IdleCount;
}

int CThreadPool::SuspendExecution() {
	AssertFatalMsg( not ThreadInMainThread(), "Tried to suspend threadpool execution outside of main thread" );

	// If not already suspended
	if ( m_State != State::SUSPENDED ) {
		if ( (m_State = State::SUSPENDED) != State::SUSPENDED ) {
			// suspension failed...
			return 0;
		}

		// managed to change state! let's wait for the threads to pause
		for ( auto thread : m_Threads ) {
			// TODO: wait for them
		}
	}

	return 1;
}
int CThreadPool::ResumeExecution() {
	AssertFatalMsg( not ThreadInMainThread(), "Tried to resume threadpool execution outside of main thread" );

	AssertMsg( m_State != State::SUSPENDED, "Attempted resume when not suspended" );

	if ( m_State == State::SUSPENDED ) {
		if ( (m_State = State::EXECUTING) != State::EXECUTING ) {
			// resume failed (somehow)...
			return 0;
		}

		// managed to change state! let's wait for the threads to resume
		for ( auto thread : m_Threads ) {
			// TODO: wait for them
		}
	}
	return 0;
}

int CThreadPool::YieldWait( CThreadEvent * *pEvents, int nEvents, bool bWaitAll, unsigned timeout ) {
	AssertUnreachable(); return {};
}
int CThreadPool::YieldWait( CJob**, int nJobs, bool bWaitAll, unsigned timeout ) {
	AssertUnreachable(); return {};
}
void CThreadPool::Yield( unsigned timeout ) {
	AssertUnreachable();
}

void CThreadPool::AddJob( CJob* pJob ) {
	if (! pJob ) {
		return;
	}

	// wait for a worker to be available (? is this needed ?)
	// while ( m_IdleCount == 0 ) { }

	// add the job to the queue and update its status
	m_Mutex.Lock();
		pJob->AddRef();
		m_Queue.AddToTail( pJob );

		uint32 idx{ m_Queue.Tail() };
		const int32 priority{ pJob->GetPriority() };

		while ( idx != CUtlLinkedList<CJob*>::InvalidIndex() && priority > m_Queue[idx]->GetPriority() ) {
			idx = m_Queue.Previous( idx );
		}

		if ( idx != CUtlLinkedList<CJob*>::InvalidIndex() ) {
			m_Queue.InsertAfter( idx, pJob );
		} else {
			m_Queue.AddToHead( pJob );
		}

		pJob->m_pThreadPool = this;
		pJob->m_status = JOB_STATUS_PENDING;
	m_Mutex.Unlock();

	// tell our workers that we have a job
	m_JobAccepted.Reset();
	m_JobAvailable.Set();
	m_JobAccepted.Wait();
}

void CThreadPool::ExecuteHighPriorityFunctor( CFunctor* pFunctor ) {
	AssertUnreachable();
}

void CThreadPool::ChangePriority( CJob* pJob, JobPriority_t priority ) {
	AssertUnreachable();
}

int CThreadPool::ExecuteToPriority( JobPriority_t toPriority, JobFilter_t pfnFilter ) {
	AssertUnreachable();
	return {};
}
int CThreadPool::AbortAll() {
	if ( CThread::GetCurrentCThread() != m_CoordinatorThread ) {
		SuspendExecution();
	}


	// abort them all
	m_Mutex.Lock();
		for ( const auto job : m_Queue ) {
			job->Abort();
			job->Release();
		}
		m_Queue.RemoveAll();
	m_Mutex.Unlock();

	if ( CThread::GetCurrentCThread() != m_CoordinatorThread ) {
		ResumeExecution();
	}

	return 0;
}

void CThreadPool::AddFunctorInternal( CFunctor* pFunctor, CJob** ppJob, const char* pszDescription, unsigned flags ) {
	AssertUnreachable();
}

CJob* CThreadPool::GetDummyJob() {
	AssertUnreachable();
	return {};
}

void CThreadPool::Distribute( bool bDistribute, int* pAffinityTable ) {
	AssertUnreachable();
}

bool CThreadPool::Start( const ThreadPoolStartParams_t& startParams, const char* pszNameOverride ) {
	AssertUnreachable();
	return {};
}

unsigned CThreadPool::PoolThreadFunc( void* pParam ) {
	const auto pOwner = static_cast<CThreadPool*>( pParam );
	pOwner->m_IdleCount += 1;

	while ( true ) {
		// handle incoming jobs
		if ( pOwner->m_JobAvailable.Check() ) {
			pOwner->m_IdleCount -= 1;

			// accept the job
			const auto head{ pOwner->m_Queue.Head() };
			CJob* job = pOwner->m_Queue.Element( head );
			if ( job == nullptr ) {
				// job got sniped from our hands, go back idling
				pOwner->m_IdleCount += 1;
				continue;
			}
			pOwner->m_Queue.Remove( head );
			pOwner->m_JobAccepted.Set();

			// run the job
			job->TryExecute();
			job->Release();

			pOwner->m_IdleCount += 1;
		}
		// handle exit command
		if ( pOwner->m_Exit.Check() ) {
			return 0;
		}
	}
}


/*
bool CThreadPool::RemoveJob( CJob* pJob ) {
	if ( !pJob ) {
		return false;
	}

	AUTO_LOCK( m_mutex );

	if ( !m_Queue.IsValidIndex( pJob->m_QueueID ) ) {
		return false;
	}

	// Take the job out
	m_Queue.Remove( pJob->m_QueueID );
	pJob->m_QueueID = m_Queue.InvalidIndex();
	pJob->m_pFulfiller = NULL;
	pJob->Release();

	// Release master to put more in
	ReleasePut();

	// If we're transitioning to empty...
	if ( m_Queue.Count() == 0 ) {
		// Block the worker until there's something to do...
		m_JobSignal.Reset();
	}

	return true;
}

int CThreadPool::ExecuteToPriority( JobPriority_t iToPriority ) {
	int nExecuted = 0;

	if ( CThread::GetCurrentCThread() != this ) {
		SuspendExecution();
	}

	if ( m_Queue.Count() ) {
		CJob* pJob = NULL;

		while ( ( pJob = GetJob() ) != NULL ) {
			if ( pJob->GetPriority() >= iToPriority ) {
				pJob->Execute();
				pJob->Release();
				pJob = NULL;
				nExecuted++;
			} else {
				break;
			}
		}

		// Extracted one of lower priority, so reinsert it...
		if ( pJob ) {
			AddJob( pJob );
			pJob->Release();
		}
	}

	if ( CThread::GetCurrentCThread() != this ) {
		ResumeExecution();
	}
	return nExecuted;
}


CJob* CThreadPool::GetJob() {
	CJob* pReturn = NULL;
	m_mutex.Lock();
	unsigned i = m_Queue.Head();

	if ( i != m_Queue.InvalidIndex() ) {
		pReturn = m_Queue[ i ];
		pReturn->AddRef();
		RemoveJob( pReturn );
	}

	m_mutex.Unlock();
	return pReturn;
}

bool CThreadPool::Start( unsigned nBytesStack ) {
	if ( CWorkerThread::Start( nBytesStack ) ) {
		BoostPriority();
		return true;
	}
	return false;
}

int CThreadPool::Run() {
	enum FulfillerEvent_t {
		CALL_FROM_MASTER,
		JOB_REQUEST
	};

	g_TestThreadPool.Init( 4, 0, 2, false );

	// Wait for either a call from the master thread, or an item in the queue...
	DWORD waitResult;
	bool bExit = false;
	HANDLE waitHandles[ 2 ];

	waitHandles[ CALL_FROM_MASTER ] = GetCallHandle();
	waitHandles[ JOB_REQUEST ] = GetJobSignalHandle();

	while ( !bExit &&
			( waitResult = WaitForMultipleObjects( 2, waitHandles, false, INFINITE ) ) != WAIT_FAILED ) {
		switch ( waitResult - WAIT_OBJECT_0 ) {
				// It's a call from the master thread...
			case CALL_FROM_MASTER: {
				switch ( GetCallParam() ) {
					case AF_EXIT:
						Reply( true );
						bExit = true;
						break;

					case AF_SUSPEND:
						g_TestThreadPool.WaitForIdle();
						Reply( true );
						Suspend();
						break;

					default:
						AssertMsg( 0, "Unknown call async fulfiller" );
						Reply( false );
						break;
				}
				break;
			}

				// Otherwise, if there's a read request to service...
			case JOB_REQUEST: {
				// Get the request
				CJob* pJob;

				while ( ( pJob = GetJob() ) != NULL ) {
					// Job can be NULL if the main thread may have preempted and fulfilled
					// the job on its own.
					g_TestThreadPool.Execute( pJob );
					pJob->Release();
				}

				break;
			}

			default:
				AssertMsg( 0, "There was nothing to do!" );
		}
	}

	g_TestThreadPool.Term();
	return 0;
}

//-----------------------------------------------------------------------------
// CJob
//-----------------------------------------------------------------------------
JobStatus_t CJob::Execute() {
	AUTO_LOCK( m_mutex );
	AddRef();

	JobStatus_t result;

	switch ( m_status ) {
		case JOB_STATUS_UNSERVICED:
		case JOB_STATUS_PENDING: {
			if ( m_pFulfiller )// Jobs can exist on thier own
			{
				CAutoLock autoLock( m_pFulfiller->GetQueueLock() );
				if ( m_pFulfiller ) {
					m_pFulfiller->RemoveJob( this );
				}
			}

			// Service it
			m_status = JOB_STATUS_INPROGRESS;
			ThreadSleep( 0 );
			result = m_status = DoExecute();
			DoCleanup();
			ThreadSleep( 0 );
			break;
		}

		case JOB_STATUS_INPROGRESS:
			AssertMsg( 0, "Mutex Should have protected use while processing" );
			// fall through...

		case JOB_OK:
		case JOB_STATUS_ABORTED:
			result = m_status;
			break;

		default:
			AssertMsg( m_status < JOB_OK, "Unknown async job state" );
			result = m_status;
	}

	Release();
	return result;
}

JobStatus_t CJob::TryExecute() {
	// TryLock() would only fail if another thread has entered
	// Execute() or Abort()
	if ( TryLock() ) {
		// ...service the request
		Execute();
		Unlock();
	}
	return m_status;
}

JobStatus_t CJob::Abort( bool bDiscard ) {
	AUTO_LOCK( m_mutex );
	AddRef();

	JobStatus_t result;

	switch ( m_status ) {
		case JOB_STATUS_UNSERVICED:
		case JOB_STATUS_PENDING: {
			if ( m_pFulfiller )// Jobs can exist on thier own
			{
				CAutoLock autoLock( m_pFulfiller->GetQueueLock() );
				if ( m_pFulfiller ) {
					m_pFulfiller->RemoveJob( this );
				}
			}

			result = m_status = DoAbort( bDiscard );
			if ( bDiscard )
				DoCleanup();
		} break;

		case JOB_STATUS_ABORTED:
		case JOB_STATUS_INPROGRESS:
		case JOB_OK:
			result = m_status;
			break;

		default:
			AssertMsg( m_status < JOB_OK, "Unknown async job state" );
			result = m_status;
	}

	Release();
	return result;
}
*/

IThreadPool* CreateThreadPool();
void DestroyThreadPool( IThreadPool* pPool );
void RunThreadPoolTests();


namespace {
	CThreadPool s_ThreadPool{};
}
IThreadPool* g_pThreadPool{ &s_ThreadPool };
