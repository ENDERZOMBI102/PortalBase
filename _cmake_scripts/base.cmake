# base.cmake

# Initialize them with default values that we then set later

set(IS_WINDOWS 0)
set(IS_LINUX 0)
set(IS_POSIX 0)

set(IS_SOURCESDK 1)

if (WIN32)
	set(IS_WINDOWS 1)
	set(CMAKE_IMPORT_LIBRARY_SUFFIX ".lib")
elseif (LINUX)
	set(IS_LINUX 1)
	set(CMAKE_IMPORT_LIBRARY_SUFFIX ".so")
endif()

option(RETAIL "Build in retail mode" OFF)
option(STAGING_ONLY "Staging only" OFF)

set(RAD_TELEMETRY_DISABLED ${IS_SOURCESDK})
set(TF_BETA 0)
set(BUILD_REPLAY 0)
set(DEDICATED 0)

add_compile_definitions(
	$<$<BOOL:${RETAIL}>:_RETAIL>
	$<$<BOOL:${STAGING_ONLY}>:STAGING_ONLY>
	$<${TF_BETA}:TF_BETA>
	$<${RAD_TELEMETRY_DISABLED}:RAD_TELEMETRY_DISABLED>
	_DLL_EXT=${CMAKE_SHARED_LIBRARY_SUFFIX}
	FRAME_POINTER_OMISSION_DISABLED
)
