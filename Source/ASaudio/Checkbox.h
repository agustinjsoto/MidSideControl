#pragma once
#include "ASaudioLookAndFeel.h"

// Pozo de 24 px con tilde ámbar.
class Checkbox : public juce::Button
{
public:
    explicit Checkbox (const juce::String& name);
    void paintButton (juce::Graphics&, bool isMouseOver, bool isButtonDown) override;
    static constexpr int size = 24;
};
