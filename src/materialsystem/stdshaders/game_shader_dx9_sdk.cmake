# game_shader_dx9_sdk.cmake

include( "${CMAKE_CURRENT_LIST_DIR}/game_shader_dx9_base.cmake")

add_library(game_shader_dx9_sdk MODULE)

set_target_properties(
	game_shader_dx9_sdk PROPERTIES
	PREFIX ""
	LIBRARY_OUTPUT_DIRECTORY "${GAMEDIR}/mod_sdk/bin"
)

target_use_game_shader_dx9_base(game_shader_dx9_sdk)