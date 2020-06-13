include(ExternalProject)

find_library(LIBSODIUM_LIB NAMES sodium.a sodium.lib DOC "libsodium library (optional)")
find_path(LIBSODIUM_INCLUDEDIR NAMES sodium.h DOC "libsodium include path")

add_library(libsodium INTERFACE)
if(
  "${LIBSODIUM_LIB}" STREQUAL "LIBSODIUM_LIB-NOTFOUND"
  OR "${LIBSODIUM_INCLUDEDIR}" STREQUAL "LIBSODIUM_INCLUDEDIR-NOTFOUND"
)
  target_compile_definitions(libsodium INTERFACE -DSODIUM_STATIC=1)
  if (MSVC)
    message(STATUS "Using msvc libsodium")
    include(${CMAKE_CURRENT_LIST_DIR}/libsodium_msvc.cmake)
  else()
    message(STATUS "Using source libsodium")
    include(${CMAKE_CURRENT_LIST_DIR}/libsodium_source.cmake)
  endif()
else()
  message(STATUS "Using system libsodium")
  target_link_libraries(libsodium INTERFACE ${LIBSODIUM_LIB})
  target_include_directories(libsodium INTERFACE ${LIBSODIUM_INCLUDEDIR})
endif()
