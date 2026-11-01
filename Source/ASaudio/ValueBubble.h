#pragma once
#include "ASaudioLookAndFeel.h"

// Globito con el valor que aparece sobre el control activo (arrastre: mientras
// se mantiene; rueda/flechas: 0,9 s). Un único componente compartido por el
// editor; debe vivir en el mismo padre que los controles.
class ValueBubble : public juce::Component, private juce::Timer
{
public:
    ValueBubble();

    void setEnabledByUser (bool shouldShow);          // ajuste "Value bubbles"
    bool isEnabledByUser() const noexcept { return enabledByUser; }

    void show  (juce::Component& target, juce::Rectangle<int> anchorInTarget, const juce::String& text);
    void flash (juce::Component& target, juce::Rectangle<int> anchorInTarget, const juce::String& text);
    void hideBubble();

    void paint (juce::Graphics&) override;

private:
    void timerCallback() override { hideBubble(); }
    void place (juce::Component& target, juce::Rectangle<int> anchorInTarget, const juce::String& text);

    juce::String text;
    bool enabledByUser = true;
    static constexpr int arrow = 5;
    int arrowX = 0;
};
