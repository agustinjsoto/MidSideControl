#include "PluginProcessor.h"
#include "PluginEditor.h"

MidSideVolumeAudioProcessorEditor::MidSideVolumeAudioProcessorEditor (MidSideVolumeAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), goniometer (p)
{
    // ---------- Fader Mid ----------
    midSlider.setSliderStyle (juce::Slider::LinearVertical);
    midSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 20);
    addAndMakeVisible (midSlider);

    midLabel.setText ("Mid", juce::dontSendNotification);
    midLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (midLabel);

    // ---------- Fader Side ----------
    sideSlider.setSliderStyle (juce::Slider::LinearVertical);
    sideSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 20);
    addAndMakeVisible (sideSlider);

    sideLabel.setText ("Side", juce::dontSendNotification);
    sideLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (sideLabel);

    // ---------- Knob de volumen de salida ----------
    outputKnob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    outputKnob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 20);
    outputKnob.setRotaryParameters (juce::MathConstants<float>::pi * 1.2f,
                                     juce::MathConstants<float>::pi * 2.8f,
                                     true);
    addAndMakeVisible (outputKnob);

    outputLabel.setText ("Output", juce::dontSendNotification);
    outputLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (outputLabel);

    // ---------- Knob de Pan ----------
    panKnob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    panKnob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 20);
    panKnob.setRotaryParameters (juce::MathConstants<float>::pi * 1.2f,
                                 juce::MathConstants<float>::pi * 2.8f,
                                 true);
    addAndMakeVisible (panKnob);

    panLabel.setText ("Pan", juce::dontSendNotification);
    panLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (panLabel);

    // ---------- Botón de ajustes ----------
    settingsButton.setTooltip ("Settings");
    settingsButton.onClick = [this] { showSettingsMenu(); };
    addAndMakeVisible (settingsButton);

    // ---------- Goniómetro ----------
    addAndMakeVisible(goniometer);

    // ---------- Color de los circulitos (thumb) de los faders y los knobs ----------
    // Para cambiarlo después, solo tocá esta línea.
    const juce::Colour kThumbColour{ 0xff689e75 };

    for (auto* slider : { &midSlider, &sideSlider, &outputKnob, &panKnob })
        slider->setColour(juce::Slider::thumbColourId, kThumbColour);

    // ---------- Attachments: conectan cada control con su parámetro ----------
    midAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        audioProcessor.apvts, MidSideVolumeAudioProcessor::midGainId, midSlider);

    sideAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        audioProcessor.apvts, MidSideVolumeAudioProcessor::sideGainId, sideSlider);

    outputAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        audioProcessor.apvts, MidSideVolumeAudioProcessor::outputVolumeId, outputKnob);

    panAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        audioProcessor.apvts, MidSideVolumeAudioProcessor::panId, panKnob);

    setSize (400, 440);
}

MidSideVolumeAudioProcessorEditor::~MidSideVolumeAudioProcessorEditor() = default;

void MidSideVolumeAudioProcessorEditor::showSettingsMenu()
{
    auto* param = dynamic_cast<juce::AudioParameterChoice*> (
        audioProcessor.apvts.getParameter (MidSideVolumeAudioProcessor::panLawId));

    if (param == nullptr)
        return;

    juce::PopupMenu menu;
    menu.addSectionHeader ("Pan Law");

    // IDs de los items = índice de la opción + 1 (el ID 0 significa "nada elegido").
    const int current = param->getIndex();
    for (int i = 0; i < param->choices.size(); ++i)
        menu.addItem (i + 1, param->choices[i], true, i == current);

    juce::Component::SafePointer<MidSideVolumeAudioProcessorEditor> safeThis (this);

    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (settingsButton),
                        [safeThis, param] (int result)
                        {
                            if (result <= 0 || safeThis == nullptr)
                                return;

                            // beginChangeGesture/endChangeGesture le avisan al DAW
                            // que el cambio lo hizo el usuario (para automatización/undo).
                            param->beginChangeGesture();
                            param->setValueNotifyingHost (param->convertTo0to1 ((float) (result - 1)));
                            param->endChangeGesture();
                        });
}

void MidSideVolumeAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (20.0f, juce::Font::bold));
    g.drawFittedText ("Mid-Side Control", getLocalBounds().removeFromTop (40),
                       juce::Justification::centred, 1);
}

void MidSideVolumeAudioProcessorEditor::resized()
{
    // Botón de ajustes arriba a la derecha, en la franja del título.
    settingsButton.setBounds (getWidth() - 20 - 24, 10, 24, 24);

    auto area = getLocalBounds().reduced (20);
    area.removeFromTop (40); // deja lugar para el título dibujado en paint()

    // Columna izquierda (ancho fijo) para los 2 faders y el knob de pan abajo.
    // Lo que sobra a la derecha es la columna del knob de output, con el
    // goniómetro arriba de él.
    auto fadersArea = area.removeFromLeft (150);
    auto knobArea   = area;

    const int labelHeight = 20;

    // --- Knob de Pan (abajo de los faders, mismo alto/posición que el de Output) ---
    // 130 = 90 (knob) + 20 (label) + 10 + 10 de márgenes.
    auto panArea = fadersArea.removeFromBottom (130).reduced (10);
    panLabel.setBounds (panArea.removeFromBottom (labelHeight));
    const int panKnobSize = juce::jmin (panArea.getWidth(), 90);
    panKnob.setBounds (panArea.removeFromBottom (panKnobSize).withSizeKeepingCentre (panKnobSize, panKnobSize));

    // --- Faders ---
    auto midArea  = fadersArea.removeFromLeft (fadersArea.getWidth() / 2).reduced (10, 0);
    auto sideArea = fadersArea.reduced (10, 0);

    midLabel.setBounds (midArea.removeFromBottom (labelHeight));
    midSlider.setBounds (midArea);

    sideLabel.setBounds (sideArea.removeFromBottom (labelHeight));
    sideSlider.setBounds (sideArea);

    // --- Knob de Output (abajo de la columna derecha) ---
    knobArea = knobArea.reduced(10);
    outputLabel.setBounds(knobArea.removeFromBottom(labelHeight));

    const int knobSize = juce::jmin(knobArea.getWidth(), 90);
    // Lo tomamos desde abajo (justo pegado al label).
    outputKnob.setBounds(knobArea.removeFromBottom(knobSize).withSizeKeepingCentre(knobSize, knobSize));

    // --- Goniómetro: ocupa el resto del espacio que queda arriba del knob ---
    knobArea.removeFromBottom (10); // separación entre el goniómetro y el knob
    goniometer.setBounds (knobArea);
}
