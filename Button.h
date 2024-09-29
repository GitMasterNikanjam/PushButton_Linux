#ifndef _BUTTON_H
#define _BUTTON_H

    #include <iostream>
    #include <bcm2835.h>
    #include <pigpio.h>                                 // For GPIO configuration            
    #include <chrono>                                   // For time management
    #include <thread>
    #include <mutex>

    using namespace std;

    class Button
    {
        public:

            std::string errorMessage;
            
            /*
                BUTTUN object constructor. Set button pin and its input mode. Not apply setting.
                Hint: begin() method needs after this for apply setting on hardware.
                pud: Pullup/Pulldown. PUD_OFF:0, PUD_DOWN:1, PUD_UP:2 
            */
            Button(uint8_t pin, uint8_t pud);

            /*
                Apply setting on hardware. Start BUTTON action.
                @return true if successed.
            */
            bool begin(void);

            /*
                Clean setting on hardware. Stop  BUTTON action. 
            */
            void clean(void);

            /*
                Return Button digital input value.
            */
            uint8_t value(void);

            /*
                Return Button pressed state. pressed:1, not pressed:0
            */
            uint8_t state(void);

        protected:

            // GPIO pin number
            uint8_t _pin;
            
            // Pullup/Pulldown mode. PUD_OFF:0, PUD_DOWN:1, PUD_UP:2 
            uint8_t _pud;

            // Duration time that user pressed button. [us]
            uint64_t _pressedDur;

            uint64_t _T;

            /*
                external interrupts handle function.
                When Button trigged, this function execute.
            */ 
            void _InterruptHandler(int /*gpio*/, int /*level*/, uint32_t /*tick*/); 
    };

    class ResetButton : public Button
    {
        public:
            /*
            BUTTUN object constructor. Set button pin and its input mode. Not apply setting.
            Hint: begin() method needs after this for apply setting on hardware.
            pud: Pullup/Pulldown. PUD_OFF:0, PUD_DOWN:1, PUD_UP:2 
            */
            ResetButton(uint8_t pin, uint8_t pud) : Button(pin, pud) {};

            // Check reset button for shutdown or reboot system.
            bool check(void);

    };

#endif