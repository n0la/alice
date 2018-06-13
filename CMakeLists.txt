CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

FIND_PACKAGE(PkgConfig REQUIRED)
FIND_PACKAGE(Threads REQUIRED)

PKG_CHECK_MODULES(DICE libdice REQUIRED)
PKG_CHECK_MODULES(IRC libirc REQUIRED)
PKG_CHECK_MODULES(EVENT libevent REQUIRED)
PKG_CHECK_MODULES(YAML yaml-0.1 REQUIRED)

SET(TARGET "alice")
SET(SOURCES
  "src/alice.c"
  "src/cmd.c"
  "src/plugin.c"
  "src/yamlconfig.c"
  )

SET(HEADERS
  "include/alice.h"
  "include/cmd.h"
  "include/log.h"
  "include/plugin.h"
  "include/yamlconfig.h"
  )

SET(PLUGINS
  "src/plugins/dice.c"
  "src/plugins/nickserv.c"
  )

INCLUDE_DIRECTORIES(
  ${DICE_INCLUDE_DIRS}
  ${IRC_INCLUDE_DIRS}
  ${EVENT_INCLUDE_DIRS}
  ${YAML_INCLUDE_DIRS}
  "${CMAKE_SOURCE_DIR}/include"
  )

ADD_EXECUTABLE(${TARGET} ${SOURCES} ${PLUGINS} ${HEADERS})
TARGET_LINK_LIBRARIES(${TARGET}
  ${DICE_LIBRARIES}
  ${IRC_LIBRARIES}
  ${EVENT_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
  ${YAML_LIBRARIES}
  )

INSTALL(TARGETS ${TARGET} DESTINATION bin)
