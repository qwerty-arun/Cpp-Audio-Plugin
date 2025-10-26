/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
//#include "../SimpleMultiBandComp/Source/DSP/Fifo.h"
#include <Fifo.h>

//==============================================================================
/**
*/
class CAudioPluginAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    CAudioPluginAudioProcessor();
    ~CAudioPluginAudioProcessor() override;

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

    //strongly typed enumeration (scoped enumeration)
    enum class DSP_Option
    {
        Phaser,
        Chorus,
        OverDrive,
        LadderFilter,
        GeneralFilter,
        END_OF_LIST
    };

    //static function
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    //declare an instance
    juce::AudioProcessorValueTreeState apvts{ *this, nullptr, "Settings", createParameterLayout() };

    //array alias
    using DSP_Order = std::array < DSP_Option, static_cast<size_t>(DSP_Option::END_OF_LIST)>;

    //FIFO Instance
    SimpleMBComp::Fifo<DSP_Order> dspOrderFifo, restoreDspOrderFifo;

    /*
        Phaser:
            Rate: Hz
            Depth: 0 to 1
            Ceenter freq: Hz
            Feedback: -1 to +1
            Mix: 0 to 1
    */

    //cached audio parameter pointers for each of the parameters above.
    juce::AudioParameterFloat* phaserRateHz = nullptr;
    juce::AudioParameterFloat* phaserCenterFreqHz = nullptr;
    juce::AudioParameterFloat* phaserDepthPercent = nullptr;
    juce::AudioParameterFloat* phaserFeedbackPercent = nullptr;
    juce::AudioParameterFloat* phaserMixPercent = nullptr;
    juce::AudioParameterBool* phaserBypass = nullptr;

    juce::AudioParameterFloat* chorusRateHz = nullptr;
    juce::AudioParameterFloat* chorusDepthPercent = nullptr;
    juce::AudioParameterFloat* chorusCenterDelayMs = nullptr;
    juce::AudioParameterFloat* chorusFeedbackPercent = nullptr;
    juce::AudioParameterFloat* chorusMixPercent = nullptr;
    juce::AudioParameterBool* chorusBypass = nullptr;

    juce::AudioParameterFloat* overdriveSaturation = nullptr;
    juce::AudioParameterBool* overdriveBypass = nullptr;

    juce::AudioParameterChoice* ladderFilterMode = nullptr;
    juce::AudioParameterFloat* ladderFilterCutoffHz = nullptr;
    juce::AudioParameterFloat* ladderFilterResonance = nullptr;
    juce::AudioParameterFloat* ladderFilterDrive = nullptr;
    juce::AudioParameterBool* ladderFilterBypass= nullptr;

    juce::AudioParameterChoice* generalFilterMode = nullptr;
    juce::AudioParameterFloat* generalFilterFreqHz = nullptr;
    juce::AudioParameterFloat* generalFilterQuality = nullptr;
    juce::AudioParameterFloat* generalFilterGain = nullptr;
    juce::AudioParameterBool* generalFilterBypass= nullptr;

    juce::AudioParameterInt* selectedTab = nullptr;

    juce::SmoothedValue<float>
        phaserRateHzSmoother,
        phaserCenterFreqHzSmoother,
        phaserDepthPercentSmoother,
        phaserFeedbackPercentSmoother,
        phaserMixPercentSmoother,
        chorusRateHzSmoother,
        chorusDepthPercentSmoother,
        chorusCenterDelayMsSmoother,
        chorusFeedbackPercentSmoother,
        chorusMixPercentSmoother,
        overdriveSaturationSmoother,
        ladderFilterCutoffHzSmoother,
        ladderFilterResonanceSmoother,
        ladderFilterDriveSmoother,
        generalFilterFreqHzSmoother,
        generalFilterQualitySmoother,
        generalFilterGainSmoother;

    juce::Atomic<bool> guiNeedsLatestDspOrder{ false };

    enum class GeneralFilterMode
    {
        Peak,
        Bandpass,
        Notch,
        Allpass,
        END_OF_LIST
    };

    std::vector<juce::RangedAudioParameter*> getParamsForOption(DSP_Option option);

private:
    DSP_Order dspOrder; // Create an object
    
    template<typename DSP> // class template, we can create versions for different DSP effect types
    struct DSP_Choice : juce::dsp::ProcessorBase
    {
        void prepare(const juce::dsp::ProcessSpec& spec) override
        {
            dsp.prepare(spec);
        }
        void process(const juce::dsp::ProcessContextReplacing<float>& context) override
        {
            dsp.process(context);
        }
        void reset() override
        {
            dsp.reset();
        }

        DSP dsp;
    };

    struct MonoChannelDSP
    {
        MonoChannelDSP(CAudioPluginAudioProcessor& proc) : p(proc) {}
        DSP_Choice<juce::dsp::DelayLine<float>> delay;
        DSP_Choice<juce::dsp::Phaser<float>> phaser;
        DSP_Choice<juce::dsp::Chorus<float>> chorus;
        DSP_Choice<juce::dsp::LadderFilter<float>> overdrive, ladderFilter;
        DSP_Choice<juce::dsp::IIR::Filter<float>> generalFilter;

        void prepare(const juce::dsp::ProcessSpec& spec);

        void updateDSPFromParams();

        void process(juce::dsp::AudioBlock<float> block, const DSP_Order& dspOrder);

    private:
        CAudioPluginAudioProcessor& p;

        GeneralFilterMode filterMode = GeneralFilterMode::END_OF_LIST;
        float filterFreq = 0.f, filterQ = 0.f, filterGain = -100.f;
    };

    MonoChannelDSP leftChannel{ *this };
    MonoChannelDSP rightChannel{ *this };

    struct ProcessState
    {
        juce::dsp::ProcessorBase* processor = nullptr;
        bool bypassed = false;
    };

    using DSP_Pointers = std::array<ProcessState, static_cast<size_t>(DSP_Option::END_OF_LIST)>;

# define VERIFY_BYPASS_FUNCTIONALITY false

    template<typename ParamType, typename Params, typename Funcs> 
    void initCachedParams(Params paramsArray, Funcs funcsArray)
    {
        for (size_t i = 0; i < paramsArray.size(); ++i)
        {
            auto ptrToParamPtr = paramsArray[i];
            *ptrToParamPtr = dynamic_cast<ParamType>(apvts.getParameter(funcsArray[i]()));
            jassert(*ptrToParamPtr != nullptr);
        }
    }

    std::vector<juce::SmoothedValue<float>*> getSmoothers();

    enum class SmootherUpdateMode
    {
        initialize,
        liveInRealtime
    };

    void updateSmoothersFromParams(int numSamplesToSkip, SmootherUpdateMode init);
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CAudioPluginAudioProcessor)
};
