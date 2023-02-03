#include "gfc_input.h"
#include "simple_logger.h"
#include "simple_json.h"
#include "totw_fiend.h"

static Party playerParty = { 0 };
static Menagerie playerMenagerie = { 0 };

void ally_list_load() {
	SJson* save = sj_load("save/savegame.dat");
	if (!save) {
		slog("Could not load savegame.dat, aborting load.");
		return;
	}
	SJson* menagerie = sj_object_get_value(save, "fiend list");
	if (!menagerie) {
		slog("Could not find fiend list, aborting load.");
		sj_free(save);
		return;
	}
	SJList* keys = sj_object_get_keys_list(menagerie);
	playerMenagerie.fiendCount = keys->count;
	for (int i = 0; i < playerMenagerie.fiendCount; i++) {
		TextWord name = ""; 
		gfc_word_cpy(name, (const char*)sj_list_get_nth(keys, i));
		SJson* fiend = sj_object_get_value(menagerie, name);
		FiendData* data = read_fiend(sj_get_string_value(sj_object_get_value(fiend, "species")));
		sj_get_integer_value(sj_object_get_value(fiend, "level"), &(data->level));
		gfc_word_cpy(data->name, name);
		playerMenagerie.fiends[i] = data;
		calculate_stats(data);
		data->HP = data->stats[MHP];
		data->MP = data->stats[MMP];
	}
	sj_list_delete(keys);

	party_load();

	atexit(party_close);
}
Bool ally_list_is_loaded() {
	return playerParty.unitCount > 0 && playerMenagerie.fiendCount;
}
void party_load() {
	SJson* save = sj_load("save/savegame.dat");
	if (!save) {
		slog("Could not load savegame.dat, aborting party load.");
		return;
	}
	SJson* party = sj_object_get_value(save, "party");
	if (!party) {
		slog("Could not find party list, aborting party load.");
		sj_free(save);
		return;
	}
	int members = sj_array_get_count(party);
	for (int i = 0; i < members; i++) {
		gfc_word_cpy(playerParty.fiendNames[members - 1 - i], sj_get_string_value(sj_array_get_nth(party, i)));
		for (int j = 0; j < playerMenagerie.fiendCount; j++) {
			if (gfc_word_cmp(playerParty.fiendNames[members - 1 - i], playerMenagerie.fiends[j]->name)) {
				playerParty.fiends[members - 1 - i] = playerMenagerie.fiends[j];
				break;
			}
		}
		playerParty.unitCount++;
	}
}
void party_close() {
	for (int i = 0; i < 100; i++) {
		if (playerMenagerie.fiends[i]) {
			memset(&(playerMenagerie.fiends[i]), 0, sizeof(FiendData));
		}
	}
}
int party_get_member_count() {
	return playerParty.unitCount;
}
Entity* party_read_member(int member) {
	if (!(playerParty.fiends[member])) return NULL;
	Entity* fiend = entity_new();
	fiend->data = playerParty.fiends[member];
	return fiend;
}
FiendData* party_read_member_data(int member) {
	if (!(playerParty.fiends[member])) return NULL;
	return playerParty.fiends[member];
}

void dummy_think(Entity* self) {
	if (gfc_input_controller_button_pressed(0, "D_U"))
		self->position.y -= 1;
	else if (gfc_input_controller_button_held(0, "D_U"))
		self->position.y -= 0.1;
	if (gfc_input_controller_button_pressed(0, "D_D"))
		self->position.y += 1;
	else if (gfc_input_controller_button_held(0, "D_D"))
		self->position.y += 0.1;
	if (gfc_input_controller_button_pressed(0, "D_L"))
		self->position.x -= 1;
	else if (gfc_input_controller_button_held(0, "D_L"))
		self->position.x -= 0.1;
	if (gfc_input_controller_button_pressed(0, "D_R"))
		self->position.x += 1;
	else if (gfc_input_controller_button_held(0, "D_R"))
		self->position.x += 0.1;
}

FiendData* read_fiend(const char* species) {
	SJson* file, *selectedFiend;
	file = sj_load("config/fiends.cfg");
	if (!file) {
		slog("Could not access fiend data list.");
		return NULL;
	}
	selectedFiend = sj_object_get_value(file, species);
	if (!selectedFiend) {
		slog("Could not find selected fiend.");
		sj_free(file);
	}
	FiendData* data = gfc_allocate_array(sizeof(FiendData), 1);
	gfc_line_cpy(data->species, species);
	gfc_line_cpy(data->name, species);
	sj_get_integer_value(sj_object_get_value(selectedFiend, "size"), &(data->size));
	sj_get_integer_value(sj_object_get_value(selectedFiend, "rank"), &(data->rank));
	data->sprite = gf2d_sprite_load_all(sj_object_get_value_as_string(selectedFiend, "sprite"), ((data->size < 3) ? 64 : 128), 64, 1, 0);

	TextWord type;
	gfc_word_cpy(type, sj_object_get_value_as_string(selectedFiend, "type"));
	if (sj_string_cmp(sj_object_to_json_string(sj_object_get_value(selectedFiend,"type")),"toughie")) {
		slog("It's a toughie!");
		data->type = FndT_Toughie;
	}
	if (sj_string_cmp(sj_object_to_json_string(sj_object_get_value(selectedFiend, "type")), "trickster")){
		data->type = FndT_Trickster;
	}
	if (sj_string_cmp(sj_object_to_json_string(sj_object_get_value(selectedFiend, "type")), "mage")){
		data->type = FndT_Mage;
	}

	TextWord tact;
	gfc_word_cpy(tact, sj_object_get_value_as_string(selectedFiend, "tactic"));
	if (sj_string_cmp(sj_object_to_json_string(sj_object_get_value(selectedFiend, "tactic")), "destruction")) {
		slog("It's a destroyer!");
		data->tactic = Destruction;
	}
	if (sj_string_cmp(sj_object_to_json_string(sj_object_get_value(selectedFiend, "tactic")), "chaos")) {
		data->tactic = Chaos;
	}
	if (sj_string_cmp(sj_object_to_json_string(sj_object_get_value(selectedFiend, "tactic")), "support")) {
		data->tactic = Support;
	}
	if (sj_string_cmp(sj_object_to_json_string(sj_object_get_value(selectedFiend, "tactic")), "magicless")) {
		data->tactic = Magicless;
	}
	sj_echo(sj_object_get_value(selectedFiend, "stats"));
	sj_get_integer_value(sj_array_get_nth(sj_array_get_nth(sj_object_get_value(selectedFiend, "stats"), MHP), 0), &(data->statsMin[MHP]));
	sj_get_integer_value(sj_array_get_nth(sj_array_get_nth(sj_object_get_value(selectedFiend, "stats"), MMP), 0), &(data->statsMin[MMP]));
	sj_get_integer_value(sj_array_get_nth(sj_array_get_nth(sj_object_get_value(selectedFiend, "stats"), ATK), 0), &(data->statsMin[ATK]));
	sj_get_integer_value(sj_array_get_nth(sj_array_get_nth(sj_object_get_value(selectedFiend, "stats"), DEF), 0), &(data->statsMin[DEF]));
	sj_get_integer_value(sj_array_get_nth(sj_array_get_nth(sj_object_get_value(selectedFiend, "stats"), PWR), 0), &(data->statsMin[PWR]));
	sj_get_integer_value(sj_array_get_nth(sj_array_get_nth(sj_object_get_value(selectedFiend, "stats"), AGL), 0), &(data->statsMin[AGL]));

	sj_get_integer_value(sj_array_get_nth(sj_array_get_nth(sj_object_get_value(selectedFiend, "stats"), MHP), 1), &(data->statsMax[MHP]));
	sj_get_integer_value(sj_array_get_nth(sj_array_get_nth(sj_object_get_value(selectedFiend, "stats"), MMP), 1), &(data->statsMax[MMP]));
	sj_get_integer_value(sj_array_get_nth(sj_array_get_nth(sj_object_get_value(selectedFiend, "stats"), ATK), 1), &(data->statsMax[ATK]));
	sj_get_integer_value(sj_array_get_nth(sj_array_get_nth(sj_object_get_value(selectedFiend, "stats"), DEF), 1), &(data->statsMax[DEF]));
	sj_get_integer_value(sj_array_get_nth(sj_array_get_nth(sj_object_get_value(selectedFiend, "stats"), PWR), 1), &(data->statsMax[PWR]));
	sj_get_integer_value(sj_array_get_nth(sj_array_get_nth(sj_object_get_value(selectedFiend, "stats"), AGL), 1), &(data->statsMax[AGL]));

	sj_free(file);
	return data;
}

void calculate_stats(FiendData* tgt) {
	for (int i = 0; i < 6; i++) {
		int stat = calculate_stat(tgt->statsMin[i], tgt->statsMax[i], tgt->level);
		tgt->stats[i] = stat;
	}
}
int calculate_stat(int minimum, int maximum, int level) {
	level = SDL_clamp(level, 1, 50);
	float a, b;
	if (maximum == 0) {
		return 0;
	}
	else if (minimum == 0) {
		a = (float)minimum;
		b = powf((float)maximum / a, 0.02);
	}
	else {
		b = powf((float)maximum / (float)minimum, (float)1 / (float)49);
		a = (float)maximum / powf(b, 50);
	}
	
	return (int)roundf(a * powf(b,level));
}