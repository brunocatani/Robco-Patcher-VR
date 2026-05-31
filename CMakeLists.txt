cmake_minimum_required(VERSION 3.23)

# ---- Project Info ----
set(NAME "RobCo_Patcher")
set(VERSION 1.0.0)

message(">>> Start build for '${NAME}' v${VERSION}")

# ---- Folders ----
set(ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(SOURCE_DIR "${ROOT_DIR}/src")
set(BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}")

# ---- Include guards ----
if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(FATAL_ERROR ">> In-source builds not allowed.")
endif()

# ---- Setup vcpkg BEFORE project() ----
macro(set_from_environment VARIABLE)
  if(NOT DEFINED ${VARIABLE} AND DEFINED ENV{${VARIABLE}})
    set(${VARIABLE} $ENV{${VARIABLE}})
  endif()
endmacro()

set_from_environment(VCPKG_ROOT)
if(DEFINED VCPKG_ROOT)
  set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
  set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "")
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "")
else()
  message(FATAL_ERROR ">> Variable VCPKG_ROOT is not set!")
endif()

# ---- Locate CommonLibF4VR ----
set_from_environment(COMMON_LIB_F4VR_PATH)
if(NOT DEFINED COMMON_LIB_F4VR_PATH)
  set(COMMON_LIB_F4VR_PATH "${ROOT_DIR}/../CommonLibF4VR/CommonLibF4")
endif()
if(NOT EXISTS "${COMMON_LIB_F4VR_PATH}/CMakeLists.txt")
  message(FATAL_ERROR ">> CommonLibF4VR not found at ${COMMON_LIB_F4VR_PATH}. Set COMMON_LIB_F4VR_PATH to the CommonLibF4VR/CommonLibF4 directory.")
endif()
message(">>> Using CommonLibF4VR at: ${COMMON_LIB_F4VR_PATH}")

# ---- Create Project ----
project(${NAME} VERSION ${VERSION} LANGUAGES CXX)
message(">>> Building project '${PROJECT_NAME}' v${PROJECT_VERSION}")
include(CTest)

# ---- Build flags ----
add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
# LTO disabled — original code has void* functions without explicit return
# on all paths, which MSVC LTCG treats as fatal during code generation.
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION OFF)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)

# ---- Bring in CommonLibF4VR (VR-only build) ----
set(ENABLE_FALLOUT_F4 OFF CACHE BOOL "" FORCE)
set(ENABLE_FALLOUT_NG OFF CACHE BOOL "" FORCE)
set(ENABLE_FALLOUT_VR ON CACHE BOOL "" FORCE)
set(F4SE_SUPPORT_XBYAK ON CACHE BOOL "" FORCE)
add_subdirectory(${COMMON_LIB_F4VR_PATH} "CommonLibF4" EXCLUDE_FROM_ALL)

# ---- rapidcsv (needed by CommonLibF4VR for address library) ----
find_path(RAPIDCSV_INCLUDE_DIRS "rapidcsv.h")

# ---- Sources ----
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "${SOURCE_DIR}/*.cpp"
    "${SOURCE_DIR}/*.h"
)

# ---- Create DLL ----
add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)

# Suppress C4715 (not all control paths return a value) — original code
# uses void* return type but doesn't always return explicitly.
target_compile_options(${PROJECT_NAME} PRIVATE /wd4715)

target_precompile_headers(${PROJECT_NAME} PRIVATE "${SOURCE_DIR}/PCH.h")

target_include_directories(${PROJECT_NAME} PRIVATE
    "${SOURCE_DIR}"
    ${RAPIDCSV_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    CommonLibF4
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    UNICODE
    _UNICODE
    NOMINMAX
    WIN32_LEAN_AND_MEAN
    ENABLE_FALLOUT_VR
)

# ---- Post build ----
add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${ROOT_DIR}/package/Data/F4SE/Plugins"
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> "${ROOT_DIR}/package/Data/F4SE/Plugins/"
    COMMENT "Packaging ${PROJECT_NAME}..."
)

if(BUILD_TESTING)
    add_executable(PatchDiagnosticsTests
        tests/PatchDiagnosticsTests.cpp
        "${SOURCE_DIR}/PatchDiagnostics.cpp"
    )
    target_compile_features(PatchDiagnosticsTests PRIVATE cxx_std_23)
    target_include_directories(PatchDiagnosticsTests PRIVATE "${SOURCE_DIR}")
    add_test(NAME PatchDiagnosticsTests COMMAND PatchDiagnosticsTests)
endif()

message(">>> ${PROJECT_NAME} configured successfully")
