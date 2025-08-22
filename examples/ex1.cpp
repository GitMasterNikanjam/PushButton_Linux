/**
 * @file button_demo.cpp
 * @brief Example usage of the AUXIO-based Button library.
 *
 * Demonstrates:
 * - Configuring a GPIO pin as a button input
 * - Polling the button state in a loop
 * - Using interrupt-driven callbacks with debounce
 */

 // g++ -std=c++17 -O2 -lpthread -lgpiod -o button_demo button_demo.cpp Button.cpp AUXIO.cpp

 
#include <csignal>
#include <iostream>
#include "Button.h"

// GPIO line to use for button (adjust per board mapping)
constexpr uint8_t BUTTON_PIN = 17;   // Example: GPIO17
constexpr uint8_t PULL_MODE  = 2;    // 0=OFF, 1=DOWN, 2=UP

// Ctrl+C handler
std::atomic<bool> running{true};
void sigintHandler(int) { running = false; }

// Example callback for interrupts
void myButtonHandler(bool rising, long sec, long nsec) {
    if (rising)
        std::cout << "[CALLBACK] Rising edge at " << sec << "." << nsec << "\n";
    else
        std::cout << "[CALLBACK] Falling edge at " << sec << "." << nsec << "\n";
}

int main() {
    std::signal(SIGINT, sigintHandler);

    Button btn(BUTTON_PIN, PULL_MODE);

    if (!btn.begin()) {
        std::cerr << "Init error: " << btn.errorMessage << "\n";
        return 1;
    }

    // Option 1: Polling mode
    std::cout << "Polling button... Press Ctrl+C to exit.\n";
    while (running) {
        if (btn.state()) {
            std::cout << "Button pressed!\n";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    // Option 2: Interrupt mode (uncomment to test)
    /*
    if (!btn.beginInterrupt(myButtonHandler, true, 5000)) {
        std::cerr << "Interrupt error: " << btn.errorMessage << "\n";
        return 1;
    }
    std::cout << "Waiting for button events (Ctrl+C to quit)...\n";
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    */

    btn.clean();
    return 0;
}
