set( CMAKE_SYSTEM_NAME "Linux" )
set( CMAKE_SYSTEM_PROCESSOR "i386" )

set( CMAKE_C_COMPILER_TARGET "i386-linux" )
set( CMAKE_CXX_COMPILER_TARGET "i386-linux" )

# gcc needs special treatment to target x86
string( FIND "${CMAKE_C_COMPILER}" "gcc" GCC_POS )
string( FIND "${CMAKE_CXX_COMPILER}" "g++" GXX_POS )
if ( NOT (GCC_POS EQUAL -1 AND GXX_POS EQUAL -1) )
	set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32" )
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32" )
endif ()
