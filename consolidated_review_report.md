# CC Scout Studio Panel Goldfish Review

Target [approved design](docs/designs/cc-scout-studio-panel.md), lines 1-189.
The feature spec ends at line 124. The implementation plan begins at line 125.

**Verdict** One blocker remains before implementation. The plan requires
quick pad taps to be visible and leaves the behavior that would show them
undecided. The MPK-only studio treatment and direct feed for other devices
remain the basis of the design.

```text
One MIDI dispatch:    pad press -> last release
Next GTK draw:                              -> released pad
Required tap cue:                           -> brief activity, pad released
```

## Findings

### A1. Blocker | Define quick-tap feedback

The plan defers a flash to hardware testing, but acceptance requires a
visible cue. The reader handles up to 64 events in one dispatch and the
pad grid immediately applies the combined held state. If press and release
both precede the next draw, the user can see only the released state.
Define a distinct, bounded cue for that case, including what triggers it,
how long it lasts, how a new press affects it, and when to clear it.
Preserve the existing held state and Learn Pad capture highlight. Add a
deterministic same-batch test and list any additional files the cue needs.
See design lines 97-101, 150-158, and 182.
[Event reader](../cc-scout-gui/src/main.c#L89)
[Pad aggregation](../cc-scout-gui/src/pad_grid.c#L86)

### A2. Major | Specify the expanded layout and text scaling

The sketch shows Raw MIDI collapsed. The existing outer layout scrolls
only the feed, so taller tiles and larger text can leave little room when
the feed opens. Show the expanded view at 640 x 560. Specify what stays
visible, which region scrolls, how much feed space is usable, and how
keyboard users reach the toggle and full learned names. Set a text scale
and sample long names for the check. Define how the feed scrolls to its
latest row once it is visible and allocated.
See lines 22-48, 129-138, and 179-183.
[Current layout](../cc-scout-gui/src/main.c#L497)
[Label sizing](../cc-scout-gui/src/knob_grid.c#L18)

### A3. Major | Apply the selected layout independently of ALSA discovery

An initial enumeration error can skip the layout update. The existing
monitor update also returns early if the ports are unchanged. Initialize
the layout from the effective selected device before scanning and apply
mode changes before the port comparison can return early. State and test
MPK startup with no device or with enumeration failure, return to MPK
mode, reconnect with the feed both expanded and collapsed, and the
separate history reset on port change.
See lines 44-48, 59-65, 129-137, and 184-185.
[Enumeration](../cc-scout-gui/src/main.c#L361)
[Monitor update](../cc-scout-gui/src/main.c#L241)

### A4. Major | Give GTK time to paint and handle input under MIDI load

The MIDI source can rearm immediately after every full batch, and the
reader requests scrolling for each batch. A bounded buffer and timer
count limit retained work, but GTK drawing and input can still be delayed
under continuous backlog. Define how dispatch yields, skip scroll
requests while Raw MIDI is hidden, and set a repeatable input rate,
duration, and response bound for drawing and interaction in both layouts.
See lines 79-93, 138-139, and 186.
[Dispatch](../cc-scout-gui/src/main.c#L179)
[Current scrolling](../cc-scout-gui/src/main.c#L144)

### A5. Major | Set pulse timing and cancellation rules

The detailed plan refreshes a per-slot deadline, but leaves the duration
provisional. Pin the initial acceptance value and how it is measured.
Specify timer-only cancellation at teardown, separate from the live reset
that writes widget state. Check teardown independently from reconnect.
State whether a reassigned knob slot inherits the previous readout and
pulse. Learn All can change its binding and update its name in place.
See lines 89-93, 139-149, and 181-187.
[Window cleanup](../cc-scout-gui/src/main.c#L407)
[Knob reset](../cc-scout-gui/src/knob_grid.c#L32)
[Binding save](../cc-scout-gui/src/midi_learn.c#L268)

### A6. Major | Pin build and styling contracts

Specify the stylesheet URI, provider lifetime, GTK 4.6-compatible load
operation, shared CSS classes and states, and how the pad marker is
drawn while preserving the label child. Group the stylesheet, manifest,
build rule, and loader in a buildable phase. Give each phase a build or
smoke check. Consider extending the existing resource bundle while
preserving the preset URI.
See lines 125-175.
[Current resource build](../cc-scout-gui/CMakeLists.txt#L20)

### A7. Documentation | Make risks, terms, and rationale easier to verify

Group the known failure modes and mitigations in the feature spec.
Keep the alternatives' concrete trade-offs and link the decision record
if retaining attribution. The structural reviewer flagged attribution,
while the design critic accepted the technical rationale. Use a short
example of mirrored pad events and a readout such as `CW +1`. Refer to
the Learn Pad reference in Settings and distinguish the Raw MIDI toggle
from the feed it shows. Describe verification in observable terms. The
sibling links work in the local layout and would need portable targets
if the document is shared outside it.
See lines 10-20, 40-43, 75-123, and 179-187.

## Traceability

| Stage | Fresh reviewer lenses | Consolidated actions |
| --- | --- | --- |
| 5 Initial Zoom Out | Zoom Out | A1, A3, A6 |
| 6 Comprehension | Comprehension, design only | A2, A7 |
| 7 Critic | Design, Structural, Style | A1, A2, A3, A4, A5, A7 |
| 8 Intermediate Zoom Out | Zoom Out | A1, A2, A3, A4, A5, A6 |
| 9 Implementation Readiness | Implementation Readiness, Codebase Details, Style | A1, A2, A3, A4, A5, A6, A7 |

All five stages completed. The review used the design and the approved
linked files, except the comprehension test, which read the design
alone. The findings come from document and source inspection. Hardware
timing and visual behavior remain to be verified.
