const query = new URLSearchParams(window.location.search);
const isAndroidWrapper = query.get("wrapper") === "android";
const storageKey = "soundscape-pocket-state-v1";
const resourceBase = isAndroidWrapper ? "./Resources" : "../Resources";
const backgroundLayerBaseGain = 0.11;
const soloBaseGain = 0.68;
const defaultBackgroundLayerVolume = 100;

const bankDefinitions = [
    {
        id: "ambient",
        title: "Ambient",
        note: "Choir, piano, and synth textures.",
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

const soloDefinitions = [
    { id: "light-stream", title: "Light Stream", path: "1. Light Stream.mp3", note: "Bright water texture." },
    { id: "river", title: "River", path: "2. Stream.mp3", note: "A broader steady flow." },
    { id: "waterfall", title: "Waterfall", path: "3. Waterfall.mp3", note: "Dense rushing water." },
    { id: "campfire", title: "Campfire", path: "4. Campfire.mp3", note: "Low crackle and warmth." },
    { id: "cat-purr", title: "Cat Purr", path: "5. Cat Purr.mp3", note: "Soft, close comfort." },
    { id: "crows", title: "Crows", path: "6. Crows.mp3", note: "Sparse outdoor movement." }
];

const dom = {
    wakeButtons: Array.from(document.querySelectorAll("[data-role='wake-audio']")),
    statusText: document.querySelector("#status-text"),
    stopButtons: Array.from(document.querySelectorAll("[data-role='stop-all']")),
    masterBackground: document.querySelector("#master-background"),
    masterBackgroundReadout: document.querySelector("#master-background-readout"),
    masterSolo: document.querySelector("#master-solo"),
    masterSoloReadout: document.querySelector("#master-solo-readout"),
    bankGrid: document.querySelector("#bank-grid"),
    soloGrid: document.querySelector("#solo-grid"),
    bankCount: document.querySelector("#bank-count"),
    soloCount: document.querySelector("#solo-count"),
    presetButtons: Array.from(document.querySelectorAll("[data-role='mix-preset']")),
    jumpSoundscapes: document.querySelector("#jump-soundscapes"),
    jumpSolos: document.querySelector("#jump-solos"),
    soundscapeSection: document.querySelector("#soundscape-section"),
    soloSection: document.querySelector("#solo-section")
};

let state = loadState();
let audioContext = null;
let backgroundBus = null;
let soloBus = null;
let compressor = null;
let masterBus = null;

const trackEngines = new Map();

initialize();

function initialize() {
    document.body.classList.toggle("wrapper-mode", isAndroidWrapper);

    dom.masterBackground.value = String(state.masterBackground);
    dom.masterSolo.value = String(state.masterSolo);

    renderBankCards();
    renderSoloCards();
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

    dom.presetButtons.forEach((button) => {
        button.addEventListener("click", () => {
            applyMixPreset(button);
        });
    });

    dom.jumpSoundscapes.addEventListener("click", () => {
        dom.soundscapeSection?.scrollIntoView({ behavior: "smooth", block: "start" });
    });

    dom.jumpSolos.addEventListener("click", () => {
        dom.soloSection?.scrollIntoView({ behavior: "smooth", block: "start" });
    });

    syncWakeButtonState();
    updatePresetStates();
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
            layerVolumes: bank.tracks.map(() => defaultBackgroundLayerVolume)
        })),
        solos: soloDefinitions.map(() => ({ active: false, volume: 62, pan: 0 }))
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
        return {
            masterBackground: clampNumber(parsed.masterBackground, fallback.masterBackground, 0, 100),
            masterSolo: clampNumber(parsed.masterSolo, fallback.masterSolo, 0, 100),
            banks: bankDefinitions.map((bank, index) => ({
                active: Boolean(parsed.banks?.[index]?.active),
                volume: clampNumber(parsed.banks?.[index]?.volume, fallback.banks[index].volume, 0, 100),
                layerVolumes: bank.tracks.map((_, layerIndex) => clampNumber(
                    parsed.banks?.[index]?.layerVolumes?.[layerIndex],
                    fallback.banks[index].layerVolumes[layerIndex],
                    0,
                    100
                ))
            })),
            solos: soloDefinitions.map((_, index) => ({
                active: Boolean(parsed.solos?.[index]?.active),
                volume: clampNumber(parsed.solos?.[index]?.volume, fallback.solos[index].volume, 0, 100),
                pan: clampNumber(parsed.solos?.[index]?.pan, fallback.solos[index].pan, -100, 100)
            }))
        };
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

function setBankMixerExpanded(card, expanded) {
    const toggle = card.querySelector("[data-role='bank-blend-toggle']");
    const mixer = card.querySelector("[data-role='bank-layer-mixer']");

    if (!toggle || !mixer) {
        return;
    }

    card.classList.toggle("is-expanded", expanded);
    mixer.hidden = !expanded;
    toggle.setAttribute("aria-expanded", String(expanded));
    toggle.textContent = expanded ? "Hide layers" : "Blend layers";
}

function renderBankCards() {
    dom.bankGrid.innerHTML = "";

    bankDefinitions.forEach((bank, index) => {
        const article = document.createElement("article");
        article.className = "sound-card";
        article.dataset.role = "bank-card";
        article.dataset.index = String(index);

        article.innerHTML = `
            <div class="sound-card-header">
                <button class="pill-button sound-toggle" type="button" data-role="bank-toggle" data-index="${index}" data-state-label="Standby">${bank.title}</button>
                <p class="card-note">${bank.note}</p>
            </div>
            <div class="slider-stack">
                <span>Scene level</span>
                <input type="range" min="0" max="100" step="1" data-role="bank-volume" data-index="${index}">
                <strong data-role="bank-volume-readout" data-index="${index}">70%</strong>
            </div>
            <div class="micro-row">
                <span class="micro-copy" data-role="bank-layer-summary" data-index="${index}">6 looping layers</span>
                <button class="status-chip blend-toggle" type="button" data-role="bank-blend-toggle" data-index="${index}" aria-expanded="false">Blend layers</button>
            </div>
            <div class="layer-mixer" data-role="bank-layer-mixer" data-index="${index}" hidden>
                ${bank.tracks.map((path, layerIndex) => `
                    <label class="layer-row">
                        <div class="layer-row-head">
                            <span class="layer-name">${getTrackLabel(path)}</span>
                            <strong class="layer-readout" data-role="bank-layer-readout" data-index="${index}" data-layer-index="${layerIndex}">100%</strong>
                        </div>
                        <input type="range" min="0" max="100" step="1" data-role="bank-layer-volume" data-index="${index}" data-layer-index="${layerIndex}">
                    </label>
                `).join("")}
            </div>
        `;

        dom.bankGrid.append(article);
    });

    dom.bankGrid.querySelectorAll("[data-role='bank-toggle']").forEach((button) => {
        button.addEventListener("click", async () => {
            const index = Number(button.dataset.index);
            await ensureAudioReady();
            state.banks[index].active = !state.banks[index].active;
            persistState();
            syncPlayback();
            refreshCardStates();
            updateStatus();
        });
    });

    dom.bankGrid.querySelectorAll("[data-role='bank-volume']").forEach((slider) => {
        slider.addEventListener("input", () => {
            const index = Number(slider.dataset.index);
            state.banks[index].volume = Number(slider.value);
            persistState();
            updateAllGains();
            refreshCardStates();
        });
    });

    dom.bankGrid.querySelectorAll("[data-role='bank-blend-toggle']").forEach((button) => {
        button.addEventListener("click", () => {
            const card = button.closest("[data-role='bank-card']");
            if (!card) {
                return;
            }

            const isExpanded = button.getAttribute("aria-expanded") === "true";
            setBankMixerExpanded(card, !isExpanded);
        });
    });

    dom.bankGrid.querySelectorAll("[data-role='bank-layer-volume']").forEach((slider) => {
        slider.addEventListener("input", () => {
            const bankIndex = Number(slider.dataset.index);
            const layerIndex = Number(slider.dataset.layerIndex);
            state.banks[bankIndex].layerVolumes[layerIndex] = Number(slider.value);
            persistState();
            updateAllGains();
            refreshCardStates();
        });
    });

    refreshCardStates();
}

function renderSoloCards() {
    dom.soloGrid.innerHTML = "";

    soloDefinitions.forEach((solo, index) => {
        const article = document.createElement("article");
        article.className = "sound-card";
        article.dataset.role = "solo-card";
        article.dataset.index = String(index);

        article.innerHTML = `
            <div class="sound-card-header">
                <button class="pill-button sound-toggle" type="button" data-role="solo-toggle" data-index="${index}" data-state-label="Standby">${solo.title}</button>
                <p class="card-note">${solo.note}</p>
            </div>
            <div class="control-grid">
                <label class="control-label">
                    <span>Level</span>
                    <input type="range" min="0" max="100" step="1" data-role="solo-volume" data-index="${index}">
                    <strong data-role="solo-volume-readout" data-index="${index}">62%</strong>
                </label>
                <label class="control-label">
                    <span>Pan</span>
                    <input type="range" min="-100" max="100" step="1" data-role="solo-pan" data-index="${index}">
                    <strong data-role="solo-pan-readout" data-index="${index}">Center</strong>
                </label>
            </div>
        `;

        dom.soloGrid.append(article);
    });

    dom.soloGrid.querySelectorAll("[data-role='solo-toggle']").forEach((button) => {
        button.addEventListener("click", async () => {
            const index = Number(button.dataset.index);
            await ensureAudioReady();
            state.solos[index].active = !state.solos[index].active;
            persistState();
            syncPlayback();
            refreshCardStates();
            updateStatus();
        });
    });

    dom.soloGrid.querySelectorAll("[data-role='solo-volume']").forEach((slider) => {
        slider.addEventListener("input", () => {
            const index = Number(slider.dataset.index);
            state.solos[index].volume = Number(slider.value);
            persistState();
            updateAllGains();
            refreshCardStates();
        });
    });

    dom.soloGrid.querySelectorAll("[data-role='solo-pan']").forEach((slider) => {
        slider.addEventListener("input", () => {
            const index = Number(slider.dataset.index);
            state.solos[index].pan = Number(slider.value);
            persistState();
            updateAllGains();
            refreshCardStates();
        });
    });

    refreshCardStates();
}

function refreshCardStates() {
    document.querySelectorAll("[data-role='bank-card']").forEach((card) => {
        const index = Number(card.dataset.index);
        const config = state.banks[index];
        const toggle = card.querySelector("[data-role='bank-toggle']");
        const slider = card.querySelector("[data-role='bank-volume']");
        const readout = card.querySelector("[data-role='bank-volume-readout']");
        const summary = card.querySelector("[data-role='bank-layer-summary']");

        card.classList.toggle("is-active", config.active);
        toggle.classList.toggle("is-active", config.active);
        toggle.dataset.stateLabel = config.active ? "Live" : "Standby";
        slider.value = String(config.volume);
        readout.textContent = `${config.volume}%`;

        const isCustomBlend = config.layerVolumes.some((value) => value !== defaultBackgroundLayerVolume);
        summary.textContent = isCustomBlend ? "Custom layer blend" : "6 looping layers";

        card.querySelectorAll("[data-role='bank-layer-volume']").forEach((layerSlider) => {
            const layerIndex = Number(layerSlider.dataset.layerIndex);
            const layerVolume = config.layerVolumes[layerIndex];
            const layerReadout = card.querySelector(`[data-role='bank-layer-readout'][data-layer-index='${layerIndex}']`);
            layerSlider.value = String(layerVolume);
            layerReadout.textContent = `${layerVolume}%`;
        });
    });

    document.querySelectorAll("[data-role='solo-card']").forEach((card) => {
        const index = Number(card.dataset.index);
        const config = state.solos[index];
        const toggle = card.querySelector("[data-role='solo-toggle']");
        const volumeSlider = card.querySelector("[data-role='solo-volume']");
        const volumeReadout = card.querySelector("[data-role='solo-volume-readout']");
        const panSlider = card.querySelector("[data-role='solo-pan']");
        const panReadout = card.querySelector("[data-role='solo-pan-readout']");

        card.classList.toggle("is-active", config.active);
        toggle.classList.toggle("is-active", config.active);
        toggle.dataset.stateLabel = config.active ? "Live" : "Standby";
        volumeSlider.value = String(config.volume);
        volumeReadout.textContent = `${config.volume}%`;
        panSlider.value = String(config.pan);
        panReadout.textContent = formatPan(config.pan);
    });

    updateCounts();
}

function updateCounts() {
    const activeBanks = state.banks.filter((bank) => bank.active).length;
    const activeSolos = state.solos.filter((solo) => solo.active).length;
    dom.bankCount.textContent = `${activeBanks} active`;
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

    sourceNode.connect(gainNode);

    if (options.allowPan && typeof audioContext.createStereoPanner === "function") {
        panNode = audioContext.createStereoPanner();
        gainNode.connect(panNode);
        panNode.connect(soloBus);
    } else {
        gainNode.connect(options.allowPan ? soloBus : backgroundBus);
    }

    const engine = { audio, gainNode, panNode };
    trackEngines.set(key, engine);
    return engine;
}

function syncPlayback() {
    if (!audioContext) {
        refreshCardStates();
        return;
    }

    bankDefinitions.forEach((bank, bankIndex) => {
        bank.tracks.forEach((path, trackIndex) => {
            const engineKey = `${bank.id}-${trackIndex}`;
            const shouldPlay = state.banks[bankIndex].active;

            if (!shouldPlay && !trackEngines.has(engineKey)) {
                return;
            }

            const engine = getTrackEngine(engineKey, path);

            if (shouldPlay) {
                playLoop(engine);
            } else {
                engine.audio.pause();
            }
        });
    });

    soloDefinitions.forEach((solo, soloIndex) => {
        const engineKey = `solo-${solo.id}`;
        const shouldPlay = state.solos[soloIndex].active;

        if (!shouldPlay && !trackEngines.has(engineKey)) {
            return;
        }

        const engine = getTrackEngine(engineKey, solo.path, { allowPan: true });
        if (shouldPlay) {
            playLoop(engine);
        } else {
            engine.audio.pause();
        }
    });

    updateAllGains();
}

function playLoop(engine) {
    const playPromise = engine.audio.play();
    if (playPromise && typeof playPromise.catch === "function") {
        playPromise.catch(() => {
            dom.statusText.textContent = "Tap Wake Audio again if playback is still muted.";
        });
    }
}

function updateAllGains() {
    if (!audioContext) {
        return;
    }

    bankDefinitions.forEach((bank, bankIndex) => {
        const bankLevel = (state.masterBackground / 100) * (state.banks[bankIndex].volume / 100) * backgroundLayerBaseGain;
        bank.tracks.forEach((path, trackIndex) => {
            const engineKey = `${bank.id}-${trackIndex}`;
            if (!trackEngines.has(engineKey)) {
                return;
            }

            const engine = getTrackEngine(engineKey, path);
            const layerLevel = bankLevel * (state.banks[bankIndex].layerVolumes[trackIndex] / 100);
            engine.gainNode.gain.value = state.banks[bankIndex].active ? layerLevel : 0;
        });
    });

    soloDefinitions.forEach((solo, soloIndex) => {
        const soloLevel = (state.masterSolo / 100) * (state.solos[soloIndex].volume / 100) * soloBaseGain;
        const engineKey = `solo-${solo.id}`;
        if (!trackEngines.has(engineKey)) {
            return;
        }

        const engine = getTrackEngine(engineKey, solo.path, { allowPan: true });
        engine.gainNode.gain.value = state.solos[soloIndex].active ? soloLevel : 0;

        if (engine.panNode) {
            engine.panNode.pan.value = state.solos[soloIndex].pan / 100;
        }
    });

    updateStatus();
}

function updateStatus() {
    const activeBanks = state.banks.filter((bank) => bank.active).length;
    const activeSolos = state.solos.filter((solo) => solo.active).length;

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

    dom.statusText.textContent = `${activeBanks} soundscape${activeBanks === 1 ? "" : "s"} and ${activeSolos} solo loop${activeSolos === 1 ? "" : "s"} active.`;
}