#pragma once
#include "ASaudioLookAndFeel.h"

// Tira de 20 LEDs (0,1 de correlación por LED) que se encienden desde el centro:
// ámbar hacia +1 (en fase), rojo hacia -1 (fase invertida). Escala debajo.
class LedCorrelation : public juce::Component
{
public:
    explicit LedCorrelation (const juce::String& label = "Correlation");

    void setValue (float correlation);        // -1 .. +1
    float getValue() const noexcept { return value; }

    static constexpr int width = 240;
    static constexpr int height = 72;         // tira 46 + label

    void paint (juce::Graphics&) override;
    void lookAndFeelChanged() override { staticLayer.invalidate(); repaint(); }

private:
    void paintStatic (juce::Graphics&) const;

    juce::String labelText;
    float value = 0.0f;
    int litCount = 0;                         // con signo
    mutable asaudio::CachedLayer staticLayer;
};
