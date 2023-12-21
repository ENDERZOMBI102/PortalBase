# vtfdiff.cmake

set(VTFDIFF_DIR ${CMAKE_CURRENT_LIST_DIR})
set(
	VTFDIFF_SOURCE_FILES

	"${VTFDIFF_DIR}/vtfdiff.cpp"
)

add_executable(vtfdiff ${VTFDIFF_SOURCE_FILES})

set_target_properties(
	vtfdiff PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${GAMEDIR}/bin"
)

target_link_libraries(
	vtfdiff PRIVATE
	"${LIBPUBLIC}/bitmap${CMAKE_STATIC_LIBRARY_SUFFIX}"
	mathlib
	"${LIBPUBLIC}/tier2${CMAKE_STATIC_LIBRARY_SUFFIX}"
	"${LIBPUBLIC}/vtf${CMAKE_STATIC_LIBRARY_SUFFIX}"
)