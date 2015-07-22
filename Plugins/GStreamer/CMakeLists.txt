PROJECT(Gstreamer)
include(cmake)

FIND_PACKAGE(GStreamer_0_10)



FILE(GLOB_RECURSE src "src/*.cpp")
FILE(GLOB_RECURSE hdr "src/*.hpp")

INCLUDE_DIRECTORIES(${INCLUDE_DIRS})
LINK_DIRECTORIES(${LINK_DIRS})


ADD_LIBRARY(Gstreamer SHARED ${src} ${hdr})
TARGET_LINK_LIBRARIES(Gstreamer EagleLib ${LINK_LIBS})
ADD_DEPENDENCIES(Gstreamer EagleLib)

