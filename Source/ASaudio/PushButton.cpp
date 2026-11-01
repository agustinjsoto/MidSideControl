#include "PushButton.h"
using namespace asaudio;

PushButton::PushButton (const juce::String& caption, Kind k) : juce::Button (caption), kind (k)
{
    setButtonText (caption);
    setClickingTogglesState (k == Kind::latch);
    setSize (width, height);
    setTitle (caption);
    setWantsKeyboardFocus (true);
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
}

void PushButton::paintButton (juce::Graphics& g, bool over, bool down)
{
    const float alpha = isEnabled() ? 1.0f : 0.45f;
    g.setOpacity (alpha);

    draw::well (g, getLocalBounds().toFloat().withTrimmedBottom (1.0f), 6.0f);

    const auto key = getLocalBounds().toFloat().withTrimmedBottom (1.0f).reduced (3.0f);
    const bool lit = (kind == Kind::latch ? getToggleState() : false) || (down && isEnabled());

    {
        juce::Graphics::ScopedSaveState ss (g);
        g.beginTransparencyLayer (alpha);
        draw::keyCap (g, key, 4.0f, lit, (over && isEnabled() && ! lit) ? 0.15f : 0.0f);

        const auto font = fonts::label (13.0f, true, 0.06f);
        const auto caption = getButtonText().toUpperCase();

        if (lit) draw::glowText (g, caption, key, juce::Justification::centred, font, col::amber);
        else     { g.setFont (font); g.setColour (themeOf (*this).inkDim); g.drawText (caption, key, juce::Justification::centred, false); }
        g.endTransparencyLayer();
    }

    if (hasKeyboardFocus (true))
    {
        g.setColour (col::amber);
        g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (1.0f), 6.0f, 2.0f);
    }
}
