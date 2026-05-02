#pragma once

#include <JuceHeader.h>
#include "../Player/Player.h"

/** Class containing all audio processes */

class Processor : public juce::AudioIODeviceCallback
{
public:
	/** Constructor of Processor class */
	Processor();

	/** Destructor */
	~Processor();

	/** Returns the Player */
	Player* getPlayer() { return &player; }

	/** Returns the audio device manager */
	juce::AudioDeviceManager& getAudioDeviceManager() { return audioDeviceManager; }
	void audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
		int numInputChannels,
		float* const* outputChannelData,
		int numOutputChannels,
		int numSamples,
		const juce::AudioIODeviceCallbackContext& context) override;
	void audioDeviceAboutToStart(juce::AudioIODevice* device) override;
	void audioDeviceStopped() override;
	
private:
	juce::AudioDeviceManager audioDeviceManager;
	Player player;

};