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

struct ExtendedTabbedButtonBar : juce::TabbedButtonBar, juce::DragAndDropTarget, juce::DragAndDropContainer
{
    ExtendedTabbedButtonBar();
    bool isInterestedInDragSource(const SourceDetails& dragSourceDetails) override;
    /*
        Drag-to-reorder is accomplished by the TabbedButtonBar being a dragAndDrop Target and a DragAndDropContainer, and by listening to mouse events on each TabBarButton.

        The sequence flow is:
        mouseDown on a tab. This starts the DragAndDropContainer responding to mouseDrag events.
        first, itemDragEnter is called when the first mouseEvent happens.
        as the mouse is moved, itemDragMove() is called. the tabBarButtons are constrained to the bounds of the ExtendedTabbedButtonBar, so they'll never be dragged outside of it.

        itemDragMove() checks the x coordinate of the item being dragged and compares it with its neighbors.
        tab indexes are swapped if a tab crosses over the middle of another tab.
    */
    void itemDragEnter(const SourceDetails& dragSourceDetails) override;
    void itemDragMove(const SourceDetails& dragSourceDetails) override;
    void itemDragExit(const SourceDetails& dragSourceDetails) override;
    void itemDropped(const SourceDetails& dragSourceDetails) override;

    void mouseDown(const juce::MouseEvent& e) override;

    juce::TabBarButton* createTabButton(const juce::String& tabName, int tabIndex) override;

    struct Listener
    {
        virtual ~Listener() = default;
        virtual void tabOrderChanged(CAudioPluginAudioProcessor::DSP_Order newOrder) = 0;
    };

    void addListener(Listener* l);
    void removeListener(Listener* l);

private:
    juce::TabBarButton* findDraggedItem(const SourceDetails& dragSourceDetails);
    int findDraggedItemIndex(const SourceDetails& dragSourceDetails);
    juce::Array<juce::TabBarButton*> getTabs();

    juce::ListenerList<Listener> listeners;
    juce::ScaledImage dragImage;
};

struct HorizontalConstrainer : juce::ComponentBoundsConstrainer
{
    HorizontalConstrainer(std::function<juce::Rectangle<int>()> confinerBoundsGetter,
                          std::function<juce::Rectangle<int>()> confineeBoundsGetter);

    void checkBounds(juce::Rectangle<int>& bounds,
        const juce::Rectangle<int>& previousBounds,
        const juce::Rectangle<int>& limits,
        bool isStretchingTop,
        bool isStretchingLeft,
        bool isStretchingBottom,
        bool isStretchingRight) override;

private:
    std::function<juce::Rectangle<int>()> boundsToConfineToGetter;
    std::function<juce::Rectangle<int>()> boundsOfConfineeGetter;
};

struct ExtendedTabBarButton : juce::TabBarButton
{
    ExtendedTabBarButton(const juce::String& name, juce::TabbedButtonBar& owner, CAudioPluginAudioProcessor::DSP_Option dspOption);
    juce::ComponentDragger dragger;
    std::unique_ptr<HorizontalConstrainer> constrainer;

    void mouseDown(const juce::MouseEvent& e) override;

    void mouseDrag(const juce::MouseEvent& e) override;
    CAudioPluginAudioProcessor::DSP_Option getOption() const { return option; }

    int getBestTabLength(int depth) override;
private:
    CAudioPluginAudioProcessor::DSP_Option option;
};

struct DSP_Gui : juce::Component
{
    DSP_Gui(CAudioPluginAudioProcessor& p);

    void resized() override;
    void paint(juce::Graphics& g) override;

    void rebuildInterface(std::vector<juce::RangedAudioParameter*> params);

    CAudioPluginAudioProcessor& processor;

    std::vector <std::unique_ptr<juce::Slider>> sliders;
    std::vector <std::unique_ptr<juce::ComboBox>> comboBoxes;
    std::vector <std::unique_ptr<juce::Button>> buttons;

    std::vector <std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sliderAttachments;
    std::vector <std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> comboboxAttachments;
    std::vector <std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> buttonAttachments;
};

//==============================================================================
/**
*/
class CAudioPluginAudioProcessorEditor  : public juce::AudioProcessorEditor, ExtendedTabbedButtonBar::Listener, juce::Timer
{
public:
    CAudioPluginAudioProcessorEditor (CAudioPluginAudioProcessor&);
    ~CAudioPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void tabOrderChanged(CAudioPluginAudioProcessor::DSP_Order newOrder) override;
    void timerCallback() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    CAudioPluginAudioProcessor& audioProcessor;
    DSP_Gui dspGUI{ audioProcessor };
    ExtendedTabbedButtonBar tabbedComponent;

    void addTabsFromDSPOrder(CAudioPluginAudioProcessor::DSP_Order);
    void rebuildInterface();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CAudioPluginAudioProcessorEditor)
};
