#pragma once
#include "ParamControl.h"

// Fader vertical de 64 px: pista negra, tapa clara de 30x40 con rayas de agarre,
// escala "tipo REAPER" con 9 tramos iguales y el último hasta -inf.
// El valor del parámetro está en dB; el mínimo del parámetro es "-inf".
class Fader : public ParamControl
{
public:
    // Marcas de la escala (9 tramos iguales, de arriba hacia abajo).
    struct Scale
    {
        std::array<float, 9> db;                 // dB en cada marca (la décima es -inf)
        std::array<const char*, 10> labels;      // textos de la escala
        int defaultTramo;                        // tramo donde cae 0 dB

        static Scale reaper();                   // +12 6 0 -5 -10 -20 -30 -40 -50 -inf   (catálogo)
        static Scale reaperPlus24();             // +24 12 6 0 -6 -12 -24 -40 -50 -inf    (para rangos hasta +24 dB)
    };

    Fader (juce::RangedAudioParameter& parameter, const juce::String& label, const Scale& scale = Scale::reaper());

    static constexpr int width = 64;
    static constexpr int height = 308;

    void paint (juce::Graphics&) override;
    void resized() override;
    void lookAndFeelChanged() override { staticLayer.invalidate(); repaint(); }

protected:
    double valueToPosition (float realValue) const override;
    float  positionToValue (double newPosition) const override;
    double dragPixelsForFullRange (bool fine) const override { return fine ? 832.0 : 208.0; }
    double wheelStep() const override { return 0.02; }
    double keyStep (bool shift) const override { return shift ? 0.05 : 0.01; }
    juce::Rectangle<int> getBubbleAnchor() const override;
    void onMouseDownBody (const juce::MouseEvent&) override;

private:
    float capY (double n) const;                  // y local del centro de la tapa
    void paintStaticLayer (juce::Graphics&) const;
    float minDb() const { return getParameter().convertFrom0to1 (0.0f); }

    Scale scale;
    mutable asaudio::CachedLayer staticLayer;

    static constexpr float valueRowH = 20.0f;
    static constexpr float svgTop = 26.0f;               // y local del origen del viewBox (-10)
};
