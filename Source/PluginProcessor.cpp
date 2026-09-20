#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

//==============================================================================
MidSideVolumeAudioProcessor::MidSideVolumeAudioProcessor()
    : AudioProcessor (BusesProperties()
                           .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                           .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    midGainParam      = apvts.getRawParameterValue (midGainId);
    sideGainParam      = apvts.getRawParameterValue (sideGainId);
    outputVolumeParam = apvts.getRawParameterValue (outputVolumeId);
}

MidSideVolumeAudioProcessor::~MidSideVolumeAudioProcessor() = default;

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout MidSideVolumeAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // El mínimo (minusInfinityDb) es el punto en el que consideramos que el
    // fader está en "-inf" (silencio total), no solo -60dB de atenuación.
    auto dbRange = juce::NormalisableRange<float> (minusInfinityDb, 24.0f, 0.1f);
    dbRange.setSkewForCentre (-6.0f); // hace que la mitad del recorrido caiga cerca de -6dB, se siente más natural que lineal

    // Convierte el valor numérico a texto, mostrando "-inf" en el mínimo
    // en vez de "-60.0".
    auto dbTextFromValue = [] (float value, int)
    {
        return value <= minusInfinityDb + 0.01f ? juce::String ("-inf") : juce::String (value, 1);
    };
    auto dbValueFromText = [] (const juce::String& text)
    {
        return text.containsIgnoreCase ("inf") ? minusInfinityDb : text.getFloatValue();
    };

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { midGainId, 1 },
        "Mid",
        dbRange,
        0.0f,
        juce::AudioParameterFloatAttributes()
            .withLabel ("dB")
            .withStringFromValueFunction (dbTextFromValue)
            .withValueFromStringFunction (dbValueFromText)));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { sideGainId, 1 },
        "Side",
        dbRange,
        0.0f,
        juce::AudioParameterFloatAttributes()
            .withLabel ("dB")
            .withStringFromValueFunction (dbTextFromValue)
            .withValueFromStringFunction (dbValueFromText)));

    auto outputRange = juce::NormalisableRange<float> (minusInfinityDb, 12.0f, 0.1f);
    outputRange.setSkewForCentre (-6.0f);

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { outputVolumeId, 1 },
        "Output",
        outputRange,
        0.0f,
        juce::AudioParameterFloatAttributes()
            .withLabel ("dB")
            .withStringFromValueFunction (dbTextFromValue)
            .withValueFromStringFunction (dbValueFromText)));

    return layout;
}

//==============================================================================
void MidSideVolumeAudioProcessor::prepareToPlay(double sampleRate, int)
{
    // Mismos tiempos que el goniómetro de referencia en JSFX: el "zoom" se
    // achica rápido (3ms) si la señal se pone más fuerte, y se agranda de a
    // poco (1s) cuando baja. La correlación se suaviza en 0.6s.
    peakAttackCoeff = (float)std::exp(-1.0 / (0.003 * sampleRate));
    peakReleaseCoeff = (float)std::exp(-1.0 / (1.000 * sampleRate));
    correlationSmoothingCoeff = (float)(1.0 / (sampleRate * 0.6));
}

void MidSideVolumeAudioProcessor::releaseResources() {}

bool MidSideVolumeAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // Este plugin trabaja sobre M/S, así que requiere estéreo in/out.
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo()
        && layouts.getMainInputChannelSet()  == juce::AudioChannelSet::stereo();
}

void MidSideVolumeAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    // Si por algún motivo no hay 2 canales, no tocamos nada.
    if (buffer.getNumChannels() < 2)
        return;

    auto* left  = buffer.getWritePointer (0);
    auto* right = buffer.getWritePointer (1);

    // El segundo argumento le dice a JUCE: "si el valor es <= este umbral,
    // la ganancia es 0.0 exacto (silencio real), no una atenuación de -60dB".
    const float midGainLinear    = juce::Decibels::decibelsToGain (midGainParam->load(),    minusInfinityDb);
    const float sideGainLinear   = juce::Decibels::decibelsToGain (sideGainParam->load(),   minusInfinityDb);
    const float outputGainLinear = juce::Decibels::decibelsToGain (outputVolumeParam->load(), minusInfinityDb);

    const int numSamples = buffer.getNumSamples();

    int gonioWriteIndex = goniometerWriteIndex.load(std::memory_order_relaxed);
    float peak = displayPeakLevel.load(std::memory_order_relaxed);
    float correlation = correlationValue.load(std::memory_order_relaxed);

    for (int i = 0; i < numSamples; ++i)
    {
        const float l = left[i];
        const float r = right[i];

        // L/R -> Mid/Side
        float mid = (l + r) * 0.5f;
        float side = (l - r) * 0.5f;

        // Ganancia independiente para mid y side
        mid *= midGainLinear;
        side *= sideGainLinear;

        // Esto es exactamente lo que se ve en el goniómetro (con el volumen
        // de salida ya aplicado); auto-gain y correlación se calculan sobre
        // estos mismos valores.
        const float dispMid = mid * outputGainLinear;
        const float dispSide = side * outputGainLinear;

        goniometerMidBuffer[(size_t)gonioWriteIndex] = dispMid;
        goniometerSideBuffer[(size_t)gonioWriteIndex] = dispSide;
        gonioWriteIndex = (gonioWriteIndex + 1) % goniometerBufferSize;

        // --- Auto-gain (seguidor de pico, muestra a muestra) ---
        const float instantLevel = std::sqrt(dispMid * dispMid + dispSide * dispSide);
        const float peakCoeff = (instantLevel > peak) ? peakAttackCoeff : peakReleaseCoeff;
        peak = instantLevel + (peak - instantLevel) * peakCoeff;

        // --- Correlación (Mid vs Side, igual criterio que el JSFX) ---
        const float am = std::abs(dispMid);
        const float as = std::abs(dispSide);
        float instCorr = (am >= as) ? 1.0f : -1.0f;
        if (am == as && am > 0.001f)
            instCorr = 0.0f;
        correlation += (instCorr - correlation) * correlationSmoothingCoeff;

        // Mid/Side -> L/R, y aplicamos el volumen de salida general
        left[i] = (mid + side) * outputGainLinear;
        right[i] = (mid - side) * outputGainLinear;
    }

    goniometerWriteIndex.store(gonioWriteIndex, std::memory_order_relaxed);
    displayPeakLevel.store(juce::jmax(peak, 0.12f), std::memory_order_relaxed);
    correlationValue.store(correlation, std::memory_order_relaxed);
}

//==============================================================================
juce::AudioProcessorEditor* MidSideVolumeAudioProcessor::createEditor()
{
    return new MidSideVolumeAudioProcessorEditor (*this);
}

//==============================================================================
void MidSideVolumeAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Guarda el estado de los parámetros (para que el DAW recuerde los valores).
    if (auto state = apvts.copyState(); true)
    {
        std::unique_ptr<juce::XmlElement> xml (state.createXml());
        copyXmlToBinary (*xml, destData);
    }
}

void MidSideVolumeAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState != nullptr)
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
// Esta función es requerida por JUCE para instanciar el plugin.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MidSideVolumeAudioProcessor();
}
