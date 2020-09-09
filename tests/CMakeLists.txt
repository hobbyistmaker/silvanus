cmake_policy(SET CMP0048 NEW)
cmake_minimum_required(VERSION 3.17)
#
#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED True)
##set(CMAKE_CXX_FLAGS "-Wall -Wextra")
#set(CMAKE_CXX_FLAGS_DEBUG "-g")
#set(CMAKE_CXX_FLAGS_RELEASE "-O2")
#
#set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum Fusion 360 supported version of MacOS/OSX.")
#set(CMAKE_OSX_SYSROOT "/Library/Developer/CommandLineTools/SDKs/MacOSX10.13.sdk")
#set_property(GLOBAL PROPERTY USE_FOLDERS ON)
#
#if(APPLE)
#    set(AUTODESK_API_FOLDER "$ENV{HOME}/Library/Application\ Support/Autodesk/Autodesk\ Fusion\ 360/API")
#    set(SILVANUS_LIB "${PROJECT_NAME}.dylib")
#    set(LOCAL_INCLUDES "/usr/local/include")
#    set(LOCAL_LIB "/usr/local/lib")
#elseif(WIN32)
#    set(AUTODESK_HASH d2d901e2ef5ae5c384b4ac001a48c4e37589dc43) # Pulled from the Autodesk folder name under your APPDATA folder
#    set(AUTODESK_API_FOLDER "$ENV{APPDATA}/Autodesk/Autodesk Fusion 360/API")
#    set(SILVANUS_LIB "${PROJECT_NAME}.dll")
#endif(APPLE)
#
#set(AUTODESK_ADDINS_FOLDER "${AUTODESK_API_FOLDER}/Addins")
#set(AUTODESK_FOLDER "${AUTODESK_API_FOLDER}/CPP")
#set(F360_INCLUDES "${AUTODESK_FOLDER}/include")
#set(F360_LIBS "${AUTODESK_FOLDER}/lib")
#
#
#if (MSVC_VERSION GREATER_EQUAL "1900") # Make sure that the Visual Studio compiler is using C++17
#    include(CheckCXXCompilerFlag)
#    CHECK_CXX_COMPILER_FLAG("/std:c++17" _cpp_latest_flag_supported)
#    if (_cpp_latest_flag_supported)
#        add_compile_options("/std:c++17")
#    endif()
#endif()
#
#file(GLOB_RECURSE SOURCE_FILES
#        LIST_DIRECTORIES true
#        lib/*.cpp
#        lib/*.hpp
#        lib/*.h
#        )
#message( STATUS "Target name: ${TARGET_NAME}" )
#message( STATUS "Users home folder: $ENV{HOME}" )
#message( STATUS "Autodesk Fusion 360 Includes folder: ${F360_INCLUDES}" )
#message( STATUS "Target file destination: ${CMAKE_CURRENT_SOURCE_DIR}" )
#message( STATUS "Binary dir: ${CMAKE_BINARY_DIR}" )
#message( STATUS "Source dir: ${CMAKE_SOURCE_DIR}" )
#message( STATUS "Found sources: ${SOURCE_FILES}" )
#if(WIN32)
#    message( STATUS "App data: $ENV{LOCALAPPDATA}")
#endif()
#
#link_directories("${F360_LIBS}")
#
#set(TEST_LIST SilvanusPro)

#foreach(NAME IN LISTS TEST_LIST)
#    list(APPEND TEST_SOURCE_LIST ${NAME}.test.cpp)
#endforeach()
#list(APPEND TEST_SOURCE_LIST SilvanusPro.test.cpp)
#
#set(TARGET_NAME tests)
#
#add_executable(${TARGET_NAME} main.cpp ${TEST_SOURCE_LIST})
#target_link_libraries(${TARGET_NAME} PUBLIC SilvanusPro)
#target_include_directories(${TARGET_NAME} PUBLIC )
#target_include_directories(${TARGET_NAME} PRIVATE "${F360_INCLUDES}/Core")
#target_include_directories(${TARGET_NAME} PRIVATE "${F360_INCLUDES}/Core/UserInterface")
#
#if(APPLE)
#    target_include_directories(${TARGET_NAME} PRIVATE "${LOCAL_INCLUDES}")
#    target_include_directories(${TARGET_NAME} PRIVATE "${LOCAL_INCLUDES}/catch2")
#endif()
#
#if(APPLE)
#    target_include_directories(${TARGET_NAME} PRIVATE "${LOCAL_INCLUDES}")
#    target_include_directories(${TARGET_NAME} PRIVATE "${LOCAL_INCLUDES}/entt")
#    find_library(ADSK_CORE core.dylib PATHS "${F360_LIBS}/")
#    find_library(ADSK_FUSION fusion.dylib PATHS "${F360_LIBS}/")
#    find_library(ADSK_CAM cam.dylib PATHS "${F360_LIBS}/")
#    find_library(BOOST_FILE libboost_filesystem.a PATHS "${LOCAL_LIB}")
#elseif(WIN32)
#    find_package(EnTT CONFIG REQUIRED) # Assumes EnTT installed through vcpkg with Visual Studio
#    target_link_libraries(${TARGET_NAME} PRIVATE EnTT::EnTT)
#    find_library(ADSK_CORE core.lib PATHS "${F360_LIBS}/")
#    find_library(ADSK_FUSION fusion.lib PATHS "${F360_LIBS}/")
#    find_library(ADSK_CAM cam.lib PATHS "${F360_LIBS}/")
#endif()
#
#target_link_libraries(${TARGET_NAME} PUBLIC ${ADSK_CORE} ${ADSK_FUSION} ${ADSK_CAM})
#target_link_libraries(${TARGET_NAME} PUBLIC ${BOOST_FILE})
#
#if(CMAKE_GENERATOR STREQUAL Xcode)
#    configure_file(
#            WorkspaceSettings.xcsettings
#            ${CMAKE_BINARY_DIR}/${TARGET_NAME}.xcodeproj/project.xcworkspace/xcuserdata/$ENV{USER}.xcuserdata/WorkspaceSettings.xcsettings
#            COPYONLY
#    )
#endif()
#
#add_test(
#        NAME ${TARGET_NAME}
#        COMMAND ${TARGET_NAME} -o report.xml -r junit
#)