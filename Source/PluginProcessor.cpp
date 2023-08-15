/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PresetListBox.h"


//==============================================================================

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    Ek0Ka0s::addMSParameters(layout);
    Ek0Ka0s::addMidParameters(layout);
    Ek0Ka0s::addSideParameters(layout);
    return layout;
}

//==============================================================================


Ek0Ka0sAudioProcessor::Ek0Ka0sAudioProcessor()
    : foleys::MagicProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
        treeState(*this, nullptr, ProjectInfo::projectName, createParameterLayout())
{

        FOLEYS_SET_SOURCE_PATH(__FILE__);

        auto file = juce::File::getSpecialLocation(juce::File::currentApplicationFile)
        .getChildFile("Contents")
        .getChildFile("Resources")
        .getChildFile("magic.xml");
    
        if (file.existsAsFile())
        magicState.setGuiValueTree(file);
        else
        magicState.setGuiValueTree(BinaryData::magic_xml, BinaryData::magic_xmlSize);

        /*
        // MAGIC GUI: add a meter at the output
        outputMeter = magicState.createAndAddObject<foleys::MagicLevelSource>("output");
        oscilloscope = magicState.createAndAddObject<foleys::MagicOscilloscope>("waveform");

        analyser = magicState.createAndAddObject<foleys::MagicAnalyser>("analyser");
        magicState.addBackgroundProcessing(analyser);
        */

        presetList = magicState.createAndAddObject<PresetListBox>("presets");
        presetList->onSelectionChanged = [&](int number)
            {
                loadPresetInternal(number);
            };
        magicState.addTrigger("save-preset", [this]
            {
                savePresetInternal();
            });

        magicState.setApplicationSettingsFile(juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
            .getChildFile(ProjectInfo::companyName)
            .getChildFile(ProjectInfo::projectName + juce::String(".settings")));

        magicState.setPlayheadUpdateFrequency(30);

}

Ek0Ka0sAudioProcessor::~Ek0Ka0sAudioProcessor()
{
}

//==============================================================================
const juce::String Ek0Ka0sAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Ek0Ka0sAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Ek0Ka0sAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Ek0Ka0sAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Ek0Ka0sAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Ek0Ka0sAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Ek0Ka0sAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Ek0Ka0sAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String Ek0Ka0sAudioProcessor::getProgramName (int index)
{
    return {};
}

void Ek0Ka0sAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================

void Ek0Ka0sAudioProcessor::savePresetInternal()
{
    presetNode = magicState.getSettings().getOrCreateChildWithName("presets", nullptr);

    juce::ValueTree preset{ "Preset" };
    preset.setProperty("name", "Preset " + juce::String(presetNode.getNumChildren() + 1), nullptr);

    foleys::ParameterManager manager(*this);
    manager.saveParameterValues(preset);

    presetNode.appendChild(preset, nullptr);
}

void Ek0Ka0sAudioProcessor::loadPresetInternal(int index)
{
    presetNode = magicState.getSettings().getOrCreateChildWithName("presets", nullptr);
    auto preset = presetNode.getChild(index);

    foleys::ParameterManager manager(*this);
    manager.loadParameterValues(preset);
}

//==============================================================================
void Ek0Ka0sAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    


    juce::dsp::ProcessSpec spec;                     // Here we create the processSpec type spec variable, 
    spec.maximumBlockSize = samplesPerBlock;         // which holds samples per block of audio, 
    spec.sampleRate = sampleRate;                    // sample rate and number of outputs to be passed to
    spec.numChannels = getTotalNumOutputChannels();  // the prepare function of other DSP modules

    // Filter Modules initialization                    << Like filters here

    MidFilterModule.reset();
    SideFilterModule.reset();
    MidFilterModule.prepare(spec);                   
    SideFilterModule.prepare(spec);

    // LFO initialization

    lfoMid.prepare(spec);
    lfoSide.prepare(spec);

    //lfoMid.setWaveform(Osc::Random);
    //lfoSide.setWaveform(Osc::SH);

    // Delay Modules Initializiation                    << Delays here and so on...

    MidDelayModule.reset();
    SideDelayModule.reset();
    MidDelayModule.prepare(spec);
    SideDelayModule.prepare(spec);


    //SmoothedValues -> Creates linear interpolation in parameter changes.
    
    float rampTime = 0.02;

        //GUI

    Width_Target.reset(sampleRate, rampTime);

    Time_Mid_Target.reset(sampleRate, rampTime);
    Time_Side_Target.reset(sampleRate, rampTime);

    LFO_Depth_Mid_Target.reset(sampleRate, rampTime);
    LFO_Depth_Side_Target.reset(sampleRate, rampTime);
    LFO_Speed_Mid_Target.reset(sampleRate, rampTime);
    LFO_Speed_Side_Target.reset(sampleRate, rampTime);
          
}

void Ek0Ka0sAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Ek0Ka0sAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
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

void Ek0Ka0sAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    double sampeleratero = getSampleRate();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i) // Clears channels from trash data
        buffer.clear (i, 0, buffer.getNumSamples());
    {
            
            auto* channelDataLeft = buffer.getWritePointer(0);
            auto* channelDataRight = buffer.getWritePointer(1);

            for (int sample = 0; sample < buffer.getNumSamples(); ++sample) // Sample Processing
            {                                           
               
               // Input Selection
                                
               Width = Width_Target.getNextValue(); // gets value from ramp
                                                        
               if (Input_Type == "Stereo") // Mid/Side encoding and Stereo Widening   
               {
                   xMidRaw = (2 - Width) * (channelDataLeft[sample] + channelDataRight[sample]);
                   xSideRaw = Width * (channelDataLeft[sample] - channelDataRight[sample]);
               }               

               else   // Or Simply Mid/Side Mixer if input is Mid/Side
               {
                   xMidRaw = channelDataLeft[sample] * (2 - Width);
                   xSideRaw = channelDataRight[sample] * (Width);
               }

               // Volume Control

               xMidRaw *= 0.5;
               xSideRaw *= 0.5;

               // Filtering

               const float xMidFilt = MidFilterModule.processSample(0, xMidRaw);
               const float xSideFilt = SideFilterModule.processSample(0, xSideRaw);

               //LFO Phase Calculation <- needed for LFOs

               lfoValueMid = lfoMid.output(LFO_Speed_Mid_Target.getNextValue(), LFO_Depth_Mid_Target.getNextValue(), &xMidRaw);
               lfoValueSide = lfoSide.output(LFO_Speed_Side_Target.getNextValue(), LFO_Depth_Side_Target.getNextValue(), &xSideRaw);

               //Time Modulation -> Time Ramped Value added to LFOs'; lambda makes value always positive

               Time_Side = [](double Time) {if (Time >= 0) { return Time; } else { return -Time; }  }(Time_Side_Target.getNextValue() + lfoValueSide);
               Time_Mid =  [](double Time) {if (Time >= 0) { return Time; } else { return -Time; }  }(Time_Mid_Target.getNextValue() + lfoValueMid);

               //Mid Delay
                
               float dry = xMidFilt;                                                
               float wet = MidDelayModule.popSample(0, Time_Mid);                   // Read a delayed sample
               MidDelayModule.pushSample(0, xMidFilt + (wet * Feedback_Mid));       // Write a sample into buffer + feedback
               float xMid = (dry * (Send_Mid - 1)) + (wet * Send_Mid);              // Dry + Wet signals

               //Side Delay

               dry = xSideFilt;
               wet = SideDelayModule.popSample(0, Time_Side);
               SideDelayModule.pushSample(0, xSideFilt + (wet * Feedback_Side));
               float xSide = (dry * (Send_Side - 1)) + (wet * Send_Side);

               // Output Handling

               if (Output_Type == "Stereo")
               { 
                    
                    newLeft = xMid + xSide;                                                                 
                    newRight = xMid - xSide;


                    if (Input_Type == "Stereo") // If Stereo i/o -> Volume control
                    {
                        float volumeScale;

                        if (Width <= 1.f)
                        {
                            volumeScale = juce::jmap(Width, 1.0f, 0.0f, 0.0f, -6.0f);
                        }
                        else
                        {
                            volumeScale = juce::jmap(Width, 1.0f, 0.0f, 0.0f, 4.f);
                        }

                        channelDataLeft[sample] = newLeft * juce::Decibels::decibelsToGain(volumeScale);
                        channelDataRight[sample] = newRight * juce::Decibels::decibelsToGain(volumeScale);                     
                    }

                    else
                    {
                    channelDataLeft[sample] = newLeft;
                    channelDataRight[sample] = newRight;
                    }
                    
               }
               else // output == mid/side
               {    // Channels Are Left in Mid and Right in Side                                     
                    channelDataLeft[sample] = xMid;
                    channelDataRight[sample] = xSide;
               }
            }
     
    }
}

//Function called when parameter is changed
void Ek0Ka0sAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    // Here we detect changes made by parameterIDs in the editor for processing in the processBlock.

    // Width and Input Section

    if (parameterID == "stereowidth")

        Width_Target.setTargetValue(newValue);       

    else if (parameterID == "input")
    {
        switch ((int)newValue)
        {
        case 0:
            Input_Type = "Stereo";
            break;
        case 1:
            Input_Type = "Mid/Side";
            break;
        }
    }

    else if (parameterID == "output")
    {
        switch ((int)newValue)
        {
        case 0:
            Output_Type = "Stereo";
            break;
        case 1:
            Output_Type = "Mid/Side";
            break;
        }
    }

    //Filter Section

        //Mid

    else if (parameterID == "cutoffmid")
    { 
        Cut_Off_Mid = newValue;
        MidFilterModule.setCutoffFrequency(Cut_Off_Mid);
    }

    else if (parameterID == "modemid")
    {
        switch ((int)newValue) // Something happens to Input_Type
        {
        case 0:
            MidFilterModule.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
            break;
        case 1:
            MidFilterModule.setType(juce::dsp::StateVariableTPTFilterType::bandpass);
            break;
        case 2:
            MidFilterModule.setType(juce::dsp::StateVariableTPTFilterType::highpass);
            break;
        }
    }

    else if (parameterID == "resonancemid")
    {
        MidFilterModule.setResonance(newValue);
    }
 

        //Side

    else if (parameterID == "cutoffside")
    {
        Cut_Off_Side = newValue;
        SideFilterModule.setCutoffFrequency(Cut_Off_Side);
    }

    else if (parameterID == "modeside")
    {
        switch ((int)newValue) // Something happens to Input_Type
        {
        case 0:
            SideFilterModule.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
            break;
        case 1:
            SideFilterModule.setType(juce::dsp::StateVariableTPTFilterType::bandpass);
            break;
        case 2:
            SideFilterModule.setType(juce::dsp::StateVariableTPTFilterType::highpass);
            break;
        }
    }

    else if (parameterID == "resonanceside")
    {
        SideFilterModule.setResonance(newValue);
    }


    //Delay Section


        //Mid
    else if (parameterID == "sendmid")
    {
        Send_Mid = newValue;
    }
    else if (parameterID == "timemid")
    {
                                      
        Time_Mid_Target.setTargetValue(newValue);

    }
    else if (parameterID == "lfospeedmid")
    {
        LFO_Speed_Mid_Target.setTargetValue(newValue);
    }
    else if (parameterID == "lfodepthmid")
    {
        LFO_Depth_Mid_Target.setTargetValue(newValue);
    }

    else if (parameterID == "waveformmid")
    {
        switch ((int)newValue)
        {
        case 0:
            lfoMid.setWaveform(Osc::Waveform::Sine);
            break;
        case 1:
            lfoMid.setWaveform(Osc::Waveform::Triangle);
            break;
        case 2:
            lfoMid.setWaveform(Osc::Waveform::Sawtooth);
            break;
        case 3:
            lfoMid.setWaveform(Osc::Waveform::Square);
            break;
        case 4:
            lfoMid.setWaveform(Osc::Waveform::Random);
            break;
        case 5:
            lfoMid.setWaveform(Osc::Waveform::SH);
            break;
        }
    }

    else if (parameterID == "feedbackmid")
    {
        Feedback_Mid = newValue;
    }
        //Side
    else if (parameterID == "sendside")
    {
        Send_Side = newValue;
    }
    else if (parameterID == "timeside")
    {
        Time_Side_Target.setTargetValue(newValue);

    }
    else if (parameterID == "lfospeedside")
    {
        LFO_Speed_Side_Target.setTargetValue(newValue);
    }
    else if (parameterID == "lfodepthside")
    {
        LFO_Depth_Side_Target.setTargetValue(newValue);
    }

    else if (parameterID == "waveformside")
    {
        switch ((int)newValue)
        {
        case 0:
            lfoSide.setWaveform(Osc::Waveform::Sine);
            break;
        case 1:
            lfoSide.setWaveform(Osc::Waveform::Triangle);
            break;
        case 2:
            lfoSide.setWaveform(Osc::Waveform::Sawtooth);
            break;
        case 3:
            lfoSide.setWaveform(Osc::Waveform::Square);
            break;
        case 4:
            lfoSide.setWaveform(Osc::Waveform::Random);
            break;
        case 5:
            lfoSide.setWaveform(Osc::Waveform::SH);
            break;
        }
    }

    else if (parameterID == "feedbackside")
    {
        Feedback_Side = newValue;
    }
}


//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Ek0Ka0sAudioProcessor();
}
