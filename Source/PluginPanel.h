#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "ASaudio/ASaudioLookAndFeel.h"
#include "ASaudio/UiSettings.h"
#include "ASaudio/GroupFrame.h"
#include "ASaudio/Fader.h"
#include "ASaudio/Knob.h"
#include "ASaudio/BipolarKnob.h"
#include "ASaudio/Goniometer.h"
#include "ASaudio/LedCorrelation.h"
#include "ASaudio/GearButton.h"
#include "ASaudio/ValueBubble.h"
#include "ASaudio/SettingsPanel.h"

// Panel completo del plugin (tamaño lógico fijo 560 x 664). El editor lo
// escala con una transformación según "Window size".
class PluginPanel : public juce::Component, private juce::Timer
{
public:
    static constexpr int baseWidth = 560;
    static constexpr int baseHeight = 664;

    PluginPanel (MidSideVolumeAudioProcessor& processor,
                 UiSettings& settings,
                 asaudio::ASaudioLookAndFeel& lookAndFeel,
                 std::function<void (float)> zoomChanged);
    ~PluginPanel() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void lookAndFeelChanged() override { staticLayer.invalidate(); repaint(); }

private:
    void timerCallback() override;
    void paintStatic (juce::Graphics&) const;
    void applyTheme (const juce::String& name);

    MidSideVolumeAudioProcessor& processor;
    UiSettings& settings;
    asaudio::ASaudioLookAndFeel& laf;
    std::function<void (float)> onZoomChanged;

    mutable asaudio::CachedLayer staticLayer;

    GroupFrame frameMidSide  { "Mid / Side" };
    GroupFrame frameScope    { "Stereo scope" };
    GroupFrame frameOutput   { juce::String::fromUTF8 ("Pan \xc2\xb7 Mono Maker \xc2\xb7 Output") };

    Fader midFader;
    Fader sideFader;
    Goniometer gonio;
    LedCorrelation correlation;
    BipolarKnob panKnob;
    Knob monoKnob;
    Knob outputKnob;

    GearButton gearButton;
    ValueBubble bubble;
    SettingsPanel settingsPanel;

    int lastReadIndex = 0;
};
