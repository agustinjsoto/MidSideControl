#pragma once
#include "ASaudioLookAndFeel.h"
#include "Plate.h"
#include "GearButton.h"
#include "SegmentedSelector.h"
#include "Dropdown.h"
#include "Checkbox.h"
#include "PushButton.h"

// Modal de 480 px centrado sobre el panel, con velo oscuro (~60 %).
// Ocupa todo el padre; Esc, la cruz, Close o un clic en el velo lo cierran.
//
// Filas: Window size, Scope style, Theme, Pan law (dropdown, parámetro del
// plugin), Value bubbles. Los ajustes que no son de DSP los guarda el editor.
class SettingsPanel : public juce::Component
{
public:
    // panLawParam: parámetro de tipo AudioParameterChoice (índice = valor real).
    explicit SettingsPanel (juce::AudioParameterChoice& panLawParam);
    ~SettingsPanel() override;

    void open();
    void close();

    // Valores iniciales (sin disparar callbacks)
    void setWindowSizeIndex (int i)   { sizeSel.setSelectedIndex (i); }
    void setScopeStyleIndex (int i)   { scopeSel.setSelectedIndex (i); }
    void setThemeIndex (int i)        { themeSel.setSelectedIndex (i); }
    void setBubbles (bool on)         { bubblesBox.setToggleState (on, juce::dontSendNotification); }

    std::function<void (int)>  onWindowSize;    // 0..3 = 75 / 100 / 125 / 150 %
    std::function<void (int)>  onScopeStyle;    // 0 = INK, 1 = PHOSPHOR
    std::function<void (int)>  onTheme;         // 0 = DEFAULT, 1 = BLACK
    std::function<void (bool)> onBubbles;

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;
    bool keyPressed (const juce::KeyPress&) override;

    static constexpr int modalWidth = 480;

private:
    juce::Rectangle<int> getModalBounds() const;

    juce::AudioParameterChoice& panLaw;
    juce::ParameterAttachment panLawAttachment;

    Plate plate { "Settings" };
    GearButton closeX { GearButton::Glyph::close };

    SegmentedSelector sizeSel  { { "75%", "100%", "125%", "150%" }, 200 };
    SegmentedSelector scopeSel { { "INK", "PHOSPHOR" }, 200 };
    SegmentedSelector themeSel { { "DEFAULT", "BLACK" }, 200 };
    Dropdown panLawDrop;
    Checkbox bubblesBox { "Value bubbles" };
    PushButton closeButton { "Close" };

    static constexpr int rowH = 44;
    static constexpr int rows = 5;
};
