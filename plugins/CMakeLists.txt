cmake_minimum_required(VERSION 3.10.2)

project(streaming-remote)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMake)

set(BASE_SOURCES
  base/ClientHandler.cpp
  base/Config.cpp
  base/MessageInterface.cpp
  base/Output.cpp
  base/SocketMessageInterface.cpp
  base/SocketServer.cpp
  base/StreamingSoftware.cpp
  base/WebSocketMessageInterface.cpp
)

find_package(sodium REQUIRED)
find_package(Qt5Core CONFIG REQUIRED)
find_package(Qt5Network CONFIG REQUIRED)
find_package(Qt5WebSockets CONFIG REQUIRED)
set(BASE_LIBS Qt5::Core Qt5::Network Qt5::WebSockets sodium)

set(WITH_OBS_PLUGIN OFF CACHE BOOL "Build the OBS plugin")
if(WITH_OBS_PLUGIN)
  set(OBS_SOURCES
    ${BASE_SOURCES}
    obs/plugin-main.cpp
    obs/OBS.cpp
    obs/OBSConfigDialog.cpp
  )

  set(OBS_SOURCE_DIR "" CACHE PATH "Path to OBS source directory")
  set(OBS_BUILD_DIR "${OBS_SOURCE_DIR}/build" CACHE PATH "Path to OBS build directory")

  find_path(
    LIBOBS_INCLUDE_DIR
    obs-module.h
    HINTS "${OBS_SOURCE_DIR}/libobs/"
  )

  find_path(
    OBS_FRONTEND_API_INCLUDE_DIR
    obs-frontend-api.h
    HINTS "${OBS_SOURCE_DIR}/UI/obs-frontend-api/"
  )

  add_library(obs-streaming-remote SHARED ${OBS_SOURCES})
  include_directories(
    obs-streaming-remote ${LIBOBS_INCLUDE_DIR} ${OBS_FRONTEND_API_INCLUDE_DIR}
  )

  find_library(
    LIBOBS_LIB
    NAMES
    obs 
    HINTS
    "${OBS_SOURCE_DIR}/libobs/"
    "${OBS_BUILD_DIR}/libobs/"
  )

  find_library(
    OBS_FRONTEND_API_LIB
    obs-frontend-api
    HINTS
    "${OBS_SOURCE_DIR}/UI/obs-frontend-api/"
    "${OBS_BUILD_DIR}/UI/obs-frontend-api/"
    "${OBS_DEPENDENCIES_DIR}/win32/bin/"
    "${OBS_DEPENDENCIES_DIR}/win64/bin/"
  )

  find_package(Qt5Widgets CONFIG REQUIRED)
  target_link_libraries(
    obs-streaming-remote
    Qt5::Widgets
    ${BASE_LIBS}
    ${LIBOBS_LIB}
    ${OBS_FRONTEND_API_LIB}
  )
  if (NOT WIN32)
    set_target_properties(obs-streaming-remote PROPERTIES PREFIX "" SUFFIX ".so")
  endif()
endif()

set(WITH_XSPLIT_PLUGIN OFF CACHE BOOL "Build the XSplit plugin")
if(WITH_XSPLIT_PLUGIN)

  set(XSPLIT_SOURCES
    ${BASE_SOURCES}
    xsplit/plugin-main.cpp
    xsplit/XSplit.cpp
    xsplit/portability.cpp
  )

  add_library(xsplit-streaming-remote SHARED ${XSPLIT_SOURCES})
  target_link_libraries(xsplit-streaming-remote ${BASE_LIBS})
endif()
