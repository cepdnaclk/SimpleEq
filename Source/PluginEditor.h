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
		juce::Slider::TextEntryBoxPosition::TextBoxBelow)
	{
        setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
        setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::aliceblue);
	}
};

struct ResponseCurveComponent : juce::Component,
    juce::AudioProcessorParameter::Listener,
    juce::Timer
{
    ResponseCurveComponent(SimpleEqAudioProcessor&);
    ~ResponseCurveComponent() override;

    void parameterValueChanged(int parameterIndex, float newValue) override;
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override {}
    void timerCallback() override;

	void paint(juce::Graphics& g) override;

private:
    SimpleEqAudioProcessor& audioProcessor;
    juce::Atomic<bool> parametersChanged{ false };

	MonoChain monoChain;

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

	ResponseCurveComponent responseCurveComponent;

	using APVTS = juce::AudioProcessorValueTreeState;
	using RotarySliderAttachment = APVTS::SliderAttachment;

    RotarySliderAttachment lowCutFreqSliderAttachment,
        highCutFreqSliderAttachment,
        band1FreqSliderAttachment, band1GainSliderAttachment, band1QualitySliderAttachment,
        band2FreqSliderAttachment, band2GainSliderAttachment, band2QualitySliderAttachment,
        band3FreqSliderAttachment, band3GainSliderAttachment, band3QualitySliderAttachment,
        band4FreqSliderAttachment, band4GainSliderAttachment, band4QualitySliderAttachment,
		lowCutSlopeSliderAttachment, highCutSlopeSliderAttachment;


    juce::TextButton bandButtons[6];  // Buttons for each band (1-6)
    //juce::Slider freqSlider;         // Frequency slider (used for all bands)
    //juce::Slider slopeSlider;        // Slope slider (for low/high cut)
    //juce::Slider gainSlider;         // Gain slider (for bands 2-5)
    //juce::Slider qSlider;            // Q-factor slider (for bands 2-5)

    int activeBand = -1;  // Tracks which band is currently active
    
    // Helper functions
    void initializeSlider(juce::Slider& slider);
	void updateSlidersForBand(int bandIndex);  // Updates visible sliders based on avtive band

	std::vector<juce::Slider*> getSliders();

    //53,152,219
    //39,226,252
	juce::Array<juce::Colour> bandColours{ juce::Colour(192,102,102), juce::Colour(154,89,181), juce::Colour(27,188,156),
        juce::Colour(53,152,219), juce::Colour(230,124,37), juce::Colour(196,80,128) };


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleEqAudioProcessorEditor)
};
