#include "PlayerGUI.h"

#include <algorithm>

namespace
{
constexpr int outerPadding = 22;
constexpr int cardGap = 14;
constexpr int cardCornerSize = 22;
constexpr int soloRowHeight = 64;
constexpr int activeStripeWidth = 6;
constexpr int transientStatusLifetimeMs = 3500;

constexpr float driftBackground = 82.0f;
constexpr float driftSolo = 48.0f;
constexpr float focusBackground = 68.0f;
constexpr float focusSolo = 60.0f;
constexpr float detailBackground = 50.0f;
constexpr float detailSolo = 74.0f;

constexpr std::array<const char*, Player::numBackgroundBanks> fallbackBackgroundBankNames =
{
    "Ambient",
    "Rainy Day",
    "Seaside",
    "Drone",
    "Forest",
    "Summer Field"
};

constexpr std::array<const char*, Player::numForegroundSounds> fallbackForegroundSoundNames =
{
    "Light Stream",
    "River",
    "Waterfall",
    "Campfire",
    "Cat Purr",
    "Crows"
};

const auto backgroundTop = juce::Colour::fromRGB(0x02, 0x03, 0x0A);
const auto backgroundBlue = juce::Colour::fromRGB(0x07, 0x13, 0x31);
const auto backgroundPurple = juce::Colour::fromRGB(0x24, 0x0C, 0x42);
const auto backgroundBottom = juce::Colour::fromRGB(0x67, 0x10, 0x57);
const auto cardFill = juce::Colour::fromRGBA(0x0B, 0x0E, 0x1E, 0xE0);
const auto cardStroke = juce::Colour::fromRGBA(0xFF, 0x3B, 0xB7, 0x4A);
const auto cardHighlight = juce::Colour::fromRGBA(0x8B, 0x4D, 0xFF, 0x3E);
const auto cardHighlightStroke = juce::Colour::fromRGB(0xFF, 0x3B, 0xB7);
const auto titleColour = juce::Colour::fromRGB(0xF7, 0xEA, 0xFF);
const auto bodyColour = juce::Colour::fromRGB(0xB9, 0xA9, 0xD9);
const auto accentColour = juce::Colour::fromRGB(0xFF, 0x3B, 0xB7);
const auto accentSoft = juce::Colour::fromRGB(0x8B, 0x4D, 0xFF);
const auto knobColour = juce::Colour::fromRGB(0x36, 0xC5, 0xFF);
const auto knobTrackColour = juce::Colour::fromRGB(0x2A, 0x1D, 0x48);
const auto sliderTrackColour = juce::Colour::fromRGBA(0xFF, 0x3B, 0xB7, 0x88);
const auto successColour = juce::Colour::fromRGB(0x36, 0xC5, 0xFF);
const auto panelButtonColour = juce::Colour::fromRGB(0x12, 0x10, 0x25);
const auto textFieldColour = juce::Colour::fromRGBA(0x11, 0x0D, 0x21, 0xF2);

juce::String sanitisePresetName(juce::String name)
{
    return name.trim().replaceCharacters("\r\n\t", "   ")
        .replace("  ", " ")
        .trim()
        .substring(0, 32);
}

juce::String pluralise(int count, const juce::String& singular, const juce::String& plural)
{
    return juce::String(count) + " " + (count == 1 ? singular : plural);
}

void prepareLabel(juce::Label& label,
                  const juce::String& text,
                  float fontHeight,
                  juce::Colour colour,
                  juce::Justification justification = juce::Justification::centredLeft)
{
    label.setText(text, juce::dontSendNotification);
    label.setFont(juce::Font(fontHeight, juce::Font::plain));
    label.setJustificationType(justification);
    label.setColour(juce::Label::textColourId, colour);
    label.setInterceptsMouseClicks(false, false);
}

void drawCard(juce::Graphics& g, juce::Rectangle<int> bounds, bool highlighted = false)
{
    if (bounds.isEmpty())
    {
        return;
    }

    const auto card = bounds.toFloat();
    g.setColour(cardFill);
    g.fillRoundedRectangle(card, static_cast<float>(cardCornerSize));

    if (highlighted)
    {
        g.setColour(cardHighlight);
        g.fillRoundedRectangle(card.reduced(1.0f), static_cast<float>(cardCornerSize - 2));
        g.setColour(cardHighlightStroke);
        g.drawRoundedRectangle(card.reduced(1.0f), static_cast<float>(cardCornerSize - 2), 2.0f);
    }
    else
    {
        g.setColour(cardStroke);
        g.drawRoundedRectangle(card.reduced(0.5f), static_cast<float>(cardCornerSize), 1.0f);
    }
}
}

PlayerGUI::PlayerGUI()
{
    setOpaque(true);
    setLookAndFeel(&lookAndFeel);

    initialiseLookAndFeel();
    initialiseSettings();
    state = createDefaultState();
    restoreState();
    restorePresets();

    configureSectionLabel(titleLabel, "Soundscape Desktop", 28.0f, titleColour);
    configureSectionLabel(subtitleLabel, "Desktop mix surface for ambient beds, solo loops, saved scenes, and quick layer shaping.", 14.0f, bodyColour);
    configureSectionLabel(statusLabel, "Preparing audio...", 13.0f, bodyColour);
    configureSectionLabel(masterSectionLabel, "Pocket Mix", 16.0f, titleColour);
    configureSectionLabel(backgroundMasterLabel, "Beds", 13.0f, bodyColour);
    configureSectionLabel(foregroundMasterLabel, "Loops", 13.0f, bodyColour);
    configureSectionLabel(presetsSectionLabel, "Saved Scenes", 16.0f, titleColour);
    configureSectionLabel(bedsSectionLabel, "Ambient Beds", 16.0f, titleColour);
    configureSectionLabel(layersSectionLabel, "Layer Blend", 16.0f, titleColour);
    configureSectionLabel(selectedBankLabel, "Select a bed", 18.0f, titleColour);
    configureSectionLabel(selectedBankHintLabel, "Pick a bed to shape its layers. Reset keeps the bed on or off, just like Pocket.", 13.0f, bodyColour);
    configureSectionLabel(solosSectionLabel, "Solo Loops", 16.0f, titleColour);
    prepareLabel(presetCountLabel, "0 saved", 12.0f, bodyColour, juce::Justification::centredRight);
    prepareLabel(presetEmptyLabel, "Save the current scene to keep bed toggles, layer blend, loop pan, and master mix together.", 13.0f, bodyColour, juce::Justification::centred);

    addAndMakeVisible(titleLabel);
    addAndMakeVisible(subtitleLabel);
    addAndMakeVisible(statusLabel);
    addAndMakeVisible(masterSectionLabel);
    addAndMakeVisible(backgroundMasterLabel);
    addAndMakeVisible(foregroundMasterLabel);
    addAndMakeVisible(presetsSectionLabel);
    addAndMakeVisible(bedsSectionLabel);
    addAndMakeVisible(layersSectionLabel);
    addAndMakeVisible(selectedBankLabel);
    addAndMakeVisible(selectedBankHintLabel);
    addAndMakeVisible(solosSectionLabel);
    addAndMakeVisible(presetCountLabel);
    addAndMakeVisible(presetEmptyLabel);

    configurePrimaryButton(stopAllButton);
    configureSecondaryButton(resetAllSlidersButton);
    configurePresetButton(driftPresetButton, "Drift");
    configurePresetButton(focusPresetButton, "Focus");
    configurePresetButton(detailPresetButton, "Detail");
    configurePrimaryButton(savePresetButton);
    configureSecondaryButton(loadPresetButton);
    configureSecondaryButton(deletePresetButton);

    addAndMakeVisible(stopAllButton);
    addAndMakeVisible(resetAllSlidersButton);
    addAndMakeVisible(driftPresetButton);
    addAndMakeVisible(focusPresetButton);
    addAndMakeVisible(detailPresetButton);
    addAndMakeVisible(savePresetButton);
    addAndMakeVisible(loadPresetButton);
    addAndMakeVisible(deletePresetButton);

    stopAllButton.addListener(this);
    resetAllSlidersButton.addListener(this);
    driftPresetButton.addListener(this);
    focusPresetButton.addListener(this);
    detailPresetButton.addListener(this);
    savePresetButton.addListener(this);
    loadPresetButton.addListener(this);
    deletePresetButton.addListener(this);

    configureHorizontalSlider(backgroundMasterSlider);
    configureHorizontalSlider(foregroundMasterSlider);
    backgroundMasterSlider.setTextValueSuffix("%");
    foregroundMasterSlider.setTextValueSuffix("%");
    addAndMakeVisible(backgroundMasterSlider);
    addAndMakeVisible(foregroundMasterSlider);
    backgroundMasterSlider.addListener(this);
    foregroundMasterSlider.addListener(this);

    presetNameEditor.setTextToShowWhenEmpty("Type a scene name", bodyColour.withAlpha(0.7f));
    presetNameEditor.setColour(juce::TextEditor::backgroundColourId, textFieldColour);
    presetNameEditor.setColour(juce::TextEditor::outlineColourId, cardStroke);
    presetNameEditor.setColour(juce::TextEditor::focusedOutlineColourId, accentColour);
    presetNameEditor.setColour(juce::TextEditor::textColourId, titleColour);
    presetNameEditor.setColour(juce::TextEditor::highlightColourId, accentSoft.withAlpha(0.32f));
    presetNameEditor.setColour(juce::CaretComponent::caretColourId, titleColour);
    presetNameEditor.setFont(juce::Font(14.0f, juce::Font::plain));
    presetNameEditor.onTextChange = [this]
    {
        updateSavePresetAvailability();
    };
    presetNameEditor.onReturnKey = [this]
    {
        saveCurrentPreset();
    };
    addAndMakeVisible(presetNameEditor);

    presetListBox.setModel(this);
    presetListBox.setRowHeight(52);
    presetListBox.setColour(juce::ListBox::backgroundColourId, juce::Colours::transparentBlack);
    presetListBox.setOutlineThickness(0);
    addAndMakeVisible(presetListBox);

    for (int bankIndex = 0; bankIndex < Player::numBackgroundBanks; ++bankIndex)
    {
        configureSecondaryButton(bankToggleButtons[bankIndex]);
        configureSecondaryButton(bankSelectButtons[bankIndex]);
        configureSecondaryButton(bankResetButtons[bankIndex]);
        configureHorizontalSlider(bankVolumeSliders[bankIndex]);
        bankVolumeSliders[bankIndex].setTextValueSuffix("%");

        bankToggleButtons[bankIndex].addListener(this);
        bankSelectButtons[bankIndex].addListener(this);
        bankResetButtons[bankIndex].addListener(this);
        bankVolumeSliders[bankIndex].addListener(this);

        prepareLabel(bankSummaryLabels[bankIndex], {}, 12.0f, bodyColour);

        addAndMakeVisible(bankToggleButtons[bankIndex]);
        addAndMakeVisible(bankSelectButtons[bankIndex]);
        addAndMakeVisible(bankResetButtons[bankIndex]);
        addAndMakeVisible(bankVolumeSliders[bankIndex]);
        addAndMakeVisible(bankSummaryLabels[bankIndex]);
    }

    for (int layerIndex = 0; layerIndex < Player::maxBackgroundLayers; ++layerIndex)
    {
        prepareLabel(layerLabels[layerIndex], {}, 12.5f, bodyColour);
        configureHorizontalSlider(layerSliders[layerIndex]);
        layerSliders[layerIndex].setTextValueSuffix("%");
        layerSliders[layerIndex].addListener(this);
        addAndMakeVisible(layerLabels[layerIndex]);
        addAndMakeVisible(layerSliders[layerIndex]);
    }

    for (int soundIndex = 0; soundIndex < Player::numForegroundSounds; ++soundIndex)
    {
        configureSecondaryButton(soloToggleButtons[soundIndex]);
        configureHorizontalSlider(soloVolumeSliders[soundIndex]);
        configureHorizontalSlider(soloPanSliders[soundIndex], true);

        soloToggleButtons[soundIndex].addListener(this);
        soloVolumeSliders[soundIndex].addListener(this);
        soloPanSliders[soundIndex].addListener(this);

        soloVolumeSliders[soundIndex].setTextValueSuffix("%");
        soloPanSliders[soundIndex].setTextValueSuffix("%");

        addAndMakeVisible(soloToggleButtons[soundIndex]);
        addAndMakeVisible(soloVolumeSliders[soundIndex]);
        addAndMakeVisible(soloPanSliders[soundIndex]);
    }

    updatePlayerLabels();
    syncUiFromState();
    setAudioControlsEnabled(false);
    updatePresetList();
    updateStatusText();
    startTimerHz(10);
}

PlayerGUI::~PlayerGUI()
{
    stopTimer();
    presetListBox.setModel(nullptr);
    setLookAndFeel(nullptr);
}

void PlayerGUI::setPlayer(Player* playerPtr)
{
    player = playerPtr;
    updateLoadState();
    updatePlayerLabels();
    syncUiFromState();
    syncPlayerFromState();
}

void PlayerGUI::paint(juce::Graphics& g)
{
    juce::ColourGradient gradient(backgroundTop,
                                  0.0f,
                                  0.0f,
                                  backgroundBottom,
                                  0.0f,
                                  static_cast<float>(getHeight()),
                                  false);
    gradient.addColour(0.32, backgroundBlue);
    gradient.addColour(0.7, backgroundPurple);
    g.setGradientFill(gradient);
    g.fillAll();

    g.setColour(accentColour.withAlpha(0.18f));
    g.fillEllipse(static_cast<float>(getWidth() - 340), -60.0f, 340.0f, 340.0f);
    g.setColour(knobColour.withAlpha(0.16f));
    g.fillEllipse(-110.0f, static_cast<float>(getHeight() - 300), 320.0f, 320.0f);
    g.setColour(accentSoft.withAlpha(0.14f));
    g.fillEllipse(static_cast<float>(getWidth() * 0.34f), static_cast<float>(getHeight() - 180), 260.0f, 220.0f);

    drawCard(g, headerCardBounds);
    drawCard(g, masterCardBounds);
    drawCard(g, presetsCardBounds);
    drawCard(g, bedsCardBounds);
    drawCard(g, layersCardBounds);
    drawCard(g, solosCardBounds);

    for (int bankIndex = 0; bankIndex < Player::numBackgroundBanks; ++bankIndex)
    {
        const auto highlighted = bankIndex == selectedBackgroundBank;
        drawCard(g, bankCardBounds[bankIndex], highlighted);

        if (state.banks[bankIndex].active)
        {
            auto stripeBounds = bankCardBounds[bankIndex].withWidth(activeStripeWidth).reduced(0, 12).toFloat();
            g.setColour(successColour.withAlpha(0.9f));
            g.fillRoundedRectangle(stripeBounds, 3.0f);
        }
    }

    for (int soundIndex = 0; soundIndex < Player::numForegroundSounds; ++soundIndex)
    {
        drawCard(g, soloCardBounds[soundIndex], state.solos[soundIndex].active);
    }
}

void PlayerGUI::resized()
{
    auto bounds = getLocalBounds().reduced(outerPadding);
    headerCardBounds = bounds.removeFromTop(104);
    bounds.removeFromTop(cardGap);

    if (getWidth() >= 1240)
    {
        auto topRow = bounds.removeFromTop(356);
        bounds.removeFromTop(cardGap);
        auto bottomRow = bounds;

        auto leftRail = topRow.removeFromLeft(376);
        masterCardBounds = leftRail.removeFromTop(168);
        leftRail.removeFromTop(cardGap);
        presetsCardBounds = leftRail;
        topRow.removeFromLeft(cardGap);
        bedsCardBounds = topRow;

        auto bottomLeft = bottomRow.removeFromLeft(680);
        bottomRow.removeFromLeft(cardGap);
        layersCardBounds = bottomLeft;
        solosCardBounds = bottomRow;
    }
    else
    {
        masterCardBounds = bounds.removeFromTop(168);
        bounds.removeFromTop(cardGap);
        presetsCardBounds = bounds.removeFromTop(236);
        bounds.removeFromTop(cardGap);
        bedsCardBounds = bounds.removeFromTop(350);
        bounds.removeFromTop(cardGap);
        layersCardBounds = bounds.removeFromTop(280);
        bounds.removeFromTop(cardGap);
        solosCardBounds = bounds;
    }

    {
        auto inner = headerCardBounds.reduced(18, 16);
        auto actions = inner.removeFromRight(332);
        titleLabel.setBounds(inner.removeFromTop(28));
        inner.removeFromTop(2);
        subtitleLabel.setBounds(inner.removeFromTop(20));
        inner.removeFromTop(6);
        statusLabel.setBounds(inner.removeFromTop(18));

        auto actionRow = actions.removeFromTop(36);
        stopAllButton.setBounds(actionRow.removeFromLeft(152));
        actionRow.removeFromLeft(10);
        resetAllSlidersButton.setBounds(actionRow);
    }

    {
        auto inner = masterCardBounds.reduced(18, 14);
        masterSectionLabel.setBounds(inner.removeFromTop(24));
        inner.removeFromTop(8);

        auto backgroundRow = inner.removeFromTop(46);
        backgroundMasterLabel.setBounds(backgroundRow.removeFromLeft(104));
        backgroundRow.removeFromLeft(10);
        backgroundMasterSlider.setBounds(backgroundRow);

        inner.removeFromTop(8);

        auto foregroundRow = inner.removeFromTop(46);
        foregroundMasterLabel.setBounds(foregroundRow.removeFromLeft(104));
        foregroundRow.removeFromLeft(10);
        foregroundMasterSlider.setBounds(foregroundRow);

        inner.removeFromTop(10);
        auto presetRow = inner.removeFromTop(36);
        const auto buttonWidth = (presetRow.getWidth() - 16) / 3;
        driftPresetButton.setBounds(presetRow.removeFromLeft(buttonWidth));
        presetRow.removeFromLeft(8);
        focusPresetButton.setBounds(presetRow.removeFromLeft(buttonWidth));
        presetRow.removeFromLeft(8);
        detailPresetButton.setBounds(presetRow);
    }

    {
        auto inner = presetsCardBounds.reduced(18, 14);
        auto titleRow = inner.removeFromTop(24);
        presetsSectionLabel.setBounds(titleRow.removeFromLeft(180));
        presetCountLabel.setBounds(titleRow);
        inner.removeFromTop(8);

        presetNameEditor.setBounds(inner.removeFromTop(34));
        inner.removeFromTop(8);

        auto buttonsRow = inner.removeFromTop(36);
        const auto buttonWidth = (buttonsRow.getWidth() - 16) / 3;
        savePresetButton.setBounds(buttonsRow.removeFromLeft(buttonWidth));
        buttonsRow.removeFromLeft(8);
        loadPresetButton.setBounds(buttonsRow.removeFromLeft(buttonWidth));
        buttonsRow.removeFromLeft(8);
        deletePresetButton.setBounds(buttonsRow);
        inner.removeFromTop(10);

        presetListBox.setBounds(inner);
        presetEmptyLabel.setBounds(inner.reduced(8));
    }

    {
        auto inner = bedsCardBounds.reduced(18, 14);
        bedsSectionLabel.setBounds(inner.removeFromTop(24));
        inner.removeFromTop(10);

        const auto useThreeColumns = inner.getWidth() >= 860;
        const auto columnCount = useThreeColumns ? 3 : 2;
        const auto rowCount = (Player::numBackgroundBanks + columnCount - 1) / columnCount;
        const auto gridWidth = inner.getWidth();
        const auto gridHeight = inner.getHeight();
        const auto columnGap = 12;
        const auto rowGap = 10;
        const auto columnWidth = (gridWidth - columnGap * (columnCount - 1)) / columnCount;
        const auto cardHeight = (gridHeight - rowGap * (rowCount - 1)) / rowCount;

        for (int bankIndex = 0; bankIndex < Player::numBackgroundBanks; ++bankIndex)
        {
            const auto rowIndex = bankIndex / columnCount;
            const auto columnIndex = bankIndex % columnCount;
            const auto x = inner.getX() + (columnWidth + columnGap) * columnIndex;
            const auto y = inner.getY() + (cardHeight + rowGap) * rowIndex;
            bankCardBounds[bankIndex] = { x, y, columnWidth, cardHeight };

            const auto compactCard = cardHeight < 104 || columnWidth < 320;
            const auto horizontalPadding = compactCard ? 10 : 14;
            const auto verticalPadding = compactCard ? 8 : 12;
            const auto topRowHeight = compactCard ? 24 : 34;
            const auto toggleWidth = compactCard ? 74 : 96;
            const auto resetWidth = compactCard ? 56 : 72;
            const auto sectionGap = compactCard ? 4 : 8;
            const auto summaryHeight = compactCard ? 14 : 20;
            const auto sliderHeight = compactCard ? 22 : 32;

            auto cardInner = bankCardBounds[bankIndex].reduced(horizontalPadding, verticalPadding);
            auto topRow = cardInner.removeFromTop(topRowHeight);
            bankToggleButtons[bankIndex].setBounds(topRow.removeFromRight(toggleWidth));
            topRow.removeFromRight(8);
            bankResetButtons[bankIndex].setBounds(topRow.removeFromRight(resetWidth));
            topRow.removeFromRight(8);
            bankSelectButtons[bankIndex].setBounds(topRow);

            cardInner.removeFromTop(sectionGap);
            bankSummaryLabels[bankIndex].setBounds(cardInner.removeFromTop(summaryHeight));
            cardInner.removeFromTop(sectionGap);
            bankVolumeSliders[bankIndex].setBounds(cardInner.removeFromTop(sliderHeight));
        }
    }

    {
        auto inner = layersCardBounds.reduced(18, 14);
        layersSectionLabel.setBounds(inner.removeFromTop(24));
        inner.removeFromTop(6);
        selectedBankLabel.setBounds(inner.removeFromTop(28));
        selectedBankHintLabel.setBounds(inner.removeFromTop(22));
        inner.removeFromTop(10);

        const auto visibleLayerCount = getBackgroundLayerCount(selectedBackgroundBank);
        for (int layerIndex = 0; layerIndex < Player::maxBackgroundLayers; ++layerIndex)
        {
            if (layerIndex >= visibleLayerCount)
            {
                layerLabels[layerIndex].setBounds({});
                layerSliders[layerIndex].setBounds({});
                continue;
            }

            auto row = inner.removeFromTop(30);
            layerLabels[layerIndex].setBounds(row.removeFromLeft(170));
            row.removeFromLeft(10);
            layerSliders[layerIndex].setBounds(row);
            inner.removeFromTop(8);
        }
    }

    {
        auto inner = solosCardBounds.reduced(18, 14);
        solosSectionLabel.setBounds(inner.removeFromTop(24));
        inner.removeFromTop(10);

        for (int soundIndex = 0; soundIndex < Player::numForegroundSounds; ++soundIndex)
        {
            auto rowBounds = inner.removeFromTop(soloRowHeight);
            soloCardBounds[soundIndex] = rowBounds;

            auto row = rowBounds.reduced(12, 10);
            soloToggleButtons[soundIndex].setBounds(row.removeFromLeft(156));
            row.removeFromLeft(10);
            auto panBounds = row.removeFromRight(164);
            row.removeFromRight(10);
            soloVolumeSliders[soundIndex].setBounds(row);
            soloPanSliders[soundIndex].setBounds(panBounds);

            inner.removeFromTop(8);
        }
    }
}

void PlayerGUI::buttonClicked(juce::Button* button)
{
    if (button == &stopAllButton)
    {
        stopAllPlayback();
        return;
    }

    if (button == &resetAllSlidersButton)
    {
        resetAllSliders();
        return;
    }

    if (button == &driftPresetButton)
    {
        applyMixPreset(driftBackground, driftSolo, "Drift mix applied.");
        return;
    }

    if (button == &focusPresetButton)
    {
        applyMixPreset(focusBackground, focusSolo, "Focus mix applied.");
        return;
    }

    if (button == &detailPresetButton)
    {
        applyMixPreset(detailBackground, detailSolo, "Detail mix applied.");
        return;
    }

    if (button == &savePresetButton)
    {
        saveCurrentPreset();
        return;
    }

    if (button == &loadPresetButton)
    {
        loadSelectedPreset();
        return;
    }

    if (button == &deletePresetButton)
    {
        deleteSelectedPreset();
        return;
    }

    for (int bankIndex = 0; bankIndex < Player::numBackgroundBanks; ++bankIndex)
    {
        if (button == &bankToggleButtons[bankIndex])
        {
            state.banks[bankIndex].active = !state.banks[bankIndex].active;
            selectedBackgroundBank = bankIndex;
            commitStateChange(getBackgroundBankName(bankIndex) + (state.banks[bankIndex].active ? " enabled." : " disabled."));
            return;
        }

        if (button == &bankSelectButtons[bankIndex])
        {
            selectedBackgroundBank = bankIndex;
            syncUiFromState();
            setStatusMessage(getBackgroundBankName(bankIndex) + " selected for layer blend.");
            return;
        }

        if (button == &bankResetButtons[bankIndex])
        {
            resetBankSliders(bankIndex);
            return;
        }
    }

    for (int soundIndex = 0; soundIndex < Player::numForegroundSounds; ++soundIndex)
    {
        if (button == &soloToggleButtons[soundIndex])
        {
            state.solos[soundIndex].active = !state.solos[soundIndex].active;
            commitStateChange(getForegroundSoundName(soundIndex) + (state.solos[soundIndex].active ? " enabled." : " disabled."));
            return;
        }
    }
}

void PlayerGUI::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &backgroundMasterSlider)
    {
        state.masterBackground = clampPercent(static_cast<float>(backgroundMasterSlider.getValue()));
        commitStateChange();
        return;
    }

    if (slider == &foregroundMasterSlider)
    {
        state.masterSolo = clampPercent(static_cast<float>(foregroundMasterSlider.getValue()));
        commitStateChange();
        return;
    }

    for (int bankIndex = 0; bankIndex < Player::numBackgroundBanks; ++bankIndex)
    {
        if (slider == &bankVolumeSliders[bankIndex])
        {
            state.banks[bankIndex].volume = clampPercent(static_cast<float>(bankVolumeSliders[bankIndex].getValue()));
            commitStateChange();
            return;
        }
    }

    for (int layerIndex = 0; layerIndex < Player::maxBackgroundLayers; ++layerIndex)
    {
        if (slider == &layerSliders[layerIndex])
        {
            state.banks[selectedBackgroundBank].layerVolumes[layerIndex] = clampPercent(static_cast<float>(layerSliders[layerIndex].getValue()));
            commitStateChange();
            return;
        }
    }

    for (int soundIndex = 0; soundIndex < Player::numForegroundSounds; ++soundIndex)
    {
        if (slider == &soloVolumeSliders[soundIndex])
        {
            state.solos[soundIndex].volume = clampPercent(static_cast<float>(soloVolumeSliders[soundIndex].getValue()));
            commitStateChange();
            return;
        }

        if (slider == &soloPanSliders[soundIndex])
        {
            state.solos[soundIndex].pan = juce::jlimit(-100.0f, 100.0f, static_cast<float>(soloPanSliders[soundIndex].getValue()));
            commitStateChange();
            return;
        }
    }
}

int PlayerGUI::getNumRows()
{
    return static_cast<int>(savedPresets.size());
}

void PlayerGUI::paintListBoxItem(int rowNumber,
                                 juce::Graphics& g,
                                 int width,
                                 int height,
                                 bool rowIsSelected)
{
    if (rowNumber < 0 || rowNumber >= static_cast<int>(savedPresets.size()))
    {
        return;
    }

    const auto bounds = juce::Rectangle<int>(0, 0, width, height).reduced(2);
    g.setColour(rowIsSelected ? accentSoft.withAlpha(0.3f) : juce::Colour::fromRGBA(0x11, 0x0E, 0x22, 0xD8));
    g.fillRoundedRectangle(bounds.toFloat(), 12.0f);
    g.setColour(rowIsSelected ? accentColour : cardStroke);
    g.drawRoundedRectangle(bounds.toFloat(), 12.0f, rowIsSelected ? 1.8f : 1.0f);

    const auto& preset = savedPresets[static_cast<size_t>(rowNumber)];
    auto inner = bounds.reduced(12, 8);
    auto rightInfo = inner.removeFromRight(96);

    g.setColour(titleColour);
    g.setFont(juce::Font(14.5f, juce::Font::bold));
    g.drawFittedText(preset.name, inner.removeFromTop(18), juce::Justification::centredLeft, 1);

    g.setColour(bodyColour);
    g.setFont(juce::Font(12.5f, juce::Font::plain));
    g.drawFittedText(summarisePreset(preset), inner, juce::Justification::centredLeft, 1);

    g.setColour(bodyColour.withAlpha(0.82f));
    g.setFont(juce::Font(11.5f, juce::Font::plain));
    const auto stamp = juce::Time(preset.updatedAt).formatted("%d %b\n%H:%M");
    g.drawFittedText(stamp, rightInfo, juce::Justification::centredRight, 2);
}

void PlayerGUI::selectedRowsChanged(int lastRowSelected)
{
    selectedPresetRow = lastRowSelected;
    updatePresetButtons();
}

void PlayerGUI::listBoxItemDoubleClicked(int row, const juce::MouseEvent& event)
{
    juce::ignoreUnused(event);
    selectedPresetRow = row;
    loadSelectedPreset();
}

void PlayerGUI::timerCallback()
{
    updateLoadState();

    if (statusMessage.isNotEmpty() && juce::Time::getCurrentTime().toMilliseconds() >= statusMessageExpiryMs)
    {
        statusMessage.clear();
        statusMessageExpiryMs = 0;
        updateStatusText();
    }
}

float PlayerGUI::cube(float input) const
{
    const auto normalized = clampPercent(input) / 100.0f;
    return normalized * normalized * normalized;
}

float PlayerGUI::clampPercent(float value) const
{
    return juce::jlimit(0.0f, 100.0f, value);
}

float PlayerGUI::clampPanPercent(float value) const
{
    return juce::jlimit(-100.0f, 100.0f, value) / 200.0f + 0.5f;
}

PlayerGUI::AppState PlayerGUI::createDefaultState() const
{
    AppState defaults;

    for (auto& bank : defaults.banks)
    {
        bank.active = false;
        bank.volume = 70.0f;
        bank.layerVolumes.fill(50.0f);
    }

    for (auto& solo : defaults.solos)
    {
        solo.active = false;
        solo.volume = 62.0f;
        solo.pan = 0.0f;
    }

    return defaults;
}

PlayerGUI::AppState PlayerGUI::deserialiseState(const juce::var& source, const AppState& fallback) const
{
    AppState next = fallback;
    const auto* object = source.getDynamicObject();
    if (object == nullptr)
    {
        return next;
    }

    next.masterBackground = clampPercent(static_cast<float>(object->getProperty("masterBackground")));
    next.masterSolo = clampPercent(static_cast<float>(object->getProperty("masterSolo")));

    if (const auto* banks = object->getProperty("banks").getArray())
    {
        for (int bankIndex = 0; bankIndex < juce::jmin(Player::numBackgroundBanks, banks->size()); ++bankIndex)
        {
            if (const auto* bankObject = banks->getReference(bankIndex).getDynamicObject())
            {
                next.banks[bankIndex].active = static_cast<bool>(bankObject->getProperty("active"));
                next.banks[bankIndex].volume = clampPercent(static_cast<float>(bankObject->getProperty("volume")));

                if (const auto* layers = bankObject->getProperty("layerVolumes").getArray())
                {
                    for (int layerIndex = 0; layerIndex < juce::jmin(Player::maxBackgroundLayers, layers->size()); ++layerIndex)
                    {
                        next.banks[bankIndex].layerVolumes[layerIndex] = clampPercent(static_cast<float>(layers->getReference(layerIndex)));
                    }
                }
            }
        }
    }

    if (const auto* solos = object->getProperty("solos").getArray())
    {
        for (int soundIndex = 0; soundIndex < juce::jmin(Player::numForegroundSounds, solos->size()); ++soundIndex)
        {
            if (const auto* soloObject = solos->getReference(soundIndex).getDynamicObject())
            {
                next.solos[soundIndex].active = static_cast<bool>(soloObject->getProperty("active"));
                next.solos[soundIndex].volume = clampPercent(static_cast<float>(soloObject->getProperty("volume")));
                next.solos[soundIndex].pan = juce::jlimit(-100.0f, 100.0f, static_cast<float>(soloObject->getProperty("pan")));
            }
        }
    }

    return next;
}

juce::var PlayerGUI::serialiseState(const AppState& source) const
{
    auto root = std::make_unique<juce::DynamicObject>();
    root->setProperty("masterBackground", source.masterBackground);
    root->setProperty("masterSolo", source.masterSolo);

    juce::Array<juce::var> banks;
    for (int bankIndex = 0; bankIndex < Player::numBackgroundBanks; ++bankIndex)
    {
        auto bankObject = std::make_unique<juce::DynamicObject>();
        bankObject->setProperty("active", source.banks[bankIndex].active);
        bankObject->setProperty("volume", source.banks[bankIndex].volume);

        juce::Array<juce::var> layers;
        for (int layerIndex = 0; layerIndex < Player::maxBackgroundLayers; ++layerIndex)
        {
            layers.add(source.banks[bankIndex].layerVolumes[layerIndex]);
        }

        bankObject->setProperty("layerVolumes", juce::var(layers));
        banks.add(juce::var(bankObject.release()));
    }

    juce::Array<juce::var> solos;
    for (int soundIndex = 0; soundIndex < Player::numForegroundSounds; ++soundIndex)
    {
        auto soloObject = std::make_unique<juce::DynamicObject>();
        soloObject->setProperty("active", source.solos[soundIndex].active);
        soloObject->setProperty("volume", source.solos[soundIndex].volume);
        soloObject->setProperty("pan", source.solos[soundIndex].pan);
        solos.add(juce::var(soloObject.release()));
    }

    root->setProperty("banks", juce::var(banks));
    root->setProperty("solos", juce::var(solos));
    return juce::var(root.release());
}

juce::String PlayerGUI::summarisePreset(const SavedPreset& preset) const
{
    const auto activeBanks = static_cast<int>(std::count_if(preset.state.banks.begin(),
                                                            preset.state.banks.end(),
                                                            [](const BankState& bank)
                                                            {
                                                                return bank.active;
                                                            }));

    const auto activeSolos = static_cast<int>(std::count_if(preset.state.solos.begin(),
                                                            preset.state.solos.end(),
                                                            [](const SoloState& solo)
                                                            {
                                                                return solo.active;
                                                            }));

    return pluralise(activeBanks, "bed", "beds")
        + ", "
        + pluralise(activeSolos, "loop", "loops")
        + " | Mix "
        + juce::String(static_cast<int>(preset.state.masterBackground))
        + "/"
        + juce::String(static_cast<int>(preset.state.masterSolo));
}

juce::String PlayerGUI::createPresetId() const
{
    return juce::Uuid().toDashedString();
}

juce::String PlayerGUI::getBackgroundBankName(int bankIndex) const
{
    if (player != nullptr)
    {
        const auto name = player->getBackgroundBankName(bankIndex).trim();
        if (name.isNotEmpty())
        {
            return name;
        }
    }

    return bankIndex >= 0 && bankIndex < Player::numBackgroundBanks
        ? juce::String(fallbackBackgroundBankNames[static_cast<size_t>(bankIndex)])
        : juce::String();
}

juce::String PlayerGUI::getForegroundSoundName(int soundIndex) const
{
    if (player != nullptr)
    {
        const auto name = player->getForegroundSoundName(soundIndex).trim();
        if (name.isNotEmpty())
        {
            return name;
        }
    }

    return soundIndex >= 0 && soundIndex < Player::numForegroundSounds
        ? juce::String(fallbackForegroundSoundNames[static_cast<size_t>(soundIndex)])
        : juce::String();
}

int PlayerGUI::getBackgroundLayerCount(int bankIndex) const
{
    if (player != nullptr)
    {
        return juce::jlimit(0, Player::maxBackgroundLayers, player->getBackgroundLayerCount(bankIndex));
    }

    return 0;
}

juce::String PlayerGUI::formatPanLabel(float pan) const
{
    if (std::abs(pan) < 0.5f)
    {
        return "Centre";
    }

    return pan < 0.0f
        ? "L " + juce::String(static_cast<int>(std::round(std::abs(pan))))
        : "R " + juce::String(static_cast<int>(std::round(std::abs(pan))));
}

void PlayerGUI::initialiseLookAndFeel()
{
    lookAndFeel.setColour(juce::TextButton::buttonColourId, panelButtonColour);
    lookAndFeel.setColour(juce::TextButton::buttonOnColourId, accentSoft);
    lookAndFeel.setColour(juce::TextButton::textColourOffId, titleColour);
    lookAndFeel.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    lookAndFeel.setColour(juce::Slider::thumbColourId, juce::Colours::white);
    lookAndFeel.setColour(juce::Slider::trackColourId, sliderTrackColour);
    lookAndFeel.setColour(juce::Slider::backgroundColourId, knobTrackColour.withAlpha(0.55f));
    lookAndFeel.setColour(juce::Slider::rotarySliderFillColourId, knobColour);
    lookAndFeel.setColour(juce::Slider::rotarySliderOutlineColourId, knobTrackColour);
    lookAndFeel.setColour(juce::Slider::textBoxTextColourId, titleColour);
    lookAndFeel.setColour(juce::Slider::textBoxBackgroundColourId, textFieldColour);
    lookAndFeel.setColour(juce::Slider::textBoxOutlineColourId, cardStroke);
    lookAndFeel.setColour(juce::Slider::textBoxHighlightColourId, accentSoft.withAlpha(0.28f));
    lookAndFeel.setColour(juce::Label::textColourId, bodyColour);
    lookAndFeel.setColour(juce::ListBox::outlineColourId, juce::Colours::transparentBlack);
}

void PlayerGUI::initialiseSettings()
{
    juce::PropertiesFile::Options options;
    options.applicationName = "SoundscapeApplication";
    options.filenameSuffix = "settings";
    options.folderName = "FiveReasons";
    options.osxLibrarySubFolder = "Application Support";
    options.commonToAllUsers = false;
    settingsFile = std::make_unique<juce::PropertiesFile>(options);
}

void PlayerGUI::restoreState()
{
    if (settingsFile == nullptr)
    {
        return;
    }

    const auto raw = settingsFile->getValue("desktopDashboardState");
    if (raw.isEmpty())
    {
        return;
    }

    state = deserialiseState(juce::JSON::parse(raw), createDefaultState());
}

void PlayerGUI::restorePresets()
{
    savedPresets.clear();
    if (settingsFile == nullptr)
    {
        return;
    }

    const auto raw = settingsFile->getValue("desktopDashboardPresets");
    if (raw.isEmpty())
    {
        return;
    }

    const auto parsed = juce::JSON::parse(raw);
    const auto* array = parsed.getArray();
    if (array == nullptr)
    {
        return;
    }

    for (const auto& item : *array)
    {
        const auto* object = item.getDynamicObject();
        if (object == nullptr)
        {
            continue;
        }

        const auto name = sanitisePresetName(object->getProperty("name").toString());
        if (name.isEmpty())
        {
            continue;
        }

        SavedPreset preset;
        preset.id = object->getProperty("id").toString();
        if (preset.id.isEmpty())
        {
            preset.id = createPresetId();
        }
        preset.name = name;
        preset.updatedAt = static_cast<juce::int64>(object->getProperty("updatedAt"));
        if (preset.updatedAt <= 0)
        {
            preset.updatedAt = juce::Time::getCurrentTime().toMilliseconds();
        }
        preset.state = deserialiseState(object->getProperty("state"), createDefaultState());
        savedPresets.push_back(std::move(preset));
    }

    std::sort(savedPresets.begin(),
              savedPresets.end(),
              [](const SavedPreset& left, const SavedPreset& right)
              {
                  return left.updatedAt > right.updatedAt;
              });

    if (savedPresets.size() > static_cast<size_t>(maxSavedPresets))
    {
        savedPresets.resize(static_cast<size_t>(maxSavedPresets));
    }
}

void PlayerGUI::persistState()
{
    if (settingsFile == nullptr)
    {
        return;
    }

    settingsFile->setValue("desktopDashboardState", juce::JSON::toString(serialiseState(state)));
    settingsFile->saveIfNeeded();
}

void PlayerGUI::persistPresets()
{
    if (settingsFile == nullptr)
    {
        return;
    }

    juce::Array<juce::var> array;
    for (const auto& preset : savedPresets)
    {
        auto object = std::make_unique<juce::DynamicObject>();
        object->setProperty("id", preset.id);
        object->setProperty("name", preset.name);
        object->setProperty("updatedAt", preset.updatedAt);
        object->setProperty("state", serialiseState(preset.state));
        array.add(juce::var(object.release()));
    }

    settingsFile->setValue("desktopDashboardPresets", juce::JSON::toString(juce::var(array)));
    settingsFile->saveIfNeeded();
}

void PlayerGUI::configurePrimaryButton(juce::TextButton& button)
{
    button.setColour(juce::TextButton::buttonColourId, accentColour);
    button.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    button.setColour(juce::TextButton::buttonOnColourId, accentColour.darker(0.18f));
    button.setTriggeredOnMouseDown(false);
}

void PlayerGUI::configureSecondaryButton(juce::TextButton& button)
{
    button.setColour(juce::TextButton::buttonColourId, panelButtonColour);
    button.setColour(juce::TextButton::textColourOffId, titleColour);
    button.setColour(juce::TextButton::buttonOnColourId, accentSoft);
    button.setTriggeredOnMouseDown(false);
}

void PlayerGUI::configurePresetButton(juce::TextButton& button, const juce::String& text)
{
    configureSecondaryButton(button);
    button.setButtonText(text);
}

void PlayerGUI::configureSectionLabel(juce::Label& label, const juce::String& text, float fontHeight, juce::Colour colour)
{
    prepareLabel(label, text, fontHeight, colour);
}

void PlayerGUI::configureHorizontalSlider(juce::Slider& slider, bool bipolar)
{
    slider.setSliderStyle(juce::Slider::LinearHorizontal);
    slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, bipolar ? 64 : 56, 22);
    slider.setRange(bipolar ? -100.0 : 0.0, 100.0, 1.0);
    slider.setColour(juce::Slider::thumbColourId, juce::Colours::white);
    slider.setColour(juce::Slider::trackColourId, sliderTrackColour);
    slider.setColour(juce::Slider::backgroundColourId, knobTrackColour.withAlpha(0.42f));
    slider.setColour(juce::Slider::textBoxTextColourId, titleColour);
    slider.setColour(juce::Slider::textBoxBackgroundColourId, textFieldColour);
    slider.setColour(juce::Slider::textBoxOutlineColourId, cardStroke);
    slider.setColour(juce::Slider::textBoxHighlightColourId, accentSoft.withAlpha(0.28f));
}

void PlayerGUI::updateLoadState()
{
    const auto previousState = loadState;

    if (player == nullptr)
    {
        loadState = LoadState::NoPlayer;
    }
    else if (player->didLoadFail())
    {
        loadState = LoadState::Failed;
    }
    else if (player->isLoaded())
    {
        loadState = LoadState::Ready;
    }
    else if (player->isLoading())
    {
        loadState = LoadState::Loading;
    }
    else
    {
        loadState = LoadState::Preparing;
    }

    if (loadState != previousState)
    {
        setAudioControlsEnabled(loadState == LoadState::Ready);
        updatePlayerLabels();
        syncUiFromState();
        syncPlayerFromState();
        updateStatusText();
    }
}

void PlayerGUI::setAudioControlsEnabled(bool enabled)
{
    audioControlsEnabled = enabled;

    stopAllButton.setEnabled(enabled);
    resetAllSlidersButton.setEnabled(enabled);
    driftPresetButton.setEnabled(enabled);
    focusPresetButton.setEnabled(enabled);
    detailPresetButton.setEnabled(enabled);
    presetNameEditor.setEnabled(enabled);
    backgroundMasterSlider.setEnabled(enabled);
    foregroundMasterSlider.setEnabled(enabled);

    for (int bankIndex = 0; bankIndex < Player::numBackgroundBanks; ++bankIndex)
    {
        bankToggleButtons[bankIndex].setEnabled(enabled);
        bankSelectButtons[bankIndex].setEnabled(enabled);
        bankResetButtons[bankIndex].setEnabled(enabled);
        bankVolumeSliders[bankIndex].setEnabled(enabled);
    }

    for (int layerIndex = 0; layerIndex < Player::maxBackgroundLayers; ++layerIndex)
    {
        layerSliders[layerIndex].setEnabled(enabled);
    }

    for (int soundIndex = 0; soundIndex < Player::numForegroundSounds; ++soundIndex)
    {
        soloToggleButtons[soundIndex].setEnabled(enabled);
        soloVolumeSliders[soundIndex].setEnabled(enabled);
        soloPanSliders[soundIndex].setEnabled(enabled);
    }

    updatePresetButtons();
    updateSavePresetAvailability();
}

void PlayerGUI::updatePlayerLabels()
{
    for (int bankIndex = 0; bankIndex < Player::numBackgroundBanks; ++bankIndex)
    {
        bankSelectButtons[bankIndex].setButtonText(getBackgroundBankName(bankIndex));
    }

    for (int soundIndex = 0; soundIndex < Player::numForegroundSounds; ++soundIndex)
    {
        soloToggleButtons[soundIndex].setButtonText(getForegroundSoundName(soundIndex));
    }

    updateSelectedBankUi();
}

void PlayerGUI::updatePresetButtons()
{
    const auto driftSelected = std::abs(state.masterBackground - driftBackground) < 0.5f
        && std::abs(state.masterSolo - driftSolo) < 0.5f;
    const auto focusSelected = std::abs(state.masterBackground - focusBackground) < 0.5f
        && std::abs(state.masterSolo - focusSolo) < 0.5f;
    const auto detailSelected = std::abs(state.masterBackground - detailBackground) < 0.5f
        && std::abs(state.masterSolo - detailSolo) < 0.5f;

    driftPresetButton.setColour(juce::TextButton::buttonColourId, driftSelected ? accentColour : panelButtonColour);
    driftPresetButton.setColour(juce::TextButton::textColourOffId, driftSelected ? juce::Colours::white : titleColour);
    focusPresetButton.setColour(juce::TextButton::buttonColourId, focusSelected ? accentColour : panelButtonColour);
    focusPresetButton.setColour(juce::TextButton::textColourOffId, focusSelected ? juce::Colours::white : titleColour);
    detailPresetButton.setColour(juce::TextButton::buttonColourId, detailSelected ? accentColour : panelButtonColour);
    detailPresetButton.setColour(juce::TextButton::textColourOffId, detailSelected ? juce::Colours::white : titleColour);

    loadPresetButton.setEnabled(audioControlsEnabled && selectedPresetRow >= 0 && selectedPresetRow < static_cast<int>(savedPresets.size()));
    deletePresetButton.setEnabled(audioControlsEnabled && selectedPresetRow >= 0 && selectedPresetRow < static_cast<int>(savedPresets.size()));
}

void PlayerGUI::updatePresetList()
{
    presetListBox.updateContent();

    if (selectedPresetRow >= static_cast<int>(savedPresets.size()))
    {
        selectedPresetRow = static_cast<int>(savedPresets.size()) - 1;
    }

    if (selectedPresetRow >= 0)
    {
        presetListBox.selectRow(selectedPresetRow, false, false);
    }
    else
    {
        presetListBox.deselectAllRows();
    }

    presetCountLabel.setText(juce::String(savedPresets.size()) + " saved", juce::dontSendNotification);
    presetEmptyLabel.setVisible(savedPresets.empty());
    updatePresetButtons();
}

void PlayerGUI::updateSavePresetAvailability()
{
    savePresetButton.setEnabled(audioControlsEnabled && sanitisePresetName(presetNameEditor.getText()).isNotEmpty());
}

void PlayerGUI::updateSelectedBankUi()
{
    const auto bankName = getBackgroundBankName(selectedBackgroundBank);
    const auto layerCount = getBackgroundLayerCount(selectedBackgroundBank);

    const auto bankIsActive = state.banks[selectedBackgroundBank].active;
    selectedBankLabel.setText(bankName.isNotEmpty() ? bankName + (bankIsActive ? " | Live" : " | Standby") : "Select a bed", juce::dontSendNotification);
    selectedBankHintLabel.setText(layerCount > 0
                                      ? juce::String(layerCount) + " layer" + (layerCount == 1 ? "" : "s") + " ready | Reset keeps the bed on/off state."
                                      : "Audio labels will populate after the library is ready.",
                                  juce::dontSendNotification);

    for (int layerIndex = 0; layerIndex < Player::maxBackgroundLayers; ++layerIndex)
    {
        const auto visible = layerIndex < layerCount;
        layerLabels[layerIndex].setVisible(visible);
        layerSliders[layerIndex].setVisible(visible);

        if (!visible)
        {
            continue;
        }

        auto layerName = player != nullptr ? player->getBackgroundLayerName(selectedBackgroundBank, layerIndex) : juce::String();
        if (layerName.isEmpty())
        {
            layerName = "Layer " + juce::String(layerIndex + 1);
        }

        layerLabels[layerIndex].setText(layerName, juce::dontSendNotification);
        layerSliders[layerIndex].setValue(state.banks[selectedBackgroundBank].layerVolumes[layerIndex], juce::dontSendNotification);
    }

    resized();
}

void PlayerGUI::updateBankCards()
{
    for (int bankIndex = 0; bankIndex < Player::numBackgroundBanks; ++bankIndex)
    {
        const auto selected = bankIndex == selectedBackgroundBank;
        const auto active = state.banks[bankIndex].active;
        const auto layerCount = getBackgroundLayerCount(bankIndex);
        const auto name = getBackgroundBankName(bankIndex);

        bankSelectButtons[bankIndex].setButtonText(name);
        bankToggleButtons[bankIndex].setButtonText(active ? "Live" : "Standby");
        bankResetButtons[bankIndex].setButtonText("Reset");
        bankVolumeSliders[bankIndex].setValue(state.banks[bankIndex].volume, juce::dontSendNotification);
        bankSelectButtons[bankIndex].setTooltip(selected ? "Currently editing layer blend for " + name + "." : "Select " + name + " to edit its layer blend.");
        bankToggleButtons[bankIndex].setTooltip(active ? "Fade " + name + " out." : "Fade " + name + " in.");
        bankResetButtons[bankIndex].setTooltip("Reset " + name + " scene level and layer blend.");
        bankVolumeSliders[bankIndex].setTooltip(name + " scene level");

        bankSelectButtons[bankIndex].setColour(juce::TextButton::buttonColourId, selected ? accentSoft : panelButtonColour);
        bankSelectButtons[bankIndex].setColour(juce::TextButton::textColourOffId, selected ? juce::Colours::white : titleColour);
        bankToggleButtons[bankIndex].setColour(juce::TextButton::buttonColourId, active ? accentColour : panelButtonColour);
        bankToggleButtons[bankIndex].setColour(juce::TextButton::textColourOffId, active ? juce::Colours::white : titleColour);

        juce::String summary = active ? "Live" : "Standby";
        if (selected)
        {
            summary = "Selected | " + summary;
        }

        bankSummaryLabels[bankIndex].setText(
            summary
                + juce::String(" | ")
                + juce::String(layerCount)
                + " layer"
                + (layerCount == 1 ? "" : "s"),
            juce::dontSendNotification);
    }
}

void PlayerGUI::updateSoloCards()
{
    for (int soundIndex = 0; soundIndex < Player::numForegroundSounds; ++soundIndex)
    {
        const auto active = state.solos[soundIndex].active;
        const auto soundName = getForegroundSoundName(soundIndex);
        soloToggleButtons[soundIndex].setButtonText(soundName);
        soloVolumeSliders[soundIndex].setValue(state.solos[soundIndex].volume, juce::dontSendNotification);
        soloPanSliders[soundIndex].setValue(state.solos[soundIndex].pan, juce::dontSendNotification);
        soloToggleButtons[soundIndex].setTooltip(active ? "Fade " + soundName + " out." : "Fade " + soundName + " in.");
        soloVolumeSliders[soundIndex].setTooltip(soundName + " level");
        soloPanSliders[soundIndex].setTooltip(soundName + " pan: " + formatPanLabel(state.solos[soundIndex].pan));

        soloToggleButtons[soundIndex].setColour(juce::TextButton::buttonColourId, active ? accentColour : panelButtonColour);
        soloToggleButtons[soundIndex].setColour(juce::TextButton::textColourOffId, active ? juce::Colours::white : titleColour);
    }
}

void PlayerGUI::updateStatusText()
{
    if (statusMessage.isNotEmpty() && juce::Time::getCurrentTime().toMilliseconds() < statusMessageExpiryMs)
    {
        statusLabel.setText(statusMessage, juce::dontSendNotification);
        statusLabel.setColour(juce::Label::textColourId, bodyColour);
        return;
    }

    statusMessage.clear();
    statusMessageExpiryMs = 0;

    switch (loadState)
    {
        case LoadState::NoPlayer:
            statusLabel.setText("Waiting for audio engine.", juce::dontSendNotification);
            statusLabel.setColour(juce::Label::textColourId, bodyColour);
            break;
        case LoadState::Preparing:
            statusLabel.setText("Preparing audio engine...", juce::dontSendNotification);
            statusLabel.setColour(juce::Label::textColourId, bodyColour);
            break;
        case LoadState::Loading:
            statusLabel.setText("Loading sound library...", juce::dontSendNotification);
            statusLabel.setColour(juce::Label::textColourId, bodyColour);
            break;
        case LoadState::Ready:
            statusLabel.setText("Audio ready. Beds, loops, presets, and 1-second fades are live.", juce::dontSendNotification);
            statusLabel.setColour(juce::Label::textColourId, successColour.darker(0.12f));
            break;
        case LoadState::Failed:
            statusLabel.setText("Audio library failed to load. Check the Resources folder.", juce::dontSendNotification);
            statusLabel.setColour(juce::Label::textColourId, accentColour.darker(0.2f));
            break;
    }
}

void PlayerGUI::setStatusMessage(const juce::String& message)
{
    statusMessage = message;
    statusMessageExpiryMs = juce::Time::getCurrentTime().toMilliseconds() + transientStatusLifetimeMs;
    updateStatusText();
}

void PlayerGUI::commitStateChange(const juce::String& message)
{
    persistState();
    syncUiFromState();
    syncPlayerFromState();

    if (message.isNotEmpty())
    {
        setStatusMessage(message);
    }
    else
    {
        updateStatusText();
    }
}

void PlayerGUI::syncUiFromState()
{
    backgroundMasterSlider.setValue(state.masterBackground, juce::dontSendNotification);
    foregroundMasterSlider.setValue(state.masterSolo, juce::dontSendNotification);

    updatePresetButtons();
    updateSavePresetAvailability();
    updateSelectedBankUi();
    updateBankCards();
    updateSoloCards();
    repaint();
}

void PlayerGUI::syncPlayerFromState()
{
    if (player == nullptr)
    {
        return;
    }

    player->setGlobalBackgroundGain(cube(state.masterBackground));
    player->setGlobalForegroundGain(cube(state.masterSolo));

    for (int bankIndex = 0; bankIndex < Player::numBackgroundBanks; ++bankIndex)
    {
        player->setBackgroundBankGain(bankIndex, cube(state.banks[bankIndex].volume));
        player->setBackgroundBankPlaying(bankIndex, state.banks[bankIndex].active);

        for (int layerIndex = 0; layerIndex < Player::maxBackgroundLayers; ++layerIndex)
        {
            player->setBackgroundLayerGain(bankIndex, layerIndex, cube(state.banks[bankIndex].layerVolumes[layerIndex]));
        }
    }

    for (int soundIndex = 0; soundIndex < Player::numForegroundSounds; ++soundIndex)
    {
        player->setForegroundGain(soundIndex, cube(state.solos[soundIndex].volume));
        player->setForegroundPan(soundIndex, clampPanPercent(state.solos[soundIndex].pan));
        player->setForegroundSoundPlaying(soundIndex, state.solos[soundIndex].active);
    }
}

void PlayerGUI::setMasterMix(float backgroundValue, float soloValue)
{
    state.masterBackground = clampPercent(backgroundValue);
    state.masterSolo = clampPercent(soloValue);
}

void PlayerGUI::applyMixPreset(float backgroundValue, float soloValue, const juce::String& label)
{
    setMasterMix(backgroundValue, soloValue);
    commitStateChange(label);
}

void PlayerGUI::saveCurrentPreset()
{
    auto name = sanitisePresetName(presetNameEditor.getText());
    if (name.isEmpty())
    {
        updateSavePresetAvailability();
        return;
    }

    const auto existing = std::find_if(savedPresets.begin(),
                                       savedPresets.end(),
                                       [&name](const SavedPreset& preset)
                                       {
                                           return preset.name.equalsIgnoreCase(name);
                                       });

    SavedPreset nextPreset;
    nextPreset.id = existing != savedPresets.end() ? existing->id : createPresetId();
    nextPreset.name = name;
    nextPreset.updatedAt = juce::Time::getCurrentTime().toMilliseconds();
    nextPreset.state = state;

    if (existing != savedPresets.end())
    {
        *existing = nextPreset;
    }
    else
    {
        savedPresets.push_back(nextPreset);
    }

    std::sort(savedPresets.begin(),
              savedPresets.end(),
              [](const SavedPreset& left, const SavedPreset& right)
              {
                  return left.updatedAt > right.updatedAt;
              });

    if (savedPresets.size() > static_cast<size_t>(maxSavedPresets))
    {
        savedPresets.resize(static_cast<size_t>(maxSavedPresets));
    }

    persistPresets();
    updatePresetList();

    const auto selected = std::find_if(savedPresets.begin(),
                                       savedPresets.end(),
                                       [&nextPreset](const SavedPreset& preset)
                                       {
                                           return preset.id == nextPreset.id;
                                       });

    selectedPresetRow = selected != savedPresets.end() ? static_cast<int>(std::distance(savedPresets.begin(), selected)) : -1;
    updatePresetList();
    presetNameEditor.clear();
    updateSavePresetAvailability();
    setStatusMessage("Saved preset: " + name);
}

void PlayerGUI::loadSelectedPreset()
{
    if (selectedPresetRow < 0 || selectedPresetRow >= static_cast<int>(savedPresets.size()))
    {
        return;
    }

    state = savedPresets[static_cast<size_t>(selectedPresetRow)].state;
    commitStateChange("Loaded preset: " + savedPresets[static_cast<size_t>(selectedPresetRow)].name);
}

void PlayerGUI::deleteSelectedPreset()
{
    if (selectedPresetRow < 0 || selectedPresetRow >= static_cast<int>(savedPresets.size()))
    {
        return;
    }

    const auto name = savedPresets[static_cast<size_t>(selectedPresetRow)].name;
    savedPresets.erase(savedPresets.begin() + selectedPresetRow);
    if (selectedPresetRow >= static_cast<int>(savedPresets.size()))
    {
        selectedPresetRow = static_cast<int>(savedPresets.size()) - 1;
    }

    persistPresets();
    updatePresetList();
    setStatusMessage("Deleted preset: " + name);
}

void PlayerGUI::stopAllPlayback()
{
    for (auto& bank : state.banks)
    {
        bank.active = false;
    }

    for (auto& solo : state.solos)
    {
        solo.active = false;
    }

    commitStateChange("Stopped all beds and loops.");
}

void PlayerGUI::resetAllSliders()
{
    const auto defaults = createDefaultState();
    state.masterBackground = defaults.masterBackground;
    state.masterSolo = defaults.masterSolo;

    for (int bankIndex = 0; bankIndex < Player::numBackgroundBanks; ++bankIndex)
    {
        state.banks[bankIndex].volume = defaults.banks[bankIndex].volume;
        state.banks[bankIndex].layerVolumes = defaults.banks[bankIndex].layerVolumes;
    }

    for (int soundIndex = 0; soundIndex < Player::numForegroundSounds; ++soundIndex)
    {
        state.solos[soundIndex].volume = defaults.solos[soundIndex].volume;
        state.solos[soundIndex].pan = defaults.solos[soundIndex].pan;
    }

    commitStateChange("All sliders reset to default values.");
}

void PlayerGUI::resetBankSliders(int bankIndex)
{
    if (bankIndex < 0 || bankIndex >= Player::numBackgroundBanks)
    {
        return;
    }

    const auto defaults = createDefaultState();
    state.banks[bankIndex].volume = defaults.banks[bankIndex].volume;
    state.banks[bankIndex].layerVolumes = defaults.banks[bankIndex].layerVolumes;
    selectedBackgroundBank = bankIndex;
    commitStateChange(getBackgroundBankName(bankIndex) + " sliders reset to defaults.");
}