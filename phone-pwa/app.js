const query = new URLSearchParams(window.location.search);
const isAndroidWrapper = query.get("wrapper") === "android";
const storageKey = "soundscape-pocket-state-v1";
const presetStorageKey = "soundscape-pocket-presets-v1";
const resourceBase = isAndroidWrapper ? "./Resources" : "../Resources";
const manifestPath = `${resourceBase}/asset-manifest.current.json`;
const backgroundLayerBaseGain = 0.11;
const soloBaseGain = 0.68;
const defaultBackgroundLayerVolume = 100;
const maxSavedPresets = 16;
const toggleFadeSeconds = 1.0;
const bankStemLoopSeconds = 180;
const maxActiveStreamUnits = 16;
const stemStreamUnits = 1;
const soloStreamUnits = 1;
const layerMixDeadband = 1.0;

let mixPriorityCounter = 1;

const fallbackBankDefinitions = [
    {
        id: "ambient",
        title: "Ambient",
        note: "Choir, piano, and synth textures.",
        stemPath: "Soundscapes/Stems/ambient-stem.mp3",
        stemGainCompensation: 6,
        stemDurationSeconds: bankStemLoopSeconds,
        tracks: [
            "Soundscapes/Ambient/1. Bass.mp3",
            "Soundscapes/Ambient/2. Guitar.mp3",
            "Soundscapes/Ambient/3. Male Choir.mp3",
            "Soundscapes/Ambient/4. Female Choir.mp3",
            "Soundscapes/Ambient/5. Pluck Synth.mp3",
            "Soundscapes/Ambient/6. Piano.mp3"
        ]
    },
    {
        id: "storm",
        title: "Rainy Day",
        note: "Rain, wind, and distant thunder.",
        stemPath: "Soundscapes/Stems/storm-stem.mp3",
        stemGainCompensation: 6,
        stemDurationSeconds: bankStemLoopSeconds,
        tracks: [
            "Soundscapes/Storm/1. Light Rain.mp3",
            "Soundscapes/Storm/2. Breeze.mp3",
            "Soundscapes/Storm/3. Distant Thunder.mp3",
            "Soundscapes/Storm/4. Heavy Rain.mp3",
            "Soundscapes/Storm/5. Cold Wind.mp3",
            "Soundscapes/Storm/6. Close Thunder.mp3"
        ]
    },
    {
        id: "beach",
        title: "Seaside",
        note: "Wind, waves, and gulls.",
        stemPath: "Soundscapes/Stems/beach-stem.mp3",
        stemGainCompensation: 6,
        stemDurationSeconds: bankStemLoopSeconds,
        tracks: [
            "Soundscapes/Beach/1. Wind.mp3",
            "Soundscapes/Beach/2. Distant Waves.mp3",
            "Soundscapes/Beach/3. Pebble Waves.mp3",
            "Soundscapes/Beach/4. Gentle Waves.mp3",
            "Soundscapes/Beach/5. Ship Creak.mp3",
            "Soundscapes/Beach/6. Seagulls.mp3"
        ]
    },
    {
        id: "drone",
        title: "Drone",
        note: "Industrial hum and harmonic drone.",
        stemPath: "Soundscapes/Stems/drone-stem.mp3",
        stemGainCompensation: 6,
        stemDurationSeconds: bankStemLoopSeconds,
        tracks: [
            "Soundscapes/Drone/6. Synth.mp3",
            "Soundscapes/Drone/1. Guitar Feedback.mp3",
            "Soundscapes/Drone/4. Blender.mp3",
            "Soundscapes/Drone/5. Bottle.mp3",
            "Soundscapes/Drone/2. Oven.mp3",
            "Soundscapes/Drone/3. White Noise.mp3"
        ]
    },
    {
        id: "forest",
        title: "Forest",
        note: "Leaves, birdsong, and bees.",
        stemPath: "Soundscapes/Stems/forest-stem.mp3",
        stemGainCompensation: 6,
        stemDurationSeconds: bankStemLoopSeconds,
        tracks: [
            "Soundscapes/Forest/1. Leaves.mp3",
            "Soundscapes/Forest/2. Leaves.mp3",
            "Soundscapes/Forest/3. Birdsong.mp3",
            "Soundscapes/Forest/4. Birdsong.mp3",
            "Soundscapes/Forest/5. Birdsong.mp3",
            "Soundscapes/Forest/6. Bees.mp3"
        ]
    },
    {
        id: "field",
        title: "Summer Field",
        note: "Grass, cicadas, and evening insects.",
        stemPath: "Soundscapes/Stems/field-stem.mp3",
        stemGainCompensation: 6,
        stemDurationSeconds: bankStemLoopSeconds,
        tracks: [
            "Soundscapes/Field/1. Grass Rustle.mp3",
            "Soundscapes/Field/2. Summer Wind.mp3",
            "Soundscapes/Field/3. Crickets 1.mp3",
            "Soundscapes/Field/4. Crickets 2.mp3",
            "Soundscapes/Field/5. Cicadas.mp3",
            "Soundscapes/Field/6. Owl.mp3"
        ]
    }
];

const fallbackSoloDefinitions = [
    { id: "light-stream", title: "Light Stream", path: "1. Light Stream.mp3", note: "Bright water texture." },
    { id: "river", title: "River", path: "2. Stream.mp3", note: "A broader steady flow." },
    { id: "waterfall", title: "Waterfall", path: "3. Waterfall.mp3", note: "Dense rushing water." },
    { id: "campfire", title: "Campfire", path: "4. Campfire.mp3", note: "Low crackle and warmth." },
    { id: "cat-purr", title: "Cat Purr", path: "5. Cat Purr.mp3", note: "Soft, close comfort." },
    { id: "crows", title: "Crows", path: "6. Crows.mp3", note: "Sparse outdoor movement." }
];

let bankDefinitions = [];
let soloDefinitions = [];

const dom = {
    wakeButtons: Array.from(document.querySelectorAll("[data-role='wake-audio']")),
    statusText: document.querySelector("#status-text"),
    stopButtons: Array.from(document.querySelectorAll("[data-role='stop-all']")),
    resetAllSlidersButton: document.querySelector("#reset-all-sliders"),
    masterBackground: document.querySelector("#master-background"),
    masterBackgroundReadout: document.querySelector("#master-background-readout"),
    masterSolo: document.querySelector("#master-solo"),
    masterSoloReadout: document.querySelector("#master-solo-readout"),
    bankGrid: document.querySelector("#bank-grid"),
    soloGrid: document.querySelector("#solo-grid"),
    bankPrev: document.querySelector("#bank-prev"),
    bankNext: document.querySelector("#bank-next"),
    soloPrev: document.querySelector("#solo-prev"),
    soloNext: document.querySelector("#solo-next"),
    bankCount: document.querySelector("#bank-count"),
    soloCount: document.querySelector("#solo-count"),
    layerPanel: document.querySelector("#layer-panel"),
    layerPanelTitle: document.querySelector("#layer-panel-title"),
    layerPanelStatus: document.querySelector("#layer-panel-status"),
    layerPanelCopy: document.querySelector("#layer-panel-copy"),
    layerPanelVolume: document.querySelector("#layer-panel-volume"),
    layerPanelVolumeReadout: document.querySelector("#layer-panel-volume-readout"),
    layerPanelReset: document.querySelector("#layer-panel-reset"),
    layerPanelMixer: document.querySelector("#layer-panel-mixer"),
    soloPanel: document.querySelector("#solo-panel"),
    soloPanelTitle: document.querySelector("#solo-panel-title"),
    soloPanelStatus: document.querySelector("#solo-panel-status"),
    soloPanelCopy: document.querySelector("#solo-panel-copy"),
    soloPanelVolume: document.querySelector("#solo-panel-volume"),
    soloPanelVolumeReadout: document.querySelector("#solo-panel-volume-readout"),
    soloPanelPan: document.querySelector("#solo-panel-pan"),
    soloPanelPanReadout: document.querySelector("#solo-panel-pan-readout"),
    presetButtons: Array.from(document.querySelectorAll("[data-role='mix-preset']")),
    presetName: document.querySelector("#preset-name"),
    savePresetButton: document.querySelector("#save-preset"),
    customPresetList: document.querySelector("#custom-preset-list"),
    savedPresetCount: document.querySelector("#saved-preset-count"),
    jumpSoundscapes: document.querySelector("#jump-soundscapes"),
    jumpSolos: document.querySelector("#jump-solos"),
    soundscapeSection: document.querySelector("#soundscape-section"),
    soloSection: document.querySelector("#solo-section")
};

let state = null;
let savedPresets = [];
let audioContext = null;
let backgroundBus = null;
let soloBus = null;
let compressor = null;
let masterBus = null;
let selectedBankIndex = 0;
let selectedSoloIndex = 0;
const carouselSwipeThreshold = 28;

const trackEngines = new Map();
const bankPlaybackStates = [];

initialize().catch((error) => {
    console.error(error);
    dom.statusText.textContent = "Sound library failed to load.";
});

async function initialize() {
    document.body.classList.toggle("wrapper-mode", isAndroidWrapper);

    const definitions = await loadLibraryDefinitions();
    bankDefinitions = definitions.beds;
    soloDefinitions = definitions.solos;
    bankPlaybackStates.splice(0, bankPlaybackStates.length, ...bankDefinitions.map(() => ({
        activeMode: "none",
        timelineSeconds: 0,
        timelineContextTime: 0
    })));

    state = loadState();
    savedPresets = loadSavedPresets();
    ensureValidSelections();

    dom.masterBackground.value = String(state.masterBackground);
    dom.masterSolo.value = String(state.masterSolo);

    renderBankCards();
    renderSoloCards();
    renderSavedPresets();
    updateMasterReadouts();
    updateStatus();

    dom.masterBackground.addEventListener("input", () => {
        setMasterMix(Number(dom.masterBackground.value), state.masterSolo);
    });

    dom.masterSolo.addEventListener("input", () => {
        setMasterMix(state.masterBackground, Number(dom.masterSolo.value));
    });

    dom.wakeButtons.forEach((button) => {
        button.addEventListener("click", async () => {
            await ensureAudioReady();
        });
    });

    dom.stopButtons.forEach((button) => {
        button.addEventListener("click", () => {
            stopAllPlayback();
        });
    });

    dom.resetAllSlidersButton.addEventListener("click", () => {
        resetAllSliders();
    });

    dom.presetButtons.forEach((button) => {
        button.addEventListener("click", () => {
            applyMixPreset(button);
        });
    });

    dom.presetName.addEventListener("input", () => {
        updateSavePresetAvailability();
    });

    dom.presetName.addEventListener("keydown", (event) => {
        if (event.key !== "Enter") {
            return;
        }

        event.preventDefault();
        saveCurrentPreset();
    });

    dom.savePresetButton.addEventListener("click", () => {
        saveCurrentPreset();
    });

    dom.jumpSoundscapes.addEventListener("click", () => {
        dom.soundscapeSection?.scrollIntoView({ behavior: "smooth", block: "start" });
    });

    dom.jumpSolos.addEventListener("click", () => {
        dom.soloSection?.scrollIntoView({ behavior: "smooth", block: "start" });
    });

    attachCarouselGestures(dom.bankGrid,
        () => setSelectedBank(selectedBankIndex - 1),
        () => setSelectedBank(selectedBankIndex + 1));

    attachCarouselGestures(dom.soloGrid,
        () => setSelectedSolo(selectedSoloIndex - 1),
        () => setSelectedSolo(selectedSoloIndex + 1));

    dom.bankPrev?.addEventListener("click", () => {
        setSelectedBank(selectedBankIndex - 1, { scrollCard: true });
    });

    dom.bankNext?.addEventListener("click", () => {
        setSelectedBank(selectedBankIndex + 1, { scrollCard: true });
    });

    dom.layerPanelVolume?.addEventListener("input", () => {
        if (selectedBankIndex < 0) {
            return;
        }

        state.banks[selectedBankIndex].volume = Number(dom.layerPanelVolume.value);
        persistState();
        updateAllGains();
        refreshCardStates();
    });

    dom.layerPanelReset?.addEventListener("click", () => {
        if (selectedBankIndex < 0) {
            return;
        }

        resetBankSliders(selectedBankIndex);
    });

    dom.soloPrev?.addEventListener("click", () => {
        setSelectedSolo(selectedSoloIndex - 1, { scrollCard: true });
    });

    dom.soloNext?.addEventListener("click", () => {
        setSelectedSolo(selectedSoloIndex + 1, { scrollCard: true });
    });

    dom.soloPanelVolume?.addEventListener("input", () => {
        if (selectedSoloIndex < 0) {
            return;
        }

        state.solos[selectedSoloIndex].volume = Number(dom.soloPanelVolume.value);
        persistState();
        updateAllGains();
        refreshCardStates();
    });

    dom.soloPanelPan?.addEventListener("input", () => {
        if (selectedSoloIndex < 0) {
            return;
        }

        state.solos[selectedSoloIndex].pan = Number(dom.soloPanelPan.value);
        persistState();
        updateAllGains();
        refreshCardStates();
    });

    syncWakeButtonState();
    updatePresetStates();
    updateSavePresetAvailability();
}

async function loadLibraryDefinitions() {
    try {
        const response = await fetch(manifestPath, { cache: "no-store" });
        if (!response.ok) {
            throw new Error(`Manifest request failed: ${response.status}`);
        }

        const manifest = await response.json();
        const beds = Array.isArray(manifest?.beds)
            ? manifest.beds.map(mapManifestBedToBankDefinition).filter((bank) => bank.tracks.length > 0)
            : [];
        const solos = Array.isArray(manifest?.soloLoops)
            ? manifest.soloLoops.map(mapManifestSoloToDefinition).filter((solo) => Boolean(solo.path))
            : [];

        if (beds.length === 0 || solos.length === 0) {
            throw new Error("Manifest did not contain usable bed and solo definitions.");
        }

        return { beds, solos };
    } catch (error) {
        console.warn("Using fallback library definitions.", error);
        return {
            beds: fallbackBankDefinitions.map((bank) => ({
                ...bank,
                trackLabels: bank.tracks.map((path) => getTrackLabel(path))
            })),
            solos: [...fallbackSoloDefinitions]
        };
    }
}

function mapManifestBedToBankDefinition(bed) {
    const customLayers = Array.isArray(bed?.delivery?.customLayers) ? bed.delivery.customLayers : [];
    return {
        id: typeof bed?.id === "string" && bed.id ? bed.id : `bed-${Math.random().toString(36).slice(2, 8)}`,
        title: typeof bed?.displayName === "string" && bed.displayName ? bed.displayName : "Untitled Bed",
        note: typeof bed?.description === "string" ? bed.description : "",
        stemPath: typeof bed?.delivery?.stem?.path === "string" ? bed.delivery.stem.path : "",
        stemGainCompensation: typeof bed?.delivery?.stem?.stemGainCompensation === "number"
            ? bed.delivery.stem.stemGainCompensation
            : customLayers.length,
        stemDurationSeconds: typeof bed?.delivery?.stem?.loopDurationSeconds === "number"
            ? bed.delivery.stem.loopDurationSeconds
            : bankStemLoopSeconds,
        tracks: customLayers
            .map((layer) => layer?.path)
            .filter((path) => typeof path === "string" && path),
        trackLabels: customLayers.map((layer) => {
            if (typeof layer?.displayName === "string" && layer.displayName) {
                return layer.displayName;
            }

            return getTrackLabel(layer?.path || "Layer");
        })
    };
}

function mapManifestSoloToDefinition(solo) {
    return {
        id: typeof solo?.id === "string" && solo.id ? solo.id : `solo-${Math.random().toString(36).slice(2, 8)}`,
        title: typeof solo?.displayName === "string" && solo.displayName ? solo.displayName : "Untitled Solo",
        path: typeof solo?.deliveryPath === "string" ? solo.deliveryPath : "",
        note: typeof solo?.description === "string"
            ? solo.description
            : (typeof solo?.notes === "string" ? solo.notes : "")
    };
}

function setMasterMix(backgroundValue, soloValue) {
    state.masterBackground = clampNumber(backgroundValue, state.masterBackground, 0, 100);
    state.masterSolo = clampNumber(soloValue, state.masterSolo, 0, 100);
    dom.masterBackground.value = String(state.masterBackground);
    dom.masterSolo.value = String(state.masterSolo);
    persistState();
    updateMasterReadouts();
    updatePresetStates();
    updateAllGains();
}

function applyMixPreset(button) {
    const background = Number(button.dataset.background);
    const solo = Number(button.dataset.solo);
    setMasterMix(background, solo);
}

function createStateSnapshot(source = state) {
    return {
        masterBackground: source.masterBackground,
        masterSolo: source.masterSolo,
        banks: source.banks.map((bank) => ({
            active: bank.active,
            volume: bank.volume,
            layerVolumes: [...bank.layerVolumes],
            mixMode: bank.mixMode,
            mixPriority: bank.mixPriority
        })),
        solos: source.solos.map((solo) => ({
            active: solo.active,
            volume: solo.volume,
            pan: solo.pan
        }))
    };
}

function isDefaultLayerVolume(value) {
    return Math.abs(value - defaultBackgroundLayerVolume) <= layerMixDeadband;
}

function areLayerVolumesDefault(layerVolumes) {
    return layerVolumes.every((value) => isDefaultLayerVolume(value));
}

function getDesiredBankMixMode(bankState) {
    return bankState.mixMode === "custom" ? "custom" : "stem";
}

function getNextMixPriority() {
    mixPriorityCounter += 1;
    return mixPriorityCounter;
}

function normalizeState(source, fallback = createDefaultState()) {
    return {
        masterBackground: clampNumber(source?.masterBackground, fallback.masterBackground, 0, 100),
        masterSolo: clampNumber(source?.masterSolo, fallback.masterSolo, 0, 100),
        banks: bankDefinitions.map((bank, index) => {
            const layerVolumes = bank.tracks.map((_, layerIndex) => clampNumber(
                source?.banks?.[index]?.layerVolumes?.[layerIndex],
                fallback.banks[index].layerVolumes[layerIndex],
                0,
                100
            ));
            const desiredMixMode = source?.banks?.[index]?.mixMode === "custom" || !areLayerVolumesDefault(layerVolumes)
                ? "custom"
                : "stem";

            const mixPriority = desiredMixMode === "custom"
                ? clampNumber(source?.banks?.[index]?.mixPriority, fallback.banks[index].mixPriority, 0, Number.MAX_SAFE_INTEGER)
                : 0;

            mixPriorityCounter = Math.max(mixPriorityCounter, mixPriority || 0);

            return {
                active: Boolean(source?.banks?.[index]?.active),
                volume: clampNumber(source?.banks?.[index]?.volume, fallback.banks[index].volume, 0, 100),
                layerVolumes,
                mixMode: desiredMixMode,
                mixPriority
            };
        }),
        solos: soloDefinitions.map((_, index) => ({
            active: Boolean(source?.solos?.[index]?.active),
            volume: clampNumber(source?.solos?.[index]?.volume, fallback.solos[index].volume, 0, 100),
            pan: clampNumber(source?.solos?.[index]?.pan, fallback.solos[index].pan, -100, 100)
        }))
    };
}

function applyState(nextState) {
    state = normalizeState(nextState);
    persistState();
    dom.masterBackground.value = String(state.masterBackground);
    dom.masterSolo.value = String(state.masterSolo);
    updateMasterReadouts();
    updatePresetStates();
    refreshCardStates();
    syncPlayback();
}

function getPresetNameInputValue() {
    return dom.presetName.value.trim().replace(/\s+/g, " ").slice(0, 32);
}

function updateSavePresetAvailability() {
    dom.savePresetButton.disabled = getPresetNameInputValue().length === 0;
}

function createPresetId() {
    if (window.crypto?.randomUUID) {
        return window.crypto.randomUUID();
    }

    return `preset-${Date.now()}-${Math.random().toString(36).slice(2, 8)}`;
}

function normalizeSavedPreset(entry) {
    const name = typeof entry?.name === "string" ? entry.name.trim().replace(/\s+/g, " ").slice(0, 32) : "";
    if (!name) {
        return null;
    }

    return {
        id: typeof entry.id === "string" && entry.id ? entry.id : createPresetId(),
        name,
        updatedAt: typeof entry.updatedAt === "number" ? entry.updatedAt : Date.now(),
        state: normalizeState(entry.state)
    };
}

function loadSavedPresets() {
    try {
        const raw = window.localStorage.getItem(presetStorageKey);
        if (!raw) {
            return [];
        }

        const parsed = JSON.parse(raw);
        if (!Array.isArray(parsed)) {
            return [];
        }

        return parsed
            .map((entry) => normalizeSavedPreset(entry))
            .filter(Boolean)
            .sort((left, right) => right.updatedAt - left.updatedAt)
            .slice(0, maxSavedPresets);
    } catch (_error) {
        return [];
    }
}

function persistSavedPresets() {
    window.localStorage.setItem(presetStorageKey, JSON.stringify(savedPresets));
}

function summarizePreset(savedPreset) {
    const activeBanks = savedPreset.state.banks.filter((bank) => bank.active).length;
    const activeSolos = savedPreset.state.solos.filter((solo) => solo.active).length;
    return `${activeBanks} bed${activeBanks === 1 ? "" : "s"}, ${activeSolos} loop${activeSolos === 1 ? "" : "s"} | Mix ${savedPreset.state.masterBackground}/${savedPreset.state.masterSolo}`;
}

function renderSavedPresets() {
    dom.savedPresetCount.textContent = `${savedPresets.length} saved`;

    if (savedPresets.length === 0) {
        dom.customPresetList.innerHTML = `
            <div class="saved-preset-empty">
                Save the current scene to keep your bank toggles, layer blend, solo levels, pan, and master mix together.
            </div>
        `;
        return;
    }

    dom.customPresetList.innerHTML = savedPresets.map((savedPreset) => `
        <article class="saved-preset-card" data-role="saved-preset-card" data-preset-id="${savedPreset.id}">
            <div class="saved-preset-copy">
                <strong>${escapeHtml(savedPreset.name)}</strong>
                <p>${escapeHtml(summarizePreset(savedPreset))}</p>
            </div>
            <div class="saved-preset-actions">
                <button class="preset-chip" type="button" data-role="custom-preset-load" data-preset-id="${savedPreset.id}">Load</button>
                <button class="preset-chip saved-preset-delete" type="button" data-role="custom-preset-delete" data-preset-id="${savedPreset.id}">Delete</button>
            </div>
        </article>
    `).join("");

    dom.customPresetList.querySelectorAll("[data-role='custom-preset-load']").forEach((button) => {
        button.addEventListener("click", () => {
            loadSavedPreset(button.dataset.presetId);
        });
    });

    dom.customPresetList.querySelectorAll("[data-role='custom-preset-delete']").forEach((button) => {
        button.addEventListener("click", () => {
            deleteSavedPreset(button.dataset.presetId);
        });
    });
}

function saveCurrentPreset() {
    const name = getPresetNameInputValue();
    if (!name) {
        updateSavePresetAvailability();
        return;
    }

    const existingIndex = savedPresets.findIndex((savedPreset) => savedPreset.name.toLowerCase() === name.toLowerCase());
    const nextPreset = {
        id: existingIndex >= 0 ? savedPresets[existingIndex].id : createPresetId(),
        name,
        updatedAt: Date.now(),
        state: createStateSnapshot()
    };

    if (existingIndex >= 0) {
        savedPresets.splice(existingIndex, 1);
    }

    savedPresets.unshift(nextPreset);
    savedPresets = savedPresets.slice(0, maxSavedPresets);
    persistSavedPresets();
    renderSavedPresets();

    dom.presetName.value = "";
    updateSavePresetAvailability();
    dom.statusText.textContent = `Saved preset: ${name}`;
}

function loadSavedPreset(presetId) {
    const savedPreset = savedPresets.find((entry) => entry.id === presetId);
    if (!savedPreset) {
        return;
    }

    applyState(savedPreset.state);
    dom.statusText.textContent = audioContext && audioContext.state === "running"
        ? `Loaded preset: ${savedPreset.name}`
        : `Loaded preset: ${savedPreset.name}. Tap Wake Audio to hear it.`;
}

function deleteSavedPreset(presetId) {
    const presetIndex = savedPresets.findIndex((entry) => entry.id === presetId);
    if (presetIndex < 0) {
        return;
    }

    const [{ name }] = savedPresets.splice(presetIndex, 1);
    persistSavedPresets();
    renderSavedPresets();
    dom.statusText.textContent = `Deleted preset: ${name}`;
}

function stopAllPlayback() {
    state.banks.forEach((bank) => {
        bank.active = false;
    });
    state.solos.forEach((solo) => {
        solo.active = false;
    });
    persistState();
    syncPlayback();
    refreshCardStates();
    updateStatus();
}

function syncWakeButtonState() {
    const wakeLabel = document.body.classList.contains("audio-live") ? "Audio Live" : "Wake Audio";
    dom.wakeButtons.forEach((button) => {
        button.textContent = button.classList.contains("dock-button")
            ? (document.body.classList.contains("audio-live") ? "Live" : "Wake")
            : wakeLabel;
    });
}

function updatePresetStates() {
    dom.presetButtons.forEach((button) => {
        const matches = Number(button.dataset.background) === state.masterBackground
            && Number(button.dataset.solo) === state.masterSolo;
        button.classList.toggle("is-selected", matches);
    });
}

function createDefaultState() {
    return {
        masterBackground: 72,
        masterSolo: 65,
        banks: bankDefinitions.map((bank) => ({
            active: false,
            volume: 70,
            layerVolumes: bank.tracks.map(() => defaultBackgroundLayerVolume),
            mixMode: "stem",
            mixPriority: 0
        })),
        solos: soloDefinitions.map(() => ({ active: false, volume: 62, pan: 0 }))
    };
}

function createSliderResetState(source = state) {
    const defaults = createDefaultState();

    return {
        masterBackground: defaults.masterBackground,
        masterSolo: defaults.masterSolo,
        banks: source.banks.map((bank, index) => ({
            ...bank,
            volume: defaults.banks[index].volume,
            layerVolumes: [...defaults.banks[index].layerVolumes],
            mixMode: defaults.banks[index].mixMode,
            mixPriority: defaults.banks[index].mixPriority
        })),
        solos: source.solos.map((solo, index) => ({
            ...solo,
            volume: defaults.solos[index].volume,
            pan: defaults.solos[index].pan
        }))
    };
}

function loadState() {
    const fallback = createDefaultState();

    try {
        const raw = window.localStorage.getItem(storageKey);
        if (!raw) {
            return fallback;
        }

        const parsed = JSON.parse(raw);
        return normalizeState(parsed, fallback);
    } catch (_error) {
        return fallback;
    }
}

function persistState() {
    window.localStorage.setItem(storageKey, JSON.stringify(state));
}

function clampNumber(value, fallback, min, max) {
    if (typeof value !== "number" || Number.isNaN(value)) {
        return fallback;
    }

    return Math.min(max, Math.max(min, value));
}

function getTrackLabel(path) {
    const fileName = path.split("/").pop() || path;
    return fileName
        .replace(/\.mp3$/i, "")
        .replace(/^\d+\.\s*/, "");
}

function getStemEngineKey(bankIndex) {
    return `stem-${bankDefinitions[bankIndex].id}`;
}

function normalizeLoopPosition(seconds, durationSeconds) {
    if (!Number.isFinite(durationSeconds) || durationSeconds <= 0) {
        return 0;
    }

    const normalized = seconds % durationSeconds;
    return normalized < 0 ? normalized + durationSeconds : normalized;
}

function getBankTimelinePosition(bankIndex) {
    const playbackState = bankPlaybackStates[bankIndex];
    if (!audioContext || playbackState.activeMode === "none") {
        return playbackState.timelineSeconds;
    }

    return normalizeLoopPosition(
        playbackState.timelineSeconds + (audioContext.currentTime - playbackState.timelineContextTime),
        bankDefinitions[bankIndex].stemDurationSeconds || bankStemLoopSeconds
    );
}

function setBankTimelinePosition(bankIndex, seconds, mode) {
    const playbackState = bankPlaybackStates[bankIndex];
    playbackState.timelineSeconds = normalizeLoopPosition(
        seconds,
        bankDefinitions[bankIndex].stemDurationSeconds || bankStemLoopSeconds
    );
    playbackState.timelineContextTime = audioContext ? audioContext.currentTime : 0;
    playbackState.activeMode = mode;
}

function getBankStemLevel(bankIndex) {
    const bankLevel = (state.masterBackground / 100) * (state.banks[bankIndex].volume / 100) * backgroundLayerBaseGain;
    return bankLevel * (bankDefinitions[bankIndex].stemGainCompensation || bankDefinitions[bankIndex].tracks.length);
}

function getBankLayerLevel(bankIndex, trackIndex) {
    const bankLevel = (state.masterBackground / 100) * (state.banks[bankIndex].volume / 100) * backgroundLayerBaseGain;
    return bankLevel * (state.banks[bankIndex].layerVolumes[trackIndex] / 100);
}

function bankHasStem(bankIndex) {
    return Boolean(bankDefinitions[bankIndex].stemPath);
}

function computePlaybackPlan(nextState = state) {
    const bankModes = nextState.banks.map((bank, index) => {
        if (!bank.active) {
            return "stem";
        }

        return bankHasStem(index) ? "stem" : "custom";
    });

    let totalUnits = nextState.solos.reduce((sum, solo) => sum + (solo.active ? soloStreamUnits : 0), 0);

    nextState.banks.forEach((bank, index) => {
        if (!bank.active) {
            return;
        }

        totalUnits += bankHasStem(index) ? stemStreamUnits : bankDefinitions[index].tracks.length;
    });

    nextState.banks
        .map((bank, index) => ({ bank, index }))
        .filter(({ bank, index }) => bank.active && getDesiredBankMixMode(bank) === "custom" && bankHasStem(index))
        .sort((left, right) => (right.bank.mixPriority || 0) - (left.bank.mixPriority || 0))
        .forEach(({ index }) => {
            const extraUnits = bankDefinitions[index].tracks.length - stemStreamUnits;
            if (totalUnits + extraUnits <= maxActiveStreamUnits) {
                bankModes[index] = "custom";
                totalUnits += extraUnits;
            }
        });

    return { bankModes, totalUnits };
}

function canApplyPlaybackState(nextState, rejectionMessage) {
    const playbackPlan = computePlaybackPlan(nextState);
    if (playbackPlan.totalUnits <= maxActiveStreamUnits) {
        return true;
    }

    dom.statusText.textContent = `${rejectionMessage} ${playbackPlan.totalUnits}/${maxActiveStreamUnits} stream units would be active.`;
    return false;
}

function createProjectedStateForBank(bankIndex, nextBankState) {
    return {
        ...state,
        banks: state.banks.map((bank, index) => (index === bankIndex ? nextBankState : bank))
    };
}

function createProjectedStateForSolo(soloIndex, nextSoloState) {
    return {
        ...state,
        solos: state.solos.map((solo, index) => (index === soloIndex ? nextSoloState : solo))
    };
}

function getBankSummaryLabel(bankState, resolvedMode) {
    const desiredMode = getDesiredBankMixMode(bankState);
    if (!bankState.active) {
        return desiredMode === "custom" ? "Custom blend saved" : "Stem blend";
    }

    if (desiredMode === "custom" && resolvedMode === "custom") {
        return "Custom layer blend";
    }

    if (desiredMode === "custom") {
        return "Custom saved | stem live";
    }

    return "Stem blend";
}

function ensureValidSelections() {
    selectedBankIndex = bankDefinitions.length > 0
        ? Math.min(Math.max(selectedBankIndex, 0), bankDefinitions.length - 1)
        : -1;
    selectedSoloIndex = soloDefinitions.length > 0
        ? Math.min(Math.max(selectedSoloIndex, 0), soloDefinitions.length - 1)
        : -1;
}

function wrapIndex(index, length) {
    if (length <= 0) {
        return -1;
    }

    return ((index % length) + length) % length;
}

function attachCarouselGestures(track, onPrev, onNext) {
    if (!track) {
        return;
    }

    const pointerState = {
        active: false,
        pointerId: -1,
        startX: 0,
        startY: 0
    };

    const resetPointerState = () => {
        pointerState.active = false;
        pointerState.pointerId = -1;
        pointerState.startX = 0;
        pointerState.startY = 0;
    };

    track.addEventListener("pointerdown", (event) => {
        if (event.pointerType === "mouse" && event.button !== 0) {
            return;
        }

        if (event.target.closest("button, input, label")) {
            return;
        }

        pointerState.active = true;
        pointerState.pointerId = event.pointerId;
        pointerState.startX = event.clientX;
        pointerState.startY = event.clientY;
        try {
            track.setPointerCapture?.(event.pointerId);
        } catch (error) {
            void error;
        }
    });

    track.addEventListener("pointerup", (event) => {
        if (!pointerState.active || event.pointerId !== pointerState.pointerId) {
            return;
        }

        const deltaX = event.clientX - pointerState.startX;
        const deltaY = event.clientY - pointerState.startY;
        try {
            track.releasePointerCapture?.(event.pointerId);
        } catch (error) {
            void error;
        }
        resetPointerState();

        if (Math.abs(deltaX) < carouselSwipeThreshold || Math.abs(deltaX) <= Math.abs(deltaY)) {
            return;
        }

        if (deltaX < 0) {
            onNext();
            return;
        }

        onPrev();
    });

    track.addEventListener("pointercancel", () => {
        resetPointerState();
    });
}

function setSelectedBank(index, { scrollPanel = false, scrollCard = false } = {}) {
    if (bankDefinitions.length === 0) {
        return;
    }

    selectedBankIndex = wrapIndex(index, bankDefinitions.length);
    refreshCardStates();

    if (scrollPanel && dom.layerPanel) {
        dom.layerPanel.scrollIntoView({ behavior: "smooth", block: "nearest" });
    }
}

function setSelectedSolo(index, { scrollCard = false, scrollPanel = false } = {}) {
    if (soloDefinitions.length === 0) {
        return;
    }

    selectedSoloIndex = wrapIndex(index, soloDefinitions.length);
    refreshCardStates();

    if (scrollPanel && dom.soloPanel) {
        dom.soloPanel.scrollIntoView({ behavior: "smooth", block: "nearest" });
    }
}

function renderSelectedBankPanel(playbackPlan = computePlaybackPlan()) {
    ensureValidSelections();

    if (!dom.layerPanel || !dom.layerPanelMixer || selectedBankIndex < 0) {
        return;
    }

    const bank = bankDefinitions[selectedBankIndex];
    const bankState = state.banks[selectedBankIndex];
    const resolvedMode = playbackPlan.bankModes[selectedBankIndex];
    const layerCountLabel = `${bank.tracks.length} layer${bank.tracks.length === 1 ? "" : "s"}`;

    dom.layerPanelTitle.textContent = bank.title;
    dom.layerPanelStatus.textContent = bankState.active ? "Live" : "Standby";
    dom.layerPanelStatus.classList.toggle("is-live", bankState.active);
    dom.layerPanelCopy.textContent = `${bank.note} ${layerCountLabel}. ${getBankSummaryLabel(bankState, resolvedMode)}.`;
    dom.layerPanelVolume.value = String(bankState.volume);
    dom.layerPanelVolumeReadout.textContent = `${bankState.volume}%`;
    dom.layerPanelMixer.innerHTML = bank.tracks.map((path, layerIndex) => `
        <label class="layer-row">
            <div class="layer-row-head">
                <span class="layer-name">${bank.trackLabels?.[layerIndex] || getTrackLabel(path)}</span>
                <strong class="layer-readout" data-role="bank-layer-readout" data-index="${selectedBankIndex}" data-layer-index="${layerIndex}">100%</strong>
            </div>
            <input type="range" min="0" max="100" step="1" data-role="bank-layer-volume" data-index="${selectedBankIndex}" data-layer-index="${layerIndex}">
        </label>
    `).join("");

    dom.layerPanelMixer.querySelectorAll("[data-role='bank-layer-volume']").forEach((slider) => {
        const layerIndex = Number(slider.dataset.layerIndex);
        const layerVolume = bankState.layerVolumes[layerIndex];
        const layerReadout = dom.layerPanelMixer.querySelector(`[data-role='bank-layer-readout'][data-layer-index='${layerIndex}']`);
        slider.value = String(layerVolume);
        layerReadout.textContent = `${layerVolume}%`;

        slider.addEventListener("input", () => {
            const nextLayerVolumes = [...state.banks[selectedBankIndex].layerVolumes];
            nextLayerVolumes[layerIndex] = Number(slider.value);

            const nextMixMode = areLayerVolumesDefault(nextLayerVolumes) ? "stem" : "custom";
            state.banks[selectedBankIndex] = {
                ...state.banks[selectedBankIndex],
                layerVolumes: nextLayerVolumes,
                mixMode: nextMixMode,
                mixPriority: nextMixMode === "custom" ? getNextMixPriority() : 0
            };

            persistState();
            const nextPlaybackPlan = computePlaybackPlan();
            if (state.banks[selectedBankIndex].active && nextMixMode === "custom" && nextPlaybackPlan.bankModes[selectedBankIndex] !== "custom") {
                dom.statusText.textContent = "Custom blend saved, but stem mode is live until some streams are turned off.";
            }

            syncPlayback();
            refreshCardStates();
        });
    });
}

function renderSelectedSoloPanel() {
    ensureValidSelections();

    if (!dom.soloPanel || selectedSoloIndex < 0) {
        return;
    }

    const solo = soloDefinitions[selectedSoloIndex];
    const soloState = state.solos[selectedSoloIndex];

    dom.soloPanelTitle.textContent = solo.title;
    dom.soloPanelStatus.textContent = soloState.active ? "Live" : "Standby";
    dom.soloPanelStatus.classList.toggle("is-live", soloState.active);
    dom.soloPanelCopy.textContent = `${solo.note} Level ${soloState.volume}% | ${formatPan(soloState.pan)}.`;
    dom.soloPanelVolume.value = String(soloState.volume);
    dom.soloPanelVolumeReadout.textContent = `${soloState.volume}%`;
    dom.soloPanelPan.value = String(soloState.pan);
    dom.soloPanelPanReadout.textContent = formatPan(soloState.pan);
}

function resetBankSliders(bankIndex) {
    const defaults = createDefaultState().banks[bankIndex];
    const nextBankState = {
        ...state.banks[bankIndex],
        volume: defaults.volume,
        layerVolumes: [...defaults.layerVolumes],
        mixMode: defaults.mixMode,
        mixPriority: defaults.mixPriority
    };

    applyState(createProjectedStateForBank(bankIndex, nextBankState));
    dom.statusText.textContent = `${bankDefinitions[bankIndex].title} sliders reset to defaults.`;
}

function resetAllSliders() {
    applyState(createSliderResetState());
    dom.statusText.textContent = "All sliders reset to default values.";
}

function renderBankCards() {
    dom.bankGrid.innerHTML = "";
    ensureValidSelections();

    bankDefinitions.forEach((bank, index) => {
        const article = document.createElement("article");
        article.className = "sound-card carousel-card";
        article.dataset.role = "bank-card";
        article.dataset.index = String(index);

        article.innerHTML = `
            <div class="sound-card-header">
                <button class="pill-button sound-toggle" type="button" data-role="bank-toggle" data-index="${index}" data-state-label="Standby">${bank.title}</button>
                <p class="card-note">${bank.note}</p>
            </div>
        `;

        dom.bankGrid.append(article);
    });

    dom.bankGrid.querySelectorAll("[data-role='bank-card']").forEach((card) => {
        card.addEventListener("click", (event) => {
            if (event.target.closest("button, input, label")) {
                return;
            }

            setSelectedBank(Number(card.dataset.index), { scrollCard: true });
        });
    });

    dom.bankGrid.querySelectorAll("[data-role='bank-toggle']").forEach((button) => {
        button.addEventListener("click", async () => {
            const index = Number(button.dataset.index);
            setSelectedBank(index, { scrollCard: true });
            await ensureAudioReady();
            const nextBankState = {
                ...state.banks[index],
                active: !state.banks[index].active
            };

            const projectedState = createProjectedStateForBank(index, nextBankState);
            if (nextBankState.active && !canApplyPlaybackState(projectedState, "This phone is already at its stream budget.")) {
                return;
            }

            state.banks[index] = nextBankState;
            persistState();
            syncPlayback();
            refreshCardStates();
            updateStatus();
        });
    });

    refreshCardStates();
}

function renderSoloCards() {
    dom.soloGrid.innerHTML = "";
    ensureValidSelections();

    soloDefinitions.forEach((solo, index) => {
        const article = document.createElement("article");
        article.className = "sound-card carousel-card";
        article.dataset.role = "solo-card";
        article.dataset.index = String(index);

        article.innerHTML = `
            <div class="sound-card-header">
                <button class="pill-button sound-toggle" type="button" data-role="solo-toggle" data-index="${index}" data-state-label="Standby">${solo.title}</button>
                <p class="card-note">${solo.note}</p>
            </div>
        `;

        dom.soloGrid.append(article);
    });

    dom.soloGrid.querySelectorAll("[data-role='solo-card']").forEach((card) => {
        card.addEventListener("click", (event) => {
            if (event.target.closest("button, input, label")) {
                return;
            }

            setSelectedSolo(Number(card.dataset.index), { scrollCard: true });
        });
    });

    dom.soloGrid.querySelectorAll("[data-role='solo-toggle']").forEach((button) => {
        button.addEventListener("click", async () => {
            const index = Number(button.dataset.index);
            setSelectedSolo(index, { scrollCard: true });
            await ensureAudioReady();
            const nextSoloState = {
                ...state.solos[index],
                active: !state.solos[index].active
            };

            const projectedState = createProjectedStateForSolo(index, nextSoloState);
            if (nextSoloState.active && !canApplyPlaybackState(projectedState, "This phone is already at its stream budget.")) {
                return;
            }

            state.solos[index] = nextSoloState;
            persistState();
            syncPlayback();
            refreshCardStates();
            updateStatus();
        });
    });

    refreshCardStates();
}

function refreshCardStates() {
    const playbackPlan = computePlaybackPlan();
    ensureValidSelections();
    const previousBankIndex = wrapIndex(selectedBankIndex - 1, bankDefinitions.length);
    const nextBankIndex = wrapIndex(selectedBankIndex + 1, bankDefinitions.length);
    const previousSoloIndex = wrapIndex(selectedSoloIndex - 1, soloDefinitions.length);
    const nextSoloIndex = wrapIndex(selectedSoloIndex + 1, soloDefinitions.length);

    document.querySelectorAll("[data-role='bank-card']").forEach((card) => {
        const index = Number(card.dataset.index);
        const config = state.banks[index];
        const toggle = card.querySelector("[data-role='bank-toggle']");
        const selected = index === selectedBankIndex;
        const previous = !selected && index === previousBankIndex;
        const next = !selected && index === nextBankIndex;

        card.classList.toggle("is-active", config.active);
        card.classList.toggle("is-selected", selected);
        card.classList.toggle("is-prev", previous);
        card.classList.toggle("is-next", next);
        toggle.classList.toggle("is-active", config.active);
        toggle.dataset.stateLabel = config.active ? "Live" : "Standby";
    });

    renderSelectedBankPanel(playbackPlan);

    document.querySelectorAll("[data-role='solo-card']").forEach((card) => {
        const index = Number(card.dataset.index);
        const config = state.solos[index];
        const toggle = card.querySelector("[data-role='solo-toggle']");
        const selected = index === selectedSoloIndex;
        const previous = !selected && index === previousSoloIndex;
        const next = !selected && index === nextSoloIndex;

        card.classList.toggle("is-active", config.active);
        card.classList.toggle("is-selected", selected);
        card.classList.toggle("is-prev", previous);
        card.classList.toggle("is-next", next);
        toggle.classList.toggle("is-active", config.active);
        toggle.dataset.stateLabel = config.active ? "Live" : "Standby";
    });

    renderSelectedSoloPanel();

    updateCounts();
}

function updateCounts() {
    const activeBanks = state.banks.filter((bank) => bank.active).length;
    const activeSolos = state.solos.filter((solo) => solo.active).length;
    const playbackPlan = computePlaybackPlan();
    dom.bankCount.textContent = `${activeBanks} active | ${playbackPlan.totalUnits}/${maxActiveStreamUnits} units`;
    dom.soloCount.textContent = `${activeSolos} active`;
}

function updateMasterReadouts() {
    dom.masterBackgroundReadout.textContent = `${state.masterBackground}%`;
    dom.masterSoloReadout.textContent = `${state.masterSolo}%`;
}

function formatPan(value) {
    if (Math.abs(value) < 4) {
        return "Center";
    }

    return value < 0 ? `${Math.abs(value)}% Left` : `${value}% Right`;
}

async function ensureAudioReady() {
    if (!audioContext) {
        createAudioGraph();
    }

    if (audioContext.state === "suspended") {
        await audioContext.resume();
    }

    document.body.classList.add("audio-live");
    syncWakeButtonState();
    syncPlayback();
    updateStatus();
}

function createAudioGraph() {
    const AudioContextClass = window.AudioContext || window.webkitAudioContext;

    if (!AudioContextClass) {
        dom.statusText.textContent = "This browser cannot start the audio engine.";
        throw new Error("Web Audio API unavailable");
    }

    audioContext = new AudioContextClass();
    backgroundBus = audioContext.createGain();
    soloBus = audioContext.createGain();
    compressor = audioContext.createDynamicsCompressor();
    masterBus = audioContext.createGain();

    compressor.threshold.value = -18;
    compressor.knee.value = 24;
    compressor.ratio.value = 4;
    compressor.attack.value = 0.01;
    compressor.release.value = 0.18;
    masterBus.gain.value = 0.92;

    backgroundBus.connect(compressor);
    soloBus.connect(compressor);
    compressor.connect(masterBus);
    masterBus.connect(audioContext.destination);
}

function getTrackEngine(key, path, options = {}) {
    if (!audioContext) {
        createAudioGraph();
    }

    if (trackEngines.has(key)) {
        return trackEngines.get(key);
    }

    const audio = new Audio(`${resourceBase}/${path}`);
    audio.loop = true;
    audio.preload = "auto";
    audio.playsInline = true;
    audio.setAttribute("playsinline", "true");

    const sourceNode = audioContext.createMediaElementSource(audio);
    const gainNode = audioContext.createGain();
    let panNode = null;

    gainNode.gain.value = 0;

    sourceNode.connect(gainNode);

    if (options.allowPan && typeof audioContext.createStereoPanner === "function") {
        panNode = audioContext.createStereoPanner();
        gainNode.connect(panNode);
        panNode.connect(soloBus);
    } else {
        gainNode.connect(options.allowPan ? soloBus : backgroundBus);
    }

    const engine = {
        audio,
        gainNode,
        panNode,
        isFadingOut: false,
        pauseTimeoutId: null
    };
    trackEngines.set(key, engine);
    return engine;
}

function clearPendingPause(engine) {
    if (engine.pauseTimeoutId !== null) {
        window.clearTimeout(engine.pauseTimeoutId);
        engine.pauseTimeoutId = null;
    }
}

function scheduleGain(engine, targetGain, fadeSeconds = 0) {
    if (!audioContext) {
        return;
    }

    const now = audioContext.currentTime;
    const parameter = engine.gainNode.gain;
    const currentValue = parameter.value;
    parameter.cancelScheduledValues(now);
    parameter.setValueAtTime(currentValue, now);

    if (fadeSeconds <= 0) {
        parameter.setValueAtTime(targetGain, now);
        return;
    }

    parameter.linearRampToValueAtTime(targetGain, now + fadeSeconds);
}

function startEnginePlayback(engine, targetGain, options = {}) {
    clearPendingPause(engine);
    engine.isFadingOut = false;

    if (engine.audio.paused) {
        if (Number.isFinite(options.syncTime) && Number.isFinite(engine.audio.duration) && engine.audio.duration > 0) {
            engine.audio.currentTime = normalizeLoopPosition(options.syncTime, engine.audio.duration);
        } else {
            engine.audio.currentTime = 0;
        }
    }

    playLoop(engine);
    scheduleGain(engine, targetGain, toggleFadeSeconds);
}

function stopEnginePlayback(engine) {
    clearPendingPause(engine);
    engine.isFadingOut = true;
    scheduleGain(engine, 0, toggleFadeSeconds);
    engine.pauseTimeoutId = window.setTimeout(() => {
        engine.audio.pause();
        engine.pauseTimeoutId = null;
        engine.isFadingOut = false;
    }, toggleFadeSeconds * 1000);
}

function setImmediateGain(engine, targetGain) {
    if (!audioContext) {
        return;
    }

    if (engine.isFadingOut) {
        return;
    }

    scheduleGain(engine, targetGain, 0.08);
}

function startStemPlayback(bankIndex, targetGain, syncTime) {
    const engine = getTrackEngine(getStemEngineKey(bankIndex), bankDefinitions[bankIndex].stemPath);
    startEnginePlayback(engine, targetGain, { syncTime });
}

function stopStemPlayback(bankIndex) {
    const engineKey = getStemEngineKey(bankIndex);
    if (!trackEngines.has(engineKey)) {
        return;
    }

    stopEnginePlayback(trackEngines.get(engineKey));
}

function startLayerPlayback(bankIndex, syncTime) {
    bankDefinitions[bankIndex].tracks.forEach((path, trackIndex) => {
        const engineKey = `${bankDefinitions[bankIndex].id}-${trackIndex}`;
        const engine = getTrackEngine(engineKey, path);
        startEnginePlayback(engine, getBankLayerLevel(bankIndex, trackIndex), { syncTime });
    });
}

function stopLayerPlayback(bankIndex) {
    bankDefinitions[bankIndex].tracks.forEach((_, trackIndex) => {
        const engineKey = `${bankDefinitions[bankIndex].id}-${trackIndex}`;
        if (!trackEngines.has(engineKey)) {
            return;
        }

        stopEnginePlayback(trackEngines.get(engineKey));
    });
}

function syncPlayback() {
    if (!audioContext) {
        refreshCardStates();
        return;
    }

    const playbackPlan = computePlaybackPlan();

    bankDefinitions.forEach((bank, bankIndex) => {
        const bankState = state.banks[bankIndex];
        const timelinePosition = getBankTimelinePosition(bankIndex);

        if (!bankState.active) {
            stopStemPlayback(bankIndex);
            stopLayerPlayback(bankIndex);
            setBankTimelinePosition(bankIndex, timelinePosition, "none");
            return;
        }

        const resolvedMode = playbackPlan.bankModes[bankIndex];
        if (resolvedMode === "custom") {
            startLayerPlayback(bankIndex, timelinePosition);
            stopStemPlayback(bankIndex);
        } else {
            startStemPlayback(bankIndex, getBankStemLevel(bankIndex), timelinePosition);
            stopLayerPlayback(bankIndex);
        }

        setBankTimelinePosition(bankIndex, timelinePosition, resolvedMode);
    });

    soloDefinitions.forEach((solo, soloIndex) => {
        const engineKey = `solo-${solo.id}`;
        const shouldPlay = state.solos[soloIndex].active;

        if (!shouldPlay && !trackEngines.has(engineKey)) {
            return;
        }

        const engine = getTrackEngine(engineKey, solo.path, { allowPan: true });
        if (shouldPlay) {
            const soloLevel = (state.masterSolo / 100) * (state.solos[soloIndex].volume / 100) * soloBaseGain;
            startEnginePlayback(engine, soloLevel);
        } else {
            stopEnginePlayback(engine);
        }
    });

    updateAllGains(playbackPlan);
}

function playLoop(engine) {
    const playPromise = engine.audio.play();
    if (playPromise && typeof playPromise.catch === "function") {
        playPromise.catch(() => {
            dom.statusText.textContent = "Tap Wake Audio again if playback is still muted.";
        });
    }
}

function updateAllGains(playbackPlan = computePlaybackPlan()) {
    if (!audioContext) {
        return;
    }

    bankDefinitions.forEach((bank, bankIndex) => {
        const resolvedMode = playbackPlan.bankModes[bankIndex];
        const stemEngineKey = getStemEngineKey(bankIndex);
        if (trackEngines.has(stemEngineKey)) {
            const stemEngine = trackEngines.get(stemEngineKey);
            setImmediateGain(stemEngine, state.banks[bankIndex].active && resolvedMode === "stem" ? getBankStemLevel(bankIndex) : 0);
        }

        bank.tracks.forEach((path, trackIndex) => {
            const engineKey = `${bank.id}-${trackIndex}`;
            if (!trackEngines.has(engineKey)) {
                return;
            }

            const engine = getTrackEngine(engineKey, path);
            const layerLevel = getBankLayerLevel(bankIndex, trackIndex);
            setImmediateGain(engine, state.banks[bankIndex].active && resolvedMode === "custom" ? layerLevel : 0);
        });
    });

    soloDefinitions.forEach((solo, soloIndex) => {
        const soloLevel = (state.masterSolo / 100) * (state.solos[soloIndex].volume / 100) * soloBaseGain;
        const engineKey = `solo-${solo.id}`;
        if (!trackEngines.has(engineKey)) {
            return;
        }

        const engine = getTrackEngine(engineKey, solo.path, { allowPan: true });
        setImmediateGain(engine, state.solos[soloIndex].active ? soloLevel : 0);

        if (engine.panNode) {
            engine.panNode.pan.value = state.solos[soloIndex].pan / 100;
        }
    });

    updateStatus(playbackPlan);
}

function updateStatus(playbackPlan = computePlaybackPlan()) {
    const activeBanks = state.banks.filter((bank) => bank.active).length;
    const activeSolos = state.solos.filter((solo) => solo.active).length;
    const deferredCustomBeds = state.banks.filter((bank, index) => bank.active && getDesiredBankMixMode(bank) === "custom" && playbackPlan.bankModes[index] !== "custom").length;

    if (!audioContext) {
        dom.statusText.textContent = "Idle. Tap any card to start playback.";
        return;
    }

    if (audioContext.state !== "running") {
        dom.statusText.textContent = "Audio is armed but paused. Tap Wake Audio to resume.";
        return;
    }

    if (activeBanks === 0 && activeSolos === 0) {
        dom.statusText.textContent = "Audio live. No layers are playing yet.";
        return;
    }

    if (deferredCustomBeds > 0) {
        dom.statusText.textContent = `${activeBanks} soundscape${activeBanks === 1 ? "" : "s"}, ${activeSolos} solo loop${activeSolos === 1 ? "" : "s"}, ${deferredCustomBeds} custom blend${deferredCustomBeds === 1 ? "" : "s"} saved in stem mode.`;
        return;
    }

    dom.statusText.textContent = `${activeBanks} soundscape${activeBanks === 1 ? "" : "s"} and ${activeSolos} solo loop${activeSolos === 1 ? "" : "s"} active.`;
}

function escapeHtml(value) {
    return String(value)
        .replaceAll("&", "&amp;")
        .replaceAll("<", "&lt;")
        .replaceAll(">", "&gt;")
        .replaceAll('"', "&quot;")
        .replaceAll("'", "&#39;");
}