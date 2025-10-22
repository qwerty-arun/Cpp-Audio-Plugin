/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

static juce::String getDSPOptionName(CAudioPluginAudioProcessor::DSP_Option option)
{
    switch (option)
    {
        case CAudioPluginAudioProcessor::DSP_Option::Phaser:
            return "PHASER";
            break;
        case CAudioPluginAudioProcessor::DSP_Option::Chorus:
            return "CHORUS";
            break;
        case CAudioPluginAudioProcessor::DSP_Option::OverDrive:
            return "OVERDRIVE";
            break;
        case CAudioPluginAudioProcessor::DSP_Option::LadderFilter:
            return "LADDERFILTER";
            break;
        case CAudioPluginAudioProcessor::DSP_Option::GeneralFilter:
            return "GEN FILTER";
            break;
        case CAudioPluginAudioProcessor::DSP_Option::END_OF_LIST:
            jassertfalse;
    }
    return "NO SELECTION";
}

//==============================================================================
CAudioPluginAudioProcessorEditor::CAudioPluginAudioProcessorEditor (CAudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    dspOrderButton.onClick = [this]()
        {
            juce::Random r;
            CAudioPluginAudioProcessor::DSP_Order dspOrder;

            auto range = juce::Range<int>(static_cast<int>(CAudioPluginAudioProcessor::DSP_Option::Phaser), 
                                          static_cast<int>(CAudioPluginAudioProcessor::DSP_Option::END_OF_LIST));

            tabbedComponent.clearTabs();
            for (auto& v : dspOrder)
            {
                auto entry = r.nextInt(range);
                v = static_cast<CAudioPluginAudioProcessor::DSP_Option>(entry);
                tabbedComponent.addTab(getDSPOptionName(v), juce::Colours::white, -1);
            }
            DBG(juce::Base64::toBase64(dspOrder.data(), dspOrder.size()));
            //jassertfalse;

            audioProcessor.dspOrderFifo.push(dspOrder);
        };

    addAndMakeVisible(dspOrderButton);
    addAndMakeVisible(tabbedComponent);
    setSize (400, 300);
}

CAudioPluginAudioProcessorEditor::~CAudioPluginAudioProcessorEditor()
{
}

//==============================================================================
void CAudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void CAudioPluginAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    auto bounds = getLocalBounds();
    dspOrderButton.setBounds(bounds.removeFromTop(30).withSizeKeepingCentre(150, 30));
    bounds.removeFromTop(10);
    tabbedComponent.setBounds(bounds.withHeight(30));
}
