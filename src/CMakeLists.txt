cmake_policy(SET CMP0048 NEW)
cmake_minimum_required(VERSION 3.17)
project (SilvanusPro VERSION 0.6.0.0)

if (CMAKE_BINARY_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    message(FATAL_ERROR "Building in-source is not supported!")
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
#set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum Fusion 360 supported version of MacOS/OSX.")
set(CMAKE_OSX_SYSROOT "/Library/Developer/CommandLineTools/SDKs/MacOSX10.13.sdk")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

configure_file(${PROJECT_NAME}.h.in ${PROJECT_NAME}.h)

if(APPLE)
    set(AUTODESK_API_FOLDER "$ENV{HOME}/Library/Application\ Support/Autodesk/Autodesk\ Fusion\ 360/API")
    set(SILVANUS_LIB "${PROJECT_NAME}.dylib")
    set(LOCAL_INCLUDES "/usr/local/include")
    set(LOCAL_LIB "/usr/local/lib")
elseif(WIN32)
    set(AUTODESK_HASH d2d901e2ef5ae5c384b4ac001a48c4e37589dc43) # Pulled from the Autodesk folder name under your APPDATA folder
    set(AUTODESK_API_FOLDER "$ENV{APPDATA}/Autodesk/Autodesk Fusion 360/API")
    set(SILVANUS_LIB "${PROJECT_NAME}.dll")
endif(APPLE)

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
        CONFIGURE_DEPENDS
        lib/*.cpp
        lib/*.hpp
        lib/*.h
        )
message( STATUS "Project name: ${PROJECT_NAME}" )
message( STATUS "Users home folder: $ENV{HOME}" )
message( STATUS "Autodesk Fusion 360 Includes folder: ${F360_INCLUDES}" )
message( STATUS "Target file destination: ${CMAKE_CURRENT_SOURCE_DIR}" )
message( STATUS "Binary dir: ${CMAKE_BINARY_DIR}" )
message( STATUS "Source dir: ${CMAKE_SOURCE_DIR}" )
message( STATUS "Found sources: ${SOURCE_FILES}" )
if(WIN32)
    message( STATUS "App data: $ENV{LOCALAPPDATA}")
endif()

link_directories("${F360_LIBS}")

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
set_target_properties(
        ${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}"
)
target_sources(${PROJECT_NAME}
        PRIVATE ${SOURCE_FILES}
        PUBLIC ${PROJECT_NAME}.cpp
        )

install(TARGETS ${PROJECT_NAME} DESTINATION ${AUTODESK_ADDINS_FOLDER}/${PROJECT_NAME})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${SILVANUS_LIB} DESTINATION ${AUTODESK_ADDINS_FOLDER}/${PROJECT_NAME})
install(FILES ${CMAKE_SOURCE_DIR}/${PROJECT_NAME}.manifest DESTINATION ${AUTODESK_ADDINS_FOLDER}/${PROJECT_NAME})
install(DIRECTORY ${CMAKE_SOURCE_DIR}/resources DESTINATION ${AUTODESK_ADDINS_FOLDER}/${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src/lib)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src/lib/common)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src/lib/generatebox)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src/lib/generatebox/dialog)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src/lib/generatebox/dialog/entities)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src/lib/generatebox/dialog/presentation)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src/lib/generatebox/dialog/systems)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src/lib/generatebox/entities)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src/lib/generatebox/fusion)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src/lib/generatebox/systems)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src/lib/generatebox/render)
target_include_directories(${PROJECT_NAME} PRIVATE ${F360_INCLUDES})

target_include_directories(${PROJECT_NAME} PRIVATE "${F360_INCLUDES}/Core")
target_include_directories(${PROJECT_NAME} PRIVATE "${F360_INCLUDES}/Core/UserInterface")

if(APPLE)
    target_include_directories(${PROJECT_NAME} PRIVATE "${LOCAL_INCLUDES}")
    target_include_directories(${PROJECT_NAME} PRIVATE "${LOCAL_INCLUDES}/entt")
    find_library(ADSK_CORE core.dylib PATHS "${F360_LIBS}/")
    find_library(ADSK_FUSION fusion.dylib PATHS "${F360_LIBS}/")
    find_library(ADSK_CAM cam.dylib PATHS "${F360_LIBS}/")
    find_library(BOOST_FILE libboost_filesystem.a PATHS "${LOCAL_LIB}")
elseif(WIN32)
    find_package(EnTT CONFIG REQUIRED) # Assumes EnTT installed through vcpkg with Visual Studio
    target_link_libraries(${PROJECT_NAME} PRIVATE EnTT::EnTT)
    find_library(ADSK_CORE core.lib PATHS "${F360_LIBS}/")
    find_library(ADSK_FUSION fusion.lib PATHS "${F360_LIBS}/")
    find_library(ADSK_CAM cam.lib PATHS "${F360_LIBS}/")
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC ${ADSK_CORE} ${ADSK_FUSION} ${ADSK_CAM})
target_link_libraries(${PROJECT_NAME} PUBLIC ${BOOST_FILE})

if(CMAKE_GENERATOR STREQUAL Xcode)
    configure_file(
            WorkspaceSettings.xcsettings
            ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.xcodeproj/project.xcworkspace/xcuserdata/$ENV{USER}.xcuserdata/WorkspaceSettings.xcsettings
            COPYONLY
    )
endif()
