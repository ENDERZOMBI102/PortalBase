//
// Created by ENDERZOMBI102 on 31/08/2024.
//
#include "appframework/AppFramework.h"
#include "tier0/icommandline.h"
#include "filesystem_init.h"
#include "vstdlib/cvar.h"
// This must be the final include in a .cpp file!!!
#include "memdbgon.h"


namespace {
	void* s_AppInstance{ nullptr };
}

void* GetAppInstance() {
	return s_AppInstance;
}

void SetAppInstance( void* hInstance ) {
	s_AppInstance = hInstance;
}

int AppMain( void* hInstance, void* hPrevInstance, const char* lpCmdLine, int nCmdShow, CAppSystemGroup* pAppSystemGroup ) {
	CommandLine()->CreateCmdLine( lpCmdLine );
	s_AppInstance = hInstance;
	return pAppSystemGroup->Run();
}
int AppMain( const int argc, char** argv, CAppSystemGroup* pAppSystemGroup ) {
	CommandLine()->CreateCmdLine( argc, argv );
	return pAppSystemGroup->Run();
}

int AppStartup( void* hInstance, void* hPrevInstance, const char* lpCmdLine, int nCmdShow, CAppSystemGroup* pAppSystemGroup ) {
	CommandLine()->CreateCmdLine( lpCmdLine );
	s_AppInstance = hInstance;
	return pAppSystemGroup->Startup();
}
int AppStartup( const int argc, char** argv, CAppSystemGroup* pAppSystemGroup ) {
	CommandLine()->CreateCmdLine( argc, argv );
	return pAppSystemGroup->Startup();
}
void AppShutdown( CAppSystemGroup* pAppSystemGroup ) {
	pAppSystemGroup->Shutdown();
}


// --- CSteamApplication ---
CSteamApplication::CSteamApplication( CSteamAppSystemGroup* pAppSystemGroup )
	: m_pChildAppSystemGroup{ pAppSystemGroup } { }

int CSteamApplication::Startup() {
	return m_pChildAppSystemGroup->Startup();
}
void CSteamApplication::Shutdown() {
	m_pChildAppSystemGroup->Shutdown();
}

// CSteamApplication - IAppSystem
bool CSteamApplication::Create() {
	// load ICVar/cvar factory
	if ( not AddSystem( LoadModule( VStdLib_GetICVarFactory() ), CVAR_INTERFACE_VERSION ) ) {
		return false;
	}

	// load the fs module
	char fsDllName[1024];
	FileSystem_SetErrorMode( FSErrorMode_t::FS_ERRORMODE_AUTO );
	const auto res{ FileSystem_GetFileSystemDLLName( fsDllName, 1024, m_bSteam ) };
	if ( res != FSReturnCode_t::FS_OK ) {
		const char* error;
		switch ( res ) {
			case FS_MISSING_GAMEINFO_FILE: error = "MISSING_GAMEINFO_FILE"; break;
			case FS_INVALID_GAMEINFO_FILE: error = "INVALID_GAMEINFO_FILE"; break;
			case FS_INVALID_PARAMETERS: error = "INVALID_PARAMETERS"; break;
			case FS_UNABLE_TO_INIT: error = "UNABLE_TO_INIT"; break;
			case FS_MISSING_STEAM_DLL: error = "MISSING_STEAM_DLL"; break;
			default: error = "N/A"; break;
		}
		Warning( "Failed to find filesystem module. (%s)\n", error );
		return false;
	}
	m_pFileSystem = dynamic_cast<IFileSystem*>( AddSystem( LoadModule( fsDllName ), FILESYSTEM_INTERFACE_VERSION ) );
	if ( not m_pFileSystem ) {
		return false;
	}

	g_pFullFileSystem = m_pFileSystem;
	// give the fs module to child group, so it can play with it
	m_pChildAppSystemGroup->Setup( m_pFileSystem, this );

	return m_pChildAppSystemGroup->Create();
}
bool CSteamApplication::PreInit() {
	return m_pChildAppSystemGroup->PreInit();
}
int CSteamApplication::Main() {
	return m_pChildAppSystemGroup->Main();
}
void CSteamApplication::PostShutdown() {
	m_pChildAppSystemGroup->PostShutdown();
}
void CSteamApplication::Destroy() {
	m_pChildAppSystemGroup->Destroy();
}
