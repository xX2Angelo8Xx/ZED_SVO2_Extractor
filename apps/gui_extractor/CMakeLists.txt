# GUI Extractor Application
cmake_minimum_required(VERSION 3.15)

# Add GUI executable
add_executable(gui_extractor
    main.cpp
    gui_application.cpp
    gui_application.hpp
)

# Include directories
target_include_directories(gui_extractor PRIVATE
    ${PROJECT_SOURCE_DIR}/common
    ${PROJECT_SOURCE_DIR}/external/imgui
    ${PROJECT_SOURCE_DIR}/external/imgui/backends
)

# Link libraries
target_link_libraries(gui_extractor PRIVATE
    zed_common
    imgui
    ${ZED_LIBS}
    ${OpenCV_LIBS}
)

# Windows-specific settings
# if(WIN32)
#     set_target_properties(gui_extractor PROPERTIES
#         WIN32_EXECUTABLE TRUE  # Prevents console window (disabled for debugging)
#     )
# endif()

# Set output directory
set_target_properties(gui_extractor PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

message(STATUS "GUI Extractor configured")
