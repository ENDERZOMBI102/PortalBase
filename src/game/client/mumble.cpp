//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Provides an interface to enable positional audio support in Mumble
//
//===========================================================================//

#if defined( PLATFORM_WINDOWS )
	#include <memoryapi.h>
	#include <handleapi.h>
#elif defined( PLATFORM_POSIX )
	#include <fcntl.h>
	#include <sys/mman.h>
	#include <sys/stat.h>
	#include <sys/types.h>
#endif

#include "cbase.h"
#include "mumble.h"
#include "view.h"
#include "c_baseplayer.h"

#if !defined( NO_STEAM )
	#include "steam/isteamuser.h"
	#include "steam/steam_api.h"
#endif

// return the mod dir (rather than the complete -game param, which can be a path)
const char* COM_GetModDirectory();

struct MumbleSharedMemory_t {
	uint32 uiVersion;
	uint32 uiTick;
	float fAvatarPosition[ 3 ];
	float fAvatarFront[ 3 ];
	float fAvatarTop[ 3 ];
	wchar_t name[ 256 ];
	float fCameraPosition[ 3 ];
	float fCameraFront[ 3 ];
	float fCameraTop[ 3 ];
	wchar_t identity[ 256 ];
	uint32 context_len;
	unsigned char context[ 256 ];
	wchar_t description[ 2048 ];
};

MumbleSharedMemory_t* g_pMumbleMemory = nullptr;
#if IsWindows()
	HANDLE g_hMapObject = nullptr;
#endif

ConVar sv_mumble_positionalaudio( "sv_mumble_positionalaudio", "1", FCVAR_REPLICATED, "Allows players using Mumble to have support for positional audio." );

//-----------------------------------------------------------------------------
// Singleton
//-----------------------------------------------------------------------------
static CMumbleSystem g_MumbleSystem;

IGameSystem* MumbleSystem() {
	return &g_MumbleSystem;
}

bool CMumbleSystem::Init() {
	m_bHasSetPlayerUniqueId = false;
	m_nTeamSetInUniqueId = 0;
	m_szSteamIDCurrentServer[ 0 ] = '\0';
	m_cubSteamIDCurrentServer = 0;

	ListenForGameEvent( "server_spawn" );

	return true;
}

void CMumbleSystem::LevelInitPostEntity() {
	if ( g_pMumbleMemory ) {
		return;
	}

	#if IsWindows()
		g_hMapObject = OpenFileMappingW( FILE_MAP_ALL_ACCESS, false, L"MumbleLink" );
		if ( g_hMapObject == nullptr ) {
			return;
		}

		g_pMumbleMemory = static_cast<MumbleSharedMemory_t*>( MapViewOfFile( g_hMapObject, FILE_MAP_ALL_ACCESS, 0, 0, sizeof( MumbleSharedMemory_t ) ) );
		if ( g_pMumbleMemory == nullptr ) {
			CloseHandle( g_hMapObject );
			g_hMapObject = nullptr;
		}
	#elif IsPosix()
		char memname[ 256 ];
		V_sprintf_safe( memname, "/MumbleLink.%d", getuid() );

		int shmfd = shm_open( memname, O_RDWR, S_IRUSR | S_IWUSR );

		if ( shmfd < 0 ) {
			return;
		}

		g_pMumbleMemory = static_cast<MumbleSharedMemory_t*>( mmap( nullptr, sizeof( MumbleSharedMemory_t ), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0 ) );

		if ( g_pMumbleMemory == reinterpret_cast<void*>( -1 ) ) {
			g_pMumbleMemory = nullptr;
		}
	#endif
}

void CMumbleSystem::LevelShutdownPreEntity() {
	#if IsWindows()
		if ( g_hMapObject ) {
			CloseHandle( g_hMapObject );
			g_pMumbleMemory = nullptr;
			g_hMapObject = nullptr;
		}
	#elif IsPosix()
		if ( g_pMumbleMemory ) {
			munmap( g_pMumbleMemory, sizeof( MumbleSharedMemory_t ) );
			g_pMumbleMemory = nullptr;
		}
	#endif
}

void VectorToMumbleFloatArray( const Vector& vec, float* rgfl ) {
	// Traditional left-handed coordinate system
	rgfl[ 0 ] = vec.x;// X
	rgfl[ 1 ] = vec.z;// Y is up
	rgfl[ 2 ] = vec.y;// Z is into the screen
}

void CMumbleSystem::PostRender() {
	#if !defined( NO_STEAM )
		if ( not g_pMumbleMemory or not sv_mumble_positionalaudio.GetBool() ) {
			return;
		}

		if ( g_pMumbleMemory->uiVersion != 2 ) {
			V_wcscpy_safe( g_pMumbleMemory->name, L"Source engine: " );
			wchar_t wcsGameDir[ MAX_PATH ];
			Q_UTF8ToUnicode( COM_GetModDirectory(), wcsGameDir, sizeof( wcsGameDir ) );
			V_wcscat_safe( g_pMumbleMemory->name, wcsGameDir );

			V_wcscpy_safe( g_pMumbleMemory->description, L"Links Source engine games to Mumble." );
			g_pMumbleMemory->uiVersion = 2;
		}

		g_pMumbleMemory->uiTick += 1;

		Vector vecOriginPlayer;
		Vector vecOriginCamera = MainViewOrigin();
		QAngle anglesPlayer;
		const QAngle anglesCamera = MainViewAngles();

		C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
		if ( pPlayer ) {
			vecOriginPlayer = pPlayer->EyePosition();
			anglesPlayer = pPlayer->GetAbsAngles();
		} else {
			vecOriginPlayer = vecOriginCamera;
			anglesPlayer = anglesCamera;
		}

		anglesPlayer.x = 0;

		Vector vecPlayerForward, vecPlayerUp, vecCameraForward, vecCameraUp;
		AngleVectors( anglesPlayer, &vecPlayerForward, nullptr, &vecPlayerUp );
		AngleVectors( anglesCamera, &vecCameraForward, nullptr, &vecCameraUp );

		// 1 Source unit is about one inch
		// 1 mumble unit = 1 meter
		vecOriginPlayer *= METERS_PER_INCH;
		vecOriginCamera *= METERS_PER_INCH;

		VectorToMumbleFloatArray( vecPlayerForward, g_pMumbleMemory->fAvatarFront );
		VectorToMumbleFloatArray( vecPlayerUp, g_pMumbleMemory->fAvatarTop );
		VectorToMumbleFloatArray( vecOriginPlayer, g_pMumbleMemory->fAvatarPosition );

		VectorToMumbleFloatArray( vecCameraForward, g_pMumbleMemory->fCameraFront );
		VectorToMumbleFloatArray( vecCameraUp, g_pMumbleMemory->fCameraTop );
		VectorToMumbleFloatArray( vecOriginCamera, g_pMumbleMemory->fCameraPosition );

		if ( pPlayer and m_bHasSetPlayerUniqueId and m_nTeamSetInUniqueId != pPlayer->GetTeamNumber() ) {
			// Player changed team since we set the unique ID. Set it again.
			m_bHasSetPlayerUniqueId = false;
		}

		if ( not m_bHasSetPlayerUniqueId and steamapicontext and steamapicontext->SteamUser() ) {
			const CSteamID steamid = steamapicontext->SteamUser()->GetSteamID();
			if ( steamid.IsValid() ) {
				const int unTeam = pPlayer ? pPlayer->GetTeamNumber() : 0;
				char szSteamId[ 256 ];
				V_sprintf_safe( szSteamId, "universe:%u;account_type:%u;id:%u;instance:%u;team:%d", steamid.GetEUniverse(), steamid.GetEAccountType(), steamid.GetAccountID(), steamid.GetUnAccountInstance(), unTeam );

				wchar_t wcsSteamId[ 256 ];
				Q_UTF8ToUnicode( szSteamId, wcsSteamId, sizeof( wcsSteamId ) );

				// Identifier which uniquely identifies a certain player in a context.
				V_wcscpy_safe( g_pMumbleMemory->identity, wcsSteamId );

				m_bHasSetPlayerUniqueId = true;
				m_nTeamSetInUniqueId = unTeam;
			}
		}

		// Context should be equal for players which should be able to hear each other positional and
		// differ for those who shouldn't (e.g. it could contain the server+port and team)
		memcpy( g_pMumbleMemory->context, &m_szSteamIDCurrentServer, m_cubSteamIDCurrentServer );
		g_pMumbleMemory->context_len = m_cubSteamIDCurrentServer;
	#endif
}

void CMumbleSystem::FireGameEvent( IGameEvent* event ) {
	#if !defined( NO_STEAM )
		const char* eventname = event->GetName();

		if ( not eventname or not eventname[ 0 ] ) {
			return;
		}

		if ( not Q_strcmp( "server_spawn", eventname ) ) {
			V_strcpy_safe( m_szSteamIDCurrentServer, event->GetString( "steamid", "" ) );
			m_cubSteamIDCurrentServer = strlen( m_szSteamIDCurrentServer ) + 1;
		}
	#endif
}
