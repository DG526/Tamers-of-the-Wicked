#ifndef __GUI_H__
#define __GUI_H__

#include "gfc_input.h"
#include "gfc_types.h"
#include "gfc_color.h"

#include "gf2d_sprite.h"
#include "gf2d_draw.h"

typedef enum {
	Window,
	Text,
	Option,
	Letter,
	PageList,
	Meter,
	ButtonHint,
	Container,
	SpriteGUI
}GUIType;

typedef struct GUI_S {
	Bool inuse;
	Bool visible;
	Sprite* sprite;
	Vector2D position;
	GUIType type;
	Color color;
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
	TextLine text;
	GUI* left, * right, * up, * down;
	Bool isPointingUp, isPointingDown;
	Bool isCurrentlyActive;
	Bool grayed;
	Bool selectedNow;
	Bool selected;
	void* hoverArg1; //Argument passed to onHover()
	void* hoverArg2, * hoverArg3;
	void* choiceArg1; //Argument passed to onChoose()
	void* choiceArg2, * choiceArg3;
	//int hoverArgs, choiceArgs;
	int (*onHover)(void*, ...);
	int (*onChoose)(void* arg, ...);
}OptionData;
typedef struct {
	Bool isCurrentlyActive;
	int maxLinesPerPage, currentPage, pageCount;
	int items;
	int selectedItemIndex;
	List* options;
	float rightArrowRelPos;
	float arrowYOffset;
}PageListData;
typedef struct {
	TextBlock text;
	GUI* left, * right, * up, * down;
	char letter, shiftLetter;
	Bool shifted, selected, selectedNow;
	TextLine* appendTo;
}LetterData;
typedef struct {
	Vector2D proportions;
	Sprite* fillBar;
	Color barCol;
	float fill;
}MeterData;
typedef struct {
	TextLine text;
	Sprite* icons;
	int icon;
}ButtonHintData;


void gui_manager_init(Uint32 max);

GUI* gui_window_create(Vector2D position, Vector2D size, int layer);
GUI* gui_text_create(Vector2D position, TextBlock text, Bool scrolling, int layer);
GUI* gui_option_create(Vector2D position, TextLine text, Bool isDefault, int layer);
GUI* gui_letter_create(Vector2D position, char letter, char shiftLetter, Bool isDefault, TextLine* appendTo, int layer);
GUI* gui_meter_create(Vector2D position, Vector2D size, Color color, int layer);
GUI* gui_sprite_create(Vector2D position, Sprite* sprite, Color color, int layer);
GUI* gui_page_list_create(Vector2D position, int itemCount, int maxPerPage, float rightArrowXPos, int layer);
void gui_page_list_refresh(GUI* pageList);
void gui_page_list_refresh_visibility(GUI* pageList);
/**
*@param position: where to put the hint
*@param text: what the hint is for
*@param icon: 0 = cross, 1 = square, 2 = triangle, 3 = circle
*@param layer: render order
*/
GUI* gui_hint_create(Vector2D position, TextLine text, int icon, int layer);

void gui_draw(GUI* self);
void gui_draw_all();

void gui_update(GUI* self);
void gui_update_all();

void gui_free(GUI* self);
void gui_free_all();

typedef struct {
	List* elements;
	GUI* returnToOnClose;
	int layer;
}SubmenuData;

GUI* gui_error_create(TextLine message, int layer, GUI* returnTo);

#endif