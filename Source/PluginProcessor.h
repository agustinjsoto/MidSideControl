#pragma once

#include <JuceHeader.h>

class MidSideVolumeAudioProcessor : public juce::AudioProcessor
{
public:
    MidSideVolumeAudioProcessor();
    ~MidSideVolumeAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    //==============================================================================
    const juce::String getName() const override { return JucePlugin_Name; }

    bool acceptsMidi() const override  { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    //==============================================================================
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    // Árbol de parámetros compartido entre el processor y el editor (para poder
    // hacer los attachments de los faders y el knob).
    juce::AudioProcessorValueTreeState apvts;

    // IDs de los parámetros, por si los necesitás desde el editor.
    static constexpr auto midGainId      = "midGain";
    static constexpr auto sideGainId     = "sideGain";
    static constexpr auto outputVolumeId = "outputVolume";
    static constexpr auto panId          = "pan";
    static constexpr auto panLawId       = "panLaw";

    // Valor de dB que tratamos como "-inf" (silencio total). Tiene que
    // coincidir con el mínimo que le pongas a cada NormalisableRange.
    static constexpr float minusInfinityDb = -60.0f;

    // --- Pan law ---
    // Cuántos dB baja cada canal cuando el pan está en el centro (respecto
    // de cuando está todo hacia un lado). El índice coincide con el orden de
    // las opciones del parámetro "panLaw": +0.0, -2.5, -3.0, -4.5, -6.0.
    // El índice 0 (+0.0) es el valor por defecto.
    static constexpr std::array<float, 5> panLawCentreDb { 0.0f, -2.5f, -3.0f, -4.5f, -6.0f };

    //==============================================================================
    // --- Datos para el goniómetro (vectorscopio) ---
    // El processor va escribiendo acá las muestras de Mid/Side ya procesadas
    // (con las ganancias aplicadas), y el editor las lee para dibujarlas.
    // No usamos locks: para una visualización, si algún punto se pisa o se lee
    // "a medio escribir" no se nota ni importa; evitamos meter latencia al audio.
    static constexpr int goniometerBufferSize = 2048;

    const std::array<float, goniometerBufferSize>& getGoniometerMidBuffer() const  { return goniometerMidBuffer; }
    const std::array<float, goniometerBufferSize>& getGoniometerSideBuffer() const { return goniometerSideBuffer; }

    int getGoniometerWriteIndex() const { return goniometerWriteIndex.load(std::memory_order_relaxed); }

    // --- Auto-gain y correlación para el goniómetro ---
    // Se calculan acá, muestra a muestra en el hilo de audio, para tener
    // tiempos de reacción reales en ms, sin depender del frame-rate del editor.
    float getDisplayPeakLevel() const { return displayPeakLevel.load(std::memory_order_relaxed); }
    float getCorrelation() const { return correlationValue.load(std::memory_order_relaxed); }

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Punteros atómicos a los valores actuales de cada parámetro (thread-safe,
    // se leen directamente en processBlock sin locks).
    std::atomic<float>* midGainParam      = nullptr;
    std::atomic<float>* sideGainParam     = nullptr;
    std::atomic<float>* outputVolumeParam = nullptr;
    std::atomic<float>* panParam          = nullptr;
    std::atomic<float>* panLawParam       = nullptr; // índice de la opción elegida (0..4)

    std::array<float, goniometerBufferSize> goniometerMidBuffer {};
    std::array<float, goniometerBufferSize> goniometerSideBuffer {};
    std::atomic<int> goniometerWriteIndex{ 0 };

    // Coeficientes calculados en prepareToPlay a partir del sample rate real.
    // Attack ~3ms, release ~1s (auto-gain), suavizado de correlación ~0.6s:
    // mismos tiempos que el goniómetro de referencia en JSFX.
    float peakAttackCoeff = 0.0f;
    float peakReleaseCoeff = 0.0f;
    float correlationSmoothingCoeff = 0.0f;

    std::atomic<float> displayPeakLevel{ 0.12f };
    std::atomic<float> correlationValue{ 0.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidSideVolumeAudioProcessor)
};
