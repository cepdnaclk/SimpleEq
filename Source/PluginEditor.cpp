/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleEqAudioProcessorEditor::SimpleEqAudioProcessorEditor(SimpleEqAudioProcessor& p)
	: AudioProcessorEditor(&p), audioProcessor(p),
	lowCutFreqSliderAttachment(audioProcessor.apvts, "LowCut Freq", lowCutFreqSlider),
	lowCutSlopeSliderAttachment(audioProcessor.apvts, "LowCut Slope", lowCutSlopeSlider),
	highCutFreqSliderAttachment(audioProcessor.apvts, "HighCut Freq", highCutFreqSlider),
	highCutSlopeSliderAttachment(audioProcessor.apvts, "HighCut Slope", highCutSlopeSlider),
	band1FreqSliderAttachment(audioProcessor.apvts, "Peak1 Freq", band1FreqSlider),
	band1GainSliderAttachment(audioProcessor.apvts, "Peak1 Gain", band1GainSlider),
	band1QualitySliderAttachment(audioProcessor.apvts, "Peak1 Quality", band1QualitySlider),
	band2FreqSliderAttachment(audioProcessor.apvts, "Peak2 Freq", band2FreqSlider),
	band2GainSliderAttachment(audioProcessor.apvts, "Peak2 Gain", band2GainSlider),
	band2QualitySliderAttachment(audioProcessor.apvts, "Peak2 Quality", band2QualitySlider),
	band3FreqSliderAttachment(audioProcessor.apvts, "Peak3 Freq", band3FreqSlider),
	band3GainSliderAttachment(audioProcessor.apvts, "Peak3 Gain", band3GainSlider),
	band3QualitySliderAttachment(audioProcessor.apvts, "Peak3 Quality", band3QualitySlider),
	band4FreqSliderAttachment(audioProcessor.apvts, "Peak4 Freq", band4FreqSlider),
	band4GainSliderAttachment(audioProcessor.apvts, "Peak4 Gain", band4GainSlider),
	band4QualitySliderAttachment(audioProcessor.apvts, "Peak4 Quality", band4QualitySlider)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

	
	// Initialize spectrum analyzer placeholder
	addAndMakeVisible(spectrumAnalyzerPlaceholder);

	// Initialize band buttons
	for (int i = 0; i < 6; ++i)
	{
		bandButtons[i].setButtonText(juce::String(i + 1));
		bandButtons[i].setClickingTogglesState(false);
		bandButtons[i].setColour(juce::TextButton::buttonColourId, bandColours[i]);
		//bandButtons[i].setColour(juce::TextButton::textColourOffId, juce::Colours::black);

		// Use onClick for button handling
		bandButtons[i].onClick = [this, i]() {
			activeBand = i;
			DBG("Active Band: " << (activeBand + 1));
			//bandButtons[i].setColour(juce::TextButton::buttonColourId, bandColours[i]);
			updateSlidersForBand(activeBand);
		};

		addAndMakeVisible(bandButtons[i]);
	}
	

	for (auto* comp : getSliders())
	{
		initializeSlider(*comp);
	}

	// Set the size of the editor
	setSize(800, 600);

}

SimpleEqAudioProcessorEditor::~SimpleEqAudioProcessorEditor()
{

}

//==============================================================================
void SimpleEqAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    //g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));



	// Fill background 
	//20,29,36
	//21,30,37
	//48,48,48
	g.fillAll(juce::Colour(20,29,36));

	// Draw plugin title
	g.setColour(juce::Colours::white);
	g.setFont(15.0f);
	g.drawText("EQ", getLocalBounds(), juce::Justification::centredTop);

	// Spectrum analyzer placeholder
	g.setColour(juce::Colours::black);
	auto spectrumAnalyzerArea = spectrumAnalyzerPlaceholder.getBounds();
	g.fillRect(spectrumAnalyzerArea);
	g.setColour(juce::Colours::white);
	g.drawRect(spectrumAnalyzerArea, 1);
}

void SimpleEqAudioProcessorEditor::resized()
{
    
	auto bounds = getLocalBounds();

	// Reserve space for the spectrum analyzer at the top
	auto spectrumAnalyzerArea = bounds.removeFromTop(bounds.getHeight() / 2);
	spectrumAnalyzerPlaceholder.setBounds(spectrumAnalyzerArea);


	// Dynamic button sizing
	float buttonWidthRatio = 0.06f; // 8% of the width per button
	float buttonHeightRatio = 0.08f; // 8% of the height
	int buttonWidth = static_cast<int>(bounds.getWidth() * buttonWidthRatio);
	int buttonHeight = static_cast<int>(bounds.getHeight() * buttonHeightRatio);
	int buttonSpacing = static_cast<int>(buttonWidth * 0.1f); // 20% of button width for spacing

	// Layout band buttons
	int totalWidth = (buttonWidth * 6) + (buttonSpacing * 5);
	int startX = (bounds.getWidth() - totalWidth) / 2;
	int startY = spectrumAnalyzerArea.getBottom() + 10;


	for (int i = 0; i < 6; ++i)
		bandButtons[i].setBounds(startX + i * (buttonWidth + buttonSpacing), startY, buttonWidth, buttonHeight);


	// Dynamic slider sizing
	float sliderWidthRatio = 0.5f; // 15% of the width
	float sliderHeightRatio = 0.5f; // 20% of the height
	int sliderWidth = static_cast<int>(bounds.getHeight() * sliderWidthRatio);
	int sliderHeight = static_cast<int>(bounds.getHeight() * sliderHeightRatio);
	int sliderSpacing = static_cast<int>(sliderWidth * 0.2f); // 30% of slider width for spacing

	int sliderY = startY + buttonHeight + 50;

	int totalsliderwidth = (sliderWidth * 2) + sliderSpacing;
	DBG("width" << bounds.getWidth());
	int sliderstartx = ((bounds.getWidth() / 2) - (totalsliderwidth / 2));

	// Layout sliders
	switch (activeBand) {
	case 0:

		lowCutFreqSlider.setBounds(sliderstartx, sliderY, sliderWidth, sliderHeight);
		lowCutSlopeSlider.setBounds(sliderstartx + sliderWidth + sliderSpacing, sliderY, sliderWidth, sliderHeight);
		break;
	case 1:
		band1FreqSlider.setBounds((bounds.getWidth() - sliderWidth) / 2, sliderY, sliderWidth, sliderHeight);
		band1GainSlider.setBounds((bounds.getWidth() - sliderWidth) / 2 - sliderWidth - sliderSpacing, sliderY, sliderWidth, sliderHeight);
		band1QualitySlider.setBounds((bounds.getWidth() - sliderWidth) / 2 + sliderWidth + sliderSpacing, sliderY, sliderWidth, sliderHeight);
		break;
	case 2:
		band2FreqSlider.setBounds((bounds.getWidth() - sliderWidth) / 2, sliderY, sliderWidth, sliderHeight);
		band2GainSlider.setBounds((bounds.getWidth() - sliderWidth) / 2 - sliderWidth - sliderSpacing, sliderY, sliderWidth, sliderHeight);
		band2QualitySlider.setBounds((bounds.getWidth() - sliderWidth) / 2 + sliderWidth + sliderSpacing, sliderY, sliderWidth, sliderHeight);
		break;
	case 3:
		band3FreqSlider.setBounds((bounds.getWidth() - sliderWidth) / 2, sliderY, sliderWidth, sliderHeight);
		band3GainSlider.setBounds((bounds.getWidth() - sliderWidth) / 2 - sliderWidth - sliderSpacing, sliderY, sliderWidth, sliderHeight);
		band3QualitySlider.setBounds((bounds.getWidth() - sliderWidth) / 2 + sliderWidth + sliderSpacing, sliderY, sliderWidth, sliderHeight);
		break;
	case 4:
		band4FreqSlider.setBounds((bounds.getWidth() - sliderWidth) / 2, sliderY, sliderWidth, sliderHeight);
		band4GainSlider.setBounds((bounds.getWidth() - sliderWidth) / 2 - sliderWidth - sliderSpacing, sliderY, sliderWidth, sliderHeight);
		band4QualitySlider.setBounds((bounds.getWidth() - sliderWidth) / 2 + sliderWidth + sliderSpacing, sliderY, sliderWidth, sliderHeight);
		break;
	case 5:
		highCutFreqSlider.setBounds(sliderstartx, sliderY, sliderWidth, sliderHeight);
		highCutSlopeSlider.setBounds(sliderstartx + sliderWidth + sliderSpacing, sliderY, sliderWidth, sliderHeight);
		break;
	default:
		band1FreqSlider.setBounds((bounds.getWidth() - sliderWidth) / 2, sliderY, sliderWidth, sliderHeight);
		band1GainSlider.setBounds((bounds.getWidth() - sliderWidth) / 2 - sliderWidth - sliderSpacing, sliderY, sliderWidth, sliderHeight);
		band1QualitySlider.setBounds((bounds.getWidth() - sliderWidth) / 2 + sliderWidth + sliderSpacing, sliderY, sliderWidth, sliderHeight);
		break;
	}

	
}

void SimpleEqAudioProcessorEditor::initializeSlider(juce::Slider& slider)
{
	//slider.setSliderStyle(style);
	//slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
	//slider.setName(name);
	slider.setVisible(false);  // Start with sliders hidden
	addAndMakeVisible(slider);
}



void SimpleEqAudioProcessorEditor::updateSlidersForBand(int bandIndex)
{
	// Hide all sliders initially
	for (auto* comp : getSliders())
	{
		comp->setVisible(false);
	}

	switch (bandIndex) {
	case 0:
		lowCutFreqSlider.setVisible(true);
		lowCutSlopeSlider.setVisible(true);
		break;
	case 1:
		band1FreqSlider.setVisible(true);
		band1GainSlider.setVisible(true);
		band1QualitySlider.setVisible(true);
		break;
	case 2:
		band2FreqSlider.setVisible(true);
		band2GainSlider.setVisible(true);
		band2QualitySlider.setVisible(true);
		break;
	case 3:
		band3FreqSlider.setVisible(true);
		band3GainSlider.setVisible(true);
		band3QualitySlider.setVisible(true);
		break;
	case 4:
		band4FreqSlider.setVisible(true);
		band4GainSlider.setVisible(true);
		band4QualitySlider.setVisible(true);
		break;
	case 5:
		highCutFreqSlider.setVisible(true);
		highCutSlopeSlider.setVisible(true);
		break;
	default:
		band1FreqSlider.setVisible(true);
		band1GainSlider.setVisible(true);
		band1QualitySlider.setVisible(true);
		break;	
	}
	
	// Trigger a layout update
	resized();
}

std::vector<juce::Slider*> SimpleEqAudioProcessorEditor::getSliders()
{
    return
	{
		&lowCutFreqSlider,&lowCutSlopeSlider,
		&highCutFreqSlider,&highCutSlopeSlider,
		&band1FreqSlider, &band1GainSlider, &band1QualitySlider,
		&band2FreqSlider, &band2GainSlider, &band2QualitySlider,
		&band3FreqSlider, &band3GainSlider, &band3QualitySlider,
		&band4FreqSlider, &band4GainSlider, &band4QualitySlider
	};
}
