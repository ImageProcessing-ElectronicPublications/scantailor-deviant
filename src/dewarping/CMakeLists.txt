PROJECT("Dewarping library")

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/core)

SET(
        sources
        Curve.cpp Curve.h
        DepthPerception.cpp DepthPerception.h
        DistortionModel.cpp DistortionModel.h
)
SOURCE_GROUP("Sources" FILES ${sources})

ADD_LIBRARY(dewarping STATIC ${sources})
QT5_USE_MODULES(dewarping Widgets Xml)

