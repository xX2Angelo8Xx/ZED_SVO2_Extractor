# Common Library - Shared utilities for all applications

# Source files
set(COMMON_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/metadata.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/file_utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/svo_handler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/error_handler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/output_manager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extraction_engine.cpp
)

# Header files
set(COMMON_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/metadata.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/file_utils.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/svo_handler.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/error_handler.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/output_manager.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/extraction_engine.hpp
)

# Create static library
add_library(zed_common STATIC ${COMMON_SOURCES} ${COMMON_HEADERS})

# Include directories
target_include_directories(zed_common
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${ZED_INCLUDE_DIRS}
        ${OpenCV_INCLUDE_DIRS}
        ${CUDAToolkit_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(zed_common
    PUBLIC
        ${ZED_LIBRARIES}
        ${OpenCV_LIBS}
)

# Compiler-specific flags
if(MSVC)
    target_compile_options(zed_common PRIVATE
        /W4                 # Warning level 4
        /WX-                # Warnings not as errors (for rapid prototyping)
        /MP                 # Multi-processor compilation
        /permissive-        # Standards conformance
        /wd4201             # Suppress: nonstandard extension (ZED SDK)
        /wd4251             # Suppress: DLL interface warnings (ZED SDK)
        /wd4305             # Suppress: truncation warnings (ZED SDK)
        /wd4100             # Suppress: unreferenced parameter (ZED SDK)
    )
    
    # Add ZED SDK and OpenCV DLL directories to PATH for debugging
    set_target_properties(zed_common PROPERTIES
        VS_DEBUGGER_ENVIRONMENT "PATH=${ZED_DLL_DIR};${OpenCV_DLL_DIR};%PATH%"
    )
endif()

# Set output name
set_target_properties(zed_common PROPERTIES
    OUTPUT_NAME "zed_common"
    DEBUG_POSTFIX "d"
)

# IDE folder organization
set_target_properties(zed_common PROPERTIES FOLDER "Libraries")

# Installation
install(TARGETS zed_common
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib)

install(FILES ${COMMON_HEADERS}
        DESTINATION include/zed_common)
