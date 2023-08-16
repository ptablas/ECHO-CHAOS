/*
  ==============================================================================

    Ek0Ka0s.cpp
    Created: 15 Aug 2023 6:22:24pm
    Author:  Pablo Tablas

  ==============================================================================
*/

#include "Ek0Ka0s.h"

namespace IDs
{
    // Common Parameters

    constexpr auto* stereowidth = "stereowidth";
    constexpr auto* input = "input";
    constexpr auto* output = "output";

    // Mid Parameters

    //Filter
    constexpr auto* cutoffmid = "cutoffmid";
    constexpr auto* resonancemid = "resonancemid";
    constexpr auto* modemid = "modemid";
    //Delay
    constexpr auto* sendmid = "sendmid";
    constexpr auto* timemid = "timemid";
    constexpr auto* feedbackmid = "feedbackmid";
    //LFO
    constexpr auto* lfospeedmid = "lfospeedmid";
    constexpr auto* lfodepthmid = "lfodepthmid";
    constexpr auto* waveformmid = "waveformmid";

    // Side Parameters

    //Filter
    constexpr auto* cutoffside = "cutoffside";
    constexpr auto* resonanceside = "resonanceside";
    constexpr auto* modeside = "modeside";
    //Delay
    constexpr auto* sendside = "sendside";
    constexpr auto* timeside = "timeside";
    constexpr auto* feedbackside = "feedbackside";
    //LFO
    constexpr auto* lfospeedside = "lfospeedside";
    constexpr auto* lfodepthside = "lfodepthside";
    constexpr auto* waveformside = "waveformside";
}

void Ek0Ka0s::addMSParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout)
{
    auto stereowidth = std::make_unique<juce::AudioParameterFloat>("stereowidth", "StereoWidth", 0.f, 2.f, 1.f);
    auto input = std::make_unique<juce::AudioParameterChoice>("input", "Input", juce::StringArray("Stereo", "Mid/Side"), 0);
    auto output = std::make_unique<juce::AudioParameterChoice>("output", "Output", juce::StringArray("Stereo", "Mid/Side"), 0);

    auto group = std::make_unique<juce::AudioProcessorParameterGroup>("ms", "MS", "|",
        std::move(stereowidth),
        std::move(input),
        std::move(output));
    layout.add(std::move(group));
}

void Ek0Ka0s::addMidParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout)
{
    //Filter 
    auto cutoffmid = std::make_unique<juce::AudioParameterFloat>("cutoffmid", "cutoffMid", juce::NormalisableRange<float> {20.f, 20000.0f, 0.0001f, 0.6f}, 200.f);  // <-creates skew factor  
    auto resonancemid = std::make_unique<juce::AudioParameterFloat>("resonancemid", "ResonanceMid", 0.1, 0.7f, 0.0001);                                                //   (more of the dial
    auto modemid = std::make_unique<juce::AudioParameterChoice>("modemid", "Filter Type Mid", juce::StringArray("LPF", "BPF", "HPF"), 0);                         //   affects lower side)

    //Delay
    auto sendmid = std::make_unique<juce::AudioParameterFloat>("sendmid", "SendMid", 0.f, 1.f, 0.f); //controls dry/wet of signal
    auto timemid = std::make_unique<juce::AudioParameterFloat>("timemid", "TimeMid", 0.f, 20000.f, 0.f); // Delay time in samples
    auto feedbackmid = std::make_unique<juce::AudioParameterFloat>("feedbackmid", "FeedbackMid", 0.f, 0.9f, 0.0001f);
    
    //LFO
    auto lfospeedmid = std::make_unique<juce::AudioParameterFloat>("lfospeedmid", "LFOSpeedMid", juce::NormalisableRange<float> {0.f, 10.f, 0.0001f, 0.6f}, 0.f); //in Hertz
    auto lfodepthmid = std::make_unique<juce::AudioParameterFloat>("lfodepthmid", "LFODepthMid", juce::NormalisableRange<float> {0.f, 20000.f / 2.f, 0.0001f, 0.6f}, 0.f); // in miliseconds (since it modulates time) Again skew factor
    auto waveformmid = std::make_unique<juce::AudioParameterChoice>("waveformmid", "WaveformMid", juce::StringArray("Sine", "Triangle", "Sawtooth", "Square", "Random", "Sample & Hold"), 0);


    auto group = std::make_unique<juce::AudioProcessorParameterGroup>("mid", "MID", "|",
        std::move(cutoffmid),
        std::move(resonancemid),
        std::move(modemid),

        std::move(sendmid),
        std::move(timemid),
        std::move(feedbackmid),

        std::move(lfospeedmid),
        std::move(lfodepthmid),
        std::move(waveformmid));
    layout.add(std::move(group));
}

void Ek0Ka0s::addSideParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout)
{
    //Filter 
    auto cutoffside = std::make_unique<juce::AudioParameterFloat>("cutoffside", "cutoffSide", juce::NormalisableRange<float> {20.f, 20000.0f, 0.0001f, 0.6f}, 200.f);  // <-creates skew factor  
    auto resonanceside = std::make_unique<juce::AudioParameterFloat>("resonanceside", "ResonanceSide", 0.1, 0.7f, 0.0001);                                                //   (more of the dial
    auto modeside = std::make_unique<juce::AudioParameterChoice>("modeside", "Filter Type Side", juce::StringArray("LPF", "BPF", "HPF"), 0);                         //   affects lower side)

    //Delay
    auto sendside = std::make_unique<juce::AudioParameterFloat>("sendside", "SendSide", 0.f, 1.f, 0.f); //controls dry/wet of signal
    auto timeside = std::make_unique<juce::AudioParameterFloat>("timeside", "TimeSide", 0.f, 20000.f, 0.f); // Delay time in samples
    auto feedbackside = std::make_unique<juce::AudioParameterFloat>("feedbackside", "FeedbackSide", 0.f, 0.9f, 0.0001f);

    //LFO
    auto lfospeedside = std::make_unique<juce::AudioParameterFloat>("lfospeedside", "LFOSpeedSide", juce::NormalisableRange<float> {0.f, 10.f, 0.0001f, 0.6f}, 0.f); //in Hertz
    auto lfodepthside = std::make_unique<juce::AudioParameterFloat>("lfodepthside", "LFODepthSide", juce::NormalisableRange<float> {0.f, 20000.f / 2.f, 0.0001f, 0.6f}, 0.f); // in miliseconds (since it modulates time) Again skew factor
    auto waveformside = std::make_unique<juce::AudioParameterChoice>("waveformside", "WaveformSide", juce::StringArray("Sine", "Triangle", "Sawtooth", "Square", "Random", "Sample & Hold"), 0);


    auto group = std::make_unique<juce::AudioProcessorParameterGroup>("side", "SIDE", "|",
        std::move(cutoffside),
        std::move(resonanceside),
        std::move(modeside),

        std::move(sendside),
        std::move(timeside),
        std::move(feedbackside),

        std::move(lfospeedside),
        std::move(lfodepthside),
        std::move(waveformside));
    layout.add(std::move(group));
}