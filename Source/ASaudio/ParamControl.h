#pragma once
#include "ASaudioLookAndFeel.h"
#include "ValueBubble.h"
#include "ValueEntry.h"

// Base de los controles de parámetro (Knob, BipolarKnob, Fader).
//
// Es un Component propio (no un juce::Slider) conectado con un
// juce::ParameterAttachment: así controlamos el gesto completo (arrastre
// vertical, Shift = fino, rueda, flechas, doble clic = default) y los
// begin/endGesture hacia el host salen bien igual.
//
// El modelo interno es una "posición" 0..1 en pantalla; cada subclase define
// cómo se convierte a valor real del parámetro.
class ParamControl : public juce::Component
{
public:
    ParamControl (juce::RangedAudioParameter& parameter, const juce::String& labelText);
    ~ParamControl() override = default;

    void setBubble (ValueBubble* b) noexcept { bubble = b; }

    // Formato del valor mostrado y del texto que aparece al editar.
    std::function<juce::String (float realValue)> formatValue;
    std::function<juce::String (float realValue)> formatEditValue;

    const juce::String& getLabelText() const noexcept { return labelText; }
    juce::RangedAudioParameter& getParameter() const noexcept { return param; }

    // Valor real actual del parámetro.
    float getCurrentValue() const { return param.convertFrom0to1 (param.getValue()); }
    juce::String getValueText() const;

protected:
    // --- a definir / sobreescribir por cada control ---
    virtual double valueToPosition (float realValue) const { return param.convertTo0to1 (realValue); }
    virtual float  positionToValue (double newPosition) const { return param.convertFrom0to1 ((float) newPosition); }
    virtual double snapPosition (double p) const           { return p; }          // detente
    virtual double dragPixelsForFullRange (bool fine) const = 0;
    virtual double wheelStep() const                       { return 0.02; }
    virtual double keyStep (bool shift) const              { juce::ignoreUnused (shift); return 0.02; }
    virtual juce::Rectangle<int> getBubbleAnchor() const = 0;
    virtual void onMouseDownBody (const juce::MouseEvent&) {}                     // Fader: salto a la posición

    // Llamar al final del constructor de la subclase.
    void startAttachment();

    double getPosition() const noexcept { return position; }
    void setPositionByUser (double newPosition, bool partOfGesture);

    ValueEntry entry;
    juce::String labelText;

    // --- Component ---
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseDoubleClick (const juce::MouseEvent&) override;
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;
    bool keyPressed (const juce::KeyPress&) override;
    void focusGained (FocusChangeType) override { repaint(); }
    void focusLost (FocusChangeType) override   { repaint(); }
    std::unique_ptr<juce::AccessibilityHandler> createAccessibilityHandler() override;

    bool hasSelectionFocus() const { return hasKeyboardFocus (false); }

private:
    void parameterChanged (float realValue);
    void nudge (double delta);
    void showBubbleNow (bool transient);

    juce::RangedAudioParameter& param;
    juce::ParameterAttachment attachment;
    ValueBubble* bubble = nullptr;

    double position = 0.0;
    bool dragging = false, lastFine = false;
    float dragStartY = 0.0f;
    double dragStartPosition = 0.0;

    friend class ParamAccessibility;
};
