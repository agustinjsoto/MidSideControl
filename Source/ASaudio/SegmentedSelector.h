#pragma once
#include "ASaudioLookAndFeel.h"

// Pozo oscuro con teclas iguales; la activa en ámbar con halo. Selección exclusiva.
class SegmentedSelector : public juce::Component
{
public:
    SegmentedSelector (const juce::StringArray& items, int width = 140);

    void setSelectedIndex (int index, bool notify = false);
    int getSelectedIndex() const noexcept { return selected; }

    std::function<void (int)> onChange;

    static constexpr int height = 22;

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    bool keyPressed (const juce::KeyPress&) override;
    void focusGained (FocusChangeType) override { repaint(); }
    void focusLost (FocusChangeType) override   { repaint(); }

private:
    juce::Rectangle<float> keyBounds (int i) const;

    juce::StringArray items;
    int selected = 0;
};
