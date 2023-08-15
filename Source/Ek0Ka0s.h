/*
  ==============================================================================

    Ek0Ka0s.h
    Created: 15 Aug 2023 6:22:24pm
    Author:  Pablo Tablas

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class Ek0Ka0s
{
public:

    static void addMSParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout);
    static void addMidParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout);
    static void addSideParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout);

    Ek0Ka0s() = default;
};