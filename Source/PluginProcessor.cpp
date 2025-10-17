/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

auto getPhaserRateName() { return juce::String("Phaser RateHz"); }
auto getPhaserCenterFreqName() { return juce::String("Phaser Center FreqHz"); }
auto getPhaserDepthName() { return juce::String("Phaser Depth %"); }
auto getPhaserFeedbackName() { return juce::String("Phaser Feedback %"); }
auto getPhaserMixName() { return juce::String("Phaser Mix %"); }

auto getChorusRateName() { return juce::String("Chorus RateHz"); }
auto getChorusDepthName() { return juce::String("Chorus Depth %"); }
auto getChorusCenterDelayName() { return juce::String("Chorus Center Delay Ms"); }
auto getChorusFeedbackName() { return juce::String("Chorus Feedback %"); }
auto getChorusMixName() { return juce::String("Chorus Mix %"); }


//==============================================================================
CAudioPluginAudioProcessor::CAudioPluginAudioProcessor()
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
    /*
    * array of pointers to pointers->each element is a pointer to one of your member variables, like &phaserRateHz,
    * which itself is a juce::AudioParameterFloat*
    */
    auto phaserParams = std::array
    {
        &phaserRateHz,
        &phaserCenterFreqHz,
        &phaserDepthPercent,
        &phaserFeedbackPercent,
        &phaserMixPercent
    };

    // array of function pointers -> each one returns the string ID of a parameter
    auto phaserFuncs = std::array
    {
        &getPhaserRateName,
        &getPhaserCenterFreqName,
        &getPhaserDepthName,
        & getPhaserFeedbackName,
        &getPhaserMixName
    };

    for (size_t i = 0; i < phaserParams.size(); ++i)
    {
        auto ptrToParamPtr = phaserParams[i];
        *ptrToParamPtr = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter( phaserFuncs[i]() ));
        jassert(*ptrToParamPtr != nullptr);
    }

}

CAudioPluginAudioProcessor::~CAudioPluginAudioProcessor()
{
}

//==============================================================================
const juce::String CAudioPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool CAudioPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool CAudioPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool CAudioPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double CAudioPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int CAudioPluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int CAudioPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void CAudioPluginAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String CAudioPluginAudioProcessor::getProgramName (int index)
{
    return {};
}

void CAudioPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void CAudioPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void CAudioPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool CAudioPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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


juce::AudioProcessorValueTreeState::ParameterLayout CAudioPluginAudioProcessor::createParameterLayout() {
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    //name = nameFunction();
    //layout.add(std::make_unique<juce::AudioParameterFloat>(
    //    juce::ParameterID{name, versionHint},
    //    name,
    //    parameterRange,
    //    defaultValue,
    //    unitSuffix
    //));

    const int versionHint = 1;
    /*
    Phaser:
        Rate: Hz
        Depth: 0 to 1
        Center freq: Hz
        Feedback: -1 to +1
        Mix: 0 to 1
    */

    //phaser rate
    auto name = getPhaserRateName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ name, versionHint },
        name,
        juce::NormalisableRange<float>(0.01f, 2.f, 0.01f, 1.f),
        0.2f,
        "Hz"));

    //phaser depth: 0 to 1
    name = getPhaserDepthName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ name, versionHint },
        name,
        juce::NormalisableRange<float>(0.01f, 1.f, 0.01f, 1.f),
        0.05f,
        "%"));

    //phaser center freq: audio Hz
    name = getPhaserCenterFreqName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ name, versionHint },
        name,
        juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 1.f),
        1000.f,
        "Hz"));

    //phaser feedback: -1 to 1
    name = getPhaserFeedbackName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ name, versionHint },
        name,
        juce::NormalisableRange<float>(-1.f, 1.f, 0.01f, 1.f),
        0.0f,
        "%"));

    //phaser mix: 0 - 1
    name = getPhaserMixName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ name, versionHint },
        name,
        juce::NormalisableRange<float>(0.01f, 1.f, 0.01f, 1.f),
        0.05f,
        "%"));

    /*
    Chorus:
    Rate: Hz
    Depth: 0 to 1
    Center delay: ms (1 to 100)
    Feedback: -1 to +1
    Mix: 0 to 1
    */

    return layout;
}

void CAudioPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
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

    //[DONE]: add APVTs
    //TODO: create audio parameters for all dsp choices
    //TODO: update DSP here from audio parameters
    //TODO: save/load settingss
    //TODO: save/load DSP order
    //TODO: Drag-To-Recorder GUI
    //TODO: GUI design for each DSP instance?
    //TODO: metering
    //TODO: prepare all DSP
    //TODO: wet/dry know [BONUS]
    //TODO: mono & stereo versions [mono is BONUS]
    //TODO: modulators [BONUS]
    //TODO: thread-safe filter updating [BONUS]
    //TODO: pre/post filtering [BONUS]
    //TODO: delay module [BONUS]

    // default instance
    auto newDSPOrder = DSP_Order();

    // try to pull
    while (dspOrderFifo.pull(newDSPOrder)) {

    }

    // if you pulled, replace dspOrder;
    if (newDSPOrder != DSP_Order()) {
        dspOrder = newDSPOrder;
    }

    // now convert dspOrder into an array of pointers
    DSP_Pointers dspPointers;

    for (size_t i = 0; i < dspPointers.size(); i++)
    {
        switch (dspOrder[i]) {
            case DSP_Option::Phase:
                dspPointers[i] = &phaser;
                break;
            case DSP_Option::Chorus:
                dspPointers[i] = &chorus;
                break;
            case DSP_Option::OverDrive:
                dspPointers[i] = &overdrive;
                break;
            case DSP_Option::LadderFilter:
                dspPointers[i] = &ladderFilter;
                break;
            case DSP_Option::END_OF_LIST:
                jassertfalse;
                break; 
        }
    }

    // now process
    auto block = juce::dsp::AudioBlock<float>(buffer);
    auto context = juce::dsp::ProcessContextReplacing<float>(block);

    for (size_t i = 0; i < dspPointers.size(); ++i)
    {
        if (dspPointers[i] != nullptr) {
            dspPointers[i]->process(context);
        }
    }
}

//==============================================================================
bool CAudioPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* CAudioPluginAudioProcessor::createEditor()
{
    return new CAudioPluginAudioProcessorEditor (*this);
}

//==============================================================================
void CAudioPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void CAudioPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CAudioPluginAudioProcessor();
}
