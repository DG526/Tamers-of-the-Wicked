#include "simple_logger.h"
#include "gf2d_font.h"
#include "totw_gui.h"
#include "totw_game_status.h"

typedef struct {
	Uint32 entityMax;
	GUI* entityList;
}GUIManager;

static GUIManager guiManager = { 0 };

void gui_manager_close() {
	if (guiManager.entityList)
		free(guiManager.entityList);
	memset(&guiManager, 0, sizeof(GUIManager));
	slog("GUI system closed.");
}

void gui_manager_init(Uint32 max) {
	if (max <= 0) {
		slog("Cannot initialize GUI system: < 1 GUI items specified!");
		return;
	}
	guiManager.entityList = gfc_allocate_array(sizeof(GUI), max);
	if (!guiManager.entityList) {
		slog("Failed to initialize GUI system!");
		return;
	}
	guiManager.entityMax = max;
	atexit(gui_manager_close);
	slog("GUI system initialized.");
}

GUI* gui_window_create(Vector2D position, Vector2D size, int layer) {
	WindowData* data = gfc_allocate_array(sizeof(WindowData), 1);
	if (!data) return NULL;
	Sprite* panel = gf2d_sprite_load_all("images/window_frame.png", 8, 8, 3, 0);
	if (size.x < panel->frame_w * 2) size.x = panel->frame_w * 2;
	if (size.y < panel->frame_h * 2) size.y = panel->frame_h * 2;
	vector2d_copy(data->proportions, size);
	for (int i = 0; i < guiManager.entityMax; i++) {
		if (guiManager.entityList[i].inuse) continue;
		guiManager.entityList[i].inuse = 1;
		guiManager.entityList[i].type = Window;
		vector2d_copy(guiManager.entityList[i].position, position);
		guiManager.entityList[i].data = data;
		guiManager.entityList[i].layer = layer;
		guiManager.entityList[i].sprite = panel;
		guiManager.entityList[i].color = gfc_color(1, 1, 1, 1);
		return &guiManager.entityList[i];
	}
	memset(data, 0, sizeof(WindowData));
	return NULL;
}
GUI* gui_text_create(Vector2D position, TextBlock text, Bool scrolling, int layer) {
	TextData* data = gfc_allocate_array(sizeof(TextData), 1);
	if (!data) return NULL;
	memcpy(data->text, text, sizeof(TextBlock));
	data->scrolling = scrolling;
	for (int i = 0; i < guiManager.entityMax; i++) {
		if (guiManager.entityList[i].inuse) continue;
		guiManager.entityList[i].inuse = 1;
		guiManager.entityList[i].type = Text;
		vector2d_copy(guiManager.entityList[i].position, position);
		guiManager.entityList[i].data = data;
		guiManager.entityList[i].layer = layer;
		guiManager.entityList[i].color = gfc_color(0, 0, 0, 1);
		return &guiManager.entityList[i];
	}
	memset(data, 0, sizeof(TextData));
	return NULL;
}
GUI* gui_option_create(Vector2D position, TextLine text, Bool isDefault, int layer) {
	OptionData* data = gfc_allocate_array(sizeof(OptionData), 1);
	if (!data) return NULL;
	Sprite* cursor = gf2d_sprite_load_all("images/selection_cursor.png", 8, 7, 2, 0);
	memcpy(data->text, text, sizeof(TextLine));
	data->selectedNow = isDefault;
	for (int i = 0; i < guiManager.entityMax; i++) {
		if (guiManager.entityList[i].inuse) continue;
		guiManager.entityList[i].inuse = 1;
		guiManager.entityList[i].type = Option;
		vector2d_copy(guiManager.entityList[i].position, position);
		guiManager.entityList[i].data = data;
		guiManager.entityList[i].layer = layer;
		guiManager.entityList[i].sprite = cursor;
		guiManager.entityList[i].color = gfc_color(0, 0, 0, 1);
		return &guiManager.entityList[i];
	}
	memset(data, 0, sizeof(OptionData));
	return NULL;
}
GUI* gui_letter_create(Vector2D position, char letter, char shiftLetter, Bool isDefault, TextLine* appendTo, int layer) {
	LetterData* data = gfc_allocate_array(sizeof(LetterData), 1);
	if (!data) return NULL;
	Sprite* cursor = gf2d_sprite_load_all("images/selection_cursor.png", 8, 7, 2, 0);
	data->letter = letter;
	data->shiftLetter = shiftLetter;
	data->appendTo = appendTo;
	data->selectedNow = isDefault;
	for (int i = 0; i < guiManager.entityMax; i++) {
		if (guiManager.entityList[i].inuse) continue;
		guiManager.entityList[i].inuse = 1;
		guiManager.entityList[i].type = Letter;
		vector2d_copy(guiManager.entityList[i].position, position);
		guiManager.entityList[i].data = data;
		guiManager.entityList[i].layer = layer;
		guiManager.entityList[i].sprite = cursor;
		guiManager.entityList[i].color = gfc_color(0, 0, 0, 1);
		return &guiManager.entityList[i];
	}
	memset(data, 0, sizeof(LetterData));
	return NULL;
}
GUI* gui_meter_create(Vector2D position, Vector2D size, Color color, int layer) {
	MeterData* data = gfc_allocate_array(sizeof(MeterData), 1);
	if (!data) return NULL;
	Sprite* frame = gf2d_sprite_load_all("images/bar_outline.png", 2, 2, 3, 0);
	Sprite* fill = gf2d_sprite_load_all("images/bar_fill.png", 4, 4, 1, 0);
	if (size.x < frame->frame_w * 2) size.x = frame->frame_w * 2;
	if (size.y < frame->frame_h * 2) size.y = frame->frame_h * 2;
	vector2d_copy(data->proportions, size);
	gfc_color_copy(data->barCol, color);
	data->fillBar = fill;
	for (int i = 0; i < guiManager.entityMax; i++) {
		if (guiManager.entityList[i].inuse) continue;
		guiManager.entityList[i].inuse = 1;
		guiManager.entityList[i].type = Meter;
		vector2d_copy(guiManager.entityList[i].position, position);
		guiManager.entityList[i].data = data;
		guiManager.entityList[i].layer = layer;
		guiManager.entityList[i].sprite = frame;
		return &guiManager.entityList[i];
	}
	memset(data, 0, sizeof(MeterData));
	return NULL;
}
GUI* gui_hint_create(Vector2D position, TextLine text, int icon, int layer) {
	ButtonHintData* data = gfc_allocate_array(sizeof(ButtonHintData), 1);
	if (!data) return NULL;
	Sprite* icons = gf2d_sprite_load_all("images/button_icons.png", 10, 10, 2, 0);
	data->icon = icon;
	memcpy(data->text, text, sizeof(TextLine));
	for (int i = 0; i < guiManager.entityMax; i++) {
		if (guiManager.entityList[i].inuse) continue;
		guiManager.entityList[i].inuse = 1;
		guiManager.entityList[i].type = ButtonHint;
		vector2d_copy(guiManager.entityList[i].position, position);
		guiManager.entityList[i].data = data;
		guiManager.entityList[i].layer = layer;
		guiManager.entityList[i].sprite = icons;
		guiManager.entityList[i].color = gfc_color(0, 0, 0, 1);
		return &guiManager.entityList[i];
	}
	memset(data, 0, sizeof(ButtonHintData));
	return NULL;
}

void gui_free(GUI* self) {
	if (!self) {
		slog("No entity provided for freeing.");
		return;
	}
	if (self->sprite)
		gf2d_sprite_free(self->sprite);
	switch (self->type) {
	case Window:
		memset(self->data, 0, sizeof(WindowData));
		break;
	case Text:
		memset(self->data, 0, sizeof(TextData));
		break;
	case Option:
		memset(self->data, 0, sizeof(OptionData));
		break;
	case Letter:
		memset(self->data, 0, sizeof(LetterData));
		break;
	case Meter:
		memset(self->data, 0, sizeof(MeterData));
		break;
	case Container:{
		int c = gfc_list_get_count(((SubmenuData*)(self->data))->elements);
		for (int i = 0; i < c; i++) {
			gui_free((GUI*)(gfc_list_get_nth(((SubmenuData*)(self->data))->elements, i)));
		}
		gfc_list_delete(((SubmenuData*)(self->data))->elements);
		memset(self->data, 0, sizeof(SubmenuData));
		break;
		}
	}
	memset(self, 0, sizeof(GUI));
}
void gui_free_all() {

	for (int i = 0; i < guiManager.entityMax; i++) {
		if (!guiManager.entityList[i].inuse) continue;
		gui_free(&(guiManager.entityList[i]));
	}
}

void gui_draw(GUI* self) {
	if (!self || !self->visible) return;
	switch (self->type) {
	case Window:
		if (self->sprite) {
			Vector2D cabs = vector2d(
				max(0, ((WindowData*)(self->data))->proportions.x - (self->sprite->frame_w * 2)),
				max(0, ((WindowData*)(self->data))->proportions.y - (self->sprite->frame_h * 2))
			);
			Vector2D c = vector2d(cabs.x / self->sprite->frame_w, cabs.y / self->sprite->frame_h);
			Vector2D tb = vector2d(c.x, 1);
			Vector2D lr = vector2d(1, c.y);
			//TL Corner
			gf2d_sprite_draw(
				self->sprite,
				self->position,
				NULL,
				NULL,
				NULL,
				NULL,
				&self->color,
				0
			);
			//Top
			gf2d_sprite_draw(
				self->sprite,
				vector2d(self->position.x + self->sprite->frame_w, self->position.y),
				&tb,
				NULL,
				NULL,
				NULL,
				&self->color,
				1
			);
			//TR Corner
			gf2d_sprite_draw(
				self->sprite,
				vector2d(self->position.x + self->sprite->frame_w + cabs.x, self->position.y),
				NULL,
				NULL,
				NULL,
				NULL,
				&self->color,
				2
			);
			//Left
			gf2d_sprite_draw(
				self->sprite,
				vector2d(self->position.x, self->position.y + self->sprite->frame_h),
				&lr,
				NULL,
				NULL,
				NULL,
				&self->color,
				3
			);
			//Center
			gf2d_sprite_draw(
				self->sprite,
				vector2d(self->position.x + self->sprite->frame_w, self->position.y + self->sprite->frame_h),
				&c,
				NULL,
				NULL,
				NULL,
				&self->color,
				4
			);
			//Right
			gf2d_sprite_draw(
				self->sprite,
				vector2d(self->position.x + self->sprite->frame_w + cabs.x, self->position.y + self->sprite->frame_h),
				&lr,
				NULL,
				NULL,
				NULL,
				&self->color,
				5
			);
			//BL Corner
			gf2d_sprite_draw(
				self->sprite,
				vector2d(self->position.x, self->position.y + self->sprite->frame_h + cabs.y),
				NULL,
				NULL,
				NULL,
				NULL,
				&self->color,
				6
			);
			//Bottom
			gf2d_sprite_draw(
				self->sprite,
				vector2d(self->position.x + self->sprite->frame_w, self->position.y + self->sprite->frame_h + cabs.y),
				&tb,
				NULL,
				NULL,
				NULL,
				&self->color,
				7
			);
			//BR Corner
			gf2d_sprite_draw(
				self->sprite,
				vector2d(self->position.x + self->sprite->frame_w + cabs.x, self->position.y + self->sprite->frame_h + cabs.y),
				NULL,
				NULL,
				NULL,
				NULL,
				&self->color,
				8
			);
		}
		break;
	case Text:
		if(!((TextData*)(self->data))->scrolling)
			gf2d_font_draw_line_tag(((TextData*)(self->data))->text, FT_Normal, self->color, self->position);
		else {
			TextBlock disp;
			gfc_block_cpy(disp, ((TextData*)(self->data))->text);
			disp[(int)(((TextData*)(self->data))->currentChar) + 1] = '\0';
			gf2d_font_draw_line_tag(disp, FT_Normal, self->color, self->position);
		}
		break;
	case Letter:
		if (self->sprite) {
			gf2d_sprite_draw(
				self->sprite,
				vector2d(self->position.x, self->position.y + 1),
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				(((LetterData*)(self->data))->selected) ? 1 : 0
			);
			TextWord display;
			sprintf(display, "%c", (((LetterData*)(self->data))->shifted ? ((LetterData*)(self->data))->shiftLetter : ((LetterData*)(self->data))->letter));
			gf2d_font_draw_line_tag(display, FT_Normal, self->color, vector2d(self->position.x + 10, self->position.y));
		}
		break;
	case Option:
		if (self->sprite) {
			float deg = 0;
			if (((OptionData*)(self->data))->isPointingUp) deg -= 90;
			if (((OptionData*)(self->data))->isPointingDown) deg += 90;
			gf2d_sprite_draw(
				self->sprite,
				vector2d(self->position.x, self->position.y + 1),
				NULL,
				NULL,
				&deg,
				NULL,
				NULL,
				(((OptionData*)(self->data))->selected) ? 1 : 0
			);
			gf2d_font_draw_line_tag(((OptionData*)(self->data))->text, FT_Normal, ((OptionData*)(self->data))->grayed ? gfc_color(0.5, 0.5, 0.5, 1) : self->color, vector2d(self->position.x + 10, self->position.y));;
		}
		break;
	case Meter:
		if (self->sprite) {
			Vector2D cabs = vector2d(
				max(0, ((MeterData*)(self->data))->proportions.x - (self->sprite->frame_w * 2)),
				max(0, ((MeterData*)(self->data))->proportions.y - (self->sprite->frame_h * 2))
			);
			Vector2D c = vector2d(cabs.x / self->sprite->frame_w, cabs.y / self->sprite->frame_h);
			Vector2D tb = vector2d(c.x, 1);
			Vector2D lr = vector2d(1, c.y);
			Vector2D f = vector2d(
				((((MeterData*)(self->data))->proportions.x - 2) / ((MeterData*)(self->data))->fillBar->frame_w) * SDL_clamp(((MeterData*)(self->data))->fill,0,1),
				(((MeterData*)(self->data))->proportions.y - 2) / ((MeterData*)(self->data))->fillBar->frame_h
				);
			//TL Corner
			gf2d_sprite_draw(
				self->sprite,
				self->position,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				0
			);
			//Top
			gf2d_sprite_draw(
				self->sprite,
				vector2d(self->position.x + self->sprite->frame_w, self->position.y),
				&tb,
				NULL,
				NULL,
				NULL,
				NULL,
				1
			);
			//TR Corner
			gf2d_sprite_draw(
				self->sprite,
				vector2d(self->position.x + self->sprite->frame_w + cabs.x, self->position.y),
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				2
			);
			//Left
			gf2d_sprite_draw(
				self->sprite,
				vector2d(self->position.x, self->position.y + self->sprite->frame_h),
				&lr,
				NULL,
				NULL,
				NULL,
				NULL,
				3
			);
			//Center
			gf2d_sprite_draw(
				self->sprite,
				vector2d(self->position.x + self->sprite->frame_w, self->position.y + self->sprite->frame_h),
				&c,
				NULL,
				NULL,
				NULL,
				NULL,
				4
			);
			//Right
			gf2d_sprite_draw(
				self->sprite,
				vector2d(self->position.x + self->sprite->frame_w + cabs.x, self->position.y + self->sprite->frame_h),
				&lr,
				NULL,
				NULL,
				NULL,
				NULL,
				5
			);
			//BL Corner
			gf2d_sprite_draw(
				self->sprite,
				vector2d(self->position.x, self->position.y + self->sprite->frame_h + cabs.y),
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				6
			);
			//Bottom
			gf2d_sprite_draw(
				self->sprite,
				vector2d(self->position.x + self->sprite->frame_w, self->position.y + self->sprite->frame_h + cabs.y),
				&tb,
				NULL,
				NULL,
				NULL,
				NULL,
				7
			);
			//BR Corner
			gf2d_sprite_draw(
				self->sprite,
				vector2d(self->position.x + self->sprite->frame_w + cabs.x, self->position.y + self->sprite->frame_h + cabs.y),
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				8
			);
			//FILL
			gf2d_sprite_draw(
				((MeterData*)(self->data))->fillBar,
				vector2d(self->position.x + 1, self->position.y + 1),
				&f,
				NULL,
				NULL,
				NULL,
				&(((MeterData*)(self->data))->barCol),
				0
			);
		}
		break;
	case ButtonHint:
		if (self->sprite) {
			gf2d_sprite_draw(
				self->sprite,
				vector2d(self->position.x, self->position.y + 1),
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				((ButtonHintData*)(self->data))->icon
			);
			gf2d_font_draw_line_tag(((OptionData*)(self->data))->text, FT_Normal, ((OptionData*)(self->data))->grayed ? gfc_color(0.5, 0.5, 0.5, 1) : self->color, vector2d(self->position.x + 10, self->position.y));;
		}
		break;
	}
}
void gui_draw_all() {
	for (int layer = 0; true; layer++) {
		int found = 0;
		for (int i = 0; i < guiManager.entityMax; i++) {
			if (!guiManager.entityList[i].inuse) continue;
			if (guiManager.entityList[i].layer != layer) continue;
			gui_draw(&(guiManager.entityList[i]));
			found = 1;
		}
		if (!found) break;
	}
}

void gui_update(GUI* self) {
	if (!self) return;
	if (self->type == Text) {
		if (((TextData*)(self->data))->text[(int)(((TextData*)(self->data))->currentChar + 0.4)] != '\0' && self->visible)
			((TextData*)(self->data))->currentChar += 0.4;
	}
	if (self->type == Option) {
		if (((OptionData*)(self->data))->selected && self->visible) {
			if (gfc_input_controller_button_pressed_by_index(0, 0)) {
				slog("Selected an option.");
				if(((OptionData*)(self->data))->onChoose)
					((OptionData*)(self->data))->onChoose(((OptionData*)(self->data))->choiceArgument);
				if (!self->inuse) return;
			}
			else {
				if (((OptionData*)(self->data))->down && gfc_input_controller_button_pressed(0, "D_D")) {
					if (((OptionData*)(self->data))->down->type == Option) {
						((OptionData*)(((OptionData*)(self->data))->down->data))->selectedNow = true;
						((OptionData*)(self->data))->selected = false;
					}
					else if (((OptionData*)(self->data))->down->type == Letter) {
						((LetterData*)(((OptionData*)(self->data))->down->data))->selectedNow = true;
						((OptionData*)(self->data))->selected = false;
					}
				}
				else if (((OptionData*)(self->data))->up && gfc_input_controller_button_pressed(0, "D_U")) {
					if (((OptionData*)(self->data))->up->type == Option) {
						((OptionData*)(((OptionData*)(self->data))->up->data))->selectedNow = true;
						((OptionData*)(self->data))->selected = false;
					}
					else if (((OptionData*)(self->data))->up->type == Letter) {
						((LetterData*)(((OptionData*)(self->data))->up->data))->selectedNow = true;
						((OptionData*)(self->data))->selected = false;
					}
				}
				else if (((OptionData*)(self->data))->left && gfc_input_controller_button_pressed(0, "D_L")) {
					if (((OptionData*)(self->data))->left->type == Option) {
						((OptionData*)(((OptionData*)(self->data))->left->data))->selectedNow = true;
						((OptionData*)(self->data))->selected = false;
					}
					else if (((OptionData*)(self->data))->left->type == Letter) {
						((LetterData*)(((OptionData*)(self->data))->left->data))->selectedNow = true;
						((OptionData*)(self->data))->selected = false;
					}
				}
				else if (((OptionData*)(self->data))->right && gfc_input_controller_button_pressed(0, "D_R")) {
					if (((OptionData*)(self->data))->right->type == Option) {
						((OptionData*)(((OptionData*)(self->data))->right->data))->selectedNow = true;
						((OptionData*)(self->data))->selected = false;
					}
					else if (((OptionData*)(self->data))->right->type == Letter) {
						((LetterData*)(((OptionData*)(self->data))->right->data))->selectedNow = true;
						((OptionData*)(self->data))->selected = false;
					}
				}
			}
		}
		if (((OptionData*)(self->data))->selectedNow && self->visible) {
			if (((OptionData*)(self->data))->onHover) ((OptionData*)(self->data))->onHover(((OptionData*)(self->data))->hoverArgument);
			((OptionData*)(self->data))->selectedNow = false;
			((OptionData*)(self->data))->selected = true;
		}
	}
	if (self->type == Letter) {
		if (((LetterData*)(self->data))->selected && self->visible) {
			if (gfc_input_controller_button_pressed_by_index(0, 0)) {
				slog("Selected a letter.");
				for (int i = 0; i < 10; i++) {
					if ((*(((LetterData*)(self->data))->appendTo))[i] == '\0') {
						char newChar = (((LetterData*)(self->data))->shifted ? ((LetterData*)(self->data))->shiftLetter : ((LetterData*)(self->data))->letter);
						TextLine newName;
						sprintf(newName, "%s%c", *(((LetterData*)(self->data))->appendTo), newChar);
						gfc_line_cpy((((LetterData*)(self->data))->appendTo), newName);
						//memcpy(&(*(((LetterData*)(self->data))->appendTo)[i]), &newChar, sizeof(char));
						//*(((LetterData*)(self->data))->appendTo)[i] = newChar;
						//char end = '\0';
						//memcpy(&(*(((LetterData*)(self->data))->appendTo)[i+1]), &end, sizeof(char));
						//*(((LetterData*)(self->data))->appendTo)[i + 1] = '\0';
						slog("Appended character at position %i. Name is now %s.", i, *(((LetterData*)(self->data))->appendTo));
						break;
					}
				}
			}
			else {
				if (((LetterData*)(self->data))->down && gfc_input_controller_button_pressed(0, "D_D")) {
					if (((LetterData*)(self->data))->down->type == Option) {
						((OptionData*)(((LetterData*)(self->data))->down->data))->selectedNow = true;
						((LetterData*)(self->data))->selected = false;
					}
					else if (((LetterData*)(self->data))->down->type == Letter) {
						((LetterData*)(((LetterData*)(self->data))->down->data))->selectedNow = true;
						((LetterData*)(self->data))->selected = false;
					}
				}
				else if (((LetterData*)(self->data))->up && gfc_input_controller_button_pressed(0, "D_U")) {
					if (((LetterData*)(self->data))->up->type == Option) {
						((OptionData*)(((LetterData*)(self->data))->up->data))->selectedNow = true;
						((LetterData*)(self->data))->selected = false;
					}
					else if (((LetterData*)(self->data))->up->type == Letter) {
						((LetterData*)(((LetterData*)(self->data))->up->data))->selectedNow = true;
						((LetterData*)(self->data))->selected = false;
					}
				}
				else if (((LetterData*)(self->data))->left && gfc_input_controller_button_pressed(0, "D_L")) {
					if (((LetterData*)(self->data))->left->type == Option) {
						((OptionData*)(((LetterData*)(self->data))->left->data))->selectedNow = true;
						((LetterData*)(self->data))->selected = false;
					}
					else if (((LetterData*)(self->data))->left->type == Letter) {
						((LetterData*)(((LetterData*)(self->data))->left->data))->selectedNow = true;
						((LetterData*)(self->data))->selected = false;
					}
				}
				else if (((LetterData*)(self->data))->right && gfc_input_controller_button_pressed(0, "D_R")) {
					if (((LetterData*)(self->data))->right->type == Option) {
						((OptionData*)(((LetterData*)(self->data))->right->data))->selectedNow = true;
						((LetterData*)(self->data))->selected = false;
					}
					else if (((LetterData*)(self->data))->right->type == Letter) {
						((LetterData*)(((LetterData*)(self->data))->right->data))->selectedNow = true;
						((LetterData*)(self->data))->selected = false;
					}
				}
			}
		}
		if (((LetterData*)(self->data))->selectedNow && self->visible) {
			((LetterData*)(self->data))->selectedNow = false;
			((LetterData*)(self->data))->selected = true;
		}
	}
	if (self->update) self->update(self);
}
void gui_update_all() {

	for (int i = 0; i < guiManager.entityMax; i++) {
		if (!guiManager.entityList[i].inuse) continue;
		gui_update(&(guiManager.entityList[i]));
	}
}

void window_close_on_cross(GUI* self) {
	if (gfc_input_controller_button_pressed_by_index(0, 0)) {
		if (((SubmenuData*)(self->data))->returnToOnClose) {
			if (((SubmenuData*)(self->data))->returnToOnClose->type == Option)
				((OptionData*)(((SubmenuData*)(self->data))->returnToOnClose->data))->selectedNow = true;
			else if (((SubmenuData*)(self->data))->returnToOnClose->type == Letter)
				((LetterData*)(((SubmenuData*)(self->data))->returnToOnClose->data))->selectedNow = true;
		}
		gui_free(self);
	}
}
GUI* gui_error_create(TextLine message, int layer, GUI* returnTo) {
	gfc_input_update();
	SubmenuData* data = gfc_allocate_array(sizeof(SubmenuData), 1);
	GUI* window = gui_window_create(vector2d(game_get_resolution_x() / 2 - 50, game_get_resolution_y() / 2 - 25), vector2d(100, 50), layer);
	window->color = gfc_color8(255, 107, 107, 255);
	window->visible = true;
	GUI* text = gui_text_create(vector2d(game_get_resolution_x() / 2 - 45, game_get_resolution_y() / 2 - 20), message, false, layer + 1);
	text->visible = true;
	data->elements = gfc_list_new();
	gfc_list_append(data->elements, window);
	gfc_list_append(data->elements, text);
	data->returnToOnClose = returnTo;
	for (int i = 0; i < guiManager.entityMax; i++) {
		if (guiManager.entityList[i].inuse) continue;
		guiManager.entityList[i].inuse = 1;
		guiManager.entityList[i].type = Container;
		guiManager.entityList[i].data = data;
		guiManager.entityList[i].layer = 0;
		guiManager.entityList[i].update = window_close_on_cross;
		return &guiManager.entityList[i];
	}
}