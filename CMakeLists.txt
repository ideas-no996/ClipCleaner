cmake_minimum_required(VERSION 3.21)

project(ClipCleaner VERSION 0.1.0 LANGUAGES CXX)

add_executable(ClipCleaner WIN32
    src/main.cpp
    src/text_cleaner.cpp
    src/text_cleaner.h
)

target_compile_features(ClipCleaner PRIVATE cxx_std_17)
target_compile_definitions(ClipCleaner PRIVATE
    UNICODE
    _UNICODE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)

if(MSVC)
    set_property(TARGET ClipCleaner PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    )
    target_compile_options(ClipCleaner PRIVATE /W4 /permissive-)
endif()

target_link_libraries(ClipCleaner PRIVATE
    user32
    shell32
)

add_executable(ClipCleanerTextTests
    tests/text_cleaner_tests.cpp
    src/text_cleaner.cpp
    src/text_cleaner.h
)

target_compile_features(ClipCleanerTextTests PRIVATE cxx_std_17)
target_compile_definitions(ClipCleanerTextTests PRIVATE
    UNICODE
    _UNICODE
)

if(MSVC)
    set_property(TARGET ClipCleanerTextTests PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    )
    target_compile_options(ClipCleanerTextTests PRIVATE /W4 /permissive-)
endif()

set_target_properties(ClipCleaner PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Release"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/Release"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/Release"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_SOURCE_DIR}/Release"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_SOURCE_DIR}/Release"
)
