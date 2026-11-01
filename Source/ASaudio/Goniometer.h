#pragma once
#include "ASaudioLookAndFeel.h"

// Osciloscopio estéreo rotado 45° (M arriba, S a los lados, L y R en las
// diagonales). Caja de 240 x 210 con cara crema de 222 x 192.
// Estilo INK: traza oscura sobre crema. Estilo PHOSPHOR: cara oscura, grilla y
// traza ámbar. El rastro persiste (cada cuadro se desvanece la imagen anterior).
//
// No lee el procesador: el editor le pasa las muestras por cuadro.
class Goniometer : public juce::Component
{
public:
    enum class Style { ink, phosphor };

    Goniometer();

    void setStyle (Style s);
    Style getStyle() const noexcept { return style; }

    // Radio (px) del círculo exterior; sirve para calcular el auto-gain.
    static constexpr float scopeRadius = 82.0f;

    // Una vez por cuadro: beginFrame(), addSamples() una o dos veces (buffer circular), endFrame().
    // La posición del punto es x = -side * gain, y = -mid * gain (px).
    void beginFrame();
    void addSamples (const float* mid, const float* side, int numSamples, float gain);
    void endFrame();

    void paint (juce::Graphics&) override;
    void lookAndFeelChanged() override { staticLayer.invalidate(); repaint(); }

    static constexpr int width = 240;
    static constexpr int height = 210;

private:
    void paintStatic (juce::Graphics&) const;

    Style style = Style::phosphor;
    juce::Image trace;                 // 2x de la cara, ARGB (por software), solo el rastro
    mutable asaudio::CachedLayer staticLayer;

    static constexpr float faceW = 222.0f, faceH = 192.0f, facePad = 9.0f;
    static constexpr float traceScale = 2.0f;
};
