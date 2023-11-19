PROJECT("Deskew Filter")

INCLUDE_DIRECTORIES(BEFORE "${CMAKE_CURRENT_BINARY_DIR}")

SET(
        sources
        CacheDrivenTask.cpp CacheDrivenTask.h
        Dependencies.cpp Dependencies.h
        DewarpingMode.cpp DewarpingMode.h
        DewarpingParams.cpp DewarpingParams.h
        DewarpingThumbnail.cpp DewarpingThumbnail.h
        DewarpingView.cpp DewarpingView.h
        DistortionType.cpp DistortionType.h
        Filter.cpp Filter.h
        ImageView.cpp ImageView.h
        NoDistortionView.cpp NoDistortionView.h
        OptionsWidget.cpp OptionsWidget.h
        Params.cpp Params.h
        PerspectiveParams.cpp PerspectiveParams.h
        RotationParams.cpp RotationParams.h
        RotationThumbnail.cpp RotationThumbnail.h
        Settings.cpp Settings.h
        STEX_PolylineModelShape.cpp STEX_PolylineModelShape.h
        Task.cpp Task.h
        Utils.cpp Utils.h
)
SOURCE_GROUP("Sources" FILES ${sources})
FILE(GLOB ui_files "ui/*.ui")
SOURCE_GROUP("UI Files" FILES ${ui_files})

QT5_WRAP_UI(ui_sources ${ui_files})

ADD_LIBRARY(deskew STATIC ${sources} ${ui_sources})
QT5_USE_MODULES(deskew Widgets Xml)

TRANSLATION_SOURCES(scantailor-deviant ${sources} ${ui_files})
