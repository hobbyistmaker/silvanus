cmake_policy(SET CMP0048 NEW)
cmake_minimum_required(VERSION 3.17)
project (SilvanusPro VERSION 0.5.5.0)

enable_testing()
add_subdirectory(src)
add_subdirectory(tests)

if(APPLE)
    set(LOCAL_INCLUDES "/usr/local/include")
    set(LOCAL_LIB "/usr/local/lib")
    set(AUTODESK_API_FOLDER "$ENV{HOME}/Library/Application\ Support/Autodesk/Autodesk\ Fusion\ 360/API")
    set(SILVANUS_LIB "${PROJECT_NAME}.dylib")
elseif(WIN32)
    string(REPLACE "\\" "/" USER_HOME "$ENV{APPDATA}")
    set(AUTODESK_API_FOLDER "${USER_HOME}/Autodesk/Autodesk Fusion 360/API")
    set(SILVANUS_LIB "${PROJECT_NAME}.dll")
endif()

set(AUTODESK_ADDINS_FOLDER "${AUTODESK_API_FOLDER}/Addins")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INSTALL_PREFIX "${AUTODESK_ADDINS_FOLDER}" CACHE PATH "Install Add-in" FORCE)

install(TARGETS "${PROJECT_NAME}" DESTINATION "${AUTODESK_ADDINS_FOLDER}/${PROJECT_NAME}")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/src/${SILVANUS_LIB}" DESTINATION "${AUTODESK_ADDINS_FOLDER}/${PROJECT_NAME}")
install(FILES "${CMAKE_SOURCE_DIR}/${PROJECT_NAME}.manifest" DESTINATION "${AUTODESK_ADDINS_FOLDER}/${PROJECT_NAME}")
install(DIRECTORY "${CMAKE_SOURCE_DIR}/resources" DESTINATION "${AUTODESK_ADDINS_FOLDER}/${PROJECT_NAME}")
