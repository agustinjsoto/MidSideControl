#include "ParamControl.h"
using namespace asaudio;

ParamControl::ParamControl (juce::RangedAudioParameter& p, const juce::String& label)
    : labelText (label),
      param (p),
      attachment (p, [this] (float v) { parameterChanged (v); })
{
    setWantsKeyboardFocus (true);
    setMouseClickGrabsKeyboardFocus (true);
    setTitle (label);
    setDescription (param.getName (64));

    formatValue = [this] (float real)
    {
        auto t = param.getText (param.convertTo0to1 (real), 0);
        auto unit = param.getLabel();
        return unit.isNotEmpty() ? t + " " + unit : t;
    };
    formatEditValue = [this] (float real) { return param.getText (param.convertTo0to1 (real), 0); };

    entry.getText     = [this] { return getValueText(); };
    entry.getEditText = [this] { return formatEditValue ? formatEditValue (getCurrentValue()) : getValueText(); };
    entry.onCommit    = [this] (const juce::String& s)
    {
        // el texto lo interpreta el propio parámetro (L30, -inf, 1.5k, off...)
        const float norm = param.getValueForText (s);
        attachment.setValueAsCompleteGesture (param.convertFrom0to1 (norm));
    };
    addAndMakeVisible (entry);
}

void ParamControl::startAttachment()
{
    attachment.sendInitialUpdate();
}

juce::String ParamControl::getValueText() const
{
    return formatValue ? formatValue (getCurrentValue()) : juce::String (getCurrentValue());
}

void ParamControl::parameterChanged (float realValue)
{
    // Si la posición actual ya representa este valor (salvo errores de
    // redondeo de float), la dejamos como está para que no "tiemble".
    const auto range = param.getNormalisableRange();
    const float fromPos = range.snapToLegalValue (positionToValue (position));

    if (std::abs (param.convertTo0to1 (fromPos) - param.convertTo0to1 (realValue)) > 1.0e-4f)
        position = valueToPosition (realValue);

    repaint();
    entry.repaint();
}

void ParamControl::setPositionByUser (double newPosition, bool partOfGesture)
{
    position = snapPosition (juce::jlimit (0.0, 1.0, newPosition));
    const float real = positionToValue (position);

    if (partOfGesture) attachment.setValueAsPartOfGesture (real);
    else               attachment.setValueAsCompleteGesture (real);

    repaint();
    entry.repaint();
}

void ParamControl::showBubbleNow (bool transient)
{
    if (bubble == nullptr)
        return;

    if (transient) bubble->flash (*this, getBubbleAnchor(), getValueText());
    else           bubble->show  (*this, getBubbleAnchor(), getValueText());
}

//==============================================================================
void ParamControl::mouseDown (const juce::MouseEvent& e)
{
    grabKeyboardFocus();      // selección = foco real; el halo queda hasta que otro control lo tome

    dragging = true;
    attachment.beginGesture();

    dragStartY = e.position.y;
    dragStartPosition = position;
    lastFine = e.mods.isShiftDown();

    onMouseDownBody (e);
    showBubbleNow (false);
    repaint();
}

void ParamControl::mouseDrag (const juce::MouseEvent& e)
{
    if (! dragging)
        return;

    // Shift cambia solo la sensibilidad; al cambiar de modo re-anclamos para que no salte.
    const bool fine = e.mods.isShiftDown();
    if (fine != lastFine)
    {
        lastFine = fine;
        dragStartY = e.position.y;
        dragStartPosition = position;
    }

    const double delta = (double) (dragStartY - e.position.y) / dragPixelsForFullRange (fine);
    setPositionByUser (dragStartPosition + delta, true);
    showBubbleNow (false);
}

void ParamControl::mouseUp (const juce::MouseEvent&)
{
    if (! dragging)
        return;

    dragging = false;
    attachment.endGesture();
    if (bubble != nullptr)
        bubble->hideBubble();
}

void ParamControl::mouseDoubleClick (const juce::MouseEvent&)
{
    attachment.setValueAsCompleteGesture (param.convertFrom0to1 (param.getDefaultValue()));
    position = valueToPosition (param.convertFrom0to1 (param.getDefaultValue()));
    repaint();
    entry.repaint();
    showBubbleNow (true);
}

void ParamControl::nudge (double delta)
{
    setPositionByUser (position + delta, false);
    showBubbleNow (true);
}

void ParamControl::mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails& w)
{
    float amount = std::abs (w.deltaX) > std::abs (w.deltaY) ? -w.deltaX : w.deltaY;
    if (w.isReversed)
        amount = -amount;

    if (amount == 0.0f)
        return;

    nudge ((amount > 0.0f ? 1.0 : -1.0) * wheelStep());
}

bool ParamControl::keyPressed (const juce::KeyPress& k)
{
    int dir = 0;
    if (k.isKeyCode (juce::KeyPress::upKey)   || k.isKeyCode (juce::KeyPress::rightKey)) dir = 1;
    if (k.isKeyCode (juce::KeyPress::downKey) || k.isKeyCode (juce::KeyPress::leftKey))  dir = -1;

    if (dir == 0)
        return false;

    nudge ((double) dir * keyStep (k.getModifiers().isShiftDown()));
    return true;
}

//==============================================================================
// Accesibilidad: valor actual y cambios desde lectores de pantalla.
class ParamAccessibility : public juce::AccessibilityValueInterface
{
public:
    explicit ParamAccessibility (ParamControl& c) : owner (c) {}

    bool isReadOnly() const override { return false; }
    double getCurrentValue() const override { return owner.position; }
    void setValue (double v) override { owner.setPositionByUser (v, false); }
    juce::String getCurrentValueAsString() const override { return owner.getValueText(); }

    void setValueAsString (const juce::String& s) override
    {
        owner.attachment.setValueAsCompleteGesture (owner.param.convertFrom0to1 (owner.param.getValueForText (s)));
    }

    AccessibleValueRange getRange() const override { return { { 0.0, 1.0 }, 0.01 }; }

private:
    ParamControl& owner;
};

std::unique_ptr<juce::AccessibilityHandler> ParamControl::createAccessibilityHandler()
{
    return std::make_unique<juce::AccessibilityHandler> (*this, juce::AccessibilityRole::slider,
                                                         juce::AccessibilityActions(),
                                                         juce::AccessibilityHandler::Interfaces { std::make_unique<ParamAccessibility> (*this) });
}
