
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/src/core")

SET(
        cli_only_sources
        ConsoleBatch.cpp ConsoleBatch.h
        main-cli.cpp
)

ADD_EXECUTABLE(scantailor-deviant-cli ${cli_only_sources} )

TARGET_LINK_LIBRARIES(
        scantailor-deviant-cli
        fix_orientation page_split deskew select_content page_layout output stcore
	dewarping zones interaction imageproc math foundation exporting
)

IF(OPENMP_FOUND)
        TARGET_LINK_LIBRARIES(scantailor-deviant-cli OpenMP::OpenMP_CXX)
ENDIF()

# Widgets module is used statically but not at runtime.
TARGET_LINK_LIBRARIES(scantailor-deviant-cli Qt5::Widgets Qt5::Xml)

IF(EXTRA_LIBS)
        TARGET_LINK_LIBRARIES(scantailor-deviant-cli ${EXTRA_LIBS})
ENDIF()

IF(APPLE)
        INSTALL(TARGETS scantailor-deviant-cli BUNDLE DESTINATION . RUNTIME DESTINATION bin)
ELSEIF(WIN32)
        INSTALL(TARGETS scantailor-deviant-cli RUNTIME DESTINATION .)
ELSE()
        INSTALL(TARGETS scantailor-deviant-cli RUNTIME DESTINATION bin)
ENDIF()

TRANSLATION_SOURCES(
        scantailor-deviant
        ${cli_only_sources}
)
