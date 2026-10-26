#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GoniometerComponent.h"

// Botoncito de ajustes: dibuja un engranaje (sin depender de ninguna fuente
// ni imagen externa) que se ilumina al pasar el mouse.
class SettingsButton : public juce::Button
{
public:
    SettingsButton() : juce::Button ("Settings")
    {
        setMouseCursor (juce::MouseCursor::PointingHandCursor);
    }

    void paintButton (juce::Graphics& g, bool isMouseOver, bool isButtonDown) override
    {
        const auto b = getLocalBounds().toFloat().reduced (3.0f);
        const auto centre = b.getCentre();
        const float outer = juce::jmin (b.getWidth(), b.getHeight()) * 0.5f;

        // Estrella de 8 puntas + círculo del centro con winding par = engranaje con agujero.
        juce::Path gear;
        gear.setUsingNonZeroWinding (false);
        gear.addStar (centre, 8, outer * 0.72f, outer, 0.0f);
        gear.addEllipse (centre.x - outer * 0.58f, centre.y - outer * 0.58f, outer * 1.16f, outer * 1.16f);
        gear.addEllipse (centre.x - outer * 0.28f, centre.y - outer * 0.28f, outer * 0.56f, outer * 0.56f);

        const auto base = juce::Colour (0xff8b92a3);
        g.setColour (isButtonDown ? juce::Colours::white
                     : isMouseOver ? base.brighter (0.6f)
                                   : base);
        g.fillPath (gear);
    }
};

class MidSideVolumeAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit MidSideVolumeAudioProcessorEditor (MidSideVolumeAudioProcessor&);
    ~MidSideVolumeAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // Abre el menú de ajustes (por ahora: Pan Law).
    void showSettingsMenu();

    // Referencia al processor para acceder al apvts.
    MidSideVolumeAudioProcessor& audioProcessor;

    // Los 2 faders (verticales) y los 2 knobs (rotativos): pan y volumen de salida.
    juce::Slider midSlider;
    juce::Slider sideSlider;
    juce::Slider outputKnob;
    juce::Slider panKnob;

    juce::Label midLabel;
    juce::Label sideLabel;
    juce::Label outputLabel;
    juce::Label panLabel;

    // Botón de ajustes (arriba a la derecha).
    SettingsButton settingsButton;

    // Goniómetro / vectorscopio: se ubica en el hueco que queda arriba del knob.
    GoniometerComponent goniometer;

    // Attachments: conectan cada Slider con su parámetro en el apvts,
    // manejando la sincronización de valores automáticamente.
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> midAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sideAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> panAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidSideVolumeAudioProcessorEditor)
};
