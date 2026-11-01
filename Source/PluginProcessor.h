#pragma once

#include <JuceHeader.h>

class MidSideVolumeAudioProcessor : public juce::AudioProcessor
{
public:
    MidSideVolumeAudioProcessor();
    ~MidSideVolumeAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    //==============================================================================
    const juce::String getName() const override { return JucePlugin_Name; }

    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    //==============================================================================
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    //==============================================================================
    // Árbol de parámetros compartido entre el processor y el editor (para poder
    // hacer los attachments de los faders y el knob).
    juce::AudioProcessorValueTreeState apvts;

    // IDs de los parámetros, por si los necesitás desde el editor.
    static constexpr auto midGainId = "midGain";
    static constexpr auto sideGainId = "sideGain";
    static constexpr auto outputVolumeId = "outputVolume";
    static constexpr auto panId = "pan";
    static constexpr auto panLawId = "panLaw";
    static constexpr auto monoFreqId = "monoFreq";

    // Valor de dB que tratamos como "-inf" (silencio total). Tiene que
    // coincidir con el mínimo que le pongas a cada NormalisableRange.
    static constexpr float minusInfinityDb = -60.0f;

    // --- Mono Maker: rango de la frecuencia de corte (Hz) ---
    // Mismos valores que el JSFX S_SafeBass v2.0 (10 Hz a 10 kHz, default 100).
    static constexpr float monoFreqMin = 10.0f;
    static constexpr float monoFreqMax = 10000.0f;
    static constexpr float monoFreqDefault = monoFreqMin;   // antes: 100.0f

    // El mínimo del knob (10 Hz) es "Off": por encima de este umbral el Mono
    // Maker está prendido. 0.5 Hz de margen para no depender de que el knob
    // caiga en 10.0 exacto (un corte a ~10 Hz casi no se nota, así que el
    // paso de apagado a prendido es prácticamente continuo).
    static constexpr float monoOffThreshold = monoFreqMin + 0.5f;

    // --- Pan law ---
    // Cuántos dB baja cada canal cuando el pan está en el centro (respecto
    // de cuando está todo hacia un lado). El índice coincide con el orden de
    // las opciones del parámetro "panLaw": +0.0, -2.5, -3.0, -4.5, -6.0.
    // El índice 0 (+0.0) es el valor por defecto.
    static constexpr std::array<float, 5> panLawCentreDb{ 0.0f, -2.5f, -3.0f, -4.5f, -6.0f };

    //==============================================================================
    // --- Datos para el goniómetro (vectorscopio) ---
    // El processor va escribiendo acá las muestras de Mid/Side ya procesadas
    // (con las ganancias aplicadas), y el editor las lee para dibujarlas.
    // No usamos locks: para una visualización, si algún punto se pisa o se lee
    // "a medio escribir" no se nota ni importa; evitamos meter latencia al audio.
    static constexpr int goniometerBufferSize = 2048;

    const std::array<float, goniometerBufferSize>& getGoniometerMidBuffer() const { return goniometerMidBuffer; }
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
    std::atomic<float>* midGainParam = nullptr;
    std::atomic<float>* sideGainParam = nullptr;
    std::atomic<float>* outputVolumeParam = nullptr;
    std::atomic<float>* panParam = nullptr;
    std::atomic<float>* panLawParam = nullptr; // índice de la opción elegida (0..4)
    std::atomic<float>* monoFreqParam = nullptr; // Hz (en el mínimo = Off)

    std::array<float, goniometerBufferSize> goniometerMidBuffer{};
    std::array<float, goniometerBufferSize> goniometerSideBuffer{};
    std::atomic<int> goniometerWriteIndex{ 0 };

    // Coeficientes calculados en prepareToPlay a partir del sample rate real.
    // Attack ~3ms, release ~1s (auto-gain), suavizado de correlación ~0.6s:
    // mismos tiempos que el goniómetro de referencia en JSFX.
    float peakAttackCoeff = 0.0f;
    float peakReleaseCoeff = 0.0f;
    float correlationSmoothingCoeff = 0.0f;

    std::atomic<float> displayPeakLevel{ 0.12f };
    std::atomic<float> correlationValue{ 0.0f };

    //==============================================================================
    // --- Mono Maker (port del JSFX S_SafeBass v2.0) ---
    // Crossover Linkwitz-Riley de 4to orden (LR4): 2 biquads Butterworth
    // (Q = 1/sqrt2) en cascada para el low-pass y otros 2 para el high-pass.
    // Los graves de L y R se promedian a mono; los agudos quedan estéreo.
    // Todo en double: a 10 Hz los coeficientes están muy cerca de 1.0 y en
    // float el filtro se pone ruidoso.
    struct BiquadCoeffs
    {
        double b0 = 1.0, b1 = 0.0, b2 = 0.0, a1 = 0.0, a2 = 0.0; // por defecto: pasa todo
    };

    // Forma directa II transpuesta (igual que la función bq() del JSFX).
    struct Biquad
    {
        double z1 = 0.0, z2 = 0.0;

        double process(double x, const BiquadCoeffs& c) noexcept
        {
            const double y = c.b0 * x + z1;
            z1 = c.b1 * x - c.a1 * y + z2;
            z2 = c.b2 * x - c.a2 * y;
            return y;
        }

        void reset() noexcept { z1 = z2 = 0.0; }
    };

    static constexpr double monoMakerQ = 0.70710678118654752;

    // Cada cuántas muestras (potencia de 2) se recalculan los coeficientes
    // mientras la frecuencia se está moviendo.
    static constexpr int monoCoeffUpdateInterval = 16;

    void updateMonoMakerCoefficients(double cutoffHz);
    void resetMonoMaker();

    double currentSampleRate = 44100.0;

    BiquadCoeffs monoLowCoeffs, monoHighCoeffs;
    Biquad monoLowFilter[2][2];   // [canal][etapa]
    Biquad monoHighFilter[2][2];  // [canal][etapa]
    float lastMonoCoeffFreq = -1.0f;

    // Suavizado de la frecuencia (20 ms, escala log) y del on/off (10 ms,
    // derivado de si la frecuencia está en el mínimo),
    // mismos tiempos que el JSFX pero por muestra en vez de por bloque.
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> monoFreqSmoothed{ monoFreqDefault };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> monoAmountSmoothed{ 0.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidSideVolumeAudioProcessor)
};