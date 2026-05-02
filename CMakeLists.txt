cmake_minimum_required(VERSION 3.22)

project(SoundscapeApplication VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(external/JUCE)

juce_add_gui_app(SoundscapeApplication
    PRODUCT_NAME "Soundscape Application"
    COMPANY_NAME ""
    VERSION "${PROJECT_VERSION}"
)

target_sources(SoundscapeApplication
    PRIVATE
        Source/Main.cpp
        Source/MainComponent.cpp
        Source/MainComponent.h
        Source/Player/Player.cpp
        Source/Player/Player.h
        Source/Player/PlayerGUI.cpp
        Source/Player/PlayerGUI.h
        Source/Processor/Processor.cpp
        Source/Processor/Processor.h
)

target_include_directories(SoundscapeApplication
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/JuceLibraryCode
)

target_compile_definitions(SoundscapeApplication
    PRIVATE
        JUCE_DISPLAY_SPLASH_SCREEN=1
        JUCE_USE_DARK_SPLASH_SCREEN=1
        JUCE_USE_MP3AUDIOFORMAT=1
        JUCE_STRICT_REFCOUNTEDPOINTER=1
)

target_link_libraries(SoundscapeApplication
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_gui_extra
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

add_custom_command(TARGET SoundscapeApplication POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/Resources
            $<TARGET_FILE_DIR:SoundscapeApplication>/Resources
)