/**
 * @file main.cpp
 * @brief Entry point for ZED SVO2 Extractor GUI
 * @author Angelo Amon (xX2Angelo8Xx)
 * @date November 7, 2025
 */

#include "gui_application.hpp"
#include <iostream>

int main(int argc, char** argv) {
    std::cout << "=== ZED SVO2 Extractor GUI ===" << std::endl;
    std::cout << "Version 0.1.0" << std::endl;
    std::cout << "Author: Angelo Amon (xX2Angelo8Xx)" << std::endl;
    std::cout << std::endl;

    // Create application
    zed_gui::GUIApplication app;

    // Initialize
    if (!app.initialize(1280, 720, "ZED SVO2 Extractor")) {
        std::cerr << "Failed to initialize GUI application" << std::endl;
        return 1;
    }

    // Run main loop
    app.run();

    // Cleanup
    app.shutdown();

    return 0;
}
