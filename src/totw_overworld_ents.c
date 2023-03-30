#include "simple_logger.h"

#include "totw_game_status.h"
#include "totw_battle.h"
#include "totw_player.h"
#include "totw_level.h"

#include "totw_overworld_ents.h"

Entity* owe_boss_new(const char* name, Vector2D worldCoords) {
	Entity* boss = entity_new();
	if (!boss) return NULL;
	OWE_BossData* data = gfc_allocate_array(sizeof(OWE_BossData), 1);
	if (!data) {
		entity_free(boss);
		return NULL;
	}
	SJson* file = sj_load("config/bosses.cfg");
	SJson* bossInfo = sj_object_get_value(sj_object_get_value(file, name),"overworld");
	boss->sprite = gf2d_sprite_load_all(sj_object_get_value_as_string(bossInfo, "sprite"),32,32,2,0);
	boss->frameSpeed = 1.0 / 60.0;
	SJson* dialogue = sj_object_get_value(bossInfo, "dialogue");
	data->lines = sj_array_get_count(dialogue);
	for (int i = 0; i < data->lines; i++) {
		gfc_block_cpy(data->dialogue[i], sj_get_string_value(sj_array_get_nth(dialogue, i)));
	}
	sj_free(file);
	data->line = 0;
	gfc_word_cpy(data->name, name);
	boss->think = owe_boss_think;
	boss->data = data;
	boss->interact = owe_boss_interact;
	boss->solid = true;
	boss->type = ET_Interactible;
	vector2d_copy(boss->mapPosition, worldCoords);
	boss->position = vector2d_multiply(worldCoords, vector2d(32, 32));
	slog("Created a boss at %i, %i (%i, %i)", (int)(boss->mapPosition.x), (int)(boss->mapPosition.y), (int)(boss->position.x), (int)(boss->position.y));
	return boss;
}
void owe_boss_think(Entity* self) {
	if (game_get_state() != GS_Roaming) return;
	OWE_BossData* data = self->data;
	self->frameMin = data->direction * 2;
	self->frameMax = self->frameMin + 1;
	if (self->frame < self->frameMin || self->frame >= self->frameMax+1)
		self->frame = self->frameMin;
	if (!self->interacted) return;
	if (!data->lines || data->line >= data->lines) {
		gui_free(data->dialogueBox);
		gui_free(data->dialogueText);
		load_boss_battle(data->name);
		((PlayerData*)(player_get()->data))->state = PS_Idle;
		tile_set_occupier(level_get_active_level(), self->mapPosition.x, self->mapPosition.y, NULL);
		entity_free(self);
		return;
	}
	if (gfc_input_controller_button_pressed_by_index(0, 0)) {
		data->line++;
		if (data->line < gfc_list_get_count(data->dialogue)) {
			gfc_block_cpy(((TextData*)(data->dialogueText->data))->text, data->dialogue[data->line]);
			((TextData*)(data->dialogueText->data))->currentChar = 0;
		}
	}
}
void owe_boss_interact(Entity* self) {
	slog("Interacted with boss.");
	OWE_BossData* data = self->data;
	self->interacted = true;
	((PlayerData*)(player_get()->data))->state = PS_Inactive;
	data->direction = (((PlayerData*)(player_get()->data))->direction + 2) % 4;
	data->dialogueBox = gui_window_create(vector2d(4, game_get_resolution_y() - (game_get_resolution_y() / 2 - 8 - 28)), vector2d(game_get_resolution_x() - 8, game_get_resolution_y() / 2 - 8 - 28), 0);
	data->dialogueBox->visible = true;
	data->dialogueText = gui_text_create(vector2d(9, game_get_resolution_y() - (game_get_resolution_y() / 2 - 8 - 28) + 5), data->dialogue[0], 1, 1);
	data->dialogueText->visible = true;
}

Entity* owe_portal_new(const char* target, Vector2D worldCoords, Color color) {
	Entity* portal = entity_new();
	if (!portal) return NULL;
	OWE_PortalData* data = gfc_allocate_array(sizeof(OWE_PortalData), 1);
	if (!data) {
		entity_free(portal);
		return NULL;
	}
	portal->sprite = gf2d_sprite_load_all("images/portal.png", 32, 32, 3, 0);
	portal->frameSpeed = 1.0 / 15.0;
	portal->frameMax = 5;
	portal->color = color;
	gfc_word_cpy(data->target, target);
	portal->data = data;
	portal->think = owe_portal_think;
	portal->onStepped = owe_portal_interact;
	portal->drawDepth = 1;
	portal->type = ET_Interactible;
	vector2d_copy(portal->mapPosition, worldCoords);
	portal->position = vector2d_multiply(worldCoords, vector2d(32, 32));

	return portal;
}
void owe_portal_think(Entity* self) {

}
void owe_portal_interact(Entity* self) {
	slog("stepped on a portal.");
	OWE_PortalData* data = self->data;
	TextLine dest = "";
	const char* ext = ".map";
	sprintf(dest, "maps/%s%s", data->target, ext);
	slog("Heading to %s", data->target);
	Level* oldLevel = level_get_active_level();
	entity_free_interactibles();
	level_set_active_level(level_load(dest));
	level_free(oldLevel);
}

Entity* owe_rival_new(TextWord specialty, int difficulty, Vector2D worldCoords) {
	Entity* rival = entity_new();
	if (!rival) return NULL;
	OWE_RivalData* data = gfc_allocate_array(sizeof(OWE_RivalData), 1);
	if (!data) {
		entity_free(rival);
		return NULL;
	}
	SJson* file = sj_load("config/rivals.cfg");
	SJson* rivalInfo = sj_object_get_value(file, specialty);
	rival->sprite = gf2d_sprite_load_all(sj_get_string_value(sj_object_get_value(rivalInfo, "sprite")), 32, 32, 1, 0);
	SJson* dialogueChoices = sj_object_get_value(rivalInfo, "dialogue");
	SJson* dialogue = sj_array_get_nth(dialogueChoices, min(gfc_random() * sj_array_get_count(dialogueChoices), sj_array_get_count(dialogueChoices) - 1));
	data->lines = sj_array_get_count(dialogue);
	for (int i = 0; i < data->lines; i++) {
		gfc_block_cpy(data->dialogue[i], sj_get_string_value(sj_array_get_nth(dialogue, i)));
	}
	sj_free(file);
	gfc_word_cpy(data->specialty, specialty);
	data->difficulty = difficulty;
	data->line = 0;
	rival->think = owe_rival_think;
	rival->data = data;
	rival->interact = owe_rival_interact;
	rival->solid = true;
	rival->type = ET_Interactible;
	vector2d_copy(rival->mapPosition, worldCoords);
	rival->position = vector2d_multiply(worldCoords, vector2d(32, 32));
	slog("Created a rival with specialty %s at %i, %i (%i, %i)", specialty, (int)(rival->mapPosition.x), (int)(rival->mapPosition.y), (int)(rival->position.x), (int)(rival->position.y));
	return rival;
}
void owe_rival_think(Entity* self) {
	if (game_get_state() != GS_Roaming) return;
	OWE_RivalData* data = self->data;
	self->frame = data->direction;
	if (!self->interacted) return;
	if (!data->lines || data->line >= data->lines) {
		gui_free(data->dialogueBox);
		gui_free(data->dialogueText);
		generate_new_rival_battle(data->specialty, data->difficulty);
		((PlayerData*)(player_get()->data))->state = PS_Idle;
		tile_set_occupier(level_get_active_level(), self->mapPosition.x, self->mapPosition.y, NULL);
		entity_free(self);
		return;
	}
	if (gfc_input_controller_button_pressed_by_index(0, 0)) {
		data->line++;
		if (data->line < gfc_list_get_count(data->dialogue)) {
			gfc_block_cpy(((TextData*)(data->dialogueText->data))->text, data->dialogue[data->line]);
			((TextData*)(data->dialogueText->data))->currentChar = 0;
		}
	}
}
//void owe_rival_update(Entity* self);
void owe_rival_interact(Entity* self) {
	slog("Interacted with rival.");
	OWE_RivalData* data = self->data;
	self->interacted = true;
	((PlayerData*)(player_get()->data))->state = PS_Inactive;
	data->direction = (((PlayerData*)(player_get()->data))->direction + 2) % 4;
	data->dialogueBox = gui_window_create(vector2d(4, game_get_resolution_y() - (game_get_resolution_y() / 2 - 8 - 28)), vector2d(game_get_resolution_x() - 8, game_get_resolution_y() / 2 - 8 - 28), 0);
	data->dialogueBox->visible = true;
	data->dialogueText = gui_text_create(vector2d(9, game_get_resolution_y() - (game_get_resolution_y() / 2 - 8 - 28) + 5), data->dialogue[0], 1, 1);
	data->dialogueText->visible = true;
}