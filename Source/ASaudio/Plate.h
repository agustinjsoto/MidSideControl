#pragma once
#include "ASaudioLookAndFeel.h"

// Placa grabada (latón o oscura) que cuelga sobre el borde de un GroupFrame.
class Plate : public juce::Component
{
public:
    enum class Variant { brass, dark };

    explicit Plate (const juce::String& text = {}, Variant v = Variant::brass);

    void setText (const juce::String& newText);
    Variant getVariant() const noexcept { return variant; }

    static constexpr int height = 20;
    int getRequiredWidth() const;

    void paint (juce::Graphics&) override;

private:
    juce::String text;
    Variant variant;
};
