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
        STEX_AbstractImageTransform.h
        STEX_AffineImageTransform.cpp STEX_AffineImageTransform.h
        STEX_AffineTransform.cpp STEX_AffineTransform.h
        STEX_AffineTransformedImage.cpp STEX_AffineTransformedImage.h
        STEX_BadAllocIfNull.cpp STEX_BadAllocIfNull.h
        STEX_Connectivity.h
        STEX_ConnectivityMap.cpp STEX_ConnectivityMap.h
        STEX_CopyableByMemcpy.h
        STEX_GaussBlur.cpp STEX_GaussBlur.h
        STEX_HomographicTransform.h
        STEX_LineBoundedByPolygon.cpp STEX_LineBoundedByPolygon.h
        STEX_MinMaxAccumulator.h
        STEX_MultipleTargetsSupport.h
        STEX_PlugHoles.h
        STEX_PolygonRasterizer.cpp STEX_PolygonRasterizer.h
        STEX_RasterOpGeneric.h
        STEX_RoundingHasher.cpp STEX_RoundingHasher.h
        STEX_ToPoint.h
        STEX_ToVec.h
        STEX_TraverseBorders.h
        STEX_VecNT.h
        STEX_WatershedSegmentation.cpp STEX_WatershedSegmentation.h
        TextLineRefiner.cpp TextLineRefiner.h
        TextLineSegmenter.cpp TextLineSegmenter.h
        TextLineTracer.cpp TextLineTracer.h
        TopBottomEdgeTracer.cpp TopBottomEdgeTracer.h
)
SOURCE_GROUP("Sources" FILES ${sources})

ADD_LIBRARY(dewarping STATIC ${sources})
QT5_USE_MODULES(dewarping Widgets Xml)

