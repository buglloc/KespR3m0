# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(KespR3m0)

# Display Compile Time Information
message(STATUS "--------------Compile Info------------")
message(STATUS "IDF_PATH = ${IDF_PATH}")
message(STATUS "IDF_TARGET = ${IDF_TARGET}")
message(STATUS "PROJECT_NAME = ${PROJECT_NAME}")
message(STATUS "PROJECT_DIR = ${PROJECT_DIR}")
message(STATUS "BUILD_DIR = ${BUILD_DIR}")
message(STATUS "SDKCONFIG = ${SDKCONFIG}")
message(STATUS "SDKCONFIG_DEFAULTS = ${SDKCONFIG_DEFAULTS}")
message(STATUS "CONFIG_LV_CONF_SKIP = ${CONFIG_LV_CONF_SKIP}")
message(STATUS "COMPILE_OPTIONS = ${COMPILE_OPTIONS}")
message(STATUS "---------------------------------------")
message(STATUS "CMAKE_SOURCE_DIR = ${CMAKE_SOURCE_DIR}")
message(STATUS "CMAKE_BINARY_DIR = ${CMAKE_BINARY_DIR}")
message(STATUS "---------------------------------------")

# Embed front
set(WEB_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/front")
set(WEB_DIST_DIR "${WEB_SRC_DIR}/dist")
if(CONFIG_KESPR_HTTP_FORCE_GZIP)
  set(WEB_DIST_DIR "${WEB_SRC_DIR}/dist.gz")
endif()

if(EXISTS ${WEB_DIST_DIR})
  spiffs_create_partition_image(www ${WEB_DIST_DIR} FLASH_IN_PROJECT)
else()
  message(FATAL_ERROR "${WEB_DIST_DIR} doesn't exit. Please run 'npm run build' in ${WEB_DIST_DIR}")
endif()
