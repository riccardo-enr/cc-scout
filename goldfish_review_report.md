# Goldfish Annotated Review Report

**Target** [CC Scout Studio Panel](docs/designs/cc-scout-studio-panel.md), full file

**Source** [Consolidated review](consolidated_review_report.md)

**Status** Proposed edits for review

```text
Fast pad press -> last release -> pad face returns to rest
                              -> separate amber outline flashes
```

## A1. Quick pad tap feedback | Blocker | PROPOSE FIX

**Scope** [Pad behavior](docs/designs/cc-scout-studio-panel.md#L97-L101),
[implementation](docs/designs/cc-scout-studio-panel.md#L150-L158), and verification.

The current event loop can apply press and release before GTK paints. The
pad toggle tracks the final combined held state, so a whole tap can disappear.
[Event reader](../cc-scout-gui/src/main.c#L89)
[Pad aggregation](../cc-scout-gui/src/pad_grid.c#L86)

**Proposed edit** Replace the optional hardware decision with this rule.
Measure from the first aggregate press to the last aggregate release across
both banks and ports. If that interval is under 120 ms, show an amber outline
for 120 ms from the release while the pad face returns to its raised shape.
A new aggregate press cancels the previous outline and shows the held state.
A later short release starts a fresh outline. Duplicate mirrored messages
never restart a press or flash. Clear the cue on reset and cancel its timer
on teardown. Apply the rule only to the live panel and include
`src/pad_grid.h` in the plan. Test a press and last release in one dispatch,
a mirrored release that leaves the pad held, and a new press during a flash.

The 120 ms values are proposed acceptance baselines. Hardware testing can
assess the appearance after the behavior is specified.

## A2. Expanded layout and scaling | Major | PROPOSE FIX

**Scope** [Window behavior](docs/designs/cc-scout-studio-panel.md#L22-L48)
and [verification](docs/designs/cc-scout-studio-panel.md#L179-L183).

The existing window only scrolls the feed. Added controls and larger text can
consume its entire viewport. [Window layout](../cc-scout-gui/src/main.c#L497)

**Proposed edit** Sketch Raw MIDI expanded below the faceplate. Keep device
status and the toggle fixed. Give the faceplate and feed separate scrolling
regions and reserve at least four feed text rows when expanded at 640 x 560.
Keep all controls visible with default text and allow the faceplate to scroll
at 150% text. At both sizes, let users read a full 64-character learned label
with the keyboard. Keep focus on the toggle when activating it and scroll the
feed to the newest event after allocation.

## A3. Layout before discovery | Major | PROPOSE FIX

**Scope** [Layout implementation](docs/designs/cc-scout-studio-panel.md#L129-L138).

Enumeration can fail before the layout update, and an unchanged port list can
bypass that update. [Enumeration](../cc-scout-gui/src/main.c#L361)
[Monitor update](../cc-scout-gui/src/main.c#L241)

**Proposed edit** Apply the default selected MPK layout before the first ALSA
scan. Update mode from the selected profile before the monitor restart guard.
Each entry into MPK mode starts collapsed. Reconnect and refresh within MPK
mode preserve the toggle, and port changes separately clear history. Test
waiting, failed enumeration, both reconnect states, and mode switching.

## A4. GTK responsiveness | Major | PROPOSE FIX

**Scope** [Event path](docs/designs/cc-scout-studio-panel.md#L79-L93)
and [load verification](docs/designs/cc-scout-studio-panel.md#L186).

A full batch immediately rearms the MIDI source. The reader also requests
scrolling even when the feed is hidden. [Dispatch](../cc-scout-gui/src/main.c#L179)

**Proposed edit** Keep one reader and the 64-event batch limit. Schedule MIDI
dispatch at a priority below GTK input and redraw, retaining immediate backlog
continuation at that priority. Skip hidden-feed scroll requests and coalesce
visible scroll requests to one per frame. Start with a reproducible 1,000
events/s stream for 10 seconds, plus an injected backlog exceeding one batch.
Require the toggle, Settings, and a mapped activity cue to respond within
100 ms under the test load. Check capture order and the 100-event bound.

## A5. Knob pulse lifecycle | Major | PROPOSE FIX

**Scope** [Knob implementation](docs/designs/cc-scout-studio-panel.md#L139-L149).

The plan leaves pulse timing provisional, and the current live reset touches
child widgets. [Window cleanup](../cc-scout-gui/src/main.c#L407)
[Knob reset](../cc-scout-gui/src/knob_grid.c#L32)

**Proposed edit** Require a 150 ms expiry from the latest accepted step with
one pending knob timeout. Verify the deadline using a controlled clock and
check appearance on hardware. Separate live widget reset from a teardown
operation that only cancels pending work. Keep the existing readout behavior
when Learn All reassigns a slot, with the indicator expiring normally.

## A6. Build and styling contracts | Major | PROPOSE FIX

**Scope** [File inventory](docs/designs/cc-scout-studio-panel.md#L125-L175).

The stylesheet loader, resource URI, and widget classes need to match across
files. [Existing resource pipeline](../cc-scout-gui/CMakeLists.txt#L20)

**Proposed edit** Bundle CSS at `/io/github/riccardo_enr/CCScoutGui/studio-panel.css`
and preserve the existing preset URI. Load the CSS with
`gtk_css_provider_load_from_resource()` and install one provider per app
display. Scope selectors under `.studio-panel`, with `.knob-cap`,
`.knob-activity`, `.pad-tile`, and `.pad-tap` hooks. Use the button checked
state for held depth and an outline for the released tap cue. Keep the existing
label child. Group resource packaging and loading, view layout, and activity
cues into buildable phases with a CMake build and a relevant smoke check each.

## A7. Documentation | PROPOSE FIX

**Scope** Feature spec, alternatives, and verification table.

**Proposed edit** Add a short risks and mitigations section, describe mirrored
events with one example, show a `CW +1` readout, and refer to the Learn Pad
reference in Settings. Use "toggle" for the control and "feed" for the event
view. Keep the alternatives' technical rationale and remove unsupported
attribution. Retain local sibling links for the current checkout layout.
