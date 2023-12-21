# tgadiff.cmake

set(TGADIFF_DIR ${CMAKE_CURRENT_LIST_DIR})
set(
	TGADIFF_SOURCE_FILES

	"${TGADIFF_DIR}/tgadiff.cpp"
)

add_executable(tgadiff ${TGADIFF_SOURCE_FILES})

set_target_properties(
	tgadiff PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${GAMEDIR}/bin"
)

target_link_libraries(
	tgadiff PRIVATE
	"${LIBPUBLIC}/bitmap${CMAKE_STATIC_LIBRARY_SUFFIX}"
	mathlib
	"${LIBPUBLIC}/tier2${CMAKE_STATIC_LIBRARY_SUFFIX}"
)