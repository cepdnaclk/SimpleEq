/*
  ==============================================================================

    SliderLAF.h
    Created: 24 Dec 2024 11:11:26pm
    Author:  yohan

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class SliderLAF  : public juce::Component
{
public:
    SliderLAF();
    ~SliderLAF() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SliderLAF)
};
