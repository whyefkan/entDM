cmake_minimum_required(VERSION 3.16)
project(entDM VERSION 0.1.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)

if(WIN32)
    find_package(glfw3 CONFIG REQUIRED)
    find_package(CURL CONFIG REQUIRED)
    set(ENTDM_GLFW_TARGET glfw)
    set(ENTDM_CURL_TARGET CURL::libcurl)
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GLFW REQUIRED IMPORTED_TARGET glfw3)
    set(ENTDM_GLFW_TARGET PkgConfig::GLFW)
endif()

set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui)

add_library(imgui STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC ${IMGUI_DIR} ${IMGUI_DIR}/backends)
target_link_libraries(imgui PUBLIC ${ENTDM_GLFW_TARGET} OpenGL::GL)

if(WIN32)
    set(ENTDM_USE_VENDOR_CURL FALSE)
else()
    # Development machines may keep a local curl dev tree in third_party/curl.
    set(ENTDM_VENDOR_CURL "${CMAKE_CURRENT_SOURCE_DIR}/third_party/curl/usr")
    if(EXISTS "${ENTDM_VENDOR_CURL}/include/x86_64-linux-gnu/curl/curl.h")
        set(ENTDM_CURL_INCLUDE "${ENTDM_VENDOR_CURL}/include/x86_64-linux-gnu")
        set(ENTDM_CURL_LIBRARY "${ENTDM_VENDOR_CURL}/lib/x86_64-linux-gnu/libcurl.so")
        set(ENTDM_USE_VENDOR_CURL TRUE)
    else()
        pkg_check_modules(CURL REQUIRED IMPORTED_TARGET libcurl)
        set(ENTDM_CURL_TARGET PkgConfig::CURL)
        set(ENTDM_USE_VENDOR_CURL FALSE)
    endif()
endif()

add_executable(entdm src/main.cpp src/platform.cpp)
target_compile_definitions(entdm PRIVATE
    ENTDM_EXTENSION_DIR="${CMAKE_CURRENT_SOURCE_DIR}/extension"
    ENTDM_VERSION="${PROJECT_VERSION}")

if(WIN32)
    target_compile_definitions(entdm PRIVATE NOMINMAX _CRT_SECURE_NO_WARNINGS)
    target_link_libraries(entdm PRIVATE imgui ${ENTDM_CURL_TARGET} Threads::Threads
        ws2_32 shell32 psapi)
else()
    if(ENTDM_USE_VENDOR_CURL)
        target_include_directories(entdm PRIVATE ${ENTDM_CURL_INCLUDE})
        target_link_libraries(entdm PRIVATE imgui ${ENTDM_CURL_LIBRARY} Threads::Threads)
    else()
        target_link_libraries(entdm PRIVATE imgui ${ENTDM_CURL_TARGET} Threads::Threads)
    endif()
endif()

if(WIN32)
    install(TARGETS entdm RUNTIME DESTINATION .)
    install(DIRECTORY extension DESTINATION .)
    install(FILES README.md LICENSE NOTICE DESTINATION .)
    set(CPACK_GENERATOR "ZIP")
else()
    install(TARGETS entdm RUNTIME DESTINATION bin)
    install(DIRECTORY extension DESTINATION share/entdm)
    install(FILES README.md LICENSE NOTICE DESTINATION share/doc/entdm)
    install(FILES packaging/linux/entdm.desktop DESTINATION share/applications)
    install(FILES assets/icon.png DESTINATION share/icons/hicolor/512x512/apps RENAME entdm.png)
    set(CPACK_GENERATOR "TGZ;DEB")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "whyefkan")
    set(CPACK_DEBIAN_PACKAGE_SECTION "net")
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
endif()

set(CPACK_PACKAGE_NAME "entdm")
set(CPACK_PACKAGE_VENDOR "whyefkan")
set(CPACK_PACKAGE_CONTACT "whyefkan")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Fast, lightweight multi-connection download manager")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
include(CPack)
