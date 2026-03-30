cmake_minimum_required(VERSION 3.22)
project(RC20Clone VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── JUCE via FetchContent ──────────────────────────────────────────────────────
include(FetchContent)

FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.4        # update to latest 8.x tag as needed
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

# ── Plugin target ──────────────────────────────────────────────────────────────
# AU is Mac-only; on other platforms build VST3 only.
if(APPLE)
    set(PLUGIN_FORMATS VST3 AU)
else()
    set(PLUGIN_FORMATS VST3)
endif()

juce_add_plugin(RC20Clone
    VERSION                     0.1.0
    COMPANY_NAME                "RC20Clone"
    PLUGIN_MANUFACTURER_CODE    Rc2C
    PLUGIN_CODE                 Rc20
    FORMATS                     ${PLUGIN_FORMATS}
    PRODUCT_NAME                "RC-20 Clone"
    IS_SYNTH                    FALSE
    NEEDS_MIDI_INPUT            FALSE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD     FALSE
    VST3_CATEGORIES             "Fx"
    AU_MAIN_TYPE                kAudioUnitType_Effect
)

# ── Sources ────────────────────────────────────────────────────────────────────
target_sources(RC20Clone
    PRIVATE
        source/PluginProcessor.cpp
        source/PluginEditor.cpp
        source/dsp/DriftGenerator.cpp
        source/dsp/ModulatedDelayLine.cpp
        source/modules/NoiseModule.cpp
        source/modules/WobbleModule.cpp
        source/modules/DistortionModule.cpp
        source/modules/SpaceModule.cpp
        source/modules/MagicModule.cpp
        source/modules/LimitModule.cpp
        source/ui/ModuleComponent.cpp
        source/ui/PresetSelector.cpp
)

# ── Compile definitions ────────────────────────────────────────────────────────
target_compile_definitions(RC20Clone
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

# ── Link libraries ─────────────────────────────────────────────────────────────
target_link_libraries(RC20Clone
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
