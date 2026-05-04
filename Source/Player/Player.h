#pragma once

#include <JuceHeader.h>
#include <array>
#include <atomic>
#include <thread>

class Player
{
public:
	static constexpr int numBackgroundBanks = 6;
	static constexpr int maxBackgroundLayers = 12;
	static constexpr int numForegroundSounds = 6;

	struct StereoFrame
	{
		float left = 0.0f;
		float right = 0.0f;
	};

	using BackgroundLayerGains = std::array<std::array<float, maxBackgroundLayers>, numBackgroundBanks>;
	using BackgroundPathGrid = std::array<std::array<std::string, maxBackgroundLayers>, numBackgroundBanks>;
	using BackgroundLayerCountStorage = std::array<int, numBackgroundBanks>;
	using BackgroundLayerNameGrid = std::array<std::array<juce::String, maxBackgroundLayers>, numBackgroundBanks>;

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

	/** Prepares the playback engine for the active output device */
	void prepareToPlay(double sampleRate, int samplesPerBlockExpected);

	/** Renders a block of stereo output samples */
	void renderNextBlock(float* leftChannel, float* rightChannel, int numSamples);

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

	/** Updates the global gain for background sounds */
	void setGlobalBackgroundGain(float gain);

	/** Updates the global gain for foreground sounds */
	void setGlobalForegroundGain(float gain);

	/** Updates one layer gain inside a background bank */
	void setBackgroundLayerGain(int bankIndex, int layerIndex, float gain);

	/** Updates a background bank master gain */
	void setBackgroundBankGain(int bankIndex, float gain);

	/** Updates the gain for one foreground sound */
	void setForegroundGain(int soundIndex, float gain);

	/** Updates the stereo pan for one foreground sound */
	void setForegroundPan(int soundIndex, float pan);

	/** Returns the current label for a background bank */
	juce::String getBackgroundBankName(int bankIndex) const;

	/** Returns the current label for a foreground sound */
	juce::String getForegroundSoundName(int soundIndex) const;

	/** Returns the active layer count for a background bank */
	int getBackgroundLayerCount(int bankIndex) const;

	/** Returns the current label for one layer in a background bank */
	juce::String getBackgroundLayerName(int bankIndex, int layerIndex) const;

	/** String storage of background sounds file paths */
	BackgroundPathGrid backgroundFilePathStorage{};
	BackgroundLayerCountStorage backgroundLayerCounts{};
	BackgroundLayerNameGrid backgroundLayerNames{};
	std::array<juce::String, numBackgroundBanks> backgroundBankNames{};

	/** String storage of foreground sounds file paths */
	std::array<std::string, numForegroundSounds> foregroundFilePathStorage{};
	std::array<juce::String, numForegroundSounds> foregroundSoundNames{};

private:
	using SmoothedFloat = juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>;
	using AtomicBackgroundLayerGains = std::array<std::array<std::atomic<float>, maxBackgroundLayers>, numBackgroundBanks>;
	using AtomicBackgroundBankGains = std::array<std::atomic<float>, numBackgroundBanks>;
	using AtomicForegroundGains = std::array<std::atomic<float>, numForegroundSounds>;
	using AtomicBackgroundPlayTargets = std::array<std::atomic<float>, numBackgroundBanks>;
	using AtomicForegroundPlayTargets = std::array<std::atomic<float>, numForegroundSounds>;
	using SmoothedBackgroundLayerGains = std::array<std::array<SmoothedFloat, maxBackgroundLayers>, numBackgroundBanks>;
	using SmoothedBackgroundBankGains = std::array<SmoothedFloat, numBackgroundBanks>;
	using SmoothedForegroundGains = std::array<SmoothedFloat, numForegroundSounds>;
	using SmoothedBackgroundPlayGains = std::array<SmoothedFloat, numBackgroundBanks>;
	using SmoothedForegroundPlayGains = std::array<SmoothedFloat, numForegroundSounds>;
	using SmoothedPanGains = std::array<SmoothedFloat, numForegroundSounds>;
	using BackgroundEnableStates = std::array<std::atomic<bool>, numBackgroundBanks>;
	using ForegroundEnableStates = std::array<std::atomic<bool>, numForegroundSounds>;
	using BackgroundPositions = std::array<std::array<unsigned int, maxBackgroundLayers>, numBackgroundBanks>;
	using ForegroundPositions = std::array<unsigned int, numForegroundSounds>;
	using BackgroundBuffers = std::array<std::array<juce::AudioBuffer<float>, maxBackgroundLayers>, numBackgroundBanks>;
	using ForegroundBuffers = std::array<juce::AudioBuffer<float>, numForegroundSounds>;

	bool isIndexValid(int index, int upperBound) const;
	bool isBackgroundLayerIndexValid(int bankIndex, int layerIndex) const;
	void initialiseDefaults();
	void initialiseSmoothing(double sampleRate);
	void updateSmoothedTargets();
	void snapInactiveParameters();
	bool loadMonoFile(const juce::File& file, juce::AudioFormatManager& formatManager, juce::AudioBuffer<float>& destination);
	void storeForegroundPanTargets(int soundIndex, float pan);

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

	std::atomic<float> globalBackgroundGainTarget{ 1.0f };
	std::atomic<float> globalForegroundGainTarget{ 1.0f };
	AtomicBackgroundLayerGains backgroundLayerGainTargets{};
	AtomicBackgroundBankGains backgroundBankGainTargets{};
	AtomicForegroundGains foregroundGainTargets{};
	AtomicBackgroundPlayTargets backgroundPlayTargets{};
	AtomicForegroundPlayTargets foregroundPlayTargets{};
	AtomicForegroundGains foregroundLeftPanTargets{};
	AtomicForegroundGains foregroundRightPanTargets{};
	SmoothedFloat globalBackgroundGain{};
	SmoothedFloat globalForegroundGain{};
	SmoothedBackgroundLayerGains backgroundLayerGains{};
	SmoothedBackgroundBankGains backgroundBankGains{};
	SmoothedForegroundGains foregroundGains{};
	SmoothedBackgroundPlayGains backgroundPlayGains{};
	SmoothedForegroundPlayGains foregroundPlayGains{};
	SmoothedPanGains foregroundLeftPanGains{};
	SmoothedPanGains foregroundRightPanGains{};
	double currentSampleRate = 44100.0;

	std::atomic<bool> loadCompleted{ false };
	std::atomic<bool> loadInProgress{ false };
	std::atomic<bool> loadFailed{ false };
	std::thread loadThread;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Player)
};
