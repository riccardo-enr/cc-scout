# CC Scout Studio Panel

* **Author:** Riccardo Enrico
* **Date:** 2026-11-09

## Overview

The default GTK styling gives the MIDI monitor little visual identity. Make the
MPK mini IV view feel like a piece of studio hardware. A matte-black faceplate,
tactile-looking knobs and pads, and small amber activity lights put the physical
controls at the center of the window. Keep the raw MIDI feed easy to reach for
troubleshooting.

```text
+-------------------------------------------------+
| CC Scout     Monitoring MPK mini IV    Settings |
|                                                 |
| KNOBS    ( O )     ( O )     ( O )     ( O )    |
|          Division  Swing     Mode      Oct      |
|          * CW +1   CCW -1    -         -        |
|          ( O )     ( O )     ( O )     ( O )    |
|          Latch     Sync      Gate      BPM      |
|                                                 |
| PADS     [ 5   ]   [ 6   ]   [ 7   ]   [ 8   ]  |
|          [ 1   ]   [ 2 * ]   [ 3   ]   [ 4   ]  |
|                                                 |
| [Raw MIDI]                                      |
+-------------------------------------------------+
  * Amber activity light. Layout sketch only.
```

## Analysis

- **Knob movement.** Each mapped knob reports relative steps. Give valid turns
  a short amber pulse that can extend when another turn arrives. Keep the last
  `CW` or `CCW` step visible after the light fades. A fixed pointer or a rotating
  position mark would suggest a parameter position the app cannot read.
  [Knob decoding](../cc-scout-gui/src/knob_grid.c#L38)
- **Pad state.** The app combines the two banks and mirrored MIDI/DAW messages
  for each physical pad. Keep the pad visually depressed until the last release.
  Check quick taps that begin and end in one batch. If they need a short flash,
  keep that cue distinct from the held state. Clear lights on device change or
  loss. Preserve the separate selection highlight in Learn Pad.
  [Live pad state](../cc-scout-gui/src/pad_grid.c#L62)
  [Learn Pad reference](../cc-scout-gui/src/midi_learn.c#L351)
- **Raw MIDI.** Start with the feed collapsed for the MPK mini IV. A labeled,
  keyboard-accessible **Raw MIDI** button reveals it in one action. Continue
  recording the latest 100 events while collapsed, with source addresses,
  learned labels, and separate mirrored events. Keep the feed visible for other
  devices, which have no mapped grids. Keep device, waiting, and error messages
  visible independently of the feed.
  [Event feed](../cc-scout-gui/src/main.c#L89)
  [Device handling](../cc-scout-gui/src/main.c#L284)
- **Readability and input.** Use contrast and a change in shape or depth along
  with amber. Check long learned names and larger text at the existing 640 x 560
  default size. The pictured knobs and pads are passive indicators. Settings and
  the feed must stay responsive during sustained MIDI input.
  [Pad widgets](../cc-scout-gui/src/pad_grid.c#L29)
  [Window layout](../cc-scout-gui/src/main.c#L476)

## Goals vs. Non-Goals

- **Goals:** Make the mapped controls the visual focus, make movement and held
  pads easy to spot, and keep raw events available for diagnosis.
- **Proposed boundaries:** Limit the visual treatment to the live panel. Keep
  window chrome and Settings on the desktop theme. Preserve the current GTK and
  libadwaita minimums, MIDI Learn behavior, saved bindings, device preset, and
  core interface.

## Alternatives Weighed

- **Compact feed always visible for the MPK.** Rejected by author. The feed
  would take space from the pads and knobs even when the user is monitoring the
  controls.
- **One-action Raw MIDI reveal for the MPK.** Selected by author. The control surface gets more room.
  Debugging adds a step, so the reveal must be easy to find and show recent
  history immediately. The feed stays visible for other devices.
- **Dark styling for the whole app.** A uniform finish extends into Settings,
  but overrides the documented desktop theme behavior and affects the shared
  Learn Pad grid. Reviewers favored a matte faceplate within the native window.

## Key Decisions & Staging for Next Steps

- **Agreed direction:** Matte black, tactile knobs and pads, small amber LEDs,
  and a one-click **Raw MIDI** button for the MPK mini IV. The feed stays
  visible for other devices.
- **Review proposal:** Build on the existing GTK widgets. Keep pulse timing
  bounded during dense MIDI input and clear any pending updates on reset or
  window teardown.
- **Design checks:** Try a single and repeated knob turn, a quick pad tap, a pad
  held across mirrored ports, a device reconnect, another MIDI device, and an
  existing saved mapping. Check that the feed stays reachable and readable while
  MIDI arrives and that Learn Pad still shows its capture highlight.
