#include "PluginProcessor.h"
#include "PluginEditor.h"

MidSideVolumeAudioProcessorEditor::MidSideVolumeAudioProcessorEditor (MidSideVolumeAudioProcessor& p)
    : AudioProcessorEditor (&p),
      audioProcessor (p),
      settings (p.apvts.state),
      panel (p, settings, lookAndFeel, [this] (float z) { applyZoom (z); })
{
    setLookAndFeel (&lookAndFeel);
    lookAndFeel.setTheme (asaudio::Theme::fromName (settings.getTheme()));

    addAndMakeVisible (panel);
    setResizable (false, false);
    applyZoom (settings.getZoom());
}

MidSideVolumeAudioProcessorEditor::~MidSideVolumeAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void MidSideVolumeAudioProcessorEditor::applyZoom (float zoom)
{
    // El zoom se aplica al panel (no al editor) para no pisar el factor de
    // escala que pueda poner el host.
    panel.setBounds (0, 0, PluginPanel::baseWidth, PluginPanel::baseHeight);
    panel.setTransform (juce::AffineTransform::scale (zoom));

    setSize (juce::roundToInt ((float) PluginPanel::baseWidth * zoom),
             juce::roundToInt ((float) PluginPanel::baseHeight * zoom));
}
