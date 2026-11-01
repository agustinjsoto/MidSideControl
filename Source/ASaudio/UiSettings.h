#pragma once
#include <JuceHeader.h>

// Ajustes de interfaz que no son de DSP (tema, tamaño de ventana, estilo del
// scope, burbujas). Viven en un ValueTree hijo ("UI") del estado del plugin,
// así se guardan y restauran junto con los parámetros, pero no son
// automatizables.
class UiSettings
{
public:
    explicit UiSettings (juce::ValueTree& pluginState) : root (pluginState) {}

    juce::String getTheme() const          { return get ("theme", "default").toString(); }
    float        getZoom() const           { return juce::jlimit (0.5f, 2.0f, (float) get ("zoom", 1.0).operator double()); }
    juce::String getScopeStyle() const     { return get ("scope", "phosphor").toString(); }
    bool         getBubbles() const        { return (bool) get ("bubbles", true); }

    void setTheme (const juce::String& v)      { set ("theme", v); }
    void setZoom (float v)                     { set ("zoom", (double) v); }
    void setScopeStyle (const juce::String& v) { set ("scope", v); }
    void setBubbles (bool v)                   { set ("bubbles", v); }

private:
    juce::ValueTree node() const { return root.getOrCreateChildWithName ("UI", nullptr); }
    juce::var get (const char* key, const juce::var& def) const { return node().getProperty (key, def); }
    void set (const char* key, const juce::var& v)               { node().setProperty (key, v, nullptr); }

    juce::ValueTree& root;
};
