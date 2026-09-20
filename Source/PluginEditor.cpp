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

    // ---------- Goniómetro ----------
    addAndMakeVisible(goniometer);

    // ---------- Color de los circulitos (thumb) de los faders y el knob ----------
    // Para cambiarlo después, solo tocá esta línea.
    const juce::Colour kThumbColour{ 0xff689e75 };

    for (auto* slider : { &midSlider, &sideSlider, &outputKnob })
        slider->setColour(juce::Slider::thumbColourId, kThumbColour);

    // ---------- Attachments: conectan cada control con su parámetro ----------
    midAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        audioProcessor.apvts, MidSideVolumeAudioProcessor::midGainId, midSlider);

    sideAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        audioProcessor.apvts, MidSideVolumeAudioProcessor::sideGainId, sideSlider);

    outputAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        audioProcessor.apvts, MidSideVolumeAudioProcessor::outputVolumeId, outputKnob);

    setSize (400, 440);
}

MidSideVolumeAudioProcessorEditor::~MidSideVolumeAudioProcessorEditor() = default;

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
    auto area = getLocalBounds().reduced (20);
    area.removeFromTop (40); // deja lugar para el título dibujado en paint()

    // Columna izquierda (ancho fijo) para los 2 faders. Lo que sobra a la
    // derecha es la columna del knob, con el goniómetro arriba de él.
    auto fadersArea = area.removeFromLeft (150);
    auto knobArea   = area;

    // --- Faders ---
    const int labelHeight = 20;

    auto midArea  = fadersArea.removeFromLeft (fadersArea.getWidth() / 2).reduced (10, 0);
    auto sideArea = fadersArea.reduced (10, 0);

    midLabel.setBounds (midArea.removeFromBottom (labelHeight));
    midSlider.setBounds (midArea);

    sideLabel.setBounds (sideArea.removeFromBottom (labelHeight));
    sideSlider.setBounds (sideArea);

    // --- Knob (abajo de la columna derecha) ---
    knobArea = knobArea.reduced(10);
    outputLabel.setBounds(knobArea.removeFromBottom(labelHeight));

    const int knobSize = juce::jmin(knobArea.getWidth(), 90);
    // Lo tomamos desde abajo (justo pegado al label).
    outputKnob.setBounds(knobArea.removeFromBottom(knobSize).withSizeKeepingCentre(knobSize, knobSize));

    // --- Goniómetro: ocupa el resto del espacio que queda arriba del knob ---
    knobArea.removeFromBottom (10); // separación entre el goniómetro y el knob
    goniometer.setBounds (knobArea);
}
