/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

/*
    https://forum.juce.com/t/draggabletabbedcomponent/13265/5?u=matkatmusic
    "You can create a subclass of TabbedButtonBar which is also a DragAndDropTarget. And you can create custom tab buttons which allow themselves to be dragged.
*/

struct ExtendedTabbedButtonBar : juce::TabbedButtonBar, juce::DragAndDropTarget
{
    ExtendedTabbedButtonBar() : juce::TabbedButtonBar(juce::TabbedButtonBar::Orientation::TabsAtTop) { }
    
    bool isInterestedInDragSource(const SourceDetails& dragSourceDetails) override { return false; }
    void itemDropped(const SourceDetails& dragSourceDetails) override { }
};

struct ExtendedTabBarButton : juce::TabBarButton
{

};
//==============================================================================
/**
*/
class CAudioPluginAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    CAudioPluginAudioProcessorEditor (CAudioPluginAudioProcessor&);
    ~CAudioPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    CAudioPluginAudioProcessor& audioProcessor;

    juce::TextButton dspOrderButton{ "dsp order " };

    ExtendedTabbedButtonBar tabbedComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CAudioPluginAudioProcessorEditor)
};
