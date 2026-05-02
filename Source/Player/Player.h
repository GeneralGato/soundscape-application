#pragma once

#include <JuceHeader.h>
#include <array>
#include <thread>

class Player
{
public:
	static constexpr int numBackgroundBanks = 6;
	static constexpr int numBackgroundLayers = 6;
	static constexpr int numForegroundSounds = 6;

	struct StereoFrame
	{
		float left = 0.0f;
		float right = 0.0f;
	};

	using BackgroundLayerGains = std::array<std::array<float, numBackgroundLayers>, numBackgroundBanks>;
	using BackgroundPathGrid = std::array<std::array<std::string, numBackgroundLayers>, numBackgroundBanks>;

	/** Constructor for the initialisation of Player */
	Player();

	/** Destructor of the Player class */
	~Player();

	/** Sets playback state for a background bank */
	void setBackgroundBankPlaying(int bankIndex, bool newState);

	/** Sets playback state for a foreground sound */
	void setForegroundSoundPlaying(int soundIndex, bool newState);

	/** Toggles a background bank */
	void toggleBackgroundBank(int bankIndex);

	/** Toggles a foreground sound */
	void toggleForegroundSound(int soundIndex);

	/** Returns the playback state for a background bank */
	bool isBackgroundBankPlaying(int bankIndex) const;

	/** Returns the playback state for a foreground sound */
	bool isForegroundSoundPlaying(int soundIndex) const;

	/** Renders one stereo output frame */
	StereoFrame renderNextFrame();

	/** Method that stores file paths in strings */
	void filePathStorage();

	/** Loads an audio file into the Player */
	bool load();

	/** Starts background loading for the audio library if needed */
	void loadAsync();

	/** Returns true when the audio library has finished loading */
	bool isLoaded() const { return loadCompleted.load(); }

	/** Returns true while the audio library is loading */
	bool isLoading() const { return loadInProgress.load(); }

	/** Returns true when the audio library failed to load */
	bool didLoadFail() const { return loadFailed.load(); }

	/** Stores the global gain for background sounds */
	float globalBackgroundSoundsGain = 1.0f;

	/** Stores the global gain for foreground sounds */
	float globalForegroundSoundsGain = 1.0f;

	/** Keeps tabs on the first background sounds volume */
	BackgroundLayerGains individualBackgroundSoundsGain{};

	/** String storage of background sounds file paths */
	BackgroundPathGrid backgroundFilePathStorage{};

	/** String storage of foreground sounds file paths */
	std::array<std::string, numForegroundSounds> foregroundFilePathStorage{};

	/** Stores the gain values of Background Sounds */
	std::array<float, numBackgroundBanks> bankBackgroundSoundsGain{};
	
	/** Stores the gain values of Foreground Sounds */
	std::array<float, numForegroundSounds> individualForegroundSoundsGain{};

	/** Stores pan values for Foreground Sounds */
	std::array<float, numForegroundSounds> foregroundSoundsPan{};

private:
	using BackgroundEnableStates = std::array<std::atomic<bool>, numBackgroundBanks>;
	using ForegroundEnableStates = std::array<std::atomic<bool>, numForegroundSounds>;
	using BackgroundPositions = std::array<std::array<unsigned int, numBackgroundLayers>, numBackgroundBanks>;
	using ForegroundPositions = std::array<unsigned int, numForegroundSounds>;
	using BackgroundBuffers = std::array<std::array<juce::AudioBuffer<float>, numBackgroundLayers>, numBackgroundBanks>;
	using ForegroundBuffers = std::array<juce::AudioBuffer<float>, numForegroundSounds>;

	bool isIndexValid(int index, int upperBound) const;
	void initialiseDefaults();
	bool loadMonoFile(const juce::File& file, juce::AudioFormatManager& formatManager, juce::AudioBuffer<float>& destination);

	/** Indication of on/off states of background sounds */
	BackgroundEnableStates backgroundBankEnabled{};

	/** Indication of on/off states of foreground sounds */
	ForegroundEnableStates foregroundSoundEnabled{};

	/** Stores buffer position of background sounds */
	BackgroundPositions backgroundPositions{};

	/** Stores buffer position of foreground sounds */
	ForegroundPositions foregroundPositions{};

	/** Audio buffer for each of the background sounds */
	BackgroundBuffers backgroundBuffers{};

	/** Audio buffer for each foreground sound */
	ForegroundBuffers foregroundBuffers{};

	std::atomic<bool> loadCompleted{ false };
	std::atomic<bool> loadInProgress{ false };
	std::atomic<bool> loadFailed{ false };
	std::thread loadThread;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Player)
};
