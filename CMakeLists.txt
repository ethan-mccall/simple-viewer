# SimpleViewer
# Copyright (C) 2026 Ethan McCall
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

cmake_minimum_required(VERSION 3.19)
project(SimpleViewer LANGUAGES CXX)

find_package(Qt6 6.2 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(SimpleViewer
    MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    welcome.txt
    changelog.html
    snap/snapcraft.yaml
    README.md
)

qt_add_resources(SimpleViewer "app_icons"
    PREFIX "/icons"
    BASE "icons"
    FILES
        icons/icon.ico
)

add_custom_command(TARGET SimpleViewer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/welcome.txt
        $<TARGET_FILE_DIR:SimpleViewer>/welcome.txt
    COMMENT "Copying welcome.txt next to the executable"
)

add_custom_command(TARGET SimpleViewer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/LICENSE
        $<TARGET_FILE_DIR:SimpleViewer>/LICENSE
    COMMENT "Copying LICENSE next to the executable"
)

add_custom_command(TARGET SimpleViewer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/changelog.html
        $<TARGET_FILE_DIR:SimpleViewer>/changelog.html
    COMMENT "Copying changelog.html next to the executable"
)

add_custom_command(TARGET SimpleViewer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/simple-viewer.desktop
        $<TARGET_FILE_DIR:SimpleViewer>/simple-viewer.desktop
    COMMENT "Copying simple-viewer.desktop next to the executable"
)

add_custom_command(TARGET SimpleViewer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/icons/icon.ico
        $<TARGET_FILE_DIR:SimpleViewer>/simple-viewer.ico
    COMMENT "Copying application icon next to the executable for dev .desktop"
)

add_custom_command(TARGET SimpleViewer POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        -D BINARY=$<TARGET_FILE:SimpleViewer>
        -D ICON=$<TARGET_FILE_DIR:SimpleViewer>/simple-viewer.ico
        -D OUTPUT_DIR=$<TARGET_FILE_DIR:SimpleViewer>
        -P ${CMAKE_SOURCE_DIR}/generate-dev-desktop.cmake
    COMMENT "Generating simple-viewer-dev.desktop with full paths for development"
)

target_link_libraries(SimpleViewer
    PRIVATE
        Qt::Core
        Qt::Widgets
)

include(GNUInstallDirs)

install(TARGETS SimpleViewer
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(FILES ${CMAKE_SOURCE_DIR}/simple-viewer.desktop
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications
)

install(FILES ${CMAKE_SOURCE_DIR}/icons/icon.ico
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/256x256/apps
    RENAME simple-viewer.ico
)

install(FILES ${CMAKE_SOURCE_DIR}/welcome.txt
              ${CMAKE_SOURCE_DIR}/LICENSE
              ${CMAKE_SOURCE_DIR}/changelog.html
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)

option(DEPLOY_QT_RUNTIME "Deploy full Qt runtime (for standalone packaging)" OFF)

if(DEPLOY_QT_RUNTIME)
    qt_generate_deploy_app_script(
        TARGET SimpleViewer
        OUTPUT_SCRIPT deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR
    )
    install(SCRIPT ${deploy_script})
endif()

set(CPACK_PACKAGE_NAME "simple-viewer")
set(CPACK_PACKAGE_VENDOR "Ethan McCall")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Lightweight image viewer")
set(CPACK_PACKAGE_DESCRIPTION "Simple Viewer is a fast and lightweight image viewer with support for crop, basic transforms, folder navigation, and common image formats.")

set(CPACK_PACKAGE_VERSION "1.0")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "0")

set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Ethan McCall <your-email@example.com>")  # ← Change this
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/ethan-mccall/SimpleViewer")
set(CPACK_DEBIAN_PACKAGE_SECTION "graphics")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")

set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.34), libqt6core6 (>= 6.2), libqt6widgets6 (>= 6.2), libqt6gui6 (>= 6.2)")

set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_STRIP_FILES ON)

include(CPack)
