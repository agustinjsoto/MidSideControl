#include "ValueBubble.h"
using namespace asaudio;

ValueBubble::ValueBubble()
{
    setInterceptsMouseClicks (false, false);
    setAlwaysOnTop (true);
    setVisible (false);
}

void ValueBubble::setEnabledByUser (bool shouldShow)
{
    enabledByUser = shouldShow;
    if (! shouldShow)
        hideBubble();
}

void ValueBubble::place (juce::Component& target, juce::Rectangle<int> anchorInTarget, const juce::String& newText)
{
    auto* parent = getParentComponent();
    if (parent == nullptr || ! enabledByUser)
        return;

    text = newText;

    const int w = (int) std::ceil (draw::textWidth (fonts::label (15.0f, true, 0.06f), text)) + 18;
    const int h = 22 + arrow;
    const auto anchor = parent->getLocalArea (&target, anchorInTarget);

    int x = anchor.getCentreX() - w / 2;
    x = juce::jlimit (2, juce::jmax (2, parent->getWidth() - w - 2), x);

    setBounds (x, anchor.getY() - h - 3, w, h);

    // la flecha apunta al centro del control aunque el globo se haya desplazado
    arrowX = anchor.getCentreX() - x;
    setVisible (true);
    toFront (false);
    repaint();
}

void ValueBubble::show (juce::Component& target, juce::Rectangle<int> anchorInTarget, const juce::String& t)
{
    stopTimer();
    place (target, anchorInTarget, t);
}

void ValueBubble::flash (juce::Component& target, juce::Rectangle<int> anchorInTarget, const juce::String& t)
{
    place (target, anchorInTarget, t);
    if (enabledByUser)
        startTimer (900);
}

void ValueBubble::hideBubble()
{
    stopTimer();
    setVisible (false);
}

void ValueBubble::paint (juce::Graphics& g)
{
    auto body = getLocalBounds().toFloat().withTrimmedBottom ((float) arrow).reduced (1.0f);

    g.setColour (juce::Colours::white.withAlpha (0.16f));
    g.fillRoundedRectangle (body.expanded (1.0f), 5.0f);
    g.setColour (col::valueBg);
    g.fillRoundedRectangle (body, 4.0f);

    const float ax = juce::jlimit (8.0f, (float) getWidth() - 8.0f, (float) arrowX);
    juce::Path tri;
    tri.addTriangle (ax - arrow, body.getBottom() - 0.5f, ax + arrow, body.getBottom() - 0.5f, ax, body.getBottom() + arrow);
    g.setColour (col::valueBg);
    g.fillPath (tri);

    g.setFont (fonts::label (15.0f, true, 0.06f));
    g.setColour (col::amber);
    g.drawText (text, body, juce::Justification::centred, false);
}
