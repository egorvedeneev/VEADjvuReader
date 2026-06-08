cmake_minimum_required(VERSION 3.20)
project(DjVuReader VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ── Qt ────────────────────────────────────────────────────────────────────────
find_package(Qt6 REQUIRED COMPONENTS Core Widgets PrintSupport)

# ── libdjvulibre ──────────────────────────────────────────────────────────────
# Windows (vcpkg): vcpkg install djvulibre:x64-windows
# macOS  (brew):   brew install djvulibre
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(DJVU ddjvuapi)
endif()

if(NOT DJVU_FOUND)
    # Fallback: ищем вручную
    find_path(DJVU_INCLUDE_DIR   NAMES libdjvu/ddjvuapi.h djvulibre/ddjvuapi.h)
    find_library(DJVU_LIBRARY    NAMES djvulibre djvu)
    if(DJVU_INCLUDE_DIR AND DJVU_LIBRARY)
        set(DJVU_FOUND TRUE)
        set(DJVU_INCLUDE_DIRS ${DJVU_INCLUDE_DIR})
        set(DJVU_LIBRARIES    ${DJVU_LIBRARY})
        message(STATUS "Found DjVuLibre manually: ${DJVU_LIBRARY}")
    else()
        message(WARNING
            "libdjvulibre not found!\n"
            "  Windows: vcpkg install djvulibre:x64-windows\n"
            "  macOS:   brew install djvulibre\n"
            "Building in STUB mode (no real DjVu rendering).")
        add_compile_definitions(DJVU_STUB_MODE)
    endif()
endif()

# ── Sources ───────────────────────────────────────────────────────────────────
set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/DjVuDocument.cpp
    src/DjVuDocument.h
    src/PageView.cpp
    src/PageView.h
    src/ThumbnailPanel.cpp
    src/ThumbnailPanel.h
)

qt_add_executable(DjVuReader ${SOURCES})

target_include_directories(DjVuReader PRIVATE src)

if(DJVU_FOUND)
    target_include_directories(DjVuReader PRIVATE ${DJVU_INCLUDE_DIRS})
    target_link_libraries(DjVuReader PRIVATE ${DJVU_LIBRARIES})
endif()

target_link_libraries(DjVuReader PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::PrintSupport
)

# ── macOS bundle ──────────────────────────────────────────────────────────────
if(APPLE)
    set_target_properties(DjVuReader PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER  com.opensource.djvureader
        MACOSX_BUNDLE_BUNDLE_VERSION  ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
    )
endif()

# ── Windows: убираем консоль в Release ────────────────────────────────────────
if(WIN32)
    set_target_properties(DjVuReader PROPERTIES
        WIN32_EXECUTABLE $<CONFIG:Release>
    )
endif()

# ── Install ───────────────────────────────────────────────────────────────────
install(TARGETS DjVuReader
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_generate_deploy_app_script(
    TARGET DjVuReader
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
