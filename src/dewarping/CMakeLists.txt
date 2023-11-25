PROJECT("Dewarping library")

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/core)

SET(
        sources
        Curve.cpp Curve.h
        CylindricalSurfaceDewarper.cpp CylindricalSurfaceDewarper.h
        DepthPerception.cpp DepthPerception.h
        DetectVerticalBounds.cpp DetectVerticalBounds.h
        DewarpingImageTransform.cpp DewarpingImageTransform.h
        DistortionModel.cpp DistortionModel.h
        DistortionModelBuilder.cpp DistortionModelBuilder.h
        RasterDewarper.cpp RasterDewarper.h
        STEX_HomographicTransform.h
        STEX_ToPoint.h
        STEX_ToVec.h
        STEX_VecNT.h
        TextLineRefiner.cpp TextLineRefiner.h
        TextLineSegmenter.cpp TextLineSegmenter.h
        TextLineTracer.cpp TextLineTracer.h
        TopBottomEdgeTracer.cpp TopBottomEdgeTracer.h
)
SOURCE_GROUP("Sources" FILES ${sources})

ADD_LIBRARY(dewarping STATIC ${sources})
QT5_USE_MODULES(dewarping Widgets Xml)

