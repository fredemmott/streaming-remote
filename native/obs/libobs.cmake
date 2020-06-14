set(OBS_SOURCE_DIR "" CACHE PATH "Path to obs-studio source")
set(OBS_BUILD_DIR "build" CACHE PATH "Path to obs-studio build tree, matching OBS_SOURCE_DIR")

find_library(
  OBS_LIB
  NAMES obs
  REQUIRED
  HINTS
  ${OBS_BUILD_DIR}/libobs/
  ${OBS_BUILD_DIR}/libobs/Debug/
  ${OBS_BUILD_DIR}/libobs/Release/
)
find_library(
  OBS_FRONTEND_API_LIB
  NAMES
  obs-frontend-api
  REQUIRED
  HINTS
  ${OBS_BUILD_DIR}/UI/obs-frontend-api/
  ${OBS_BUILD_DIR}/UI/obs-frontend-api/Debug/
  ${OBS_BUILD_DIR}/UI/obs-frontend-api/Release/
)

find_path(
  OBS_INCLUDE_DIR
  obs-module.h
  HINTS
  ${OBS_SOURCE_DIR}/libobs/
)
find_path(
  OBS_FRONTEND_API_INCLUDE_DIR
  obs-frontend-api.h
  HINTS
  ${OBS_SOURCE_DIR}/UI/obs-frontend-api/
)

add_library(libobs INTERFACE)
find_package(Qt5 COMPONENTS Core Widgets CONFIG REQUIRED)
option(
  FORCE_RELEASE_QT
  "Use the release build of Qt, even in debug builds; avoids runtime dependency on debug libraries"
  ON
)
if(FORCE_RELEASE_QT)
  set_target_properties(
    Qt5::Core
    Qt5::Widgets
    PROPERTIES
    MAP_IMPORTED_CONFIG_DEBUG Release
    MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
  )
endif()
target_link_libraries(
  libobs
  INTERFACE
  Qt5::Widgets
  ${OBS_LIB}
  ${OBS_FRONTEND_API_LIB}
)
target_include_directories(
  libobs
  INTERFACE
  ${OBS_INCLUDE_DIR}
  ${OBS_FRONTEND_API_INCLUDE_DIR}
)
