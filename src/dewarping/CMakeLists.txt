PROJECT("Dewarping library")

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/core)

SET(
        sources
)
SOURCE_GROUP("Sources" FILES ${sources})

ADD_LIBRARY(dewarping STATIC ${sources})
QT5_USE_MODULES(dewarping Widgets Xml)

