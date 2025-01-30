//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Contains an interface to enable positional audio support in Mumble
//
// $NoKeywords: $
//
//===========================================================================//
#pragma once
#include "GameEventListener.h"
#include "igamesystem.h"


class CMumbleSystem : public CBaseGameSystemPerFrame, public CGameEventListener {
public:
	// Methods of IGameSystem
	virtual bool Init();
	virtual void LevelInitPostEntity();
	virtual void LevelShutdownPreEntity();
	virtual void PostRender();

	// Methods of IGameSystem
	virtual void FireGameEvent( IGameEvent* event );

private:
	char m_szSteamIDCurrentServer[32] { };
	int32 m_cubSteamIDCurrentServer{};
	bool m_bHasSetPlayerUniqueId{};
	int32 m_nTeamSetInUniqueId{};
};

IGameSystem* MumbleSystem();
