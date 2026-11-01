#include "GroupFrame.h"
using namespace asaudio;

GroupFrame::GroupFrame (const juce::String& plateText, Plate::Variant variant) : plate (plateText, variant)
{
    setInterceptsMouseClicks (false, false);
    addAndMakeVisible (plate);
}

void GroupFrame::setFrameBounds (juce::Rectangle<int> frame)
{
    setBounds (frame.withTrimmedTop (-plateOverhang));
}

void GroupFrame::resized()
{
    plate.setTopLeftPosition (16, 0);
}

void GroupFrame::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat().withTrimmedTop ((float) plateOverhang).reduced (1.0f).withTrimmedBottom (1.0f);

    // filo claro de 1 px abajo
    g.setColour (juce::Colours::white.withAlpha (0.07f));
    g.drawRoundedRectangle (r.translated (0.0f, 1.0f), 8.0f, 2.0f);

    g.setColour (themeOf (*this).frameBorder);
    g.drawRoundedRectangle (r, 8.0f, 2.0f);
}
