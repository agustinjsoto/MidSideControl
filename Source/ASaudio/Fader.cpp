#include "Fader.h"
using namespace asaudio;

Fader::Scale Fader::Scale::reaper()
{
    return { { 12, 6, 0, -5, -10, -20, -30, -40, -50 },
             { "12", "6", "0", "5", "10", "20", "30", "40", "50", "\xe2\x88\x9e" }, 2 };
}

Fader::Scale Fader::Scale::reaperPlus24()
{
    return { { 24, 12, 6, 0, -6, -12, -24, -40, -50 },
             { "24", "12", "6", "0", "6", "12", "24", "40", "50", "\xe2\x88\x9e" }, 3 };
}

Fader::Fader (juce::RangedAudioParameter& p, const juce::String& label, const Scale& s)
    : ParamControl (p, label), scale (s)
{
    setSize (width, height);
    entry.setTextStyle (13.0f, 0.02f);
    startAttachment();
}

void Fader::resized()
{
    entry.setBounds (0, 0, width, (int) valueRowH);
}

//==============================================================================
// Posición 1 = arriba, 0 = abajo (-inf). 9 tramos iguales; el último va de la
// marca de -50 dB hasta el mínimo del parámetro (-inf).
float Fader::positionToValue (double n) const
{
    const float lo = minDb();
    if (n < 0.004)
        return lo;

    const double p = (1.0 - n) * 9.0;
    const int i = juce::jmin (8, (int) std::floor (p));
    const double f = p - i;

    const float db = i < 8 ? (float) (scale.db[(size_t) i] + (scale.db[(size_t) i + 1] - scale.db[(size_t) i]) * f)
                           : (float) (scale.db[8] + (lo - scale.db[8]) * f);

    return juce::jlimit (lo, getParameter().convertFrom0to1 (1.0f), db);
}

double Fader::valueToPosition (float db) const
{
    const float lo = minDb();
    if (db <= lo + 0.01f)
        return 0.0;

    const float hi = scale.db[0];
    if (db >= hi)
        return 1.0;

    int i = 0;
    while (i < 8 && db < scale.db[(size_t) i + 1])
        ++i;

    const double p = i < 8 ? i + (scale.db[(size_t) i] - db) / (scale.db[(size_t) i] - scale.db[(size_t) i + 1])
                           : 8.0 + (scale.db[8] - db) / (scale.db[8] - lo);

    return juce::jlimit (0.0, 1.0, 1.0 - p / 9.0);
}

float Fader::capY (double n) const
{
    return svgTop + 10.0f + 222.8f - 208.0f * (float) n;
}

juce::Rectangle<int> Fader::getBubbleAnchor() const
{
    return { 2 + 5, (int) capY (getPosition()) - 20, 30, 40 };
}

void Fader::onMouseDownBody (const juce::MouseEvent& e)
{
    // clic en la pista salta hasta ahí (si no se agarró la tapa)
    const float y = e.position.y;
    if (std::abs (y - capY (getPosition())) > 20.0f)
    {
        const double n = (svgTop + 10.0f + 222.8f - y) / 208.0;
        setPositionByUser (n, true);
    }
}

//==============================================================================
void Fader::paintStaticLayer (juce::Graphics& g) const
{
    const auto& theme = themeOf (*this);

    g.addTransform (juce::AffineTransform::translation (2.0f, svgTop + 10.0f));   // origen del viewBox (y=0)

    // ranura
    g.setColour (juce::Colour (0xff232524));
    g.fillRoundedRectangle (3.0f, -7.0f, 34.0f, 252.0f, 3.0f);
    g.setColour (juce::Colours::white.withAlpha (0.06f));
    g.drawRoundedRectangle (3.5f, -6.5f, 33.0f, 251.0f, 2.5f, 1.0f);

    // pista
    g.setColour (juce::Colours::black);
    g.fillRect (18.5f, 6.0f, 3.0f, 224.0f);
    g.setColour (juce::Colours::white.withAlpha (0.07f));
    g.fillRect (21.5f, 6.0f, 1.0f, 224.0f);

    // escala
    g.setFont (fonts::label (11.0f, true, 0.0f));
    g.setColour (theme.inkDim);
    for (int i = 0; i < 10; ++i)
    {
        const float y = 222.8f - 208.0f * (1.0f - (float) i / 9.0f);
        g.drawText (juce::String::fromUTF8 (scale.labels[(size_t) i]),
                    juce::Rectangle<float> (42.0f, y - 7.0f, 18.0f, 14.0f), juce::Justification::centredLeft, false);
    }
}

void Fader::paint (juce::Graphics& g)
{
    const auto& theme = themeOf (*this);
    const float cy = capY (getPosition());
    const juce::Rectangle<float> cap (2.0f + 5.0f, cy - 20.0f, 30.0f, 40.0f);

    if (hasSelectionFocus())
    {
        juce::Path halo;
        halo.addRoundedRectangle (cap, 3.5f);
        draw::focusHalo (g, halo);
    }

    staticLayer.draw (g, width, height, [this] (juce::Graphics& ig) { paintStaticLayer (ig); });

    // tapa
    juce::Path capPath;
    capPath.addRoundedRectangle (cap, 3.5f);
    juce::DropShadow (juce::Colours::black.withAlpha (0.55f), 3, { 0, 3 }).drawForPath (g, capPath);

    juce::ColourGradient grad (juce::Colour (0xfffdfdfd), 0.0f, cap.getY(), juce::Colour (0xff8e8e8e), 0.0f, cap.getBottom(), false);
    grad.addColour (0.22, juce::Colour (0xffececec));
    grad.addColour (0.49, juce::Colour (0xffe2e2e2));
    grad.addColour (0.51, juce::Colour (0xfff3f3f3));
    grad.addColour (0.80, juce::Colour (0xffe4e4e4));
    grad.addColour (0.84, juce::Colour (0xffbcbcbc));
    g.setGradientFill (grad);
    g.fillRoundedRectangle (cap, 3.5f);
    g.setColour (juce::Colour (0xff1a1a1a));
    g.drawRoundedRectangle (cap, 3.5f, 0.8f);

    // rayas de agarre (oscura + clara 1 px abajo)
    for (float off : { -12.0f, -8.5f, -5.0f, 4.0f, 7.5f, 11.0f })
    {
        g.setColour (juce::Colour (0xff7d7d7d).withAlpha (0.55f));
        g.drawLine (cap.getX() + 1.0f, cy + off, cap.getRight() - 1.0f, cy + off, 1.0f);
        g.setColour (juce::Colours::white.withAlpha (0.85f));
        g.drawLine (cap.getX() + 1.0f, cy + off + 1.0f, cap.getRight() - 1.0f, cy + off + 1.0f, 1.0f);
    }

    g.setColour (juce::Colour (0xff222222).withAlpha (0.9f));
    g.fillRect (cap.getX() + 0.4f, cy - 0.8f, cap.getWidth() - 0.8f, 1.6f);
    g.setColour (juce::Colours::white.withAlpha (0.9f));
    g.fillRect (cap.getX() + 0.4f, cap.getY() + 0.4f, cap.getWidth() - 0.8f, 1.2f);

    // label
    draw::labelText (g, labelText, { 0.0f, svgTop + 256.0f + 5.0f, (float) width, 20.0f }, juce::Justification::centredTop,
                     fonts::label (16.0f), theme.ink);
}
