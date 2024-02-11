Music_CrystalTracked:
	channel_count 3
	channel 1, Music_CrystalTracked_Ch1
	channel 2, Music_CrystalTracked_Ch2
	channel 3, Music_CrystalTracked_Ch3

Music_CrystalTracked_Ch1:
	tempo 256
	volume 7, 7
	note_type 12, 15, 8
.mainLoop:
	duty_cycle 0
	note_type 12, 10, 8
	octave 3
	tempo 230
	vibrato 0, 0, 0
	note G#, 1
	rest 1
	note G#, 1
	rest 2
	note A#, 3
	octave 4
	note C_, 4
	octave 3
	rest 2
	note D#, 2
	note G#, 1
	rest 1
	note A#, 1
	rest 1
	note G#, 1
	rest 2
	octave 2
	duty_cycle 3
	note D#, 1
	note G#, 1
	note B_, 1
	octave 3
	note C_, 1
	octave 2
	note A#, 1
	note G#, 1
	rest 1
	pitch_slide 1, 4, C_
	duty_cycle 0
	note G#, 2
	octave 3
	note G#, 1
	rest 1
	note G#, 1
	rest 2
	note A#, 3
	octave 4
	note C_, 3
	rest 2
	octave 3
	duty_cycle 3
	note F_, 1
	note G#, 1
	note A#, 1
	octave 4
	note C_, 1
	note D#, 1
	note C_, 1
	octave 3
	rest 1
	pitch_slide 1, 2, C_
	duty_cycle 0
	note G#, 2
	octave 2
	note G#, 2
	note_type 8, 10, 8
	note G_, 4
	note F_, 4
	pitch_slide 2, 4, G_
	note C#, 4
	note_type 12, 10, 8
	octave 4
	sound_call .sub1
	rest 3
	duty_cycle 2
	note E_, 1
	note C_, 1
	note F_, 1
	note E_, 1
	note C_, 1
	rest 1
	octave 3
	note F_, 1
	note E_, 1
	octave 4
	duty_cycle 0
	note G_, 2
	vibrato 0, 1, 5
	volume_envelope 6, 8
	note G#, 6
	note G_, 2
	note F_, 4
	tempo 243
	note E_, 8
	octave 3
	note G_, 4
	tempo 256
	note B_, 2
	tempo 286
	note B_, 2
	octave 4
	tempo 354
	note C_, 2
	tempo 440
	note D_, 2
	volume_envelope 5, 8
	tempo 230
	note G#, 1
	rest 1
	note G#, 1
	rest 2
	note A#, 3
	octave 5
	note C_, 4
	rest 2
	octave 4
	note D#, 2
	note G#, 1
	note A#, 1
	note G#, 1
	rest 1
	octave 3
	note G#, 1
	note B_, 1
	octave 4
	note C_, 1
	note D#, 1
	note G#, 1
	rest 1
	note G#, 1
	rest 2
	note A#, 3
	octave 5
	note C_, 2
	rest 1
	octave 3
	duty_cycle 1
	note G#, 1
	octave 4
	note C_, 1
	note D#, 1
	octave 5
	note C_, 1
	note D#, 1
	note C_, 4
	rest 2
	duty_cycle 0
	note C_, 1
	note D#, 1
	vibrato 0, 4, 3
	note C_, 6
	octave 2
	duty_cycle 2
	note G#, 2
	note_type 8, 5, 8
	note G_, 4
	note F_, 4
	vibrato 0, 2, 3
	octave 1
	pitch_slide 3, 4, B_
	note B_, 4
	note_type 12, 5, 8
	octave 4
	sound_call .sub1
	note_type 12, 5, 8
	octave 4
	rest 5
	note D_, 1
	note D#, 1
	note D_, 1
	rest 1
	note B_, 1
	rest 1
	octave 3
	note G_, 1
	rest 1
	vibrato 0, 4, 3
	duty_cycle 3
	note G#, 6
	note G_, 2
	note F_, 4
	note E_, 6
	tempo 243
	note E_, 2
	octave 2
	note G_, 4
	tempo 256
	note G#, 2
	tempo 286
	note B_, 2
	tempo 354
	note A#, 2
	octave 3
	tempo 500
	note C#, 2
	pitch_slide 1, 3, G#
	octave 8
	sound_loop 0, .mainLoop

.sub1:
	note B_, 1
	note G_, 1
	octave 5
	note C_, 1
	octave 4
	note B_, 1
	note G_, 1
	note E_, 1
	note C_, 1
	octave 3
	note G_, 1
	octave 4
	note C_, 1
	rest 1
	note D_, 1
	rest 1
	octave 3
	note B_, 1
	octave 4
	note C_, 1
	note D_, 1
	note E_, 1
	note D_, 1
	rest 1
	note B_, 1
	sound_ret

Music_CrystalTracked_Ch2:
	note_type 12, 15, 8
.mainLoop:
	volume_envelope 15, 8
	octave 2
	duty_cycle 1
	vibrato 0, 2, 2
	sound_call .sub1
	note A_, 2
	octave 3
	note C_, 2
	note F_, 1
	rest 1
	note C_, 1
	octave 2
	rest 1
	note F_, 2
	note G_, 2
	note A_, 1
	rest 1
	note G_, 1
	rest 1
	sound_call .sub2
	note C_, 2
	note G_, 2
	vibrato 0, 5, 1
	note G#, 2
	octave 3
	note C_, 2
	octave 1
	volume_envelope 10, 8
	note G#, 2
	octave 2
	note D#, 2
	volume_envelope 13, 8
	note G#, 2
	octave 3
	note C_, 2
	volume_envelope 15, 8
	sound_call .sub3
	rest 2
	octave 2
	volume_envelope 12, 8
	note D#, 2
	note G_, 2
	octave 3
	volume_envelope 15, 8
	note C_, 2
	note D#, 2
	note D_, 2
	note C_, 2
	octave 2
	note B_, 2
	vibrato 0, 1, 1
	duty_cycle 2
	volume_envelope 10, 8
	sound_call .sub1
	octave 2
	note_type 12, 10, 8
	note G#, 2
	octave 3
	note D#, 2
	note F_, 1
	rest 1
	note C_, 1
	octave 2
	rest 1
	note F_, 2
	note G_, 2
	note A_, 1
	rest 1
	note G_, 1
	rest 1
	sound_call .sub2
	octave 2
	note_type 12, 10, 8
	note C_, 2
	note G_, 2
	note G#, 1
	rest 1
	note F#, 1
	octave 1
	rest 1
	duty_cycle 0
	note G#, 2
	octave 2
	note D#, 2
	note G#, 2
	octave 3
	note C_, 2
	sound_call .sub3
	note_type 12, 10, 8
	rest 2
	octave 2
	note D#, 2
	note G_, 2
	octave 3
	note C_, 2
	note E_, 2
	note D#, 2
	note C#, 2
	octave 2
	note A#, 2
	octave 8
	sound_loop 0, .mainLoop

.sub1:
	note G#, 2
	octave 3
	note C_, 2
	note D#, 1
	rest 1
	note C_, 1
	octave 2
	rest 1
	note G#, 2
	octave 3
	note D#, 2
	note F_, 1
	rest 1
	note C_, 1
	octave 2
	rest 1
	note G#, 2
	octave 3
	note C_, 2
	note D#, 1
	rest 1
	note C_, 1
	octave 2
	rest 1
	note G#, 2
	octave 3
	note D#, 2
	note G#, 1
	rest 1
	note C_, 1
	octave 2
	rest 1
	note G#, 2
	octave 3
	note C_, 2
	note F_, 1
	rest 1
	note C_, 1
	octave 2
	rest 1
	note F_, 2
	octave 3
	note C_, 2
	note F_, 1
	rest 1
	octave 2
	note B_, 1
	rest 1
	sound_ret

.sub2:
	note C_, 2
	note G_, 2
	note B_, 1
	rest 1
	note G_, 1
	rest 1
	note C_, 2
	note G_, 2
	note A#, 1
	rest 1
	note G_, 1
	rest 1
	note C_, 2
	note G_, 2
	note A_, 1
	rest 1
	note G_, 1
	rest 1
	sound_ret

.sub3:
	note G#, 2
	note G_, 2
	note F_, 2
	note E_, 2
	sound_ret

Music_CrystalTracked_Ch3:
	note_type 12, 1, 0
.mainLoop:
	octave 2
	note_type 12, 1, 7
	sound_call .sub1
	note_type 8, 1, 7
	note G_, 4
	note F_, 4
	note C#, 4
	note_type 12, 1, 7
.loop1:
	note C_, 8
	note G_, 2
	note F_, 2
	note E_, 1
	rest 3
	sound_loop 2, .loop1
	octave 2
	volume_envelope 1, 4
	note C_, 8
	octave 1
	note G_, 2
	note F_, 2
	note C_, 8
	note D_, 2
	note G#, 2
	note G#, 2
	note G_, 2
	note G_, 2
	note G_, 2
	octave 2
	volume_envelope 1, 6
	sound_call .sub1
	octave 1
	note_type 8, 1, 6
	note G_, 4
	note F_, 4
	note C#, 4
	note_type 12, 1, 7
.loop2:
	note C_, 8
	note G_, 2
	note F_, 2
	note E_, 1
	rest 3
	sound_loop 2, .loop2
	volume_envelope 1, 8
	note C_, 1
	rest 1
	note G#, 1
	rest 1
	octave 2
	note D#, 1
	rest 1
	note G#, 1
	rest 9
	octave 1
	note C_, 1
	rest 1
	note G#, 1
	rest 1
	octave 2
	note D#, 1
	rest 1
	note G_, 1
	rest 1
	note E_, 2
	note E_, 2
	note F#, 2
	note F#, 2
	octave 8
	sound_loop 0, .mainLoop

.sub1:
	note G#, 12
	note D#, 4
	note G#, 8
	note F#, 8
	note F_, 4
	note C_, 8
	octave 1
	note G#, 2
	note G_, 2
	note F_, 6
	note G#, 2
	sound_ret
