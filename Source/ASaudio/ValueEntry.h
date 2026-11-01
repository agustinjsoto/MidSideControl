#pragma once
#include "ASaudioLookAndFeel.h"

// Caja oscura con el valor bajo cada control. Doble clic = campo de texto;
// Enter confirma, Esc cancela, perder el foco confirma.
class ValueEntry : public juce::Component
{
public:
    ValueEntry();

    std::function<juce::String()> getText;                    // texto mostrado
    std::function<juce::String()> getEditText;                // texto inicial al editar
    std::function<void (const juce::String&)> onCommit;       // texto escrito por la persona

    void setTextStyle (float sizePx, float tracking) { fontSize = sizePx; fontTracking = tracking; repaint(); }

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDoubleClick (const juce::MouseEvent&) override;

private:
    void beginEdit();
    void finishEdit (bool commit);

    std::unique_ptr<juce::TextEditor> editor;
    float fontSize = 15.0f, fontTracking = 0.06f;
};
