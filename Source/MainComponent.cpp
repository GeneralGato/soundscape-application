#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent(Processor& a)
{
    setSize(392, 680);
    DBG("Output");
    
    //Sets instances of PlayerGUI in our main window
    playerGUI.setPlayer(a.getPlayer());
    a.getPlayer()->loadAsync();

    //Makes playerGUI visible
    addAndMakeVisible(playerGUI);
}

MainComponent::~MainComponent()
{
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setFont (juce::Font (16.0f));
    g.setColour (juce::Colours::white);
}

void MainComponent::resized()
{
    // This is called when the MainComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
    playerGUI.setBounds(getLocalBounds());
}
