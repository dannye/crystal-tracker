#ifndef OPTION_DIALOGS_H
#define OPTION_DIALOGS_H

#include "widgets.h"

#pragma warning(push, 0)
#include <FL/Fl_Double_Window.H>
#pragma warning(pop)

class Option_Dialog {
protected:
	int _width;
	const char *_title;
	bool _has_reset;
	bool _canceled;
	Fl_Double_Window *_dialog;
	Fl_Group *_content;
	Default_Button *_ok_button;
	OS_Button *_cancel_button;
	OS_Button *_reset_button;
public:
	Option_Dialog(int w, const char *t = NULL);
	virtual ~Option_Dialog();
	inline bool canceled(void) const { return _canceled; }
	inline void canceled(bool c) { _canceled = c; }
protected:
	void initialize(void);
	void refresh(bool reset);
	virtual void initialize_content(void) = 0;
	virtual int refresh_content(int ww, int dy, bool reset) = 0;
	virtual void set_reset_cb() {}
	virtual void finish(void) {}
public:
	inline bool initialized(void) const { return !!_dialog; }
	void show(Fl_Widget *p, bool reset = true);
private:
	static void close_cb(Fl_Widget *, Option_Dialog *od);
	static void cancel_cb(Fl_Widget *, Option_Dialog *od);
};

class Song_Options_Dialog : public Option_Dialog {
public:
	enum class Result { RESULT_OK, RESULT_BAD_TITLE, RESULT_BAD_END, RESULT_BAD_LOOP };
	struct Song_Options {
		std::string song_name;
		bool looping;
		bool channel_1;
		bool channel_2;
		bool channel_3;
		bool channel_4;
		int32_t channel_1_loop_tick;
		int32_t channel_2_loop_tick;
		int32_t channel_3_loop_tick;
		int32_t channel_4_loop_tick;
		int32_t channel_1_end_tick;
		int32_t channel_2_end_tick;
		int32_t channel_3_end_tick;
		int32_t channel_4_end_tick;
		Result result;
	};
private:
	OS_Input *_song_name = nullptr;
	OS_Check_Button *_looping_checkbox = nullptr;
	OS_Check_Button *_channel_1_checkbox = nullptr;
	OS_Check_Button *_channel_2_checkbox = nullptr;
	OS_Check_Button *_channel_3_checkbox = nullptr;
	OS_Check_Button *_channel_4_checkbox = nullptr;
	OS_Int_Input *_channel_1_loop_tick = nullptr;
	OS_Int_Input *_channel_2_loop_tick = nullptr;
	OS_Int_Input *_channel_3_loop_tick = nullptr;
	OS_Int_Input *_channel_4_loop_tick = nullptr;
	OS_Int_Input *_channel_1_end_tick = nullptr;
	OS_Int_Input *_channel_2_end_tick = nullptr;
	OS_Int_Input *_channel_3_end_tick = nullptr;
	OS_Int_Input *_channel_4_end_tick = nullptr;
	OS_Check_Button *_synchronize_checkbox = nullptr;
	OS_Radio_Button *_beats_radio = nullptr;
	OS_Radio_Button *_ticks_radio = nullptr;
public:
	Song_Options_Dialog(const char *t);
	~Song_Options_Dialog();
	Song_Options get_options();
	void set_options(const Song_Options &options);
	const char *get_error_message(Result r);
protected:
	void initialize_content(void);
	int refresh_content(int ww, int dy, bool reset);
private:
	static void looping_checkbox_cb(OS_Check_Button *c, Song_Options_Dialog *sod);
	static void channel_checkbox_cb(OS_Check_Button *c, Song_Options_Dialog *sod);
	static void channel_loop_tick_cb(OS_Int_Input *i, Song_Options_Dialog *sod);
	static void channel_end_tick_cb(OS_Int_Input *i, Song_Options_Dialog *sod);
	static void synchronize_checkbox_cb(OS_Check_Button *c, Song_Options_Dialog *sod);
	static void beats_ticks_radio_cb(OS_Radio_Button *r, Song_Options_Dialog *sod);
};

class Ruler_Config_Dialog : public Option_Dialog {
public:
	struct Ruler_Options {
		int beats_per_measure = 4;
		int steps_per_beat = 4;
		int pickup_offset = 0;
	};
private:
	OS_Spinner *_beats_per_measure = nullptr;
	OS_Spinner *_steps_per_beat = nullptr;
	OS_Spinner *_pickup_offset = nullptr;
public:
	Ruler_Config_Dialog(const char *t);
	~Ruler_Config_Dialog();
	Ruler_Options get_options();
	void set_options(const Ruler_Options &options);
protected:
	void initialize_content(void);
	int refresh_content(int ww, int dy, bool reset);
	void set_reset_cb();
private:
	static void ruler_config_cb(Fl_Widget *w, Ruler_Config_Dialog *rcd);
	static void reset_button_cb(Fl_Widget *w, Ruler_Config_Dialog *rcd);
};

#endif
