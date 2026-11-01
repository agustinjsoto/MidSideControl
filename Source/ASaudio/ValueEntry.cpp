#include "ValueEntry.h"
using namespace asaudio;

ValueEntry::ValueEntry()
{
    setMouseCursor (juce::MouseCursor::IBeamCursor);
}

void ValueEntry::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat().withTrimmedBottom (1.0f);

    g.setColour (juce::Colours::white.withAlpha (0.10f));
    g.fillRoundedRectangle (r.translated (0.0f, 1.0f), 3.0f);
    g.setColour (col::valueBg);
    g.fillRoundedRectangle (r, 3.0f);

    {
        juce::Graphics::ScopedSaveState ss (g);
        juce::Path p;
        p.addRoundedRectangle (r, 3.0f);
        g.reduceClipRegion (p);
        g.setGradientFill (juce::ColourGradient (juce::Colours::black, 0.0f, r.getY(),
                                                 juce::Colours::transparentBlack, 0.0f, r.getY() + 4.0f, false));
        g.fillRect (r);
    }

    if (editor == nullptr && getText)
    {
        g.setFont (fonts::label (fontSize, false, fontTracking));
        g.setColour (juce::Colours::white);
        g.drawText (getText(), r.reduced (2.0f, 0.0f), juce::Justification::centred, false);
    }
}

void ValueEntry::resized()
{
    if (editor != nullptr)
        editor->setBounds (getLocalBounds());
}

void ValueEntry::mouseDoubleClick (const juce::MouseEvent&)
{
    beginEdit();
}

void ValueEntry::beginEdit()
{
    if (editor != nullptr || ! getEditText)
        return;

    editor = std::make_unique<juce::TextEditor>();
    editor->setFont (fonts::label (fontSize, false, 0.02f));
    editor->setJustification (juce::Justification::centred);
    editor->setBorder ({ 0, 2, 0, 2 });
    editor->setIndents (0, 0);
    editor->setColour (juce::TextEditor::backgroundColourId, juce::Colours::black);
    editor->setColour (juce::TextEditor::textColourId, juce::Colours::white);
    editor->setColour (juce::TextEditor::outlineColourId, col::amber);
    editor->setColour (juce::TextEditor::focusedOutlineColourId, col::amber);
    editor->setText (getEditText(), false);

    editor->onReturnKey = [this] { finishEdit (true); };
    editor->onEscapeKey = [this] { finishEdit (false); };
    editor->onFocusLost = [this] { finishEdit (true); };

    addAndMakeVisible (*editor);
    editor->setBounds (getLocalBounds());
    editor->grabKeyboardFocus();
    editor->selectAll();
    repaint();
}

void ValueEntry::finishEdit (bool commit)
{
    if (editor == nullptr)
        return;

    const auto text = editor->getText();

    // no destruimos el editor dentro de su propio callback
    juce::MessageManager::callAsync ([safe = juce::Component::SafePointer<ValueEntry> (this), commit, text]
    {
        if (safe == nullptr || safe->editor == nullptr)
            return;

        safe->editor.reset();
        safe->repaint();

        if (commit && safe->onCommit)
            safe->onCommit (text);
    });

    editor->onFocusLost = nullptr;
    editor->onReturnKey = nullptr;
    editor->onEscapeKey = nullptr;
}
