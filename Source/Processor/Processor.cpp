#include "Processor.h"

Processor::Processor()
{
    auto errorMessage = audioDeviceManager.initialiseWithDefaultDevices(0, 2);
    if (!errorMessage.isEmpty())
    { 
        DBG(errorMessage);
    }
    audioDeviceManager.addAudioCallback(this);
}

Processor::~Processor()
{
    audioDeviceManager.removeAudioCallback(this);
}

void Processor::audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
                                                 int numInputChannels,
                                                 float* const* outputChannelData,
                                                 int numOutputChannels,
                                                 int numSamples,
                                                 const juce::AudioIODeviceCallbackContext& context)
{
    juce::ignoreUnused(inputChannelData, numInputChannels, context);
    juce::ScopedNoDenormals noDenormals;

    const auto totalSamples = numSamples;

    auto* outL = (numOutputChannels > 0 && outputChannelData != nullptr) ? outputChannelData[0] : nullptr;
    auto* outR = (numOutputChannels > 1 && outputChannelData != nullptr) ? outputChannelData[1] : nullptr;

    for (int channel = 0; channel < numOutputChannels; ++channel)
    {
        if (auto* output = outputChannelData[channel])
        {
            juce::FloatVectorOperations::clear(output, totalSamples);
        }
    }

    player.renderNextBlock(outL, outR, totalSamples);
}

void Processor::audioDeviceAboutToStart(juce::AudioIODevice* device)
{
    player.prepareToPlay(device != nullptr ? device->getCurrentSampleRate() : 44100.0,
                         device != nullptr ? device->getCurrentBufferSizeSamples() : 0);
}

void Processor::audioDeviceStopped()
{
}
