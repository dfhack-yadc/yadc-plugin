PROJECT (yadc)
SET(YADC_SRCS
    yadc.cpp
    client.cpp
    config.cpp
    input.cpp
    screen.cpp
    util.cpp
)
SET(YADC_HDRS
    client.h
    config.h
    input.h
    screen.h
    util.h
)
SET_SOURCE_FILES_PROPERTIES(${YADC_HDRS} PROPERTIES HEADER_FILE_ONLY TRUE)
LIST(APPEND YADC_SRCS ${YADC_HDRS})
OPTION(BUILD_YADC ON)
IF(BUILD_YADC)
    DFHACK_PLUGIN(yadc ${YADC_SRCS} LINK_LIBRARIES clsocket dfhack-md5 jsoncpp)
ENDIF()
