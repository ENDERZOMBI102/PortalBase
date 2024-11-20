//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Expose things from GameInterface.cpp. Mostly the engine interfaces.
//
// $NoKeywords: $
//=============================================================================//
#pragma once
#include "mapentities.h"

class IReplayFactory;

extern INetworkStringTable* g_pStringTableInfoPanel;
extern INetworkStringTable* g_pStringTableServerMapCycle;

#ifdef TF_DLL
extern INetworkStringTable* g_pStringTableServerPopFiles;
#endif

// Player / Client related functions
// Most of this is implemented in gameinterface.cpp, but some of it is per-mod in files like cs_gameinterface.cpp, etc.
class CServerGameClients : public IServerGameClients {
public:
	bool ClientConnect( edict_t* pEntity, char const* pszName, char const* pszAddress, char* reject, int maxrejectlen ) override;
	void ClientActive( edict_t* pEntity, bool bLoadGame ) override;
	void ClientDisconnect( edict_t* pEntity ) override;
	void ClientPutInServer( edict_t* pEntity, const char* playername ) override;
	void ClientCommand( edict_t* pEntity, const CCommand& args ) override;
	void ClientSettingsChanged( edict_t* pEntity ) override;
	void ClientSetupVisibility( edict_t* pViewEntity, edict_t* pClient, unsigned char* pvs, int pvssize ) override;
	float ProcessUsercmds( edict_t* player, bf_read* buf, int numcmds, int totalcmds,
								   int dropped_packets, bool ignore, bool paused ) override;
	// Player is running a command
	void PostClientMessagesSent_DEPRECIATED() override;
	void SetCommandClient( int index ) override;
	CPlayerState* GetPlayerState( edict_t* player ) override;
	void ClientEarPosition( edict_t* pEntity, Vector* pEarOrigin ) override;

	void GetPlayerLimits( int& minplayers, int& maxplayers, int& defaultMaxPlayers ) const override;

	// returns number of delay ticks if player is in Replay mode (0 = no delay)
	int GetReplayDelay( edict_t* player, int& entity ) override;
	// Anything this game .dll wants to add to the bug reporter text (e.g., the entity/model under the picker crosshair)
	//  can be added here
	void GetBugReportInfo( char* buf, int buflen ) override;
	void NetworkIDValidated( const char* pszUserName, const char* pszNetworkID ) override;

	// The client has submitted a keyvalues command
	void ClientCommandKeyValues( edict_t* pEntity, KeyValues* pKeyValues ) override;

	// Notify that the player is spawned
	void ClientSpawned( edict_t* pPlayer ) override;
};


class CServerGameDLL : public IServerGameDLL {
public:
	bool DLLInit( CreateInterfaceFn engineFactory, CreateInterfaceFn physicsFactory,
						  CreateInterfaceFn fileSystemFactory, CGlobalVars* pGlobals ) override;
	void DLLShutdown() override;
	// Get the simulation interval (must be compiled with identical values into both client and game .dll for MOD!!!)
	bool ReplayInit( CreateInterfaceFn fnReplayFactory ) override;
	float GetTickInterval() const override;
	bool GameInit() override;
	void GameShutdown() override;
	bool LevelInit( const char* pMapName, char const* pMapEntities, char const* pOldLevel, char const* pLandmarkName, bool loadGame, bool background ) override;
	void ServerActivate( edict_t* pEdictList, int edictCount, int clientMax ) override;
	void LevelShutdown() override;
	void GameFrame( bool simulating ) override;      // could be called multiple times before sending data to clients
	void PreClientUpdate( bool simulating ) override;// called after all GameFrame() calls, before sending data to clients

	ServerClass* GetAllServerClasses() override;
	const char* GetGameDescription() override;
	void CreateNetworkStringTables() override;

	// Save/restore system hooks
	CSaveRestoreData* SaveInit( int size ) override;
	void SaveWriteFields( CSaveRestoreData*, char const*, void*, datamap_t*, typedescription_t*, int ) override;
	void SaveReadFields( CSaveRestoreData*, char const*, void*, datamap_t*, typedescription_t*, int ) override;
	void SaveGlobalState( CSaveRestoreData* ) override;
	void RestoreGlobalState( CSaveRestoreData* ) override;
	int CreateEntityTransitionList( CSaveRestoreData*, int ) override;
	void BuildAdjacentMapList() override;

	void PreSave( CSaveRestoreData* ) override;
	void Save( CSaveRestoreData* ) override;
	void GetSaveComment( char* comment, int maxlength, float flMinutes, float flSeconds, bool bNoTime = false ) override;
	void WriteSaveHeaders( CSaveRestoreData* ) override;

	void ReadRestoreHeaders( CSaveRestoreData* ) override;
	void Restore( CSaveRestoreData*, bool ) override;
	bool IsRestoring() override;

	// Retrieve info needed for parsing the specified user message
	bool GetUserMessageInfo( int msg_type, char* name, int maxnamelength, int& size ) override;

	CStandardSendProxies* GetStandardSendProxies() override;

	void PostInit() override;
	void Think( bool finalTick ) override;

	void OnQueryCvarValueFinished( QueryCvarCookie_t iCookie, edict_t* pPlayerEntity, EQueryCvarValueStatus eStatus, const char* pCvarName, const char* pCvarValue ) override;

	void PreSaveGameLoaded( char const* pSaveName, bool bInGame ) override;

	// Returns true if the game DLL wants the server not to be made public.
	// Used by commentary system to hide multiplayer commentary servers from the master.
	bool ShouldHideServer() override;

	void InvalidateMdlCache() override;

	void SetServerHibernation( bool bHibernating ) override;

	float m_fAutoSaveDangerousTime;
	float m_fAutoSaveDangerousMinHealthToCommit;
	bool m_bIsHibernating;

	// Called after the steam API has been activated post-level startup
	void GameServerSteamAPIActivated() override;

	// Called after the steam API has been shutdown post-level startup
	void GameServerSteamAPIShutdown() override;

	// interface to the new GC based lobby system
	IServerGCLobby* GetServerGCLobby() override;

	const char* GetServerBrowserMapOverride() override;
	const char* GetServerBrowserGameData() override;

	// Called to add output to the status command
	void Status( void ( *print )( const char* fmt, ... ) ) override;

	void PrepareLevelResources( /* in/out */ char* pszMapName, size_t nMapNameSize,
										/* in/out */ char* pszMapFile, size_t nMapFileSize ) override;

	ePrepareLevelResourcesResult AsyncPrepareLevelResources( /* in/out */ char* pszMapName, size_t nMapNameSize,
																	 /* in/out */ char* pszMapFile, size_t nMapFileSize,
																	 float* flProgress = NULL ) override;

	eCanProvideLevelResult CanProvideLevel( /* in/out */ char* pMapName, int nMapNameMax ) override;

	// Called to see if the game server is okay with a manual changelevel or map command
	bool IsManualMapChangeOkay( const char** pszReason ) override;

private:
	// This can just be a wrapper on MapEntity_ParseAllEntities, but CS does some tricks in here
	// with the entity list.
	void LevelInit_ParseAllEntities( const char* pMapEntities );
	void LoadMessageOfTheDay();
	void LoadSpecificMOTDMsg( const ConVar& convar, const char* pszStringName );
};


// Normally, when the engine calls ClientPutInServer, it calls a global function in the game DLL
// by the same name. Use this to override the function that it calls. This is used for bots.
typedef CBasePlayer* ( *ClientPutInServerOverrideFn )( edict_t* pEdict, const char* playername );

void ClientPutInServerOverride( ClientPutInServerOverrideFn fn );

// -------------------------------------------------------------------------------------------- //
// Entity list management stuff.
// -------------------------------------------------------------------------------------------- //
// These are created for map entities in order as the map entities are spawned.
class CMapEntityRef {
public:
	int m_iEdict;       // Which edict slot this entity got. -1 if CreateEntityByName failed.
	int m_iSerialNumber;// The edict serial number. TODO used anywhere ?
};

extern CUtlLinkedList<CMapEntityRef, unsigned short> g_MapEntityRefs;

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class CMapLoadEntityFilter : public IMapEntityFilter {
public:
	bool ShouldCreateEntity( const char* pClassname ) override {
		// During map load, create all the entities.
		return true;
	}

	CBaseEntity* CreateNextEntity( const char* pClassname ) override {
		CBaseEntity* pRet = CreateEntityByName( pClassname );

		CMapEntityRef ref;
		ref.m_iEdict = -1;
		ref.m_iSerialNumber = -1;

		if ( pRet ) {
			ref.m_iEdict = pRet->entindex();
			if ( pRet->edict() )
				ref.m_iSerialNumber = pRet->edict()->m_NetworkSerialNumber;
		}

		g_MapEntityRefs.AddToTail( ref );
		return pRet;
	}
};

bool IsEngineThreaded();

class CServerGameTags : public IServerGameTags {
public:
	void GetTaggedConVarList( KeyValues* pCvarTagList ) override;
};
EXPOSE_SINGLE_INTERFACE( CServerGameTags, IServerGameTags, INTERFACEVERSION_SERVERGAMETAGS );
