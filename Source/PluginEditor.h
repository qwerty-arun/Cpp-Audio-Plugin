/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <LookAndFeel.h>
#include <CustomButtons.h> //For Powerbutton

template<typename ParamsContainer>
static juce::AudioParameterBool* findBypassParam(const ParamsContainer& params)
{
    for (auto p : params)
    {
        if (auto bypass = dynamic_cast<juce::AudioParameterBool*>(p))
        {
            if (bypass->name.containsIgnoreCase("bypass"))
            {
                return bypass;
            }
        }
    }
    return nullptr;
}

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
        virtual void selectedTabChanged(int newCurrentTabIndex) = 0;
    };

    void addListener(Listener* l);
    void removeListener(Listener* l);

    void currentTabChanged(int newCurrentTabIndex, const juce::String& newCurrentTabName) override;
    void setTabColours();
private:
    juce::TabBarButton* findDraggedItem(const SourceDetails& dragSourceDetails);
    int findDraggedItemIndex(const SourceDetails& dragSourceDetails);
    juce::Array<juce::TabBarButton*> getTabs();
    
    bool reorderTabsAfterDrop();
    juce::ListenerList<Listener> listeners;
    juce::ScaledImage dragImage;
    juce::Array<juce::TabBarButton*> tabs;
    juce::Point<int> previousDraggedTabCenterPosition;
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

struct RotarySliderWithLabels;

struct DSP_Gui : juce::Component
{
    DSP_Gui(CAudioPluginAudioProcessor& p);

    void resized() override;
    void paint(juce::Graphics& g) override;

    void rebuildInterface(std::vector<juce::RangedAudioParameter*> params);
    void toggleSliderEnablement(bool enabled);

    CAudioPluginAudioProcessor& processor;

    std::vector <std::unique_ptr<RotarySliderWithLabels>> sliders;
    std::vector <std::unique_ptr<juce::ComboBox>> comboBoxes;
    std::vector <std::unique_ptr<juce::Button>> buttons;

    std::vector <std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sliderAttachments;
    std::vector <std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> comboboxAttachments;
    std::vector <std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> buttonAttachments;

    std::vector<juce::RangedAudioParameter*> currentParams;
};

//==============================================================================
/**
*/

struct PowerButtonWithParam : PowerButton
{
    PowerButtonWithParam(juce::AudioParameterBool* p);
    void changeAttachment(juce::AudioParameterBool* p);
    juce::AudioParameterBool* getParam() const { return param; }
private:
    std::unique_ptr<juce::ButtonParameterAttachment> attachment;
    juce::AudioParameterBool* param;
};

class CAudioPluginAudioProcessorEditor  : public juce::AudioProcessorEditor, ExtendedTabbedButtonBar::Listener, juce::Timer
{
public:
    CAudioPluginAudioProcessorEditor (CAudioPluginAudioProcessor&);
    ~CAudioPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void tabOrderChanged(CAudioPluginAudioProcessor::DSP_Order newOrder) override;
    void selectedTabChanged(int newCurrentTabIndex) override;

    void timerCallback() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    CAudioPluginAudioProcessor& audioProcessor;
    LookAndFeel lookAndFeel;
    DSP_Gui dspGUI{ audioProcessor };
    ExtendedTabbedButtonBar tabbedComponent;
    
    static constexpr int meterWidth = 80;
    static constexpr int fontHeight = 24;
    static constexpr int tickIndent = 8;
    static constexpr int meterChanWidth = 24;
    static constexpr int ioControlSize = 100;

    std::unique_ptr<RotarySliderWithLabels> inGainControl, outGainControl;
    std::unique_ptr<juce::SliderParameterAttachment> inGainAttachment, outGainAttachment;

    std::unique_ptr<juce::ParameterAttachment> selectedTabAttachment;

    void addTabsFromDSPOrder(CAudioPluginAudioProcessor::DSP_Order);
    void rebuildInterface();
    void refreshDSPGUIControlEnablement(PowerButtonWithParam* button);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CAudioPluginAudioProcessorEditor)
};
