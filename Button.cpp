
// #######################################################################
// Inclide Libraries:

#include "Button.h"
#include <cstdlib>
#include <iostream>

// #######################################################################
// Helpers to map PUD to AUXI bias and polarity

static inline uint8_t pud_to_bias(uint8_t pud) {
    // PUD_OFF:0, PUD_DOWN:1, PUD_UP:2  →  AUXI bias: 0/1/2
    if (pud == 1) return 1;        // pulldown
    if (pud == 2) return 2;        // pullup
    return 0;                      // off
}

static inline uint8_t pud_to_mode(uint8_t pud) {
    // Choose polarity based on wiring convention:
    // - With pull-up, button to GND → active-low (pressed when raw=0)
    // - Otherwise, assume active-high.
    return (pud == 2) ? 0 : 1;     // 0=active-low, 1=active-high
}

// #######################################################################
// Button class:

Button::Button(const char* chipPath, unsigned pin, uint8_t pud)
: _chipPath(chipPath),
  _pin(pin),
  _pud(pud),
  _mode(pud_to_mode(pud)),
  _bias(pud_to_bias(pud)),
  _auxi(chipPath, pin, /*mode=*/_mode, /*bias=*/_bias)
{}

bool Button::begin()
{
    if (!_auxi.begin()) {
        errorMessage = "AUXI begin() failed: " + _auxi.errorMessage;
        return false;
    }
    _irqActive.store(false);
    return true;
}

bool Button::beginInterrupt(GpioCallback cb, bool both_edges, uint32_t debounce_us)
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

    const auto edge = both_edges ? AUXI::Edge::Both : AUXI::Edge::Rising;
    if (!_auxi.beginInterrupt(edge, debounce_us, cb)) {
        errorMessage = "AUXI beginInterrupt() failed: " + _auxi.errorMessage;
        return false;
    }
    _irqActive.store(true);
    return true;
}

void Button::stopInterrupt()
{
    if (_irqActive.exchange(false)) {
        _auxi.stopInterrupt();
    }
}

void Button::clean()
{
    stopInterrupt();
    _auxi.clean();
}

uint8_t Button::value()
{
    // AUXI::read() returns logical (after polarity). Treat true as 1.
    const bool v = _auxi.read();
    return static_cast<uint8_t>(v ? 1 : 0);
}

uint8_t Button::state()
{
    // "Pressed" is simply the logical active state chosen by _mode.
    return value();
}

// ####################################################################
// ResetButton class:

bool ResetButton::check()
{
    if (!state()) return false;

    using namespace std::chrono_literals;

    std::cout << "System is Resetting ... !\n";
    std::this_thread::sleep_for(1s);
    std::cout << "3 Sec\n";
    std::this_thread::sleep_for(1s);
    std::cout << "2 Sec\n";
    std::this_thread::sleep_for(1s);
    std::cout << "1 Sec\n";
    std::this_thread::sleep_for(1s);

    if (state()) {
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



