# statics.cmake

function( add_libpublic_static NAME )
	add_library( ${NAME} IMPORTED STATIC )

	set_target_properties( ${NAME}
		PROPERTIES
			IMPORTED_LOCATION "${LIBPUBLIC}/${NAME}${CMAKE_STATIC_LIBRARY_SUFFIX}"
	)
endfunction()


add_libpublic_static( particles )
add_libpublic_static( dmxloader )
add_libpublic_static( matsys_controls )
add_libpublic_static( vtf )
add_libpublic_static( libz )
