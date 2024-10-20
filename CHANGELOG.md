Changelog
=========

### Crystal Tracker v0.8.3 (2024-10-20)

 * Add Move Left/Right for loop and call boxes.
 * Add warning for songs that desync badly.

### Crystal Tracker v0.8.2 (2024-10-16)

 * Add Bookmarks.
 * Home/End now scroll to first/last selected note.
 * Improve drawing on scaled displays.

### Crystal Tracker v0.8.1 (2024-09-08)

 * Minor bug fixes.

### Crystal Tracker v0.8.0 (2024-08-31)

 * Add Format Painter: Copy note properties from one note to another.
 * Add full support for stereo panning command.
 * Add BPM display to status bar.
 * Add Duplicate Note to Edit menu.
 * Add Ctrl+F3 for dumping .it file.
 * Add Ctrl+\ for centering the playhead in the middle of the screen.
 * Add confirmation dialog for clearing recent songs.
 * Pencil icon now matches selected channel color.
 * Slightly emphasize beat lines when ruler is active.
 * Increase max grid width to 48.
 * Selecting notes with Enter key: Select the note to the left of the playhead if Alt is also pressed.
 * Keep Skip Backward/Forward enabled while the song is playing, which now act like rewind and fast-forward.
 * Minor bug fixes.

### Crystal Tracker v0.7.0 (2024-06-08)

 * Better Pencil: Click and drag to pick note length.
 * Better tempo handling:
   * Visualize all tempo changes with dark purple or dark yellow lines.
   * Automatically split rests according to first channel tempo changes.
   * Disable tempo property input box for non-first channels.
   * Add Postprocess Channel to Edit menu to trigger automatic rest splitting on-demand.
 * Better Measure Ruler:
   * Add ruler config dialog to set the time signature of the ruler to match the song, including pickup notes.
   * Remember ruler config for recent files.
   * Increase max grid width from 16 to 32.
 * Better toolbar:
   * Add loop verification toolbar button.
   * Add toolbar buttons for moving/resizing notes.
   * Add Delete, Snip, Split, and Glue to toolbar.
 * Better zoom: Add extra zoom-out zoom level.
 * Better playback: Slightly improve vibrato playback for slow tempos.
 * Better editing: Add Insert Rest to Edit menu; useful for widening inner loops and calls.
 * Update to FLTK 1.4-alpha.

### Crystal Tracker v0.6.3 (2024-05-19)

 * Fix a few possible deadlocks on Mac and Windows.

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
