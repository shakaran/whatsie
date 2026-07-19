# Unit tests (QtTest). Built only when -DWHATLY_TESTS=ON. Kept lightweight: they
# link just the widgets under test and their direct dependencies, not the whole
# app (no WebEngine), so they build and run fast and headless (offscreen).
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test Widgets Gui Svg)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_SOURCE_DIR}/src)

add_executable(tst_ui_assets
    tst_ui_assets.cpp
    ${CMAKE_SOURCE_DIR}/src/about.cpp
    ${CMAKE_SOURCE_DIR}/src/rateapp.cpp
    ${CMAKE_SOURCE_DIR}/src/utils.cpp
    ${CMAKE_SOURCE_DIR}/src/debuglog.cpp
    ${CMAKE_SOURCE_DIR}/src/appprofile.cpp
    ${CMAKE_SOURCE_DIR}/src/icons.qrc
)

target_include_directories(tst_ui_assets PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}
)

# The app sources expect these build-stamp macros; give them test values.
target_compile_definitions(tst_ui_assets PRIVATE
    VERSIONSTR="test"
    GIT_HASH="test"
    GIT_BRANCH="test"
    BUILD_TIMESTAMP="test"
)

target_link_libraries(tst_ui_assets PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Svg
)

# Run headless.
add_test(NAME ui_assets COMMAND tst_ui_assets)
set_tests_properties(ui_assets PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
