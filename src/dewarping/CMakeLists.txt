PROJECT("Dewarping library")

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/core)

SET(
        sources
        BendParams.cpp BendParams.h
        Curve.cpp Curve.h
        CylindricalSurfaceDewarper.cpp CylindricalSurfaceDewarper.h
        DetectVerticalBounds.cpp DetectVerticalBounds.h
        DewarpingImageTransform.cpp DewarpingImageTransform.h
        Directrix.cpp Directrix.h
        DistortionModel.cpp DistortionModel.h
        DistortionModelBuilder.cpp DistortionModelBuilder.h
        FrameParams.cpp FrameParams.h
        FovParams.cpp FovParams.h
        ImageSize.h
        RasterDewarper.cpp RasterDewarper.h
        HomographicTransform.cpp HomographicTransform.h
        PerspectiveTransform.cpp PerspectiveTransform.h
        SizeMode.cpp SizeMode.h
        SizeParams.cpp SizeParams.h
        STEX_ToPoint.h
        STEX_ToVec.h
        STEX_VecNT.h
        TextLineRefiner.cpp TextLineRefiner.h
        TextLineSegmenter.cpp TextLineSegmenter.h
        TextLineTracer.cpp TextLineTracer.h
        ToDouble.h
        TopBottomEdgeTracer.cpp TopBottomEdgeTracer.h
)
SOURCE_GROUP("Sources" FILES ${sources})

ADD_LIBRARY(dewarping STATIC ${sources})
TARGET_LINK_LIBRARIES(dewarping Qt5::Widgets Qt5::Xml)
