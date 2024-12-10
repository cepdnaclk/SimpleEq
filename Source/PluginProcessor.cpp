/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleEqAudioProcessor::SimpleEqAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

SimpleEqAudioProcessor::~SimpleEqAudioProcessor()
{
}

//==============================================================================
const juce::String SimpleEqAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SimpleEqAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SimpleEqAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SimpleEqAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SimpleEqAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SimpleEqAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SimpleEqAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SimpleEqAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SimpleEqAudioProcessor::getProgramName (int index)
{
    return {};
}

void SimpleEqAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SimpleEqAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
	juce::dsp::ProcessSpec spec;
	spec.maximumBlockSize = samplesPerBlock;
	spec.sampleRate = sampleRate;
    spec.numChannels = 1;  //only mono chain is supported in dsp

	leftChain.prepare(spec);
	rightChain.prepare(spec);

	updateFilters();

}

void SimpleEqAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SimpleEqAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void SimpleEqAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

	//update filters
	updateFilters();

    //Processing Audio
	juce::dsp::AudioBlock<float> block(buffer);
	auto leftBlock = block.getSingleChannelBlock(0);
	auto rightBlock = block.getSingleChannelBlock(1);

	juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
	juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);

	leftChain.process(leftContext);
	rightChain.process(rightContext);
    //Processing End
}

//==============================================================================
bool SimpleEqAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SimpleEqAudioProcessor::createEditor()
{
   // return new SimpleEqAudioProcessorEditor (*this);
	return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void SimpleEqAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SimpleEqAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts)
{
    ChainSettings settings;

    settings.lowCutFreq = apvts.getRawParameterValue("LowCut Freq")->load();
    settings.lowCutSlope = static_cast<Slope>(apvts.getRawParameterValue("LowCut Slope")->load());

    settings.highCutFreq = apvts.getRawParameterValue("HighCut Freq")->load();
    settings.highCutSlope = static_cast<Slope>(apvts.getRawParameterValue("HighCut Slope")->load());

    settings.peakFreq1 = apvts.getRawParameterValue("Peak1 Freq")->load();
    settings.peakGainInDecibels1 = apvts.getRawParameterValue("Peak1 Gain")->load();
    settings.peakQuality1 = apvts.getRawParameterValue("Peak1 Quality")->load();

    settings.peakFreq2 = apvts.getRawParameterValue("Peak2 Freq")->load();
    settings.peakGainInDecibels2 = apvts.getRawParameterValue("Peak2 Gain")->load();
    settings.peakQuality2 = apvts.getRawParameterValue("Peak2 Quality")->load();

    settings.peakFreq3 = apvts.getRawParameterValue("Peak3 Freq")->load();
    settings.peakGainInDecibels3 = apvts.getRawParameterValue("Peak3 Gain")->load();
    settings.peakQuality3 = apvts.getRawParameterValue("Peak3 Quality")->load();
    
    settings.peakFreq4 = apvts.getRawParameterValue("Peak4 Freq")->load();
    settings.peakGainInDecibels4 = apvts.getRawParameterValue("Peak4 Gain")->load();
    settings.peakQuality4 = apvts.getRawParameterValue("Peak4 Quality")->load();


    return settings;

};

void SimpleEqAudioProcessor::updateCoefficients(Coefficients& old, const Coefficients& replacements)
{
	*old = *replacements;
};

void SimpleEqAudioProcessor::updatePeakFilter(const ChainSettings& chainSettings)
{
    //get filter coeffiecnts peak filter 1
    auto peakCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), chainSettings.peakFreq1, chainSettings.peakQuality1, juce::Decibels::decibelsToGain(chainSettings.peakGainInDecibels1));
    auto peakCoefficients2 = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), chainSettings.peakFreq2, chainSettings.peakQuality2, juce::Decibels::decibelsToGain(chainSettings.peakGainInDecibels2));
    auto peakCoefficients3 = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), chainSettings.peakFreq3, chainSettings.peakQuality3, juce::Decibels::decibelsToGain(chainSettings.peakGainInDecibels3));
    auto peakCoefficients4 = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), chainSettings.peakFreq4, chainSettings.peakQuality4, juce::Decibels::decibelsToGain(chainSettings.peakGainInDecibels4));


    //set filter coefficients for peak fileters
    //*leftChain.get<ChainPositions::Peak>().coefficients = *peakCoefficients;
    //*rightChain.get<ChainPositions::Peak>().coefficients = *peakCoefficients;
	updateCoefficients(leftChain.get<ChainPositions::Peak1>().coefficients, peakCoefficients);
	updateCoefficients(rightChain.get<ChainPositions::Peak1>().coefficients, peakCoefficients);

    updateCoefficients(leftChain.get<ChainPositions::Peak2>().coefficients, peakCoefficients2);
    updateCoefficients(rightChain.get<ChainPositions::Peak2>().coefficients, peakCoefficients2);

    updateCoefficients(leftChain.get<ChainPositions::Peak3>().coefficients, peakCoefficients3);
    updateCoefficients(rightChain.get<ChainPositions::Peak3>().coefficients, peakCoefficients3);

    updateCoefficients(leftChain.get<ChainPositions::Peak4>().coefficients, peakCoefficients4);
    updateCoefficients(rightChain.get<ChainPositions::Peak4>().coefficients, peakCoefficients4);



};

void SimpleEqAudioProcessor::updateLowCutFilters(const ChainSettings& chainSettings)
{
    //get filter coeffiecnts for low cut
    auto lowCutCoefficients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chainSettings.lowCutFreq, getSampleRate(), 2 * (chainSettings.lowCutSlope + 1));

    // get left and right low cut filter chains
    auto& leftLowCut = leftChain.get<ChainPositions::LowCut>();
    auto& rightLowCut = rightChain.get<ChainPositions::LowCut>();

    //updating low cut filter chains with coeffiecnts
    updateCutFilter(leftLowCut, lowCutCoefficients, chainSettings.lowCutSlope);
    updateCutFilter(rightLowCut, lowCutCoefficients, chainSettings.lowCutSlope);
};

void SimpleEqAudioProcessor::updateHighCutFilters(const ChainSettings& chainSettings)
{
    //get filter coeffiecnts for high cut
    auto highCutCoefficients = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(chainSettings.highCutFreq, getSampleRate(), 2 * (chainSettings.highCutSlope + 1));

    // get left and right high cut filter chains
    auto& leftHighCut = leftChain.get<ChainPositions::HighCut>();
    auto& rightHighCut = rightChain.get<ChainPositions::HighCut>();

    //updating high cut filter chains with coeffiecnts
    updateCutFilter(leftHighCut, highCutCoefficients, chainSettings.highCutSlope);
    updateCutFilter(rightHighCut, highCutCoefficients, chainSettings.highCutSlope);
};

void SimpleEqAudioProcessor::updateFilters()
{
	auto chainSettings = getChainSettings(apvts);

	updateLowCutFilters(chainSettings);
	updateHighCutFilters(chainSettings);
	updatePeakFilter(chainSettings);
};

juce::AudioProcessorValueTreeState::ParameterLayout
SimpleEqAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<juce::AudioParameterFloat>("LowCut Freq", "LowCut Freq", juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f), 20.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("HighCut Freq", "HighCut Freq", juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f), 20000.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak1 Freq", "Band 1 Freq", juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f), 750.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak1 Gain", "Band 1 Gain", juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f), 0.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak1 Quality", "Band 1 Quality", juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.f), 1.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak2 Freq", "Band 2 Freq", juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f), 750.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak2 Gain", "Band 2 Gain", juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f), 0.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak2 Quality", "Band 2 Quality", juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.f), 1.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak3 Freq", "Band 3 Freq", juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f), 750.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak3 Gain", "Band 3 Gain", juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f), 0.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak3 Quality", "Band 3 Quality", juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.f), 1.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak4 Freq", "Band 4 Freq", juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.25f), 750.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak4 Gain", "Band 4 Gain", juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f), 0.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak4 Quality","Band 4 Quality", juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.f), 1.f));


    juce::StringArray stringArray;
    for (int i = 0; i < 4; i++)
    {
        juce::String str;
        str << (12 + i * 12);
        str << " db/Oct";
        stringArray.add(str);
    }

    layout.add(std::make_unique<juce::AudioParameterChoice>("LowCut Slope", "LowCut Slope", stringArray, 0));
    layout.add(std::make_unique<juce::AudioParameterChoice>("HighCut Slope", "HighCut Slope", stringArray, 0));





    return layout;
};

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleEqAudioProcessor();
}
