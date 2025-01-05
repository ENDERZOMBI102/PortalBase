# shaderlib.cmake

set( SHADERLIB_DIR ${CMAKE_CURRENT_LIST_DIR} )
set( SHADERLIB_SOURCE_FILES
	# Sources
	"${SHADERLIB_DIR}/baseshader.cpp"
	"${SHADERLIB_DIR}/shaderdll.cpp"
	"${SHADERLIB_DIR}/shaderdll.hpp"
	"${SHADERLIB_DIR}/shaderdll_global.h"
	"${SHADERLIB_DIR}/shaderlib_cvar.cpp"
	"${SHADERLIB_DIR}/shaderlib_cvar.h"
	"${SHADERLIB_DIR}/ishadersystem.h"

	# Public units

	# Public headers
	"${SRCDIR}/public/shaderlib/BaseShader.h"
	"${SRCDIR}/public/shaderlib/cshader.h"
	"${SRCDIR}/public/shaderlib/ShaderDLL.h"
)

add_library( shaderlib2 STATIC ${SHADERLIB_SOURCE_FILES} )
target_link_libraries( shaderlib2
	PUBLIC
		tier0
		tier1
		mathlib
)
link_to_bin( TARGET shaderlib2 )
declare_library( TARGET shaderlib2 )

add_library( shaderlib IMPORTED STATIC )

set_target_properties( shaderlib
	PROPERTIES
		IMPORTED_LOCATION "${LIBPUBLIC}/shaderlib${CMAKE_STATIC_LIBRARY_SUFFIX}"
)
