#pragma once

// #################################################################################
// Include libraries:

#include "../AUXIO_Linux/AUXIO.h"      // uses AUXI from your latest AUXIO library
#include <cstdint>
#include <string>
#include <atomic>
#include <thread>
#include <chrono>

// #################################################################################

// Callback signature: (is_rising, seconds, nanoseconds)
using GpioCallback = void(*)(bool /*is_rising*/, long /*sec*/, long /*nsec*/);

// #################################################################################
// Button class:

/**
 * @brief Simple push-button wrapper on top of AUXI (GPIO input with optional interrupts).
 *
 * Pull modes (PUD) follow the legacy convention:
 * - 0 = OFF  (no bias)
 * - 1 = DOWN (pull-down)
 * - 2 = UP   (pull-up)  ← default and most common for buttons to ground
 *
 * Polarity is auto-derived from pull mode:
 * - PUD_UP   → active-low (pressed when raw=0)
 * - PUD_OFF/DOWN → active-high (pressed when raw=1)
 *
 * With this mapping, @ref state() returns 1 when the button is pressed.
 */
class Button
{
    public:

        std::string errorMessage;
        
        /**
         * @param chipPath  GPIO chip device path (e.g. "/dev/gpiochip0")
         * @param pin       GPIO line offset
         * @param pud       0=OFF, 1=DOWN, 2=UP (default=2)
         */
        Button(const char* chipPath, unsigned pin, uint8_t pud = 2);

        /** Configure the GPIO as input with proper bias (no interrupts). */
        bool begin(void);

        /**
         * Start edge-driven interrupts.
         * @param cb            C-style callback (cannot be null)
         * @param both_edges    true: rising+falling, false: rising-only
         * @param debounce_us   software debounce window in microseconds
         */
        bool beginInterrupt(GpioCallback cb, bool both_edges = true, uint32_t debounce_us = 5000);

        /** Stop interrupts (if running). */
        void stopInterrupt();

        /** Release resources (line/chip). Safe to call multiple times. */
        void clean(void);

        /**
         * Read current logical value (after polarity):
         * returns 1 when *active* (i.e., "pressed") based on chosen polarity.
         */
        uint8_t value(void);

        /**
         * Alias for "pressed" state. Returns 1 when pressed, 0 otherwise.
         * Equivalent to @ref value().
         */
        uint8_t state(void);

    protected:

        const char* _chipPath;
        unsigned    _pin;
        uint8_t     _pud;

        // Derived config for AUXI
        uint8_t     _mode;   // 0=active-low, 1=active-high (derived from _pud)
        uint8_t     _bias;   // 0=off, 1=pulldown, 2=pullup (direct from _pud)

        AUXI        _auxi;
        std::atomic<bool> _irqActive{false};
};

// ################################################################################
// ResetButton class:

/**
 * @brief Button that triggers reboot, and if held through a countdown, shutdown.
 *
 * Behavior:
 * - If pressed → start 3..2..1 countdown.
 * - If still pressed at the end → shutdown.
 * - Otherwise → reboot.
 *
 * (Adjust to your policy as needed.)
 */
class ResetButton : public Button
{
    public:

        using Button::Button; // inherit constructors

        /**
        * @brief If pressed, perform reboot; if still pressed after countdown, shutdown.
        * Blocks while counting down.
        */
        bool check(void);

};





