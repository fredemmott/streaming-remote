include(ExternalProject)

ExternalProject_Add(
  libsodium_msvc
  URL https://github.com/jedisct1/libsodium/releases/download/1.0.18-RELEASE/libsodium-1.0.18-msvc.zip
  URL_HASH SHA512=08b598d5dad46b23acf56f401e566835d6cfd7f0eb16e7c55670fa671530a0cea99fd413b3384974b6292709250f040f17f56880f8cf10cdf55d6eb5659b5a5e
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
)
add_dependencies(libsodium libsodium_msvc)
ExternalProject_Get_Property(libsodium_msvc SOURCE_DIR)
target_include_directories(libsodium INTERFACE "${SOURCE_DIR}/include")

if(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
  set(ARCH_SUBDIR x64)
else()
  set(ARCH_SUBDIR Win32)
endif()

target_link_libraries(
  libsodium
  INTERFACE
  optimized
  "${SOURCE_DIR}/${ARCH_SUBDIR}/Release/v142/static/${CMAKE_STATIC_LIBRARY_PREFIX}libsodium${CMAKE_STATIC_LIBRARY_SUFFIX}"
  debug
  "${SOURCE_DIR}/${ARCH_SUBDIR}/Debug/v142/static/${CMAKE_STATIC_LIBRARY_PREFIX}libsodium${CMAKE_STATIC_LIBRARY_SUFFIX}"
)
