#ifndef __GUI_H__
#define __GUI_H__

#include "gfc_input.h"
#include "gfc_types.h"

#include "gf2d_sprite.h"
#include "gf2d_draw.h"

typedef enum {
	Window,
	Text,
	Option,
	Letter
}GUIType;

typedef struct GUI_S {
	Bool inuse;
	Sprite* sprite;
	Vector2D position;
	GUIType type;
	int layer;
	void* data;
	void (*update)(struct GUI_S* self);
}GUI;

typedef struct {
	Vector2D proportions;
}WindowData;
typedef struct {
	TextBlock text;
	Bool scrolling;
	float currentChar;
	void (*onDone)();
}TextData;
typedef struct {
	TextBlock text;
	GUI* left, * right, * up, * down;
	Bool selected;
	void (*onSelect)();
}OptionData;
typedef struct {
	TextBlock text;
	GUI* left, * right, * up, * down;
	char letter;
	Bool selected;
	TextWord* appendTo;
}LetterData;

void gui_manager_init(Uint32 max);

GUI* gui_window_create(Vector2D position, Vector2D size, int layer, Sprite* panel);
GUI* gui_text_create(Vector2D position, TextBlock text, Bool scrolling, int layer);
GUI* gui_option_create(Vector2D position, TextBlock text, Bool isDefault, int layer, Sprite* cursor);
GUI* gui_letter_create(Vector2D position, char letter, Bool isDefault, TextWord* appendTo, int layer, Sprite* cursor);

void gui_draw(GUI* self);
void gui_draw_all();

void gui_update(GUI* self);
void gui_update_all();

void gui_free(GUI* self);
void gui_free_all();


#endif