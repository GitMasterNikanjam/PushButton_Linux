
// #######################################################################
// Inclide Libraries:

#include "Button.h"
#include <cstdlib>
#include <iostream>

// #######################################################################
// Button class:

Button::Button(const char* gpiodChip_path, unsigned int pin, uint8_t mode, uint8_t bias)
: _auxi(gpiodChip_path, pin, mode, bias)
{}

bool Button::begin()
{
    if (!_auxi.begin()) {
        errorMessage = "AUXI begin() failed: " + _auxi.errorMessage;
        return false;
    }
    return true;
}

bool Button::beginInterrupt(uint8_t edge, uint32_t debounce_us, GpioCallback cb)
{
    if (!cb) {
        errorMessage = "Button: callback is null.";
        return false;
    }

    // Ensure line is requested as input
    if (!_auxi.begin()) {
        errorMessage = "AUXI begin() failed: " + _auxi.errorMessage;
        return false;
    }

    AUXI::Edge sel;
    switch (edge) {
        case 0:  sel = AUXI::Edge::Both;    break;
        case 1:  sel = AUXI::Edge::Rising;  break;
        case 2:  sel = AUXI::Edge::Falling; break;
        default:
            errorMessage = "edge selection is not correct (must be 0,1,2).";
            return false;
    }

    if (!_auxi.beginInterrupt(sel, debounce_us, cb)) {
        errorMessage = "AUXI beginInterrupt() failed: " + _auxi.errorMessage;
        return false;
    }
    return true;
}

bool Button::beginInterrupt(AUXI::Edge edge, uint32_t debounce_us, GpioCallback cb)
{
    if (!cb) {
        errorMessage = "Button: callback is null.";
        return false;
    }
    if (!_auxi.begin()) {
        errorMessage = "AUXI begin() failed: " + _auxi.errorMessage;
        return false;
    }
    if (!_auxi.beginInterrupt(edge, debounce_us, cb)) {
        errorMessage = "AUXI beginInterrupt() failed: " + _auxi.errorMessage;
        return false;
    }
    return true;
}

void Button::stopInterrupt()
{
    _auxi.stopInterrupt();
}

void Button::clean()
{
    stopInterrupt();
    _auxi.clean();
}

int Button::value()
{
    return _auxi.value();
}

bool Button::read(void)
{
    return _auxi.read();
}

bool Button::get(void)
{
    return _auxi.get();
}

// ####################################################################
// ResetButton class:

bool ResetButton::check()
{
    if (!read()) return false;

    using namespace std::chrono_literals;

    std::cout << "System is Resetting ... !\n";
    std::this_thread::sleep_for(1s);
    std::cout << "3 Sec\n";
    std::this_thread::sleep_for(1s);
    std::cout << "2 Sec\n";
    std::this_thread::sleep_for(1s);
    std::cout << "1 Sec\n";
    std::this_thread::sleep_for(1s);

    if (read()) {
        std::cout << "System is Shutdown ... !\n";
        std::this_thread::sleep_for(1s);
        std::system("sudo /sbin/shutdown -h now");
        for (;;) std::this_thread::sleep_for(1s);
    }

    std::system("sudo /sbin/reboot");
    for (;;) std::this_thread::sleep_for(1s);

    // Unreachable
    return false;
}



