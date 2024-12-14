/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/

struct CustomRotarySlider : juce::Slider
{
	CustomRotarySlider() : juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
		juce::Slider::TextEntryBoxPosition::NoTextBox)
	{
        setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::aliceblue);
	}
};

class SimpleEqAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    SimpleEqAudioProcessorEditor (SimpleEqAudioProcessor&);
    ~SimpleEqAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SimpleEqAudioProcessor& audioProcessor;

    CustomRotarySlider lowCutFreqSlider,
        highCutFreqSlider,
        band1FreqSlider, band1GainSlider, band1QualitySlider,
        band2FreqSlider, band2GainSlider, band2QualitySlider,
        band3FreqSlider, band3GainSlider, band3QualitySlider,
		band4FreqSlider, band4GainSlider, band4QualitySlider,
		lowCutSlopeSlider, highCutSlopeSlider;

	using APVTS = juce::AudioProcessorValueTreeState;
	using RotarySliderAttachment = APVTS::SliderAttachment;

    RotarySliderAttachment lowCutFreqSliderAttachment,
        highCutFreqSliderAttachment,
        band1FreqSliderAttachment, band1GainSliderAttachment, band1QualitySliderAttachment,
        band2FreqSliderAttachment, band2GainSliderAttachment, band2QualitySliderAttachment,
        band3FreqSliderAttachment, band3GainSliderAttachment, band3QualitySliderAttachment,
        band4FreqSliderAttachment, band4GainSliderAttachment, band4QualitySliderAttachment,
		lowCutSlopeSliderAttachment, highCutSlopeSliderAttachment;

	std::vector<juce::Component*> getComps();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleEqAudioProcessorEditor)
};
