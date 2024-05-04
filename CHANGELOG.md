Changelog
=========

### Crystal Tracker v0.6.2 (2024-05-03)

 * Bugfix: Calls can no longer be created across the main loop line.
 * Bugfix: When creating a loop, the octave of the notes *after* the loop is now always correctly preserved.
 * Minor file parsing fixes/improvements.

### Crystal Tracker v0.6.1 (2024-02-12)

 * Bugfix: Calls containing loops cannot be inserted into other loops.

### Crystal Tracker v0.6.0 (2024-02-11)

 * Add right-click context menu with loop and call functions.
   * Loops: Reduce, Extend, Unroll, Create
   * Calls: Delete, Unpack, Create, Insert
 * Add note label toggle.
 * Add key label toggle, note label toggle, and ruler toggle to toolbar.
 * Shorten and Lengthen now drag the cursor if the cursor is aligned to the right edge of a selected note.
 * Clicking into the measure ruler now sets the playhead.
 * Placing new notes now copies the speed of the previous note when possible.
 * Note properties panel now supports alt shortcut keyboard navigation.
 * Add Select Invert function.
 * Unreferenced labels are now visualized in the timeline with a gray line.
 * Add gray flag for misc/other settings changes.
 * Add warning when labels are used by multiple channels.
 * Add error when loading a song that contains unsupported rgbds keywords (eg, rept).
 * Add warning when note properties differ on the second iteration of the main loop.
 * Add menu option checkbox to disable main loop verification.
 * Increase max undo limit from 100 to 256.

### Crystal Tracker v0.5.0 (2024-01-08)

 * Add song resizing.
 * Allow specifying song lengths in beats instead of ticks.
 * New app icon.

### Crystal Tracker v0.4.0 (2023-12-14)

 * Add rectangle select.
 * Improve tempo to bpm approximation for playback.
 * Improve playback of noise samples that never fade out. (See: Pinball's seelstage.asm)
 * More intuitive keyboard controls for the Note Properties panel.
   * Escape now aborts note property changes.
   * Enter now also deselects the textbox.
 * Fix playback of drum samples for songs that also use inline waves.
 * Warn for songs that use too many inline waves.
 * Using too many drums is now a warning and not an error.
 * Allow setting drumkit from Note Properties panel.

### Crystal Tracker v0.3.0 (2023-05-28)

 * Improve accuracy of volume fade during playback.
 * Add 64-bit build on Windows.
 * New app icon. Thanks to nyanpasu64 for the pixelart versions of the icon.

### Crystal Tracker v0.2.1 (2023-03-19)

 * Slightly improved quality of channel 4 playback.
 * drumkits.asm supports `dr` as well as `dw`.

### Crystal Tracker v0.2.0 (2023-03-18)

 * Parse drumkits and synthesize channel 4 playback.
 * Validate usage of 0-arg and 1-arg `toggle_noise` commands.
 * Pressing Escape to deselect a textbox no longer also resets the playhead.

### Crystal Tracker v0.1.0 (2023-03-05)

 * Initial release.
