# posix_base.cmake
message( NOTICE "Platform: POSIX" )

string( REPLACE "-O3" "-O2" CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}" )
string( REPLACE "-O3" "-O2" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}" )

find_package( Threads REQUIRED )

add_compile_options(
	-g
	-m32
	$<$<COMPILE_LANGUAGE:CXX>:-fpermissive>
	-Werror=return-type
	-fdiagnostics-color
	# TODO: Enable these when ready
	#-Werror=conversion
	#-Wconversion
	-Wno-narrowing
	$<$<COMPILE_LANGUAGE:CXX>:-Winvalid-offsetof>
	$<${IS_LINUX}:-U_FORTIFY_SOURCE>
	-Usprintf
	-Ustrncpy
	-UPROTECTED_THINGS_ENABLE
	# roughly equivalent to `isLinux && compilers[cxx].id == "GNU"`
	$<$<COMPILE_LANGUAGE:CXX>:$<${IS_LINUX}:$<$<CXX_COMPILER_ID:GNU>:-fabi-compat-version=2>>>
	-Bsymbolic
)

add_link_options(
	-m32
	"LINKER:-rpath,\$ORIGIN" # FIXME: Dynamic Linker runtime on linux is fucked and doesn't load `.so`s from the exe's folder...
)


add_compile_definitions(
	$<$<CXX_COMPILER_ID:GNU>:COMPILER_GCC=1>
	NO_HOOK_MALLOC
	NO_MALLOC_OVERRIDE
	$<${IS_LINUX}:_LINUX>
	$<${IS_LINUX}:LINUX>
)

if ( ${IS_LINUX} )
	if ( NOT ${DEDICATED} )
		list( APPEND ADDITIONAL_LINK_OPTIONS_EXE
			-Wl,--no-as-needed -ltcmalloc_minimal -Wl,--as-needed
		)
	endif()

	# Helps us catch any linker errors from out of order linking or in general
	list( APPEND ADDITIONAL_LINK_OPTIONS_DLL
		-Wl,--no-undefined
	)
endif()

link_libraries(
	Threads::Threads
	${CMAKE_DL_LIBS}
	$<${IS_LINUX}:m>
)

if ( ${IS_LINUX} )
	add_link_options(
		-static-libgcc
		-static-libstdc++
	)
endif()

add_compile_options(
	$<${IS_LINUX}:-march=pentium4>
	-msse2 -mfpmath=sse -mtune=core2
)

list( APPEND ADDITIONAL_LINK_LIBRARIES_DLL
#	tier0
#	tier1
#	vstdlib
)
