#include "gfc_input.h"
#include "simple_logger.h"
#include "simple_json.h"
#include "totw_fiend.h"
#include "totw_battle.h"

static Party playerParty = { 0 };
static Menagerie playerMenagerie = { 0 };

int fiend_damage(FiendData* target, int raw) {
	if (fiend_has_status(target, FS_Defending))
		raw /= 2;
	raw = max(raw, 0);
	target->HP = max(target->HP - raw, 0);
	return raw;
}
float fiend_impress(FiendData* target, float raw) {
	if (fiend_has_status(target, FS_Defending))
		raw /= 2;
	battle_impress_enemy(raw);
	return raw;
}
Bool fiend_has_status(FiendData* fiend, FiendStatusType status) {
	for (int i = 0; i < 10; i++) {
		if (fiend->statuses[i].enabled && fiend->statuses[i].status == status)
			return true;
	}
	return false;
}
void fiend_remove_status(FiendData* fiend, FiendStatusType status) {
	for (int i = 0; i < 10; i++) {
		if (fiend->statuses[i].enabled && fiend->statuses[i].status == status) {
			fiend->statuses[i].enabled = false;
			fiend->statuses[i].status = FS_Null;
			fiend->statuses[i].duration = 0;
			return;
		}
	}
}
void fiend_apply_status(FiendData* fiend, FiendStatusType status, int duration) {
	for (int i = 0; i < 10; i++) {
		if (fiend->statuses[i].enabled && fiend->statuses[i].status == status) {
			fiend->statuses[i].duration = duration;
			return;
		}
	}
	for (int i = 0; i < 10; i++) {
		if (!fiend->statuses[i].enabled) {
			fiend->statuses[i].enabled = true;
			fiend->statuses[i].status = status;
			fiend->statuses[i].duration = duration;
			return;
		}
	}
}

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
		TextLine name = ""; 
		gfc_line_cpy(name, (const char*)sj_list_get_nth(keys, i));
		SJson* fiend = sj_object_get_value(menagerie, name);
		FiendData* data = read_fiend(sj_get_string_value(sj_object_get_value(fiend, "species")));
		sj_get_integer_value(sj_object_get_value(fiend, "level"), &(data->level));
		gfc_line_cpy(data->name, name);
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
		gfc_line_cpy(playerParty.fiendNames[i], sj_get_string_value(sj_array_get_nth(party, i)));
		for (int j = 0; j < playerMenagerie.fiendCount; j++) {
			if (!gfc_word_cmp(playerParty.fiendNames[i], playerMenagerie.fiends[j]->name)) {
				playerParty.fiends[i] = playerMenagerie.fiends[j];
				playerParty.slotsTaken += playerMenagerie.fiends[j]->size;
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
	fiend->type = ET_Fiend;
	fiend->data = playerParty.fiends[member];
	((FiendData*)(fiend->data))->entity = fiend;
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

	
	if (!gfc_word_cmp(sj_get_string_value(sj_object_get_value(selectedFiend, "type")), "toughie")) {
		slog("It's a toughie!");
		data->type = FndT_Toughie;
	}
	if (!gfc_word_cmp(sj_get_string_value(sj_object_get_value(selectedFiend, "type")), "trickster")) {
		slog("It's a trickster!");
		data->type = FndT_Trickster;
	}
	if (!gfc_word_cmp(sj_get_string_value(sj_object_get_value(selectedFiend, "type")), "mage")) {
		slog("It's a mage!");
		data->type = FndT_Mage;
	}

	TextWord tact;
	gfc_word_cpy(tact, sj_object_get_value_as_string(selectedFiend, "tactic"));
	if (!gfc_word_cmp(sj_get_string_value(sj_object_get_value(selectedFiend, "tactic")), "destruction")) {
		data->tactic = Destruction;
	}
	if (!gfc_word_cmp(sj_get_string_value(sj_object_get_value(selectedFiend, "tactic")), "chaos")) {
		data->tactic = Chaos;
	}
	if (!gfc_word_cmp(sj_get_string_value(sj_object_get_value(selectedFiend, "tactic")), "support")) {
		data->tactic = Support;
	}
	if (!gfc_word_cmp(sj_get_string_value(sj_object_get_value(selectedFiend, "tactic")), "magicless")) {
		data->tactic = Magicless;
	}

	data->skills[0] = get_skill("Attack");
	data->skills[1] = get_skill("Defend");

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

Bool fiend_name_taken(TextLine query) {
	if (!query) return false;
	for (int i = 0; i < playerMenagerie.fiendCount; i++) {
		if (!gfc_line_cmp(query, playerMenagerie.fiends[i]->name))
			return true;
	}
	return false;
}

void fiend_register_new(FiendData* newFiend) {
	if (!newFiend) return;
	playerMenagerie.fiends[playerMenagerie.fiendCount] = newFiend;
	playerMenagerie.fiendCount++;
}

void party_add_fiend(FiendData* newFiend) {
	if (!newFiend) return;
	if (playerParty.unitCount >= 4) return;
	//if (playerParty.slotsTaken + newFiend->size > 4) return;
	playerParty.slotsTaken += newFiend->size;
	gfc_line_cpy(playerParty.fiendNames[playerParty.unitCount], newFiend->name);
	playerParty.fiends[playerParty.unitCount] = newFiend;
	playerParty.unitCount++;
}

SJson* menagerie_get_as_json() {
	SJson* toExport = sj_object_new();
	if (!toExport) return NULL;
	for (int i = 0; i < 100; i++) {
		if (!playerMenagerie.fiends[i]) continue;
		TextLine name;
		gfc_word_cpy(name, playerMenagerie.fiends[i]->name);
		TextLine species;
		gfc_word_cpy(species, playerMenagerie.fiends[i]->species);
		int level = playerMenagerie.fiends[i]->level;
		int xp = playerMenagerie.fiends[i]->exp;
		TextWord tactic;
		switch (playerMenagerie.fiends[i]->tactic) {
		case Destruction:
			gfc_word_cpy(tactic, "destruction");
			break;
		case Chaos:
			gfc_word_cpy(tactic, "chaos");
			break;
		case Support:
			gfc_word_cpy(tactic, "support");
			break;
		case Magicless:
			gfc_word_cpy(tactic, "magicless");
			break;
		}
		SJson* fiend = sj_object_new();
		if (!fiend) return NULL;
		sj_object_insert(fiend, "species", sj_new_str(species));
		sj_object_insert(fiend, "level", sj_new_int(level));
		sj_object_insert(fiend, "exp", sj_new_int(xp));
		sj_object_insert(fiend, "tactic", sj_new_str(tactic));
		sj_object_insert(toExport, name, fiend);
	}
	return toExport;
}

int exp_required_at_level(int level, int rank) {
	float expReq = (powf((level - 1) / 0.2f, 1.2f) + 15) * (1 + 0.25 * rank);
	return (int)(roundf(expReq));
}

int fiend_check_level_up(FiendData* self, TextBlock* textDisplay, int* step, Bool* leveledUp) {
	if (!self || !textDisplay || !step) return 1;
	static int hpG, mpG, atkG, defG, pwrG, aglG;
	switch (*step) {
	case 0: {
		int levels = 0;
		int requirement = exp_required_at_level(self->level, self->rank);
		while (self->exp >= requirement) {
			levels++;
			self->exp -= requirement;
			requirement = exp_required_at_level(self->level + levels, self->rank);
		}
		if (levels) {
			*leveledUp = true;
			TextBlock disp = "";
			sprintf(disp, "%s has leveled up to level %i!", self->name, self->level + levels);
			battle_set_main_dialogue(disp);
			//gfc_block_cpy(*textDisplay, disp);

			self->level += levels;
			int statBase[6] = { self->stats[0],self->stats[1],self->stats[2],self->stats[3],self->stats[4],self->stats[5] };
			calculate_stats(self);
			hpG = self->stats[MHP] - statBase[MHP];
			mpG = self->stats[MMP] - statBase[MMP];
			atkG = self->stats[ATK] - statBase[ATK];
			defG = self->stats[DEF] - statBase[DEF];
			pwrG = self->stats[PWR] - statBase[PWR];
			aglG = self->stats[AGL] - statBase[AGL];
		}
		else {
			return 1;
		}
		break;
	}
	case 1:
		if (*leveledUp) {
			TextBlock dispM = "", dispA = "", dispB = "", dispC = "";
			sprintf(dispM, "HP + %i                MP + %i\nATK + %i                DEF + %i\nPWR + %i                AGL + %i", hpG, mpG, atkG, defG, pwrG, aglG);
			battle_set_main_dialogue(dispM);
			//gfc_block_cpy(*textDisplay, dispM);
		}
		break;
	case 2: 
		return 1;
	}
	*step += 1;
	return 0;
}