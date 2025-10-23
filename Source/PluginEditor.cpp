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

HorizontalConstrainer::HorizontalConstrainer(std::function<juce::Rectangle<int>()> confinerBoundsGetter,
                                             std::function<juce::Rectangle<int>()> confineeBoundsGetter) 
                                                : 
    boundsToConfineToGetter(std::move(confinerBoundsGetter)),
    boundsOfConfineeGetter(std::move(confineeBoundsGetter))
{

}

void HorizontalConstrainer::checkBounds(juce::Rectangle<int>& bounds,
    const juce::Rectangle<int>& previousBounds,
    const juce::Rectangle<int>& limits,
    bool isStretchingTop,
    bool isStretchingLeft,
    bool isStretchingBottom,
    bool isStretchingRight)
{
    /*
    * 'bounds' is the bounding box that we are TRYING to set componentToConfine to.
    * we only want to support horizontal dragging within the TabButtonBar.
    * so, retain the existing Y position given to the TabBarButton by the TabbedButtonBar when the button was created.
    */
    bounds.setY(previousBounds.getY());
    /*
    * the X position needs to be limited to the left and right side of the owning TabbedButtonBar.
    * however, to prevent the right side of the TabBarButton from being dragged outside the bounds of the TabbedButtonBar, we must subtract the width of this button from the right side of the TabbedButtonnBar
    * 
    * in order for this to work, we need to know the bounds of both the TabbedButtonBar and the TabBarButton.
    * hence, loose coupling using lambda getter functions via the constructor parameters.
    * Loose coupling is preferred vs tight coupling.
    */

    if (boundsToConfineToGetter != nullptr &&
        boundsOfConfineeGetter != nullptr)
    {
        auto boundsToConfineTo = boundsToConfineToGetter();
        auto boundsOfConfinee = boundsOfConfineeGetter();

        bounds.setX(juce::jlimit(boundsToConfineTo.getX(),
            boundsToConfineTo.getRight() - boundsOfConfinee.getWidth(),
            bounds.getX()));
    }
    else
    {
        bounds.setX(juce::jlimit(limits.getX(),
            limits.getY(),
            bounds.getX()));
    }

}

ExtendedTabBarButton::ExtendedTabBarButton(const juce::String& name, juce::TabbedButtonBar& owner) : juce::TabBarButton(name, owner)
{
    constrainer = std::make_unique<HorizontalConstrainer>([&owner]() { return owner.getLocalBounds(); },
                                                            [this]() { return getBounds();});
    constrainer->setMinimumOnscreenAmounts(0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff);
}

juce::TabBarButton* ExtendedTabbedButtonBar::createTabButton(const juce::String& tabName, int tabIndex)
{
    return new ExtendedTabBarButton(tabName, *this);
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
