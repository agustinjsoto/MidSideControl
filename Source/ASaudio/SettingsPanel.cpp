#include "SettingsPanel.h"
using namespace asaudio;

SettingsPanel::SettingsPanel (juce::AudioParameterChoice& p)
    : panLaw (p),
      panLawAttachment (p, [this] (float v) { panLawDrop.setSelectedIndex ((int) std::lround (v)); }),
      panLawDrop (p.choices, 200)
{
    setWantsKeyboardFocus (true);
    setVisible (false);
    setTitle ("Settings");

    addAndMakeVisible (plate);
    addAndMakeVisible (closeX);
    for (auto* c : std::initializer_list<juce::Component*> { &sizeSel, &scopeSel, &themeSel, &panLawDrop, &bubblesBox, &closeButton })
        addAndMakeVisible (c);

    panLawDrop.setListHost (this);
    panLawDrop.onChange = [this] (int i) { panLawAttachment.setValueAsCompleteGesture ((float) i); };
    panLawAttachment.sendInitialUpdate();

    sizeSel.onChange  = [this] (int i) { if (onWindowSize) onWindowSize (i); };
    scopeSel.onChange = [this] (int i) { if (onScopeStyle) onScopeStyle (i); };
    themeSel.onChange = [this] (int i) { if (onTheme) onTheme (i); };
    bubblesBox.onClick = [this] { if (onBubbles) onBubbles (bubblesBox.getToggleState()); };

    closeX.onClick = closeButton.onClick = [this] { close(); };

    sizeSel.setTitle ("Window size");
    scopeSel.setTitle ("Scope style");
    themeSel.setTitle ("Theme");
    panLawDrop.setTitle ("Pan law");
}

SettingsPanel::~SettingsPanel() = default;

void SettingsPanel::open()
{
    setBounds (getParentComponent()->getLocalBounds());
    setVisible (true);
    toFront (true);
    grabKeyboardFocus();
}

void SettingsPanel::close()
{
    panLawDrop.closeList();
    setVisible (false);
}

juce::Rectangle<int> SettingsPanel::getModalBounds() const
{
    const int h = 34 + rowH * rows + 16 + PushButton::height + 22;
    return { (getWidth() - modalWidth) / 2, 110, modalWidth, h };
}

void SettingsPanel::resized()
{
    const auto m = getModalBounds();

    plate.setTopLeftPosition (m.getCentreX() - plate.getWidth() / 2, m.getY() - 10);
    closeX.setTopLeftPosition (m.getRight() - 12 - GearButton::size, m.getY() + 10);

    int y = m.getY() + 34;
    const int right = m.getRight() - 26;

    auto place = [&] (juce::Component& c, int row)
    {
        c.setTopLeftPosition (right - c.getWidth(), y + row * rowH + (rowH - c.getHeight()) / 2);
    };
    place (sizeSel, 0); place (scopeSel, 1); place (themeSel, 2); place (panLawDrop, 3); place (bubblesBox, 4);

    closeButton.setTopLeftPosition (right - PushButton::width, y + rows * rowH + 16);
}

void SettingsPanel::paint (juce::Graphics& g)
{
    const auto& theme = themeOf (*this);

    g.fillAll (juce::Colour::fromFloatRGBA (0.03f, 0.047f, 0.039f, 0.6f));    // velo

    const auto m = getModalBounds().toFloat();
    juce::Path p;
    p.addRoundedRectangle (m, 10.0f);
    juce::DropShadow (juce::Colours::black.withAlpha (0.6f), 24, { 0, 10 }).drawForPath (g, p);

    g.setGradientFill (juce::ColourGradient (theme.panelA, 0.0f, m.getY(), theme.panelB, 0.0f, m.getBottom(), false));
    g.fillPath (p);
    g.setColour (theme.panelEdge);
    g.drawRoundedRectangle (m.reduced (1.0f), 9.0f, 2.0f);

    static const char* names[rows] = { "Window size", "Scope style", "Theme", "Pan law", "Value bubbles" };
    const int top = (int) m.getY() + 34;

    for (int i = 0; i < rows; ++i)
    {
        const float y = (float) (top + i * rowH);
        draw::labelText (g, names[i], { m.getX() + 26.0f, y, 220.0f, (float) rowH }, juce::Justification::centredLeft,
                         fonts::label (15.0f), theme.ink);

        g.setColour (juce::Colours::black.withAlpha (0.22f));
        g.fillRect (m.getX() + 26.0f, y + (float) rowH - 1.0f, m.getWidth() - 52.0f, 1.0f);
    }
}

void SettingsPanel::mouseDown (const juce::MouseEvent& e)
{
    if (panLawDrop.isListOpen())
        panLawDrop.closeList();

    if (! getModalBounds().contains (e.getPosition()))
        close();
}

bool SettingsPanel::keyPressed (const juce::KeyPress& k)
{
    if (k.isKeyCode (juce::KeyPress::escapeKey))
    {
        if (panLawDrop.isListOpen()) panLawDrop.closeList();
        else close();
        return true;
    }
    return false;
}
