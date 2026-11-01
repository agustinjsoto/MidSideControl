#include "Plate.h"
using namespace asaudio;

Plate::Plate (const juce::String& t, Variant v) : text (t), variant (v)
{
    setInterceptsMouseClicks (false, false);
    setSize (getRequiredWidth(), height);
}

void Plate::setText (const juce::String& newText)
{
    text = newText;
    setSize (getRequiredWidth(), height);
    repaint();
}

int Plate::getRequiredWidth() const
{
    return (int) std::ceil (draw::textWidth (fonts::label (14.0f), text)) + 36;   // padding 0 18 px
}

void Plate::paint (juce::Graphics& g)
{
    const auto r = getLocalBounds().toFloat().withTrimmedBottom (1.0f);
    const bool brass = variant == Variant::brass;

    // sombra bajo la placa
    g.setColour (juce::Colours::black.withAlpha (0.45f));
    g.fillRoundedRectangle (r.translated (0.0f, 1.0f), 3.0f);

    if (brass)
    {
        g.setGradientFill (juce::ColourGradient (col::brassA, 0.0f, r.getY(), col::brassB, 0.0f, r.getBottom(), false));
        g.fillRoundedRectangle (r, 3.0f);
        g.setColour (juce::Colours::white.withAlpha (0.35f));
        g.drawLine (r.getX() + 2.0f, r.getY() + 0.5f, r.getRight() - 2.0f, r.getY() + 0.5f, 1.0f);
    }
    else
    {
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff1b1d1c), 0.0f, r.getY(),
                                                 juce::Colour (0xff0f1110), 0.0f, r.getBottom(), false));
        g.fillRoundedRectangle (r, 3.0f);
        g.setGradientFill (juce::ColourGradient (juce::Colours::black.withAlpha (0.8f), 0.0f, r.getY(),
                                                 juce::Colours::transparentBlack, 0.0f, r.getY() + 3.0f, false));
        g.fillRoundedRectangle (r, 3.0f);
    }

    // remaches
    for (float cx : { 8.0f, r.getWidth() - 8.0f })
    {
        const juce::Rectangle<float> rv (cx - 2.0f, 8.0f, 4.0f, 4.0f);
        g.setColour (juce::Colours::white.withAlpha (brass ? 0.3f : 0.0f));
        g.fillEllipse (rv.translated (0.0f, 1.0f));
        g.setColour (brass ? juce::Colour::fromFloatRGBA (0.16f, 0.10f, 0.02f, 0.7f) : juce::Colour (0xff3b3f3d));
        g.fillEllipse (rv);
    }

    g.setFont (fonts::label (14.0f));
    g.setColour (brass ? juce::Colour (0xff2a1d08) : themeOf (*this).inkDim);
    g.drawText (text, r.withTrimmedTop (0.0f), juce::Justification::centred, false);
}
