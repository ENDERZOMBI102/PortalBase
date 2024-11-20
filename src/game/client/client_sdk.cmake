# client_sdk.cmake

include("${CMAKE_CURRENT_LIST_DIR}/client_base.cmake")

set(CLIENT_SDK_DIR ${CMAKE_CURRENT_LIST_DIR})
set(
	CLIENT_SDK_SOURCE_FILES

	# Source Files
	"${CLIENT_MIDNIGHT_DIR}/c_team_objectiveresource.cpp"
	"${CLIENT_MIDNIGHT_DIR}/c_team_objectiveresource.h"
	"${CLIENT_MIDNIGHT_DIR}/c_team_train_watcher.cpp"
	"${CLIENT_MIDNIGHT_DIR}/c_team_train_watcher.h"
	"${CLIENT_MIDNIGHT_DIR}/hud_voicestatus.cpp"
	"${SRCDIR}/game/shared/predicted_viewmodel.cpp"
	"${SRCDIR}/game/shared/predicted_viewmodel.h"
	"${SRCDIR}/game/shared/teamplay_round_timer.cpp"
	"${SRCDIR}/game/shared/teamplay_round_timer.h"

	# HL2 DLL
	"${SRCDIR}/game/shared/script_intro_shared.cpp"

	# SDK
	"${CLIENT_MIDNIGHT_DIR}/sdk/c_sdk_env_sparkler.cpp"
	"${CLIENT_MIDNIGHT_DIR}/sdk/c_sdk_player.cpp"
	"${CLIENT_MIDNIGHT_DIR}/sdk/c_sdk_player.h"
	"${CLIENT_MIDNIGHT_DIR}/sdk/c_sdk_team.cpp"
	"${CLIENT_MIDNIGHT_DIR}/sdk/c_sdk_team.h"
	"${CLIENT_MIDNIGHT_DIR}/sdk/c_te_firebullets.cpp"
	"${CLIENT_MIDNIGHT_DIR}/sdk/c_weapon_stubs_midn.cpp"
	"${CLIENT_MIDNIGHT_DIR}/sdk/clientmode_sdk.cpp"
	"${CLIENT_MIDNIGHT_DIR}/sdk/clientmode_sdk.h"
	"${CLIENT_MIDNIGHT_DIR}/sdk/sdk_fx_impacts.cpp"
	"${CLIENT_MIDNIGHT_DIR}/sdk/sdk_fx_weaponfx.cpp"
	"${CLIENT_MIDNIGHT_DIR}/sdk/sdk_hud_ammo.cpp"
	"${CLIENT_MIDNIGHT_DIR}/sdk/sdk_hud_health.cpp"
	"${CLIENT_MIDNIGHT_DIR}/sdk/sdk_hud_message.cpp"
	"${CLIENT_MIDNIGHT_DIR}/sdk/sdk_hud_weaponselection.cpp"
	"${CLIENT_MIDNIGHT_DIR}/sdk/sdk_in_main.cpp"
	"${CLIENT_MIDNIGHT_DIR}/sdk/sdk_prediction.cpp"
	"${SRCDIR}/game/shared/sdk/sdk_gamerules.cpp"
	"${SRCDIR}/game/shared/sdk/sdk_gamerules.h"
	"${SRCDIR}/game/shared/sdk/sdk_player_shared.cpp"
	"${SRCDIR}/game/shared/sdk/sdk_player_shared.h"
	"${SRCDIR}/game/shared/sdk/sdk_weapon_parse.cpp"
	"${SRCDIR}/game/shared/sdk/sdk_weapon_parse.h"
	"${SRCDIR}/game/shared/sdk/sdk_weapon_melee.cpp"
	"${SRCDIR}/game/shared/sdk/sdk_weapon_melee.h"
	"${SRCDIR}/game/shared/sdk/sdk_usermessages.cpp"
	"${SRCDIR}/game/shared/sdk/sdk_playeranimstate.cpp"
	"${SRCDIR}/game/shared/sdk/sdk_playeranimstate.h"
	"${SRCDIR}/game/shared/sdk/sdk_fx_shared.cpp"
	"${SRCDIR}/game/shared/sdk/sdk_fx_shared.h"
	"${SRCDIR}/game/shared/sdk/sdk_basegrenade_projectile.cpp"
	"${SRCDIR}/game/shared/sdk/sdk_basegrenade_projectile.h"
	"${SRCDIR}/game/shared/sdk/sdk_gamemovement.cpp"
	"${SRCDIR}/game/shared/sdk/sdk_playerclass_info_parse.cpp"
	"${SRCDIR}/game/shared/sdk/sdk_playerclass_info_parse.h"
	"${SRCDIR}/game/shared/sdk/sdk_shareddefs.cpp"
	"${SRCDIR}/game/shared/sdk/sdk_shareddefs.h"

	# SDK->vgui
	"${CLIENT_SDK_DIR}/sdk/vgui/sdkviewport.cpp"
	"${CLIENT_SDK_DIR}/sdk/vgui/sdkviewport.h"
	"${CLIENT_SDK_DIR}/sdk/vgui/vgui_rootpanel_sdk.cpp"
	"${CLIENT_SDK_DIR}/sdk/vgui/vgui_rootpanel_sdk.h"

	# SDK->Weapons
	"${SRCDIR}/game/shared/sdk/weapon_sdkbase.cpp"
	"${SRCDIR}/game/shared/sdk/weapon_sdkbase.h"
	"${SRCDIR}/game/shared/sdk/weapon_basesdkgrenade.cpp"
	"${SRCDIR}/game/shared/sdk/weapon_basesdkgrenade.h"
	"${SRCDIR}/game/shared/sdk/weapon_crowbar.cpp"
	"${SRCDIR}/game/shared/sdk/weapon_grenade.cpp"
	"${SRCDIR}/game/shared/sdk/weapon_grenade.h"
	"${SRCDIR}/game/shared/sdk/weapon_mp5.cpp"
	"${SRCDIR}/game/shared/sdk/weapon_pistol.cpp"
	"${SRCDIR}/game/shared/sdk/weapon_shotgun.cpp"
)

set(
	CLIENT_SDK_EXCLUDE_SOURCES
	"${SRCDIR}/game/shared/weapon_parse_default.cpp"
)

add_library(client_sdk MODULE ${CLIENT_SDK_SOURCE_FILES})

set_target_properties(
	client_sdk PROPERTIES
	OUTPUT_NAME "client"
	PREFIX ""
	LIBRARY_OUTPUT_DIRECTORY "${GAMEDIR}/mod_sdk/bin"
)

target_use_client_base(client_sdk CLIENT_SDK_EXCLUDE_SOURCES)

target_include_directories(
	client_sdk PRIVATE
	"${CLIENT_SDK_DIR}/sdk/vgui"
	"${CLIENT_SDK_DIR}/sdk"
	"${SRCDIR}/game/shared/sdk"
)

target_compile_definitions(
	client_sdk PRIVATE
	SDKDLL
)