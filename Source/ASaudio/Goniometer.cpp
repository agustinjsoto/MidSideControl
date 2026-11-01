#include "Goniometer.h"
using namespace asaudio;

Goniometer::Goniometer()
{
    setSize (width, height);
    setInterceptsMouseClicks (false, false);
    setOpaque (false);

    // Imagen por software: la leemos y escribimos pixel a pixel cada cuadro, y
    // en Windows (Direct2D) una imagen "nativa" vive en la GPU y esa lectura/
    // escritura es lenta o directamente falla.
    trace = juce::Image (juce::Image::ARGB, (int) (faceW * traceScale), (int) (faceH * traceScale), true,
                         juce::SoftwareImageType());
}

void Goniometer::setStyle (Style s)
{
    if (style == s)
        return;

    style = s;
    trace.clear (trace.getBounds());
    staticLayer.invalidate();
    repaint();
}

//==============================================================================
void Goniometer::beginFrame()
{
    // Desvanecimiento: baja el alpha de todo el rastro y borra lo que ya no se ve.
    trace.multiplyAllAlphas (0.95f);

    juce::Image::BitmapData data (trace, juce::Image::BitmapData::readWrite);
    for (int y = 0; y < data.height; ++y)
    {
        auto* line = data.getLinePointer (y);
        for (int x = 0; x < data.width; ++x)
        {
            auto* px = reinterpret_cast<juce::PixelARGB*> (line + x * data.pixelStride);
            if (px->getAlpha() <= 10)
                px->setARGB (0, 0, 0, 0);
        }
    }
}

void Goniometer::addSamples (const float* mid, const float* side, int numSamples, float gain)
{
    // Graphics solo vive dentro de esta función (nunca con un BitmapData abierto).
    juce::Graphics tg (trace);
    tg.addTransform (juce::AffineTransform::scale (traceScale));
    // INK: tinta casi negra y más opaca para que se vea sobre la cara crema.
    // PHOSPHOR: ámbar brillante.
    const bool ink = style == Style::ink;
    tg.setColour (ink ? juce::Colour::fromFloatRGBA (0.08f, 0.06f, 0.03f, 0.9f)
                      : col::amber.withAlpha (0.8f));

    const float cx = faceW * 0.5f, cy = 96.0f;     // centro del círculo dentro de la cara
    const float r = ink ? 1.05f : 0.9f;

    for (int i = 0; i < numSamples; ++i)
    {
        const float x = cx - side[i] * gain;
        const float y = cy - mid[i] * gain;

        if (x > 0.0f && x < faceW && y > 0.0f && y < faceH)
            tg.fillEllipse (x - r, y - r, r * 2.0f, r * 2.0f);
    }
}

void Goniometer::endFrame()
{
    repaint();
}

//==============================================================================
void Goniometer::paintStatic (juce::Graphics& g) const
{
    const bool phos = style == Style::phosphor;
    const auto box = juce::Rectangle<float> (2.0f, 2.0f, 236.0f, 206.0f);
    const auto face = juce::Rectangle<float> (facePad, facePad, faceW, faceH);

    draw::creamBox (g, box, face);
    if (phos)
    {
        g.setColour (juce::Colour (0xff0e100b));
        g.fillRoundedRectangle (face, 7.0f);
    }

    const auto ink = phos ? col::amber.withAlpha (0.45f) : col::creamInk.withAlpha (0.8f);
    g.setColour (ink);

    const float cx = 120.0f, cy = 105.0f, R = scopeRadius, q = 0.7071f;

    g.drawEllipse (cx - R, cy - R, R * 2.0f, R * 2.0f, 1.4f);

    const float dash[] = { 2.0f, 3.0f };

    {   // círculo interior punteado
        juce::Path p, d;
        p.addEllipse (cx - R / 2, cy - R / 2, R, R);
        juce::PathStrokeType (0.8f).createDashedStroke (d, p, dash, 2);
        g.fillPath (d);
    }

    {   // rombo
        juce::Path p;
        p.startNewSubPath (cx, cy - R); p.lineTo (cx + R, cy); p.lineTo (cx, cy + R); p.lineTo (cx - R, cy); p.closeSubPath();
        g.strokePath (p, juce::PathStrokeType (0.8f));
    }

    // ejes punteados (vertical, horizontal y diagonales): cada uno es su propio Path
    auto dashedLine = [&g, &dash] (float x1, float y1, float x2, float y2)
    {
        juce::Path p, d;
        p.startNewSubPath (x1, y1);
        p.lineTo (x2, y2);
        juce::PathStrokeType (0.8f).createDashedStroke (d, p, dash, 2);
        g.fillPath (d);
    };
    dashedLine (cx, cy - R, cx, cy + R);
    dashedLine (cx - R, cy, cx + R, cy);
    dashedLine (cx - R * q, cy - R * q, cx + R * q, cy + R * q);
    dashedLine (cx + R * q, cy - R * q, cx - R * q, cy + R * q);

    g.setFont (fonts::label (14.0f, true, 0.0f));
    g.setColour (phos ? col::amber : col::creamInk);
    auto label = [&g] (float x, float y, const char* t)
    {
        g.drawText (t, juce::Rectangle<float> (x - 10.0f, y - 8.0f, 20.0f, 16.0f), juce::Justification::centred, false);
    };
    label (cx - (R + 11.0f) * q, cy - (R + 11.0f) * q, "L");
    label (cx + (R + 11.0f) * q, cy - (R + 11.0f) * q, "R");
    label (cx, 15.0f, "M");
    label (cx - R - 12.0f, cy, "S");
    label (cx + R + 12.0f, cy, "S");
}

void Goniometer::paint (juce::Graphics& g)
{
    staticLayer.draw (g, width, height, [this] (juce::Graphics& ig) { paintStatic (ig); });

    g.setOpacity (1.0f);
    g.drawImage (trace, juce::Rectangle<float> (facePad, facePad, faceW, faceH));

    draw::glass (g, juce::Rectangle<float> (facePad, facePad, faceW, 96.0f));
}
