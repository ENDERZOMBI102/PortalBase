//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//===========================================================================//
#pragma once
#include "appframework/IAppSystem.h"
#include "tier0/platform.h"

class CFunctor;

enum LoaderError_t {
	LOADERERROR_NONE = 0,
	LOADERERROR_FILEOPEN = -1,
	LOADERERROR_READING = -2,
};

enum LoaderPriority_t {
	LOADERPRIORITY_ANYTIME = 0,      // low priority, job can finish during gameplay
	LOADERPRIORITY_BEFOREPLAY = 1,   // job must complete before load ends
	LOADERPRIORITY_DURINGPRELOAD = 2,// job must be complete during preload phase
};

using QueuedLoaderCallback_t = void(*)( void* pContext, void* pContext2, const void* pData, int nSize, LoaderError_t loaderError );
using DynamicResourceCallback_t = void(*)( const char* pFilename, void* pContext, void* pContext2 );

struct LoaderJob_t {
	const char* m_pFilename{};             // path to resource
	const char* m_pPathID{};               // optional, can be NULL
	QueuedLoaderCallback_t m_pCallback{};  // called at i/o delivery
	void* m_pContext{};                    // caller provided data
	void* m_pContext2{};                   // caller provided data
	void* m_pTargetData{};                 // optional, caller provided target buffer
	int m_nBytesToRead{};                  // optional read clamp, otherwise 0
	uint32 m_nStartOffset{};               // optional start offset, otherwise 0
	LoaderPriority_t m_Priority{};         // data must arrive by specified interval
	bool m_bPersistTargetData{};           // caller wants ownership of i/o buffer
};

enum ResourcePreload_t {
	RESOURCEPRELOAD_UNKNOWN,
	RESOURCEPRELOAD_SOUND,
	RESOURCEPRELOAD_MATERIAL,
	RESOURCEPRELOAD_MODEL,
	RESOURCEPRELOAD_CUBEMAP,
	RESOURCEPRELOAD_STATICPROPLIGHTING,
	RESOURCEPRELOAD_ANONYMOUS,
	RESOURCEPRELOAD_COUNT
};

abstract_class IResourcePreload {
public:
	virtual ~IResourcePreload() = default;
	// Called during preload phase for ALL the resources expected by the level.
	// Caller should not do i/o but generate AddJob() requests. Resources that already exist
	// and are not referenced by this function would be candidates for purge.
	virtual bool CreateResource( const char* pName ) = 0;

	// Sent as an event hint during preload, that creation has completed, AddJob() i/o is about to commence.
	// Caller should purge any unreferenced resources before the AddJobs are performed.
	// "Must Complete" data will be guaranteed finished, at preload conclusion, before the normal load phase commences.
	virtual void PurgeUnreferencedResources() = 0;

	// Sent as an event hint that gameplay rendering is imminent.
	// Low priority jobs may still be in async flight.
	virtual void OnEndMapLoading( bool bAbort ) = 0;

	virtual void PurgeAll() = 0;
};

// Default implementation
class CResourcePreload : public IResourcePreload {
	void PurgeUnreferencedResources() override { }
	void OnEndMapLoading( bool bAbort ) override { }
	void PurgeAll() override { }
};

// UI can install progress notification
abstract_class ILoaderProgress {
public:
	virtual ~ILoaderProgress() = default;
	virtual void BeginProgress() = 0;
	virtual void EndProgress() = 0;
	/**
	 * Update the state of the progress.
	 * @note implementation must ignore calls if not scoped by Begin/End
	 */
	virtual void UpdateProgress( float progress ) = 0;
};

// spew detail
enum LoaderSpewDetail {
	LOADER_DETAIL_NONE = 0,
	LOADER_DETAIL_TIMING = 1 << 0,
	LOADER_DETAIL_COMPLETIONS = 1 << 1,
	LOADER_DETAIL_LATECOMPLETIONS = 1 << 2,
	LOADER_DETAIL_PURGES = 1 << 3
};

#define QUEUEDLOADER_INTERFACE_VERSION "QueuedLoaderVersion004"
abstract_class IQueuedLoader : public IAppSystem {
public:
	virtual void InstallLoader( ResourcePreload_t type, IResourcePreload* pLoader ) = 0;
	virtual void InstallProgress( ILoaderProgress* pProgress ) = 0;

	// Set bOptimizeReload if you want appropriate data (such as static prop lighting)
	// to persist - rather than being purged and reloaded - when going from map A to map A.
	virtual bool BeginMapLoading( const char* pMapName, bool bLoadForHDR, bool bOptimizeMapReload ) = 0;
	virtual void EndMapLoading( bool bAbort ) = 0;
	virtual bool AddJob( const LoaderJob_t* pLoaderJob ) = 0;

	// injects a resource into the map's reslist, rejected if not understood
	virtual void AddMapResource( const char* pFilename ) = 0;

	// dynamically load a map resource
	virtual void DynamicLoadMapResource( const char* pFilename, DynamicResourceCallback_t pCallback, void* pContext, void* pContext2 ) = 0;
	virtual void QueueDynamicLoadFunctor( CFunctor* pFunctor ) = 0;
	virtual bool CompleteDynamicLoad() = 0;

	// callback is asynchronous
	virtual bool ClaimAnonymousJob( const char* pFilename, QueuedLoaderCallback_t pCallback, void* pContext, void* pContext2 = nullptr ) = 0;
	// provides data if loaded, caller owns data
	virtual bool ClaimAnonymousJob( const char* pFilename, void** pData, int* pDataSize, LoaderError_t* pError = nullptr ) = 0;

	[[nodiscard]]
	virtual bool IsMapLoading() const = 0;
	[[nodiscard]]
	virtual bool IsSameMapLoading() const = 0;
	[[nodiscard]]
	virtual bool IsFinished() const = 0;

	// callers can expect that jobs are not immediately started when batching
	[[nodiscard]]
	virtual bool IsBatching() const = 0;

	[[nodiscard]]
	virtual bool IsDynamic() const = 0;

	// callers can conditionalize operational spew
	[[nodiscard]]
	virtual int GetSpewDetail() const = 0;

	virtual void PurgeAll() = 0;
};

extern IQueuedLoader* g_pQueuedLoader;
