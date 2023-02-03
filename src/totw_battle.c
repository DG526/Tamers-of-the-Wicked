#include "totw_battle.h"
#include "simple_logger.h"

static Battle battle = { 0 };
static TextBlock textPresets[64];

int in_battle() {
	if (battle.active)
		return 1;
	return 0;
}
void kill_battle() {
	for (int i = 0; i < 8; i++) {
		if (battle.battlers[i] && battle.battlers[i]->inuse && ((FiendData*)(battle.battlers[i]->data))->party == 2)
			entity_free(battle.battlers[i]);
		else if (battle.battlers[i] && battle.battlers[i]->inuse && ((FiendData*)(battle.battlers[i]->data))->party == 1)
			entity_free_leave_data(battle.battlers[i]);
	}
	slog("killing battle.");
	gui_free_all();
	memset(&battle, 0, sizeof(Battle));
}

BStateFlag new_battle_flag(BStateType type, int duration) {
	BStateFlag flag = {type, duration};
	return flag;
}

int changeDialogue(void* text) {
	switch (battle.phase) {
	case BP_Opening:
		memcpy(((TextData*)(battle.gui.textDisplay.text->data))->text, *((TextBlock*)(text)), sizeof(TextBlock));
		break;
	case BP_RoundPrep:
		memcpy(((TextData*)(battle.gui.choice_initial.dialogueText->data))->text, *((TextBlock*)(text)), sizeof(TextBlock));
		break;
	}
}

void load_battle_basics() {
	slog("Starting to load battle basics.");
	Vector2D renderSize = vector2d(300, 180);
	TextBlock encounterList;
	TextLine encounters[4];
	int foes = 0;
	for (int i = 0, j = 0; i < 4; i++) {
		char* sbuf[GFCLINELEN];
		FiendData* foe = NULL;
		for (; j < 8; j++) {
			if (battle.battlers[j] && battle.battlers[j]->inuse && ((FiendData*)(battle.battlers[j]->data))->party == 2) {
				foe = (FiendData*)(battle.battlers[j]->data);
				j++;
				foes++;
				break;
			}
		}
		if (!foe) break;
		sprintf(sbuf, "%s approaches!", foe->name);
		memcpy(encounters[i],sbuf,sizeof(TextLine));
	}
	slog("Got fiend names.");
	char* sbuf[GFCTEXTLEN];
	sprintf(sbuf, "%s\n%s\n%s\0", encounters[0], foes > 1 ? encounters[1] : "", foes > 2 ? encounters[2] : "");
	if (foes == 4)
		memcpy(sbuf, "A horde of fiends approaches!",sizeof(char) * 30);
	memcpy(encounterList, sbuf, sizeof(TextBlock));
	slog("%s", sbuf);
	/////// GUI ///////
	//Basic Text Display
	battle.gui.textDisplay.dialogueFrame = gui_window_create(vector2d(4, 76), vector2d(renderSize.x - 8, renderSize.y / 2 - 8 - 28), 0);
	battle.gui.textDisplay.text = gui_text_create(vector2d(9, 81), encounterList, true, 1);
	battle.gui.textDisplay.scrolling = true;
	battle.gui.textDisplay.dialogueFrame->visible = true;
	battle.gui.textDisplay.text->visible = true;
	//Initial choice menu
	battle.gui.choice_initial.dialogueFrame = gui_window_create(vector2d(4, 76), vector2d(renderSize.x - 128, renderSize.y / 2 - 8 - 28), 0);
	battle.gui.choice_initial.optionFrame = gui_window_create(vector2d(4 + renderSize.x - 127, 76), vector2d(120, renderSize.y / 2 - 8 - 28), 0);
	battle.gui.choice_initial.dialogueText = gui_text_create(vector2d(9, 81), "", false, 1);
	battle.gui.choice_initial.opFight = gui_option_create(vector2d(9 + renderSize.x - 127, 81), "Fight", true, 1);
	battle.gui.choice_initial.opRecruit = gui_option_create(vector2d(9 + renderSize.x - 127 + 118 / 2, 81), "Recruit", false, 1);
	battle.gui.choice_initial.opFlee = gui_option_create(vector2d(9 + renderSize.x - 127 + 118 / 2, 81 + 14), "Flee", false, 1);
	TextBlock d = "";

	((OptionData*)(battle.gui.choice_initial.opFight->data))->left = battle.gui.choice_initial.opRecruit;
	((OptionData*)(battle.gui.choice_initial.opFight->data))->right = battle.gui.choice_initial.opRecruit;
	((OptionData*)(battle.gui.choice_initial.opFight->data))->up = battle.gui.choice_initial.opFlee;
	((OptionData*)(battle.gui.choice_initial.opFight->data))->down = battle.gui.choice_initial.opFlee;
	((OptionData*)(battle.gui.choice_initial.opFight->data))->onHover = changeDialogue;
	memcpy(textPresets[1], "Fight:\n    Engage the enemy", sizeof(TextBlock));
	((OptionData*)(battle.gui.choice_initial.opFight->data))->hoverArgument = &textPresets[1];
	((OptionData*)(battle.gui.choice_initial.opFight->data))->onChoose = changeDialogue;
	memcpy(textPresets[2], "Whoops, can't do that yet.", sizeof(TextBlock));
	((OptionData*)(battle.gui.choice_initial.opFight->data))->choiceArgument = &textPresets[2];

	((OptionData*)(battle.gui.choice_initial.opRecruit->data))->left = battle.gui.choice_initial.opFight;
	((OptionData*)(battle.gui.choice_initial.opRecruit->data))->right = battle.gui.choice_initial.opFight;
	((OptionData*)(battle.gui.choice_initial.opRecruit->data))->up = battle.gui.choice_initial.opFlee;
	((OptionData*)(battle.gui.choice_initial.opRecruit->data))->down = battle.gui.choice_initial.opFlee;
	((OptionData*)(battle.gui.choice_initial.opRecruit->data))->onHover = changeDialogue;
	memcpy(textPresets[3], "Recruit:\n    Attempt to recruit\n    an enemy fiend", sizeof(TextBlock));
	((OptionData*)(battle.gui.choice_initial.opRecruit->data))->hoverArgument = &textPresets[3];
	((OptionData*)(battle.gui.choice_initial.opRecruit->data))->onChoose = changeDialogue;
	memcpy(textPresets[4], "Whoops, can't do that yet.", sizeof(TextBlock));
	((OptionData*)(battle.gui.choice_initial.opRecruit->data))->choiceArgument = &textPresets[4];

	((OptionData*)(battle.gui.choice_initial.opFlee->data))->up = battle.gui.choice_initial.opRecruit;
	((OptionData*)(battle.gui.choice_initial.opFlee->data))->down = battle.gui.choice_initial.opRecruit;
	((OptionData*)(battle.gui.choice_initial.opFlee->data))->onHover = changeDialogue;
	memcpy(textPresets[5], "Flee:\n    Attempt to escape", sizeof(TextBlock));
	((OptionData*)(battle.gui.choice_initial.opFlee->data))->hoverArgument = &textPresets[5];
	((OptionData*)(battle.gui.choice_initial.opFlee->data))->onChoose = changeDialogue;
	memcpy(textPresets[6], "Whoops, can't do that yet.", sizeof(TextBlock));
	((OptionData*)(battle.gui.choice_initial.opFlee->data))->choiceArgument = &textPresets[6];


	//Player Ally GUI
	for (int i = 0; i < party_get_member_count(); i++) {
		FiendData* dat = party_read_member_data(i);
		char* numBuf[4];
		battle.gui.allyGUI[0].frame = gui_window_create(vector2d(0 + (renderSize.x / 4 * i), renderSize.y - 48), vector2d(renderSize.x / 4, 60), 0);
		battle.gui.allyGUI[0].name = gui_text_create(vector2d(6 + (renderSize.x / 4 * i), renderSize.y - 44), dat->name, false, 1);
		sprintf(numBuf, "%i", dat->HP);
		battle.gui.allyGUI[0].hNum = gui_text_create(vector2d(4 + (renderSize.x / 4 * i), renderSize.y - 28), numBuf, false, 2);
		sprintf(numBuf, "%i", dat->MP);
		battle.gui.allyGUI[0].mNum = gui_text_create(vector2d(4 + (renderSize.x / 4 * i), renderSize.y - 15), numBuf, false, 2);
		battle.gui.allyGUI[0].hBar = gui_meter_create(vector2d(5 + 24 + (renderSize.x / 4 * i), renderSize.y - 27), vector2d(renderSize.x / 4 - 32, 8), gfc_color(1, 0, 0, 1), 1);
		battle.gui.allyGUI[0].mBar = gui_meter_create(vector2d(5 + 24 + (renderSize.x / 4 * i), renderSize.y - 14), vector2d(renderSize.x / 4 - 32, 8), gfc_color(0, 0, 0.8, 1), 1);
		((MeterData*)(battle.gui.allyGUI[0].hBar->data))->fill = (float)dat->HP / (float)dat->stats[MHP];
		((MeterData*)(battle.gui.allyGUI[0].mBar->data))->fill = (float)dat->MP / (float)dat->stats[MMP];

		battle.gui.allyGUI[0].frame->visible = true;
		battle.gui.allyGUI[0].name->visible = true;
		battle.gui.allyGUI[0].hNum->visible = true;
		battle.gui.allyGUI[0].mNum->visible = true;
		battle.gui.allyGUI[0].hBar->visible = true;
		battle.gui.allyGUI[0].mBar->visible = true;
	}

	battle.phase = BP_Opening;
}
void switchPhase(BattlePhase phase) {
	battle.phase = phase;
	switch (phase) {
	case BP_RoundPrep:
		battle.gui.textDisplay.dialogueFrame->visible = false;
		battle.gui.textDisplay.text->visible = false;

		battle.gui.choice_initial.dialogueFrame->visible = true;
		battle.gui.choice_initial.optionFrame->visible = true;
		battle.gui.choice_initial.dialogueText->visible = true;
		battle.gui.choice_initial.opFight->visible = true;
		battle.gui.choice_initial.opRecruit->visible = true;
		battle.gui.choice_initial.opFlee->visible = true;
		break;
	}
}

SJson* pluck(SJson* options, int* pointsLeft) {
	if (!sj_is_array(options)) {
		slog("NOT AN ARRAY!");
		return NULL;
	}
	int opCount = sj_array_get_count(options);
	if (opCount <= 0) return NULL;
	int pick = (int)(gfc_random() * opCount) % opCount;
	slog("Picking option %i", pick);
	SJson* chosen = sj_array_get_nth(options, pick);
	if (!chosen) {
		slog("Something went wrong with the enemy picker.");
		return NULL;
	}
	int points = 0;
	sj_get_integer_value(sj_object_get_value(chosen, "weight"), &points);
	slog("Weight of option is %i", points);
	*pointsLeft -= points;
	if (*pointsLeft < 0) return NULL; //went over
	slog("Chose a rando.");
	return chosen;
}

void generate_new_battle(TextWord dungeon, int poolID) {
	SJson* battleFile = sj_load("config/battles.cfg");
	if (!battleFile) {
		slog("Could not load battles.cfg!");
		return;
	}
	char* sbuf[20];
	sprintf(sbuf, "dungeon-%s", dungeon);
	SJson* dungeonData = sj_object_get_value(battleFile, sbuf);
	if (!dungeonData) {
		slog("Could not find dungeon battle data in battles.cfg!");
		sj_free(battleFile);
		return;
	}
	sprintf(sbuf, "Z%i", poolID);
	slog("Loading info for zone: %s", sbuf);
	SJson* zoneData = sj_object_get_value(dungeonData, sbuf);
	int qMax = 0;
	sj_get_integer_value(sj_object_get_value(zoneData, "max"), &qMax);
	if (qMax <= 0) {
		slog("Missing zone data or max battle weight.");
		sj_free(battleFile);
		return;
	}
	int num = 0;
	int pos = 0;
	for (int slots = 4, errors = 0; slots > 0 && errors < 3;) {
		if (qMax <= 0)
			break;
		SJson* search = pluck(sj_object_get_value(zoneData, "options"), &qMax);
		if (qMax < 0)
			break;
		if (!search) {
			errors++;
			continue;
		}
		FiendData* data = read_fiend(sj_get_string_value(sj_object_get_value(search, "species")));
		if (data->size > slots) {
			slog("Fiend was too big!");
			errors++;
			continue;
		}
		Entity* foe = entity_new();
		slog("Fiend entity created.");
		int level = 1;
		sj_get_integer_value(sj_object_get_value(search, "level"), &level);
		slog("Fiend @ level %i.", level);
		int exp = 0;
		sj_get_integer_value(sj_object_get_value(search, "exp"), &exp);
		slog("Fiend has %i exp worth.", exp);
		data->exp = exp;
		data->level = level;
		data->party = 2;
		gfc_line_cpy(data->species, sj_get_string_value(sj_object_get_value(search, "species")));
		gfc_line_cpy(data->name, sj_get_string_value(sj_object_get_value(search, "species")));
		slog("Copied species name.");
		foe->data = data;
		slog("Copied reference to fiend data.");
		slots -= data->size;
		battle.battlers[num] = foe;
		slog("Fiend added to list of battlers.");
		foe->sprite = data->sprite;

		foe->position = vector2d(pos, 8);
		pos += foe->sprite->frame_w + 4;

		num++;
	}
	int xOffset = 150 - (pos - 4) / 2;
	for (int i = 0; i < num; i++) {
		vector2d_add(battle.battlers[i]->position, battle.battlers[i]->position, vector2d(xOffset, 0));
	}
	battle.active = true;
	sj_free(battleFile);
	load_battle_basics();
}

void battle_update() {
	if (!battle.active) return;
	switch (battle.phase) {
	case BP_Opening:
		if (gfc_input_controller_button_pressed_by_index(0, 0) || gfc_input_controller_button_pressed_by_index(0, 1) || 
			gfc_input_controller_button_pressed_by_index(0, 2) || gfc_input_controller_button_pressed_by_index(0, 3)) {
			switchPhase(BP_RoundPrep);
		}
		break;
	}
}