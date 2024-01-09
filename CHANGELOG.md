Changelog
=========

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
