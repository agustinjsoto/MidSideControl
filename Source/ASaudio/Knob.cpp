#include "Knob.h"
using namespace asaudio;

namespace
{
    constexpr float kSweep = 135.0f;           // ±135°
    constexpr float kHaloStrength = 0.5f;      // halo de selección (1 = como el fader)

    // coordenadas del viewBox 100x100 del catálogo
    juce::Point<float> pt (float r, float angleDeg)
    {
        const float a = juce::degreesToRadians (angleDeg);
        return { 50.0f + r * std::sin (a), 50.0f - r * std::cos (a) };
    }

    juce::ColourGradient radial (juce::Rectangle<float> box, float cx, float cy, float r,
                                 juce::Colour c0, float s1, juce::Colour c1, juce::Colour c2)
    {
        const juce::Point<float> centre (box.getX() + box.getWidth() * cx, box.getY() + box.getHeight() * cy);
        juce::ColourGradient g (c0, centre, c2, centre.translated (box.getWidth() * r, 0.0f), true);
        g.addColour (s1, c1);
        return g;
    }
}

Knob::Knob (juce::RangedAudioParameter& p, const juce::String& label, bool bipolar)
    : ParamControl (p, label), isBipolar (bipolar)
{
    setSize (cellWidth, cellHeight);
    startAttachment();
}

double Knob::snapPosition (double p) const
{
    // BipolarKnob: detente en el centro (zona de ~1,8 % que se pega al 0)
    if (isBipolar && std::abs (p - 0.5) < 0.018)
        return 0.5;
    return p;
}

void Knob::resized()
{
    const int w = juce::jmax (74, (int) std::ceil (draw::textWidth (fonts::label (15.0f, false, 0.06f), "R 100 dB")) + 12);
    entry.setBounds ((cellWidth - w) / 2, (int) bodyBounds.getBottom() + 3, w, 22);
}

void Knob::paintStaticLayer (juce::Graphics& g) const
{
    const auto& theme = themeOf (*this);
    const float s = bodyBounds.getWidth() / 100.0f;

    g.addTransform (juce::AffineTransform::scale (s).translated (bodyBounds.getX(), bodyBounds.getY()));

    // marcas (11): extremos y centro (bipolar) más largas
    const int count = 11;
    for (int i = 0; i < count; ++i)
    {
        const float a = -kSweep + 270.0f * (float) i / (float) (count - 1);
        const bool centre = isBipolar && i == 5;
        const bool big = i == 0 || i == count - 1 || centre;
        const auto p1 = pt (big ? 38.0f : 39.0f, a), p2 = pt (45.0f, a);

        g.setColour (centre ? col::brassA : theme.ink.withAlpha (0.75f));
        g.drawLine (p1.x, p1.y, p2.x, p2.y, big ? 1.6f : 1.0f);
    }

    // sombra y cuerpo
    juce::Path body;
    body.addEllipse (19.0f, 19.0f, 62.0f, 62.0f);
    juce::DropShadow (juce::Colours::black.withAlpha (0.5f), 3, { 0, 4 }).drawForPath (g, body);

    const juce::Rectangle<float> bb (19.0f, 19.0f, 62.0f, 62.0f);
    g.setGradientFill (radial (bb, 0.35f, 0.28f, 0.85f, juce::Colour (0xff4a4a48), 0.6f, juce::Colour (0xff1b1b1a), juce::Colour (0xff090909)));
    g.fillPath (body);

    // anillo punteado
    juce::Path ring;
    ring.addEllipse (20.0f, 20.0f, 60.0f, 60.0f);
    juce::Path dashed;
    const float dashes[] = { 1.0f, 1.9f };
    juce::PathStrokeType (2.4f).createDashedStroke (dashed, ring, dashes, 2);
    g.setColour (juce::Colours::black.withAlpha (0.55f));
    g.fillPath (dashed);

    g.setColour (juce::Colours::white.withAlpha (0.12f));
    g.drawEllipse (bb, 1.0f);
}

void Knob::paint (juce::Graphics& g)
{
    const auto& theme = themeOf (*this);
    const float s = bodyBounds.getWidth() / 100.0f;

    // halo de selección (foco real)
    if (hasSelectionFocus())
    {
        juce::Path halo;
        halo.addEllipse (bodyBounds.getCentreX() - 31.0f * s, bodyBounds.getCentreY() - 31.0f * s, 62.0f * s, 62.0f * s);
        draw::focusHalo (g, halo, kHaloStrength);
    }

    staticLayer.draw (g, cellWidth, cellHeight, [this] (juce::Graphics& ig) { paintStaticLayer (ig); });

    // tapa que gira
    const float angle = juce::degreesToRadians (-kSweep + 270.0f * (float) getPosition());
    const auto xf = juce::AffineTransform::rotation (angle, 50.0f, 50.0f)
                        .scaled (s).translated (bodyBounds.getX(), bodyBounds.getY());

    juce::Path tri;
    tri.addTriangle (46.5f, 29.0f, 50.0f, 19.0f, 53.5f, 29.0f);
    g.setColour (juce::Colour (0xffefe6c8));
    g.fillPath (tri, xf);
    g.setColour (juce::Colours::black.withAlpha (0.4f));
    g.strokePath (tri, juce::PathStrokeType (0.6f), xf);

    juce::Path cap;
    cap.addEllipse (31.0f, 31.0f, 38.0f, 38.0f);
    {
        const juce::Rectangle<float> cb (31.0f, 31.0f, 38.0f, 38.0f);
        auto grad = radial (cb, 0.35f, 0.30f, 0.85f, juce::Colour (0xfff1dc9f), 0.55f, juce::Colour (0xffbf9a4b), juce::Colour (0xff6e5120));
        juce::FillType ft (grad);
        ft.transform = xf;
        g.setFillType (ft);
    }
    g.fillPath (cap, xf);
    g.setColour (juce::Colour (0xff4d3912));
    g.strokePath (cap, juce::PathStrokeType (0.8f), xf);

    juce::Path r13, r7;
    r13.addEllipse (37.0f, 37.0f, 26.0f, 26.0f);
    r7.addEllipse (43.0f, 43.0f, 14.0f, 14.0f);
    g.setColour (juce::Colours::black.withAlpha (0.18f));
    g.strokePath (r13, juce::PathStrokeType (1.0f), xf);
    g.setColour (juce::Colours::white.withAlpha (0.25f));
    g.strokePath (r7, juce::PathStrokeType (1.0f), xf);

    // label
    const int labelY = entry.getBottom() + 5;
    draw::labelText (g, labelText, { 0.0f, (float) labelY, (float) cellWidth, 20.0f }, juce::Justification::centredTop,
                     fonts::label (14.5f, true, 0.05f), theme.ink);
}
