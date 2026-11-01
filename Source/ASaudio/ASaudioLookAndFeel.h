#pragma once

#include <JuceHeader.h>

// =============================================================================
// ASaudio - sistema de diseño: temas, tipografías y funciones de dibujo
// compartidas por todos los componentes.
// =============================================================================
namespace asaudio
{
    //==========================================================================
    // Colores comunes a ambos temas
    namespace col
    {
        inline const juce::Colour amber       { 0xfff0b341 };
        inline const juce::Colour lampHi      { 0xfffff3c9 };
        inline const juce::Colour lampLo      { 0xffb86f12 };
        inline const juce::Colour brassA      { 0xffe3c47a };
        inline const juce::Colour brassB      { 0xff8c6a2b };
        inline const juce::Colour ledOffA     { 0xff4a3a1c };
        inline const juce::Colour ledOffB     { 0xff1c1508 };
        inline const juce::Colour ledAmberOff { 0xff2c200c };
        inline const juce::Colour ledRedOff   { 0xff2f1510 };
        inline const juce::Colour ledRed      { 0xffd24a2a };
        inline const juce::Colour well        { 0xff0d0f0e };
        inline const juce::Colour valueBg     { 0xff15110b };
        inline const juce::Colour boxDark     { 0xff14100a };
        inline const juce::Colour boxBrass    { 0xffcaa14e };
        inline const juce::Colour creamInk    { 0xff2a2216 };
        inline const juce::Colour creamA      { 0xfff7e3a0 };
        inline const juce::Colour creamB      { 0xffe4c675 };
        inline const juce::Colour creamC      { 0xffc9a655 };
        inline const juce::Colour keyTop      { 0xff3a3d3c };
        inline const juce::Colour keyBottom   { 0xff232625 };
        inline const juce::Colour selTop      { 0xff1b1308 };
        inline const juce::Colour selBottom   { 0xff2a1d0a };
    }

    //==========================================================================
    // Todo color de panel sale de acá; ningún componente hardcodea estos tokens.
    struct Theme
    {
        juce::Colour panelA, panelB, panelEdge, ink, inkDim, frameBorder;
        bool isBlack = false;

        static Theme makeDefault();
        static Theme makeBlack();
        static Theme fromName (const juce::String& name);   // "default" | "black"
        juce::String getName() const { return isBlack ? "black" : "default"; }
    };

    //==========================================================================
    // Tipografías (Barlow Condensed y Bodoni Moda Italic, embebidas en
    // BinaryData). Si no están, cae a las fuentes del sistema.
    namespace fonts
    {
        // size = tamaño en px "CSS" (em); tracking = letter-spacing en em.
        juce::Font label (float size, bool semiBold = true, float tracking = 0.12f);
        juce::Font brand (float size);
    }

    //==========================================================================
    class ASaudioLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        ASaudioLookAndFeel();

        void setTheme (const Theme& newTheme);
        const Theme& getTheme() const noexcept { return theme; }

    private:
        Theme theme;
    };

    // Tema vigente para un componente (si no cuelga de un ASaudioLookAndFeel,
    // usa el DEFAULT).
    const Theme& themeOf (const juce::Component& c);

    //==========================================================================
    // Imagen cacheada para capas estáticas. Se regenera sola si cambia el
    // tamaño o la escala física (Retina / zoom de ventana).
    struct CachedLayer
    {
        juce::Image image;
        float scale = 0.0f;
        int w = 0, h = 0;

        void invalidate() { image = {}; }

        template <typename Fn>
        void draw (juce::Graphics& g, int width, int height, Fn&& paintFn, juce::Point<int> pos = {})
        {
            const float s = juce::jmax (1.0f, g.getInternalContext().getPhysicalPixelScaleFactor());

            if (image.isNull() || s != scale || width != w || height != h)
            {
                scale = s; w = width; h = height;
                image = juce::Image (juce::Image::ARGB,
                                     juce::jmax (1, (int) std::ceil ((float) width * s)),
                                     juce::jmax (1, (int) std::ceil ((float) height * s)), true);
                juce::Graphics ig (image);
                ig.addTransform (juce::AffineTransform::scale ((float) image.getWidth() / (float) width,
                                                               (float) image.getHeight() / (float) height));
                paintFn (ig);
            }

            // JUCE dibuja una imagen con la opacidad del color actual: si antes se
            // dibujó algo translúcido (un halo, una sombra), la capa saldría
            // transparente. Siempre la dibujamos al 100 %.
            juce::Graphics::ScopedSaveState ss (g);
            g.setOpacity (1.0f);
            g.drawImage (image, juce::Rectangle<float> ((float) pos.x, (float) pos.y, (float) width, (float) height));
        }
    };

    //==========================================================================
    // Funciones de dibujo compartidas
    namespace draw
    {
        float textWidth (const juce::Font& font, const juce::String& text);

        // Texto con sombra negra de 1 px (como los labels del catálogo).
        void labelText (juce::Graphics& g, const juce::String& text, juce::Rectangle<float> area,
                        juce::Justification just, const juce::Font& font, juce::Colour colour,
                        float shadowAlpha = 0.55f);

        // Texto con halo (valores activos, teclas seleccionadas).
        void glowText (juce::Graphics& g, const juce::String& text, juce::Rectangle<float> area,
                       juce::Justification just, const juce::Font& font, juce::Colour colour);

        void glow (juce::Graphics& g, const juce::Path& shape, juce::Colour colour, int radius, float alpha);
        // halo ámbar de selección; strength 1 = completo (fader), menos = más tenue
        void focusHalo (juce::Graphics& g, const juce::Path& shape, float strength = 1.0f);

        void well (juce::Graphics& g, juce::Rectangle<float> r, float radius);        // pozo oscuro
        void keyCap (juce::Graphics& g, juce::Rectangle<float> r, float radius,
                     bool lit, float brighten = 0.0f);                                  // tecla gris / ámbar

        void screw (juce::Graphics& g, juce::Rectangle<float> r, float angleDegrees);
        void panel (juce::Graphics& g, juce::Rectangle<float> bounds, const Theme& theme);

        // Caja oscura con marco de latón y cara crema (goniómetro, VU, etc.).
        void creamBox (juce::Graphics& g, juce::Rectangle<float> box, juce::Rectangle<float> face);
        void glass (juce::Graphics& g, juce::Rectangle<float> r);                       // reflejo superior
    }
}
