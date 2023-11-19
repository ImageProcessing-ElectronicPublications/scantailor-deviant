INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/src/core" .)
# for version.h
INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}")

ADD_SUBDIRECTORY(ui)
INCLUDE_DIRECTORIES("${toplevel_ui_sources_BINARY_DIR}")

SET(resource_files resources/resources.qrc resources/icons/COPYING)
IF(WIN32)
        LIST(APPEND resource_files resources/win32/resources.rc)
ENDIF()
SOURCE_GROUP("Resources" FILES ${resource_files})

QT5_ADD_RESOURCES(resource_sources resources/resources.qrc)
SOURCE_GROUP("Generated" FILES ${resource_sources})

IF(CANBERRA_FOUND)
SET( canberra_sources CanberraSoundPlayer.cpp CanberraSoundPlayer.h)
ENDIF()

SET(
        gui_only_sources
        
        Application.cpp Application.h
        SkinnedButton.cpp SkinnedButton.h
        StageListView.cpp StageListView.h
        RelinkablePathVisualization.cpp RelinkablePathVisualization.h
        RelinkingModel.cpp RelinkingModel.h
        RelinkingSortingModel.cpp RelinkingSortingModel.h
        RelinkingListView.cpp RelinkingListView.h
        RelinkingDialog.cpp RelinkingDialog.h
        SettingsDialog.cpp SettingsDialog.h
        FixDpiDialog.cpp FixDpiDialog.h
        LoadFilesStatusDialog.cpp LoadFilesStatusDialog.h
        ProjectCreationContext.cpp ProjectCreationContext.h
        ProjectOpeningContext.cpp ProjectOpeningContext.h
        OutOfMemoryDialog.cpp OutOfMemoryDialog.h
        ThumbnailSequence.cpp ThumbnailSequence.h
        ProjectFilesDialog.cpp ProjectFilesDialog.h
        NewOpenProjectPanel.cpp NewOpenProjectPanel.h
        SystemLoadWidget.cpp SystemLoadWidget.h
        MainWindow.cpp MainWindow.h
        main.cpp
        ExportDialog.cpp ExportDialog.h
        StartBatchProcessingDialog.cpp StartBatchProcessingDialog.h
        AutoSaveTimer.cpp AutoSaveTimer.h
        OpenWithMenuProvider.cpp OpenWithMenuProvider.h
        
        settings/fancylineedit.cpp settings/fancylineedit.h
        settings/execmenu.cpp settings/execmenu.h
        ${canberra_sources}
)




IF(CMAKE_COMPILER_IS_GNUCXX)
        # There seems to be a bug in either gcc or boost that makes page sorting either crash
        # or go into infinite loop.  It's reproducible on a very simple test case on at least
        # gcc 4.4.3 and gcc 4.4.4.  It's not reproducible on gcc 4.5.1.
        # Adding -fno-strict-aliasing seems to fix it.
        SET_SOURCE_FILES_PROPERTIES(
                ThumbnailSequence.cpp PROPERTIES
                COMPILE_FLAGS "-fno-strict-aliasing"
        )
ENDIF()

SET(win32_resource_file "")
IF(WIN32)
        SET(rc_file "${CMAKE_SOURCE_DIR}/src/app/resources/win32/resources.rc")
        FILE(GLOB win32_resources resources/win32/*.ico)
        SET_SOURCE_FILES_PROPERTIES(
                "${rc_file}" PROPERTIES
                OBJECT_DEPENDS ${win32_resources}
        )
        SET(win32_resource_file "${rc_file}")
ENDIF(WIN32)

IF (WIN32)
        ADD_EXECUTABLE(
                scantailor-deviant WIN32 ${gui_only_sources} ${resource_sources} ${win32_resource_file}
        )
ELSEIF(APPLE)

        set( ICON_NAME "ScanTailorDeviant.icns" )
        set( ICON_PATH "${PROJECT_SOURCE_DIR}/src/packaging/osx/${ICON_NAME}" )
        set_source_files_properties( ${ICON_PATH} PROPERTIES MACOSX_PACKAGE_LOCATION Resources )

        ADD_EXECUTABLE(
                scantailor-deviant MACOSX_BUNDLE ${gui_only_sources} ${resource_sources} ${ICON_PATH}
        )
        set_target_properties(scantailor-deviant PROPERTIES MACOSX_BUNDLE_INFO_PLIST
                                  ${PROJECT_SOURCE_DIR}/src/packaging/osx/Info.plist.in)


ELSE()
        ADD_EXECUTABLE(
                scantailor-deviant ${gui_only_sources} ${resource_sources} ${win32_resource_file}
        )
ENDIF()

TARGET_LINK_LIBRARIES(
        scantailor-deviant
        fix_orientation page_split deskew select_content page_layout output stcore
        dewarping zones interaction imageproc math foundation exporting
)

ADD_DEPENDENCIES(scantailor-deviant fix_orientation page_split deskew select_content page_layout output stcore
        dewarping zones interaction imageproc math foundation)


QT5_USE_MODULES(scantailor-deviant Widgets Xml Svg ${MAYBE_QT_OPENGL_MODULE})

IF(EXTRA_LIBS)
        TARGET_LINK_LIBRARIES(scantailor-deviant ${EXTRA_LIBS})
ENDIF()

IF(APPLE)
        INSTALL(TARGETS scantailor-deviant BUNDLE DESTINATION . RUNTIME DESTINATION bin)
ELSEIF(WIN32)
        INSTALL(TARGETS scantailor-deviant RUNTIME DESTINATION .)
ELSE(APPLE)
        INSTALL(TARGETS scantailor-deviant RUNTIME DESTINATION bin)
ENDIF(APPLE)

# Translations
TRANSLATION_SOURCES(
        scantailor-deviant
        ${gui_only_sources}
)
FINALIZE_TRANSLATION_SET(scantailor-deviant ${TRANSLATION_FILES})
UPDATE_TRANSLATIONS_TARGET(update_translations scantailor-deviant)
