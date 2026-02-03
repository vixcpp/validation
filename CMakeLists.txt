#  @file CMakeLists.txt
#  @author Gaspard Kirira
#
#  Copyright 2025, Gaspard Kirira. All rights reserved.
#  https://github.com/vixcpp/vix
#  Use of this source code is governed by a MIT license
#  that can be found in the License file.
#
# ====================================================================
# Vix.cpp - Validation Module
# ====================================================================
# Purpose:
#   Composable validation framework for Vix.
#   Provides rules, schemas, and structured errors for validating
#   entities, models, and user inputs (HTTP/JSON/CLI).
#
#   Validation depends on vix::conversion for safe parsing of scalar
#   values (string_view -> int/bool/float/enum) before applying rules.
#
#   Builds as STATIC when sources exist, otherwise as a header-only
#   INTERFACE target.
#
# Public Targets:
#   - vix_validation  : The actual library target (STATIC or INTERFACE)
#   - vix::validation : Namespaced alias for consumers
#
# Public Dependencies:
#   - vix::conversion
#
# Installation/Export:
#   Installs into the umbrella export-set `VixTargets`.
# ====================================================================

cmake_minimum_required(VERSION 3.20)
project(vix_validation VERSION 0.1.0 LANGUAGES CXX)

include(GNUInstallDirs)

# Global settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# --------------------------------------------------------------------
# Umbrella build policy:
# When building inside the Vix umbrella, dependencies are managed by the
# umbrella (add_subdirectory order + options). Validation must not
# FetchContent nor add sibling subdirectories in umbrella mode.
# --------------------------------------------------------------------
if (DEFINED VIX_UMBRELLA_BUILD AND VIX_UMBRELLA_BUILD)
  set(VIX_VALIDATION_FETCH_CONVERSION OFF CACHE BOOL "Auto-fetch vix::conversion if missing" FORCE)
endif()

# Sources discovery
file(GLOB_RECURSE VALIDATION_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# Conversion dependency
option(VIX_VALIDATION_FETCH_CONVERSION "Auto-fetch vix::conversion if missing" ON)

if (DEFINED VIX_UMBRELLA_BUILD AND VIX_UMBRELLA_BUILD)
  if (NOT TARGET vix::conversion)
    message(FATAL_ERROR "[validation] Umbrella build: vix::conversion must be provided by umbrella before validation.")
  endif()
else()
  if (NOT TARGET vix::conversion)
    if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/../conversion/CMakeLists.txt")
      message(STATUS "[validation] Adding conversion from umbrella: ../conversion")
      add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/../conversion" "conversion")
    elseif (VIX_VALIDATION_FETCH_CONVERSION)
      include(FetchContent)
      message(STATUS "[validation] Fetching vix::conversion via FetchContent")
      FetchContent_Declare(vix_conversion
        GIT_REPOSITORY https://github.com/vixcpp/conversion.git
        GIT_TAG        v0.1.0
      )
      FetchContent_MakeAvailable(vix_conversion)
    else()
      message(FATAL_ERROR "vix::conversion not found. Enable VIX_VALIDATION_FETCH_CONVERSION=ON or provide the target before validation.")
    endif()
  endif()
endif()

set(VIX_CONVERSION_TARGET vix::conversion)

# STATIC
if (VALIDATION_SOURCES)
  message(STATUS "[validation] Building STATIC library with detected sources.")

  add_library(vix_validation STATIC ${VALIDATION_SOURCES})
  add_library(vix::validation ALIAS vix_validation)

  target_compile_features(vix_validation PUBLIC cxx_std_20)

  target_include_directories(vix_validation
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  )

  target_link_libraries(vix_validation
    PUBLIC
      ${VIX_CONVERSION_TARGET}
  )

  if (TARGET vix_warnings)
    target_link_libraries(vix_validation PUBLIC vix_warnings)
  endif()

  if (VIX_ENABLE_SANITIZERS AND TARGET vix_sanitizers)
    target_link_libraries(vix_validation PUBLIC vix_sanitizers)
  endif()

  set_target_properties(vix_validation PROPERTIES
    OUTPUT_NAME vix_validation
    VERSION ${PROJECT_VERSION}
    SOVERSION 0
    EXPORT_NAME validation
  )

  install(TARGETS vix_validation
    EXPORT VixTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )

  if (NOT (DEFINED VIX_UMBRELLA_BUILD AND VIX_UMBRELLA_BUILD))
    install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
            FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h")
  endif()

# HEADER-ONLY
else()
  message(STATUS "[validation] Building HEADER-ONLY library (no sources).")

  add_library(vix_validation INTERFACE)
  add_library(vix::validation ALIAS vix_validation)

  target_compile_features(vix_validation INTERFACE cxx_std_20)

  target_include_directories(vix_validation INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  )

  target_link_libraries(vix_validation INTERFACE
    ${VIX_CONVERSION_TARGET}
  )

  if (VIX_ENABLE_SANITIZERS AND TARGET vix_sanitizers)
    target_link_libraries(vix_validation INTERFACE vix_sanitizers)
  endif()

  install(TARGETS vix_validation
    EXPORT VixTargets
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )

  if (NOT (DEFINED VIX_UMBRELLA_BUILD AND VIX_UMBRELLA_BUILD))
    install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
            FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h")
  endif()

endif()

# Tests
option(VIX_VALIDATION_BUILD_TESTS "Build validation module tests" OFF)

if (VIX_VALIDATION_BUILD_TESTS)
  include(CTest)
  enable_testing()
  add_subdirectory(tests)
endif()

# Summary
message(STATUS "------------------------------------------------------")
message(STATUS "vix::validation configured (${PROJECT_VERSION})")
if (VALIDATION_SOURCES)
  message(STATUS "Mode: STATIC / sources found")
else()
  message(STATUS "Mode: HEADER-ONLY / no sources")
endif()
message(STATUS "Conversion target: ${VIX_CONVERSION_TARGET}")
message(STATUS "Include dir: ${CMAKE_CURRENT_SOURCE_DIR}/include (for <vix/validation/...>)")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "------------------------------------------------------")
