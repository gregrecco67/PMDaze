cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set_property(GLOBAL PROPERTY USE_FOLDERS YES)
set(CMAKE_XCODE_GENERATE_SCHEME OFF)
set(PROJECT_NAME "PMDaze")
set(PRODUCT_NAME "PM Daze")
set(COMPANY_NAME "Void-Star")
set(BUNDLE_ID "com.void-star.pm-daze")
set(MAJOR_VERSION 0)
set(MINOR_VERSION 2)
set(PATCH_LEVEL 0)
set(CURRENT_VERSION "${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_LEVEL}")
set(FORMATS VST3 Standalone) #Standalone AU VST3 LV2)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
message(STATUS "CMAKE_BUILD_TYPE unspecified; picking Release")
set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif()

message(STATUS "source_dir = ${CMAKE_CURRENT_SOURCE_DIR}")

if (WIN32)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE INTERNAL "")
endif ()

# Color our warnings and errors
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
add_compile_options(-fdiagnostics-color=always)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
add_compile_options(-fcolor-diagnostics)
endif ()

project(${PROJECT_NAME})

add_subdirectory(modules/JUCE)

foreach(module_name IN ITEMS gin gin_dsp gin_graphics gin_gui gin_plugin gin_simd)
	juce_add_module (
		"${CMAKE_CURRENT_LIST_DIR}/modules/Gin/modules/${module_name}"
		)

	set_property (TARGET "${module_name}" APPEND PROPERTY LABELS Gin)
endforeach()

add_definitions( -DVERSION_STRING="${CURRENT_VERSION}" )
# also set this in PluginEditor.cpp -> showAboutInfo()

juce_add_plugin (${PROJECT_NAME}
				PRODUCT_NAME ${PRODUCT_NAME}
				VERSION ${CURRENT_VERSION}
				COMPANY_NAME ${COMPANY_NAME}
				COMPANY_WEBSITE "https://void-star.com/"
    			ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/assets/icon.png"
				FORMATS "${FORMATS}"
				BUNDLE_ID "${BUNDLE_ID}"
				DESCRIPTION "Phase modulation synthesizer."
				PLUGIN_MANUFACTURER_CODE "Vdst"
                PLUGIN_CODE "PMDz" 
				AU_MAIN_TYPE "kAudioUnitType_MusicDevice"
				IS_SYNTH TRUE
				NEEDS_MIDI_INPUT TRUE
				NEEDS_MIDI_OUTPUT FALSE
				COPY_PLUGIN_AFTER_BUILD TRUE
				LV2URI "urn:void-star:pm-daze:${CURRENT_VERSION}"
				JUCE_DONT_DECLARE_PROJECTINFO TRUE
			)


if (APPLE)
	target_compile_options(${PROJECT_NAME} PRIVATE 
			-Wall -Wstrict-aliasing -Wunused-parameter -Wconditional-uninitialized -Woverloaded-virtual -Wreorder -Wconstant-conversion -Wbool-conversion -Wextra-semi 
			-Wunreachable-code -Winconsistent-missing-destructor-override -Wshift-sign-overflow -Wnullable-to-nonnull-conversion -Wuninitialized -Wno-missing-field-initializers 
			-Wno-ignored-qualifiers -Wno-missing-braces -Wno-char-subscripts -Wno-unused-private-field -fno-aligned-allocation -Wunused-private-field -Wunreachable-code 
			-Wenum-compare -Wshadow -Wfloat-conversion -Wshadow-uncaptured-local -Wshadow-field -Wsign-compare -Wdeprecated-this-capture -Wimplicit-float-conversion 
			-ffast-math -fno-finite-math-only -Wfloat-equal
#			"$<$<CONFIG:RELEASE>:-Werror>"
		)
		add_definitions( -DUSE_NEON )
endif ()

if (MSVC)
	target_compile_options(${PROJECT_NAME} PRIVATE 
			/wd26495 /wd4459 /wd4244 /wd4996 /wd4267 /wd4244
		)
    add_compile_definitions(MIPP_ALIGNED_LOADS USE_SSE)
endif ()

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
	add_compile_definitions(USE_SSE)
endif ()

# Binary Data

set_property (DIRECTORY APPEND PROPERTY LABELS Assets)

file (GLOB_RECURSE source_files CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/source/third_party/MTS-ESP/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/third_party/MTS-ESP/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/third_party/MTS-ESP/*.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/third_party/hiir/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/source/third_party/hiir/*.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/source/third_party/hiir/*.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/third_party/ADAAsrc/polylogarithm/Li2.*
	${CMAKE_CURRENT_SOURCE_DIR}/source/third_party/ADAAsrc/*.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/third_party/ADAAsrc/ADAA/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/dsp/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/dsp/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/dsp/*.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/source/ui/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/source/ui/*.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/ui/*.hpp
	)

target_sources (${PROJECT_NAME} PRIVATE ${source_files})
source_group (TREE ${CMAKE_CURRENT_SOURCE_DIR}/source PREFIX Source FILES ${source_files})
target_include_directories (${PROJECT_NAME} PUBLIC 
	${CMAKE_CURRENT_SOURCE_DIR}/source
	${CMAKE_CURRENT_SOURCE_DIR}/source/third_party
	${CMAKE_CURRENT_SOURCE_DIR}/source/dsp
	${CMAKE_CURRENT_SOURCE_DIR}/source/ui
	)

file(GLOB_RECURSE AssetFiles CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/*.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/*.svg"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/*.otf"
	"${CMAKE_CURRENT_SOURCE_DIR}/assets/*.json"
	"${CMAKE_CURRENT_SOURCE_DIR}/assets/*.wav"
	"${CMAKE_CURRENT_SOURCE_DIR}/assets/*.xml"
)

juce_add_binary_data(Assets SOURCES ${AssetFiles})

# Required for Linux happiness:
set_target_properties(Assets PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

set_property(GLOBAL PROPERTY USE_FOLDERS YES)

target_link_libraries (${PROJECT_NAME} 
					PRIVATE
						gin
						gin_graphics
						gin_gui
						gin_dsp
						gin_plugin
						gin_simd
						juce::juce_events
						juce::juce_graphics
						juce::juce_gui_basics
						juce::juce_gui_extra
						juce::juce_core
						juce::juce_audio_basics

						$<$<PLATFORM_ID:Linux>:curl>
					PUBLIC
						Assets
						juce::juce_recommended_config_flags
						#juce::juce_recommended_lto_flags
						juce::juce_recommended_warning_flags
					)

if (APPLE)
	set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_CLANG_LINK_OBJC_RUNTIME "NO")
endif()

#juce_generate_juce_header (${PROJECT_NAME})

get_property (debug_configs GLOBAL PROPERTY DEBUG_CONFIGURATIONS)

if (NOT debug_configs)
    set (debug_configs Debug)
endif ()

set (config_is_debug "$<IN_LIST:$<CONFIG>,${debug_configs}>")
set (config_is_release "$<NOT:${config_is_debug}>")

target_compile_definitions (${PROJECT_NAME} PRIVATE 
								JUCE_MODAL_LOOPS_PERMITTED=1
								JUCE_COREGRAPHICS_DRAW_ASYNC=1
								JUCE_VST3_CAN_REPLACE_VST2=0
								JUCE_WEB_BROWSER=0
								JUCE_SILENCE_XCODE_15_LINKER_WARNING=1
							)

if (APPLE)
	set_target_properties(${PROJECT_NAME} PROPERTIES
			XCODE_ATTRIBUTE_DEPLOYMENT_POSTPROCESSING[variant=Release] YES
			XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH[variant=Debug] "YES"
            # XCODE_ATTRIBUTE_ARCHS '$(ARCHS_STANDARD)'
			)
endif()


set (env_file "${PROJECT_SOURCE_DIR}/.env")
message ("Writing ENV file for CI: ${env_file}")
# the first call truncates, the rest append
file(WRITE  "${env_file}" "PROJECT_NAME=${PROJECT_NAME}\n")
file(APPEND "${env_file}" "PRODUCT_NAME=${PRODUCT_NAME}\n")
file(APPEND "${env_file}" "VERSION=${CURRENT_VERSION}\n")
file(APPEND "${env_file}" "MAJOR_VERSION=${MAJOR_VERSION}\n")
file(APPEND "${env_file}" "MINOR_VERSION=${MINOR_VERSION}\n")
file(APPEND "${env_file}" "PATCH_LEVEL=${PATCH_LEVEL}\n")
file(APPEND "${env_file}" "BUNDLE_ID=${BUNDLE_ID}\n")
file(APPEND "${env_file}" "COMPANY_NAME=${COMPANY_NAME}\n")

