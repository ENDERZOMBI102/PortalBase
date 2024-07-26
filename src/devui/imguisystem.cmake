# imguisystem.cmake

set( IMGUISYSTEM_DIR ${CMAKE_CURRENT_LIST_DIR} )
set(
	IMGUISYSTEM_SOURCE_FILES
		"${IMGUISYSTEM_DIR}/imgui_system.cpp"
		"${IMGUISYSTEM_DIR}/imgui_system.hpp"
		"${IMGUISYSTEM_DIR}/imgui_impl_source.cpp"
		"${IMGUISYSTEM_DIR}/imgui_impl_source.hpp"
		"${IMGUISYSTEM_DIR}/imgui_window.hpp"
		"${IMGUISYSTEM_DIR}/imconfig_source.hpp"

		# files from imgui proper because we need a custom config that pulls in source stuff
		"${THIRDPARTYDIR}/imgui/imgui.cpp"
		"${THIRDPARTYDIR}/imgui/imgui_demo.cpp"
		"${THIRDPARTYDIR}/imgui/imgui_draw.cpp"
		"${THIRDPARTYDIR}/imgui/imgui_tables.cpp"
		"${THIRDPARTYDIR}/imgui/imgui_widgets.cpp"
)

# don't enable precompiled headers here
set_source_files_properties(
	"${THIRDPARTYDIR}/imgui/imgui.cpp"
	"${THIRDPARTYDIR}/imgui/imgui_demo.cpp"
	"${THIRDPARTYDIR}/imgui/imgui_draw.cpp"
	"${THIRDPARTYDIR}/imgui/imgui_tables.cpp"
	"${THIRDPARTYDIR}/imgui/imgui_widgets.cpp"
	PROPERTIES SKIP_PRECOMPILE_HEADERS ON
)

add_library( devui STATIC ${IMGUISYSTEM_SOURCE_FILES} )

target_include_directories( devui
	PRIVATE
		"${SRCDIR}/common"
		"${SRCDIR}/public"
)

target_include_directories( devui
	PUBLIC
		"${IMGUISYSTEM_DIR}"
		"${THIRDPARTYDIR}/imgui"
)

target_compile_definitions( devui
	PRIVATE
		IMGUI_DISABLE_INCLUDE_IMCONFIG_H
		IMGUI_USER_CONFIG="imconfig_source.hpp"
)

target_link_libraries( devui
	PRIVATE
		appframework
		tier0
		tier1
		vstdlib
		tier2
)