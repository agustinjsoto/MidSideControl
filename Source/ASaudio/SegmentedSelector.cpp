#include "SegmentedSelector.h"
using namespace asaudio;

SegmentedSelector::SegmentedSelector (const juce::StringArray& i, int w) : items (i)
{
    setSize (w, height);
    setWantsKeyboardFocus (true);
    setTitle ("Selector");
}

juce::Rectangle<float> SegmentedSelector::keyBounds (int i) const
{
    const float pad = 2.0f, gap = 4.0f;
    const float n = (float) items.size();
    const float w = ((float) getWidth() - pad * 2.0f - gap * (n - 1.0f)) / n;
    return { pad + (float) i * (w + gap), pad, w, (float) getHeight() - pad * 2.0f };
}

void SegmentedSelector::setSelectedIndex (int index, bool notify)
{
    index = juce::jlimit (0, items.size() - 1, index);
    if (index == selected)
        return;

    selected = index;
    repaint();
    if (notify && onChange)
        onChange (selected);
}

void SegmentedSelector::mouseDown (const juce::MouseEvent& e)
{
    for (int i = 0; i < items.size(); ++i)
        if (keyBounds (i).expanded (2.0f, 2.0f).contains (e.position))
            setSelectedIndex (i, true);
}

bool SegmentedSelector::keyPressed (const juce::KeyPress& k)
{
    if (k.isKeyCode (juce::KeyPress::leftKey)  || k.isKeyCode (juce::KeyPress::upKey))   { setSelectedIndex (selected - 1, true); return true; }
    if (k.isKeyCode (juce::KeyPress::rightKey) || k.isKeyCode (juce::KeyPress::downKey)) { setSelectedIndex (selected + 1, true); return true; }
    return false;
}

void SegmentedSelector::paint (juce::Graphics& g)
{
    const auto& theme = themeOf (*this);
    draw::well (g, getLocalBounds().toFloat().withTrimmedBottom (1.0f), 5.0f);

    for (int i = 0; i < items.size(); ++i)
    {
        const auto r = keyBounds (i);
        const bool on = i == selected;

        draw::keyCap (g, r, 3.0f, on);

        const auto font = fonts::label (13.0f);
        if (on) draw::glowText (g, items[i], r, juce::Justification::centred, font, col::amber);
        else    { g.setFont (font); g.setColour (theme.inkDim); g.drawText (items[i], r, juce::Justification::centred, false); }
    }

    if (hasKeyboardFocus (true))
    {
        g.setColour (col::amber);
        g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (1.0f), 5.0f, 2.0f);
    }
}
