#ifndef OVERDRIVE_HANDLER_H
#define OVERDRIVE_HANDLER_H

#define BUTTON_IN_1 18
#define BUTTON_IN_2 19
#define BUTTON_IN_3 21

#include "animations.h"

class OverdriveHandler
{
    public:
        Animations *animations;
        bool isOverdriveReactive = true;
        bool isSlave = false;

        bool isOverdrive_1;
        bool isOverdrive_2;
        bool isOverdrive_3;

        bool lastOverdriveButtonState_1 = true;
        bool lastOverdriveButtonState_2 = true;
        bool lastOverdriveButtonState_3 = true;

        OverdriveHandler(Animations *animations)
        {
            this->animations = animations;
        }

        void setup()
        {
            pinMode(BUTTON_IN_1, INPUT_PULLUP);
            pinMode(BUTTON_IN_2, INPUT_PULLUP);
            pinMode(BUTTON_IN_3, INPUT_PULLUP);
        }

        void update()
        {
            // Serial.println(digitalRead(BUTTON_IN_1));

            readOverrideButton(BUTTON_IN_1, lastOverdriveButtonState_1, 0);
            readOverrideButton(BUTTON_IN_2, lastOverdriveButtonState_2, 1);
            readOverrideButton(BUTTON_IN_3, lastOverdriveButtonState_3, 2);
        }
                
        void readOverrideButton(uint8_t buttonPin, bool &lastOverdriveButtonState, byte index)
        {
            if (!isOverdriveReactive
                || isSlave) return;

            int buttonState = digitalRead(buttonPin);
            if (buttonState == LOW
                && buttonState != lastOverdriveButtonState)
            {
                startOverdrive(index);
            }

            lastOverdriveButtonState = buttonState;
        }

        void startOverdrive(byte index)
        {
            Serial.print("Overdrive button: ");
            Serial.println(index);

            animations->startOverdriveRandom(index);
        }
};

#endif