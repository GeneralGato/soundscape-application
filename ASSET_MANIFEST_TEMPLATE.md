# Soundscape Asset Manifest Template

This template is meant to describe the sound library as the long-term source of truth.

It is intentionally not tied to the earlier proof-of-concept rule of six layers per bed. The current runtimes now read from the manifest, but the manifest should still stay free to describe:

- fewer than six meaningful layers
- more than six source layers
- merged delivery layers
- temporary internet-sourced placeholders that will later be replaced with your own recordings

## What This Manifest Should Track

- the creative identity of each bed or solo loop
- who owns the source material and whether it is temporary
- which source layers exist in the ideal library
- which delivery files currently exist for the app
- which source layers were merged, replaced, or removed to make the current delivery set
- exact loop and format facts needed for implementation

## Recommended Structure

Use one entry per ambient bed and one entry per solo loop.

For beds, separate:

- `sourceLayers`: the conceptual or archived ingredients you want the library to contain over time
- `delivery`: the files the current app actually ships and plays

That split keeps the manifest useful even when:

- a bed only needs three real layers
- two weak layers get merged into one stronger delivery layer
- a future field recording replaces a temporary downloaded layer
- the runtime later moves away from the current six-slider model

## JSON Template

```json
{
  "manifestVersion": 1,
  "project": "Soundscape Application",
  "libraryStatus": "working",
  "notes": "Library truth, not runtime truth. Current app delivery should reflect the files the manifest points at, even if future runtimes expose a different set of controls.",
  "beds": [
    {
      "id": "forest",
      "displayName": "Forest",
      "status": "prototype",
      "ownership": "mixed",
      "replacePriority": "high",
      "description": "Leaves, birds, and insects with a calm outdoor bed.",
      "sourceSession": "Reason project or field-recording session name",
      "archiveMasterPath": "Archive/Forest/forest-master-48k.wav",
      "loop": {
        "durationSeconds": 180,
        "durationSamplesAt48k": 8640000,
        "sampleRateHz": 48000,
        "seamNotes": "Keep strong bird calls away from the seam.",
        "qaStatus": "needs-review"
      },
      "sourceLayers": [
        {
          "id": "leaves-close",
          "displayName": "Leaves Close",
          "role": "texture",
          "origin": "own",
          "status": "keep",
          "plannedAction": "keep",
          "notes": "Good base rustle layer."
        },
        {
          "id": "birds-near",
          "displayName": "Birds Near",
          "role": "detail",
          "origin": "downloaded-temp",
          "status": "temporary",
          "plannedAction": "replace",
          "notes": "Replace with self-recorded dawn chorus."
        },
        {
          "id": "bees-wide",
          "displayName": "Bees Wide",
          "role": "motion",
          "origin": "own",
          "status": "keep",
          "plannedAction": "merge",
          "notes": "May merge with insects-bed for current mobile delivery."
        }
      ],
      "delivery": {
        "runtimeProfile": "current-runtime",
        "stem": {
          "path": "Resources/Soundscapes/Stems/forest-stem.mp3",
          "channelFormat": "stereo",
          "defaultPlayback": true,
          "peakTarget": "-3 dBTP max"
        },
        "customLayers": [
          {
            "id": "leaves-bed",
            "order": 1,
            "displayName": "Leaves",
            "path": "Resources/Soundscapes/Forest/1. Leaves.mp3",
            "channelFormat": "mono",
            "panReady": true,
            "derivedFrom": ["leaves-close"],
            "defaultLevelPercent": 70,
            "notes": "Single-source delivery layer."
          },
          {
            "id": "birds-bed",
            "order": 2,
            "displayName": "Birdsong",
            "path": "Resources/Soundscapes/Forest/2. Birdsong.mp3",
            "channelFormat": "stereo",
            "panReady": false,
            "derivedFrom": ["birds-near"],
            "defaultLevelPercent": 52,
            "notes": "Temporary internet source until replaced."
          },
          {
            "id": "insects-bed",
            "order": 3,
            "displayName": "Insects",
            "path": "Resources/Soundscapes/Forest/3. Insects.mp3",
            "channelFormat": "stereo",
            "panReady": false,
            "derivedFrom": ["bees-wide"],
            "defaultLevelPercent": 48,
            "notes": "May absorb multiple future source layers."
          }
        ],
        "deliveryNotes": "Current runtime should mirror the custom layers defined here, even if a bed only needs three exported layers right now."
      },
      "futureIdeas": [
        "Replace temporary birds with own recordings.",
        "Try a second stem variant with less high-frequency activity for fatigue testing."
      ],
      "implementationNotes": "If the runtime still needs six custom slots, unused slots should be treated as an app-mapping problem rather than a library-design rule."
    }
  ],
  "soloLoops": [
    {
      "id": "campfire",
      "displayName": "Campfire",
      "status": "prototype",
      "origin": "own",
      "replacePriority": "low",
      "archiveMasterPath": "Archive/Solos/campfire-master-48k.wav",
      "deliveryPath": "Resources/4. Campfire.mp3",
      "sampleRateHz": 48000,
      "channelFormat": "stereo",
      "panMode": "fixed-image",
      "loopDurationSeconds": 60,
      "loopDurationSamplesAt48k": 2880000,
      "seamNotes": "Avoid obvious crackle bursts near the loop point.",
      "cleanupNotes": "RX de-click pass and gentle low-end cleanup.",
      "notes": "Stable foreground layer for longer listening sessions."
    }
  ]
}
```

## Field Guidance

- `ownership` or `origin`: use values like `own`, `downloaded-temp`, `licensed`, or `mixed`
- `status`: use values like `prototype`, `keep`, `temporary`, `replace-soon`, or `retired`
- `plannedAction`: use values like `keep`, `replace`, `merge`, `remove`, or `split`
- `sourceLayers`: the ideal library ingredients, even if they are not all exported to the app right now
- `delivery.customLayers`: only the files currently exported for the app runtime
- `derivedFrom`: the bridge between source intent and current delivery reality

## Minimal Required Fields

If you want a leaner version, these are the minimum fields worth keeping populated.

For beds:

- `id`
- `displayName`
- `status`
- `ownership`
- `loop.durationSeconds`
- `loop.durationSamplesAt48k`
- `delivery.stem.path`
- `delivery.customLayers`
- `sourceLayers`
- `implementationNotes`

For solo loops:

- `id`
- `displayName`
- `origin`
- `deliveryPath`
- `sampleRateHz`
- `channelFormat`
- `panMode`
- `loopDurationSeconds`
- `loopDurationSamplesAt48k`

## Suggested Use With Your Current Workflow

- Do cleanup and repair in RX 8.
- Do arrangement, synchronized layer rendering, and stem exports in Reason.
- Keep this manifest focused on truth about the library, not on what the current UI happens to expose.
- When the runtime changes later, update the delivery section first and keep the source layer history intact.

## Why This Fits The Current Repo State

The current phone runtime and desktop player both read from the manifest, and the desktop mixer already supports variable per-bed layer counts. This template is still designed to survive future playback changes so library work does not have to be re-modeled every time the UI or engine evolves.