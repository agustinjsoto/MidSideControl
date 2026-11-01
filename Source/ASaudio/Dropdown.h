#pragma once
#include "ASaudioLookAndFeel.h"

// Pozo oscuro de 26 px con flecha ámbar. La lista flotante se agrega a
// "listHost" (un componente grande que contenga al Dropdown, p. ej. el
// SettingsPanel) y marca el ítem activo en ámbar.
class Dropdown : public juce::Component
{
public:
    Dropdown (const juce::StringArray& items, int width = 200);
    ~Dropdown() override;

    void setListHost (juce::Component* host) noexcept { listHost = host; }

    void setSelectedIndex (int index, bool notify = false);
    int getSelectedIndex() const noexcept { return selected; }

    std::function<void (int)> onChange;

    void closeList();
    bool isListOpen() const noexcept { return list != nullptr; }

    static constexpr int height = 26;

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    bool keyPressed (const juce::KeyPress&) override;

private:
    class List;
    void openList();

    juce::StringArray items;
    int selected = 0;
    juce::Component* listHost = nullptr;
    std::unique_ptr<List> list;
};
