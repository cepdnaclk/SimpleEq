/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

enum Slope
{
	slope_12,
	slope_24,
    slope_36,
	slope_48
};

struct ChainSettings 
{
	float peakFreq{ 0 }, peakGainInDecibels{ 0 }, peakQuality{ 1.f };
	float lowCutFreq{ 0 }, highCutFreq{ 0 };

    Slope lowCutSlope{ Slope::slope_12 }, highCutSlope{ Slope::slope_12 }; 
};

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts);    


//==============================================================================
/**
*/
class SimpleEqAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    SimpleEqAudioProcessor();
    ~SimpleEqAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override; 

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
	static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
	juce::AudioProcessorValueTreeState apvts{ *this, nullptr, "Parameters", createParameterLayout() };
private:

	using Filter = juce::dsp::IIR::Filter<float>;
	using CutFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>;
	using MonoChain = juce::dsp::ProcessorChain<CutFilter, Filter, CutFilter>;
	MonoChain leftChain, rightChain;

	enum ChainPositions
	{
		LowCut,
		Peak,
		HighCut
	};

	void updatePeakFilter(const ChainSettings& chainSettings);
	using Coefficients = Filter::CoefficientsPtr;

	static void updateCoefficients(Coefficients& old, const Coefficients& replacements); 

    template <int Index,typename ChainType, typename CoefficientType>
	void updateCutFilterLinks(ChainType& chain, const CoefficientType& coefficients)
	{
        updateCoefficients(chain.template get<Index>().coefficients, coefficients[Index]);
		chain.template setBypassed<Index>(false);
	};

    template <typename ChainType, typename CoefficientType>
    void updateCutFilter(ChainType& leftLowCut, const CoefficientType& cutCoefficients, const Slope& lowCutSlope)
    {

        //bypassing all links in the  low cut chain.
        leftLowCut.template setBypassed<0>(true);
        leftLowCut.template setBypassed<1>(true);
        leftLowCut.template setBypassed<2>(true);
        leftLowCut.template setBypassed<3>(true);

        switch (lowCutSlope)
        {
        case slope_48:
			updateCutFilterLinks<3>(leftLowCut, cutCoefficients);
        case slope_36:
			updateCutFilterLinks<2>(leftLowCut, cutCoefficients);
        case slope_24:
			updateCutFilterLinks<1>(leftLowCut, cutCoefficients);
        case slope_12:
			updateCutFilterLinks<0>(leftLowCut, cutCoefficients);

        }
    };

	void updateFilters();
	void updateLowCutFilters(const ChainSettings& chainSettings);
	void updateHighCutFilters(const ChainSettings& chainSettings);

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleEqAudioProcessor)
};
