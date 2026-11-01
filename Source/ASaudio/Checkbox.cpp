#include "Checkbox.h"
using namespace asaudio;

Checkbox::Checkbox (const juce::String& name) : juce::Button (name)
{
    setClickingTogglesState (true);
    setSize (size, size);
    setTitle (name);
    setWantsKeyboardFocus (true);
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
}

void Checkbox::paintButton (juce::Graphics& g, bool, bool)
{
    draw::well (g, getLocalBounds().toFloat().withTrimmedBottom (1.0f), 5.0f);

    if (getToggleState())
    {
        juce::Path tick;
        tick.startNewSubPath (6.5f, 12.0f);
        tick.lineTo (10.5f, 16.0f);
        tick.lineTo (17.5f, 6.5f);

        juce::Path stroked;
        juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded).createStrokedPath (stroked, tick);

        draw::glow (g, stroked, col::amber, 4, 0.8f);
        g.setColour (col::amber);
        g.fillPath (stroked);
    }

    if (hasKeyboardFocus (true))
    {
        g.setColour (col::amber);
        g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (1.0f), 5.0f, 2.0f);
    }
}
