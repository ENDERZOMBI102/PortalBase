//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: implementation of player info manager
//
//=============================================================================//
#pragma once
#include "game/server/iplayerinfo.h"

//-----------------------------------------------------------------------------
// Purpose: interface for plugins to get player info
//-----------------------------------------------------------------------------
class CPlayerInfoManager : public IPlayerInfoManager {
public:
	virtual IPlayerInfo* GetPlayerInfo( edict_t* pEdict );
	virtual CGlobalVars* GetGlobalVars();
};

class CPluginBotManager : public IBotManager {
public:
	virtual IBotController* GetBotController( edict_t* pEdict );
	virtual edict_t* CreateBot( const char* botname );
};