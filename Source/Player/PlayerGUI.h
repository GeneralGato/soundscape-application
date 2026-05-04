#pragma once

#include <JuceHeader.h>
#include <array>
#include <vector>

#include "Player.h"

class PlayerGUI : public juce::Component,
                  public juce::Button::Listener,
                  public juce::Slider::Listener,
                  public juce::ListBoxModel,
                  private juce::Timer
{
public:
    PlayerGUI();
    ~PlayerGUI() override;

    void setPlayer(Player* playerPtr);

    void paint(juce::Graphics& g) override;
    void resized() override;

    void buttonClicked(juce::Button* button) override;
    void sliderValueChanged(juce::Slider* slider) override;

    int getNumRows() override;
    void paintListBoxItem(int rowNumber,
                          juce::Graphics& g,
                          int width,
                          int height,
                          bool rowIsSelected) override;
    void selectedRowsChanged(int lastRowSelected) override;
    void listBoxItemDoubleClicked(int row, const juce::MouseEvent& event) override;

private:
    static constexpr int maxSavedPresets = 16;

    enum class LoadState
    {
        NoPlayer,
        Preparing,
        Loading,
        Ready,
        Failed
    };

    struct BankState
    {
        bool active = false;
        float volume = 70.0f;
        std::array<float, Player::maxBackgroundLayers> layerVolumes{};
    };

    struct SoloState
    {
        bool active = false;
        float volume = 62.0f;
        float pan = 0.0f;
    };

    struct AppState
    {
        float masterBackground = 72.0f;
        float masterSolo = 65.0f;
        std::array<BankState, Player::numBackgroundBanks> banks{};
        std::array<SoloState, Player::numForegroundSounds> solos{};
    };

    struct SavedPreset
    {
        juce::String id;
        juce::String name;
        juce::int64 updatedAt = 0;
        AppState state;
    };

    void timerCallback() override;

    float cube(float input) const;
    float clampPercent(float value) const;
    float clampPanPercent(float value) const;

    AppState createDefaultState() const;
    AppState deserialiseState(const juce::var& source, const AppState& fallback) const;
    juce::var serialiseState(const AppState& source) const;
    juce::String summarisePreset(const SavedPreset& preset) const;
    juce::String createPresetId() const;
    juce::String getBackgroundBankName(int bankIndex) const;
    juce::String getForegroundSoundName(int soundIndex) const;
    int getBackgroundLayerCount(int bankIndex) const;
    juce::String formatPanLabel(float pan) const;

    void initialiseLookAndFeel();
    void initialiseSettings();
    void restoreState();
    void restorePresets();
    void persistState();
    void persistPresets();

    void configurePrimaryButton(juce::TextButton& button);
    void configureSecondaryButton(juce::TextButton& button);
    void configurePresetButton(juce::TextButton& button, const juce::String& text);
    void configureSectionLabel(juce::Label& label, const juce::String& text, float fontHeight, juce::Colour colour);
    void configureHorizontalSlider(juce::Slider& slider, bool bipolar = false);

    void updateLoadState();
    void setAudioControlsEnabled(bool enabled);
    void updatePlayerLabels();
    void updatePresetButtons();
    void updatePresetList();
    void updateSavePresetAvailability();
    void updateSelectedBankUi();
    void updateBankCards();
    void updateSoloCards();
    void updateStatusText();
    void setStatusMessage(const juce::String& message);
    void commitStateChange(const juce::String& message = {});
    void syncUiFromState();
    void syncPlayerFromState();

    void setMasterMix(float backgroundValue, float soloValue);
    void applyMixPreset(float backgroundValue, float soloValue, const juce::String& label);
    void saveCurrentPreset();
    void loadSelectedPreset();
    void deleteSelectedPreset();
    void stopAllPlayback();
    void resetAllSliders();
    void resetBankSliders(int bankIndex);

    Player* player{ nullptr };
    juce::LookAndFeel_V4 lookAndFeel;
    std::unique_ptr<juce::PropertiesFile> settingsFile;
    AppState state;
    std::vector<SavedPreset> savedPresets;
    LoadState loadState{ LoadState::NoPlayer };
    juce::String statusMessage;
    juce::int64 statusMessageExpiryMs = 0;
    bool audioControlsEnabled = false;
    int selectedBackgroundBank = 0;
    int selectedPresetRow = -1;

    juce::Label titleLabel;
    juce::Label subtitleLabel;
    juce::Label statusLabel;
    juce::Label masterSectionLabel;
    juce::Label backgroundMasterLabel;
    juce::Label foregroundMasterLabel;
    juce::Label presetsSectionLabel;
    juce::Label bedsSectionLabel;
    juce::Label layersSectionLabel;
    juce::Label selectedBankLabel;
    juce::Label selectedBankHintLabel;
    juce::Label solosSectionLabel;
    juce::Label presetCountLabel;
    juce::Label presetEmptyLabel;

    juce::TextButton stopAllButton{ "Stop All" };
    juce::TextButton resetAllSlidersButton{ "Reset All Sliders" };
    juce::TextButton driftPresetButton{ "Drift" };
    juce::TextButton focusPresetButton{ "Focus" };
    juce::TextButton detailPresetButton{ "Detail" };
    juce::TextButton savePresetButton{ "Save Current" };
    juce::TextButton loadPresetButton{ "Load Selected" };
    juce::TextButton deletePresetButton{ "Delete Selected" };
    juce::TextEditor presetNameEditor;
    juce::ListBox presetListBox;

    juce::Slider backgroundMasterSlider;
    juce::Slider foregroundMasterSlider;

    std::array<juce::TextButton, Player::numBackgroundBanks> bankToggleButtons;
    std::array<juce::TextButton, Player::numBackgroundBanks> bankSelectButtons;
    std::array<juce::TextButton, Player::numBackgroundBanks> bankResetButtons;
    std::array<juce::Slider, Player::numBackgroundBanks> bankVolumeSliders;
    std::array<juce::Label, Player::numBackgroundBanks> bankSummaryLabels;

    std::array<juce::Label, Player::maxBackgroundLayers> layerLabels;
    std::array<juce::Slider, Player::maxBackgroundLayers> layerSliders;

    std::array<juce::TextButton, Player::numForegroundSounds> soloToggleButtons;
    std::array<juce::Slider, Player::numForegroundSounds> soloVolumeSliders;
    std::array<juce::Slider, Player::numForegroundSounds> soloPanSliders;

    juce::Rectangle<int> headerCardBounds;
    juce::Rectangle<int> masterCardBounds;
    juce::Rectangle<int> presetsCardBounds;
    juce::Rectangle<int> bedsCardBounds;
    juce::Rectangle<int> layersCardBounds;
    juce::Rectangle<int> solosCardBounds;
    std::array<juce::Rectangle<int>, Player::numBackgroundBanks> bankCardBounds{};
    std::array<juce::Rectangle<int>, Player::numForegroundSounds> soloCardBounds{};
};
