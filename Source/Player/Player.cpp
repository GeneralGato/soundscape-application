#include "Player.h"
#include <cmath>

namespace
{
constexpr int resourceSearchDepth = 8;
constexpr float defaultIndividualGain = 0.125f;
constexpr float defaultPan = 0.5f;
constexpr float defaultMixLevel = 0.125f;

juce::File findResourcesFrom(juce::File directory)
{
    for (int depth = 0; depth < resourceSearchDepth && directory.exists(); ++depth)
    {
        auto resources = directory.getChildFile("Resources");
        if (resources.isDirectory())
        {
            return resources;
        }

        auto parent = directory.getParentDirectory();
        if (parent == directory)
        {
            break;
        }

        directory = parent;
    }

    return {};
}

juce::File findResourceRoot()
{
    auto executableDirectory = juce::File::getSpecialLocation(juce::File::currentExecutableFile)
        .getParentDirectory();

    if (auto resources = findResourcesFrom(executableDirectory); resources.isDirectory())
    {
        return resources;
    }

    if (auto resources = findResourcesFrom(juce::File::getCurrentWorkingDirectory()); resources.isDirectory())
    {
        return resources;
    }

    return {};
}

std::string buildResourcePath(const juce::File& resourceRoot, const char* relativePath)
{
    return resourceRoot.getChildFile(relativePath).getFullPathName().toStdString();
}
}

Player::Player()
{
    initialiseDefaults();
    filePathStorage();
}

Player::~Player()
{
    if (loadThread.joinable())
    {
        loadThread.join();
    }
}

void Player::initialiseDefaults()
{
    for (auto& bank : individualBackgroundSoundsGain)
    {
        bank.fill(defaultIndividualGain);
    }

    bankBackgroundSoundsGain.fill(defaultMixLevel);
    individualForegroundSoundsGain.fill(defaultIndividualGain);
    foregroundSoundsPan.fill(defaultPan);
    foregroundPositions.fill(0u);

    for (auto& positions : backgroundPositions)
    {
        positions.fill(0u);
    }

    for (auto& state : backgroundBankEnabled)
    {
        state.store(false);
    }

    for (auto& state : foregroundSoundEnabled)
    {
        state.store(false);
    }
}

bool Player::isIndexValid(int index, int upperBound) const
{
    return index >= 0 && index < upperBound;
}

void Player::loadAsync()
{
    if (loadCompleted.load())
    {
        return;
    }

    bool expected = false;
    if (!loadInProgress.compare_exchange_strong(expected, true))
    {
        return;
    }

    loadCompleted.store(false);
    loadFailed.store(false);

    if (loadThread.joinable())
    {
        loadThread.join();
    }

    loadThread = std::thread([this]
    {
        auto loadedSuccessfully = load();
        loadCompleted.store(loadedSuccessfully);
        loadFailed.store(!loadedSuccessfully);
        loadInProgress.store(false);
    });
}

void Player::setBackgroundBankPlaying(int bankIndex, bool newState)
{
    if (isIndexValid(bankIndex, numBackgroundBanks))
    {
        backgroundBankEnabled[bankIndex].store(newState);
    }
}

void Player::setForegroundSoundPlaying(int soundIndex, bool newState)
{
    if (isIndexValid(soundIndex, numForegroundSounds))
    {
        foregroundSoundEnabled[soundIndex].store(newState);
    }
}

void Player::toggleBackgroundBank(int bankIndex)
{
    setBackgroundBankPlaying(bankIndex, !isBackgroundBankPlaying(bankIndex));
}

void Player::toggleForegroundSound(int soundIndex)
{
    setForegroundSoundPlaying(soundIndex, !isForegroundSoundPlaying(soundIndex));
}

bool Player::isBackgroundBankPlaying(int bankIndex) const
{
    return isIndexValid(bankIndex, numBackgroundBanks) ? backgroundBankEnabled[bankIndex].load() : false;
}

bool Player::isForegroundSoundPlaying(int soundIndex) const
{
    return isIndexValid(soundIndex, numForegroundSounds) ? foregroundSoundEnabled[soundIndex].load() : false;
}

Player::StereoFrame Player::renderNextFrame()
{
    StereoFrame frame;
    float backgroundOutput = 0.0f;

    for (int bankIndex = 0; bankIndex < numBackgroundBanks; ++bankIndex)
    {
        if (!backgroundBankEnabled[bankIndex].load())
        {
            continue;
        }

        float bankOutput = 0.0f;

        for (int layerIndex = 0; layerIndex < numBackgroundLayers; ++layerIndex)
        {
            auto& buffer = backgroundBuffers[bankIndex][layerIndex];
            const auto bufferLength = static_cast<unsigned int>(buffer.getNumSamples());
            if (bufferLength == 0)
            {
                continue;
            }

            auto& position = backgroundPositions[bankIndex][layerIndex];
            const auto sample = buffer.getSample(0, static_cast<int>(position));
            bankOutput += sample * individualBackgroundSoundsGain[bankIndex][layerIndex];

            if (++position >= bufferLength)
            {
                position = 0;
            }
        }

        backgroundOutput += bankOutput * bankBackgroundSoundsGain[bankIndex];
    }

    backgroundOutput *= globalBackgroundSoundsGain;
    frame.left += backgroundOutput;
    frame.right += backgroundOutput;

    for (int soundIndex = 0; soundIndex < numForegroundSounds; ++soundIndex)
    {
        if (!foregroundSoundEnabled[soundIndex].load())
        {
            continue;
        }

        auto& buffer = foregroundBuffers[soundIndex];
        const auto bufferLength = static_cast<unsigned int>(buffer.getNumSamples());
        if (bufferLength == 0)
        {
            continue;
        }

        auto& position = foregroundPositions[soundIndex];
        auto sample = buffer.getSample(0, static_cast<int>(position));

        if (++position >= bufferLength)
        {
            position = 0;
        }

        sample *= individualForegroundSoundsGain[soundIndex] * globalForegroundSoundsGain;

        const auto leftPan = std::sin(juce::MathConstants<float>::halfPi * (1.0f - foregroundSoundsPan[soundIndex]));
        const auto rightPan = std::sin(juce::MathConstants<float>::halfPi * foregroundSoundsPan[soundIndex]);

        frame.left += sample * leftPan;
        frame.right += sample * rightPan;
    }

    return frame;
}

bool Player::loadMonoFile(const juce::File& file,
                          juce::AudioFormatManager& formatManager,
                          juce::AudioBuffer<float>& destination)
{
    auto reader = std::unique_ptr<juce::AudioFormatReader>(formatManager.createReaderFor(file));
    if (reader == nullptr || reader->lengthInSamples <= 0)
    {
        return false;
    }

    const auto numSamples = static_cast<int>(reader->lengthInSamples);
    destination.setSize(1, numSamples);
    destination.clear();
    reader->read(&destination, 0, numSamples, 0, true, false);
    return true;
}

bool Player::load()
{
    if (backgroundFilePathStorage[0][0].empty() || foregroundFilePathStorage[0].empty())
    {
        filePathStorage();
    }

    if (backgroundFilePathStorage[0][0].empty() || foregroundFilePathStorage[0].empty())
    {
        DBG("Could not initialise resource paths.");
        return false;
    }

    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();

    for (int bankIndex = 0; bankIndex < numBackgroundBanks; ++bankIndex)
    {
        for (int layerIndex = 0; layerIndex < numBackgroundLayers; ++layerIndex)
        {
            const auto file = juce::File(backgroundFilePathStorage[bankIndex][layerIndex]);
            if (!file.existsAsFile())
            {
                DBG("Missing background audio file: " + file.getFullPathName());
                return false;
            }

            if (!loadMonoFile(file, formatManager, backgroundBuffers[bankIndex][layerIndex]))
            {
                DBG("Could not create reader for: " + file.getFullPathName());
                return false;
            }

            backgroundPositions[bankIndex][layerIndex] = 0;
        }
    }

    for (int soundIndex = 0; soundIndex < numForegroundSounds; ++soundIndex)
    {
        const auto file = juce::File(foregroundFilePathStorage[soundIndex]);
        if (!file.existsAsFile())
        {
            DBG("Missing foreground audio file: " + file.getFullPathName());
            return false;
        }

        if (!loadMonoFile(file, formatManager, foregroundBuffers[soundIndex]))
        {
            DBG("Could not create reader for: " + file.getFullPathName());
            return false;
        }

        foregroundPositions[soundIndex] = 0;
    }

    return true;
}

void Player::filePathStorage()
{
    auto resourceRoot = findResourceRoot();
    if (!resourceRoot.isDirectory())
    {
        DBG("Resources directory was not found.");
        return;
    }

    static constexpr const char* backgroundRelativePaths[][6] =
    {
        {
            "Soundscapes/Ambient/1. Bass.mp3",
            "Soundscapes/Ambient/2. Guitar.mp3",
            "Soundscapes/Ambient/3. Male Choir.mp3",
            "Soundscapes/Ambient/4. Female Choir.mp3",
            "Soundscapes/Ambient/5. Pluck Synth.mp3",
            "Soundscapes/Ambient/6. Piano.mp3"
        },
        {
            "Soundscapes/Storm/1. Light Rain.mp3",
            "Soundscapes/Storm/2. Breeze.mp3",
            "Soundscapes/Storm/3. Distant Thunder.mp3",
            "Soundscapes/Storm/4. Heavy Rain.mp3",
            "Soundscapes/Storm/5. Cold Wind.mp3",
            "Soundscapes/Storm/6. Close Thunder.mp3"
        },
        {
            "Soundscapes/Beach/1. Wind.mp3",
            "Soundscapes/Beach/2. Distant Waves.mp3",
            "Soundscapes/Beach/3. Pebble Waves.mp3",
            "Soundscapes/Beach/4. Gentle Waves.mp3",
            "Soundscapes/Beach/5. Ship Creak.mp3",
            "Soundscapes/Beach/6. Seagulls.mp3"
        },
        {
            "Soundscapes/Drone/6. Synth.mp3",
            "Soundscapes/Drone/1. Guitar Feedback.mp3",
            "Soundscapes/Drone/4. Blender.mp3",
            "Soundscapes/Drone/5. Bottle.mp3",
            "Soundscapes/Drone/2. Oven.mp3",
            "Soundscapes/Drone/3. White Noise.mp3"
        },
        {
            "Soundscapes/Forest/1. Leaves.mp3",
            "Soundscapes/Forest/2. Leaves.mp3",
            "Soundscapes/Forest/3. Birdsong.mp3",
            "Soundscapes/Forest/4. Birdsong.mp3",
            "Soundscapes/Forest/5. Birdsong.mp3",
            "Soundscapes/Forest/6. Bees.mp3"
        },
        {
            "Soundscapes/Field/1. Grass Rustle.mp3",
            "Soundscapes/Field/2. Summer Wind.mp3",
            "Soundscapes/Field/3. Crickets 1.mp3",
            "Soundscapes/Field/4. Crickets 2.mp3",
            "Soundscapes/Field/5. Cicadas.mp3",
            "Soundscapes/Field/6. Owl.mp3"
        }
    };

    for (int bankIndex = 0; bankIndex < numBackgroundBanks; ++bankIndex)
    {
        for (int layerIndex = 0; layerIndex < numBackgroundLayers; ++layerIndex)
        {
            backgroundFilePathStorage[bankIndex][layerIndex] = buildResourcePath(resourceRoot, backgroundRelativePaths[bankIndex][layerIndex]);
        }
    }

    static constexpr const char* foregroundRelativePaths[] =
    {
        "1. Light Stream.mp3",
        "2. Stream.mp3",
        "3. Waterfall.mp3",
        "4. Campfire.mp3",
        "5. Cat Purr.mp3",
        "6. Crows.mp3"
    };

    for (int soundIndex = 0; soundIndex < numForegroundSounds; ++soundIndex)
    {
        foregroundFilePathStorage[soundIndex] = buildResourcePath(resourceRoot, foregroundRelativePaths[soundIndex]);
    }
}
