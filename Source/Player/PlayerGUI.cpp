#include "PlayerGUI.h"

namespace
{
constexpr int outerPadding = 16;
constexpr int cardGap = 12;
constexpr int cardCornerSize = 22;
constexpr int heroHeight = 88;
constexpr int mainSectionHeight = 88;
constexpr int menuEntryHeight = 34;
constexpr int menuEntryGap = 6;
constexpr int advancedHeaderHeight = 96;
constexpr int advancedGridGap = 10;
constexpr int advancedFooterTitleHeight = 30;

const auto backgroundTop = juce::Colour::fromRGB(0xF4, 0xEA, 0xDA);
const auto backgroundBottom = juce::Colour::fromRGB(0xD7, 0xE0, 0xDE);
const auto cardFill = juce::Colour::fromRGBA(0xFF, 0xFF, 0xFF, 0xD6);
const auto cardStroke = juce::Colour::fromRGBA(0x66, 0x53, 0x43, 0x2E);
const auto titleColour = juce::Colour::fromRGB(0x23, 0x1B, 0x16);
const auto bodyColour = juce::Colour::fromRGB(0x5E, 0x4D, 0x43);
const auto accentColour = juce::Colour::fromRGB(0xC8, 0x64, 0x34);
const auto accentSoft = juce::Colour::fromRGB(0xE5, 0xA4, 0x6B);
const auto knobColour = juce::Colour::fromRGB(0x2C, 0x5E, 0x63);
const auto knobTrackColour = juce::Colour::fromRGB(0xA7, 0xC4, 0xC7);
const auto sliderTrackColour = juce::Colour::fromRGBA(0x2C, 0x5E, 0x63, 0x66);

constexpr std::array<const char*, Player::numBackgroundBanks + 2> backgroundMenuLabels =
{
    "Soundscapes",
    "Ambient",
    "Rainy Day",
    "Seaside",
    "Drone",
    "Forest",
    "Summer Field",
    "Open Mixer"
};

constexpr std::array<const char*, Player::numBackgroundBanks> backgroundBankLabels =
{
    "Ambient",
    "Rainy Day",
    "Seaside",
    "Drone",
    "Forest",
    "Summer Field"
};

constexpr std::array<const char*, Player::numForegroundSounds + 2> foregroundMenuLabels =
{
    "Solo Sounds",
    "Light Stream",
    "River",
    "Waterfall",
    "Campfire",
    "Cat Purr",
    "Crows",
    "Open Mixer"
};

constexpr std::array<const char*, Player::numForegroundSounds> foregroundSoundLabels =
{
    "Light Stream",
    "River",
    "Waterfall",
    "Campfire",
    "Cat Purr",
    "Crows"
};

constexpr std::array<const char*, Player::numBackgroundLayers> backgroundLayerNames =
{
    "Layer 1",
    "Layer 2",
    "Layer 3",
    "Layer 4",
    "Layer 5",
    "Layer 6"
};

void prepareLabel(juce::Label& label, const juce::String& text, float fontHeight, juce::Colour colour)
{
    label.setText(text, juce::dontSendNotification);
    label.setFont(juce::Font(fontHeight, juce::Font::plain));
    label.setJustificationType(juce::Justification::centredLeft);
    label.setColour(juce::Label::textColourId, colour);
}

void drawCard(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    if (bounds.isEmpty())
    {
        return;
    }

    const auto card = bounds.toFloat();
    g.setColour(cardFill);
    g.fillRoundedRectangle(card, static_cast<float>(cardCornerSize));
    g.setColour(cardStroke);
    g.drawRoundedRectangle(card.reduced(0.5f), static_cast<float>(cardCornerSize), 1.0f);
}
}

PlayerGUI::PlayerGUI()
{
    setOpaque(true);
    setLookAndFeel(&lookAndFeel);

    lookAndFeel.setColour(juce::TextButton::buttonColourId, juce::Colour::fromRGB(0xF7, 0xED, 0xE5));
    lookAndFeel.setColour(juce::TextButton::buttonOnColourId, accentColour);
    lookAndFeel.setColour(juce::TextButton::textColourOffId, titleColour);
    lookAndFeel.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    lookAndFeel.setColour(juce::ToggleButton::tickColourId, accentColour);
    lookAndFeel.setColour(juce::ToggleButton::tickDisabledColourId, accentSoft.withAlpha(0.5f));
    lookAndFeel.setColour(juce::ToggleButton::textColourId, bodyColour);
    lookAndFeel.setColour(juce::Slider::rotarySliderFillColourId, knobColour);
    lookAndFeel.setColour(juce::Slider::rotarySliderOutlineColourId, knobTrackColour);
    lookAndFeel.setColour(juce::Slider::thumbColourId, juce::Colours::white);
    lookAndFeel.setColour(juce::Slider::trackColourId, sliderTrackColour);
    lookAndFeel.setColour(juce::Slider::backgroundColourId, knobTrackColour.withAlpha(0.35f));

    for (auto& levels : backgroundMixLevels)
    {
        levels.fill(50.0f);
    }

    prepareLabel(titleLabel, "Soundscape Application", 27.0f, titleColour);
    prepareLabel(subtitleLabel, "Layer ambient beds and solo loops without breaking flow.", 14.0f, bodyColour);
    prepareLabel(primarySectionLabel, "Ambient Beds", 15.0f, titleColour);
    prepareLabel(secondarySectionLabel, "Solo Sounds", 15.0f, titleColour);
    prepareLabel(advancedTitleLabel, "Shape the ambient bed", 24.0f, titleColour);
    prepareLabel(advancedSubtitleLabel, "Pick one soundscape, then tune its six backing layers.", 13.0f, bodyColour);
    prepareLabel(loadStatusLabel, "Preparing audio...", 13.0f, bodyColour);

    addAndMakeVisible(titleLabel);
    addAndMakeVisible(subtitleLabel);
    addAndMakeVisible(primarySectionLabel);
    addAndMakeVisible(secondarySectionLabel);
    addAndMakeVisible(advancedTitleLabel);
    addAndMakeVisible(advancedSubtitleLabel);
    addAndMakeVisible(loadStatusLabel);

    for (int i = 0; i < mainMenuEntryCount; ++i)
    {
        addAndMakeVisible(mainMenuBackgroundSounds[i]);
        mainMenuBackgroundSounds[i].addListener(this);
        mainMenuBackgroundSounds[i].setButtonText(backgroundMenuLabels[i]);
        configureButton(mainMenuBackgroundSounds[i], i == 0 || i == mainMenuEntryCount - 1);

        addAndMakeVisible(mainMenuForegroundSounds[i]);
        mainMenuForegroundSounds[i].addListener(this);
        mainMenuForegroundSounds[i].setButtonText(foregroundMenuLabels[i]);
        configureButton(mainMenuForegroundSounds[i], i == 0 || i == mainMenuEntryCount - 1);
    }

    addAndMakeVisible(backgroundSoundsGlobalVolume);
    backgroundSoundsGlobalVolume.setRange(0, 100);
    backgroundSoundsGlobalVolume.setValue(100, juce::dontSendNotification);
    backgroundSoundsGlobalVolume.addListener(this);
    configureRotarySlider(backgroundSoundsGlobalVolume);

    addAndMakeVisible(foregroundSoundsGlobalVolume);
    foregroundSoundsGlobalVolume.setRange(0, 100);
    foregroundSoundsGlobalVolume.setValue(100, juce::dontSendNotification);
    foregroundSoundsGlobalVolume.addListener(this);
    configureRotarySlider(foregroundSoundsGlobalVolume);

    addAndMakeVisible(backButton);
    backButton.addListener(this);
    configureButton(backButton, true);

    for (int i = 0; i < Player::numBackgroundBanks; ++i)
    {
        addAndMakeVisible(backgroundSoundsButton[i]);
        backgroundSoundsButton[i].addListener(this);
        backgroundSoundsButton[i].setButtonText(backgroundBankLabels[i]);
        configureButton(backgroundSoundsButton[i], true);

        addAndMakeVisible(backgroundSoundsIndividualVolume[i]);
        backgroundSoundsIndividualVolume[i].addListener(this);
        backgroundSoundsIndividualVolume[i].setRange(0, 100);
        backgroundSoundsIndividualVolume[i].setValue(50, juce::dontSendNotification);
        configureVerticalSlider(backgroundSoundsIndividualVolume[i]);

        addAndMakeVisible(backgroundSoundsBankVolume[i]);
        backgroundSoundsBankVolume[i].addListener(this);
        backgroundSoundsBankVolume[i].setRange(0, 100);
        backgroundSoundsBankVolume[i].setValue(50, juce::dontSendNotification);
        configureRotarySlider(backgroundSoundsBankVolume[i]);

        addAndMakeVisible(backgroundSoundsBankToggle[i]);
        backgroundSoundsBankToggle[i].addListener(this);
        backgroundSoundsBankToggle[i].setButtonText("On");
        configureToggle(backgroundSoundsBankToggle[i]);

        prepareLabel(backgroundLayerLabels[i], backgroundLayerNames[i], 11.5f, bodyColour);
        backgroundLayerLabels[i].setJustificationType(juce::Justification::centred);
        addAndMakeVisible(backgroundLayerLabels[i]);
    }

    for (int i = 0; i < Player::numForegroundSounds; ++i)
    {
        addAndMakeVisible(foregroundSoundsButton[i]);
        foregroundSoundsButton[i].addListener(this);
        foregroundSoundsButton[i].setButtonText(foregroundSoundLabels[i]);
        configureButton(foregroundSoundsButton[i], true);

        addAndMakeVisible(foregroundSoundsVolume[i]);
        foregroundSoundsVolume[i].addListener(this);
        foregroundSoundsVolume[i].setRange(0, 100);
        foregroundSoundsVolume[i].setValue(50, juce::dontSendNotification);
        configureRotarySlider(foregroundSoundsVolume[i]);

        addAndMakeVisible(foregroundSoundsPan[i]);
        foregroundSoundsPan[i].addListener(this);
        foregroundSoundsPan[i].setRange(0, 100);
        foregroundSoundsPan[i].setValue(50, juce::dontSendNotification);
        configureRotarySlider(foregroundSoundsPan[i]);
    }

    updateSelectedBackgroundBank(0);
    setAudioControlsEnabled(false);
}

PlayerGUI::~PlayerGUI()
{
    setLookAndFeel(nullptr);
}

void PlayerGUI::setPlayer(Player* playerPtr)
{
    player = playerPtr;

    if (player != nullptr)
    {
        player->setGlobalBackgroundGain(cube(static_cast<float>(backgroundSoundsGlobalVolume.getValue())));
        player->setGlobalForegroundGain(cube(static_cast<float>(foregroundSoundsGlobalVolume.getValue())));

        for (int bankIndex = 0; bankIndex < Player::numBackgroundBanks; ++bankIndex)
        {
            player->setBackgroundBankGain(bankIndex, cube(static_cast<float>(backgroundSoundsBankVolume[bankIndex].getValue())));

            for (int layerIndex = 0; layerIndex < Player::numBackgroundLayers; ++layerIndex)
            {
                player->setBackgroundLayerGain(bankIndex,
                                               layerIndex,
                                               cube(backgroundMixLevels[bankIndex][layerIndex]));
            }
        }

        for (int soundIndex = 0; soundIndex < Player::numForegroundSounds; ++soundIndex)
        {
            player->setForegroundGain(soundIndex, cube(static_cast<float>(foregroundSoundsVolume[soundIndex].getValue())));
            player->setForegroundPan(soundIndex, static_cast<float>(foregroundSoundsPan[soundIndex].getValue() / 100.0));
        }
    }

    updateLoadState();
    updateSectionButtonStates();
}

void PlayerGUI::paint(juce::Graphics& g)
{
    juce::ColourGradient backgroundGradient(backgroundTop, 0.0f, 0.0f, backgroundBottom, 0.0f, static_cast<float>(getHeight()), false);
    backgroundGradient.addColour(0.45, juce::Colour::fromRGB(0xF1, 0xDD, 0xC8));
    g.setGradientFill(backgroundGradient);
    g.fillAll();

    g.setColour(accentColour.withAlpha(0.08f));
    g.fillEllipse(static_cast<float>(getWidth() - 180), -20.0f, 220.0f, 160.0f);
    g.setColour(knobColour.withAlpha(0.08f));
    g.fillEllipse(-40.0f, static_cast<float>(getHeight() - 180), 200.0f, 180.0f);

    drawCard(g, heroCardBounds);
    drawCard(g, primaryCardBounds);
    drawCard(g, secondaryCardBounds);
    drawCard(g, advancedHeaderBounds);
    drawCard(g, advancedGridBounds);
    drawCard(g, advancedFooterBounds);

    if (currentView == ViewMode::AdvancedBackground && !advancedFooterBounds.isEmpty())
    {
        auto textBounds = advancedFooterBounds.reduced(16, 12);
        g.setColour(titleColour);
        g.setFont(juce::Font(15.0f, juce::Font::plain));
        g.drawText("Layer Mix", textBounds.removeFromTop(18), juce::Justification::centredLeft);
        g.setColour(bodyColour);
        g.setFont(juce::Font(12.0f, juce::Font::plain));
        g.drawText("Shape the selected soundscape one layer at a time.", textBounds.removeFromTop(18), juce::Justification::centredLeft);
    }

    if (currentView == ViewMode::AdvancedForeground && !advancedFooterBounds.isEmpty())
    {
        auto textBounds = advancedFooterBounds.reduced(16, 12);
        g.setColour(titleColour);
        g.setFont(juce::Font(15.0f, juce::Font::plain));
        g.drawText("Solo Control", textBounds.removeFromTop(18), juce::Justification::centredLeft);
        g.setColour(bodyColour);
        g.setFont(juce::Font(12.0f, juce::Font::plain));
        g.drawFittedText("Use the left knob for level and the right knob for pan to place each loop in the stereo field.", textBounds, juce::Justification::topLeft, 3);
    }
}

void PlayerGUI::resized()
{
    updateHeaderText();
    updateSectionButtonStates();

    switch (currentView)
    {
        case ViewMode::MainMenu:
            layoutMainMenu();
            break;
        case ViewMode::AdvancedBackground:
            layoutAdvancedBackground();
            break;
        case ViewMode::AdvancedForeground:
            layoutAdvancedForeground();
            break;
    }
}

void PlayerGUI::timerCallback()
{
    updateLoadState();
}

float PlayerGUI::cube(float input) const
{
    input = input / 100.0f;
    return input * input * input;
}

void PlayerGUI::configureButton(juce::TextButton& button, bool prominent)
{
    button.setColour(juce::TextButton::buttonColourId, prominent ? juce::Colour::fromRGB(0xEF, 0xDD, 0xCF) : juce::Colour::fromRGB(0xF7, 0xED, 0xE5));
    button.setColour(juce::TextButton::buttonOnColourId, prominent ? accentColour : knobColour);
    button.setColour(juce::TextButton::textColourOffId, titleColour);
    button.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
}

void PlayerGUI::configureToggle(juce::ToggleButton& button)
{
    button.setColour(juce::ToggleButton::tickColourId, accentColour);
    button.setColour(juce::ToggleButton::tickDisabledColourId, accentSoft.withAlpha(0.4f));
    button.setColour(juce::ToggleButton::textColourId, bodyColour);
}

void PlayerGUI::configureRotarySlider(juce::Slider& slider)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
}

void PlayerGUI::configureVerticalSlider(juce::Slider& slider)
{
    slider.setSliderStyle(juce::Slider::LinearVertical);
    slider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
}

void PlayerGUI::updateLoadState()
{
    auto nextState = LoadState::NoPlayer;
    auto nextText = juce::String("Preparing audio...");
    auto nextColour = bodyColour;
    auto shouldEnableControls = false;

    if (player == nullptr)
    {
        nextState = LoadState::NoPlayer;
    }
    else if (player->didLoadFail())
    {
        nextState = LoadState::Failed;
        nextText = "Audio library failed to load";
        nextColour = juce::Colour::fromRGB(0xA6, 0x33, 0x2A);
    }
    else if (player->isLoaded())
    {
        nextState = LoadState::Ready;
        nextText = "Audio ready";
        nextColour = juce::Colour::fromRGB(0x2D, 0x7A, 0x54);
        shouldEnableControls = true;
    }
    else if (player->isLoading())
    {
        nextState = LoadState::Loading;
        nextText = "Loading sounds...";
    }
    else
    {
        nextState = LoadState::Preparing;
    }

    if (nextState == LoadState::Preparing || nextState == LoadState::Loading)
    {
        if (!isTimerRunning())
        {
            startTimerHz(10);
        }
    }
    else
    {
        stopTimer();
    }

    if (nextState == loadState && shouldEnableControls == audioControlsEnabled)
    {
        return;
    }

    loadState = nextState;
    loadStatusLabel.setColour(juce::Label::textColourId, nextColour);
    loadStatusLabel.setText(nextText, juce::dontSendNotification);
    setAudioControlsEnabled(shouldEnableControls);
    repaint();
}

void PlayerGUI::setAudioControlsEnabled(bool enabled)
{
    audioControlsEnabled = enabled;

    for (int i = 1; i < mainMenuEntryCount; ++i)
    {
        mainMenuBackgroundSounds[i].setEnabled(enabled);
        mainMenuForegroundSounds[i].setEnabled(enabled);
    }

    backgroundSoundsGlobalVolume.setEnabled(enabled);
    foregroundSoundsGlobalVolume.setEnabled(enabled);

    for (int i = 0; i < Player::numBackgroundBanks; ++i)
    {
        backgroundSoundsButton[i].setEnabled(enabled);
        backgroundSoundsIndividualVolume[i].setEnabled(enabled);
        backgroundSoundsBankVolume[i].setEnabled(enabled);
        backgroundSoundsBankToggle[i].setEnabled(enabled);

        foregroundSoundsButton[i].setEnabled(enabled);
        foregroundSoundsVolume[i].setEnabled(enabled);
        foregroundSoundsPan[i].setEnabled(enabled);
    }

    updateSectionButtonStates();
}

void PlayerGUI::updateSelectedBackgroundBank(int bankIndex)
{
    if (bankIndex < 0 || bankIndex >= Player::numBackgroundBanks)
    {
        return;
    }

    selectedBackgroundBank = bankIndex;

    for (int i = 0; i < Player::numBackgroundBanks; ++i)
    {
        backgroundSoundsButton[i].setToggleState(i == selectedBackgroundBank, juce::dontSendNotification);
    }

    for (int i = 0; i < Player::numBackgroundLayers; ++i)
    {
        backgroundSoundsIndividualVolume[i].setValue(backgroundMixLevels[selectedBackgroundBank][i], juce::dontSendNotification);
    }

    updateHeaderText();
    repaint();
}

void PlayerGUI::updateSectionButtonStates()
{
    mainMenuBackgroundSounds[0].setToggleState(backgroundMenuExpanded, juce::dontSendNotification);
    mainMenuForegroundSounds[0].setToggleState(foregroundMenuExpanded, juce::dontSendNotification);

    if (player == nullptr)
    {
        for (int i = 0; i < Player::numBackgroundBanks; ++i)
        {
            mainMenuBackgroundSounds[i + 1].setToggleState(false, juce::dontSendNotification);
            backgroundSoundsBankToggle[i].setToggleState(false, juce::dontSendNotification);
            foregroundSoundsButton[i].setToggleState(false, juce::dontSendNotification);
            mainMenuForegroundSounds[i + 1].setToggleState(false, juce::dontSendNotification);
        }
        return;
    }

    for (int i = 0; i < Player::numBackgroundBanks; ++i)
    {
        const auto backgroundPlaying = player->isBackgroundBankPlaying(i);
        mainMenuBackgroundSounds[i + 1].setToggleState(backgroundPlaying, juce::dontSendNotification);
        backgroundSoundsBankToggle[i].setToggleState(backgroundPlaying, juce::dontSendNotification);

        const auto foregroundPlaying = player->isForegroundSoundPlaying(i);
        mainMenuForegroundSounds[i + 1].setToggleState(foregroundPlaying, juce::dontSendNotification);
        foregroundSoundsButton[i].setToggleState(foregroundPlaying, juce::dontSendNotification);
    }

    for (int i = 0; i < Player::numBackgroundBanks; ++i)
    {
        backgroundSoundsButton[i].setToggleState(i == selectedBackgroundBank, juce::dontSendNotification);
    }
}

void PlayerGUI::hideAdvancedControls()
{
    advancedHeaderBounds = {};
    advancedGridBounds = {};
    advancedFooterBounds = {};

    advancedTitleLabel.setBounds({});
    advancedSubtitleLabel.setBounds({});
    backButton.setBounds({});

    for (int i = 0; i < Player::numBackgroundBanks; ++i)
    {
        backgroundSoundsButton[i].setBounds({});
        backgroundSoundsBankVolume[i].setBounds({});
        backgroundSoundsIndividualVolume[i].setBounds({});
        backgroundSoundsBankToggle[i].setBounds({});
        backgroundLayerLabels[i].setBounds({});

        foregroundSoundsButton[i].setBounds({});
        foregroundSoundsVolume[i].setBounds({});
        foregroundSoundsPan[i].setBounds({});
    }
}

void PlayerGUI::layoutMainMenu()
{
    hideAdvancedControls();

    auto content = getLocalBounds().reduced(outerPadding);
    heroCardBounds = content.removeFromTop(heroHeight);
    content.removeFromTop(cardGap);

    const auto expandedHeight = mainSectionHeight + (mainMenuEntryCount - 1) * (menuEntryHeight + menuEntryGap);
    primaryCardBounds = content.removeFromTop(backgroundMenuExpanded ? expandedHeight : mainSectionHeight);
    content.removeFromTop(cardGap);
    secondaryCardBounds = content.removeFromTop(foregroundMenuExpanded ? expandedHeight : mainSectionHeight);

    auto hero = heroCardBounds.reduced(16, 14);
    titleLabel.setBounds(hero.removeFromTop(28));
    hero.removeFromTop(2);
    subtitleLabel.setBounds(hero.removeFromTop(18));
    hero.removeFromTop(8);
    loadStatusLabel.setBounds(hero.removeFromTop(18));

    advancedTitleLabel.setBounds({});
    advancedSubtitleLabel.setBounds({});

    auto layoutSection = [this](juce::Rectangle<int> cardBounds,
                                 juce::Label& sectionLabel,
                                 auto& buttons,
                                 juce::Slider& globalSlider,
                                 bool expanded)
    {
        auto inner = cardBounds.reduced(14);
        sectionLabel.setBounds(inner.removeFromTop(18));
        inner.removeFromTop(8);

        auto topRow = inner.removeFromTop(48);
        auto knobArea = topRow.removeFromRight(60);
        globalSlider.setBounds(knobArea.reduced(2));
        topRow.removeFromRight(10);
        buttons[0].setBounds(topRow);

        for (int i = 1; i < mainMenuEntryCount; ++i)
        {
            if (!expanded)
            {
                buttons[i].setBounds({});
                continue;
            }

            inner.removeFromTop(menuEntryGap);
            buttons[i].setBounds(inner.removeFromTop(menuEntryHeight));
        }
    };

    layoutSection(primaryCardBounds, primarySectionLabel, mainMenuBackgroundSounds, backgroundSoundsGlobalVolume, backgroundMenuExpanded);
    layoutSection(secondaryCardBounds, secondarySectionLabel, mainMenuForegroundSounds, foregroundSoundsGlobalVolume, foregroundMenuExpanded);
}

void PlayerGUI::layoutAdvancedBackground()
{
    heroCardBounds = {};
    primaryCardBounds = {};
    secondaryCardBounds = {};

    titleLabel.setBounds({});
    subtitleLabel.setBounds({});
    primarySectionLabel.setBounds({});
    secondarySectionLabel.setBounds({});

    for (int i = 0; i < mainMenuEntryCount; ++i)
    {
        mainMenuBackgroundSounds[i].setBounds({});
        mainMenuForegroundSounds[i].setBounds({});
    }

    backgroundSoundsGlobalVolume.setBounds({});
    foregroundSoundsGlobalVolume.setBounds({});

    for (int i = 0; i < Player::numForegroundSounds; ++i)
    {
        foregroundSoundsButton[i].setBounds({});
        foregroundSoundsVolume[i].setBounds({});
        foregroundSoundsPan[i].setBounds({});
    }

    auto content = getLocalBounds().reduced(outerPadding);
    advancedHeaderBounds = content.removeFromTop(advancedHeaderHeight);
    content.removeFromTop(cardGap);

    const auto gridHeight = juce::jmin(250, juce::jmax(210, content.getHeight() - 170));
    advancedGridBounds = content.removeFromTop(gridHeight);
    content.removeFromTop(cardGap);
    advancedFooterBounds = content;

    auto header = advancedHeaderBounds.reduced(16, 14);
    auto titleRow = header.removeFromTop(32);
    backButton.setBounds(titleRow.removeFromRight(80).reduced(0, 2));
    titleRow.removeFromRight(8);
    advancedTitleLabel.setBounds(titleRow);
    header.removeFromTop(4);
    advancedSubtitleLabel.setBounds(header.removeFromTop(18));
    header.removeFromTop(8);
    loadStatusLabel.setBounds(header.removeFromTop(18));

    auto grid = advancedGridBounds.reduced(12);
    const auto cellWidth = (grid.getWidth() - advancedGridGap) / 2;
    const auto cellHeight = (grid.getHeight() - (advancedGridGap * 2)) / 3;

    for (int i = 0; i < Player::numBackgroundBanks; ++i)
    {
        const auto column = i / 3;
        const auto row = i % 3;
        auto cell = juce::Rectangle<int>(grid.getX() + column * (cellWidth + advancedGridGap),
                                         grid.getY() + row * (cellHeight + advancedGridGap),
                                         cellWidth,
                                         cellHeight).reduced(2);

        auto inner = cell.reduced(8);
        auto controlStrip = inner.removeFromRight(58);
        backgroundSoundsButton[i].setBounds(inner.reduced(0, 4));
        backgroundSoundsBankVolume[i].setBounds(controlStrip.removeFromTop(40).reduced(2));
        controlStrip.removeFromTop(2);
        backgroundSoundsBankToggle[i].setBounds(controlStrip.removeFromTop(20));
    }

    auto footer = advancedFooterBounds.reduced(12, 10);
    footer.removeFromTop(advancedFooterTitleHeight);
    const auto columnWidth = footer.getWidth() / Player::numBackgroundLayers;

    for (int i = 0; i < Player::numBackgroundLayers; ++i)
    {
        auto column = footer.removeFromLeft(columnWidth);
        auto labelArea = column.removeFromBottom(18);
        backgroundLayerLabels[i].setBounds(labelArea);
        backgroundSoundsIndividualVolume[i].setBounds(column.reduced(8, 0));
    }
}

void PlayerGUI::layoutAdvancedForeground()
{
    heroCardBounds = {};
    primaryCardBounds = {};
    secondaryCardBounds = {};

    titleLabel.setBounds({});
    subtitleLabel.setBounds({});
    primarySectionLabel.setBounds({});
    secondarySectionLabel.setBounds({});

    for (int i = 0; i < mainMenuEntryCount; ++i)
    {
        mainMenuBackgroundSounds[i].setBounds({});
        mainMenuForegroundSounds[i].setBounds({});
    }

    backgroundSoundsGlobalVolume.setBounds({});
    foregroundSoundsGlobalVolume.setBounds({});

    for (int i = 0; i < Player::numBackgroundBanks; ++i)
    {
        backgroundSoundsButton[i].setBounds({});
        backgroundSoundsBankVolume[i].setBounds({});
        backgroundSoundsIndividualVolume[i].setBounds({});
        backgroundSoundsBankToggle[i].setBounds({});
        backgroundLayerLabels[i].setBounds({});
    }

    auto content = getLocalBounds().reduced(outerPadding);
    advancedHeaderBounds = content.removeFromTop(advancedHeaderHeight);
    content.removeFromTop(cardGap);

    const auto gridHeight = juce::jmin(252, juce::jmax(222, content.getHeight() - 118));
    advancedGridBounds = content.removeFromTop(gridHeight);
    content.removeFromTop(cardGap);
    advancedFooterBounds = content;

    auto header = advancedHeaderBounds.reduced(16, 14);
    auto titleRow = header.removeFromTop(32);
    backButton.setBounds(titleRow.removeFromRight(80).reduced(0, 2));
    titleRow.removeFromRight(8);
    advancedTitleLabel.setBounds(titleRow);
    header.removeFromTop(4);
    advancedSubtitleLabel.setBounds(header.removeFromTop(18));
    header.removeFromTop(8);
    loadStatusLabel.setBounds(header.removeFromTop(18));

    auto grid = advancedGridBounds.reduced(12);
    const auto cellWidth = (grid.getWidth() - advancedGridGap) / 2;
    const auto cellHeight = (grid.getHeight() - (advancedGridGap * 2)) / 3;

    for (int i = 0; i < Player::numForegroundSounds; ++i)
    {
        const auto column = i / 3;
        const auto row = i % 3;
        auto cell = juce::Rectangle<int>(grid.getX() + column * (cellWidth + advancedGridGap),
                                         grid.getY() + row * (cellHeight + advancedGridGap),
                                         cellWidth,
                                         cellHeight).reduced(2);

        auto inner = cell.reduced(8);
        foregroundSoundsButton[i].setBounds(inner.removeFromTop(30));
        inner.removeFromTop(4);
        auto controlRow = inner.removeFromTop(40);
        auto volumeArea = controlRow.removeFromLeft((controlRow.getWidth() - 8) / 2);
        controlRow.removeFromLeft(8);
        foregroundSoundsVolume[i].setBounds(volumeArea);
        foregroundSoundsPan[i].setBounds(controlRow);
    }
}

void PlayerGUI::updateHeaderText()
{
    if (currentView == ViewMode::AdvancedBackground)
    {
        advancedTitleLabel.setText("Shape the ambient bed", juce::dontSendNotification);
        advancedSubtitleLabel.setText("Selected bank: " + juce::String(backgroundBankLabels[selectedBackgroundBank]) + "  Tune its six layers below.", juce::dontSendNotification);
        return;
    }

    if (currentView == ViewMode::AdvancedForeground)
    {
        advancedTitleLabel.setText("Tune solo sounds", juce::dontSendNotification);
        advancedSubtitleLabel.setText("Each sound has its own level and stereo position.", juce::dontSendNotification);
        return;
    }

    advancedTitleLabel.setText({}, juce::dontSendNotification);
    advancedSubtitleLabel.setText({}, juce::dontSendNotification);
}

void PlayerGUI::sliderValueChanged(juce::Slider* slider)
{
    if (player == nullptr || !audioControlsEnabled)
    {
        return;
    }

    if (slider == &backgroundSoundsGlobalVolume)
    {
        player->setGlobalBackgroundGain(cube(static_cast<float>(backgroundSoundsGlobalVolume.getValue())));
        return;
    }

    if (slider == &foregroundSoundsGlobalVolume)
    {
        player->setGlobalForegroundGain(cube(static_cast<float>(foregroundSoundsGlobalVolume.getValue())));
        return;
    }

    for (int i = 0; i < Player::numBackgroundBanks; ++i)
    {
        if (slider == &backgroundSoundsIndividualVolume[i])
        {
            const auto value = static_cast<float>(backgroundSoundsIndividualVolume[i].getValue());
            backgroundMixLevels[selectedBackgroundBank][i] = value;
            player->setBackgroundLayerGain(selectedBackgroundBank, i, cube(value));
            return;
        }

        if (slider == &backgroundSoundsBankVolume[i])
        {
            player->setBackgroundBankGain(i, cube(static_cast<float>(backgroundSoundsBankVolume[i].getValue())));
            return;
        }

        if (slider == &foregroundSoundsVolume[i])
        {
            player->setForegroundGain(i, cube(static_cast<float>(foregroundSoundsVolume[i].getValue())));
            return;
        }

        if (slider == &foregroundSoundsPan[i])
        {
            player->setForegroundPan(i, static_cast<float>(foregroundSoundsPan[i].getValue() / 100.0));
            return;
        }
    }
}

void PlayerGUI::buttonClicked(juce::Button* button)
{
    if (button == &mainMenuBackgroundSounds[0])
    {
        handleBGsounds();
        return;
    }

    if (button == &mainMenuForegroundSounds[0])
    {
        handleSsounds();
        return;
    }

    if (button == &backButton)
    {
        handleBackButton();
        return;
    }

    if (button == &mainMenuBackgroundSounds[mainMenuEntryCount - 1])
    {
        if (audioControlsEnabled)
        {
            advancedBGsounds();
        }
        return;
    }

    if (button == &mainMenuForegroundSounds[mainMenuEntryCount - 1])
    {
        if (audioControlsEnabled)
        {
            advancedSSsounds();
        }
        return;
    }

    if (player == nullptr || !audioControlsEnabled)
    {
        return;
    }

    for (int i = 0; i < Player::numBackgroundBanks; ++i)
    {
        if (button == &backgroundSoundsButton[i])
        {
            updateSelectedBackgroundBank(i);
            return;
        }

        if (button == &mainMenuBackgroundSounds[i + 1] || button == &backgroundSoundsBankToggle[i])
        {
            player->toggleBackgroundBank(i);
            updateSectionButtonStates();
            repaint();
            return;
        }

        if (button == &mainMenuForegroundSounds[i + 1] || button == &foregroundSoundsButton[i])
        {
            player->toggleForegroundSound(i);
            updateSectionButtonStates();
            repaint();
            return;
        }
    }
}

void PlayerGUI::handleBGsounds()
{
    if (currentView != ViewMode::MainMenu)
    {
        return;
    }

    backgroundMenuExpanded = !backgroundMenuExpanded;
    if (backgroundMenuExpanded)
    {
        foregroundMenuExpanded = false;
    }

    resized();
    repaint();
}

void PlayerGUI::handleSsounds()
{
    if (currentView != ViewMode::MainMenu)
    {
        return;
    }

    foregroundMenuExpanded = !foregroundMenuExpanded;
    if (foregroundMenuExpanded)
    {
        backgroundMenuExpanded = false;
    }

    resized();
    repaint();
}

void PlayerGUI::advancedBGsounds()
{
    currentView = ViewMode::AdvancedBackground;
    resized();
    repaint();
}

void PlayerGUI::advancedSSsounds()
{
    currentView = ViewMode::AdvancedForeground;
    resized();
    repaint();
}

void PlayerGUI::handleBackButton()
{
    currentView = ViewMode::MainMenu;
    backgroundMenuExpanded = false;
    foregroundMenuExpanded = false;
    resized();
    repaint();
}

