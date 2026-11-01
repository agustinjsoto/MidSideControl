#include "PluginPanel.h"
using namespace asaudio;

namespace
{
    using P = MidSideVolumeAudioProcessor;

    juce::RangedAudioParameter& param(P& p, const char* id)
    {
        return *p.apvts.getParameter(id);
    }

    juce::String dbText(float v)
    {
        if (v <= P::minusInfinityDb + 0.01f)
            return juce::String::fromUTF8("-\xe2\x88\x9e");
        return juce::String(v > 0.0f ? "+" : "") + juce::String(v, 1) + " dB";
    }

    juce::String panText(float v)
    {
        const int pct = (int)std::lround(std::abs(v) * 100.0f);
        if (pct == 0)
            return "C";
        return juce::String(v < 0.0f ? "L " : "R ") + juce::String(pct);
    }

    constexpr float zoomValues[] = { 0.75f, 1.0f, 1.25f, 1.5f };
}

PluginPanel::PluginPanel(MidSideVolumeAudioProcessor& p, UiSettings& s, ASaudioLookAndFeel& lf, std::function<void(float)> zoomChanged)
    : processor(p), settings(s), laf(lf), onZoomChanged(std::move(zoomChanged)),
    midFader(param(p, P::midGainId), "Mid", Fader::Scale::reaperPlus24()),
    sideFader(param(p, P::sideGainId), "Side", Fader::Scale::reaperPlus24()),
    panKnob(param(p, P::panId), "Pan"),
    monoKnob(param(p, P::monoFreqId), "Mono Maker"),
    outputKnob(param(p, P::outputVolumeId), "Output"),
    settingsPanel(*dynamic_cast<juce::AudioParameterChoice*> (p.apvts.getParameter(P::panLawId)))
{
    setOpaque(false);
    setTitle("Mid-Side Control");

    for (auto* c : std::initializer_list<juce::Component*>{ &frameMidSide, &frameScope, &frameOutput })
        addAndMakeVisible(c);

    for (auto* c : std::initializer_list<juce::Component*>{ &midFader, &sideFader, &gonio, &correlation,
                                                              &panKnob, &monoKnob, &outputKnob, &gearButton })
        addAndMakeVisible(c);

    addChildComponent(bubble);
    addChildComponent(settingsPanel);

    // Formato de los valores
    midFader.formatValue = sideFader.formatValue = outputKnob.formatValue = dbText;
    panKnob.formatValue = panText;

    for (auto* c : std::initializer_list<ParamControl*>{ &midFader, &sideFader, &panKnob, &monoKnob, &outputKnob })
        c->setBubble(&bubble);

    // Ajustes de interfaz guardados en el estado del plugin
    {
        const auto theme = settings.getTheme();
        const float zoom = settings.getZoom();
        int zi = 1;
        for (int i = 0; i < 4; ++i)
            if (std::abs(zoomValues[i] - zoom) < 0.01f) zi = i;

        settingsPanel.setThemeIndex(theme.equalsIgnoreCase("black") ? 1 : 0);
        settingsPanel.setScopeStyleIndex(settings.getScopeStyle().equalsIgnoreCase("phosphor") ? 1 : 0);
        settingsPanel.setWindowSizeIndex(zi);
        settingsPanel.setBubbles(settings.getBubbles());

        gonio.setStyle(settings.getScopeStyle().equalsIgnoreCase("phosphor") ? Goniometer::Style::phosphor : Goniometer::Style::ink);
        bubble.setEnabledByUser(settings.getBubbles());
    }

    settingsPanel.onTheme = [this](int i) { applyTheme(i == 1 ? "black" : "default"); };
    settingsPanel.onScopeStyle = [this](int i)
        {
            settings.setScopeStyle(i == 1 ? "phosphor" : "ink");
            gonio.setStyle(i == 1 ? Goniometer::Style::phosphor : Goniometer::Style::ink);
        };
    settingsPanel.onWindowSize = [this](int i)
        {
            settings.setZoom(zoomValues[juce::jlimit(0, 3, i)]);
            if (onZoomChanged)
                onZoomChanged(zoomValues[juce::jlimit(0, 3, i)]);
        };
    settingsPanel.onBubbles = [this](bool on)
        {
            settings.setBubbles(on);
            bubble.setEnabledByUser(on);
        };

    gearButton.onClick = [this] { settingsPanel.open(); };

    lastReadIndex = processor.getGoniometerWriteIndex();
    setSize(baseWidth, baseHeight);
    startTimerHz(30);
}

PluginPanel::~PluginPanel()
{
    stopTimer();
}

void PluginPanel::applyTheme(const juce::String& name)
{
    settings.setTheme(name);
    laf.setTheme(Theme::fromName(name));
    sendLookAndFeelChange();
}

//==============================================================================
void PluginPanel::resized()
{
    gearButton.setTopLeftPosition(baseWidth - 44 - GearButton::size, (58 - GearButton::size) / 2);

    frameMidSide.setFrameBounds({ 24, 74, 190, 334 });
    frameScope.setFrameBounds({ 234, 74, 302, 334 });
    frameOutput.setFrameBounds({ 24, 436, 512, 188 });

    midFader.setTopLeftPosition(40, 90);
    sideFader.setTopLeftPosition(135, 90);

    gonio.setTopLeftPosition(265, 93);
    correlation.setTopLeftPosition(265, 93 + Goniometer::height + 14);

    panKnob.setTopLeftPosition(44, 452);
    monoKnob.setTopLeftPosition(208, 452);
    outputKnob.setTopLeftPosition(372, 452);

    if (settingsPanel.isVisible())
        settingsPanel.setBounds(getLocalBounds());
}

void PluginPanel::paintStatic(juce::Graphics& g) const
{
    const auto& theme = themeOf(*this);
    const auto bounds = juce::Rectangle<float>(0.0f, 0.0f, (float)baseWidth, (float)baseHeight);

    draw::panel(g, bounds, theme);

    // línea del encabezado
    g.setColour(theme.frameBorder);
    g.fillRect(0.0f, 58.0f, (float)baseWidth, 2.0f);
    g.setColour(juce::Colours::white.withAlpha(0.07f));
    g.fillRect(0.0f, 60.0f, (float)baseWidth, 1.0f);

    // marca + nombre del plugin
    const auto brandFont = fonts::brand(27.0f);
    draw::labelText(g, "ASaudio", { 44.0f, 0.0f, 200.0f, 58.0f }, juce::Justification::centredLeft,
        brandFont, col::brassA, 0.6f);

    // El nombre se centra en el hueco libre entre el logo y el engranaje (y no en
    // el panel entero, que lo deja pegado al logo). Para centrarlo en todo el
    // panel, poné titleCentreX = baseWidth * 0.5f.
    const float brandRight = 44.0f + draw::textWidth(brandFont, "ASaudio");
    const float gearLeft = (float)(baseWidth - 44 - GearButton::size);
    const float titleCentreX = (brandRight + gearLeft) * 0.5f;

    // el tracking agrega espacio también después de la última letra: lo compensamos
    constexpr float titleSize = 27.0f, titleTracking = 0.14f;
    const float trailing = titleSize * titleTracking;

    draw::labelText(g, "Mid-Side Control",
        { titleCentreX - 200.0f + trailing * 0.5f, 0.0f, 400.0f, 58.0f }, juce::Justification::centred,
        fonts::label(titleSize, true, titleTracking), theme.inkDim, 0.5f);
}

void PluginPanel::paint(juce::Graphics& g)
{
    staticLayer.draw(g, baseWidth, baseHeight, [this](juce::Graphics& ig) { paintStatic(ig); });
}

//==============================================================================
void PluginPanel::timerCallback()
{
    // Un único timer: alimenta el goniómetro y la correlación con lo que
    // el procesador va dejando (atómicos y buffer circular, sin locks).
    constexpr int N = P::goniometerBufferSize;
    const auto& mid = processor.getGoniometerMidBuffer();
    const auto& side = processor.getGoniometerSideBuffer();

    const int current = processor.getGoniometerWriteIndex();
    int numNew = current - lastReadIndex;
    if (numNew < 0)
        numNew += N;
    numNew = juce::jmin(numNew, N);

    // auto-gain: el procesador sigue el pico; acá solo se lee
    const float gain = (Goniometer::scopeRadius * 0.9f) / processor.getDisplayPeakLevel();

    gonio.beginFrame();

    int idx = lastReadIndex;
    for (int remaining = numNew; remaining > 0;)
    {
        const int chunk = juce::jmin(remaining, N - idx);
        gonio.addSamples(mid.data() + idx, side.data() + idx, chunk, gain);
        idx = (idx + chunk) % N;
        remaining -= chunk;
    }

    gonio.endFrame();
    lastReadIndex = current;

    correlation.setValue(processor.getCorrelation());
}