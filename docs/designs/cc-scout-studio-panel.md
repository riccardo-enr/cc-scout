# CC Scout Studio Panel

- **Author** Riccardo Enrico
- **Status** Approved
- **Date** 2026-11-09
- **Source** [Approved concept brief](../../concepts_brief.md)

## Overview

Give the MPK mini IV monitor the look of studio hardware. A matte-black
faceplate frames tactile-looking knobs and pads, with small amber lights that
show MIDI activity. The mapped controls take center stage when the app opens.

## The Problem

The current window uses stock GTK cards and buttons for the controller, and
the raw feed is always in view. That layout gives the physical controls little
visual emphasis. The feed still matters for debugging and is the only event
view for MIDI devices without mapped controls.
[Current window and device behavior](../../../cc-scout-gui/src/main.c#L284)

```text
MPK mini IV                         Other MIDI device
+----------------------------+      +----------------------------+
| Device status     Settings |      | Device status     Settings |
| Knobs  (o) (o) (o) (o)     |      |                            |
|        (o) (o) (o) (o)     |      | Raw MIDI                    |
| Pads   [5] [6] [7] [8]     |      |   source / channel / event  |
|        [1] [2*][3] [4]     |      |   ...                       |
| [Raw MIDI]                 |      |                            |
+----------------------------+      +----------------------------+
 * Amber activity light. Concept sketch.
```

## Goals

- Make all eight knobs and eight pads the visual focus at the existing
  640 x 560 default window size. Keep learned names and activity readable,
  including with larger text.
- Pulse a knob's amber light when it moves and keep the last `CW` or `CCW`
  step readable after the pulse. Make a held pad visibly depressed and pair
  the amber cue with a visible change in shape or depth. Preserve held state
  across both banks and mirrored ports.
- Start with the feed collapsed for the MPK mini IV. Open it in one action
  with a keyboard-accessible **Raw MIDI** button and show the latest 100
  events, including those captured while collapsed. Keep the feed visible
  for other MIDI devices and keep connection and error status visible in
  either view.

## Non-Goals

- Mouse-operated knobs or pads. The panel reflects incoming MIDI.
- Restyling window chrome or Settings. Both follow the desktop theme.
- Changes to MIDI Learn, saved bindings, device presets, the CC Scout core
  interface, or the minimum GTK and libadwaita versions.

## Technical Plan

Use the existing GTK window, controller grids, and raw event buffer. The
selected device already determines whether the grids are visible. Use that
same device check to choose the layout. In the MPK view, place the grids on
the matte faceplate and show the **Raw MIDI** button. For other devices, show
the existing feed directly. Keep the status and Settings entry available in
both views.
[Current device and window setup](../../../cc-scout-gui/src/main.c#L241)

```text
Incoming MIDI
    |
    +--> Raw event buffer --> Raw MIDI feed
    |                         MPK: behind Raw MIDI button
    |                         Other device: visible
    |
    +--> Existing MPK control mapping
             +--> Relative knob step --> CW/CCW text and amber pulse
             +--> Pad bank/port state --> Held appearance and amber light
```

Keep the feed's text view and bounded buffer in place when the button hides
it. The button changes visibility only. The event reader still appends rows
in arrival order, including mirrored messages, and keeps the latest 100.
Reopening the feed shows that history immediately. Preserve the existing
buffer reset when the monitored ports change.
[Event reader](../../../cc-scout-gui/src/main.c#L89)
[Port change handling](../../../cc-scout-gui/src/main.c#L284)

Give the main controller panel its own CSS scope. The pad grid is also used
by the Learn Pad dialog, where the highlight means a pad has been selected.
Keep that dialog's existing appearance and behavior. Use a static knob cap
because the hardware sends relative steps. For each valid step, update the
readout and extend the activity pulse, even when consecutive steps are equal.
Bound the amount of pending pulse work during bursts and clear it when the
monitor resets or the window closes.
[Knob grid](../../../cc-scout-gui/src/knob_grid.c#L38)
[Learn Pad reference](../../../cc-scout-gui/src/midi_learn.c#L351)

Use the current combined bank and port state for the pad's held appearance.
Verify on hardware whether a quick tap that starts and ends in one event
batch needs a separate flash to be visible. If a flash is needed, give it a
distinct cue so a released pad never looks held. Device loss and source
changes clear the live pad state through the existing reset paths.
[Pad state](../../../cc-scout-gui/src/pad_grid.c#L62)

## Alternatives Considered

### Keep a compact feed visible on the MPK panel

An always-visible feed lets the user watch raw messages while playing without
opening a panel. It also takes space from the eight knobs and eight pads at
the default window size, including when the user only needs to see the
controls. The author chose the one-click **Raw MIDI** button to give the
controls that space. The feed still records while collapsed and remains
visible for other MIDI devices.

### Apply the matte-black finish to the whole app

A uniform finish would carry the studio look into the window chrome and
Settings. It would also override the desktop theme there and require checking
the Learn Pad dialog, which shares the pad grid and uses its highlight to
show the selected pad. The review favored applying the finish to the live
MPK panel. The approved Technical Plan follows that scope.
[Desktop theme behavior](../../../cc-scout-gui/README.md#L3)
[Shared Learn Pad grid](../../../cc-scout-gui/src/midi_learn.c#L351)

## Detailed Implementation Plan

Paths below are relative to the [GUI checkout](../../../cc-scout-gui/).

1. **Modify [`src/main.c`](../../../cc-scout-gui/src/main.c).** Load the bundled
   panel stylesheet and give the live MPK faceplate a dedicated CSS class.
   Keep the header, Settings button, and connection status outside that
   scope. Keep one existing feed widget and buffer for both layouts. Add
   a keyboard-focusable **Raw MIDI** toggle and connect it to the feed's
   visibility. Entering MPK mode starts collapsed. Entering another device
   mode hides the toggle and shows the feed directly. A port refresh or
   reconnect within the same mode preserves the current toggle state. Preserve
   the existing history reset on port changes. When the feed opens, scroll
   to the latest event after the widget is visible. Keep capture and trimming
   active while it is closed. Cancel knob activity work before freeing the
   monitor view.
2. **Modify [`src/knob_grid.c`](../../../cc-scout-gui/src/knob_grid.c) and
   [`src/knob_grid.h`](../../../cc-scout-gui/src/knob_grid.h).** Add a static cap and
   an amber indicator to each tile. Keep learned names and the last signed
   step. Store an expiry for each slot and use one pending timeout for the
   grid. Each accepted step refreshes its slot's deadline, including repeated
   values. Schedule against the next expiry, turn off expired lights, and
   stop scheduling when no lights are active. Reset cancels the timeout and
   clears the indicators and readouts. Start visual tuning at 150 ms per
   pulse and check it on hardware before settling the value.
3. **Modify [`src/pad_grid.c`](../../../cc-scout-gui/src/pad_grid.c).** Add panel
   styling hooks to the existing tiles. Use their active state to drive the
   amber marker and depressed appearance on the MPK faceplate. Preserve the
   pad label as the button's child so the existing naming and Learn Pad code
   keep working. Leave note matching, bank and port aggregation, and the
   Settings highlight on their existing paths. Check quick taps on hardware
   before deciding whether to add a separate flash. If needed, use a brief
   cue distinct from the held appearance and document its timing before
   adding it.
4. **Create `src/studio-panel.css` and `src/studio-panel.gresource.xml`.**
   Define the matte faceplate, knob caps, pad depth, readable labels, and
   amber indicators in the stylesheet. Scope the selectors under the
   faceplate class so the shared Learn Pad grid keeps its desktop styling.
   Register the stylesheet as a bundled GUI resource so loading it does
   not depend on the launch directory.
5. **Modify [`CMakeLists.txt`](../../../cc-scout-gui/CMakeLists.txt).** Generate the
   studio resource with the existing `glib-compile-resources` tool and link
   it into `cc-scout-gui`. Declare the XML and CSS as build dependencies so
   stylesheet edits regenerate the resource. Keep the preset bundle and the
   GTK and libadwaita minimums in place.
6. **Modify [`README.md`](../../../cc-scout-gui/README.md).** Describe the MPK
   faceplate, the knob and pad activity cues, and the **Raw MIDI** button.
   Explain that the feed remains visible for other devices and that window
   chrome and Settings still follow the desktop theme.

### Verification Plan

| Check | Expected result |
| --- | --- |
| Build and launch from outside the checkout | The bundled stylesheet loads, and GTK reports no CSS parse errors. |
| MPK startup at 640 x 560 with long learned names and larger text | Both grids fit at default text size. With larger text, names and activity stay readable and the Raw MIDI control remains reachable. |
| Isolated and repeated turns in both directions | Only the mapped knob pulses. Repeated steps extend the pulse. The signed CW/CCW readout persists after the light fades. |
| Quick taps, both banks, and mirrored press/release | Both banks use the same physical positions. A pad stays depressed until the last held source releases. Quick taps get a visible cue, with any short flash visually distinct from a hold. |
| Open Raw MIDI by mouse and keyboard after more than 100 messages | One action reveals the latest 100 events in order, including separate mirrored messages. Capture continued while collapsed. |
| Select another device, then return to the MPK | The other device shows the feed directly. Returning to the MPK shows the grids with the feed collapsed. |
| Disconnect, reconnect, or close during active input | Indicators reset when monitoring is interrupted. No pending callback touches freed widgets. Reconnection restores monitoring. |
| Sustained MIDI input | Raw MIDI and Settings stay responsive. The event history and pending LED work stay bounded. |
| Learn Pad and saved bindings in desktop light and dark modes | The capture highlight still indicates selection. Saved labels and slots reload, and Settings follows the desktop theme. |

*Drafted with EGM*
