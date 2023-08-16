/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Ek0Ka0s.h"
#include "Osc.h"

//==============================================================================
/**
*/

class PresetListBox;

//==============================================================================

class Ek0Ka0sAudioProcessor  : public foleys::MagicProcessor,
                               public juce::AudioProcessorValueTreeState::Listener
{
public:
    //==============================================================================
    Ek0Ka0sAudioProcessor();
    ~Ek0Ka0sAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

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
    void savePresetInternal();
    void loadPresetInternal(int index);

    //==============================================================================
    void parameterChanged(const juce::String& parameterID, float newValue) override;

private:

    juce::AudioProcessorValueTreeState treeState;
    juce::ValueTree                    presetNode;


    // Initialization of Variables for parameterChanged (ultimately for processing block)

    // Width

    float Width;
    juce::SmoothedValue<float> Width_Target;

    // Input/Outputs and other variables that need to be global for the logic to work

    std::string Input_Type = "Stereo";
    std::string Output_Type = "Stereo";

    float xMidRaw;
    float xSideRaw;

    float newLeft = 0;
    float newRight = 0;


    // Initialize Filters

    juce::dsp::StateVariableTPTFilter<float> MidFilterModule; 
    juce::dsp::StateVariableTPTFilter<float> SideFilterModule;

    float Cut_Off_Mid;
    float Cut_Off_Side;

    // Initialize Delay Lagrange3rd is a high-quality interpolation <-> 3000 is longest num. of samples of delay tap

    juce::dsp::DelayLine<double, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> MidDelayModule{30000};
    juce::dsp::DelayLine<double, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> SideDelayModule{30000};

    double Send_Mid = 0.f;
    juce::SmoothedValue<double> Time_Mid_Target = 0.f;
    double Time_Mid = 0.f;
    double Feedback_Mid = 0.f;

    double Send_Side = 0.f;
    juce::SmoothedValue<double> Time_Side_Target = 0.f;
    double Time_Side = 0.f;
    double Feedback_Side = 0.f;

    //LFO Variables

    Osc lfoMid;
    Osc lfoSide;

    double lfoValueMid = 0;
    double lfoValueSide = 0;

    juce::SmoothedValue<double> LFO_Speed_Mid_Target = 0;
    juce::SmoothedValue<double> LFO_Depth_Mid_Target = 0;

    juce::SmoothedValue<double> LFO_Speed_Side_Target = 0;
    juce::SmoothedValue<double> LFO_Depth_Side_Target = 0;


    // GUI MAGIC

    PresetListBox* presetList = nullptr;

    foleys::MagicOscilloscope* midOscilloscope = nullptr;
    foleys::MagicOscilloscope* sideOscilloscope = nullptr;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ek0Ka0sAudioProcessor)
};
