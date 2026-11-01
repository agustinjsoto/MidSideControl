#pragma once
#include "Knob.h"

// Knob con detente en el centro (zona de ~1,8 % que se pega al 0) y marca
// central en latón. Ejemplo: Pan (L / C / R).
class BipolarKnob : public Knob
{
public:
    BipolarKnob (juce::RangedAudioParameter& parameter, const juce::String& label)
        : Knob (parameter, label, true) {}
};
