#pragma once
#include "ASaudioLookAndFeel.h"
#include "Plate.h"

// Marco redondeado (sin relleno) que agrupa controles, con su Plate.
// El componente incluye los 12 px que la placa sobresale por arriba.
// No captura el mouse.
class GroupFrame : public juce::Component
{
public:
    GroupFrame (const juce::String& plateText, Plate::Variant variant = Plate::Variant::brass);

    // Rectángulo del marco (sin la placa), en coordenadas del padre.
    void setFrameBounds (juce::Rectangle<int> frame);

    static constexpr int plateOverhang = 12;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    Plate plate;
};
