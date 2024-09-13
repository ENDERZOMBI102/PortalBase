//
// Created by ENDERZOMBI102 on 03/09/2024.
//
#include "queuedloader.hpp"
#include "dbg.h"
#include "strtools.h"


namespace {
	CQueuedLoader s_QueuedLoader{};
}


// ---------------
// CQueuedLoader
// ---------------
bool CQueuedLoader::Connect( CreateInterfaceFn factory ) { return true; }
void CQueuedLoader::Disconnect() { }
void* CQueuedLoader::QueryInterface( const char* pInterfaceName ) {
	if ( V_strcmp( pInterfaceName, QUEUEDLOADER_INTERFACE_VERSION ) == 0 ) {
		return this;
	}
	return nullptr;
}
InitReturnVal_t CQueuedLoader::Init() { return INIT_OK; }
void CQueuedLoader::Shutdown() { }

void CQueuedLoader::InstallLoader( ResourcePreload_t pType, IResourcePreload* pLoader ) {
	m_ResourcePreloaders.Insert( pType, pLoader );
}
void CQueuedLoader::InstallProgress( ILoaderProgress* pProgress ) {
	AssertUnreachable();
}

// Set bOptimizeReload if you want appropriate data (such as static prop lighting)
// to persist - rather than being purged and reloaded - when going from map A to map A.
bool CQueuedLoader::BeginMapLoading( const char* pMapName, bool bLoadForHDR, bool bOptimizeMapReload ) {
	AssertUnreachable();
	return {};
}
void CQueuedLoader::EndMapLoading( bool bAbort ) {
	AssertUnreachable();
}
bool CQueuedLoader::AddJob( const LoaderJob_t* pLoaderJob ) {
	AssertUnreachable();
	return {};
}

// injects a resource into the map's reslist, rejected if not understood
void CQueuedLoader::AddMapResource( const char* pFilename ) {
	AssertUnreachable();
}

// dynamically load a map resource
void CQueuedLoader::DynamicLoadMapResource( const char* pFilename, DynamicResourceCallback_t pCallback, void* pContext, void* pContext2 ) {
	AssertUnreachable();
}
void CQueuedLoader::QueueDynamicLoadFunctor( CFunctor* pFunctor ) {
	AssertUnreachable();
}
bool CQueuedLoader::CompleteDynamicLoad() {
	AssertUnreachable();
	return {};
}

// callback is asynchronous
bool CQueuedLoader::ClaimAnonymousJob( const char* pFilename, QueuedLoaderCallback_t pCallback, void* pContext, void* pContext2 ) {
	AssertUnreachable();
	return {};
}
// provides data if loaded, caller owns data
bool CQueuedLoader::ClaimAnonymousJob( const char* pFilename, void** pData, int* pDataSize, LoaderError_t* pError ) {
	AssertUnreachable();
	return {};
}

bool CQueuedLoader::IsMapLoading() const {
	AssertUnreachable();
	return {};
}
bool CQueuedLoader::IsSameMapLoading() const {
	AssertUnreachable();
	return {};
}
bool CQueuedLoader::IsFinished() const {
	AssertUnreachable();
	return {};
}

// callers can expect that jobs are not immediately started when batching
bool CQueuedLoader::IsBatching() const {
	AssertUnreachable();
	return {};
}

bool CQueuedLoader::IsDynamic() const {
	AssertUnreachable();
	return {};
}

// callers can conditionalize operational spew
int CQueuedLoader::GetSpewDetail() const {
	AssertUnreachable();
	return {};
}

void CQueuedLoader::PurgeAll() {
	AssertUnreachable();
}

IQueuedLoader* g_pQueuedLoader{ &s_QueuedLoader };
EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CQueuedLoader, IQueuedLoader, QUEUEDLOADER_INTERFACE_VERSION, s_QueuedLoader );

