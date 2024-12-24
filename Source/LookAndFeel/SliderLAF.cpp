/*
  ==============================================================================

    SliderLAF.cpp
    Created: 24 Dec 2024 11:11:26pm
    Author:  yohan

  ==============================================================================
*/

#include <JuceHeader.h>
#include "SliderLAF.h"

//==============================================================================
SliderLAF::SliderLAF()
{
   
}

void SliderLAF::drawRotarySlider(Graphics& g, int x, int y, int width, int height, float sliderPos, const float rotaryStartAngle, const float rotaryEndAngle, Slider& slider)
{
    //float diameter = fmin(width, height) * .9;
    //float radius = diameter * 0.5;

    //float angle = rotaryStartAngle + (sliderPos * (rotaryEndAngle - rotaryStartAngle));

    



    //g.setColour(slider.findColour(juce::Slider::ColourIds::rotarySliderOutlineColourId)); //outline
    //g.drawEllipse(rx, ry, diameter, diameter, 4.0f);

    //juce::Path dialTick;
    //g.setColour(slider.findColour(juce::Slider::ColourIds::trackColourId)); //tick color
    //dialTick.addRectangle(0, -radius + 2, 4.0f, radius * 0.6);
    //g.fillPath(dialTick, juce::AffineTransform::rotation(angle).translated(centerX, centerY));

    auto outline = slider.findColour(Slider::rotarySliderOutlineColourId);
    auto bounds = Rectangle<int>(x, y, width, height).toFloat().reduced(10);
    auto radius = jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    auto lineW = jmin(4.0f, radius * 0.5f) * 1.1f;
    auto arcRadius = radius - lineW * 0.5f;

	auto diameter = 2.0f * radius;
    float centerX = x + width * 0.5;
    float centerY = y + height * 0.5;
    float rx = centerX - radius;
    float ry = centerY - radius;

    juce::Rectangle<float> dialArea(rx, ry, diameter, diameter);
    g.setColour(slider.findColour(Slider::thumbColourId).withAlpha(0.7f)); //center
    g.fillEllipse(dialArea);

    // Dial-path background
    Path backgroundArc;
    backgroundArc.addCentredArc(bounds.getCentreX(),
        bounds.getCentreY(),
        arcRadius,
        arcRadius,
        0.0f,
        rotaryStartAngle,
        rotaryEndAngle,
        true);

    g.setColour(outline);
    g.strokePath(backgroundArc, PathStrokeType(lineW, PathStrokeType::curved, PathStrokeType::rounded));

    if (slider.isEnabled())
    {
        // Part of arc that is filled (values that have been passed)
        Path valueArc;

        valueArc.addCentredArc(bounds.getCentreX(),
            bounds.getCentreY(),
            arcRadius,
            arcRadius,
            0.0f,
            rotaryStartAngle,
            toAngle,
            true);

        g.setColour(slider.findColour(Slider::rotarySliderFillColourId));
        g.strokePath(valueArc, PathStrokeType(lineW, PathStrokeType::curved, PathStrokeType::rounded));
    }

    // Dial thumb
    Point<float> thumbPoint(bounds.getCentreX() + (arcRadius - lineW * 1.25) * std::cos(toAngle - MathConstants<float>::halfPi),
        bounds.getCentreY() + (arcRadius - lineW * 1.25) * std::sin(toAngle - MathConstants<float>::halfPi));

    g.setColour(slider.findColour(Slider::thumbColourId));
    g.drawLine(backgroundArc.getBounds().getCentreX(), backgroundArc.getBounds().getCentreY(), thumbPoint.getX(), thumbPoint.getY(), lineW / 2.0);


}







