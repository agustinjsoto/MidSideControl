#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

// Vectorscopio / goniómetro: dibuja Mid en el eje vertical y Side en el
// horizontal, rotado como los goniómetros clásicos de estudio (L y R quedan
// en las diagonales). Se refresca solo con un Timer, leyendo el buffer
// circular que va llenando el processor en processBlock().
//
// Tiene un botoncito de 3 puntitos arriba a la derecha que abre un menú para
// elegir cuánta "persistencia" (rastro / promedio visual) tiene el trazo.
class GoniometerComponent : public juce::Component,
                             private juce::Timer
{
public:
    explicit GoniometerComponent (MidSideVolumeAudioProcessor& processorToUse);
    ~GoniometerComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    void drawBackground (juce::Graphics& g, juce::Rectangle<float> bounds) const;
    void drawTrace(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawCorrelationMeter(juce::Graphics& g, juce::Rectangle<float> area) const;
    void fadePersistence();

    MidSideVolumeAudioProcessor& processor;

    // Imagen donde acumulamos el rastro de puntos; cada frame le bajamos el
    // alpha (fade) y dibujamos encima solo las muestras nuevas. Así el trazo
    // "persiste" en vez de parpadear de golpe con los 2048 puntos.
    juce::Image persistenceImage;

    // Hasta qué muestra del buffer circular del processor ya dibujamos.
    int lastReadIndex = 0;

    // --- Auto-gain visual ---
    // Seguimos el "pico reciente" de la señal (como un medidor de VU) y con
    // eso recalculamos el zoom del dibujo cada frame, así el patrón ocupa un
    // tamaño parecido tanto en pasajes flojos como fuertes. Sube rápido
    // cuando entra señal más fuerte y baja de a poco (efecto "release").
    float runningPeakLevel = 0.1f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GoniometerComponent)
};
