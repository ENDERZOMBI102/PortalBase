//
// Created by ENDERZOMBI102 on 28/03/2024.
//
#pragma once
#include "tier0/dbg.h"
#include "tier1/utlvector.h"
#include "utlpriorityqueue.h"
#include "vstdlib/jobthread.h"


class CThreadPool : public CRefCounted1<IThreadPool> {
public:
	CThreadPool();
	~CThreadPool() override;
public:  // IThreadPool
	// Thread functions
	bool Start( const ThreadPoolStartParams_t& startParams = ThreadPoolStartParams_t{} ) override;
	bool Stop( int timeout = TT_INFINITE ) override;

	// Functions for any thread
	unsigned GetJobCount() override;
	int NumThreads() override;
	int NumIdleThreads() override;

	/**
	 * Pauses the execution/processing of jobs.
	 */
	int SuspendExecution() override;
	/**
	 * Resumes the execution/processing of jobs.
	 */
	int ResumeExecution() override;

	// Offer the current thread to the pool
	int YieldWait( CThreadEvent** pEvents, int nEvents, bool bWaitAll = true, unsigned timeout = TT_INFINITE ) override;
	int YieldWait( CJob**, int nJobs, bool bWaitAll = true, unsigned timeout = TT_INFINITE ) override;
	void Yield( unsigned timeout ) override;

	// Add a native job to the queue (master thread)
	void AddJob( CJob* ) override;

	// All threads execute pFunctor asap. Thread will either wake up
	//  and execute or execute pFunctor right after completing current job and
	//  before looking for another job.
	void ExecuteHighPriorityFunctor( CFunctor* pFunctor ) override;

	// Change the priority of an active job
	void ChangePriority( CJob* p, JobPriority_t priority ) override;

	// Bulk job manipulation (blocking)
	int ExecuteToPriority( JobPriority_t toPriority, JobFilter_t pfnFilter = nullptr ) override;
	int AbortAll() override;

	//-----------------------------------------------------
	void Reserved1() override { }
private:
	void AddFunctorInternal( CFunctor*, CJob** = nullptr, const char* pszDescription = nullptr, unsigned flags = 0 ) override;

	// Services for internal use by job instances
	friend class CJob;

	CJob* GetDummyJob() override;
public:
	void Distribute( bool bDistribute = true, int* pAffinityTable = nullptr ) override;

	bool Start( const ThreadPoolStartParams_t& startParams, const char* pszNameOverride ) override;
private:
	static uint32 PoolThreadFunc( void* pParam );
private:
	enum State {
		EXECUTING,
		SUSPENDED
	};
	CInterlockedIntT<State> m_State;

	CThread* m_CoordinatorThread{ nullptr };
	CThreadEvent m_JobAvailable;
	CThreadEvent m_JobAccepted;
	CThreadEvent m_Exit{ true };
	CInterlockedInt m_IdleCount;

	CUtlLinkedList<CJob*> m_Queue{};
	// mutex for adding/removing items to/from the queue
	CThreadFastMutex m_Mutex{};
	CUtlVector<ThreadHandle_t> m_Threads{};
};

// JOB_INTERFACE IThreadPool* CreateThreadPool();
// JOB_INTERFACE void DestroyThreadPool( IThreadPool* pPool );
// JOB_INTERFACE void RunThreadPoolTests();
// JOB_INTERFACE IThreadPool* g_pThreadPool;
