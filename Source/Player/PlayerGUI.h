#pragma once

#include <JuceHeader.h>
#include "Player.h"

class PlayerGUI :	public juce::Component,
					public juce::Button::Listener,
					public juce::Slider::Listener,
					private juce::Timer
{
public:
	/** Constructor for the PlayerGUI class */
	PlayerGUI();

	/** Destructor for the PlayerGUI class */
	~PlayerGUI() override;

	/** Sets the Player for this GUI object to control */
	void setPlayer(Player* playerPtr);

	void paint(juce::Graphics& g) override;

	/** Resized function from the Component class*/
	void resized() override;

	/** Listens for button callbacks */
	void buttonClicked(juce::Button* button) override;

	/** Listens for slider callbacks */
	void sliderValueChanged(juce::Slider* slider) override;

private:
	static constexpr int mainMenuEntryCount = Player::numBackgroundBanks + 2;

	enum class ViewMode
	{
		MainMenu,
		AdvancedBackground,
		AdvancedForeground
	};

	using BackgroundMixLevels = std::array<std::array<float, Player::numBackgroundLayers>, Player::numBackgroundBanks>;

	void timerCallback() override;
	float cube(float input) const;
	void configureButton(juce::TextButton& button, bool prominent = false);
	void configureToggle(juce::ToggleButton& button);
	void configureRotarySlider(juce::Slider& slider);
	void configureVerticalSlider(juce::Slider& slider);
	void updateLoadState();
	void setAudioControlsEnabled(bool enabled);
	void updateSelectedBackgroundBank(int bankIndex);
	void updateSectionButtonStates();
	void hideAdvancedControls();
	void layoutMainMenu();
	void layoutAdvancedBackground();
	void layoutAdvancedForeground();
	void updateHeaderText();
	void handleBGsounds();
	void handleSsounds();
	void advancedBGsounds();
	void advancedSSsounds();
	void handleBackButton();

	//Pointer to the player object
	Player* player{ nullptr };
	juce::LookAndFeel_V4 lookAndFeel;
	juce::Label titleLabel;
	juce::Label subtitleLabel;
	juce::Label primarySectionLabel;
	juce::Label secondarySectionLabel;
	juce::Label advancedTitleLabel;
	juce::Label advancedSubtitleLabel;
	std::array<juce::Label, Player::numBackgroundLayers> backgroundLayerLabels;
	juce::Label loadStatusLabel;

	//Declaration of Play button
	juce::TextButton backButton { "Back" };

	//Declaration of makeshift multi-choice combo-box
	std::array <juce::TextButton, mainMenuEntryCount> mainMenuBackgroundSounds;
	std::array <juce::TextButton, mainMenuEntryCount> mainMenuForegroundSounds;

	//Advanced Settings BS components
	std::array <juce::TextButton, Player::numBackgroundBanks> backgroundSoundsButton;
	std::array <juce::Slider, Player::numBackgroundLayers> backgroundSoundsIndividualVolume;
	std::array <juce::Slider, Player::numBackgroundBanks> backgroundSoundsBankVolume;
	std::array <juce::ToggleButton, Player::numBackgroundBanks> backgroundSoundsBankToggle;

	//Advanced Settings SS components
	std::array <juce::TextButton, Player::numForegroundSounds> foregroundSoundsButton;
	std::array <juce::Slider, Player::numForegroundSounds> foregroundSoundsVolume;
	std::array <juce::Slider, Player::numForegroundSounds> foregroundSoundsPan;

	//Global volume sliders
	juce::Slider backgroundSoundsGlobalVolume;
	juce::Slider foregroundSoundsGlobalVolume;

	BackgroundMixLevels backgroundMixLevels{};
	ViewMode currentView{ ViewMode::MainMenu };
	bool backgroundMenuExpanded = false;
	bool foregroundMenuExpanded = false;
	bool audioControlsEnabled = false;
	int selectedBackgroundBank = 0;
	juce::Rectangle<int> heroCardBounds;
	juce::Rectangle<int> primaryCardBounds;
	juce::Rectangle<int> secondaryCardBounds;
	juce::Rectangle<int> advancedHeaderBounds;
	juce::Rectangle<int> advancedGridBounds;
	juce::Rectangle<int> advancedFooterBounds;
};
