#include "LedCorrelation.h"
using namespace asaudio;

LedCorrelation::LedCorrelation (const juce::String& label) : labelText (label)
{
    setSize (width, height);
    setInterceptsMouseClicks (false, false);
    setTitle (label);
}

void LedCorrelation::setValue (float c)
{
    value = juce::jlimit (-1.0f, 1.0f, c);
    const int n = (int) std::lround (std::abs (value) * 10.0f) * (value < 0.0f ? -1 : 1);

    if (n != litCount)
    {
        litCount = n;
        repaint();
    }
}

void LedCorrelation::paintStatic (juce::Graphics& g) const
{
    const auto& theme = themeOf (*this);

    g.setColour (juce::Colour (0xff0b0d0c));
    g.fillRoundedRectangle (2.0f, 2.0f, 236.0f, 26.0f, 5.0f);
    g.setColour (juce::Colours::white.withAlpha (0.14f));
    g.drawRoundedRectangle (2.5f, 2.5f, 235.0f, 25.0f, 5.0f, 1.0f);

    g.setFont (fonts::label (11.0f, true, 0.0f));
    for (float v : { -1.0f, -0.5f, 0.0f, 0.5f, 1.0f })
    {
        const float x = 119.35f + v * 112.35f;
        g.setColour (col::creamA.withAlpha (0.0f));
        g.setColour (theme.ink.withAlpha (0.6f));
        g.drawLine (x, 28.0f, x, 31.0f, 1.0f);

        g.setColour (theme.inkDim);
        const juce::String t = v > 0 ? "+" + juce::String (v, v == 1.0f ? 0 : 1) : juce::String (v, v == -1.0f ? 0 : (v == 0.0f ? 0 : 1));
        g.drawText (t, juce::Rectangle<float> (x - 16.0f, 33.0f, 32.0f, 12.0f), juce::Justification::centred, false);
    }
}

void LedCorrelation::paint (juce::Graphics& g)
{
    staticLayer.draw (g, width, 46, [this] (juce::Graphics& ig) { paintStatic (ig); });

    for (int k = 0; k < 20; ++k)
    {
        const bool redSide = k < 10;
        const bool on = litCount >= 0 ? (k >= 10 && k < 10 + litCount)
                                      : (k < 10 && k >= 10 + litCount);

        const juce::Rectangle<float> led (7.0f + (float) k * 11.3f, 7.0f, 10.0f, 16.0f);
        const auto colour = on ? (redSide ? col::ledRed : col::amber)
                               : (redSide ? col::ledRedOff : col::ledAmberOff);

        if (on)
        {
            juce::Path p;
            p.addRoundedRectangle (led, 0.8f);
            draw::glow (g, p, colour, 3, redSide ? 0.7f : 0.6f);
        }

        g.setColour (colour);
        g.fillRoundedRectangle (led, 0.8f);
    }

    draw::labelText (g, labelText, { 0.0f, 48.0f, (float) width, 20.0f }, juce::Justification::centredTop,
                     fonts::label (16.0f), themeOf (*this).ink);
}
