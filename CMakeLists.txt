#Build script of AcademmyScope
#Copyright (C) 2025 Volkan Orhan

#This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

#This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

#You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.16)

project(AcademyScope VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql)

set(PROJECT_SOURCES
        main.cpp
        Resources.qrc
        MainWindow.cpp MainWindow.hpp MainWindow.ui
        AboutDialog.hpp AboutDialog.cpp AboutDialog.ui
        EnumDefinitions.hpp
        BackEnd.cpp
        BackEnd.hpp
        Utils/SQLiteUtil.hpp Utils/SQLiteUtil.cpp
        Utils/StringUtil.hpp Utils/StringUtil.cpp
        Utils/DarkModeUtil.hpp Utils/DarkModeUtil.cpp
        TurkishFilterProxy.hpp TurkishFilterProxy.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(AcademyScope
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET AcademyScope APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(AcademyScope SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(AcademyScope
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(AcademyScope PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Sql)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.volkanorhan.academyscope)
endif()
set_target_properties(AcademyScope PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

####################
# macOS Liquid Glass Icon Support
#
# Copy Assets.car to Resources
set(ASSET_CAR ${CMAKE_SOURCE_DIR}/Resources/Icons/Assets.car)
set_source_files_properties(${ASSET_CAR} PROPERTIES
    MACOSX_PACKAGE_LOCATION "Resources")
target_sources(AcademyScope PRIVATE ${ASSET_CAR})

# Use custom Info.plist
set_target_properties(AcademyScope PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/macOS/Info.plist)

####################
# SQLite Support
#
# Copy YKS.sqlite only for non-Debug builds
# Ensure your source file name matches exactly (case-sensitive on Linux):
#   ${CMAKE_SOURCE_DIR}/Databases/YKS.sqlite
# If your actual file is "YKS.SQLite", change both places consistently.

# Copy YKS.sqlite into the app bundle (macOS only, non-Debug)
if(APPLE)
  set(BUNDLE_DB_DIR "$<TARGET_FILE_DIR:AcademyScope>/../Resources/Databases")

  add_custom_command(TARGET AcademyScope POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${BUNDLE_DB_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/Databases/YKS.sqlite"
            "${BUNDLE_DB_DIR}/YKS.SQLite"
    COMMENT "Copying YKS.sqlite into app bundle (macOS, non-Debug)"
    VERBATIM
    CONFIGURATIONS Release RelWithDebInfo MinSizeRel
  )
endif()

# Windows: copy next to the binary under Databases/ (Release-only)
if(WIN32)
  set(BIN_DB_DIR "$<TARGET_FILE_DIR:AcademyScope>/Databases")

  add_custom_command(TARGET AcademyScope POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${BIN_DB_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/Databases/YKS.sqlite"
            "${BIN_DB_DIR}/YKS.SQLite"
    COMMENT "Copying YKS.sqlite next to the binary (Windows, non-Debug)"
    VERBATIM
    CONFIGURATIONS Release RelWithDebInfo MinSizeRel
  )
endif()

# Linux: copy next to the binary under Databases/ (Release-only)
if(UNIX AND NOT APPLE)
  set(BIN_DB_DIR "$<TARGET_FILE_DIR:AcademyScope>/Databases")

  add_custom_command(TARGET AcademyScope POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${BIN_DB_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/Databases/YKS.sqlite"
            "${BIN_DB_DIR}/YKS.SQLite"
    COMMENT "Copying YKS.sqlite next to the binary (Linux, non-Debug)"
    VERBATIM
    CONFIGURATIONS Release RelWithDebInfo MinSizeRel
  )
endif()
#################

include(GNUInstallDirs)
install(TARGETS AcademyScope
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(AcademyScope)
endif()
