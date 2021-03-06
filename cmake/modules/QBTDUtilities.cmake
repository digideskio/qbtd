function(group_sources)
	function(__group_sources__ root prefix)
		file(GLOB entries RELATIVE "${root}" "${root}/*")
		foreach(entry ${entries})
			set(abspath "${root}/${entry}")
			if(IS_DIRECTORY "${abspath}")
				__group_sources__("${abspath}" "${prefix}/${entry}")
			else()
				list(APPEND group "${abspath}")
			endif()
		endforeach()
		file(TO_NATIVE_PATH "${prefix}" prefix)
		source_group("${prefix}" FILES ${group})
	endfunction()

	function(__group_single_root__ root)
		get_filename_component(prefix "${root}" NAME)
		__group_sources__("${root}" "${prefix}")
	endfunction()

	foreach(root ${ARGN})
		__group_single_root__("${root}")
	endforeach()
endfunction()

function(find_moc_headers results)
	foreach(header ${ARGN})
		file(STRINGS ${header} moc_token REGEX "Q_OBJECT")
		if(moc_token)
			list(APPEND tmp ${header})
		endif()
	endforeach()
	set(${results} ${tmp} PARENT_SCOPE)
endfunction()
