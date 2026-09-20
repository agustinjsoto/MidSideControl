#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GoniometerComponent.h"

class MidSideVolumeAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit MidSideVolumeAudioProcessorEditor (MidSideVolumeAudioProcessor&);
    ~MidSideVolumeAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // Referencia al processor para acceder al apvts.
    MidSideVolumeAudioProcessor& audioProcessor;

    // Los 2 faders (verticales) y el knob (rotativo) de volumen de salida.
    juce::Slider midSlider;
    juce::Slider sideSlider;
    juce::Slider outputKnob;

    juce::Label midLabel;
    juce::Label sideLabel;
    juce::Label outputLabel;

    // Goniómetro / vectorscopio: se ubica en el hueco que queda arriba del knob.
    GoniometerComponent goniometer;

    // Attachments: conectan cada Slider con su parámetro en el apvts,
    // manejando la sincronización de valores automáticamente.
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> midAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sideAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidSideVolumeAudioProcessorEditor)
};
