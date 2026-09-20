#include "GoniometerComponent.h"
#include <cmath>

namespace
{
    // Paleta sacada de la interfaz del plugin (fondo azul-gris oscuro,
    // acento celeste/cian usado en los sliders y el knob).
    const juce::Colour kBackgroundColour { 0xff20242e }; 
    const juce::Colour kGridColour       { 0xff3a4150 }; 
    const juce::Colour kLabelColour      { 0xff8b92a3 };

    // --- Etiquetas y radio del círculo ---
    // Reservamos este espacio por fuera del círculo para las letras (M, S,
    // L, R). El radio se calcula restándolo, así nunca quedan cortadas
    // contra el borde del componente.
    constexpr float kLabelMargin = 12.0f;
    constexpr float kLabelBoxW = 26.0f;
    constexpr float kLabelBoxH = 16.0f;

    // Un único lugar donde se decide el radio, para que la grilla y los
    // puntos siempre usen exactamente el mismo.
    inline float getScopeRadius(juce::Rectangle<float> bounds)
    {
        const float half = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
        return juce::jmax(4.0f, half - kLabelMargin - kLabelBoxW * 0.5f);
    }

    // Por debajo de este alpha (de 0 a 255) consideramos que el píxel ya no
    // se ve y lo borramos del todo. Subilo si todavía notás sombra.
    constexpr int kAlphaFloor = 10;

    // Persistencia fija, misma velocidad que el goniómetro de referencia en
    // JSFX (ahí sale de tapar la pantalla con negro al 5% de alpha cada
    // frame == decay de 0.95 por frame acá).
    constexpr float kPersistenceDecay = 0.95f;

    // Radio de cada puntito en píxeles. 0.7 = ~1.4px de diámetro.
    // Más chico = trazo más fino (pero también más tenue).
    constexpr float kDotRadius = 0.7f;

    // --- Medidor de correlación ---
    constexpr float kCorrelationMeterHeight = 14.0f;
    constexpr float kCorrelationMeterGap = 4.0f;

    // Verde = en fase (+1), rojo = fuera de fase (-1), amarillo en el medio.
    juce::Colour correlationToColour(float corr)
    {
        corr = juce::jlimit(-1.0f, 1.0f, corr);
        const juce::Colour red{ 0xffe0524f };
        const juce::Colour yellow{ 0xffe0c44f };
        const juce::Colour green{ 0xff4fe07a };

        return corr >= 0.0f ? yellow.interpolatedWith(green, corr)
            : yellow.interpolatedWith(red, -corr);
    }
}

GoniometerComponent::GoniometerComponent (MidSideVolumeAudioProcessor& processorToUse)
    : processor (processorToUse)
{
    lastReadIndex = processor.getGoniometerWriteIndex();
    runningPeakLevel = 0.5f; // arranca "abierto" y va ajustando solo al nivel real
    startTimerHz (30); // 30 refrescos por segundo, suficiente para que se vea fluido sin gastar CPU de más
}

GoniometerComponent::~GoniometerComponent()
{
    stopTimer();
}

void GoniometerComponent::timerCallback()
{
    repaint();
}

void GoniometerComponent::resized()
{
    // Recreamos la imagen de persistencia al tamaño actual del componente
    // (arranca totalmente transparente: ahí solo vive el rastro de puntos).
    const int w = juce::jmax (1, getWidth());
    const int h = juce::jmax (1, getHeight());

    persistenceImage = juce::Image (juce::Image::ARGB, w, h, true);
}

void GoniometerComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Franja abajo para el medidor de correlación; "bounds" queda achicado
    // para las llamadas siguientes, así el círculo y los puntos comparten
    // el mismo centro y radio.
    auto meterBounds = bounds.removeFromBottom(kCorrelationMeterHeight);
    bounds.removeFromBottom(kCorrelationMeterGap);

    drawBackground(g, bounds);
    drawTrace(g, bounds);
    drawCorrelationMeter(g, meterBounds);
}

void GoniometerComponent::drawBackground (juce::Graphics& g, juce::Rectangle<float> bounds) const
{
    g.setColour (kBackgroundColour);
    g.fillRoundedRectangle (bounds, 6.0f);

    const auto centre = bounds.getCentre();
    const auto radius = getScopeRadius(bounds);

    g.setColour (kGridColour);
    g.drawEllipse (centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f, 1.0f);

    // Cruz: vertical = Mid, horizontal = Side
    g.drawLine (centre.x, centre.y - radius, centre.x, centre.y + radius, 1.0f);
    g.drawLine (centre.x - radius, centre.y, centre.x + radius, centre.y, 1.0f);

    // Diagonales a 45°: ahí caen L (arriba-izq / abajo-der) y R (arriba-der / abajo-izq)
    const auto diag = radius * 0.70710678f; // radius / sqrt(2)
    g.setColour (kGridColour.withAlpha (0.6f));
    g.drawLine (centre.x - diag, centre.y - diag, centre.x + diag, centre.y + diag, 0.5f);
    g.drawLine (centre.x - diag, centre.y + diag, centre.x + diag, centre.y - diag, 0.5f);

    g.setFont(11.0f);
    g.setColour(kLabelColour);

    auto drawLabel = [&](const juce::String& text, float angleRadians)
        {
            const float dist = radius + kLabelMargin;
            const auto pt = centre.getPointOnCircumference(dist, dist, angleRadians);
            g.drawText(text,
                juce::Rectangle<float>(pt.x - kLabelBoxW * 0.5f, pt.y - kLabelBoxH * 0.5f, kLabelBoxW, kLabelBoxH),
                juce::Justification::centred);
        };

    // Ángulos en radianes, 0 = arriba, en sentido horario (así funciona
    // getPointOnCircumference en JUCE).
    drawLabel("M", 0.0f);                                        // arriba
    drawLabel("+S", juce::MathConstants<float>::pi * 1.5f);        // izquierda
    drawLabel("-S", juce::MathConstants<float>::pi * 0.5f);        // derecha
    drawLabel("L", juce::MathConstants<float>::pi * 1.75f);       // arriba-izquierda
    drawLabel("R", juce::MathConstants<float>::pi * 0.25f);       // arriba-derecha
}

void GoniometerComponent::drawTrace (juce::Graphics& g, juce::Rectangle<float> bounds)
{
    if (persistenceImage.isNull())
        return;

    const auto& midBuffer  = processor.getGoniometerMidBuffer();
    const auto& sideBuffer = processor.getGoniometerSideBuffer();
    const int bufferSize   = MidSideVolumeAudioProcessor::goniometerBufferSize;

    const int currentIndex = processor.getGoniometerWriteIndex();

    // Cuántas muestras nuevas llegaron desde el último frame (con tope al
    // tamaño del buffer, por si el audio estuvo parado un buen rato).
    int numNewSamples = currentIndex - lastReadIndex;
    if (numNewSamples < 0)
        numNewSamples += bufferSize;
    numNewSamples = juce::jmin (numNewSamples, bufferSize);

    // 1) Desvanecemos el rastro anterior (esto es lo que da la sensación de
    //    "promedio"/persistencia en vez de un refresco instantáneo).
    //    multiplyAllAlphas baja el alpha de cada píxel ya dibujado, sin
    //    tocar el fondo/grilla que están en otra capa.
    fadePersistence();

    // 2) Agregamos solo las muestras nuevas encima.
    {
        juce::Graphics pg(persistenceImage);

        const auto centre = bounds.getCentre();
        const auto radius = getScopeRadius(bounds);

        // Escala "auto-gain": el processor ya calcula el nivel de pico
        // muestra a muestra (attack/release en ms reales); acá solo lo leemos.
        const float autoScale = (radius * 0.9f) / processor.getDisplayPeakLevel();

        // Mismo color que el medidor de correlación: verde en fase, rojo
        // fuera de fase. Lo tomamos una vez por frame (no por punto), la
        // correlación no cambia tan rápido como para necesitar más que eso.
        pg.setColour(correlationToColour(processor.getCorrelation()));

        int idx = lastReadIndex;
        for (int n = 0; n < numNewSamples; ++n)
        {
            const float mid = midBuffer[(size_t)idx];
            const float side = sideBuffer[(size_t)idx];

            const float x = centre.x - side * autoScale;
            const float y = centre.y - mid * autoScale;

            pg.fillEllipse(x - kDotRadius, y - kDotRadius, kDotRadius * 2.0f, kDotRadius * 2.0f);

            idx = (idx + 1) % bufferSize;
        }
    }

    lastReadIndex = currentIndex;

    g.drawImageAt (persistenceImage, (int) bounds.getX(), (int) bounds.getY());
}

void GoniometerComponent::fadePersistence()
{
    persistenceImage.multiplyAllAlphas(kPersistenceDecay);

    juce::Image::BitmapData data(persistenceImage, juce::Image::BitmapData::readWrite);

    for (int y = 0; y < data.height; ++y)
    {
        auto* line = data.getLinePointer(y);

        for (int x = 0; x < data.width; ++x)
        {
            auto* px = reinterpret_cast<juce::PixelARGB*>(line + x * data.pixelStride);

            if (px->getAlpha() <= kAlphaFloor)
                px->setARGB(0, 0, 0, 0);
        }
    }
}

void GoniometerComponent::drawCorrelationMeter(juce::Graphics& g, juce::Rectangle<float> area) const
{
    const float corr = processor.getCorrelation();

    // Pista de fondo (todo el recorrido, de -1 a +1)
    g.setColour(kGridColour);
    g.fillRoundedRectangle(area, area.getHeight() * 0.5f);

    // Marca del centro (correlación 0)
    g.setColour(kLabelColour.withAlpha(0.5f));
    g.drawVerticalLine((int)area.getCentreX(), area.getY(), area.getBottom());

    // Relleno desde el centro hasta el valor actual, coloreado según qué
    // tan en fase (verde) o fuera de fase (rojo) está la señal.
    const float halfWidth = area.getWidth() * 0.5f;
    const float valueX = area.getCentreX() + corr * halfWidth;

    juce::Rectangle<float> fillArea(juce::jmin(area.getCentreX(), valueX), area.getY(),
        std::abs(valueX - area.getCentreX()), area.getHeight());

    g.setColour(correlationToColour(corr));
    g.fillRoundedRectangle(fillArea, area.getHeight() * 0.5f);
}
