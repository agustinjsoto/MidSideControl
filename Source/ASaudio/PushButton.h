#pragma once
#include "ASaudioLookAndFeel.h"

// Pozo oscuro de 92 px con tecla de 30 px. Tipos: latch (queda pulsado),
// momentary (solo mientras se mantiene), y disabled (setEnabled(false), 45 %).
class PushButton : public juce::Button
{
public:
    enum class Kind { latch, momentary };

    PushButton (const juce::String& caption, Kind kind = Kind::momentary);
    void paintButton (juce::Graphics&, bool isMouseOver, bool isButtonDown) override;

    static constexpr int width = 92;
    static constexpr int height = 36;

private:
    Kind kind;
};
