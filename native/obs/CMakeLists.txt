# OBS binaries are built with Qt 5.10.1, which does not support C++20
include(libobs.cmake)

add_library(
  obs-streaming-remote
  SHARED
  OBS.cpp
  OBS_GPLv2.cpp
  OBSConfigDialog.cpp
  plugin-main.cpp
)
sign_target(obs-streaming-remote)
target_link_libraries(
  obs-streaming-remote
  PRIVATE
  streaming-remote-plugin-core
  libobs
)
set_target_properties(
  obs-streaming-remote
  PROPERTIES
  AUTOMOC ON
)
install(TARGETS obs-streaming-remote)

if(APPLE)
  set_target_properties(obs-streaming-remote PROPERTIES PREFIX "" SUFFIX ".so")
  get_filename_component(
    Qt5_PREFIX
    ${Qt5_DIR}/../../../ # ends with lib/cmake/Qt5
    ABSOLUTE
    CACHE
  )
  add_custom_command(
    TARGET obs-streaming-remote POST_BUILD
    COMMAND
      install_name_tool
        -change
          ${Qt5_PREFIX}/lib/QtWidgets.framework/Versions/5/QtWidgets
          @executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets
        -change
          ${Qt5_PREFIX}/lib/QtGui.framework/Versions/5/QtGui
          @executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui
        -change
          ${Qt5_PREFIX}/lib/QtCore.framework/Versions/5/QtCore
          @executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore
        obs-streaming-remote.so
  )
endif()
