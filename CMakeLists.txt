PROJECT (yadc)
INCLUDE_DIRECTORIES(depends/jsonxx)
# A list of source files
SET(PROJECT_SRCS
    depends/jsonxx/jsonxx.cc
    yadc.cpp
    config.cpp
    renderer.cpp
    server.cpp
    util.cpp
)
# A list of headers
SET(PROJECT_HDRS
    depends/jsonxx/jsonxx.h
    config.h
    renderer.h
    server.h
    util.h
)
SET_SOURCE_FILES_PROPERTIES(${PROJECT_HDRS} PROPERTIES HEADER_FILE_ONLY TRUE)

# mash them together (headers are marked as headers and nothing will try to compile them)
LIST(APPEND PROJECT_SRCS ${PROJECT_HDRS})

# this makes sure all the stuff is put in proper places and linked to dfhack
DFHACK_PLUGIN(yadc ${PROJECT_SRCS} LINK_LIBRARIES clsocket)
