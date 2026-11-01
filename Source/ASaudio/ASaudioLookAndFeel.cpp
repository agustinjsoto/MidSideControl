#include "ASaudioLookAndFeel.h"

// Si el proyecto tiene BinaryData (Projucer), intentamos cargar las fuentes
// por nombre; si no están, usamos fuentes del sistema.
#if __has_include(<BinaryData.h>)
 #include <BinaryData.h>
 #define ASAUDIO_HAS_BINARYDATA 1
#else
 #define ASAUDIO_HAS_BINARYDATA 0
#endif

namespace asaudio
{
    //==========================================================================
    Theme Theme::makeDefault()
    {
        Theme t;
        t.panelA      = juce::Colour (0xff56665f);
        t.panelB      = juce::Colour (0xff3e4b46);
        t.panelEdge   = juce::Colour (0xff26302c);
        t.ink         = juce::Colour (0xffece4cc);
        t.inkDim      = juce::Colour (0xffb9b39c);
        t.frameBorder = juce::Colours::black.withAlpha (0.28f);
        t.isBlack     = false;
        return t;
    }

    Theme Theme::makeBlack()
    {
        Theme t;
        t.panelA      = juce::Colour (0xff2b2c2c);
        t.panelB      = juce::Colour (0xff121313);
        t.panelEdge   = juce::Colour (0xff050505);
        t.ink         = juce::Colour (0xffe9e6da);
        t.inkDim      = juce::Colour (0xffa9a69a);
        t.frameBorder = juce::Colours::white.withAlpha (0.12f);
        t.isBlack     = true;
        return t;
    }

    Theme Theme::fromName (const juce::String& name)
    {
        return name.equalsIgnoreCase ("black") ? makeBlack() : makeDefault();
    }

    //==========================================================================
    namespace
    {
        juce::Typeface::Ptr loadTypeface ([[maybe_unused]] const char* resourceName)
        {
           #if ASAUDIO_HAS_BINARYDATA
            int size = 0;
            if (auto* data = BinaryData::getNamedResource (resourceName, size))
                return juce::Typeface::createSystemTypefaceFor (data, (size_t) size);
           #endif
            return nullptr;
        }

        juce::Typeface::Ptr mediumFace()   { static auto f = loadTypeface ("BarlowCondensedMedium_ttf");         return f; }
        juce::Typeface::Ptr semiBoldFace() { static auto f = loadTypeface ("BarlowCondensedSemiBold_ttf");       return f; }
        juce::Typeface::Ptr brandFace()    { static auto f = loadTypeface ("BodoniModaItalicSemiBold_ttf");       return f; }
    }

    namespace fonts
    {
        juce::Font label (float size, bool semiBold, float tracking)
        {
            auto face = semiBold ? semiBoldFace() : mediumFace();

            juce::Font f = face != nullptr
                         ? juce::Font (juce::FontOptions (face))
                         : juce::Font (juce::FontOptions (size, semiBold ? juce::Font::bold : juce::Font::plain));

            return f.withPointHeight (size).withExtraKerningFactor (tracking);
        }

        juce::Font brand (float size)
        {
            auto face = brandFace();

            juce::Font f = face != nullptr
                         ? juce::Font (juce::FontOptions (face))
                         : juce::Font (juce::FontOptions (size, juce::Font::bold | juce::Font::italic));

            return f.withPointHeight (size).withExtraKerningFactor (0.01f);
        }
    }

    //==========================================================================
    ASaudioLookAndFeel::ASaudioLookAndFeel() : theme (Theme::makeDefault())
    {
        if (auto face = semiBoldFace())
            setDefaultSansSerifTypeface (face);

        setTheme (theme);
    }

    void ASaudioLookAndFeel::setTheme (const Theme& newTheme)
    {
        theme = newTheme;

        setColour (juce::ResizableWindow::backgroundColourId, theme.panelB);
        setColour (juce::TextEditor::backgroundColourId, juce::Colours::black);
        setColour (juce::TextEditor::textColourId, juce::Colours::white);
        setColour (juce::TextEditor::outlineColourId, col::amber);
        setColour (juce::TextEditor::focusedOutlineColourId, col::amber);
        setColour (juce::CaretComponent::caretColourId, col::amber);
        setColour (juce::TextEditor::highlightColourId, col::amber.withAlpha (0.35f));
    }

    const Theme& themeOf (const juce::Component& c)
    {
        if (auto* lf = dynamic_cast<ASaudioLookAndFeel*> (&c.getLookAndFeel()))
            return lf->getTheme();

        static const Theme fallback = Theme::makeDefault();
        return fallback;
    }

    //==========================================================================
    namespace draw
    {
        float textWidth (const juce::Font& font, const juce::String& text)
        {
            juce::GlyphArrangement ga;
            ga.addLineOfText (font, text, 0.0f, 0.0f);
            return ga.getBoundingBox (0, -1, false).getWidth();
        }

        void labelText (juce::Graphics& g, const juce::String& text, juce::Rectangle<float> area,
                        juce::Justification just, const juce::Font& font, juce::Colour colour, float shadowAlpha)
        {
            g.setFont (font);

            if (shadowAlpha > 0.0f)
            {
                g.setColour (juce::Colours::black.withAlpha (shadowAlpha));
                g.drawText (text, area.translated (0.0f, 1.0f), just, false);
            }

            g.setColour (colour);
            g.drawText (text, area, just, false);
        }

        void glowText (juce::Graphics& g, const juce::String& text, juce::Rectangle<float> area,
                       juce::Justification just, const juce::Font& font, juce::Colour colour)
        {
            g.setFont (font);
            g.setColour (colour.withAlpha (0.18f));

            for (auto o : { juce::Point<float> (-1.5f, 0), { 1.5f, 0 }, { 0, -1.5f }, { 0, 1.5f },
                            juce::Point<float> (-1, -1), { 1, 1 }, { -1, 1 }, { 1, -1 } })
                g.drawText (text, area.translated (o.x, o.y), just, false);

            g.setColour (colour);
            g.drawText (text, area, just, false);
        }

        void glow (juce::Graphics& g, const juce::Path& shape, juce::Colour colour, int radius, float alpha)
        {
            juce::DropShadow ds (colour.withAlpha (alpha), radius, { 0, 0 });
            ds.drawForPath (g, shape);
        }

        void focusHalo (juce::Graphics& g, const juce::Path& shape, float strength)
        {
            glow (g, shape, col::amber, juce::jmax (2, juce::roundToInt (6.0f * strength)), 0.9f * strength);
            glow (g, shape, col::amber, 2, 0.7f * strength);
        }

        void well (juce::Graphics& g, juce::Rectangle<float> r, float radius)
        {
            g.setColour (juce::Colours::white.withAlpha (0.12f));
            g.fillRoundedRectangle (r.translated (0.0f, 1.0f), radius);

            g.setColour (col::well);
            g.fillRoundedRectangle (r, radius);

            juce::Graphics::ScopedSaveState ss (g);
            juce::Path p;
            p.addRoundedRectangle (r, radius);
            g.reduceClipRegion (p);
            g.setGradientFill (juce::ColourGradient (juce::Colours::black.withAlpha (0.9f), 0.0f, r.getY(),
                                                     juce::Colours::transparentBlack, 0.0f, r.getY() + 4.5f, false));
            g.fillRect (r);
        }

        void keyCap (juce::Graphics& g, juce::Rectangle<float> r, float radius, bool lit, float brighten)
        {
            if (lit)
            {
                g.setGradientFill (juce::ColourGradient (col::selTop, 0.0f, r.getY(), col::selBottom, 0.0f, r.getBottom(), false));
                g.fillRoundedRectangle (r, radius);

                juce::Graphics::ScopedSaveState ss (g);
                juce::Path p;
                p.addRoundedRectangle (r, radius);
                g.reduceClipRegion (p);
                g.setGradientFill (juce::ColourGradient (juce::Colours::black.withAlpha (0.8f), 0.0f, r.getY(),
                                                         juce::Colours::transparentBlack, 0.0f, r.getY() + 3.5f, false));
                g.fillRect (r);
            }
            else
            {
                g.setColour (juce::Colours::black.withAlpha (0.6f));
                g.fillRoundedRectangle (r.translated (0.0f, 1.0f), radius);

                g.setGradientFill (juce::ColourGradient (col::keyTop.brighter (brighten), 0.0f, r.getY(),
                                                         col::keyBottom.brighter (brighten), 0.0f, r.getBottom(), false));
                g.fillRoundedRectangle (r, radius);

                g.setColour (juce::Colours::white.withAlpha (0.12f));
                g.drawLine (r.getX() + radius * 0.6f, r.getY() + 0.5f, r.getRight() - radius * 0.6f, r.getY() + 0.5f, 1.0f);
            }
        }

        void screw (juce::Graphics& g, juce::Rectangle<float> r, float angleDegrees)
        {
            const auto c = r.getCentre();

            g.setColour (juce::Colours::black.withAlpha (0.6f));
            g.fillEllipse (r.translated (0.0f, 1.0f));

            g.setGradientFill (juce::ColourGradient (col::brassA, r.getX() + r.getWidth() * 0.35f, r.getY() + r.getHeight() * 0.30f,
                                                     col::brassB, r.getRight(), r.getBottom(), true));
            g.fillEllipse (r);
            g.setColour (juce::Colours::black.withAlpha (0.25f));
            g.drawEllipse (r.reduced (0.5f), 1.0f);

            // ranura en cruz
            for (int k = 0; k < 2; ++k)
            {
                const auto rot = juce::AffineTransform::rotation (juce::degreesToRadians (angleDegrees + 90.0f * (float) k), c.x, c.y);
                juce::Path slot;
                slot.addRoundedRectangle (c.x - r.getWidth() * 0.5f + 2.0f, c.y - 1.5f, r.getWidth() - 4.0f, 3.0f, 1.0f);

                g.setColour (juce::Colours::white.withAlpha (0.18f));
                g.fillPath (slot, rot.translated (0.0f, 1.0f));
                g.setColour (juce::Colour::fromFloatRGBA (0.16f, 0.10f, 0.02f, 0.8f));
                g.fillPath (slot, rot);
            }
        }

        static const juce::Image& noiseTile()
        {
            static juce::Image tile = []
            {
                juce::Image img (juce::Image::ARGB, 160, 160, true);
                juce::Image::BitmapData bd (img, juce::Image::BitmapData::writeOnly);
                juce::Random rnd (42);

                for (int y = 0; y < bd.height; ++y)
                    for (int x = 0; x < bd.width; ++x)
                        bd.setPixelColour (x, y, juce::Colour::fromRGBA (0, 0, 0, (juce::uint8) rnd.nextInt (112)));

                return img;
            }();
            return tile;
        }

        void panel (juce::Graphics& g, juce::Rectangle<float> bounds, const Theme& theme)
        {
            juce::Path outline;
            outline.addRoundedRectangle (bounds, 12.0f);

            g.setGradientFill (juce::ColourGradient (theme.panelA, 0.0f, bounds.getY(), theme.panelB, 0.0f, bounds.getBottom(), false));
            g.fillPath (outline);

            {
                juce::Graphics::ScopedSaveState ss (g);
                g.reduceClipRegion (outline);

                // textura de ruido fino
                g.setTiledImageFill (noiseTile(), 0, 0, 1.0f);
                g.fillRect (bounds);

                // sombra interior inferior
                g.setGradientFill (juce::ColourGradient (juce::Colours::transparentBlack, 0.0f, bounds.getBottom() - 60.0f,
                                                         juce::Colours::black.withAlpha (0.25f), 0.0f, bounds.getBottom(), false));
                g.fillRect (bounds);

                // luz superior sutil
                g.setColour (juce::Colours::white.withAlpha (0.08f));
                g.fillRect (bounds.getX() + 8.0f, bounds.getY() + 2.0f, bounds.getWidth() - 16.0f, 3.0f);
            }

            // borde interior de 2 px
            g.setColour (theme.panelEdge);
            g.drawRoundedRectangle (bounds.reduced (1.0f), 11.0f, 2.0f);

            // 4 tornillos de latón, a 14 px de cada esquina
            const float s = 16.0f, m = 14.0f;
            screw (g, { bounds.getX() + m,                 bounds.getY() + m,                  s, s }, 20.0f);
            screw (g, { bounds.getRight() - m - s,         bounds.getY() + m,                  s, s }, 75.0f);
            screw (g, { bounds.getX() + m,                 bounds.getBottom() - m - s,         s, s }, 140.0f);
            screw (g, { bounds.getRight() - m - s,         bounds.getBottom() - m - s,         s, s }, 50.0f);
        }

        void creamBox (juce::Graphics& g, juce::Rectangle<float> box, juce::Rectangle<float> face)
        {
            g.setColour (col::boxDark);
            g.fillRoundedRectangle (box, 12.0f);
            g.setColour (col::boxBrass.withAlpha (0.7f));
            g.drawRoundedRectangle (box.reduced (1.0f), 12.0f, 2.0f);

            // cara: radial desde abajo al centro (#f7e3a0 -> #e4c675 -> #c9a655)
            juce::ColourGradient grad (col::creamA, face.getCentreX(), face.getBottom(),
                                       col::creamC, face.getCentreX() + face.getWidth(), face.getBottom(), true);
            grad.addColour (0.7, col::creamB);
            g.setGradientFill (grad);
            g.fillRoundedRectangle (face, 7.0f);
        }

        void glass (juce::Graphics& g, juce::Rectangle<float> r)
        {
            g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.28f), 0.0f, r.getY(),
                                                     juce::Colours::white.withAlpha (0.0f), 0.0f, r.getBottom(), false));
            g.fillRoundedRectangle (r, 7.0f);
        }
    }
}
