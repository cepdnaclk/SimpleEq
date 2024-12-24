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
using namespace juce;
class SliderLAF  : public juce::LookAndFeel_V4
{
public:

    SliderLAF();

    void drawRotarySlider(Graphics& g, int x, int y, int width, int height, float sliderPos,
		const float rotaryStartAngle, const float rotaryEndAngle, Slider& slider) override;

};
