#pragma once
#include "ParamControl.h"

// Perilla de 96 px, barrido de 270° (-135° a +135°), 11 marcas, tapa de latón.
// Celda de 144 px de ancho con valor editable y label debajo.
// Escala lineal/log/pasos: la define el NormalisableRange del parámetro.
class Knob : public ParamControl
{
public:
    Knob (juce::RangedAudioParameter& parameter, const juce::String& label, bool bipolar = false);

    static constexpr int cellWidth = 144;
    static constexpr int cellHeight = 160;     // 8 de aire para el halo + 96 + valor + label

    void paint (juce::Graphics&) override;
    void resized() override;
    void lookAndFeelChanged() override { staticLayer.invalidate(); repaint(); }

protected:
    double dragPixelsForFullRange (bool fine) const override { return fine ? 700.0 : 220.0; }
    double snapPosition (double p) const override;
    juce::Rectangle<int> getBubbleAnchor() const override { return bodyBounds.toNearestInt(); }

private:
    void paintStaticLayer (juce::Graphics&) const;

    bool isBipolar;
    juce::Rectangle<float> bodyBounds { 24.0f, 8.0f, 96.0f, 96.0f };
    mutable asaudio::CachedLayer staticLayer;
};
