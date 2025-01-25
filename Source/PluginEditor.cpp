/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace BandColours
{
	const juce::Array<juce::Colour> bandColours{ juce::Colour(192,102,102), juce::Colour(154,89,181), juce::Colour(27,188,156),
		juce::Colour(53,152,219), juce::Colour(230,124,37), juce::Colour(196,80,128) };

}


ResponseCurveComponent::ResponseCurveComponent(SimpleEqAudioProcessor& p) : audioProcessor(p)
{
	startTimerHz(60);
	const auto& params = audioProcessor.getParameters();
	for (auto param : params)
	{
		param->addListener(this);
	}

	// Force an initial update of the response curve
	parametersChanged.set(true);
	timerCallback();
}

ResponseCurveComponent::~ResponseCurveComponent()
{
	const auto& params = audioProcessor.getParameters();
	for (auto param : params)
	{
		param->removeListener(this);
	}
}

void ResponseCurveComponent::parameterValueChanged(int parameterIndex, float newValue)
{
	parametersChanged.set(true);
}


void ResponseCurveComponent::timerCallback()
{
	if (parametersChanged.compareAndSetBool(false, true))
	{
		// Update the monochain
		//updating peak filters in 4 bands
		auto chainSettings = getChainSettings(audioProcessor.apvts);
		auto peakCoefficientsVecotr = makePeakFilter(chainSettings, audioProcessor.getSampleRate());

		updateCoefficients(monoChain.get<ChainPositions::Peak1>().coefficients, peakCoefficientsVecotr[0]);
		updateCoefficients(monoChain.get<ChainPositions::Peak2>().coefficients, peakCoefficientsVecotr[1]);
		updateCoefficients(monoChain.get<ChainPositions::Peak3>().coefficients, peakCoefficientsVecotr[2]);
		updateCoefficients(monoChain.get<ChainPositions::Peak4>().coefficients, peakCoefficientsVecotr[3]);

		//updating low cut filters
		auto lowCutCoefficients = makeLowCutFilter(chainSettings, audioProcessor.getSampleRate());
		updateCutFilter(monoChain.get<ChainPositions::LowCut>(), lowCutCoefficients, chainSettings.lowCutSlope);

		//updating high cut filters
		auto highCutCoefficients = makeHighCutFilter(chainSettings, audioProcessor.getSampleRate());
		updateCutFilter(monoChain.get<ChainPositions::HighCut>(), highCutCoefficients, chainSettings.highCutSlope);

		// call a repaint
		repaint();

	}
}

void ResponseCurveComponent::paint(juce::Graphics& g)
{
	// (Our component is opaque, so we must completely fill the background with a solid colour)
	//g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));


	using namespace juce;


	// Draw plugin title
	g.setColour(juce::Colours::white);
	g.setFont(15.0f);
	g.drawText("EQ", getLocalBounds(), Justification::centredTop);

	//drawing response curve
	auto responseCurveArea = getLocalBounds();

	auto w = responseCurveArea.getWidth();

	auto& lowcut = monoChain.get<ChainPositions::LowCut>();
	auto& peak1 = monoChain.get<ChainPositions::Peak1>();
	auto& peak2 = monoChain.get<ChainPositions::Peak2>();
	auto& peak3 = monoChain.get<ChainPositions::Peak3>();
	auto& peak4 = monoChain.get<ChainPositions::Peak4>();
	auto& highcut = monoChain.get<ChainPositions::HighCut>();

	auto sampleRate = audioProcessor.getSampleRate();

	std::vector<double> mags;
	mags.resize(w);

	for (int i = 0; i < w; i++) {

		double mag = 1.f;
		//mapping pixel value to frequency in hearing range
		double freq = mapToLog10(double(i) / double(w), 20.0, 20000.0);

		//checking whether the band is bypassed
		if (!monoChain.isBypassed< ChainPositions::Peak1>()) {
			mag *= peak1.coefficients->getMagnitudeForFrequency(freq, sampleRate);
		}
		if (!monoChain.isBypassed<ChainPositions::Peak2>()) {
			mag *= peak2.coefficients->getMagnitudeForFrequency(freq, sampleRate);
		}
		if (!monoChain.isBypassed<ChainPositions::Peak3>()) {
			mag *= peak3.coefficients->getMagnitudeForFrequency(freq, sampleRate);
		}
		if (!monoChain.isBypassed<ChainPositions::Peak4>()) {
			mag *= peak4.coefficients->getMagnitudeForFrequency(freq, sampleRate);
		}

		if (!lowcut.isBypassed<0>()) {
			mag *= lowcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
		}
		if (!lowcut.isBypassed<1>()) {
			mag *= lowcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
		}
		if (!lowcut.isBypassed<2>()) {
			mag *= lowcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
		}
		if (!lowcut.isBypassed<3>()) {
			mag *= lowcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
		}

		if (!highcut.isBypassed<0>()) {
			mag *= highcut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
		}
		if (!highcut.isBypassed<1>()) {
			mag *= highcut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
		}
		if (!highcut.isBypassed<2>()) {
			mag *= highcut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
		}
		if (!highcut.isBypassed<3>()) {
			mag *= highcut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
		}

		mags[i] = Decibels::gainToDecibels(mag);

	}

	Path responseCurve;

	const double outputMin = responseCurveArea.getBottom();
	const double outputMax = responseCurveArea.getY();

	auto map = [outputMin, outputMax](double input)
		{
			return jmap(input, -24.0, 24.0, outputMin, outputMax);
		};

	responseCurve.startNewSubPath(responseCurveArea.getX(), map(mags.front()));

	for (size_t i = 1; i < mags.size(); ++i) {
		responseCurve.lineTo(responseCurveArea.getX() + i, map(mags[i]));
	}

	//response curve backgorund gradient
	// Define the colors using an array
	Array<Colour> colours{
		Colour(17, 25, 31),  // #11191F (start color)
		Colour(31, 49, 91),  // #1F315B (27% stop)
		Colour(39, 62, 117)  // #31519B (80% stop)
	};

	// Define the gradient's start and end points for the response curve area
	auto startPoint = Point<float>(responseCurveArea.getX(), responseCurveArea.getY());                // Top of the area
	auto endPoint = Point<float>(responseCurveArea.getX(), responseCurveArea.getBottom());            // Bottom of the area

	// Create a linear gradient with the defined colors
	ColourGradient gradient(
		colours[0], startPoint.x, startPoint.y,  // Start color at the top
		colours[2], endPoint.x, endPoint.y, false // End color at the bottom
	);
	gradient.addColour(0.60f, colours[1]); // 27% stop (rgba(31,49,91,1))
	gradient.addColour(0.94f, colours[2]); // 80% stop (rgba(49,81,155,1))

	// Apply the gradient
	g.setGradientFill(gradient);
	g.fillRect(responseCurveArea);



	/*g.setColour(Colours::black);
	g.fillRect(responseCurveArea);*/

	/*g.setColour(Colours::white);
	g.drawRect(responseCurveArea, 1);*/

	g.setColour(Colours::skyblue);
	g.strokePath(responseCurve, PathStrokeType(2.0f));



}

//==============================================================================
SimpleEqAudioProcessorEditor::SimpleEqAudioProcessorEditor(SimpleEqAudioProcessor& p)
	: AudioProcessorEditor(&p), audioProcessor(p),
	lowCutFreqSlider(BandColours::bandColours[0]), lowCutSlopeSlider(BandColours::bandColours[0]),
	highCutFreqSlider(BandColours::bandColours[5]), highCutSlopeSlider(BandColours::bandColours[5]),
	band1FreqSlider(BandColours::bandColours[1]), band1GainSlider(BandColours::bandColours[1]), band1QualitySlider(BandColours::bandColours[1]),
	band2FreqSlider(BandColours::bandColours[2]), band2GainSlider(BandColours::bandColours[2]), band2QualitySlider(BandColours::bandColours[2]),
	band3FreqSlider(BandColours::bandColours[3]), band3GainSlider(BandColours::bandColours[3]), band3QualitySlider(BandColours::bandColours[3]),
	band4FreqSlider(BandColours::bandColours[4]), band4GainSlider(BandColours::bandColours[4]), band4QualitySlider(BandColours::bandColours[4]),
	responseCurveComponent(audioProcessor),
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
	//addAndMakeVisible(responseCurvePlaceholder);

	// Initialize band buttons
	for (int i = 0; i < 6; ++i)
	{
		bandButtons[i].setButtonText(juce::String(i + 1));
		bandButtons[i].setClickingTogglesState(false);
		bandButtons[i].setColour(juce::TextButton::buttonColourId, BandColours::bandColours[i].darker(0.3F));
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

	// Initialize response curve component
	addAndMakeVisible(responseCurveComponent);

	// Force the response curve to update with the current state
	responseCurveComponent.repaint();

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


	using namespace juce;
	// Fill background 
	//20,29,36
	//21,30,37
	//48,48,48
	//g.fillAll(Colour(20,29,36));
	
	 // Define gradient colors using fromFloatRGBA
	//Array<Colour> colours{
	//	Colour::fromFloatRGBA(4.0f / 255.0f, 0.0f, 66.0f / 255.0f, 1.0f),    // rgba(4, 0, 66, 1)
	//	Colour::fromFloatRGBA(29.0f / 255.0f, 29.0f / 255.0f, 117.0f / 255.0f, 1.0f), // rgba(29, 29, 117, 1)
	//	Colour::fromFloatRGBA(75.0f / 255.0f, 3.0f / 255.0f, 177.0f / 255.0f, 1.0f)  // rgba(75, 3, 177, 1)
	//};

	Array<Colour> colours{
	Colour(17, 25, 31), // #1d1d75
	Colour(20, 29, 50), // #1d1d75 (repeated for gradient consistency)
	Colour(17, 25, 31)  // #4b03b1
	};

	// Define the gradient's start and end points for a 45-degree angle
	auto bounds = getLocalBounds().toFloat();
	float width = bounds.getWidth();
	float height = bounds.getHeight();

	Point<float> startPoint(bounds.getX(), bounds.getY());                 // Top-left corner
	Point<float> endPoint(bounds.getX() + width, bounds.getY() + height); // Bottom-right corner

	// Create a linear gradient with the defined colours
	ColourGradient gradient(
		colours[0], startPoint.x, startPoint.y, // Start color at top-left
		colours[2], endPoint.x, endPoint.y, false // End color at bottom-right
	);
	gradient.addColour(0.25f, colours[0]); // 25% stop
	gradient.addColour(0.64f, colours[1]); // 64% stop
	gradient.addColour(1.0f, colours[2]);  // 100% stop

	// Apply the gradient
	g.setGradientFill(gradient);
	g.fillRect(getLocalBounds());

	
}

void SimpleEqAudioProcessorEditor::resized()
{
    
	auto bounds = getLocalBounds();

	// Reserve space for the response curve at the top
	auto responseCurveArea = bounds.removeFromTop(bounds.getHeight() / 2);
	responseCurveComponent.setBounds(responseCurveArea);


	// Dynamic button sizing
	float buttonWidthRatio = 0.06f; // 8% of the width per button
	float buttonHeightRatio = 0.08f; // 8% of the height
	int buttonWidth = static_cast<int>(bounds.getWidth() * buttonWidthRatio);
	int buttonHeight = static_cast<int>(bounds.getHeight() * buttonHeightRatio);
	int buttonSpacing = static_cast<int>(buttonWidth * 0.1f); // 20% of button width for spacing

	// Layout band buttons
	int totalWidth = (buttonWidth * 6) + (buttonSpacing * 5);
	int startX = (bounds.getWidth() - totalWidth) / 2;
	int startY = responseCurveArea.getBottom() + 20;


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
