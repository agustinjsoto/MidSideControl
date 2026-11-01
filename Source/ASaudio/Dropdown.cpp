#include "Dropdown.h"
using namespace asaudio;

//==============================================================================
class Dropdown::List : public juce::Component
{
public:
    List (Dropdown& o) : owner (o)
    {
        setWantsKeyboardFocus (true);
        setSize (o.getWidth(), 6 + rowH * o.items.size());
    }

    void paint (juce::Graphics& g) override
    {
        const auto r = getLocalBounds().toFloat();

        juce::Path shadow;
        shadow.addRoundedRectangle (r.reduced (1.0f), 5.0f);
        juce::DropShadow (juce::Colours::black.withAlpha (0.6f), 8, { 0, 4 }).drawForPath (g, shadow);

        g.setColour (juce::Colours::white.withAlpha (0.16f));
        g.fillRoundedRectangle (r, 5.0f);
        g.setColour (col::valueBg);
        g.fillRoundedRectangle (r.reduced (1.0f), 5.0f);

        const auto font = fonts::label (14.0f, true, 0.1f);
        for (int i = 0; i < owner.items.size(); ++i)
        {
            const auto row = rowBounds (i);

            if (i == hover)
            {
                g.setColour (juce::Colour (0xff2a2f2d));
                g.fillRoundedRectangle (row, 3.0f);
            }

            const auto textArea = row.withTrimmedLeft (10.0f);
            if (i == owner.selected) draw::glowText (g, owner.items[i], textArea, juce::Justification::centredLeft, font, col::amber);
            else { g.setFont (font); g.setColour (themeOf (*this).inkDim); g.drawText (owner.items[i], textArea, juce::Justification::centredLeft, false); }
        }
    }

    void mouseMove (const juce::MouseEvent& e) override  { setHover (indexAt (e.position)); }
    void mouseExit (const juce::MouseEvent&) override    { setHover (-1); }
    void mouseDown (const juce::MouseEvent& e) override  { const int i = indexAt (e.position); if (i >= 0) pick (i); }

    bool keyPressed (const juce::KeyPress& k) override
    {
        if (k.isKeyCode (juce::KeyPress::escapeKey)) { owner.closeList(); return true; }
        if (k.isKeyCode (juce::KeyPress::downKey))   { setHover (juce::jmin (owner.items.size() - 1, hover + 1)); return true; }
        if (k.isKeyCode (juce::KeyPress::upKey))     { setHover (juce::jmax (0, hover - 1)); return true; }
        if (k.isKeyCode (juce::KeyPress::returnKey) || k.isKeyCode (juce::KeyPress::spaceKey))
        {
            if (hover >= 0) pick (hover);
            return true;
        }
        return false;
    }

    void focusLost (FocusChangeType) override
    {
        juce::MessageManager::callAsync ([safe = juce::Component::SafePointer<Dropdown> (&owner)]
        {
            if (safe != nullptr)
                safe->closeList();
        });
    }

    void setHover (int i) { if (i != hover) { hover = i; repaint(); } }
    int hover = -1;

private:
    static constexpr int rowH = 24;

    juce::Rectangle<float> rowBounds (int i) const { return { 3.0f, 3.0f + (float) (i * rowH), (float) getWidth() - 6.0f, (float) rowH }; }
    int indexAt (juce::Point<float> p) const { for (int i = 0; i < owner.items.size(); ++i) if (rowBounds (i).contains (p)) return i; return -1; }

    void pick (int i)
    {
        auto& o = owner;
        o.setSelectedIndex (i, true);
        o.closeList();
    }

    Dropdown& owner;
};

//==============================================================================
Dropdown::Dropdown (const juce::StringArray& i, int w) : items (i)
{
    setSize (w, height);
    setWantsKeyboardFocus (true);
    setTitle ("Dropdown");
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
}

Dropdown::~Dropdown() = default;

void Dropdown::setSelectedIndex (int index, bool notify)
{
    index = juce::jlimit (0, items.size() - 1, index);
    if (index == selected)
        return;

    selected = index;
    repaint();
    if (notify && onChange)
        onChange (selected);
}

void Dropdown::openList()
{
    if (listHost == nullptr || list != nullptr)
        return;

    list = std::make_unique<List> (*this);
    listHost->addAndMakeVisible (*list);

    const auto p = listHost->getLocalPoint (this, juce::Point<int> (0, height + 4));
    list->setTopLeftPosition (p);
    list->hover = selected;
    list->toFront (true);
    list->grabKeyboardFocus();
}

void Dropdown::closeList()
{
    if (list == nullptr)
        return;

    list->setVisible (false);
    if (listHost != nullptr)
        listHost->removeChildComponent (list.get());
    list.reset();
    repaint();
}

void Dropdown::mouseDown (const juce::MouseEvent&)
{
    grabKeyboardFocus();
    if (isListOpen()) closeList(); else openList();
}

bool Dropdown::keyPressed (const juce::KeyPress& k)
{
    if (k.isKeyCode (juce::KeyPress::returnKey) || k.isKeyCode (juce::KeyPress::spaceKey) || k.isKeyCode (juce::KeyPress::downKey))
    {
        openList();
        return true;
    }
    return false;
}

void Dropdown::paint (juce::Graphics& g)
{
    const auto r = getLocalBounds().toFloat().withTrimmedBottom (1.0f);
    draw::well (g, r, 5.0f);

    g.setFont (fonts::label (14.0f, true, 0.1f));
    g.setColour (themeOf (*this).ink);
    g.drawText (items[selected], r.withTrimmedLeft (10.0f), juce::Justification::centredLeft, false);

    juce::Path arrow;
    const float ax = r.getRight() - 15.0f, ay = r.getCentreY() - 2.5f;
    arrow.addTriangle (ax - 5.0f, ay, ax + 5.0f, ay, ax, ay + 5.0f);
    g.setColour (col::amber);
    g.fillPath (arrow);

    if (hasKeyboardFocus (true))
    {
        g.setColour (col::amber);
        g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (1.0f), 5.0f, 2.0f);
    }
}
