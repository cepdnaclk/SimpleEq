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
		band4FreqSlider, band4GainSlider, band4QualitySlider;

	std::vector<juce::Component*> getComps();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleEqAudioProcessorEditor)
};
