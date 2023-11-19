PROJECT("Dewarping library")

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/core)

SET(
        sources
        Curve.cpp Curve.h
        CylindricalSurfaceDewarper.cpp CylindricalSurfaceDewarper.h
        DepthPerception.cpp DepthPerception.h
        DetectVerticalBounds.cpp DetectVerticalBounds.h
        DistortionModel.cpp DistortionModel.h
        STEX_HomographicTransform.h
        STEX_MultipleTargetsSupport.h
        STEX_ToPoint.h
        STEX_ToVec.h
)
SOURCE_GROUP("Sources" FILES ${sources})

ADD_LIBRARY(dewarping STATIC ${sources})
QT5_USE_MODULES(dewarping Widgets Xml)

