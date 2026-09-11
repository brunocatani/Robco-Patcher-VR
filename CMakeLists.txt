cmake_minimum_required(VERSION 3.23)

# ---- Project Info ----
set(NAME "RobCo_Patcher")
set(VERSION 1.2.0)

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
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/VerifyFo4VrLoader.cmake")
robco_verify_fo4vr_loader("${SOURCE_DIR}/main.cpp")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${SOURCE_DIR}/main.cpp")

# ---- Build flags ----
add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)

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

target_precompile_headers(${PROJECT_NAME} PRIVATE "${SOURCE_DIR}/PCH.h")

target_include_directories(${PROJECT_NAME} PRIVATE
    "${SOURCE_DIR}"
    ${RAPIDCSV_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    CommonLibF4
    Shell32
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    UNICODE
    _UNICODE
    NOMINMAX
    WIN32_LEAN_AND_MEAN
    ENABLE_FALLOUT_VR
)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:/Zi>)
    target_link_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:/DEBUG:FULL /OPT:REF /OPT:ICF>)
endif()

# ---- Build-enforced loader compatibility gate ----
add_custom_target(verify_fo4vr_loader
    COMMAND ${CMAKE_COMMAND}
        -DLOADER_SOURCE=${SOURCE_DIR}/main.cpp
        -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/VerifyFo4VrLoader.cmake
    VERBATIM
)
add_dependencies(${PROJECT_NAME} verify_fo4vr_loader)

# ---- Post build ----
add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${ROOT_DIR}/package/Data/F4SE/Plugins"
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> "${ROOT_DIR}/package/Data/F4SE/Plugins/"
    COMMENT "Packaging ${PROJECT_NAME}..."
)

option(POST_BUILD_COPY_PLUGIN "Deploy the plugin to an MO2 mod directory" OFF)
set(COPY_PLUGIN_BASE_PATH "" CACHE PATH "MO2 mod directory used by custom-fast")
if(POST_BUILD_COPY_PLUGIN)
    if(COPY_PLUGIN_BASE_PATH STREQUAL "")
        message(FATAL_ERROR "POST_BUILD_COPY_PLUGIN requires COPY_PLUGIN_BASE_PATH")
    endif()
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${COPY_PLUGIN_BASE_PATH}/F4SE/Plugins"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:${PROJECT_NAME}>"
            "${COPY_PLUGIN_BASE_PATH}/F4SE/Plugins/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_PDB_FILE:${PROJECT_NAME}>"
            "${COPY_PLUGIN_BASE_PATH}/F4SE/Plugins/$<TARGET_PDB_FILE_NAME:${PROJECT_NAME}>"
        COMMENT "Deploying ${PROJECT_NAME} through custom-fast"
        VERBATIM
    )
endif()

if(BUILD_TESTING)
    add_executable(PatchDiagnosticsTests
        tests/PatchDiagnosticsTests.cpp
        "${SOURCE_DIR}/PatchDiagnostics.cpp"
    )
    target_compile_features(PatchDiagnosticsTests PRIVATE cxx_std_23)
    target_include_directories(PatchDiagnosticsTests PRIVATE "${SOURCE_DIR}")
    add_test(NAME PatchDiagnosticsTests COMMAND PatchDiagnosticsTests)
    add_test(
        NAME VerifyLoaderValidator
        COMMAND ${CMAKE_COMMAND}
            -DVALIDATOR=${CMAKE_CURRENT_SOURCE_DIR}/cmake/VerifyFo4VrLoader.cmake
            -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/VerifyLoaderValidator.cmake
    )
endif()

message(">>> ${PROJECT_NAME} configured successfully")
