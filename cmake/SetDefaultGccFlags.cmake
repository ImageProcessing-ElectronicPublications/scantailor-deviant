INCLUDE(TestCXXAcceptsFlag)

MACRO(ST_SET_DEFAULT_GCC_FLAGS)
	IF(CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_FLAGS MATCHES "\\s*")
		SET(dead_strip_ldflags_ "")
		SET(gc_sections_cflags_ "")
		SET(gc_sections_ldflags_ "")
		SET(no_inline_dllexport_cflags_ "")
        SET(werror_return_type_cflags_ "")

		CHECK_CXX_ACCEPTS_FLAG(
			"-ffunction-sections -fdata-sections -Wl,--gc-sections"
			gc_sections_supported_
		)
		IF(gc_sections_supported_)
			SET(gc_sections_cflags_ "-ffunction-sections -fdata-sections")
			SET(gc_sections_ldflags_ "-Wl,--gc-sections")
		ENDIF(gc_sections_supported_)

		IF(WIN32)	# disable this chack on linux due this problem:
					# https://askubuntu.com/questions/1288365/pbuilder-dist-fails-if-cmake-cant-find-non-mandatary-c-feature-support
			CHECK_CXX_ACCEPTS_FLAG("-fno-keep-inline-dllexport" no_inline_dllexport_supported_)
			IF(no_inline_dllexport_supported_)
				SET(no_inline_dllexport_cflags_ "-fno-keep-inline-dllexport")
			ENDIF()
		ENDIF(WIN32)
		
		CHECK_CXX_ACCEPTS_FLAG("-Werror=return-type" werror_return_type_supported_)
		IF(werror_return_type_supported_)
            SET(werror_return_type_cflags_ "-Werror=return-type")
		ENDIF()

		IF(MINGW)
			CHECK_CXX_ACCEPTS_FLAG("-shared-libgcc -static-libstdc++" supported_)
			IF(supported_)
				# This is the configuration we want for 32-bit MinGW.
				# Note that the default for libstdc++ recently changed
				# from static to shared. We don't want to bundle
				# another DLL, so we force it back.
				# For 64-bit MinGW, such configuration is invalid and
				# fortunately gets rejected.
				SET(stdlibs_shared_static_ "-shared-libgcc -static-libstdc++")
			ELSE()
				# This configuration is used for 64-bit MinGW.
				SET(stdlibs_shared_static_ "")
			ENDIF()
		ENDIF()	
		
		# GCC on Windows doesn't support -fvisibility, but doesn't reject it either,
		# printing warnings instead.
		IF(NOT WIN32)
			CHECK_CXX_ACCEPTS_FLAG("-fvisibility=hidden" visibility_supported_)
			IF(visibility_supported_)
				SET(visibility_cflags_ "-fvisibility=hidden")
			ELSE(visibility_supported_)
				SET(visibility_cflags_ "")
			ENDIF(visibility_supported_)
		ENDIF()
		
		IF(NOT COMPILER_FLAGS_OVERRIDDEN)
			# Flags common for all build configurations.
			SET(
				CMAKE_CXX_FLAGS
					"-ffast-math ${werror_return_type_cflags_} ${no_inline_dllexport_cflags_} ${stdlibs_shared_static_}"
				CACHE STRING "Common C++ flags for all build configurations." FORCE
			)
		
			# Release
			SET(
				CMAKE_CXX_FLAGS_RELEASE
				"-DNDEBUG -O2 ${visibility_cflags_} ${gc_sections_cflags_}"
				CACHE STRING "C++ flags for Release builds." FORCE
			)
			SET(
				CMAKE_EXE_LINKER_FLAGS_RELEASE
				"${gc_sections_ldflags_} ${dead_strip_ldflags_}"
				CACHE STRING "Link flags for Release builds." FORCE
			)
			
			# MinSizeRel
			SET(
				CMAKE_CXX_FLAGS_MINSIZEREL
				"-DNDEBUG -Os ${visibility_cflags_} ${gc_sections_cflags_}"
				CACHE STRING "C++ flags for MinSizeRel builds." FORCE
			)
			SET(
				CMAKE_EXE_LINKER_FLAGS_MINSIZEREL
				"${gc_sections_ldflags_} ${dead_strip_ldflags_}"
				CACHE STRING "Link flags for MinSizeRel builds." FORCE
			)
			
			# RelWithDebInfo
			SET(
				CMAKE_CXX_FLAGS_RELWITHDEBINFO
				"-DNDEBUG -g -O2 ${visibility_cflags_}"
				CACHE STRING "C++ flags for RelWithDebInfo builds." FORCE
			)
			SET(
				CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO ""
				CACHE STRING "Link flags for RelWithDebInfo builds." FORCE
			)
			
			# Debug
			SET(
				CMAKE_CXX_FLAGS_DEBUG "-DDEBUG -g" CACHE STRING
				"C++ flags for Debug builds." FORCE
			)
			SET(
				CMAKE_EXE_LINKER_FLAGS_DEBUG ""
				CACHE STRING "Link flags for Debug builds." FORCE
			)
			
			SET(COMPILER_FLAGS_OVERRIDDEN YES CACHE INTERNAL "" FORCE)
		
		ENDIF(NOT COMPILER_FLAGS_OVERRIDDEN)
		
	ENDIF(CMAKE_COMPILER_IS_GNUCXX AND AND CMAKE_CXX_FLAGS MATCHES "\\s*")
ENDMACRO(ST_SET_DEFAULT_GCC_FLAGS)
