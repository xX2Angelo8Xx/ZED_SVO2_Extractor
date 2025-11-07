# Video Extractor Application

# Executable
add_executable(video_extractor_cli
    video_extractor_cli.cpp
)

# Include directories
target_include_directories(video_extractor_cli
    PRIVATE
        ${CMAKE_SOURCE_DIR}/common
        ${ZED_INCLUDE_DIRS}
        ${OpenCV_INCLUDE_DIRS}
        ${CUDAToolkit_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(video_extractor_cli
    PRIVATE
        zed_common
        ${ZED_LIBRARIES}
        ${OpenCV_LIBS}
)

# Compiler flags
if(MSVC)
    target_compile_options(video_extractor_cli PRIVATE
        /W4                 # Warning level 4
        /WX-                # Warnings not as errors
        /MP                 # Multi-processor compilation
        /permissive-        # Standards conformance
        /wd4201             # Suppress: nonstandard extension (ZED SDK)
        /wd4251             # Suppress: DLL interface warnings (ZED SDK)
        /wd4305             # Suppress: truncation warnings (ZED SDK)
        /wd4100             # Suppress: unreferenced parameter (ZED SDK)
    )
    
    # Add DLL directories to PATH for debugging
    set_target_properties(video_extractor_cli PROPERTIES
        VS_DEBUGGER_ENVIRONMENT "PATH=${ZED_DLL_DIR};${OpenCV_DLL_DIR};%PATH%"
    )
endif()

# Set output directory
set_target_properties(video_extractor_cli PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

# IDE folder organization
set_target_properties(video_extractor_cli PROPERTIES FOLDER "Applications")

# Installation
install(TARGETS video_extractor_cli
        RUNTIME DESTINATION bin)
