/**
 * @file Button.h
 * @brief High-level push-button wrapper classes built on top of AUXIO (AUXI).
 *
 * Provides a simple abstraction for reading buttons using the AUXI
 * GPIO input wrapper (libgpiod v1.x). Supports:
 * - Pull-up / pull-down / no-bias modes (legacy PUD convention)
 * - Active-high or active-low logic (auto derived from pull mode)
 * - Polling reads (`value()`, `state()`)
 * - Event-driven interrupts with software debounce and callback
 *
 * Includes a specialized @ref ResetButton that triggers reboot or
 * shutdown depending on hold duration.
 */

// ################################################################################

#pragma once

// #################################################################################
// Include libraries:

#include "../AUXIO_Linux/AUXIO.h"      // uses AUXI from your latest AUXIO library
#include <cstdint>
#include <string>
#include <thread>
#include <chrono>

// #################################################################################

/**
 * @brief GPIO edge event callback signature.
 *
 * Invoked by @ref beginInterrupt() when an edge event occurs.
 *
 * @param is_rising True for rising edge, false for falling edge.
 * @param sec       Kernel timestamp (seconds).
 * @param nsec      Kernel timestamp (nanoseconds).
 */
using GpioCallback = void(*)(bool /*is_rising*/, long /*sec*/, long /*nsec*/);

// #################################################################################
// Button class:

/**
 * @class Button
 * @brief Wrapper for a push-button input line, built on AUXI.
 *
 * Pull modes (PUD) follow the legacy convention:
 * - 0 = OFF   (no bias)
 * - 1 = DOWN  (pull-down bias)
 * - 2 = UP    (pull-up bias, default)
 *
 * Logic polarity is derived from the pull mode:
 * - PUD_UP   → active-low (pressed = raw=0)
 * - PUD_OFF / PUD_DOWN → active-high (pressed = raw=1)
 *
 * This ensures @ref state() consistently returns 1 when pressed.
 */
class Button
{
    public:

        /**
         * @brief Stores last error message (set if an operation fails).
         */
        std::string errorMessage;
        
        /**
         * @brief Construct a button object.
         *
         * @param gpiodChip_path GPIO chip device path (e.g. "/dev/gpiochip0").
         * @param pin       GPIO line offset.
         * @param pud      Pull mode: 0=OFF, 1=DOWN, 2=UP (default=0).
         */
        Button(const char* gpiodChip_path, unsigned int pin, uint8_t mode = 1, uint8_t pud = 0);

        /**
         * @brief Configure the GPIO as input with the requested bias.
         * @return true on success, false on error (check @ref errorMessage).
         */
        bool begin(void);

        /**
         * @brief Start edge-driven interrupts.
         *
         * Requests events from libgpiod and launches an internal thread to
         * dispatch events to the user callback.
         *
         * @param cb           Callback function pointer (cannot be null).
         * @param edge         0:both edge, 1: rising, 2: falling
         * @param debounce_us  Debounce filter in microseconds (default 5000).
         * @return true on success, false on error (check @ref errorMessage).
         */
        bool beginInterrupt(uint8_t edge = 0, uint32_t debounce_us = 5000, GpioCallback cb = nullptr);

        /**
         * @brief Stop the interrupt thread (if active).
         *
         * Safe to call multiple times.
         */
        void stopInterrupt();

        /**
         * @brief Release resources (line and chip).
         *
         * Calls @ref stopInterrupt() first, then AUXI::clean().
         * Safe to call multiple times.
         */
        void clean(void);

        // Read digital input level.
        int value(void);

        /**
         * @brief Alias for "pressed" state.
         *  
         * Equivalent to @ref value(). Returns 1 if pressed, 0 otherwise.
         */
        bool read(void);

        // get last state of button. true if pressed, false if not pressed.
        bool get(void);

    protected:

        AUXI        _auxi;          ///< Underlying AUXI instance
};

// ################################################################################
// ResetButton class:

/**
 * @class ResetButton
 * @brief Special button with reboot/shutdown semantics.
 *
 * Behavior:
 * - If pressed → begins countdown.
 * - If still pressed after countdown → shutdown system.
 * - If released before countdown ends → reboot system.
 */
class ResetButton : public Button
{
    public:

        using Button::Button; // inherit constructors

        /**
         * @brief Execute reboot/shutdown behavior.
         *
         * If pressed, prints a countdown (3..2..1). If still pressed
         * after countdown, performs shutdown; otherwise, performs reboot.
         *
         * @return true if button was initially pressed, false otherwise.
         * @note This call blocks while counting down and executing system calls.
         */
        bool check(void);

};





