#pragma once
#include "ASaudioLookAndFeel.h"

// Botón redondo de 22 px. Con ícono de engranaje (gira 45° al pasar el mouse)
// o, en modo "close", con una cruz (cierre del SettingsPanel).
class GearButton : public juce::Button, private juce::Timer
{
public:
    enum class Glyph { gear, close };

    explicit GearButton (Glyph g = Glyph::gear);

    void paintButton (juce::Graphics&, bool isMouseOver, bool isButtonDown) override;

    static constexpr int size = 22;

private:
    void timerCallback() override;
    void mouseEnter (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override;

    Glyph glyph;
    float angle = 0.0f, target = 0.0f;    // grados
};
