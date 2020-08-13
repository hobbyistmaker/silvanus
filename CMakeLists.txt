cmake_policy(SET CMP0048 NEW)
cmake_minimum_required(VERSION 3.17)
project (SilvanusPro VERSION 0.5.5.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum Fusion 360 supported version of MacOS/OSX.")
set(CMAKE_OSX_SYSROOT "/Library/Developer/CommandLineTools/SDKs/MacOSX10.13.sdk")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

configure_file(src/SilvanusPro.h.in src/SilvanusPro.h)

IF(APPLE)
    set(AUTODESK_API_FOLDER "$ENV{HOME}/Library/Application\ Support/Autodesk/Autodesk\ Fusion\ 360/API")
    set(LOCAL_INCLUDES "/usr/local/include")
ELSEIF(WIN32)
    set(AUTODESK_HASH d2d901e2ef5ae5c384b4ac001a48c4e37589dc43) # Pulled from the Autodesk folder name under your APPDATA folder
    set(AUTODESK_API_FOLDER "$ENV{APPDATA}/Autodesk/Autodesk Fusion 360/API")
ENDIF(APPLE)

set(AUTODESK_ADDINS_FOLDER "${AUTODESK_API_FOLDER}/Addins")
set(AUTODESK_FOLDER "${AUTODESK_API_FOLDER}/CPP")
set(F360_INCLUDES "${AUTODESK_FOLDER}/include")
set(F360_LIBS "${AUTODESK_FOLDER}/lib")

if (MSVC_VERSION GREATER_EQUAL "1900") # Make sure that the Visual Studio compiler is using C++17
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("/std:c++17" _cpp_latest_flag_supported)
    if (_cpp_latest_flag_supported)
        add_compile_options("/std:c++17")
    endif()
endif()

file(GLOB_RECURSE SOURCE_FILES
        LIST_DIRECTORIES true
        src/lib/*.cpp
        src/lib/*.hpp
        src/lib/*.h
        )

message( STATUS "Users home folder: $ENV{HOME}" )
message( STATUS "Autodesk Fusion 360 Includes folder: ${F360_INCLUDES}" )
message( STATUS "Target file destination: ${CMAKE_CURRENT_SOURCE_DIR}" )
message( STATUS "Binary dir: ${CMAKE_BINARY_DIR}" )
message( STATUS "Source dir: ${CMAKE_SOURCE_DIR}" )
message( STATUS "Found sources: ${SOURCE_FILES}" )
IF(WIN32)
    message( STATUS "App data: $ENV{LOCALAPPDATA}")
ENDIF()

link_directories("${F360_LIBS}")

add_library(SilvanusPro SHARED ${SOURCE_FILES})
set_target_properties(SilvanusPro PROPERTIES PREFIX "")
set_target_properties(
        SilvanusPro PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}"
)
target_sources(SilvanusPro
        PRIVATE ${SOURCE_FILES}
        PUBLIC src/SilvanusPro.cpp
        )

include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/src/lib)
include_directories(${CMAKE_SOURCE_DIR}/src/lib/common)
include_directories(${CMAKE_SOURCE_DIR}/src/lib/generatebox)
include_directories(${CMAKE_SOURCE_DIR}/src/lib/generatebox/entities)
include_directories(${CMAKE_SOURCE_DIR}/src/lib/generatebox/fusion)
include_directories(${CMAKE_SOURCE_DIR}/src/lib/generatebox/systems)
include_directories(${CMAKE_SOURCE_DIR}/src/lib/generatebox/render)
include_directories(${F360_INCLUDES})

target_include_directories(SilvanusPro PRIVATE "${F360_INCLUDES}")
target_include_directories(SilvanusPro PRIVATE "${F360_INCLUDES}/Core")
target_include_directories(SilvanusPro PRIVATE "${F360_INCLUDES}/Core/UserInterface")

IF(APPLE)
    target_include_directories(SilvanusPro PRIVATE "${LOCAL_INCLUDES}")
    target_include_directories(SilvanusPro PRIVATE "${LOCAL_INCLUDES}/entt")
    find_library(ADSK_CORE core.dylib PATHS "${F360_LIBS}/")
    find_library(ADSK_FUSION fusion.dylib PATHS "${F360_LIBS}/")
    find_library(ADSK_CAM cam.dylib PATHS "${F360_LIBS}/")
ELSEIF(WIN32)
    find_package(EnTT CONFIG REQUIRED) # Assumes EnTT installed through vcpkg with Visual Studio
    target_link_libraries(SilvanusPro PRIVATE EnTT::EnTT)
    find_library(ADSK_CORE core.lib PATHS "${F360_LIBS}/")
    find_library(ADSK_FUSION fusion.lib PATHS "${F360_LIBS}/")
    find_library(ADSK_CAM cam.lib PATHS "${F360_LIBS}/")
ENDIF()

target_link_libraries(SilvanusPro PUBLIC ${ADSK_CORE} ${ADSK_FUSION} ${ADSK_CAM})

if(CMAKE_GENERATOR STREQUAL Xcode)
    configure_file(
            WorkspaceSettings.xcsettings
            ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.xcodeproj/project.xcworkspace/xcuserdata/$ENV{USER}.xcuserdatad/WorkspaceSettings.xcsettings
            COPYONLY
    )
endif()

IF(APPLE)
    file(REMOVE ${AUTODESK_ADDINS_FOLDER}/SilvanusPro/resources)
    file(REMOVE ${AUTODESK_ADDINS_FOLDER}/SilvanusPro/SilvanusPro.manifest)
    file(REMOVE ${AUTODESK_ADDINS_FOLDER}/SilvanusPro/SilvanusPro.dylib)
    file(COPY ${CMAKE_SOURCE_DIR}/resources DESTINATION ${AUTODESK_ADDINS_FOLDER}/SilvanusPro)
    file(COPY ${CMAKE_SOURCE_DIR}/SilvanusPro.manifest DESTINATION ${AUTODESK_ADDINS_FOLDER}/SilvanusPro)
    file(COPY ${CMAKE_BINARY_DIR}/SilvanusPro.dylib DESTINATION ${AUTODESK_ADDINS_FOLDER}/SilvanusPro)
ENDIF()
