/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleEqAudioProcessorEditor::SimpleEqAudioProcessorEditor (SimpleEqAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

	for (auto* comp : getComps())
	{
		addAndMakeVisible(comp);
	}

    setSize (1000, 600);
}

SimpleEqAudioProcessorEditor::~SimpleEqAudioProcessorEditor()
{
}

//==============================================================================
void SimpleEqAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (15.0f));
    //g.drawFittedText ("Hello World! Im yohan this is my first build", getLocalBounds(), juce::Justification::centred, 1);
}

void SimpleEqAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

	auto bounds = getLocalBounds();
	auto responseArea = bounds.removeFromTop(bounds.getHeight() * (1.0/3.0));

	auto lowCutArea = bounds.removeFromLeft(bounds.getWidth() * (1.0/6.0));
	auto band1Area = bounds.removeFromLeft(bounds.getWidth() * (1.0/5.0));
	auto band2Area = bounds.removeFromLeft(bounds.getWidth() * (1.0 / 4.0));
	auto band3Area = bounds.removeFromLeft(bounds.getWidth() * (1.0/3.0));
	auto band4Area = bounds.removeFromLeft(bounds.getWidth() * (1.0 / 2.0));
	auto highCutArea = bounds;

	lowCutFreqSlider.setBounds(lowCutArea.removeFromTop(lowCutArea.getHeight()*0.5));
	lowCutSlopeSlider.setBounds(lowCutArea);

	band1FreqSlider.setBounds(band1Area.removeFromTop(band1Area.getHeight() * 0.33));
	band1GainSlider.setBounds(band1Area.removeFromTop(band1Area.getHeight() * 0.5));
	band1QualitySlider.setBounds(band1Area);

	band2FreqSlider.setBounds(band2Area.removeFromTop(band2Area.getHeight() * 0.33));
	band2GainSlider.setBounds(band2Area.removeFromTop(band2Area.getHeight() * 0.5));
	band2QualitySlider.setBounds(band2Area);

	band3FreqSlider.setBounds(band3Area.removeFromTop(band3Area.getHeight() * 0.33));
	band3GainSlider.setBounds(band3Area.removeFromTop(band3Area.getHeight() * 0.5));
	band3QualitySlider.setBounds(band3Area);

	band4FreqSlider.setBounds(band4Area.removeFromTop(band4Area.getHeight() * 0.33));
	band4GainSlider.setBounds(band4Area.removeFromTop(band4Area.getHeight() * 0.5));
	band4QualitySlider.setBounds(band4Area);

	highCutFreqSlider.setBounds(highCutArea.removeFromTop(highCutArea.getHeight() *0.5));
	highCutSlopeSlider.setBounds(highCutArea);
	
}

std::vector<juce::Component*> SimpleEqAudioProcessorEditor::getComps()
{
    return
	{
		&lowCutFreqSlider,
		&highCutFreqSlider,
		&band1FreqSlider, &band1GainSlider, &band1QualitySlider,
		&band2FreqSlider, &band2GainSlider, &band2QualitySlider,
		&band3FreqSlider, &band3GainSlider, &band3QualitySlider,
		&band4FreqSlider, &band4GainSlider, &band4QualitySlider,
		&lowCutSlopeSlider,&highCutSlopeSlider

	};
}
