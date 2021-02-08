configure_file(version.h.in version.h)

set(
  SOURCES
  XSplit.cpp
  plugin-main.cpp
)

if(WIN32)
  list(APPEND SOURCES portability-windows.cpp)
else()
  list(APPEND SOURCES portability-stdcpp.cpp)
endif()

add_library(
  xsplit-streaming-remote
  SHARED
  ${SOURCES}
)

set_target_properties(
	streaming-remote-plugin-core
	PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS off
)

install(TARGETS xsplit-streaming-remote)
if(WIN32)
  install(FILES "$<TARGET_PDB_FILE:xsplit-streaming-remote>" TYPE BIN OPTIONAL)
endif()

target_link_libraries(
  xsplit-streaming-remote
  PRIVATE
  streaming-remote-plugin-core
)
target_include_directories(
  xsplit-streaming-remote
  PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}
)

if (WIN32)
  target_link_libraries(
    xsplit-streaming-remote
    PRIVATE
    optimized
    comsuppw
    debug
    comsuppwd
  )
endif()

sign_target(xsplit-streaming-remote)
install(TARGETS xsplit-streaming-remote)
