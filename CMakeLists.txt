CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)

IF(POLICY CMP0043)
        # Don't ignore COMPILE_DEFINITIONS_{CONFIG} directives, as these are used
	# by Qt.
	CMAKE_POLICY(SET CMP0043 OLD)
ENDIF()

IF (POLICY CMP0071)
        CMAKE_POLICY(SET CMP0071 OLD)
ENDIF()

IF (POLICY CMP0072)
        CMAKE_POLICY(SET CMP0072 OLD)
ENDIF()

SET(
        CMAKE_USER_MAKE_RULES_OVERRIDE
        "${CMAKE_SOURCE_DIR}/cmake/default_cflags.cmake"
)
SET(
        CMAKE_USER_MAKE_RULES_OVERRIDE_CXX
        "${CMAKE_SOURCE_DIR}/cmake/default_cxxflags.cmake"
)

PROJECT("Scan Tailor Deviant")

IF(MSVC)
        # Disable checked iterators for extra performance.
        ADD_DEFINITIONS(-D_SECURE_SCL=0)
ENDIF()

IF(DEBUG_CLI)
        ADD_DEFINITIONS(-DDEBUG_CLI)
ENDIF(DEBUG_CLI)

OPTION(ENABLE_TESTS "Enable testing. For debug purpose only. Do not enable if you are not a programmer." OFF)
IF(ENABLE_TESTS)
        ENABLE_TESTING()
ENDIF()

# An undocumented side-effect of CONFIGURE_FILE() is that it makes
# the whole project depend on the file we are parsing / copying.
CONFIGURE_FILE(
        "${PROJECT_SOURCE_DIR}/version.h"
        "${PROJECT_BINARY_DIR}/.version.h" COPYONLY
)

# Prevent this leftover from old builds to be used in favour
# of the one in ${PROJECT_SOURCE_DIR}
IF(NOT "${PROJECT_BINARY_DIR}" STREQUAL "${PROJECT_SOURCE_DIR}")
        FILE(REMOVE "${PROJECT_BINARY_DIR}/version.h")
ENDIF()

# Extract VERSION and VERSION_QUAD from version.h
FILE(READ "${PROJECT_SOURCE_DIR}/version.h" version_h_contents)
STRING(
        REGEX REPLACE
        ".*#define[ \\t]+VERSION[ \\t]+\"([^\"]*)\".*"
        "\\1" VERSION "${version_h_contents}"
)
IF("${VERSION}" STREQUAL "${version_h_contents}")
        MESSAGE(FATAL_ERROR "Failed to extract VERSION from version.h")
ENDIF()

# VERSION_QUAD must be either empty or be in the form of X.Y.Z.Y
STRING(
        REGEX REPLACE
        ".*#define[ \\t]+VERSION_QUAD[ \\t]+\"(([0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+)?)\".*"
        "\\1" VERSION_QUAD "${version_h_contents}"
)
IF("${VERSION_QUAD}" STREQUAL "${version_h_contents}")
        MESSAGE(FATAL_ERROR "Failed to extract VERSION_QUAD from version.h")
ENDIF()

# For config.h
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}")

INCLUDE(CheckIncludeFile)
INCLUDE(CheckIncludeFileCXX)
INCLUDE(TestCXXAcceptsFlag)
INCLUDE(cmake/FindPthreads.cmake)
IF(UNIX AND NOT APPLE)
INCLUDE(cmake/FindCanberra.cmake)
ENDIF(UNIX AND NOT APPLE)
INCLUDE(cmake/SetDefaultBuildType.cmake)
INCLUDE(cmake/SetDefaultGccFlags.cmake)
INCLUDE(cmake/UpdateTranslations.cmake)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

ST_SET_DEFAULT_BUILD_TYPE(Release)
IF(CMAKE_COMPILER_IS_GNUCC)
        ST_SET_DEFAULT_GCC_FLAGS()
ENDIF(CMAKE_COMPILER_IS_GNUCC)

GET_FILENAME_COMPONENT(source_outer_dir "${PROJECT_SOURCE_DIR}/.." ABSOLUTE)
GET_FILENAME_COMPONENT(build_outer_dir "${PROJECT_BINARY_DIR}/.." ABSOLUTE)

INCLUDE(FindOpenGL)
SET(use_opengl OFF)
IF(OPENGL_FOUND AND QT_QTOPENGL_FOUND)
        # Disabled for now due to various problems associated with it.
        # SET(use_opengl ON)
ENDIF()
OPTION(ENABLE_OPENGL "OpenGL may be used for UI acceleration" ${use_opengl})

SET(CMAKE_AUTOMOC ON)
SET(Qt_MIN_VERSION 5.7.0)
IF(APPLE)
   # hardcoded path to Qt5CoreConfig.cmake etc. [truf]
   SET(CMAKE_PREFIX_PATH "/Users/robert/build/Qt/5.8/clang_64/")
ENDIF(APPLE)
FIND_PACKAGE(Qt5Core ${Qt_MIN_VERSION} REQUIRED)
FIND_PACKAGE(Qt5Gui ${Qt_MIN_VERSION} REQUIRED)
FIND_PACKAGE(Qt5Widgets ${Qt_MIN_VERSION} REQUIRED)
FIND_PACKAGE(Qt5Xml ${Qt_MIN_VERSION} REQUIRED)
FIND_PACKAGE(Qt5Network ${Qt_MIN_VERSION} REQUIRED)
FIND_PACKAGE(Qt5Svg ${Qt_MIN_VERSION} REQUIRED)
FIND_PACKAGE(Qt5LinguistTools ${Qt_MIN_VERSION} REQUIRED)
IF(ENABLE_OPENGL)
        FIND_PACKAGE(Qt5OpenGL ${Qt_MIN_VERSION} REQUIRED)
ENDIF()

SET(EXTRA_LIBS "")

FIND_PACKAGE(JPEG REQUIRED)
INCLUDE_DIRECTORIES("${JPEG_INCLUDE_DIRS}")
LIST(APPEND EXTRA_LIBS ${JPEG_LIBRARIES})

OPTION(ENABLE_OPENJPEG "OpenJPEG library may be used to support JPEG 2000 (.jp2) images." ON)
IF (ENABLE_OPENJPEG)
        find_package(OPENJPEG REQUIRED)
        INCLUDE_DIRECTORIES("${OPENJPEG_INCLUDE_DIRS}")
        LIST(APPEND EXTRA_LIBS ${OPENJPEG_LIBRARIES})
ENDIF(ENABLE_OPENJPEG)

FIND_PACKAGE(ZLIB REQUIRED)
INCLUDE_DIRECTORIES("${ZLIB_INCLUDE_DIRS}")
LIST(APPEND EXTRA_LIBS ${ZLIB_LIBRARIES})

FIND_PACKAGE(PNG REQUIRED)
INCLUDE_DIRECTORIES("${PNG_INCLUDE_DIRS}")
LIST(APPEND EXTRA_LIBS ${PNG_LIBRARIES})

FIND_PACKAGE(TIFF REQUIRED)
INCLUDE_DIRECTORIES("${TIFF_INCLUDE_DIRS}")
LIST(APPEND EXTRA_LIBS ${TIFF_LIBRARIES})
IF(WIN32)
        ADD_DEFINITIONS(-DUSE_LIBTIFF_DLL)
ENDIF()

FIND_PACKAGE(Eigen3 REQUIRED)
INCLUDE_DIRECTORIES(${EIGEN3_INCLUDE_DIR})
ADD_COMPILE_OPTIONS(-DEIGEN_MPL2_ONLY)
ADD_DEFINITIONS(-DEIGEN_DONT_ALIGN_STATICALLY)

IF(WIN32)
        SET(Boost_USE_STATIC_LIBS ON)
ELSE(WIN32)
        ADD_DEFINITIONS(-DBOOST_TEST_DYN_LINK)
ENDIF(WIN32)

ADD_DEFINITIONS(-DBOOST_MULTI_INDEX_DISABLE_SERIALIZATION)
ADD_DEFINITIONS(-DBOOST_BIND_GLOBAL_PLACEHOLDERS)

SET(Boost_USE_MULTITHREADED ON)
IF(ENABLE_TESTS)
        FIND_PACKAGE(Boost 1.35.0 COMPONENTS unit_test_framework prg_exec_monitor REQUIRED)
ELSE()
        FIND_PACKAGE(Boost 1.35.0 REQUIRED)
ENDIF()
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

OPTION(ENABLE_EXIV2 "Exiv2 library may be used to manage image metadata." ON)
IF (ENABLE_EXIV2)
        find_package(EXIV2 REQUIRED)
        INCLUDE_DIRECTORIES("${EXIV2_INCLUDE_DIRS}")
        LIST(APPEND EXTRA_LIBS ${EXIV2_LIBRARIES})
        ADD_DEFINITIONS(-DHAVE_EXIV2)
ENDIF()

IF(UNIX)
        FindPthreads()
	IF(PTHREADS_FOUND)
	        ADD_DEFINITIONS(${PTHREADS_CFLAGS})
		LINK_LIBRARIES(${PTHREADS_LIBS})
		ELSE(PTHREADS_FOUND)
		MESSAGE(
		        FATAL_ERROR
			"Could not detect threading flags.\n"
			"Try specifying them manually in PTHREADS_CFLAGS and PTHREADS_LIBS."
			)
	ENDIF(PTHREADS_FOUND)
ELSEIF(WIN32)
        ADD_DEFINITIONS(-DNOMINMAX)
ENDIF(UNIX)

IF(CANBERRA_FOUND)
        ADD_DEFINITIONS(-DHAVE_CANBERRA)
        INCLUDE_DIRECTORIES("${CANBERRA_INCLUDE_DIRS}")
        LIST(APPEND EXTRA_LIBS ${CANBERRA_LIBRARIES})
ENDIF(CANBERRA_FOUND)


CHECK_INCLUDE_FILE(stdint.h HAVE_STDINT_H)
IF(NOT HAVE_STDINT_H)
        CONFIGURE_FILE(compat/pstdint.h "${CMAKE_CURRENT_BINARY_DIR}/stdint.h" COPYONLY)
        INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}")
ELSE(NOT HAVE_STDINT_H)
        FILE(REMOVE "${CMAKE_CURRENT_BINARY_DIR}/stdint.h")
ENDIF(NOT HAVE_STDINT_H)


IF(WIN32)
   LIST(APPEND EXTRA_LIBS "ws2_32.lib")
ENDIF(WIN32)


SET(MAYBE_QT_OPENGL_MODULE "")
IF(ENABLE_OPENGL)
        LIST(APPEND EXTRA_LIBS ${OPENGL_LIBRARIES})
        SET(MAYBE_QT_OPENGL_MODULE OpenGL)
ENDIF()

# Prepare config.h
IF(WIN32)
        SET(TRANSLATIONS_DIR_REL "translations")
        SET(STYLESHEETS_DIR_REL "stylesheets")
ELSEIF(APPLE)
        SET(TRANSLATIONS_DIR_REL "../Resources/translations")
        SET(STYLESHEETS_DIR_REL "../Resources/stylesheets")
ELSE()
        SET(TRANSLATIONS_DIR_REL "share/scantailor-deviant/translations")
        SET(STYLESHEETS_DIR_REL "share/scantailor-deviant/stylesheets")
        SET(PIXMAPS_DIR_ABS "${CMAKE_INSTALL_PREFIX}/share/scantailor-deviant/stylesheets")
ENDIF()
SET(TRANSLATIONS_DIR_ABS "${CMAKE_INSTALL_PREFIX}/${TRANSLATIONS_DIR_REL}")
SET(STYLESHEETS_DIR_ABS "${CMAKE_INSTALL_PREFIX}/${STYLESHEETS_DIR_REL}")


CONFIGURE_FILE(config.h.in ${CMAKE_BINARY_DIR}/config.h @ONLY)


ADD_SUBDIRECTORY(src)


IF(NOT WIN32 AND NOT APPLE)
        INSTALL(FILES debian/scantailor-deviant.svg DESTINATION "share/pixmaps/")
        INSTALL(FILES debian/scantailor-deviant.desktop DESTINATION "share/applications/")
ENDIF(NOT WIN32 AND NOT APPLE)

IF(WIN32)
        # Install Runtime
        set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION .)
	INCLUDE(InstallRequiredSystemLibraries)

        # Install Qt
        SET(
                qt5_libs
                ${Qt5Widgets_LIBRARIES} ${Qt5Gui_LIBRARIES}
                ${Qt5Core_LIBRARIES} ${Qt5Xml_LIBRARIES}  ${Qt5Svg_LIBRARIES}
        )
        IF(ENABLE_OPENGL)
                LIST(APPEND qt5_libs ${Qt5OpenGL_LIBRARIES})
        ENDIF()
        FOREACH(target ${qt5_libs})
                GET_TARGET_PROPERTY(debug_loc "${target}" LOCATION_DEBUG)
                GET_TARGET_PROPERTY(release_loc "${target}" LOCATION_RELEASE)
                install(FILES "${debug_loc}"   DESTINATION . CONFIGURATIONS Debug)
                install(FILES "${release_loc}" DESTINATION . CONFIGURATIONS Release MinSizeRel RelWithDebInfo)
        ENDFOREACH()

        # Install Qt's plugins.
        CMAKE_PATH(SET Qt5_DIR NORMALIZE "${Qt5Core_DIR}/../../..")
        CMAKE_PATH(APPEND Qt5_DIR "plugins" OUTPUT_VARIABLE PLUGINS_DIR)

        # Install system plugin
        install(FILES "${PLUGINS_DIR}/platforms/qwindows.dll"  DESTINATION platforms CONFIGURATIONS Release MinSizeRel RelWithDebInfo)
        install(FILES "${PLUGINS_DIR}/platforms/qwindowsd.dll" DESTINATION platforms CONFIGURATIONS Debug)

        # Install style plugin
        install(FILES "${PLUGINS_DIR}/styles/qwindowsvistastyle.dll"  DESTINATION styles CONFIGURATIONS Release MinSizeRel RelWithDebInfo)
        install(FILES "${PLUGINS_DIR}/styles/qwindowsvistastyled.dll" DESTINATION styles CONFIGURATIONS Debug)
        
        # Install accessibility plugin
        IF (Qt5Widgets_VERSION VERSION_LESS 5.4.0)
                install(FILES "${PLUGINS_DIR}/accessible/qtaccessiblewidgets.dll"  DESTINATION accessible CONFIGURATIONS Release MinSizeRel RelWithDebInfo)
                install(FILES "${PLUGINS_DIR}/accessible/qtaccessiblewidgetsd.dll" DESTINATION accessible CONFIGURATIONS Debug)
        ENDIF()

        # Install image format plugins
        install(FILES "${PLUGINS_DIR}/imageformats/qjpeg.dll"  DESTINATION imageformats CONFIGURATIONS Release MinSizeRel RelWithDebInfo)
        install(FILES "${PLUGINS_DIR}/imageformats/qjpegd.dll" DESTINATION imageformats CONFIGURATIONS Debug)

        install(FILES "${PLUGINS_DIR}/imageformats/qtiff.dll"  DESTINATION imageformats CONFIGURATIONS Release MinSizeRel RelWithDebInfo)
        install(FILES "${PLUGINS_DIR}/imageformats/qtiffd.dll" DESTINATION imageformats CONFIGURATIONS Debug)

        # Install image format plugins required to display images in stylesheets
        install(FILES "${PLUGINS_DIR}/imageformats/qico.dll"   DESTINATION imageformats CONFIGURATIONS Release MinSizeRel RelWithDebInfo)
        install(FILES "${PLUGINS_DIR}/imageformats/qicod.dll"  DESTINATION imageformats CONFIGURATIONS Debug)

        install(FILES "${PLUGINS_DIR}/imageformats/qsvg.dll"   DESTINATION imageformats CONFIGURATIONS Release MinSizeRel RelWithDebInfo)
        install(FILES "${PLUGINS_DIR}/imageformats/qsvgd.dll"  DESTINATION imageformats CONFIGURATIONS Debug)

        install(FILES "${PLUGINS_DIR}/iconengines/qsvgicon.dll"  DESTINATION iconengines CONFIGURATIONS Release MinSizeRel RelWithDebInfo)
        install(FILES "${PLUGINS_DIR}/iconengines/qsvgicond.dll" DESTINATION iconengines CONFIGURATIONS Debug)

        # Install stylesheets
        install(DIRECTORY "${CMAKE_SOURCE_DIR}/src/stylesheets"
                DESTINATION . 
                PATTERN "CMakeLists.txt" EXCLUDE
                PATTERN ".gitignore" EXCLUDE
                PATTERN ".git" EXCLUDE)

        # Install libpng
        FIND_FILE(LIBPNG_DLL
                NAMES libpng.dll libpng16.dll
                PATHS ${CMAKE_PREFIX_PATH}
                PATH_SUFFIXES bin lib
                NO_DEFAULT_PATH)
        IF(NOT LIBPNG_DLL)
                MESSAGE(SEND_ERROR "Could not find LIBPNG dll. You may specify it manually.")
        ENDIF()
        install(FILES "${LIBPNG_DLL}"  DESTINATION .)

        # Install libjpeg
        FIND_FILE(LIBJPEG_DLL
                NAMES libjpeg.dll jpeg.dll jpeg62.dll libjpeg-62.dll
                PATHS ${CMAKE_PREFIX_PATH}
                PATH_SUFFIXES bin lib
                NO_DEFAULT_PATH)
        IF(NOT LIBJPEG_DLL)
                MESSAGE(SEND_ERROR "Could not find LIBJPEG dll. You may specify it manually.")
        ENDIF()
        install(FILES "${LIBJPEG_DLL}"  DESTINATION .)

        # Install libtigg
        FIND_FILE(LIBTIFF_DLL
                NAMES libtiff.dll tiff.dll
                PATHS ${CMAKE_PREFIX_PATH}
                PATH_SUFFIXES bin lib
                NO_DEFAULT_PATH)
        IF(NOT LIBTIFF_DLL)
                MESSAGE(SEND_ERROR "Could not find LIBTIFF dll. You may specify it manually.")
        ENDIF()
        install(FILES "${LIBTIFF_DLL}"  DESTINATION .)

        # Install zlib
        FIND_FILE(ZLIB_DLL
                NAMES zdll.dll zlib.dll zlib1.dll z.dll libzlib.dll
                PATHS ${CMAKE_PREFIX_PATH}
                PATH_SUFFIXES bin lib
                NO_DEFAULT_PATH)
        IF(NOT ZLIB_DLL)
                MESSAGE(SEND_ERROR "Could not find ZLIB dll. You may specify it manually.")
        ENDIF()
        install(FILES "${ZLIB_DLL}"  DESTINATION .)

        # Install openjpeg
        IF(ENABLE_OPENJPEG)
                FIND_FILE(OPENJPEG_DLL
                        NAMES openjp2.dll libopenjp2.dll
                        PATHS ${CMAKE_PREFIX_PATH}
                        PATH_SUFFIXES bin lib
                        NO_DEFAULT_PATH)
                IF(NOT OPENJPEG_DLL)
                        MESSAGE(SEND_ERROR "Could not find OPENJPEG dll. You may specify it manually.")
                ENDIF()
                install(FILES "${OPENJPEG_DLL}"  DESTINATION .)
        ENDIF()

        # Install exiv
        IF(ENABLE_EXIV2)
                FIND_FILE(EXIV2_DLL
                        NAMES exiv2.dll libexiv2.dll
                        PATHS ${CMAKE_PREFIX_PATH}
                        PATH_SUFFIXES bin lib
                        NO_DEFAULT_PATH)
                IF(NOT EXIV2_DLL)
                        MESSAGE(SEND_ERROR "Could not find EXIV2 dll. You may specify it manually.")
                ENDIF()
                install(FILES "${EXIV2_DLL}"  DESTINATION .)
        ENDIF()
ELSEIF(APPLE)
ELSE()
        INSTALL(DIRECTORY "${CMAKE_SOURCE_DIR}/src/stylesheets" DESTINATION "share/scantailor-deviant")

        FIND_PROGRAM(GZIP gzip REQUIRED)
        EXECUTE_PROCESS(
                COMMAND "${GZIP}" "-9n"
                WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
                INPUT_FILE "${CMAKE_SOURCE_DIR}/debian/changelog"
                OUTPUT_FILE "changelog.gz"
                RESULT_VARIABLE COMMAND_ERROR)
        IF(COMMAND_ERROR)
                MESSAGE(FATAL_ERROR "Could not gzip changelog")
        ENDIF()

        INSTALL(FILES "${CMAKE_BINARY_DIR}/changelog.gz"     DESTINATION "share/doc/scantailor-deviant")
        INSTALL(FILES "${CMAKE_SOURCE_DIR}/debian/copyright" DESTINATION "share/doc/scantailor-deviant")
ENDIF(WIN32)

SET(CPACK_PACKAGE_NAME "scantailor-deviant")
STRING(REGEX 
        MATCH "^([0-9]+)\\.([0-9]+)\\.([0-9]+)" 
        CPACK_PACKAGE_VERSION ${VERSION})
SET(CPACK_PACKAGE_VERSION_MAJOR ${CMAKE_MATCH_1})
SET(CPACK_PACKAGE_VERSION_MINOR ${CMAKE_MATCH_2})
SET(CPACK_PACKAGE_VERSION_PATCH ${CMAKE_MATCH_3})
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "trufanov-nok <trufanovan@gmail.com>")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
SET(CPACK_PACKAGE_EXECUTABLES "scantailor-deviant;${CMAKE_PROJECT_NAME} ${VERSION}")
SET(CPACK_CREATE_DESKTOP_LINKS "scantailor-deviant")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY 
        "Alternative version of ScanTailor: interactive post-processing tool for scanned pages"
)
SET(CPACK_STRIP_FILES YES)

IF(WIN32)
        SET(CPACK_SOURCE_GENERATOR ZIP)
        SET(CPACK_GENERATOR NSIS ZIP 7Z)
        SET(CPACK_PACKAGE_INSTALL_DIRECTORY "${CMAKE_PROJECT_NAME}")
        SET(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/src/app/resources/win32/icon.ico")
        SET(CPACK_NSIS_COMPRESSOR "/SOLID lzma")
        SET(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
        SET(CPACK_NSIS_MODIFY_PATH ON)
        SET(CPACK_NSIS_DISPLAY_NAME "${CMAKE_PROJECT_NAME} ${VERSION}")
        SET(CPACK_NSIS_PACKAGE_NAME "${CMAKE_PROJECT_NAME}")
        SET(CPACK_NSIS_INSTALLED_ICON_NAME "scantailor-deviant.exe")
        SET(CPACK_NSIS_EXECUTABLES_DIRECTORY .)
ELSEIF(APPLE)
ELSE()
        SET(CPACK_DEBIAN_PACKAGE_DESCRIPTION 
                "Scan Tailor is an interactive post-processing tool for scanned pages.\n"
                " It performs operations such as page splitting, deskewing, adding/removing\n"
                " borders, and others. You give it raw scans, and you get pages ready to be\n"
                " printed or assembled into a PDF or DJVU file. Scanning, optical character\n"
                " recognition, and assembling multi-page documents are out of scope of this\n"
                " project.")
        STRING(REPLACE ";" "" CPACK_DEBIAN_PACKAGE_DESCRIPTION ${CPACK_DEBIAN_PACKAGE_DESCRIPTION})
        SET(CPACK_DEBIAN_PACKAGE_SECTION graphics)
        SET(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
        SET(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/ImageProcessing-ElectronicPublications/scantailor-deviant/")
ENDIF()

SET(CPACK_SOURCE_IGNORE_FILES
        "/\\\\.svn/"
        "/\\\\.git/"
        "~$"
        "\\\\.pcs$"
        "TODO.txt"
        "CMakeLists.txt.user"
        "/doxygen/"
        "${CMAKE_BINARY_DIR}"
)

IF(NOT WIN32 AND NOT APPLE)
        SET(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
ENDIF()

INCLUDE(CPack)
