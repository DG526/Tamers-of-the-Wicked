#include "simple_logger.h"
#include "gf2d_font.h"
#include "totw_gui.h"

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

GUI* gui_window_create(Vector2D position, Vector2D size, int layer, Sprite* panel) {
	WindowData* data = gfc_allocate_array(sizeof(WindowData), 1);
	if (!data) return NULL;
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
		return &guiManager.entityList[i];
	}
	memset(data, 0, sizeof(TextData));
	return NULL;
}
GUI* gui_option_create(Vector2D position, TextBlock text, Bool isDefault, int layer, Sprite* cursor) {
	OptionData* data = gfc_allocate_array(sizeof(OptionData), 1);
	if (!data) return NULL;
	memcpy(data->text, text, sizeof(TextBlock));
	data->selected = isDefault;
	for (int i = 0; i < guiManager.entityMax; i++) {
		if (guiManager.entityList[i].inuse) continue;
		guiManager.entityList[i].inuse = 1;
		guiManager.entityList[i].type = Option;
		vector2d_copy(guiManager.entityList[i].position, position);
		guiManager.entityList[i].data = data;
		guiManager.entityList[i].layer = layer;
		guiManager.entityList[i].sprite = cursor;
		return &guiManager.entityList[i];
	}
	memset(data, 0, sizeof(OptionData));
	return NULL;
}
GUI* gui_letter_create(Vector2D position, char letter, Bool isDefault, TextWord* appendTo, int layer, Sprite* cursor) {
	LetterData* data = gfc_allocate_array(sizeof(LetterData), 1);
	if (!data) return NULL;
	data->letter = letter;
	data->appendTo = appendTo;
	data->selected = isDefault;
	for (int i = 0; i < guiManager.entityMax; i++) {
		if (guiManager.entityList[i].inuse) continue;
		guiManager.entityList[i].inuse = 1;
		guiManager.entityList[i].type = Letter;
		vector2d_copy(guiManager.entityList[i].position, position);
		guiManager.entityList[i].data = data;
		guiManager.entityList[i].layer = layer;
		guiManager.entityList[i].sprite = cursor;
		return &guiManager.entityList[i];
	}
	memset(data, 0, sizeof(LetterData));
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
	if (!self) return;
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
		}
		break;
	case Text:
		if(!((TextData*)(self->data))->scrolling)
			gf2d_font_draw_line_tag(((TextData*)(self->data))->text, FT_Normal, gfc_color(0, 0, 0, 1), self->position);
		else {
			TextBlock disp;
			gfc_block_cpy(disp, ((TextData*)(self->data))->text);
			disp[(int)(((TextData*)(self->data))->currentChar) + 1] = '\0';
			gf2d_font_draw_line_tag(disp, FT_Normal, gfc_color(0, 0, 0, 1), self->position);
		}
		break;
	case Option:
		if (self->sprite) {
			gf2d_sprite_draw(
				self->sprite,
				self->position,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				(((OptionData*)(self->data))->selected) ? 1 : 0
			);
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
		((TextData*)(self->data))->currentChar += 0.5;
	}
	if (self->update) self->update(self);
}
void gui_update_all() {

	for (int i = 0; i < guiManager.entityMax; i++) {
		if (!guiManager.entityList[i].inuse) continue;
		gui_update(&(guiManager.entityList[i]));
	}
}