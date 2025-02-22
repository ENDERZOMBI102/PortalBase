# vbsp.cmake

set(VBSP_DIR ${CMAKE_CURRENT_LIST_DIR})
set(
	VBSP_SOURCE_FILES

	"${VBSP_DIR}/boundbox.cpp"
	"${VBSP_DIR}/brushbsp.cpp"
	"${SRCDIR}/public/collisionutils.cpp"
	"${VBSP_DIR}/csg.cpp"
	"${VBSP_DIR}/cubemap.cpp"
	"${VBSP_DIR}/detail.cpp"
	"${VBSP_DIR}/detailobjects.cpp"
	"${SRCDIR}/public/disp_common.cpp"
	"${VBSP_DIR}/disp_ivp.cpp"
	"${SRCDIR}/public/disp_powerinfo.cpp"
	"${VBSP_DIR}/disp_vbsp.cpp"
	"${VBSP_DIR}/faces.cpp"
	"${VBSP_DIR}/glfile.cpp"
	"${VBSP_DIR}/ivp.cpp"
	"${VBSP_DIR}/leakfile.cpp"
	"${SRCDIR}/public/loadcmdline.cpp"
	"${SRCDIR}/public/lumpfiles.cpp"
	"${VBSP_DIR}/map.cpp"
	"${VBSP_DIR}/manifest.cpp"
	"${VBSP_DIR}/materialpatch.cpp"
	"${VBSP_DIR}/materialsub.cpp"
	"${SRCDIR}/utils/common/mstristrip.cpp"
	"${VBSP_DIR}/nodraw.cpp"
	"${VBSP_DIR}/normals.cpp"
	"${VBSP_DIR}/overlay.cpp"
	"${SRCDIR}/utils/common/physdll.cpp"
	"${VBSP_DIR}/portals.cpp"
	"${VBSP_DIR}/prtfile.cpp"
	"${SRCDIR}/public/scratchpad3d.cpp"
	"${SRCDIR}/utils/common/scratchpad_helpers.cpp"
	"${VBSP_DIR}/staticprop.cpp"
	"${VBSP_DIR}/textures.cpp"
	"${VBSP_DIR}/tree.cpp"
	"${SRCDIR}/utils/common/utilmatlib.cpp"
	"${VBSP_DIR}/vbsp.cpp"
	"${VBSP_DIR}/worldvertextransitionfixup.cpp"
	"${VBSP_DIR}/writebsp.cpp"
	"${SRCDIR}/public/zip_utils.cpp"

	# Common Files
	"${SRCDIR}/utils/common/bsplib.cpp"
	"${SRCDIR}/public/builddisp.cpp"
	"${SRCDIR}/public/chunkfile.cpp"
	"${SRCDIR}/utils/common/cmdlib.cpp"
	"${SRCDIR}/public/filesystem_helpers.cpp"
	"${SRCDIR}/public/filesystem_init.cpp"
	"${SRCDIR}/utils/common/filesystem_tools.cpp"
	"${SRCDIR}/utils/common/map_shared.cpp"
	"${SRCDIR}/utils/common/pacifier.cpp"
	"${SRCDIR}/utils/common/polylib.cpp"
	"${SRCDIR}/utils/common/scriplib.cpp"
	"${SRCDIR}/utils/common/threads.cpp"
	"${SRCDIR}/utils/common/tools_minidump.cpp"
	"${SRCDIR}/utils/common/tools_minidump.h"
	"${SRCDIR}/utils/common/tools_stub.cpp"

	# Header Files
	"${VBSP_DIR}/boundbox.h"
	"${VBSP_DIR}/csg.h"
	"${VBSP_DIR}/detail.h"
	"${SRCDIR}/public/disp_powerinfo.h"
	"${VBSP_DIR}/disp_vbsp.h"
	"${SRCDIR}/public/disp_vertindex.h"
	"${VBSP_DIR}/faces.h"
	"${VBSP_DIR}/map.h"
	"${VBSP_DIR}/manifest.h"
	"${VBSP_DIR}/materialpatch.h"
	"${VBSP_DIR}/materialsub.h"
	"${SRCDIR}/utils/common/scratchpad_helpers.h"
	"${VBSP_DIR}/vbsp.h"
	"${VBSP_DIR}/worldvertextransitionfixup.h"
	"${VBSP_DIR}/writebsp.h"

	# Common Header Files
	"${SRCDIR}/utils/common/bsplib.h"
	"${SRCDIR}/public/builddisp.h"
	"${SRCDIR}/public/chunkfile.h"
	"${SRCDIR}/utils/common/cmdlib.h"
	"${VBSP_DIR}/disp_ivp.h"
	"${SRCDIR}/public/filesystem.h"
	"${SRCDIR}/public/filesystem_helpers.h"
	"${SRCDIR}/utils/common/filesystem_tools.h"
	"${SRCDIR}/public/gamebspfile.h"
	"${SRCDIR}/public/tier1/interface.h"
	"${VBSP_DIR}/ivp.h"
	"${SRCDIR}/utils/common/map_shared.h"
	"${SRCDIR}/utils/common/pacifier.h"
	"${SRCDIR}/utils/common/polylib.h"
	"${SRCDIR}/public/tier1/tokenreader.h"
	"${SRCDIR}/utils/common/utilmatlib.h"
	"${SRCDIR}/public/zip_uncompressed.h"

	# Public Headers
	"${SRCDIR}/public/mathlib/amd3dx.h"
	"${SRCDIR}/public/arraystack.h"
	"${SRCDIR}/public/tier0/basetypes.h"
	"${SRCDIR}/public/bspfile.h"
	"${SRCDIR}/public/bspflags.h"
	"${SRCDIR}/public/bsptreedata.h"
	"${SRCDIR}/public/mathlib/bumpvects.h"
	"${SRCDIR}/public/tier1/byteswap.h"
	"${SRCDIR}/public/cmodel.h"
	"${SRCDIR}/public/collisionutils.h"
	"${SRCDIR}/public/tier0/commonmacros.h"
	"${SRCDIR}/public/tier0/dbg.h"
	"${SRCDIR}/public/disp_common.h"
	"${SRCDIR}/public/iscratchpad3d.h"
	"${SRCDIR}/public/mathlib/mathlib.h"
	"${SRCDIR}/utils/common/mstristrip.h"
	"${SRCDIR}/public/nmatrix.h"
	"${SRCDIR}/public/ntree.h"
	"${SRCDIR}/public/nvector.h"
	"${SRCDIR}/public/phyfile.h"
	"${SRCDIR}/utils/common/physdll.h"
	"${SRCDIR}/utils/common/qfiles.h"
	"${SRCDIR}/public/scratchpad3d.h"
	"${SRCDIR}/utils/common/scriplib.h"
	"${SRCDIR}/public/studio.h"
	"${SRCDIR}/utils/common/threads.h"
	"${SRCDIR}/public/tier1/utlbuffer.h"
	"${SRCDIR}/public/tier1/utllinkedlist.h"
	"${SRCDIR}/public/tier1/utlmemory.h"
	"${SRCDIR}/public/tier1/utlrbtree.h"
	"${SRCDIR}/public/tier1/utlsymbol.h"
	"${SRCDIR}/public/tier1/utlvector.h"
	"${SRCDIR}/public/vcollide.h"
	"${SRCDIR}/public/mathlib/vector.h"
	"${SRCDIR}/public/mathlib/vector2d.h"
	"${SRCDIR}/public/mathlib/vector4d.h"
	"${SRCDIR}/public/mathlib/vmatrix.h"
	"${SRCDIR}/public/vphysics_interface.h"
	"${SRCDIR}/public/mathlib/vplane.h"
	"${SRCDIR}/public/wadtypes.h"
	"${SRCDIR}/public/worldsize.h"
)

add_executable(vbsp ${VBSP_SOURCE_FILES})

set_target_properties(vbsp
	PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY "${GAMEDIR}/bin"
)

target_include_directories(vbsp
	PRIVATE
		"${SRCDIR}/utils/common"
)

target_compile_definitions(vbsp
	PRIVATE
		MACRO_MATHLIB
		PROTECTED_THINGS_DISABLE
)

target_link_libraries(vbsp
	PRIVATE
		$<${IS_WINDOWS}:ws2_32>
		$<${IS_WINDOWS}:odbc32>
		$<${IS_WINDOWS}:odbccp32>
		$<${IS_WINDOWS}:winmm>

		fgdlib
		lzma
		mathlib
		tier2
		vtf
		bitmap
		tier1
		tier0
		vstdlib
#		OpengGL
	INTERFACE
		SDL3::SDL3-shared
)