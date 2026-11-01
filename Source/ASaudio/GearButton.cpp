#include "GearButton.h"
using namespace asaudio;

GearButton::GearButton (Glyph gl) : juce::Button (gl == Glyph::gear ? "Settings" : "Close"), glyph (gl)
{
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
    setSize (size, size);
    setTitle (getName());
    setWantsKeyboardFocus (true);
}

void GearButton::mouseEnter (const juce::MouseEvent& e)
{
    juce::Button::mouseEnter (e);
    if (glyph == Glyph::gear) { target = 45.0f; startTimerHz (60); }
}

void GearButton::mouseExit (const juce::MouseEvent& e)
{
    juce::Button::mouseExit (e);
    if (glyph == Glyph::gear) { target = 0.0f; startTimerHz (60); }
}

void GearButton::timerCallback()
{
    // easing corto (~250 ms)
    angle += (target - angle) * 0.25f;
    if (std::abs (target - angle) < 0.2f) { angle = target; stopTimer(); }
    repaint();
}

void GearButton::paintButton (juce::Graphics& g, bool over, bool down)
{
    const auto r = getLocalBounds().toFloat().reduced (3.0f);
    const auto c = r.getCentre();
    const float rad = r.getWidth() * 0.5f;

    // anillos: negro, gris claro
    g.setColour (juce::Colours::black.withAlpha (0.4f));
    g.fillEllipse (r.expanded (3.0f).translated (0.0f, 2.0f));
    g.setColour (juce::Colour::fromFloatRGBA (1, 1, 1, 0.16f));
    g.fillEllipse (r.expanded (1.5f));
    g.setColour (juce::Colour (0xff0e0f0e));
    g.fillEllipse (r.expanded (1.0f));

    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff3a403d), r.getX() + rad * 0.7f, r.getY() + rad * 0.6f,
                                             juce::Colour (0xff1a1d1c), r.getRight(), r.getBottom(), true));
    g.fillEllipse (r);

    if (down)
    {
        g.setGradientFill (juce::ColourGradient (juce::Colours::black.withAlpha (0.7f), 0.0f, r.getY(),
                                                 juce::Colours::transparentBlack, 0.0f, r.getY() + 5.0f, false));
        g.fillEllipse (r);
    }

    const auto ink = (over || hasKeyboardFocus (false)) ? col::brassA : themeOf (*this).ink;
    g.setColour (ink);

    if (glyph == Glyph::gear)
    {
        // viewBox 24 -> 14 px
        const float s = 14.0f / 24.0f;
        auto xf = juce::AffineTransform::translation (-12.0f, -12.0f)
                      .scaled (s)
                      .rotated (juce::degreesToRadians (angle))
                      .translated (c.x, c.y);

        juce::Path ring;
        ring.addEllipse (12.0f - 6.2f, 12.0f - 6.2f, 12.4f, 12.4f);
        juce::Path stroked;
        juce::PathStrokeType (3.4f).createStrokedPath (stroked, ring);
        g.fillPath (stroked, xf);

        juce::Path tooth;
        tooth.addRoundedRectangle (10.3f, 2.0f, 3.4f, 5.2f, 1.0f);
        for (int i = 0; i < 8; ++i)
            g.fillPath (tooth, juce::AffineTransform::rotation (juce::degreesToRadians (45.0f * (float) i), 12.0f, 12.0f).followedBy (xf));
    }
    else
    {
        juce::Path x;
        const float d = 3.6f;
        x.startNewSubPath (c.x - d, c.y - d); x.lineTo (c.x + d, c.y + d);
        x.startNewSubPath (c.x + d, c.y - d); x.lineTo (c.x - d, c.y + d);
        g.strokePath (x, juce::PathStrokeType (1.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    if (hasKeyboardFocus (true))
    {
        g.setColour (col::amber);
        g.drawEllipse (getLocalBounds().toFloat().reduced (0.5f), 2.0f);
    }
}
