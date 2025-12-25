cmake_minimum_required(VERSION 3.22)

project(Dawdrey VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add JUCE using FetchContent
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        master # Use master for macOS 15 support
)
FetchContent_MakeAvailable(JUCE)

# Plugin Format Configuration
juce_add_plugin(Dawdrey
    COMPANY_NAME "KhasMek" # Or your name/company
    BUNDLE_ID "com.KhasMek.Dawdrey"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE "Khas"
    PLUGIN_CODE "Dawd"
    FORMATS AU VST3 AUv3 Standalone
    PRODUCT_NAME "Dawdrey"
)

juce_generate_juce_header(Dawdrey)

# Source Files
target_sources(Dawdrey PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/DSP/FeedbackSynthEngine.cpp
    Source/DSP/FeedbackSynthEngine.h
    Source/DSP/KarplusString.cpp
    Source/DSP/KarplusString.h
    Source/DSP/BiquadFilters.cpp
    Source/DSP/BiquadFilters.cpp
    Source/DSP/BiquadFilters.h
    Source/DSP/PitchShifter.h
    Source/DSP/daisysp/Overdrive.cpp
    Source/DSP/daisysp/Overdrive.h
    Source/DSP/daisysp/ReverbSc.cpp
    Source/DSP/daisysp/ReverbSc.h
    Source/PresetManager.cpp
    Source/PresetManager.h
)

# Include Directories
target_include_directories(Dawdrey PRIVATE
    Source
    Source/DSP
)

# Link Libraries
target_link_libraries(Dawdrey PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_core
    juce::juce_data_structures
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_dsp
    DawdreyAssets
)

# Compile Definitions
target_compile_definitions(Dawdrey PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

# Binary Data
juce_add_binary_data(DawdreyAssets SOURCES
    Resources/Metropolis-Regular.otf
)
