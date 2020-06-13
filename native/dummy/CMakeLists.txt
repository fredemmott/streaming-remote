set(
  SOURCES
  Dummy.cpp
  main.cpp
)

add_executable(
  dummy
  Dummy.cpp
  main.cpp
)

target_link_libraries(
  dummy
  PRIVATE
  streaming-remote-plugin-core
)

set_target_properties(
  dummy
  PROPERTIES
  CXX_STANDARD 20
)
