//
// Created by ENDERZOMBI102 on 03/09/2024.
//
#include "queuedloader.hpp"
#include "dbg.h"
#include "filesystem.h"
#include "strtools.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


// ---------------
// CQueuedLoader
// ---------------
bool CQueuedLoader::Connect( CreateInterfaceFn factory ) {
	m_FileSystem = static_cast<IFileSystem*>( factory( FILESYSTEM_INTERFACE_VERSION, nullptr ) );
	AssertFatalMsg( m_FileSystem, "Somehow, the IFileSystem implementation is missing, what???" );
	return true;
}
void CQueuedLoader::Disconnect() {
	m_FileSystem = nullptr;
}
void* CQueuedLoader::QueryInterface( const char* pInterfaceName ) {
	return nullptr;
}
InitReturnVal_t CQueuedLoader::Init() {
	return INIT_OK;
}
void CQueuedLoader::Shutdown() { }

void CQueuedLoader::InstallLoader( ResourcePreload_t pType, IResourcePreload* pLoader ) {
	m_ResourcePreloaders.Insert( pType, pLoader );
}
void CQueuedLoader::InstallProgress( ILoaderProgress* pProgress ) {
	m_ProgressListeners.AddToTail( pProgress );
}

bool CQueuedLoader::BeginMapLoading( const char* pMapName, bool bLoadForHDR, bool bOptimizeMapReload ) {
	AssertFatalMsg( pMapName, "Why were we requested to load map `nullptr`???" );
	// setup stuff
	m_SameMap = V_strcmp( m_LastMap, pMapName );
	V_strncpy( m_CurrentMap, pMapName, static_cast<int>( std::size( m_CurrentMap ) ) );

	// notify observers
	for ( const auto it : m_ProgressListeners ) {
		it->BeginProgress();
	}
	for ( const auto& node : m_ResourcePreloaders ) {
		node.elem->PurgeUnreferencedResources();
	}

	AssertUnreachable();
	return {};
}
void CQueuedLoader::EndMapLoading( bool bAbort ) {
	// notify observers
	for ( const auto it : m_ProgressListeners ) {
		it->EndProgress();
	}
	for ( const auto& node : m_ResourcePreloaders ) {
		node.elem->OnEndMapLoading( bAbort );
	}

	V_strncpy( m_LastMap, m_CurrentMap, static_cast<int>( std::size( m_CurrentMap ) ) );
	AssertUnreachable();
}
bool CQueuedLoader::AddJob( const LoaderJob_t* pLoaderJob ) {
	m_Jobs.AddToTail( pLoaderJob );
	return {};
}

void CQueuedLoader::AddMapResource( const char* pFilename ) {
	AssertUnreachable();
}

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

bool CQueuedLoader::ClaimAnonymousJob( const char* pFilename, QueuedLoaderCallback_t pCallback, void* pContext, void* pContext2 ) {
	AssertUnreachable();
	return {};
}
bool CQueuedLoader::ClaimAnonymousJob( const char* pFilename, void** pData, int* pDataSize, LoaderError_t* pError ) {
	AssertUnreachable();
	return {};
}

bool CQueuedLoader::IsMapLoading() const {
	return not m_Finished;
}
bool CQueuedLoader::IsSameMapLoading() const {
	return m_SameMap;
}
bool CQueuedLoader::IsFinished() const {
	return m_Finished;
}

bool CQueuedLoader::IsBatching() const {
	return m_Batching;
}

bool CQueuedLoader::IsDynamic() const {
	return m_Dynamic;
}

int CQueuedLoader::GetSpewDetail() const {
	return m_SpewDetail;
}

void CQueuedLoader::PurgeAll() {
	// notify observers
	for ( const auto& node : m_ResourcePreloaders ) {
		node.elem->PurgeAll();
	}
}

namespace { CQueuedLoader s_QueuedLoader{}; }
IQueuedLoader* g_pQueuedLoader{ &s_QueuedLoader };
EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CQueuedLoader, IQueuedLoader, QUEUEDLOADER_INTERFACE_VERSION, s_QueuedLoader );
