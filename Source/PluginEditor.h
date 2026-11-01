#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "ASaudio/ASaudioLookAndFeel.h"
#include "ASaudio/UiSettings.h"
#include "PluginPanel.h"

// Editor del plugin: solo hospeda el PluginPanel (tamaño lógico fijo) y lo
// escala según el ajuste "Window size" (75 / 100 / 125 / 150 %).
class MidSideVolumeAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit MidSideVolumeAudioProcessorEditor (MidSideVolumeAudioProcessor&);
    ~MidSideVolumeAudioProcessorEditor() override;

    void paint (juce::Graphics&) override {}      // todo lo dibuja el panel

private:
    void applyZoom (float zoom);

    MidSideVolumeAudioProcessor& audioProcessor;

    asaudio::ASaudioLookAndFeel lookAndFeel;
    UiSettings settings;
    PluginPanel panel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidSideVolumeAudioProcessorEditor)
};
