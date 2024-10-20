//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//===========================================================================//
#include "IGameUIFuncs.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"
#include "VGuiMatSurface/IMatSystemSurface.h"
#include "appframework/IAppSystemGroup.h"
#include "c_basetempentity.h"
#include "c_rope.h"
#include "c_soundscape.h"
#include "c_te_legacytempents.h"
#include "c_user_message_register.h"
#include "c_vguiscreen.h"
#include "c_world.h"
#include "cbase.h"
#include "client_factorylist.h"
#include "clienteffectprecachesystem.h"
#include "clientmode.h"
#include "clientsideeffects.h"
#include "colorcorrectionmgr.h"
#include "datacache/idatacache.h"
#include "datacache/imdlcache.h"
#include "detailobjectsystem.h"
#include "engine/IEngineTrace.h"
#include "engine/IStaticPropMgr.h"
#include "engine/ishadowmgr.h"
#include "engine/ivdebugoverlay.h"
#include "engine/ivmodelinfo.h"
#include "enginesprite.h"
#include "env_wind_shared.h"
#include "filesystem.h"
#include "game/client/IGameClientExports.h"
#include "gamerules_register.h"
#include "gamestringpool.h"
#include "hltvcamera.h"
#include "hud_basechat.h"
#include "hud_closecaption.h"
#include "hud_crosshair.h"
#include "iclientmode.h"
#include "iefx.h"
#include "ienginevgui.h"
#include "igameevents.h"
#include "ihudlcd.h"
#include "iinput.h"
#include "imessagechars.h"
#include "initializer.h"
#include "inputsystem/iinputsystem.h"
#include "ivieweffects.h"
#include "iviewrender.h"
#include "iviewrender_beams.h"
#include "ivmodemanager.h"
#include "kbutton.h"
#include "materialsystem/imaterialsystemhardwareconfig.h"
#include "materialsystem/imaterialsystemstub.h"
#include "networkstringtable_clientdll.h"
#include "panelmetaclassmgr.h"
#include "particlemgr.h"
#include "perfvisualbenchmark.h"
#include "physics.h"
#include "physics_saverestore.h"
#include "physpropclientside.h"
#include "prediction.h"
#include "ragdoll_shared.h"
#include "rendertexture.h"
#include "saverestore.h"
#include "saverestoretypes.h"
#include "scenefilecache/ISceneFileCache.h"
#include "smoke_fog_overlay.h"
#include "soundenvelope.h"
#include "steam/steam_api.h"
#include "tier0/icommandline.h"
#include "tier0/vprof.h"
#include "tier2/tier2dm.h"
#include "tier3/tier3.h"
#include "toolframework_client.h"
#include "usermessages.h"
#include "vgui_controls/AnimationController.h"
#include "vgui_int.h"
#include "vguicenterprint.h"
#include "view.h"
#include "view_scene.h"
#include "view_shared.h"
#include "voice_status.h"
#include <crtmemdebug.h>
#if defined( REPLAY_ENABLED )
	#include "replay/replaycamera.h"
	#include "replay/replay_ragdoll.h"
	#include "qlimits.h"
	#include "replay/replay.h"
	#include "replay/ireplaysystem.h"
	#include "replay/iclientreplay.h"
	#include "replay/ienginereplay.h"
	#include "replay/ireplaymanager.h"
	#include "replay/ireplayscreenshotmanager.h"
	#include "replay/iclientreplaycontext.h"
	#include "replay/vgui/replayconfirmquitdlg.h"
	#include "replay/vgui/replaybrowsermainpanel.h"
	#include "replay/vgui/replayinputpanel.h"
	#include "replay/vgui/replayperformanceeditor.h"
#endif
#include "cdll_bounded_cvars.h"
#include "engine/imatchmaking.h"
#include "gamestats.h"
#include "ipresence.h"
#include "matsys_controls/matsyscontrols.h"
#include "particle_parse.h"
#include "vgui/ILocalize.h"
#include "vgui/IVGui.h"
#if defined( TF_CLIENT_DLL )
	#include "rtime.h"
	#include "tf_hud_disconnect_prompt.h"
	#include "../engine/audio/public/sound.h"
	#include "tf_shared_content_manager.h"
#endif
#include "client_virtualreality.h"
#include "clientsteamcontext.h"
#include "mouthinfo.h"
#include "mumble.h"
#include "renamed_recvtable_compat.h"
#include "sourcevr/isourcevirtualreality.h"

// NVNT includes
#include "con_nprint.h"
#include "haptics/haptic_msgs.h"
#include "haptics/haptic_utils.h"
#include "haptics/ihaptics.h"
#include "hud_macros.h"

#if defined( TF_CLIENT_DLL )
	#include "abuse_report.h"
#endif

#if defined( USES_ECON_ITEMS )
	#include "econ_item_system.h"
#endif

#if defined( TF_CLIENT_DLL )
	#include "econ/tool_items/custom_texture_cache.h"
#endif

#if defined( WORKSHOP_IMPORT_ENABLED )
	#include "fbxsystem/fbxsystem.h"
#endif

extern vgui::IInputInternal* g_InputInternal;

//=============================================================================
// HPE_BEGIN
// [dwenger] Necessary for stats display
//=============================================================================

#include "achievements_and_stats_interface.h"

//=============================================================================
// HPE_END
//=============================================================================


#if defined( PORTAL )
	#include "PortalRender.h"
#endif

#if defined( SIXENSE )
	#include "sixense/in_sixense.h"
#endif
#if IsWindows()
	#include "shaderapihack.hpp"
#endif
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


extern IClientMode* GetClientModeNormal();

// IF YOU ADD AN INTERFACE, EXTERN IT IN THE HEADER FILE.
IVEngineClient* engine = nullptr;
IVModelRender* modelrender = nullptr;
IVEfx* effects = nullptr;
IVRenderView* render = nullptr;
IVDebugOverlay* debugoverlay = nullptr;
IMaterialSystemStub* materials_stub = nullptr;
IDataCache* datacache = nullptr;
IVModelInfoClient* modelinfo = nullptr;
IEngineVGui* enginevgui = nullptr;
INetworkStringTableContainer* networkstringtable = nullptr;
ISpatialPartition* partition = nullptr;
IFileSystem* filesystem = nullptr;
IShadowMgr* shadowmgr = nullptr;
IStaticPropMgrClient* staticpropmgr = nullptr;
IEngineSound* enginesound = nullptr;
ISharedGameRules* sharedgamerules = nullptr;
IEngineTrace* enginetrace = nullptr;
IGameUIFuncs* gameuifuncs = nullptr;
IGameEventManager2* gameeventmanager = nullptr;
ISoundEmitterSystemBase* soundemitterbase = nullptr;
IInputSystem* inputsystem = nullptr;
ISceneFileCache* scenefilecache = nullptr;
IMatchmaking* matchmaking = nullptr;
IUploadGameStats* gamestatsuploader = nullptr;
IClientReplayContext* g_pClientReplayContext = nullptr;
#if defined( REPLAY_ENABLED )
	IReplayManager* g_pReplayManager = nullptr;
	IReplayMovieManager* g_pReplayMovieManager = nullptr;
	IReplayScreenshotManager* g_pReplayScreenshotManager = nullptr;
	IReplayPerformanceManager* g_pReplayPerformanceManager = nullptr;
	IReplayPerformanceController* g_pReplayPerformanceController = nullptr;
	IEngineReplay* g_pEngineReplay = nullptr;
	IEngineClientReplay* g_pEngineClientReplay = nullptr;
	IReplaySystem* g_pReplay = nullptr;
#endif

IHaptics* haptics = nullptr;  // NVNT haptics system interface singleton

//=============================================================================
// HPE_BEGIN
// [dwenger] Necessary for stats display
//=============================================================================

AchievementsAndStatsInterface* g_pAchievementsAndStatsInterface = nullptr;

//=============================================================================
// HPE_END
//=============================================================================

IGameSystem* SoundEmitterSystem();
IGameSystem* ToolFrameworkClientSystem();

// Engine player info, no game related infos here
BEGIN_BYTESWAP_DATADESC( player_info_s )
	DEFINE_ARRAY( name, FIELD_CHARACTER, MAX_PLAYER_NAME_LENGTH ),
	DEFINE_FIELD( userID, FIELD_INTEGER ),
	DEFINE_ARRAY( guid, FIELD_CHARACTER, SIGNED_GUID_LEN + 1 ),
	DEFINE_FIELD( friendsID, FIELD_INTEGER ),
	DEFINE_ARRAY( friendsName, FIELD_CHARACTER, MAX_PLAYER_NAME_LENGTH ),
	DEFINE_FIELD( fakeplayer, FIELD_BOOLEAN ),
	DEFINE_FIELD( ishltv, FIELD_BOOLEAN ),
	#if defined( REPLAY_ENABLED )
		DEFINE_FIELD( isreplay, FIELD_BOOLEAN ),
	#endif
	DEFINE_ARRAY( customFiles, FIELD_INTEGER, MAX_CUSTOM_FILES ),
	DEFINE_FIELD( filesDownloaded, FIELD_INTEGER ),
END_BYTESWAP_DATADESC()

static bool g_bRequestCacheUsedMaterials = false;
void RequestCacheUsedMaterials() {
	g_bRequestCacheUsedMaterials = true;
}

void ProcessCacheUsedMaterials() {
	if ( !g_bRequestCacheUsedMaterials ) {
		return;
	}

	g_bRequestCacheUsedMaterials = false;
	if ( materials ) {
		materials->CacheUsedMaterials();
	}
}

// String tables
INetworkStringTable* g_pStringTableParticleEffectNames = nullptr;
INetworkStringTable* g_StringTableEffectDispatch = nullptr;
INetworkStringTable* g_StringTableVguiScreen = nullptr;
INetworkStringTable* g_pStringTableMaterials = nullptr;
INetworkStringTable* g_pStringTableInfoPanel = nullptr;
INetworkStringTable* g_pStringTableClientSideChoreoScenes = nullptr;
INetworkStringTable* g_pStringTableServerMapCycle = nullptr;

#ifdef TF_CLIENT_DLL
	INetworkStringTable* g_pStringTableServerPopFiles = nullptr;
	INetworkStringTable* g_pStringTableServerMapCycleMvM = nullptr;
#endif

static CGlobalVarsBase dummyvars( true );
// So stuff that might reference gpGlobals during DLL initialization won't have a nullptr pointer.
// Once the engine calls Init on this DLL, this pointer gets assigned to the shared data in the engine
CGlobalVarsBase* gpGlobals = &dummyvars;
class CHudChat;
class CViewRender;
extern CViewRender g_DefaultViewRender;

extern void StopAllRumbleEffects();

static C_BaseEntityClassList* s_pClassLists = nullptr;
C_BaseEntityClassList::C_BaseEntityClassList() {
	m_pNextClassList = s_pClassLists;
	s_pClassLists = this;
}
C_BaseEntityClassList::~C_BaseEntityClassList() = default;

// Any entities that want an OnDataChanged during simulation register for it here.
class CDataChangedEvent {
public:
	CDataChangedEvent() = default;
	CDataChangedEvent( IClientNetworkable* ent, DataUpdateType_t updateType, int* pStoredEvent ) {
		m_pEntity = ent;
		m_UpdateType = updateType;
		m_pStoredEvent = pStoredEvent;
	}

	IClientNetworkable* m_pEntity{};
	DataUpdateType_t m_UpdateType{};
	int* m_pStoredEvent{};
};

ISaveRestoreBlockHandler* GetEntitySaveRestoreBlockHandler();
ISaveRestoreBlockHandler* GetViewEffectsRestoreBlockHandler();

CUtlLinkedList<CDataChangedEvent> g_DataChangedEvents;
ClientFrameStage_t g_CurFrameStage{ FRAME_UNDEFINED };


class IMoveHelper;

void DispatchHudText( const char* pszName );

[[maybe_unused]]
static ConVar s_CV_ShowParticleCounts( "showparticlecounts", "0", 0, "Display number of particles drawn per frame" );
[[maybe_unused]]
static ConVar s_cl_team( "cl_team", "default", FCVAR_USERINFO | FCVAR_ARCHIVE, "Default team when joining a game" );
[[maybe_unused]]
static ConVar s_cl_class( "cl_class", "default", FCVAR_USERINFO | FCVAR_ARCHIVE, "Default class when joining a game" );


// Physics system
bool g_bLevelInitialized;
bool g_bTextMode = false;
IClientPurchaseInterfaceV2* g_pClientPurchaseInterface = reinterpret_cast<class IClientPurchaseInterfaceV2*>( &g_bTextMode + 156 );

static ConVar* g_pcv_ThreadMode = nullptr;

//-----------------------------------------------------------------------------
// Purpose: interface for gameui to modify voice bans
//-----------------------------------------------------------------------------
class CGameClientExports : public IGameClientExports {
public:
	// ingame voice manipulation
	bool IsPlayerGameVoiceMuted( int playerIndex ) override {
		return GetClientVoiceMgr()->IsPlayerBlocked( playerIndex );
	}

	void MutePlayerGameVoice( int playerIndex ) override {
		GetClientVoiceMgr()->SetPlayerBlockedState( playerIndex, true );
	}

	void UnmutePlayerGameVoice( int playerIndex ) override {
		GetClientVoiceMgr()->SetPlayerBlockedState( playerIndex, false );
	}

	void OnGameUIActivated() override {
		IGameEvent* event = gameeventmanager->CreateEvent( "gameui_activated" );
		if ( event ) {
			gameeventmanager->FireEventClientSide( event );
		}
	}

	void OnGameUIHidden() override {
		IGameEvent* event = gameeventmanager->CreateEvent( "gameui_hidden" );
		if ( event ) {
			gameeventmanager->FireEventClientSide( event );
		}
	}

	//=============================================================================
	// HPE_BEGIN
	// [dwenger] Necessary for stats display
	//=============================================================================

	void CreateAchievementsPanel( vgui::Panel* pParent ) override {
		if ( g_pAchievementsAndStatsInterface ) {
			g_pAchievementsAndStatsInterface->CreatePanel( pParent );
		}
	}

	void DisplayAchievementPanel() override {
		if ( g_pAchievementsAndStatsInterface ) {
			g_pAchievementsAndStatsInterface->DisplayPanel();
		}
	}

	void ShutdownAchievementPanel() override {
		if ( g_pAchievementsAndStatsInterface ) {
			g_pAchievementsAndStatsInterface->ReleasePanel();
		}
	}

	[[nodiscard]]
	int32 GetAchievementsPanelMinWidth() const override {
		if ( g_pAchievementsAndStatsInterface ) {
			return g_pAchievementsAndStatsInterface->GetAchievementsPanelMinWidth();
		}

		return 0;
	}

	//=============================================================================
	// HPE_END
	//=============================================================================
	[[nodiscard]]
	const char* GetHolidayString() override {
		return UTIL_GetActiveHolidayString();
	}
};

EXPOSE_SINGLE_INTERFACE( CGameClientExports, IGameClientExports, GAMECLIENTEXPORTS_INTERFACE_VERSION );

class CClientDLLSharedAppSystems : public IClientDLLSharedAppSystems {
public:
	CClientDLLSharedAppSystems() {
		AddAppSystem( "soundemittersystem" DLL_EXT_STRING, SOUNDEMITTERSYSTEM_INTERFACE_VERSION );
		AddAppSystem( "scenefilecache" DLL_EXT_STRING, SCENE_FILE_CACHE_INTERFACE_VERSION );
	}

	int Count() override {
		return m_Systems.Count();
	}
	char const* GetDllName( int idx ) override {
		return m_Systems[idx].m_pModuleName;
	}
	char const* GetInterfaceName( int idx ) override {
		return m_Systems[idx].m_pInterfaceName;
	}

private:
	void AddAppSystem( char const* moduleName, char const* interfaceName ) {
		m_Systems.AddToTail({ .m_pModuleName = moduleName, .m_pInterfaceName = interfaceName });
	}

	CUtlVector<AppSystemInfo_t> m_Systems{};
};

EXPOSE_SINGLE_INTERFACE( CClientDLLSharedAppSystems, IClientDLLSharedAppSystems, CLIENT_DLL_SHARED_APPSYSTEMS );


//-----------------------------------------------------------------------------
// Helper interface for voice.
//-----------------------------------------------------------------------------
class CHLVoiceStatusHelper : public IVoiceStatusHelper {
public:
	void GetPlayerTextColor( int entindex, int color[3] ) override {
		color[0] = color[1] = color[2] = 128;
	}

	void UpdateCursorState() override { }

	bool CanShowSpeakerLabels() override {
		return true;
	}
};
static CHLVoiceStatusHelper g_VoiceStatusHelper;

//-----------------------------------------------------------------------------
// Code to display which entities are having their bones setup each frame.
//-----------------------------------------------------------------------------

ConVar cl_ShowBoneSetupEnts( "cl_ShowBoneSetupEnts", "0", 0, "Show which entities are having their bones setup each frame." );

class CBoneSetupEnt {
public:
	char m_ModelName[128];
	int m_Index;
	int m_Count;
};

bool BoneSetupCompare( const CBoneSetupEnt& a, const CBoneSetupEnt& b ) {
	return a.m_Index < b.m_Index;
}

CUtlRBTree<CBoneSetupEnt> g_BoneSetupEnts( BoneSetupCompare );


void TrackBoneSetupEnt( C_BaseAnimating* pEnt ) {
	#if IsDebug()
		if ( IsRetail() ) {
			return;
		}

		if ( !cl_ShowBoneSetupEnts.GetInt() ) {
			return;
		}

		CBoneSetupEnt ent;
		ent.m_Index = pEnt->entindex();
		unsigned short i = g_BoneSetupEnts.Find( ent );
		if ( i == g_BoneSetupEnts.InvalidIndex() ) {
			Q_strncpy( ent.m_ModelName, modelinfo->GetModelName( pEnt->GetModel() ), sizeof( ent.m_ModelName ) );
			ent.m_Count = 1;
			g_BoneSetupEnts.Insert( ent );
		} else {
			g_BoneSetupEnts[i].m_Count++;
		}
	#endif
}

void DisplayBoneSetupEnts() {
	#if IsDebug()
		if ( IsRetail() ) {
			return;
		}

		if ( !cl_ShowBoneSetupEnts.GetInt() ) {
			return;
		}

		unsigned short i;
		int nElements = 0;
		for ( i = g_BoneSetupEnts.FirstInorder(); i != g_BoneSetupEnts.LastInorder(); i = g_BoneSetupEnts.NextInorder( i ) ) {
			++nElements;
		}

		engine->Con_NPrintf( 0, "%d bone setup ents (name/count/entindex) ------------", nElements );

		con_nprint_s printInfo;
		printInfo.time_to_live = -1;
		printInfo.fixed_width_font = true;
		printInfo.color[0] = printInfo.color[1] = printInfo.color[2] = 1;

		printInfo.index = 2;
		for ( i = g_BoneSetupEnts.FirstInorder(); i != g_BoneSetupEnts.LastInorder(); i = g_BoneSetupEnts.NextInorder( i ) ) {
			CBoneSetupEnt* pEnt = &g_BoneSetupEnts[i];

			if ( pEnt->m_Count >= 3 ) {
				printInfo.color[0] = 1;
				printInfo.color[1] = 0;
				printInfo.color[2] = 0;
			} else if ( pEnt->m_Count == 2 ) {
				printInfo.color[0] = static_cast<float>( 200 ) / 255;
				printInfo.color[1] = static_cast<float>( 220 ) / 255;
				printInfo.color[2] = 0;
			} else {
				printInfo.color[0] = 1;
				printInfo.color[1] = 1;
				printInfo.color[2] = 1;
			}
			engine->Con_NXPrintf( &printInfo, "%25s / %3d / %3d", pEnt->m_ModelName, pEnt->m_Count, pEnt->m_Index );
			printInfo.index++;
		}

		g_BoneSetupEnts.RemoveAll();
	#endif
}

//-----------------------------------------------------------------------------
// Purpose: engine to client .dll interface
//-----------------------------------------------------------------------------
class CHLClient : public IBaseClientDLL {
public:
	CHLClient();

	int Init( CreateInterfaceFn appSystemFactory, CreateInterfaceFn physicsFactory, CGlobalVarsBase* pGlobals ) override;

	void PostInit() override;
	void Shutdown() override;

	bool ReplayInit( CreateInterfaceFn fnReplayFactory ) override;
	bool ReplayPostInit() override;

	void LevelInitPreEntity( const char* pMapName ) override;
	void LevelInitPostEntity() override;
	void LevelShutdown() override;

	ClientClass* GetAllClasses() override;

	int HudVidInit() override;
	void HudProcessInput( bool bActive ) override;
	void HudUpdate( bool bActive ) override;
	void HudReset() override;
	void HudText( const char* message ) override;

	// Mouse Input Interfaces
	void IN_ActivateMouse() override;
	void IN_DeactivateMouse() override;
	void IN_Accumulate() override;
	void IN_ClearStates() override;
	bool IN_IsKeyDown( const char* name, bool& isdown ) override;
	void IN_OnMouseWheeled( int nDelta ) override;
	// Raw signal
	int IN_KeyEvent( int eventcode, ButtonCode_t keynum, const char* pszCurrentBinding ) override;
	void IN_SetSampleTime( float frametime ) override;
	// Create movement command
	void CreateMove( int sequence_number, float input_sample_frametime, bool active ) override;
	void ExtraMouseSample( float frametime, bool active ) override;
	bool WriteUsercmdDeltaToBuffer( bf_write* buf, int from, int to, bool isnewcommand ) override;
	void EncodeUserCmdToBuffer( bf_write& buf, int slot ) override;
	void DecodeUserCmdFromBuffer( bf_read& buf, int slot ) override;


	void View_Render( vrect_t* rect ) override;
	void RenderView( const CViewSetup& view, int nClearFlags, int whatToDraw ) override;
	void View_Fade( ScreenFade_t* pSF ) override;

	void SetCrosshairAngle( const QAngle& angle ) override;

	void InitSprite( CEngineSprite* pSprite, const char* loadname ) override;
	void ShutdownSprite( CEngineSprite* pSprite ) override;

	[[nodiscard]]
	int GetSpriteSize() const override;

	void VoiceStatus( int entindex, qboolean bTalking ) override;

	void InstallStringTableCallback( const char* tableName ) override;

	void FrameStageNotify( ClientFrameStage_t curStage ) override;

	bool DispatchUserMessage( int msg_type, bf_read& msg_data ) override;

	// Save/restore system hooks
	CSaveRestoreData* SaveInit( int size ) override;
	void SaveWriteFields( CSaveRestoreData*, const char*, void*, datamap_t*, typedescription_t*, int ) override;
	void SaveReadFields( CSaveRestoreData*, const char*, void*, datamap_t*, typedescription_t*, int ) override;
	void PreSave( CSaveRestoreData* ) override;
	void Save( CSaveRestoreData* ) override;
	void WriteSaveHeaders( CSaveRestoreData* ) override;
	void ReadRestoreHeaders( CSaveRestoreData* ) override;
	void Restore( CSaveRestoreData*, bool ) override;
	void DispatchOnRestore() override;
	void WriteSaveGameScreenshot( const char* pFilename ) override;

	// Given a list of "S(wavname) S(wavname2)" tokens, look up the localized text and emit
	//  the appropriate close caption if running with closecaption = 1
	void EmitSentenceCloseCaption( char const* tokenstream ) override;
	void EmitCloseCaption( char const* captionname, float duration ) override;

	CStandardRecvProxies* GetStandardRecvProxies() override;

	bool CanRecordDemo( char* errorMsg, int length ) const override;

	void OnDemoRecordStart( char const* pDemoBaseName ) override;
	void OnDemoRecordStop() override;
	void OnDemoPlaybackStart( char const* pDemoBaseName ) override;
	void OnDemoPlaybackStop() override;

	bool ShouldDrawDropdownConsole() override;

	// Get client screen dimensions
	int GetScreenWidth() override;
	int GetScreenHeight() override;

	// save game screenshot writing
	void WriteSaveGameScreenshotOfSize( const char* pFilename, int width, int height, bool bCreatePowerOf2Padded /*=false*/, bool bWriteVTF /*=false*/ ) override;

	// Gets the location of the player viewpoint
	bool GetPlayerView( CViewSetup& playerView ) override;

	// Matchmaking
	void SetupGameProperties( CUtlVector<XUSER_CONTEXT>& contexts, CUtlVector<XUSER_PROPERTY>& properties ) override;
	uint GetPresenceID( const char* pIDName ) override;
	const char* GetPropertyIdString( uint id ) override;
	void GetPropertyDisplayString( uint id, uint value, char* pOutput, int nBytes ) override;
	void StartStatsReporting( HANDLE handle, bool bArbitrated ) override;

	void InvalidateMdlCache() override;

	void ReloadFilesInList( IFileList* pFilesToReload ) override;

	// Let the client handle UI toggle - if this function returns false, the UI will toggle, otherwise it will not.
	bool HandleUiToggle() override;

	// Allow the console to be shown?
	bool ShouldAllowConsole() override;

	// Get renamed recv tables
	CRenamedRecvTableInfo* GetRenamedRecvTableInfos() override;

	// Get the mouthinfo for the sound being played inside UI panels
	CMouthInfo* GetClientUIMouthInfo() override;

	// Notify the client that a file has been received from the game server
	void FileReceived( const char* fileName, unsigned int transferID ) override;

	const char* TranslateEffectForVisionFilter( const char* pchEffectType, const char* pchEffectName ) override;

	void ClientAdjustStartSoundParams( struct StartSoundParams_t& params ) override;

	// Returns true if the disconnect command has been handled by the client
	bool DisconnectAttempt() override;

public:
	void PrecacheMaterial( const char* pMaterialName );

	bool IsConnectedUserInfoChangeAllowed( IConVar* pCvar ) override;

private:
	void UncacheAllMaterials();
	void ResetStringTablePointers();

	CUtlVector<IMaterial*> m_CachedMaterials;
};


CHLClient gHLClient;
IBaseClientDLL* clientdll = &gHLClient;

EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CHLClient, IBaseClientDLL, CLIENT_DLL_INTERFACE_VERSION, gHLClient );


//-----------------------------------------------------------------------------
// Precaches a material
//-----------------------------------------------------------------------------
void PrecacheMaterial( const char* pMaterialName ) {
	gHLClient.PrecacheMaterial( pMaterialName );
}

//-----------------------------------------------------------------------------
// Converts a previously precached material into an index
//-----------------------------------------------------------------------------
int GetMaterialIndex( const char* pMaterialName ) {
	if ( pMaterialName ) {
		int nIndex = g_pStringTableMaterials->FindStringIndex( pMaterialName );
		Assert( nIndex >= 0 );
		if ( nIndex >= 0 ) {
			return nIndex;
		}
	}

	// This is the invalid string index
	return 0;
}

//-----------------------------------------------------------------------------
// Converts precached material indices into strings
//-----------------------------------------------------------------------------
const char* GetMaterialNameFromIndex( int nIndex ) {
	if ( nIndex != g_pStringTableMaterials->GetMaxStrings() - 1 ) {
		return g_pStringTableMaterials->GetString( nIndex );
	}
	return nullptr;
}


//-----------------------------------------------------------------------------
// Precaches a particle system
//-----------------------------------------------------------------------------
void PrecacheParticleSystem( const char* pParticleSystemName ) {
	g_pStringTableParticleEffectNames->AddString( false, pParticleSystemName );
	g_pParticleSystemMgr->PrecacheParticleSystem( pParticleSystemName );
}


//-----------------------------------------------------------------------------
// Converts a previously precached particle system into an index
//-----------------------------------------------------------------------------
int GetParticleSystemIndex( const char* pParticleSystemName ) {
	if ( pParticleSystemName ) {
		int nIndex = g_pStringTableParticleEffectNames->FindStringIndex( pParticleSystemName );
		if ( nIndex != INVALID_STRING_INDEX ) {
			return nIndex;
		}
		DevWarning( "Client: Missing precache for particle system \"%s\"!\n", pParticleSystemName );
	}

	// This is the invalid string index
	return 0;
}

//-----------------------------------------------------------------------------
// Converts precached particle system indices into strings
//-----------------------------------------------------------------------------
const char* GetParticleSystemNameFromIndex( int nIndex ) {
	if ( nIndex < g_pStringTableParticleEffectNames->GetMaxStrings() ) {
		return g_pStringTableParticleEffectNames->GetString( nIndex );
	}
	return "error";
}

//-----------------------------------------------------------------------------
// Returns true if host_thread_mode is set to non-zero (and engine is running in threaded mode)
//-----------------------------------------------------------------------------
bool IsEngineThreaded() {
	if ( g_pcv_ThreadMode ) {
		return g_pcv_ThreadMode->GetBool();
	}
	return false;
}

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------

CHLClient::CHLClient() {
	// Kinda bogus, but the logic in the engine is too convoluted to put it there
	g_bLevelInitialized = false;
}


extern IGameSystem* ViewportClientSystem();


//-----------------------------------------------------------------------------
ISourceVirtualReality* g_pSourceVR = nullptr;

// Purpose: Called when the DLL is first loaded.
// Input  : engineFactory -
// Output : int
//-----------------------------------------------------------------------------
int CHLClient::Init( CreateInterfaceFn appSystemFactory, CreateInterfaceFn physicsFactory, CGlobalVarsBase* pGlobals ) {
	InitCRTMemDebug();
	MathLib_Init( 2.2f, 2.2f, 0.0f, 2.0f );


	#if defined( SIXENSE )
		g_pSixenseInput = new SixenseInput;
	#endif

	// Hook up global variables
	gpGlobals = pGlobals;

	ConnectTier1Libraries( &appSystemFactory, 1 );
	ConnectTier2Libraries( &appSystemFactory, 1 );
	ConnectTier3Libraries( &appSystemFactory, 1 );

	#if !defined( NO_STEAM )
		ClientSteamContext().Activate();
	#endif

	// We aren't happy unless we get all of our interfaces.
	// please don't collapse this into one monolithic boolean expression (impossible to debug)
	if ( ( engine = static_cast<IVEngineClient*>( appSystemFactory( VENGINE_CLIENT_INTERFACE_VERSION, nullptr ) ) ) == nullptr ) {
		return false;
	}
	if ( ( modelrender = static_cast<IVModelRender*>( appSystemFactory( VENGINE_HUDMODEL_INTERFACE_VERSION, nullptr ) ) ) == nullptr ) {
		return false;
	}
	if ( ( effects = static_cast<IVEfx*>( appSystemFactory( VENGINE_EFFECTS_INTERFACE_VERSION, nullptr ) ) ) == nullptr ) {
		return false;
	}
	if ( ( enginetrace = static_cast<IEngineTrace*>( appSystemFactory( INTERFACEVERSION_ENGINETRACE_CLIENT, nullptr ) ) ) == nullptr ) {
		return false;
	}
	if ( ( render = static_cast<IVRenderView*>( appSystemFactory( VENGINE_RENDERVIEW_INTERFACE_VERSION, nullptr ) ) ) == nullptr ) {
		return false;
	}
	if ( ( debugoverlay = static_cast<IVDebugOverlay*>( appSystemFactory( VDEBUG_OVERLAY_INTERFACE_VERSION, nullptr ) ) ) == nullptr ){
		return false;
	}
	if ( ( datacache = static_cast<IDataCache*>( appSystemFactory( DATACACHE_INTERFACE_VERSION, nullptr ) ) ) == nullptr ){
		return false;
	}
	if ( !mdlcache ) {
		return false;
	}
	if ( ( modelinfo = static_cast<IVModelInfoClient*>( appSystemFactory( VMODELINFO_CLIENT_INTERFACE_VERSION, nullptr ) ) ) == nullptr ){
		return false;
	}
	if ( ( enginevgui = static_cast<IEngineVGui*>( appSystemFactory( VENGINE_VGUI_VERSION, nullptr ) ) ) == nullptr ){
		return false;
	}
	if ( ( networkstringtable = static_cast<INetworkStringTableContainer*>( appSystemFactory( INTERFACENAME_NETWORKSTRINGTABLECLIENT, nullptr ) ) ) == nullptr ){
		return false;
	}
	if ( ( partition = static_cast<ISpatialPartition*>( appSystemFactory( INTERFACEVERSION_SPATIALPARTITION, nullptr ) ) ) == nullptr ){
		return false;
	}
	if ( ( shadowmgr = static_cast<IShadowMgr*>( appSystemFactory( ENGINE_SHADOWMGR_INTERFACE_VERSION, nullptr ) ) ) == nullptr ){
		return false;
	}
	if ( ( staticpropmgr = static_cast<IStaticPropMgrClient*>( appSystemFactory( INTERFACEVERSION_STATICPROPMGR_CLIENT, nullptr ) ) ) == nullptr ){
		return false;
	}
	if ( ( enginesound = static_cast<IEngineSound*>( appSystemFactory( IENGINESOUND_CLIENT_INTERFACE_VERSION, nullptr ) ) ) == nullptr ){
		return false;
	}
	if ( ( filesystem = static_cast<IFileSystem*>( appSystemFactory( FILESYSTEM_INTERFACE_VERSION, nullptr ) ) ) == nullptr ){
		return false;
	}
	if ( ( gameuifuncs = static_cast<IGameUIFuncs*>( appSystemFactory( VENGINE_GAMEUIFUNCS_VERSION, nullptr ) ) ) == nullptr ){
		return false;
	}
	if ( ( gameeventmanager = static_cast<IGameEventManager2*>( appSystemFactory( INTERFACEVERSION_GAMEEVENTSMANAGER2, nullptr ) ) ) == nullptr ){
		return false;
	}
	if ( ( soundemitterbase = static_cast<ISoundEmitterSystemBase*>( appSystemFactory( SOUNDEMITTERSYSTEM_INTERFACE_VERSION, nullptr ) ) ) == nullptr ){
		return false;
	}
	if ( ( inputsystem = static_cast<IInputSystem*>( appSystemFactory( INPUTSYSTEM_INTERFACE_VERSION, nullptr ) ) ) == nullptr ){
		return false;
	}
	if ( ( scenefilecache = static_cast<ISceneFileCache*>( appSystemFactory( SCENE_FILE_CACHE_INTERFACE_VERSION, nullptr ) ) ) == nullptr ){
		return false;
	}
	if ( ( gamestatsuploader = static_cast<IUploadGameStats*>( appSystemFactory( INTERFACEVERSION_UPLOADGAMESTATS, nullptr ) ) ) == nullptr ){
		return false;
	}

	#if defined( REPLAY_ENABLED )
		if ( IsPC() && ( g_pEngineReplay = static_cast<IEngineReplay*>( appSystemFactory( ENGINE_REPLAY_INTERFACE_VERSION, nullptr ) ) ) == nullptr ){
			return false;
		}
		if ( IsPC() && ( g_pEngineClientReplay = static_cast<IEngineClientReplay*>( appSystemFactory( ENGINE_REPLAY_CLIENT_INTERFACE_VERSION, nullptr ) ) ) == nullptr ){
			return false;
		}
	#endif

	if ( !g_pMatSystemSurface ){
		return false;
	}

	#if defined( WORKSHOP_IMPORT_ENABLED )
		if ( !ConnectDataModel( appSystemFactory ) ){
			return false;
		}
		if ( InitDataModel() != INIT_OK ) {
			return false;
		}
		InitFbx();
	#endif

	// it's ok if this is nullptr. That just means the sourcevr.dll wasn't found
	g_pSourceVR = static_cast<ISourceVirtualReality*>( appSystemFactory( SOURCE_VIRTUAL_REALITY_INTERFACE_VERSION, nullptr ) );

	FactoryList_Store({ .appSystemFactory = appSystemFactory, .physicsFactory = physicsFactory });

	// Yes, both the client and game .dlls will try to Connect, the soundemittersystem.dll will handle this gracefully
	if ( !soundemitterbase->Connect( appSystemFactory ) ) {
		return false;
	}

	if ( CommandLine()->FindParm( "-textmode" ) ) {
		g_bTextMode = true;
	}

	if ( CommandLine()->FindParm( "-makedevshots" ) ) {
		g_MakingDevShots = true;
	}

	// Not fatal if the material system stub isn't around.
	materials_stub = static_cast<IMaterialSystemStub*>( appSystemFactory( MATERIAL_SYSTEM_STUB_INTERFACE_VERSION, nullptr ) );

	if ( !g_pMaterialSystemHardwareConfig ) {
		return false;
	}

	// Initialize the console variables.
	ConVar_Register( FCVAR_CLIENTDLL );

	g_pcv_ThreadMode = g_pCVar->FindVar( "host_thread_mode" );

	if ( !Initializer::InitializeAllObjects() ) {
		return false;
	}

	if ( !ParticleMgr()->Init( MAX_TOTAL_PARTICLES, materials ) ) {
		return false;
	}


	if ( !VGui_Startup( appSystemFactory ) ) {
		return false;
	}

	vgui::VGui_InitMatSysInterfacesList( "ClientDLL", &appSystemFactory, 1 );

	// Add the client systems.

	// Client Leaf System has to be initialized first, since DetailObjectSystem uses it
	IGameSystem::Add( GameStringSystem() );
	IGameSystem::Add( SoundEmitterSystem() );
	IGameSystem::Add( ToolFrameworkClientSystem() );
	IGameSystem::Add( ClientLeafSystem() );
	IGameSystem::Add( DetailObjectSystem() );
	IGameSystem::Add( ViewportClientSystem() );
	IGameSystem::Add( ClientEffectPrecacheSystem() );
	IGameSystem::Add( g_pClientShadowMgr );
	IGameSystem::Add( g_pColorCorrectionMgr );  // NOTE: This must happen prior to ClientThinkList (color correction is updated there)
	IGameSystem::Add( ClientThinkList() );
	IGameSystem::Add( ClientSoundscapeSystem() );
	IGameSystem::Add( PerfVisualBenchmark() );
	IGameSystem::Add( MumbleSystem() );

	#if IsWindows()
		// MaterialSystem hacky code to increase pixel constants
		CMaterialConfigWrapper wrapper;

		Msg( "Applying shader-api hack!\n" );
		Msg( "Before hack:\n" );
		wrapper.PrintPixelConstants();
		wrapper.SetNumPixelConstants( 225 );
		wrapper.SetNumBooleanPixelConstants( 225 );
		wrapper.SetNumIntegerPixelConstants( 225 );
		Msg( "After hack:\n" );
		wrapper.PrintPixelConstants();
	#endif

	#if defined( TF_CLIENT_DLL )
		IGameSystem::Add( CustomTextureToolCacheGameSystem() );
		IGameSystem::Add( TFSharedContentManager() );
	#endif

	#if defined( TF_CLIENT_DLL )
		if ( g_AbuseReportMgr != nullptr ) {
			IGameSystem::Add( g_AbuseReportMgr );
		}
	#endif

	#if defined( CLIENT_DLL ) && defined( COPY_CHECK_STRESSTEST )
		IGameSystem::Add( GetPredictionCopyTester() );
	#endif

	modemanager->Init();

	g_pClientMode->InitViewport();

	gHUD.Init();

	g_pClientMode->Init();

	if ( !IGameSystem::InitAllSystems() ) {
		return false;
	}

	g_pClientMode->Enable();

	if ( !view ) {
		view = reinterpret_cast<IViewRender*>( &g_DefaultViewRender );
	}

	view->Init();
	vieweffects->Init();

	C_BaseTempEntity::PrecacheTempEnts();

	input->Init_All();

	VGui_CreateGlobalPanels();

	InitSmokeFogOverlay();

	// Register user messages..
	CUserMessageRegister::RegisterAll();

	ClientVoiceMgr_Init();

	// Embed voice status icons inside chat element
	{
		vgui::VPANEL parent = enginevgui->GetPanel( PANEL_CLIENTDLL );
		GetClientVoiceMgr()->Init( &g_VoiceStatusHelper, parent );
	}

	if ( !PhysicsDLLInit( physicsFactory ) ) {
		return false;
	}

	g_pGameSaveRestoreBlockSet->AddBlockHandler( GetEntitySaveRestoreBlockHandler() );
	g_pGameSaveRestoreBlockSet->AddBlockHandler( GetPhysSaveRestoreBlockHandler() );
	g_pGameSaveRestoreBlockSet->AddBlockHandler( GetViewEffectsRestoreBlockHandler() );

	ClientWorldFactoryInit();

	C_BaseAnimating::InitBoneSetupThreadPool();

	#if IsWindows()
		// NVNT connect haptics system
		ConnectHaptics( appSystemFactory );
	#endif
	HookHapticMessages();  // Always hook the messages

	return true;
}

bool CHLClient::ReplayInit( CreateInterfaceFn fnReplayFactory ) {
	#if defined( REPLAY_ENABLED )
		if ( !IsPC() ) {
			return false;
		}
		if ( ( g_pReplay = static_cast<IReplaySystem*>( fnReplayFactory( REPLAY_INTERFACE_VERSION, nullptr ) ) ) == nullptr ) {
			return false;
		}
		if ( ( g_pClientReplayContext = g_pReplay->CL_GetContext() ) == nullptr ) {
			return false;
		}

		return true;
	#else
		return false;
	#endif
}

bool CHLClient::ReplayPostInit() {
	#if defined( REPLAY_ENABLED )
		if ( ( g_pReplayManager = g_pClientReplayContext->GetReplayManager() ) == nullptr )
			return false;
		if ( ( g_pReplayScreenshotManager = g_pClientReplayContext->GetScreenshotManager() ) == nullptr )
			return false;
		if ( ( g_pReplayPerformanceManager = g_pClientReplayContext->GetPerformanceManager() ) == nullptr )
			return false;
		if ( ( g_pReplayPerformanceController = g_pClientReplayContext->GetPerformanceController() ) == nullptr )
			return false;
		if ( ( g_pReplayMovieManager = g_pClientReplayContext->GetMovieManager() ) == nullptr )
			return false;
		return true;
	#else
		return false;
	#endif
}

//-----------------------------------------------------------------------------
// Purpose: Called after client & server DLL are loaded and all systems initialized
//-----------------------------------------------------------------------------
void CHLClient::PostInit() {
	IGameSystem::PostInitAllSystems();

	#if defined( SIXENSE )
		// allow sixsense input to perform post-init operations
		g_pSixenseInput->PostInit();
	#endif

	g_ClientVirtualReality.StartupComplete();
}

//-----------------------------------------------------------------------------
// Purpose: Called when the client .dll is being dismissed
//-----------------------------------------------------------------------------
void CHLClient::Shutdown() {
	if ( g_pAchievementsAndStatsInterface ) {
		g_pAchievementsAndStatsInterface->ReleasePanel();
	}

	#if defined( SIXENSE )
		g_pSixenseInput->Shutdown();
		delete g_pSixenseInput;
		g_pSixenseInput = nullptr;
	#endif

	C_BaseAnimating::ShutdownBoneSetupThreadPool();
	ClientWorldFactoryShutdown();

	g_pGameSaveRestoreBlockSet->RemoveBlockHandler( GetViewEffectsRestoreBlockHandler() );
	g_pGameSaveRestoreBlockSet->RemoveBlockHandler( GetPhysSaveRestoreBlockHandler() );
	g_pGameSaveRestoreBlockSet->RemoveBlockHandler( GetEntitySaveRestoreBlockHandler() );

	ClientVoiceMgr_Shutdown();

	Initializer::FreeAllObjects();

	g_pClientMode->Disable();
	g_pClientMode->Shutdown();

	input->Shutdown_All();
	C_BaseTempEntity::ClearDynamicTempEnts();
	TermSmokeFogOverlay();
	view->Shutdown();
	g_pParticleSystemMgr->UncacheAllParticleSystems();
	UncacheAllMaterials();

	IGameSystem::ShutdownAllSystems();

	gHUD.Shutdown();
	VGui_Shutdown();

	ParticleMgr()->Term();

	ClearKeyValuesCache();

	#if !defined( NO_STEAM )
		ClientSteamContext().Shutdown();
	#endif

	#if defined( WORKSHOP_IMPORT_ENABLED )
		ShutdownDataModel();
		DisconnectDataModel();
		ShutdownFbx();
	#endif

	// This call disconnects the VGui libraries which we rely on later in the shutdown path, so don't do it
	//	DisconnectTier3Libraries( );
	DisconnectTier2Libraries();
	ConVar_Unregister();
	DisconnectTier1Libraries();

	gameeventmanager = nullptr;

	#if IsWindows()
		// NVNT Disconnect haptics system
		DisconnectHaptics();
	#endif
}


//-----------------------------------------------------------------------------
// Purpose:
//  Called when the game initializes
//  and whenever the vid_mode is changed
//  so the HUD can reinitialize itself.
// Output : int
//-----------------------------------------------------------------------------
int CHLClient::HudVidInit() {
	gHUD.VidInit();

	GetClientVoiceMgr()->VidInit();

	return 1;
}

//-----------------------------------------------------------------------------
// Method used to allow the client to filter input messages before the
// move record is transmitted to the server
//-----------------------------------------------------------------------------
void CHLClient::HudProcessInput( bool bActive ) {
	g_pClientMode->ProcessInput( bActive );
}

//-----------------------------------------------------------------------------
// Purpose: Called when shared data gets changed, allows dll to modify data
// Input  : bActive -
//-----------------------------------------------------------------------------
void CHLClient::HudUpdate( bool bActive ) {
	float frametime = gpGlobals->frametime;

	#if defined( TF_CLIENT_DLL )
		CRTime::UpdateRealTime();
	#endif

	GetClientVoiceMgr()->Frame( frametime );

	gHUD.UpdateHud( bActive );

	{
		C_BaseAnimating::AutoAllowBoneAccess boneaccess( true, false );
		IGameSystem::UpdateAllSystems( frametime );
	}

	// run vgui animations
	vgui::GetAnimationController()->UpdateAnimations( engine->Time() );

	hudlcd->SetGlobalStat( "(time_int)", VarArgs( "%d", (int) gpGlobals->curtime ) );
	hudlcd->SetGlobalStat( "(time_float)", VarArgs( "%.2f", gpGlobals->curtime ) );

	// I don't think this is necessary any longer, but I will leave it until
	// I can check into this further.
	C_BaseTempEntity::CheckDynamicTempEnts();

	#if defined( SIXENSE )
		// If we're not connected, update sixense so we can move the mouse cursor when in the menus
		if ( !engine->IsConnected() || engine->IsPaused() ) {
			g_pSixenseInput->SixenseFrame( 0, nullptr );
		}
	#endif
}

//-----------------------------------------------------------------------------
// Purpose: Called to restore to "non"HUD state.
//-----------------------------------------------------------------------------
void CHLClient::HudReset() {
	gHUD.VidInit();
	PhysicsReset();
}

//-----------------------------------------------------------------------------
// Purpose: Called to add hud text message
//-----------------------------------------------------------------------------
void CHLClient::HudText( const char* message ) {
	DispatchHudText( message );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CHLClient::ShouldDrawDropdownConsole() {
	#if defined( REPLAY_ENABLED )
		extern ConVar hud_freezecamhide;
		extern bool IsTakingAFreezecamScreenshot();

		if ( hud_freezecamhide.GetBool() && IsTakingAFreezecamScreenshot() ) {
			return false;
		}
	#endif

	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
// Output : ClientClass
//-----------------------------------------------------------------------------
ClientClass* CHLClient::GetAllClasses() {
	return g_pClientClassHead;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHLClient::IN_ActivateMouse() {
	input->ActivateMouse();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHLClient::IN_DeactivateMouse() {
	input->DeactivateMouse();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHLClient::IN_Accumulate() {
	input->AccumulateMouse();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHLClient::IN_ClearStates() {
	input->ClearStates();
}

//-----------------------------------------------------------------------------
// Purpose: Engine can query for particular keys
// Input  : *name -
//-----------------------------------------------------------------------------
bool CHLClient::IN_IsKeyDown( const char* name, bool& isdown ) {
	kbutton_t* key = input->FindKey( name );
	if ( !key ) {
		return false;
	}

	isdown = key->state & 1 ? true : false;

	// Found the key by name
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Engine can issue a key event
// Input  : eventcode -
//			keynum -
//			*pszCurrentBinding -
void CHLClient::IN_OnMouseWheeled( int nDelta ) {
	#if defined( REPLAY_ENABLED )
		CReplayPerformanceEditorPanel* pPerfEditor = ReplayUI_GetPerformanceEditor();
		if ( pPerfEditor ) {
			pPerfEditor->OnInGameMouseWheelEvent( nDelta );
		}
	#endif
}

//-----------------------------------------------------------------------------
// Purpose: Engine can issue a key event
// Input  : eventcode -
//			keynum -
//			*pszCurrentBinding -
// Output : int
//-----------------------------------------------------------------------------
int CHLClient::IN_KeyEvent( int eventcode, ButtonCode_t keynum, const char* pszCurrentBinding ) {
	return input->KeyEvent( eventcode, keynum, pszCurrentBinding );
}

void CHLClient::ExtraMouseSample( float frametime, bool active ) {
	Assert( C_BaseEntity::IsAbsRecomputationsEnabled() );
	Assert( C_BaseEntity::IsAbsQueriesValid() );

	C_BaseAnimating::AutoAllowBoneAccess boneaccess( true, false );

	MDLCACHE_CRITICAL_SECTION();
	input->ExtraMouseSample( frametime, active );
}

void CHLClient::IN_SetSampleTime( float frametime ) {
	input->Joystick_SetSampleTime( frametime );
	input->IN_SetSampleTime( frametime );

	#if defined( SIXENSE )
		g_pSixenseInput->ResetFrameTime( frametime );
	#endif
}
//-----------------------------------------------------------------------------
// Purpose: Fills in usercmd_s structure based on current view angles and key/controller inputs
// Input  : frametime - timestamp for last frame
//			*cmd - the command to fill in
//			active - whether the user is fully connected to a server
//-----------------------------------------------------------------------------
void CHLClient::CreateMove( int sequence_number, float input_sample_frametime, bool active ) {

	Assert( C_BaseEntity::IsAbsRecomputationsEnabled() );
	Assert( C_BaseEntity::IsAbsQueriesValid() );

	C_BaseAnimating::AutoAllowBoneAccess boneaccess( true, false );

	MDLCACHE_CRITICAL_SECTION();
	input->CreateMove( sequence_number, input_sample_frametime, active );
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : *buf -
//			from -
//			to -
//-----------------------------------------------------------------------------
bool CHLClient::WriteUsercmdDeltaToBuffer( bf_write* buf, int from, int to, bool isnewcommand ) {
	return input->WriteUsercmdDeltaToBuffer( buf, from, to, isnewcommand );
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : buf -
//			buffersize -
//			slot -
//-----------------------------------------------------------------------------
void CHLClient::EncodeUserCmdToBuffer( bf_write& buf, int slot ) {
	input->EncodeUserCmdToBuffer( buf, slot );
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : buf -
//			buffersize -
//			slot -
//-----------------------------------------------------------------------------
void CHLClient::DecodeUserCmdFromBuffer( bf_read& buf, int slot ) {
	input->DecodeUserCmdFromBuffer( buf, slot );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CHLClient::View_Render( vrect_t* rect ) {
	VPROF( "View_Render" );

	// UNDONE: This gets hit at startup sometimes, investigate - will cause NaNs in calcs inside Render()
	if ( rect->width == 0 || rect->height == 0 ) {
		return;
	}

	view->Render( rect );
	UpdatePerfStats();
}


//-----------------------------------------------------------------------------
// Gets the location of the player viewpoint
//-----------------------------------------------------------------------------
bool CHLClient::GetPlayerView( CViewSetup& playerView ) {
	playerView = *view->GetPlayerViewSetup();
	return true;
}

//-----------------------------------------------------------------------------
// Matchmaking
//-----------------------------------------------------------------------------
void CHLClient::SetupGameProperties( CUtlVector<XUSER_CONTEXT>& contexts, CUtlVector<XUSER_PROPERTY>& properties ) {
	presence->SetupGameProperties( contexts, properties );
}

uint CHLClient::GetPresenceID( const char* pIDName ) {
	return presence->GetPresenceID( pIDName );
}

const char* CHLClient::GetPropertyIdString( const uint id ) {
	return presence->GetPropertyIdString( id );
}

void CHLClient::GetPropertyDisplayString( uint id, uint value, char* pOutput, int nBytes ) {
	presence->GetPropertyDisplayString( id, value, pOutput, nBytes );
}

void CHLClient::StartStatsReporting( HANDLE handle, bool bArbitrated ) {
	presence->StartStatsReporting( handle, bArbitrated );
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CHLClient::InvalidateMdlCache() {
	for ( C_BaseEntity* pEntity = ClientEntityList().FirstBaseEntity(); pEntity; pEntity = ClientEntityList().NextBaseEntity( pEntity ) ) {
		auto* pAnimating = dynamic_cast<C_BaseAnimating*>( pEntity );
		if ( pAnimating ) {
			pAnimating->InvalidateMdlCache();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : *pSF -
//-----------------------------------------------------------------------------
void CHLClient::View_Fade( ScreenFade_t* pSF ) {
	if ( pSF != nullptr ) {
		vieweffects->Fade( *pSF );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Per level init
//-----------------------------------------------------------------------------
void CHLClient::LevelInitPreEntity( char const* pMapName ) {
	// HACK: Bogus, but the logic is too complicated in the engine
	if ( g_bLevelInitialized ) {
		return;
	}
	g_bLevelInitialized = true;

	input->LevelInit();

	vieweffects->LevelInit();

	//Tony; loadup per-map manifests.
	ParseParticleEffectsMap( pMapName, true );

	// Tell mode manager that map is changing
	modemanager->LevelInit( pMapName );
	ParticleMgr()->LevelInit();

	hudlcd->SetGlobalStat( "(mapname)", pMapName );

	C_BaseTempEntity::ClearDynamicTempEnts();
	clienteffects->Flush();
	view->LevelInit();
	tempents->LevelInit();
	ResetToneMapping( 1.0 );

	IGameSystem::LevelInitPreEntityAllSystems( pMapName );

	#if defined( USES_ECON_ITEMS )
		GameItemSchema_t* pItemSchema = ItemSystem()->GetItemSchema();
		if ( pItemSchema ) {
			pItemSchema->BInitFromDelayedBuffer();
		}
	#endif

	ResetWindspeed();

	#if !defined( NO_ENTITY_PREDICTION )
		// don't do prediction if single player!
		// don't set direct because of FCVAR_USERINFO
		if ( gpGlobals->maxClients > 1 ) {
			if ( !cl_predict->GetInt() ) {
				engine->ClientCmd( "cl_predict 1" );
			}
		} else {
			if ( cl_predict->GetInt() ) {
				engine->ClientCmd( "cl_predict 0" );
			}
		}
	#endif

	// Check low violence settings for this map
	g_RagdollLVManager.SetLowViolence( pMapName );

	gHUD.LevelInit();

	#if defined( REPLAY_ENABLED )
		// Initialize replay ragdoll recorder
		if ( !engine->IsPlayingDemo() ) {
			CReplayRagdollRecorder::Instance().Init();
		}
	#endif
}


//-----------------------------------------------------------------------------
// Purpose: Per level init
//-----------------------------------------------------------------------------
void CHLClient::LevelInitPostEntity() {
	IGameSystem::LevelInitPostEntityAllSystems();
	C_PhysPropClientside::RecreateAll();
	internalCenterPrint->Clear();
}

//-----------------------------------------------------------------------------
// Purpose: Reset our global string table pointers
//-----------------------------------------------------------------------------
void CHLClient::ResetStringTablePointers() {
	g_pStringTableParticleEffectNames = nullptr;
	g_StringTableEffectDispatch = nullptr;
	g_StringTableVguiScreen = nullptr;
	g_pStringTableMaterials = nullptr;
	g_pStringTableInfoPanel = nullptr;
	g_pStringTableClientSideChoreoScenes = nullptr;
	g_pStringTableServerMapCycle = nullptr;

	#if defined( TF_CLIENT_DLL )
		g_pStringTableServerPopFiles = nullptr;
		g_pStringTableServerMapCycleMvM = nullptr;
	#endif
}

//-----------------------------------------------------------------------------
// Purpose: Per level de-init
//-----------------------------------------------------------------------------
void CHLClient::LevelShutdown() {
	// HACK: Bogus, but the logic is too complicated in the engine
	if ( !g_bLevelInitialized ) {
		return;
	}

	g_bLevelInitialized = false;

	// Disable abs recomputations when everything is shutting down
	CBaseEntity::EnableAbsRecomputations( false );

	// Level shutdown sequence.
	// First do the pre-entity shutdown of all systems
	IGameSystem::LevelShutdownPreEntityAllSystems();

	C_PhysPropClientside::DestroyAll();

	modemanager->LevelShutdown();

	// Remove temporary entities before removing entities from the client entity list so that the te_* may
	// clean up before hand.
	tempents->LevelShutdown();

	// Now release/delete the entities
	cl_entitylist->Release();

	C_BaseEntityClassList* pClassList = s_pClassLists;
	while ( pClassList ) {
		pClassList->LevelShutdown();
		pClassList = pClassList->m_pNextClassList;
	}

	// Now do the post-entity shutdown of all systems
	IGameSystem::LevelShutdownPostEntityAllSystems();

	view->LevelShutdown();
	beams->ClearBeams();
	ParticleMgr()->RemoveAllEffects();

	StopAllRumbleEffects();

	gHUD.LevelShutdown();

	internalCenterPrint->Clear();

	messagechars->Clear();

	#if !defined( TF_CLIENT_DLL )
		// don't want to do this for TF2 because we have particle systems in our
		// character loadout screen that can be viewed when we're not connected to a server
		g_pParticleSystemMgr->UncacheAllParticleSystems();
	#endif
	UncacheAllMaterials();

	// string tables are cleared on disconnect from a server, so reset our global pointers to nullptr
	ResetStringTablePointers();

	#if defined( REPLAY_ENABLED )
		// Shutdown the ragdoll recorder
		CReplayRagdollRecorder::Instance().Shutdown();
		CReplayRagdollCache::Instance().Shutdown();
	#endif
}


//-----------------------------------------------------------------------------
// Purpose: Engine received crosshair offset ( autoaim )
// Input  : angle -
//-----------------------------------------------------------------------------
void CHLClient::SetCrosshairAngle( const QAngle& angle ) {
	auto* crosshair = GET_HUDELEMENT( CHudCrosshair );
	if ( crosshair ) {
		crosshair->SetCrosshairAngle( angle );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Helper to initialize sprite from .spr semaphor
// Input  : *pSprite -
//			*loadname -
//-----------------------------------------------------------------------------
void CHLClient::InitSprite( CEngineSprite* pSprite, const char* loadname ) {
	if ( pSprite ) {
		pSprite->Init( loadname );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : *pSprite -
//-----------------------------------------------------------------------------
void CHLClient::ShutdownSprite( CEngineSprite* pSprite ) {
	if ( pSprite ) {
		pSprite->Shutdown();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Tells engine how much space to allocate for sprite objects
// Output : int
//-----------------------------------------------------------------------------
int CHLClient::GetSpriteSize() const {
	return sizeof( CEngineSprite );
}


//-----------------------------------------------------------------------------
// Purpose:
// Input  : entindex -
//			bTalking -
//-----------------------------------------------------------------------------
void CHLClient::VoiceStatus( int entindex, qboolean bTalking ) {
	GetClientVoiceMgr()->UpdateSpeakerStatus( entindex, !!bTalking );
}


//-----------------------------------------------------------------------------
// Called when the string table for materials changes
//-----------------------------------------------------------------------------
void OnMaterialStringTableChanged( void* object, INetworkStringTable* stringTable, int stringNumber, const char* newString, void const* newData ) {
	// Make sure this puppy is precached
	gHLClient.PrecacheMaterial( newString );
	RequestCacheUsedMaterials();
}


//-----------------------------------------------------------------------------
// Called when the string table for particle systems changes
//-----------------------------------------------------------------------------
void OnParticleSystemStringTableChanged( void* object, INetworkStringTable* stringTable, int stringNumber, const char* newString, void const* newData ) {
	// Make sure this puppy is precached
	g_pParticleSystemMgr->PrecacheParticleSystem( newString );
	RequestCacheUsedMaterials();
}


//-----------------------------------------------------------------------------
// Called when the string table for VGUI changes
//-----------------------------------------------------------------------------
void OnVguiScreenTableChanged( void* object, INetworkStringTable* stringTable, int stringNumber, const char* newString, void const* newData ) {
	// Make sure this puppy is precached
	vgui::Panel* pPanel = PanelMetaClassMgr()->CreatePanelMetaClass( newString, 100, nullptr, nullptr );
	if ( pPanel ) {
		PanelMetaClassMgr()->DestroyPanelMetaClass( pPanel );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Preload the string on the client (if single player it should already be in the cache from the server!!!)
// Input  : *object -
//			*stringTable -
//			stringNumber -
//			*newString -
//			*newData -
//-----------------------------------------------------------------------------
void OnSceneStringTableChanged( void* object, INetworkStringTable* stringTable, int stringNumber, const char* newString, void const* newData ) {
}

//-----------------------------------------------------------------------------
// Purpose: Hook up any callbacks here, the table definition has been parsed but
//  no data has been added yet
//-----------------------------------------------------------------------------
void CHLClient::InstallStringTableCallback( const char* tableName ) {
	// Here, cache off string table IDs
	if ( !Q_strcasecmp( tableName, "VguiScreen" ) ) {
		// Look up the id
		g_StringTableVguiScreen = networkstringtable->FindTable( tableName );

		// When the material list changes, we need to know immediately
		g_StringTableVguiScreen->SetStringChangedCallback( nullptr, OnVguiScreenTableChanged );
	} else if ( !Q_strcasecmp( tableName, "Materials" ) ) {
		// Look up the id
		g_pStringTableMaterials = networkstringtable->FindTable( tableName );

		// When the material list changes, we need to know immediately
		g_pStringTableMaterials->SetStringChangedCallback( nullptr, OnMaterialStringTableChanged );
	} else if ( !Q_strcasecmp( tableName, "EffectDispatch" ) ) {
		g_StringTableEffectDispatch = networkstringtable->FindTable( tableName );
	} else if ( !Q_strcasecmp( tableName, "InfoPanel" ) ) {
		g_pStringTableInfoPanel = networkstringtable->FindTable( tableName );
	} else if ( !Q_strcasecmp( tableName, "Scenes" ) ) {
		g_pStringTableClientSideChoreoScenes = networkstringtable->FindTable( tableName );
		g_pStringTableClientSideChoreoScenes->SetStringChangedCallback( nullptr, OnSceneStringTableChanged );
	} else if ( !Q_strcasecmp( tableName, "ParticleEffectNames" ) ) {
		g_pStringTableParticleEffectNames = networkstringtable->FindTable( tableName );
		networkstringtable->SetAllowClientSideAddString( g_pStringTableParticleEffectNames, true );
		// When the particle system list changes, we need to know immediately
		g_pStringTableParticleEffectNames->SetStringChangedCallback( nullptr, OnParticleSystemStringTableChanged );
	} else if ( !Q_strcasecmp( tableName, "ServerMapCycle" ) ) {
		g_pStringTableServerMapCycle = networkstringtable->FindTable( tableName );
	}
	#ifdef TF_CLIENT_DLL
		else if ( !Q_strcasecmp( tableName, "ServerPopFiles" ) ) {
			g_pStringTableServerPopFiles = networkstringtable->FindTable( tableName );
		} else if ( !Q_strcasecmp( tableName, "ServerMapCycleMvM" ) ) {
			g_pStringTableServerMapCycleMvM = networkstringtable->FindTable( tableName );
		}
	#endif

	InstallStringTableCallback_GameRules();
}


//-----------------------------------------------------------------------------
// Material precache
//-----------------------------------------------------------------------------
void CHLClient::PrecacheMaterial( const char* pMaterialName ) {
	Assert( pMaterialName );

	int nLen = Q_strlen( pMaterialName );
	auto* pTempBuf = static_cast<char*>( stackalloc( nLen + 1 ) );
	memcpy( pTempBuf, pMaterialName, nLen + 1 );
	char* pFound = Q_strstr( pTempBuf, ".vmt\0" );
	if ( pFound ) {
		*pFound = 0;
	}

	IMaterial* pMaterial = materials->FindMaterial( pTempBuf, TEXTURE_GROUP_PRECACHED );
	if ( !IsErrorMaterial( pMaterial ) ) {
		pMaterial->IncrementReferenceCount();
		m_CachedMaterials.AddToTail( pMaterial );
	} else {
		printf( "\n ##### CHLClient::PrecacheMaterial could not find material %s (%s)", pMaterialName, pTempBuf );
	}
}

void CHLClient::UncacheAllMaterials() {
	for ( int i = m_CachedMaterials.Count(); --i >= 0; ) {
		m_CachedMaterials[i]->DecrementReferenceCount();
	}
	m_CachedMaterials.RemoveAll();
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : *pszName -
//			iSize -
//			*pbuf -
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CHLClient::DispatchUserMessage( int msg_type, bf_read& msg_data ) {
	return usermessages->DispatchUserMessage( msg_type, msg_data );
}


void SimulateEntities() {
	VPROF_BUDGET( "Client SimulateEntities", VPROF_BUDGETGROUP_CLIENT_SIM );

	// Service timer events (think functions).
	ClientThinkList()->PerformThinkFunctions();

	// TODO: make an ISimulateable interface so C_BaseNetworkables can simulate?
	{
		VPROF_( "C_BaseEntity::Simulate", 1, VPROF_BUDGETGROUP_CLIENT_SIM, false, BUDGETFLAG_CLIENT );
		C_BaseEntityIterator iterator;
		C_BaseEntity* pEnt;
		while ( ( pEnt = iterator.Next() ) != nullptr ) {
			pEnt->Simulate();
		}
	}
}


bool AddDataChangeEvent( IClientNetworkable* ent, DataUpdateType_t updateType, int* pStoredEvent ) {
	VPROF( "AddDataChangeEvent" );

	Assert( ent );
	// Make sure we don't already have an event queued for this guy.
	if ( *pStoredEvent >= 0 ) {
		Assert( g_DataChangedEvents[*pStoredEvent].m_pEntity == ent );

		// DATA_UPDATE_CREATED always overrides DATA_UPDATE_CHANGED.
		if ( updateType == DATA_UPDATE_CREATED ) {
			g_DataChangedEvents[*pStoredEvent].m_UpdateType = updateType;
		}

		return false;
	}

	*pStoredEvent = g_DataChangedEvents.AddToTail( CDataChangedEvent( ent, updateType, pStoredEvent ) );
	return true;
}


void ClearDataChangedEvent( int iStoredEvent ) {
	if ( iStoredEvent != -1 ) {
		g_DataChangedEvents.Remove( iStoredEvent );
	}
}


void ProcessOnDataChangedEvents() {
	VPROF_( "ProcessOnDataChangedEvents", 1, VPROF_BUDGETGROUP_CLIENT_SIM, false, BUDGETFLAG_CLIENT );
	FOR_EACH_LL( g_DataChangedEvents, i ) {
		CDataChangedEvent* pEvent = &g_DataChangedEvents[i];

		// Reset their stored event identifier.
		*pEvent->m_pStoredEvent = -1;

		// Send the event.
		IClientNetworkable* pNetworkable = pEvent->m_pEntity;
		pNetworkable->OnDataChanged( pEvent->m_UpdateType );
	}

	g_DataChangedEvents.Purge();
}


void UpdateClientRenderableInPVSStatus() {
	// Vis for this view should already be setup at this point.

	// For each client-only entity, notify it if it's newly coming into the PVS.
	CUtlLinkedList<CClientEntityList::CPVSNotifyInfo>& theList = ClientEntityList().GetPVSNotifiers();
	FOR_EACH_LL( theList, i ) {
		CClientEntityList::CPVSNotifyInfo* pInfo = &theList[i];

		if ( pInfo->m_InPVSStatus & INPVS_YES ) {
			// Ok, this entity already thinks it's in the PVS. No need to notify it.
			// We need to set the INPVS_YES_THISFRAME flag if it's in this frame at all, so we
			// don't tell the entity it's not in the PVS anymore at the end of the frame.
			if ( !( pInfo->m_InPVSStatus & INPVS_THISFRAME ) ) {
				if ( g_pClientLeafSystem->IsRenderableInPVS( pInfo->m_pRenderable ) ) {
					pInfo->m_InPVSStatus |= INPVS_THISFRAME;
				}
			}
		} else {
			// This entity doesn't think it's in the PVS yet. If it is now in the PVS, let it know.
			if ( g_pClientLeafSystem->IsRenderableInPVS( pInfo->m_pRenderable ) ) {
				pInfo->m_InPVSStatus |= ( INPVS_YES | INPVS_THISFRAME | INPVS_NEEDSNOTIFY );
			}
		}
	}
}

void UpdatePVSNotifiers() {
	MDLCACHE_CRITICAL_SECTION();

	// At this point, all the entities that were rendered in the previous frame have INPVS_THISFRAME set
	// so we can tell the entities that aren't in the PVS anymore so.
	CUtlLinkedList<CClientEntityList::CPVSNotifyInfo>& theList = ClientEntityList().GetPVSNotifiers();
	FOR_EACH_LL( theList, i ) {
		CClientEntityList::CPVSNotifyInfo* pInfo = &theList[i];

		// If this entity thinks it's in the PVS, but it wasn't in the PVS this frame, tell it so.
		if ( pInfo->m_InPVSStatus & INPVS_YES ) {
			if ( pInfo->m_InPVSStatus & INPVS_THISFRAME ) {
				if ( pInfo->m_InPVSStatus & INPVS_NEEDSNOTIFY ) {
					pInfo->m_pNotify->OnPVSStatusChanged( true );
				}
				// Clear it for the next time around.
				pInfo->m_InPVSStatus &= ~( INPVS_THISFRAME | INPVS_NEEDSNOTIFY );
			} else {
				pInfo->m_InPVSStatus &= ~INPVS_YES;
				pInfo->m_pNotify->OnPVSStatusChanged( false );
			}
		}
	}
}


void OnRenderStart() {
	VPROF( "OnRenderStart" );
	MDLCACHE_CRITICAL_SECTION();
	MDLCACHE_COARSE_LOCK();

	#if defined( PORTAL )
		g_pPortalRender->UpdatePortalPixelVisibility();  // updating this one or two lines before querying again just isn't cutting it. Update as soon as it's cheap to do so.
	#endif

	partition->SuppressLists( PARTITION_ALL_CLIENT_EDICTS, true );
	C_BaseEntity::SetAbsQueriesValid( false );

	Rope_ResetCounters();

	// Interpolate server entities and move aiments.
	{
		PREDICTION_TRACKVALUECHANGESCOPE( "interpolation" );
		C_BaseEntity::InterpolateServerEntities();
	}

	{
		// vprof node for this block of math
		VPROF( "OnRenderStart: dirty bone caches" );
		// Invalidate any bone information.
		C_BaseAnimating::InvalidateBoneCaches();

		C_BaseEntity::SetAbsQueriesValid( true );
		C_BaseEntity::EnableAbsRecomputations( true );

		// Enable access to all model bones except view models.
		// This is necessary for aim-ent computation to occur properly
		C_BaseAnimating::PushAllowBoneAccess( true, false, "OnRenderStart->CViewRender::SetUpView" );  // pops in CViewRender::SetUpView

		// FIXME: This needs to be done before the player moves; it forces
		//        AimEnts the player may be attached to to forcibly update their position
		C_BaseEntity::MarkAimEntsDirty();
	}

	// Make sure the camera simulation happens before OnRenderStart, where it's used.
	// NOTE: the only thing that happens in CAM_Think is thirdperson related code.
	input->CAM_Think();

	// This will place the player + the view models + all parent
	// entities	at the correct abs position so that their attachment points
	// are at the correct location
	view->OnRenderStart();

	RopeManager()->OnRenderStart();

	// This will place all entities in the correct position in world space and in the KD-tree
	C_BaseAnimating::UpdateClientSideAnimations();

	partition->SuppressLists( PARTITION_ALL_CLIENT_EDICTS, false );

	// Process OnDataChanged events.
	ProcessOnDataChangedEvents();

	// Reset the overlay alpha. Entities can change the state of this in their think functions.
	g_SmokeFogOverlayAlpha = 0;

	// This must occur prior to SimulatEntities,
	// which is where the client thinks for c_colorcorrection + c_colorcorrectionvolumes
	// update the color correction weights.
	// FIXME: The place where IGameSystem::Update is called should be in here
	// so we don't have to explicitly call ResetColorCorrectionWeights + SimulateEntities, etc.
	g_pColorCorrectionMgr->ResetColorCorrectionWeights();

	// Simulate all the entities.
	SimulateEntities();
	PhysicsSimulate();

	C_BaseAnimating::ThreadedBoneSetup();

	{
		VPROF_( "Client TempEnts", 0, VPROF_BUDGETGROUP_CLIENT_SIM, false, BUDGETFLAG_CLIENT );
		// This creates things like temp entities.
		engine->FireEvents();

		// Update temp entities
		tempents->Update();

		// Update temp ent beams...
		beams->UpdateTempEntBeams();

		// Lock the frame from beam additions
		SetBeamCreationAllowed( false );
	}

	// Update particle effects (eventually, the effects should use Simulate() instead of having
	// their own update system).
	{
		// Enable FP exceptions here when FP_EXCEPTIONS_ENABLED is defined,
		// to help track down bad math.
		FPExceptionEnabler enableExceptions;
		VPROF_BUDGET( "ParticleMgr()->Simulate", VPROF_BUDGETGROUP_PARTICLE_SIMULATION );
		ParticleMgr()->Simulate( gpGlobals->frametime );
	}

	// Now that the view model's position is setup and aiments are marked dirty, update
	// their positions so they're in the leaf system correctly.
	C_BaseEntity::CalcAimEntPositions();

	// For entities marked for recording, post bone messages to IToolSystems
	if ( ToolsEnabled() ) {
		C_BaseEntity::ToolRecordEntities();
	}

	#if defined( REPLAY_ENABLED )
		// This will record any ragdolls if Replay mode is enabled on the server
		CReplayRagdollRecorder::Instance().Think();
		CReplayRagdollCache::Instance().Think();
	#endif

	// Finally, link all the entities into the leaf system right before rendering.
	C_BaseEntity::AddVisibleEntities();
}


void OnRenderEnd() {
	// Disallow access to bones (access is enabled in CViewRender::SetUpView).
	C_BaseAnimating::PopBoneAccess( "CViewRender::SetUpView->OnRenderEnd" );

	UpdatePVSNotifiers();

	DisplayBoneSetupEnts();
}


void CHLClient::FrameStageNotify( ClientFrameStage_t curStage ) {
	g_CurFrameStage = curStage;

	switch ( curStage ) {
		default:
			break;

		case FRAME_RENDER_START: {
			VPROF( "CHLClient::FrameStageNotify FRAME_RENDER_START" );

			// Last thing before rendering, run simulation.
			OnRenderStart();
			break;
		}

		case FRAME_RENDER_END: {
			VPROF( "CHLClient::FrameStageNotify FRAME_RENDER_END" );
			OnRenderEnd();

			PREDICTION_SPEWVALUECHANGES();
			break;
		}

		case FRAME_NET_UPDATE_START: {
			VPROF( "CHLClient::FrameStageNotify FRAME_NET_UPDATE_START" );
			// disabled all recomputations while we update entities
			C_BaseEntity::EnableAbsRecomputations( false );
			C_BaseEntity::SetAbsQueriesValid( false );
			Interpolation_SetLastPacketTimeStamp( engine->GetLastTimeStamp() );
			partition->SuppressLists( PARTITION_ALL_CLIENT_EDICTS, true );

			PREDICTION_STARTTRACKVALUE( "netupdate" );
			break;
		}
		case FRAME_NET_UPDATE_END: {
			ProcessCacheUsedMaterials();

			// reenable abs recomputation since now all entities have been updated
			C_BaseEntity::EnableAbsRecomputations( true );
			C_BaseEntity::SetAbsQueriesValid( true );
			partition->SuppressLists( PARTITION_ALL_CLIENT_EDICTS, false );

			PREDICTION_ENDTRACKVALUE();
			break;
		}
		case FRAME_NET_UPDATE_POSTDATAUPDATE_START: {
			VPROF( "CHLClient::FrameStageNotify FRAME_NET_UPDATE_POSTDATAUPDATE_START" );
			PREDICTION_STARTTRACKVALUE( "postdataupdate" );
			break;
		}
		case FRAME_NET_UPDATE_POSTDATAUPDATE_END: {
			VPROF( "CHLClient::FrameStageNotify FRAME_NET_UPDATE_POSTDATAUPDATE_END" );
			PREDICTION_ENDTRACKVALUE();
			// Let prediction copy off pristine data
			prediction->PostEntityPacketReceived();
			HLTVCamera()->PostEntityPacketReceived();
			#if defined( REPLAY_ENABLED )
				ReplayCamera()->PostEntityPacketReceived();
			#endif
			break;
		}
		case FRAME_START: {
			// Mark the frame as open for client fx additions
			SetFXCreationAllowed( true );
			SetBeamCreationAllowed( true );
			C_BaseEntity::CheckCLInterpChanged();
			break;
		}
	}
}

CSaveRestoreData* SaveInit( int size );

// Save/restore system hooks
CSaveRestoreData* CHLClient::SaveInit( int size ) {
	return ::SaveInit( size );
}

void CHLClient::SaveWriteFields( CSaveRestoreData* pSaveData, const char* pname, void* pBaseData, datamap_t* pMap, typedescription_t* pFields, int fieldCount ) {
	CSave saveHelper( pSaveData );
	saveHelper.WriteFields( pname, pBaseData, pMap, pFields, fieldCount );
}

void CHLClient::SaveReadFields( CSaveRestoreData* pSaveData, const char* pname, void* pBaseData, datamap_t* pMap, typedescription_t* pFields, int fieldCount ) {
	CRestore restoreHelper( pSaveData );
	restoreHelper.ReadFields( pname, pBaseData, pMap, pFields, fieldCount );
}

void CHLClient::PreSave( CSaveRestoreData* s ) {
	g_pGameSaveRestoreBlockSet->PreSave( s );
}

void CHLClient::Save( CSaveRestoreData* s ) {
	CSave saveHelper( s );
	g_pGameSaveRestoreBlockSet->Save( &saveHelper );
}

void CHLClient::WriteSaveHeaders( CSaveRestoreData* s ) {
	CSave saveHelper( s );
	g_pGameSaveRestoreBlockSet->WriteSaveHeaders( &saveHelper );
	g_pGameSaveRestoreBlockSet->PostSave();
}

void CHLClient::ReadRestoreHeaders( CSaveRestoreData* s ) {
	CRestore restoreHelper( s );
	g_pGameSaveRestoreBlockSet->PreRestore();
	g_pGameSaveRestoreBlockSet->ReadRestoreHeaders( &restoreHelper );
}

void CHLClient::Restore( CSaveRestoreData* s, bool b ) {
	CRestore restore( s );
	g_pGameSaveRestoreBlockSet->Restore( &restore, b );
	g_pGameSaveRestoreBlockSet->PostRestore();
}

static CUtlVector<EHANDLE> g_RestoredEntities;

void AddRestoredEntity( C_BaseEntity* pEntity ) {
	if ( !pEntity ) {
		return;
	}

	g_RestoredEntities.AddToTail( EHANDLE( pEntity ) );
}

void CHLClient::DispatchOnRestore() {
	for ( int i = 0; i < g_RestoredEntities.Count(); i++ ) {
		if ( g_RestoredEntities[i] != nullptr ) {
			MDLCACHE_CRITICAL_SECTION();
			g_RestoredEntities[i]->OnRestore();
		}
	}
	g_RestoredEntities.RemoveAll();
}

void CHLClient::WriteSaveGameScreenshot( const char* pFilename ) {
	view->WriteSaveGameScreenshot( pFilename );
}

// Given a list of "S(wavname) S(wavname2)" tokens, look up the localized text and emit
//  the appropriate close caption if running with closecaption = 1
void CHLClient::EmitSentenceCloseCaption( char const* tokenstream ) {
	extern ConVar closecaption;

	if ( !closecaption.GetBool() ) {
		return;
	}

	auto* hudCloseCaption = GET_HUDELEMENT( CHudCloseCaption );
	if ( hudCloseCaption ) {
		hudCloseCaption->ProcessSentenceCaptionStream( tokenstream );
	}
}


void CHLClient::EmitCloseCaption( char const* captionname, float duration ) {
	extern ConVar closecaption; // NOLINT(*-redundant-declaration)

	if ( !closecaption.GetBool() ) {
		return;
	}

	auto* hudCloseCaption = GET_HUDELEMENT( CHudCloseCaption );
	if ( hudCloseCaption ) {
		hudCloseCaption->ProcessCaption( captionname, duration );
	}
}

CStandardRecvProxies* CHLClient::GetStandardRecvProxies() {
	return &g_StandardRecvProxies;
}

bool CHLClient::CanRecordDemo( char* errorMsg, int length ) const {
	if ( GetClientModeNormal() ) {
		return GetClientModeNormal()->CanRecordDemo( errorMsg, length );
	}

	return true;
}

void CHLClient::OnDemoRecordStart( char const* pDemoBaseName ) {
}

void CHLClient::OnDemoRecordStop() {
}

void CHLClient::OnDemoPlaybackStart( char const* pDemoBaseName ) {
	#if defined( REPLAY_ENABLED )
		// Load any ragdoll override frames from disk
		char szRagdollFile[MAX_OSPATH];
		V_snprintf( szRagdollFile, sizeof( szRagdollFile ), "%s.dmx", pDemoBaseName );
		CReplayRagdollCache::Instance().Init( szRagdollFile );
	#endif
}

void CHLClient::OnDemoPlaybackStop() {
	#if defined( DEMOPOLISH_ENABLED )
		if ( DemoPolish_GetController().m_bInit ) {
			DemoPolish_GetController().Shutdown();
		}
	#endif

	#if defined( REPLAY_ENABLED )
		CReplayRagdollCache::Instance().Shutdown();
	#endif
}

int CHLClient::GetScreenWidth() {
	return ScreenWidth();
}

int CHLClient::GetScreenHeight() {
	return ScreenHeight();
}

// NEW INTERFACES
// save game screenshot writing
void CHLClient::WriteSaveGameScreenshotOfSize( const char* pFilename, int width, int height, bool bCreatePowerOf2Padded /*=false*/,
											   bool bWriteVTF /*=false*/ ) {
	view->WriteSaveGameScreenshotOfSize( pFilename, width, height, bCreatePowerOf2Padded, bWriteVTF );
}

// See RenderViewInfo_t
void CHLClient::RenderView( const CViewSetup& view, int nClearFlags, int whatToDraw ) {
	VPROF( "RenderView" );
	::view->RenderView( view, nClearFlags, whatToDraw );
}

void ReloadSoundEntriesInList( IFileList* pFilesToReload );

//-----------------------------------------------------------------------------
// For sv_pure mode. The filesystem figures out which files the client needs to reload to be "pure" ala the server's preferences.
//-----------------------------------------------------------------------------
void CHLClient::ReloadFilesInList( IFileList* pFilesToReload ) {
	ReloadParticleEffectsInList( pFilesToReload );
	ReloadSoundEntriesInList( pFilesToReload );
}

bool CHLClient::HandleUiToggle() {
	#if defined( REPLAY_ENABLED )
		if ( !g_pEngineReplay || !g_pEngineReplay->IsSupportedModAndPlatform() ) {
			return false;
		}

		CReplayPerformanceEditorPanel* pEditor = ReplayUI_GetPerformanceEditor();
		if ( !pEditor ) {
			return false;
		}

		pEditor->HandleUiToggle();

		return true;

	#else
		return false;
	#endif
}

bool CHLClient::ShouldAllowConsole() {
	return true;
}

CRenamedRecvTableInfo* CHLClient::GetRenamedRecvTableInfos() {
	return g_pRenamedRecvTableInfoHead;
}

CMouthInfo g_ClientUIMouth;
// Get the mouthinfo for the sound being played inside UI panels
CMouthInfo* CHLClient::GetClientUIMouthInfo() {
	return &g_ClientUIMouth;
}

void CHLClient::FileReceived( const char* fileName, unsigned int transferID ) {
	if ( g_pGameRules ) {
		g_pGameRules->OnFileReceived( fileName, transferID );
	}
}

void CHLClient::ClientAdjustStartSoundParams( StartSoundParams_t& params ) {
	#if defined( TF_CLIENT_DLL )
		CBaseEntity* pEntity = ClientEntityList().GetEnt( params.soundsource );

		// A player speaking
		if ( params.entchannel == CHAN_VOICE && GameRules() && pEntity && pEntity->IsPlayer() ) {
			// Use high-pitched voices for other players if the local player has an item that allows them to hear it (Pyro Goggles)
			if ( !GameRules()->IsLocalPlayer( params.soundsource ) && IsLocalPlayerUsingVisionFilterFlags( TF_VISION_FILTER_PYRO ) ) {
				params.pitch *= 1.3f;
			}
			// Halloween voice futzery?
			else {
				float flVoicePitchScale = 1.f;
				CALL_ATTRIB_HOOK_FLOAT_ON_OTHER( pEntity, flVoicePitchScale, voice_pitch_scale );

				int iHalloweenVoiceSpell = 0;
				CALL_ATTRIB_HOOK_INT_ON_OTHER( pEntity, iHalloweenVoiceSpell, halloween_voice_modulation );
				if ( iHalloweenVoiceSpell > 0 ) {
					params.pitch *= 0.8f;
				} else if ( flVoicePitchScale != 1.f ) {
					params.pitch *= flVoicePitchScale;
				}
			}
		}
	#endif
}

const char* CHLClient::TranslateEffectForVisionFilter( const char* pchEffectType, const char* pchEffectName ) {
	if ( !GameRules() ) {
		return pchEffectName;
	}

	return GameRules()->TranslateEffectForVisionFilter( pchEffectType, pchEffectName );
}

bool CHLClient::DisconnectAttempt() {
	bool bRet = false;

	#if defined( TF_CLIENT_DLL )
		bRet = HandleDisconnectAttempt();
	#endif

	return bRet;
}

bool CHLClient::IsConnectedUserInfoChangeAllowed( IConVar* pCvar ) {
	return GameRules() ? GameRules()->IsConnectedUserInfoChangeAllowed( nullptr ) : true;
}

#if !defined( NO_STEAM )
	CSteamID GetSteamIDForPlayerIndex( int iPlayerIndex ) {
		player_info_t pi;
		if ( steamapicontext && steamapicontext->SteamUtils() ) {
			if ( engine->GetPlayerInfo( iPlayerIndex, &pi ) ) {
				if ( pi.friendsID ) {
					return { pi.friendsID, 1, steamapicontext->SteamUtils()->GetConnectedUniverse(), k_EAccountTypeIndividual };
				}
			}
		}
		return {};
	}
#endif
