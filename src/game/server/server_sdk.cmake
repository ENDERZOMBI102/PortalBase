# server_sdk.cmake

include("${CMAKE_CURRENT_LIST_DIR}/server_base.cmake")

if (${IS_SOURCESDK})
	include("${CMAKE_CURRENT_LIST_DIR}/nav_mesh.cmake")
endif()

set(SERVER_SDK_DIR ${CMAKE_CURRENT_LIST_DIR})
set(
	SERVER_SDK_SOURCE_FILES

	"${SERVER_SDK_DIR}/ai_relationship.cpp"
	"${SERVER_SDK_DIR}/basegrenade_concussion.cpp"
	"${SERVER_SDK_DIR}/basegrenade_contact.cpp"
	"${SERVER_SDK_DIR}/basegrenade_timed.cpp"
	"${SERVER_SDK_DIR}/EntityFlame.h"
	"${SERVER_SDK_DIR}/hl2/Func_Monitor.cpp"
	"${SERVER_SDK_DIR}/grenadethrown.cpp"
	"${SERVER_SDK_DIR}/grenadethrown.h"
	"${SERVER_SDK_DIR}/h_cycler.cpp"
	"${SERVER_SDK_DIR}/h_cycler.h"
	"${SERVER_SDK_DIR}/monstermaker.cpp"
	"${SERVER_SDK_DIR}/monstermaker.h"
	"${SERVER_SDK_DIR}/physics_bone_follower.h"
	"${SRCDIR}/game/shared/predicted_viewmodel.cpp"
	"${SRCDIR}/game/shared/predicted_viewmodel.h"
	"${SRCDIR}/game/shared/ragdoll_shared.h"
	"${SRCDIR}/game/shared/solidsetdefaults.h"
	"${SRCDIR}/game/shared/hl2/survival_gamerules.cpp"
	"${SERVER_SDK_DIR}/team_objectiveresource.cpp"
	"${SERVER_SDK_DIR}/team_objectiveresource.h"
	"${SERVER_SDK_DIR}/team_spawnpoint.cpp"
	"${SERVER_SDK_DIR}/team_spawnpoint.h"
	"${SERVER_SDK_DIR}/team_control_point.cpp"
	"${SERVER_SDK_DIR}/team_control_point.h"
	"${SERVER_SDK_DIR}/team_control_point_master.cpp"
	"${SERVER_SDK_DIR}/team_control_point_master.h"
	"${SERVER_SDK_DIR}/team_control_point_round.cpp"
	"${SERVER_SDK_DIR}/team_control_point_round.h"
	"${SERVER_SDK_DIR}/team_train_watcher.cpp"
	"${SERVER_SDK_DIR}/team_train_watcher.h"
	"${SRCDIR}/game/shared/teamplayroundbased_gamerules.cpp"
	"${SRCDIR}/game/shared/touchlink.h"
	"${SERVER_SDK_DIR}/trigger_area_capture.cpp"
	"${SERVER_SDK_DIR}/trigger_area_capture.h"
	"${SRCDIR}/game/shared/teamplay_round_timer.cpp"
	"${SRCDIR}/game/shared/teamplay_round_timer.h"

	# HL2 DLL
	"${SERVER_SDK_DIR}/npc_talker.cpp"
	"${SERVER_SDK_DIR}/npc_talker.h"
	"${SRCDIR}/game/shared/script_intro_shared.cpp"
	"${SERVER_SDK_DIR}/weapon_cubemap.cpp"

	# SDK
	"${SERVER_SDK_DIR}/sdk/sdk_bot_temp.cpp"
	"${SERVER_SDK_DIR}/sdk/sdk_bot_temp.h"
	"${SERVER_SDK_DIR}/sdk/sdk_client.cpp"
	"${SERVER_SDK_DIR}/sdk/sdk_gameinterface.cpp"
	#"${SERVER_SDK_DIR}/sdk/sdk_gameinterface.h"

	# Entities
	"${SERVER_SDK_DIR}/sdk/sdk_logicalentity.cpp"
	"${SERVER_SDK_DIR}/sdk/sdk_modelentity.cpp"
	"${SERVER_SDK_DIR}/sdk/sdk_brushentity.cpp"
	"${SERVER_SDK_DIR}/sdk/sdk_vehicle_jeep.cpp"
	"${SERVER_SDK_DIR}/sdk/sdk_env_message.cpp"
	"${SERVER_SDK_DIR}/sdk/sdk_env_sparkler.cpp"

	"${SERVER_SDK_DIR}/sdk/sdk_team.cpp"
	"${SERVER_SDK_DIR}/sdk/sdk_team.h"
	"${SERVER_SDK_DIR}/sdk/sdk_eventlog.cpp"
	
	"${SRCDIR}/game/shared/sdk/sdk_gamerules.cpp"
	"${SRCDIR}/game/shared/sdk/sdk_gamerules.h"

	# Player
	"${SERVER_SDK_DIR}/sdk/sdk_player.cpp"
	"${SERVER_SDK_DIR}/sdk/sdk_player.h"
	"${SERVER_SDK_DIR}/sdk/sdk_playermove.cpp"
	"${SRCDIR}/game/shared/sdk/sdk_player_shared.cpp"
	"${SRCDIR}/game/shared/sdk/sdk_player_shared.h"
	"${SRCDIR}/game/shared/sdk/sdk_playerclass_info_parse.cpp"
	"${SRCDIR}/game/shared/sdk/sdk_playerclass_info_parse.h"
	"${SRCDIR}/game/shared/sdk/sdk_playeranimstate.cpp"
	"${SRCDIR}/game/shared/sdk/sdk_playeranimstate.h"

	"${SRCDIR}/game/shared/sdk/sdk_gamemovement.cpp"
	"${SRCDIR}/game/shared/sdk/sdk_usermessages.cpp"

	# weapon code
	"${SRCDIR}/game/shared/sdk/sdk_weapon_parse.cpp"
	"${SRCDIR}/game/shared/sdk/sdk_weapon_parse.h"
	"${SRCDIR}/game/shared/sdk/sdk_weapon_melee.cpp"
	"${SRCDIR}/game/shared/sdk/sdk_weapon_melee.h"
	
	"${SRCDIR}/game/shared/sdk/sdk_fx_shared.cpp"
	"${SRCDIR}/game/shared/sdk/sdk_fx_shared.h"
	"${SRCDIR}/game/shared/sdk/sdk_basegrenade_projectile.cpp"
	"${SRCDIR}/game/shared/sdk/sdk_basegrenade_projectile.h"
	"${SRCDIR}/game/shared/sdk/sdk_shareddefs.cpp"
	"${SRCDIR}/game/shared/sdk/sdk_shareddefs.h"

	# SDK->Weapons
	"${SERVER_SDK_DIR}/sdk/te_firebullets.cpp"
	"${SERVER_SDK_DIR}/sdk/te_firebullets.h"
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
	SERVER_SDK_EXCLUDE_SOURCES
)

add_library(server_sdk MODULE ${SERVER_SDK_SOURCE_FILES})

set_target_properties(
	server_sdk PROPERTIES
	OUTPUT_NAME "server"
	PREFIX ""
	LIBRARY_OUTPUT_DIRECTORY "${GAMEDIR}/mod_sdk/bin"
)

target_use_server_base(server_sdk SERVER_SDK_EXCLUDE_SOURCES)

target_include_directories(
	server_sdk PRIVATE
	"${SERVER_SDK_DIR}/sdk"
	"${SRCDIR}/game/shared/sdk"
)

target_compile_definitions(
	server_sdk PRIVATE
	SDKDLL
)

if (${IS_SOURCESDK})
	target_use_nav_mesh(server_sdk)
endif()