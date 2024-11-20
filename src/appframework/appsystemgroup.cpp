//
// Created by ENDERZOMBI102 on 02/05/2024.
//   Based on code by OzxyBox: https://github.com/Source-SDK-Resources/source-sdk-example-qt
//
#include "appframework/IAppSystemGroup.h"
#include "filesystem_init.h"
// This must be the final include in a .cpp file!!!
#include "memdbgon.h"


namespace {
	CAppSystemGroup* s_RootAppSystem{ nullptr };
	// TODO: Use this to give better error messages
	int s_FailedSystemIndex{ -1 };
}

// ----------------------
// CAppSystemGroup
// ----------------------
CAppSystemGroup::CAppSystemGroup( CAppSystemGroup* pParentAppSystem )
	: m_Modules(), m_Systems(), m_SystemDict(), m_pParentAppSystem(pParentAppSystem) {
}

int CAppSystemGroup::Run() {
	s_RootAppSystem = this;
	int res{1};

	if ( not this->Create() ) {
		Error( "AppSystemGroup::Create() returned `false`, this is usually because modules are missing or corrupt, check log for possibly more info." );
		return 1;
	}
	if ( this->ConnectSystems() ) {
		if ( this->PreInit() ) {
			if ( this->InitSystems() != InitReturnVal_t::INIT_FAILED ) {
				res = this->Main();
				this->ShutdownSystems();  // TODO: Figure out if this can be pulled out of the if
			}
			this->PostShutdown();
		}
		this->DisconnectSystems();
	}
	this->UnloadAllModules();

    return res;
}

int CAppSystemGroup::Startup() {
	if ( not this->ConnectSystems() ) {
		return 1;
	}
	if ( not this->PreInit() ) {
		return 2;
	}
	if ( InitSystems() == InitReturnVal_t::INIT_FAILED ) {
		return 3;
	}
	return 0;
}
void CAppSystemGroup::Shutdown() {
	this->ShutdownSystems();
	this->PostShutdown();
	this->DisconnectSystems();
	this->UnloadAllModules();
}

CAppSystemGroup::AppSystemGroupStage_t CAppSystemGroup::GetErrorStage() const {
	return m_nErrorStage;
}

// protected
AppModule_t CAppSystemGroup::LoadModule( const char* pDLLName ) {
	for (  auto i{0}; i < m_Modules.Count(); i += 1 ) {
		if ( m_Modules[i].m_pModuleName && V_strcmp( m_Modules[i].m_pModuleName, pDLLName ) == 0 ) {
			return i;
		}
	}

	CSysModule* sysModule{ Sys_LoadModule( pDLLName ) };
	if ( not sysModule ) {
		Warning( "Failed to load module `%s`", pDLLName );
		return CUtlVector<Module_t>::InvalidIndex();
	}

	const CreateInterfaceFn factory{ Sys_GetFactory( sysModule ) };
	if ( not factory ) {
		Warning( "Failed to load module factory of `%s`", pDLLName );
		return CUtlVector<Module_t>::InvalidIndex();
	}

	const Module_t module{ .m_pModule = sysModule, .m_Factory = factory, .m_pModuleName = pDLLName };
	return m_Modules.AddToTail( module );
}
AppModule_t CAppSystemGroup::LoadModule( const CreateInterfaceFn factory ) {
	for ( int i{ 0 }; i < m_Modules.Count(); i += 1 ) {
		if ( m_Modules[i].m_Factory == factory ) {
			return i;
		}
	}

	const Module_t module{ .m_pModule = nullptr, .m_Factory = factory, .m_pModuleName = nullptr };
	return m_Modules.AddToTail( module );
}

IAppSystem* CAppSystemGroup::AddSystem( const AppModule_t pModule, const char* pInterfaceName ) {
	auto index{ m_SystemDict.Find( pInterfaceName ) };
	if ( index == CUtlDict<int, uint16>::InvalidIndex() ) {
		index = m_Systems.AddToTail();
	} else {
		Warning( "System `%s` has been added multiple times! This will overrides the last value!", pInterfaceName );
	}

	int retCode;
	const auto mod{ m_Modules[pModule] };
	const auto system{ static_cast<IAppSystem*>( mod.m_Factory( pInterfaceName, &retCode ) ) };
	if ( retCode != IFACE_OK ) {
		Warning( "Failed to load system for interface `%s` from module `%s`.\n", pInterfaceName, mod.m_pModuleName ? mod.m_pModuleName : "N/A" );
		return nullptr;
	}

	m_SystemDict.Insert( pInterfaceName, index );
	return system;
}
void CAppSystemGroup::AddSystem( IAppSystem* pAppSystem, const char* pInterfaceName ) {
	const auto index{ m_Systems.AddToTail( pAppSystem ) };
	m_SystemDict.Insert( pInterfaceName, index );
}

bool CAppSystemGroup::AddSystems( AppSystemInfo_t* pSystems ) {
	for ( int i{0}; pSystems[i].m_pModuleName; i += 1 ) {
		const auto info{ pSystems[i] };
		const auto module{ LoadModule( info.m_pModuleName ) };
		if ( module == CUtlVector<Module_t>::InvalidIndex() ) {
			s_FailedSystemIndex = i;
			m_nErrorStage = AppSystemGroupStage_t::CREATION;
			return false;
		}

		if ( not AddSystem( module,  info.m_pInterfaceName ) ) {
			s_FailedSystemIndex = i;
			m_nErrorStage = AppSystemGroupStage_t::CREATION;
			return false;
		}
	}

    return true;
}

void* CAppSystemGroup::FindSystem( const char* pInterfaceName ) {
	return GetFactory()( pInterfaceName, nullptr );
}

CreateInterfaceFn CAppSystemGroup::GetFactory() {
	return []( const char* pInterfaceName, int* pRetCode ) -> void* {
		AssertMsg( s_RootAppSystem != nullptr, "RootFactory was asked for an interface when no AppSystemGroup was ran!" );
		uint16 index{ s_RootAppSystem->m_SystemDict.Find( pInterfaceName ) };
		if ( index != CUtlDict<int, uint16>::InvalidIndex() ) {
			if ( pRetCode ) {
				*pRetCode = IFACE_OK;
			}
			return s_RootAppSystem->m_Systems[index];
		}

//		for ( const auto system : s_RootAppSystem->m_Systems ) {
//			if ( auto* iface = system->QueryInterface( pInterfaceName ) ) {
//				if ( pRetCode ) {
//					*pRetCode = IFACE_OK;
//				}
//				return iface;
//			}
//		}

		return Sys_GetFactoryThis()( pInterfaceName, pRetCode );
	};
}

// private
int CAppSystemGroup::OnStartup() { AssertUnreachable(); return {}; }
void CAppSystemGroup::OnShutdown() { AssertUnreachable(); }

void CAppSystemGroup::UnloadAllModules() {
	for ( const auto& module : m_Modules ) {
		Sys_UnloadModule( module.m_pModule );
	}
}
void CAppSystemGroup::RemoveAllSystems() { AssertUnreachable(); }

bool CAppSystemGroup::ConnectSystems() {
	const auto factory{ GetFactory() };

	for ( int i{0}; i < m_Systems.Count(); i += 1 ) {
		if ( not m_Systems[i]->Connect( factory ) ) {
			s_FailedSystemIndex = i;
			m_nErrorStage = AppSystemGroupStage_t::CONNECTION;
			return false;
		}
	}

	return true;
}
void CAppSystemGroup::DisconnectSystems() {
	for ( int i{0}; i < m_Systems.Count(); i += 1 ) {
		m_Systems[i]->Disconnect();
	}
}

InitReturnVal_t CAppSystemGroup::InitSystems() {
	for ( int i{0}; i < m_Systems.Count(); i += 1 ) {
		if ( m_Systems[i]->Init() != InitReturnVal_t::INIT_OK ) {
			s_FailedSystemIndex = i;
			m_nErrorStage = AppSystemGroupStage_t::INITIALIZATION;
			return InitReturnVal_t::INIT_FAILED;
		}
	}

	return InitReturnVal_t::INIT_OK;
}
void CAppSystemGroup::ShutdownSystems() {
	for ( int i{0}; i < m_Systems.Count(); i += 1 ) {
		m_Systems[i]->Shutdown();
	}
}

CAppSystemGroup* CAppSystemGroup::GetParent() {
	return m_pParentAppSystem;
}

CSysModule* CAppSystemGroup::LoadModuleDLL( const char* pDLLName ) {
	return Sys_LoadModule( pDLLName );
}

void CAppSystemGroup::ReportStartupFailure( int nErrorStage, int nSysIndex ) { AssertUnreachable(); }


// ----------------------
// CSteamAppSystemGroup
// ----------------------
CSteamAppSystemGroup::CSteamAppSystemGroup( IFileSystem* pFileSystem, CAppSystemGroup* pParentAppSystem )
	: CAppSystemGroup( pParentAppSystem ), m_pFileSystem{ pFileSystem } { }

void CSteamAppSystemGroup::Setup( IFileSystem* pFileSystem, CAppSystemGroup* pParentAppSystem ) {
	if ( pFileSystem ) {
		m_pFileSystem = pFileSystem;
	}
	if ( pParentAppSystem ) {
		m_pParentAppSystem = pParentAppSystem;
	}
}

bool CSteamAppSystemGroup::SetupSearchPaths( const char* pStartingDir, const bool bOnlyUseStartingDir, const bool bIsTool ) {
	// find the `gameinfo.txt`
	CFSSteamSetupInfo steamInfo;
	steamInfo.m_pDirectoryName = pStartingDir;
	steamInfo.m_bOnlyUseDirectoryName = bOnlyUseStartingDir;
	steamInfo.m_bToolsMode = bIsTool;
	steamInfo.m_bSetSteamDLLPath = true;
	steamInfo.m_bSteam = m_pFileSystem->IsSteam();
	if ( FileSystem_SetupSteamEnvironment( steamInfo ) != FS_OK ) {
		Error( "Failed to find `gameinfo.txt`! aborting." );
		return false;
	}
	V_strcpy_safe( m_pGameInfoPath, steamInfo.m_GameInfoPath );

	// mount the game
	CFSMountContentInfo fsInfo;
	fsInfo.m_pFileSystem = m_pFileSystem;
	fsInfo.m_bToolsMode = bIsTool;
	fsInfo.m_pDirectoryName = steamInfo.m_GameInfoPath;
	if ( FileSystem_MountContent( fsInfo ) != FS_OK ) {
		Error( "Failed to mount gameinfo! aborting." );
		return false;
	}

	// add `GAME` search paths
	CFSSearchPathsInit searchPathsInit;
	searchPathsInit.m_pDirectoryName = steamInfo.m_GameInfoPath;
	searchPathsInit.m_pFileSystem = fsInfo.m_pFileSystem;
	// TODO: Rest of params
	if ( FileSystem_LoadSearchPaths( searchPathsInit ) != FS_OK ) {
		Error( "Failed to load game search paths! aborting." );
		return false;
	}

	// add platform folder to search path
	char platform[ MAX_PATH ];
	V_strncpy( platform, steamInfo.m_GameInfoPath, MAX_PATH );
	V_StripTrailingSlash( platform );
	V_strncat( platform, "/../platform", MAX_PATH, MAX_PATH );
	fsInfo.m_pFileSystem->AddSearchPath( platform, "PLATFORM" );

	return true;
}

const char* CSteamAppSystemGroup::GetGameInfoPath() const {
	return m_pGameInfoPath;
}

CSysModule* CSteamAppSystemGroup::LoadModuleDLL( const char* pDLLName ) {
	return Sys_LoadModule( pDLLName );
}
