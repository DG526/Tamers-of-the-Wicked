#include "simple_logger.h"
#include "totw_game_status.h"
#include "totw_bgm.h"
#include "totw_camera.h"
#include "totw_battle.h"
#include "totw_naming_screen.h"
#include "totw_item.h"

static Battle battle = { 0 };
static TextBlock textPresets[64];

void refresh_item_options();

int battle_get_current_fiend() {
	return battle.currentFiend;
}
void battle_next_fiend() {
	battle.currentFiend++;
}
void battle_impress_enemy(float percent) {
	battle.recruitChance += percent;
	battle.recruiting = true;
}
Bool battle_check_lose() {
	for (int i = 0; i < 8; i++)
	{
		if (battle.battlers[i] && battle.battlers[i]->inuse && ((FiendData*)(battle.battlers[i]->data))->party == 1 
			&& ((FiendData*)(battle.battlers[i]->data))->HP > 0)
			return false;
	}
	return true;
}
Bool battle_check_win() {
	for (int i = 0; i < 8; i++)
	{
		if (battle.battlers[i] && battle.battlers[i]->inuse && ((FiendData*)(battle.battlers[i]->data))->party == 2
			&& ((FiendData*)(battle.battlers[i]->data))->HP > 0)
			return false;
	}
	return true;
}

FiendData* battle_get_party_member(int party, int member) {
	if (party <= 0 || party > 2 || member <= 0 || member > 4) return NULL;
	int memFound = 1;
	for (int i = 0; i < 8; i++) {
		if (battle.battlers[i] && battle.battlers[i]->inuse && ((FiendData*)(battle.battlers[i]->data))->party == party) {
			if (memFound == member)
				return (FiendData*)(battle.battlers[i]->data);
			memFound++;
		}
	}
	return NULL;
}
void battle_set_main_dialogue(TextBlock text) {
	memcpy(((TextData*)(battle.gui.textDisplay.text->data))->text, *((TextBlock*)(text)), sizeof(TextBlock));
}
void battle_set_skill_dialogue(TextLine text) {
	memcpy(((TextData*)(battle.gui.choice_command.dialogueText->data))->text, text, sizeof(TextBlock));
}

void battle_wait(int milliseconds) {
	battle.timeForMove = SDL_GetTicks64() + milliseconds / game_get_wait_speed();
}

int in_battle() {
	if (battle.active)
		return 1;
	return 0;
}
void kill_battle() {
	FiendData* recruited = NULL;
	if (battle.recruited) {
		for (int i = 0; i < 8; i++) {
			if (battle.battlers[i] && battle.battlers[i]->inuse && ((FiendData*)(battle.battlers[i]->data))->party == 2 &&
				battle.recruited && ((FiendData*)(battle.battlers[i]->data)) == battle_get_party_member(2, battle.recruitTarget)) {
				slog("Separating recruited fiend's data.");
				recruited = (FiendData*)(battle.battlers[i]->data);
				entity_free_leave_data(battle.battlers[i]);
				break;
			}
		}
	}
	for (int i = 0; i < 8; i++) {
		if (battle.battlers[i] && battle.battlers[i]->inuse && ((FiendData*)(battle.battlers[i]->data))->party == 2)
			entity_free(battle.battlers[i]);
		else if (battle.battlers[i] && battle.battlers[i]->inuse && ((FiendData*)(battle.battlers[i]->data))->party == 1)
			entity_free_leave_data(battle.battlers[i]);
	}
	slog("killing battle.");
	gui_free_all();
	if (recruited) {
		recruited->party = 1;
		naming_system_new(recruited);
		game_set_state(GS_Naming);
	}
	else
		game_set_state(GS_Roaming);
	memset(&battle, 0, sizeof(Battle));
	battle_sounds_free();
}

BStateFlag new_battle_flag(BStateType type, int duration) {
	BStateFlag flag = {type, duration};
	return flag;
}

void switchPhase(BattlePhase phase) {
	BattlePhase oldPhase = battle.phase;
	battle.phase = phase;

	battle.gui.textDisplay.dialogueFrame->visible = false;
	battle.gui.textDisplay.text->visible = false;

	battle.gui.choice_initial.dialogueFrame->visible = false;
	battle.gui.choice_initial.optionFrame->visible = false;
	battle.gui.choice_initial.dialogueText->visible = false;
	battle.gui.choice_initial.opFight->visible = false;
	battle.gui.choice_initial.opRecruit->visible = false;
	battle.gui.choice_initial.opFlee->visible = false;
	battle.gui.choice_initial.opItem->visible = false;

	battle.gui.target_enemy.dialogueFrame->visible = false;
	battle.gui.target_enemy.dialogueText->visible = false;
	for (int i = 0; battle.gui.target_enemy.options[i] != NULL; i++) {
		battle.gui.target_enemy.options[i]->visible = false;
	}

	battle.gui.target_ally.dialogueFrame->visible = false;
	battle.gui.target_ally.dialogueText->visible = false;
	for (int i = 0; battle.gui.target_ally.options[i] != NULL; i++) {
		battle.gui.target_ally.options[i]->visible = false;
	}

	battle.gui.choice_tactic.dialogueFrame->visible = false;
	battle.gui.choice_tactic.dialogueText->visible = false;
	battle.gui.choice_tactic.optionFrame->visible = false;
	battle.gui.choice_tactic.opChaos->visible = false;
	battle.gui.choice_tactic.opDestruction->visible = false;
	battle.gui.choice_tactic.opSupport->visible = false;
	battle.gui.choice_tactic.opCommand->visible = false;
	((OptionData*)(battle.gui.choice_tactic.opChaos->data))->selected = false;
	((OptionData*)(battle.gui.choice_tactic.opDestruction->data))->selected = false;
	((OptionData*)(battle.gui.choice_tactic.opSupport->data))->selected = false;
	((OptionData*)(battle.gui.choice_tactic.opCommand->data))->selected = false;

	battle.gui.choice_command.dialogueFrame->visible = false;
	battle.gui.choice_command.dialogueText->visible = false;
	battle.gui.choice_command.optionFrame->visible = false;
	for (int i = 0; i < 10; i++) {
		battle.gui.choice_command.opSkills[i]->visible = false;
		((OptionData*)(battle.gui.choice_command.opSkills[i]->data))->selected = false;
	}
	for (int i = 0; i < 8; i++) {
		battle.gui.choice_command.manaCosts[i]->visible = false;
	}

	battle.gui.choice_items.dialogueFrame->visible = false;
	battle.gui.choice_items.optionFrame->visible = false;
	battle.gui.choice_items.dialogueText->visible = false;
	battle.gui.choice_items.options->visible = false;

	switch (phase) {
	case BP_RoundPrep:
		battle.gui.choice_initial.dialogueFrame->visible = true;
		battle.gui.choice_initial.optionFrame->visible = true;
		battle.gui.choice_initial.dialogueText->visible = true;
		battle.gui.choice_initial.opFight->visible = true;
		battle.gui.choice_initial.opRecruit->visible = true;
		battle.gui.choice_initial.opFlee->visible = true;
		battle.gui.choice_initial.opItem->visible = true;
		if (gfc_list_get_count(inventory_get_items()) == 0) {
			((OptionData*)(battle.gui.choice_initial.opItem->data))->grayed = true;
		}
		break;
	case BP_Acting:
		battle.currentFiend = 1;
	case BP_Ending:
		battle.gui.textDisplay.dialogueFrame->visible = true;
		battle.gui.textDisplay.text->visible = true;
		((TextData*)(battle.gui.textDisplay.text))->scrolling = false;
		break;
	case BP_PreppingTactic:
		battle.gui.choice_tactic.dialogueFrame->visible = true;
		battle.gui.choice_tactic.dialogueText->visible = true;
		TextBlock disp = "";
		sprintf(disp, "How will %s act?", battle_get_party_member(1, battle_get_current_fiend())->name);
		gfc_block_cpy(((TextData*)(battle.gui.choice_tactic.dialogueText->data))->text, disp);
		battle.gui.choice_tactic.optionFrame->visible = true;
		battle.gui.choice_tactic.opChaos->visible = true;
		battle.gui.choice_tactic.opDestruction->visible = true;
		//battle.gui.choice_tactic.opSupport->visible = true;
		battle.gui.choice_tactic.opCommand->visible = true;

		switch (battle_get_party_member(1, battle_get_current_fiend())->tactic) {
		case Destruction:
			((OptionData*)(battle.gui.choice_tactic.opDestruction->data))->selectedNow = true;
			break;
		case Chaos:
			((OptionData*)(battle.gui.choice_tactic.opChaos->data))->selectedNow = true;
			break;
		case Support:
			((OptionData*)(battle.gui.choice_tactic.opSupport->data))->selectedNow = true;
			break;
		}
		break;
	case BP_PreppingAction:
		battle.gui.choice_command.dialogueFrame->visible = true;
		battle.gui.choice_command.dialogueText->visible = true;
		battle.gui.choice_command.optionFrame->visible = true;
		for (int i = 0; i < 10; i++) {
			if (battle_get_party_member(1, battle_get_current_fiend())->skills[i].inUse) {
				battle.gui.choice_command.opSkills[i]->visible = true;
				gfc_line_cpy(((OptionData*)(battle.gui.choice_command.opSkills[i]->data))->text, battle_get_party_member(1, battle_get_current_fiend())->skills[i].name);
				((OptionData*)(battle.gui.choice_command.opSkills[i]->data))->hoverArg1 = battle_get_party_member(1, battle_get_current_fiend())->skills[i].description;
			}
		}
		for (int i = 0; i < 8; i++) {
			if (battle_get_party_member(1, battle_get_current_fiend())->skills[i + 2].inUse) {
				battle.gui.choice_command.manaCosts[i]->visible = true;
				TextBlock mc = "ALL";
				if (gfc_line_cmp(battle_get_party_member(1, battle_get_current_fiend())->skills[i].name, "MartyrSurge"))
					sprintf(mc, "%i", battle_get_party_member(1, battle_get_current_fiend())->skills[i+2].manaCost);
				gfc_word_cpy(((TextData*)(battle.gui.choice_command.manaCosts[i]->data))->text, mc);
			}
		}
		((OptionData*)(battle.gui.choice_command.opSkills[0]->data))->selectedNow = true;
		break;
	case BP_PreppingRecruit:
		battle.gui.target_enemy.dialogueFrame->visible = true;
		battle.gui.target_enemy.dialogueText->visible = true;
		Bool first = false;
		int enemies = (battle_get_party_member(2, 4) ? 4 : (battle_get_party_member(2, 3) ? 3 : (battle_get_party_member(2, 2) ? 2 : 1)));
		for (int i = 0; battle.gui.target_enemy.options[i]; i++) {
			if (battle_get_party_member(2, i+1)->HP > 0) {
				battle.gui.target_enemy.options[i]->visible = true;
				((OptionData*)(battle.gui.target_enemy.options[i]->data))->choiceArg1 = i + 1;
				((OptionData*)(battle.gui.target_enemy.options[i]->data))->onChoose = startRound_recruit;
				((OptionData*)(battle.gui.target_enemy.options[i]->data))->selected = false;
				if (!first) {
					((OptionData*)(battle.gui.target_enemy.options[i]->data))->selectedNow = true;
					first = true;
				}
			}
		}
		for (int i = 0; battle.gui.target_enemy.options[i]; i++) {
			if (battle_get_party_member(2, i + 1)->HP > 0) {
				Bool foundLeft = false, foundRight = false;
				for (int j = i + 1; !foundRight; j++) {
					j %= enemies;
					if (j == i) {
						slog("Met end of option list for Left.");
						break;
					}
					if (battle.gui.target_enemy.options[j] && battle.gui.target_enemy.options[j]->visible) {
						((OptionData*)(battle.gui.target_enemy.options[i]->data))->right = battle.gui.target_enemy.options[j];
						foundRight = true;
					}
				}
				for (int j = i - 1; !foundLeft; j--) {
					if (j < 0) j = enemies - 1;
					if (j == i) {
						slog("Met end of option list for Right.");
						break;
					}
					if (battle.gui.target_enemy.options[j] && battle.gui.target_enemy.options[j]->visible) {
						((OptionData*)(battle.gui.target_enemy.options[i]->data))->left = battle.gui.target_enemy.options[j];
						foundLeft = true;
					}
				}
			}
		}
		gfc_input_update();
		break;
	case BP_PreppingItem:
		battle.gui.choice_items.dialogueFrame->visible = true;
		battle.gui.choice_items.optionFrame->visible = true;
		battle.gui.choice_items.dialogueText->visible = true;
		battle.gui.choice_items.options->visible = true;
		gui_page_list_refresh_visibility(battle.gui.choice_items.options);
		break;
	}
}
int hover_item(int item) {
	Item* itemRef = gfc_list_get_nth(inventory_get_items(), item);
	gfc_block_cpy(((TextData*)(battle.gui.choice_items.dialogueText->data))->text, itemRef->description);
	return 0;
}
int select_item(int item) {
	battle.itemUsed = item;
	switchPhase(BP_PreppingItemAim);
	return 0;
}
void refresh_item_options() {
	List* items = inventory_get_items();
	List* ops = ((PageListData*)(battle.gui.choice_items.options->data))->options;
	if (!items || !ops) return;
	gfc_list_foreach(ops, gui_free);
	gfc_list_clear(ops);
	for (int i = 0; i < gfc_list_get_count(items); i++) {
		Item* item = gfc_list_get_nth(items, i);
		gfc_list_append(ops, gui_option_create(vector2d(0, 0), item->name, i == 0, 2));
		GUI* lastOp = gfc_list_get_nth(ops, i);
		((OptionData*)(lastOp->data))->grayed = !(item->useTime & 1);
	}
	gui_page_list_refresh(battle.gui.choice_items.options);
}
void battle_switch_phase(BattlePhase phase) {
	switchPhase(phase);
}
int flee(void* unused) {
	battle_set_main_dialogue("Run for your lives!");
	switchPhase(BP_Ending);
	bgm_pause();
	return 1;
}
int changeDialogue(void* text) {
	switch (battle.phase) {
	case BP_Opening:
	case BP_PreAction:
		memcpy(((TextData*)(battle.gui.textDisplay.text->data))->text, *((TextBlock*)(text)), sizeof(TextBlock));
		break;
	case BP_RoundPrep:
		memcpy(((TextData*)(battle.gui.choice_initial.dialogueText->data))->text, *((TextBlock*)(text)), sizeof(TextBlock));
		break;
	}
	return 1;
}
void battle_handle_outcome() {
	if (battle_check_lose()) {
		switchPhase(BP_Ending);
		battle_set_main_dialogue("Your party has wiped out!");
		bgm_pause();
		Sound* failTune = gfc_sound_load("audio/sfx/Wiped Out.mp3", 1, 2);
		gfc_sound_play(failTune, 0, 1, -1, -1);
		gfc_sound_free(failTune);
		battle_wait(2500);
		return;
	}
	if (battle_check_win()) {
		switchPhase(BP_Ending);
		battle_set_main_dialogue("You win!");
		bgm_pause();
		Sound* victoryTune = gfc_sound_load("audio/sfx/Victory.mp3", 1, 2);
		gfc_sound_play(victoryTune, 0, 1, -1, -1);
		gfc_sound_free(victoryTune);
		battle_wait(1000);
		battle.victorious = true;
		battle.turn = 0;
		for (int i = 1; i <= party_get_member_count(); i++) {
			if (battle_get_party_member(1, i)->HP > 0)
				battle_get_party_member(1, i)->exp += battle.accumulatedEXP;
		}
		return;
	}
}
int startRound(void* unused) {
	switchPhase(BP_PreAction);
	((TextData*)(battle.gui.textDisplay.text->data))->scrolling = false;
	changeDialogue(&"");
	return 1;
}
int startRound_fight(void* unused) {
	for (int i = 1; i <= 4; i++) {
		FiendData* member = battle_get_party_member(1, i);
		if (member && member->HP > 0) {
			member->selectedSkill = get_skill(NULL);
			//member->skillTarget = vector2d(2, 0);
		}
	}
	battle.recruitAttempt++;
	battle.recruitChance = 0;
	startRound(NULL);
	return 1;
}
int startRound_recruit(void* target) {
	battle.recruitTarget = target;
	FiendData* impressee = battle_get_party_member(2, battle.recruitTarget);
	if (!impressee || impressee->HP <= 0) {
		battle.recruitTarget = 1;
		impressee = battle_get_party_member(2, battle.recruitTarget);
		while (!impressee || impressee->HP <= 0) {
			battle.recruitTarget++;
			if (battle.recruitTarget >= 5) return 0;
		}
	}
	for (int i = 1; i <= 4; i++) {
		FiendData* member = battle_get_party_member(1, i);
		if (member && member->HP > 0) {
			switch (member->type) {
			case FndT_Toughie:
				member->selectedSkill = get_skill("Toughie Recruit");
				break;
			case FndT_Trickster:
				member->selectedSkill = get_skill("Trickster Recruit");
				break;
			case FndT_Mage:
				member->selectedSkill = get_skill("Mage Recruit");
				break;
			}
			member->skillTarget = vector2d(2, battle.recruitTarget);
		}
	}
	battle.recruitAttempt++;
	battle.recruitChance = 0;
	startRound(NULL);
	return 1;
}

void load_battle_basics() {
	slog("Starting to load battle basics.");
	battle_sounds_init();
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
	battle.gui.choice_initial.opItem = gui_option_create(vector2d(9 + renderSize.x - 127, 81 + 14), "Items", false, 1);
	battle.gui.choice_initial.opFlee = gui_option_create(vector2d(9 + renderSize.x - 127 + 118 / 2, 81 + 14), "Flee", false, 1);
	TextBlock d = "";

	((OptionData*)(battle.gui.choice_initial.opFight->data))->left = battle.gui.choice_initial.opRecruit;
	((OptionData*)(battle.gui.choice_initial.opFight->data))->right = battle.gui.choice_initial.opRecruit;
	((OptionData*)(battle.gui.choice_initial.opFight->data))->up = battle.gui.choice_initial.opItem;
	((OptionData*)(battle.gui.choice_initial.opFight->data))->down = battle.gui.choice_initial.opItem;
	((OptionData*)(battle.gui.choice_initial.opFight->data))->onHover = changeDialogue;
	memcpy(textPresets[1], "Fight:\n    Engage the enemy", sizeof(TextBlock));
	((OptionData*)(battle.gui.choice_initial.opFight->data))->hoverArg1 = &textPresets[1];
	((OptionData*)(battle.gui.choice_initial.opFight->data))->onChoose = switchPhase;
	((OptionData*)(battle.gui.choice_initial.opFight->data))->choiceArg1 = BP_PreppingTactic;

	((OptionData*)(battle.gui.choice_initial.opRecruit->data))->left = battle.gui.choice_initial.opFight;
	((OptionData*)(battle.gui.choice_initial.opRecruit->data))->right = battle.gui.choice_initial.opFight;
	((OptionData*)(battle.gui.choice_initial.opRecruit->data))->up = battle.gui.choice_initial.opFlee;
	((OptionData*)(battle.gui.choice_initial.opRecruit->data))->down = battle.gui.choice_initial.opFlee;
	((OptionData*)(battle.gui.choice_initial.opRecruit->data))->onHover = changeDialogue;
	memcpy(textPresets[3], "Recruit:\n    Attempt to recruit\n    an enemy fiend", sizeof(TextBlock));
	((OptionData*)(battle.gui.choice_initial.opRecruit->data))->hoverArg1 = &textPresets[3];
	((OptionData*)(battle.gui.choice_initial.opRecruit->data))->onChoose = switchPhase;
	((OptionData*)(battle.gui.choice_initial.opRecruit->data))->choiceArg1 = BP_PreppingRecruit;

	((OptionData*)(battle.gui.choice_initial.opItem->data))->left = battle.gui.choice_initial.opFlee;
	((OptionData*)(battle.gui.choice_initial.opItem->data))->right = battle.gui.choice_initial.opFlee;
	((OptionData*)(battle.gui.choice_initial.opItem->data))->up = battle.gui.choice_initial.opFight;
	((OptionData*)(battle.gui.choice_initial.opItem->data))->down = battle.gui.choice_initial.opFight;
	((OptionData*)(battle.gui.choice_initial.opItem->data))->onHover = changeDialogue;
	memcpy(textPresets[3], "Item:\n    Use an Item.", sizeof(TextBlock));
	((OptionData*)(battle.gui.choice_initial.opItem->data))->hoverArg1 = &textPresets[3];
	((OptionData*)(battle.gui.choice_initial.opItem->data))->onChoose = switchPhase;
	((OptionData*)(battle.gui.choice_initial.opItem->data))->choiceArg1 = BP_PreppingItem;
	if (gfc_list_get_count(inventory_get_items()) == 0) {
		((OptionData*)(battle.gui.choice_initial.opItem->data))->grayed = true;
	}

	((OptionData*)(battle.gui.choice_initial.opFlee->data))->left = battle.gui.choice_initial.opItem;
	((OptionData*)(battle.gui.choice_initial.opFlee->data))->right = battle.gui.choice_initial.opItem;
	((OptionData*)(battle.gui.choice_initial.opFlee->data))->up = battle.gui.choice_initial.opRecruit;
	((OptionData*)(battle.gui.choice_initial.opFlee->data))->down = battle.gui.choice_initial.opRecruit;
	((OptionData*)(battle.gui.choice_initial.opFlee->data))->onHover = changeDialogue;
	memcpy(textPresets[5], "Flee:\n    Attempt to escape", sizeof(TextBlock));
	((OptionData*)(battle.gui.choice_initial.opFlee->data))->hoverArg1 = &textPresets[5];
	((OptionData*)(battle.gui.choice_initial.opFlee->data))->onChoose = flee;
	memcpy(textPresets[6], "Whoops, can't do that yet.", sizeof(TextBlock));
	((OptionData*)(battle.gui.choice_initial.opFlee->data))->choiceArg1 = &textPresets[6];

	//Targeting GUI
	battle.gui.target_enemy.dialogueFrame = gui_window_create(vector2d(4, 76 + 13), vector2d(renderSize.x - 8, renderSize.y / 2 - 8 - 28 - 13), 0);
	battle.gui.target_enemy.dialogueText = gui_text_create(vector2d(9, 81 + 13), "Select target.", false, 1);
	for (int i = 0; i < foes; i++) {
		Entity* t = battle_get_party_member(2, i+1)->entity;
		int x = t->position.x + t->sprite->frame_w / 2 - 2;
		int y = t->position.y + t->sprite->frame_h + 10;
		battle.gui.target_enemy.options[i] = gui_option_create(vector2d(x, y), "", false, 1);
		((OptionData*)(battle.gui.target_enemy.options[i]->data))->isPointingUp = true;
	}
	battle.gui.target_ally.dialogueFrame = gui_window_create(vector2d(4, 76), vector2d(renderSize.x - 8, renderSize.y / 2 - 8 - 28 - 13), 0);
	battle.gui.target_ally.dialogueText = gui_text_create(vector2d(9, 81), "Select target.", false, 1);
	for (int i = 0; i < party_get_member_count(); i++) {
		battle.gui.target_ally.options[i] = gui_option_create(vector2d(renderSize.x / 8 + (renderSize.x / 4 * i) + 2, renderSize.y - 48 - 10), "", false, 1);
		((OptionData*)(battle.gui.target_ally.options[i]->data))->isPointingDown = true;
	}

	//Tactics GUI
	battle.gui.choice_tactic.dialogueFrame = gui_window_create(vector2d(4, 76), vector2d(renderSize.x - 128, renderSize.y / 2 - 8 - 28), 0);
	battle.gui.choice_tactic.dialogueText = gui_text_create(vector2d(9, 81), "", false, 1);
	battle.gui.choice_tactic.optionFrame = gui_window_create(vector2d(4 + renderSize.x - 127, 76), vector2d(120, renderSize.y / 2 - 8 - 28), 0);
	battle.gui.choice_tactic.opDestruction = gui_option_create(vector2d(9 + renderSize.x - 127, 81), "Destroy", false, 1);
	battle.gui.choice_tactic.opChaos = gui_option_create(vector2d(9 + renderSize.x - 127 + 118 / 2, 81), "Chaos", false, 1);
	battle.gui.choice_tactic.opSupport = gui_option_create(vector2d(9 + renderSize.x - 127, 81 + 14), "Support", false, 1);
	battle.gui.choice_tactic.opCommand = gui_option_create(vector2d(9 + renderSize.x - 127 + 118 / 2, 81 + 14), "Order", false, 1);
	battle.gui.choice_tactic.opCommand->color = gfc_color(0.5, 0, 1, 1);
	OptionData* destruction = battle.gui.choice_tactic.opDestruction->data;
	OptionData* chaos = battle.gui.choice_tactic.opChaos->data;
	OptionData* support = battle.gui.choice_tactic.opSupport->data;
	OptionData* comd = battle.gui.choice_tactic.opCommand->data;
	destruction->left = battle.gui.choice_tactic.opChaos;
	destruction->right = battle.gui.choice_tactic.opChaos;
	destruction->down = battle.gui.choice_tactic.opSupport;
	destruction->up = battle.gui.choice_tactic.opSupport;
	destruction->onChoose = fiend_change_tactic;
	destruction->choiceArg1 = Destruction;
	
	chaos->left = battle.gui.choice_tactic.opDestruction;
	chaos->right = battle.gui.choice_tactic.opDestruction;
	chaos->down = battle.gui.choice_tactic.opCommand;
	chaos->up = battle.gui.choice_tactic.opCommand;
	chaos->onChoose = fiend_change_tactic;
	chaos->choiceArg1 = Chaos;
	
	support->left = battle.gui.choice_tactic.opCommand;
	support->right = battle.gui.choice_tactic.opCommand;
	support->down = battle.gui.choice_tactic.opDestruction;
	support->up = battle.gui.choice_tactic.opDestruction;
	support->onChoose = fiend_change_tactic;
	support->choiceArg1 = Support;
	
	comd->left = battle.gui.choice_tactic.opSupport;
	comd->right = battle.gui.choice_tactic.opSupport;
	comd->down = battle.gui.choice_tactic.opChaos;
	comd->up = battle.gui.choice_tactic.opChaos;
	comd->onChoose = switchPhase;
	comd->choiceArg1 = BP_PreppingAction;

	//Skills GUI
	int halfX = (renderSize.x - 8) / 2;
	int unitY = (renderSize.y / 2 - 8) / 5;
	battle.gui.choice_command.dialogueFrame = gui_window_create(vector2d(4, 76), vector2d(renderSize.x - 8, renderSize.y / 2 - 8 - 28), 0);
	battle.gui.choice_command.optionFrame = gui_window_create(vector2d(4, 4), vector2d(renderSize.x - 8, renderSize.y / 2 - 8), 0);
	battle.gui.choice_command.dialogueText = gui_text_create(vector2d(9, renderSize.y / 2 + 1), "", false, 1);
	for (int i = 0; i < 10; i++) {
		battle.gui.choice_command.opSkills[i] = gui_option_create(vector2d(9 + halfX * (i % 2), 9 + (int)(i / 2) * unitY), "Dummy", i == 0, 1);
		if (i > 1) {
			battle.gui.choice_command.manaCosts[i - 2] = gui_text_create(vector2d(9 + halfX * (i % 2 + 1) - 25, 9 + (int)(i / 2) * unitY), "0", false, 1);
			battle.gui.choice_command.manaCosts[i - 2]->color = gfc_color(0, 0, 1, 1);
		}
	}
	for (int i = 0; i < 10; i++) {
		OptionData* dat = battle.gui.choice_command.opSkills[i]->data;
		dat->onChoose = fiend_pick_skill;
		dat->choiceArg1 = i;
		dat->onHover = battle_set_skill_dialogue;
		dat->left = (i % 2 == 0 ? battle.gui.choice_command.opSkills[i + 1] : battle.gui.choice_command.opSkills[i - 1]);
		dat->right = (i % 2 == 0 ? battle.gui.choice_command.opSkills[i + 1] : battle.gui.choice_command.opSkills[i - 1]);
		dat->up = (i < 2 ? battle.gui.choice_command.opSkills[i + 8] : battle.gui.choice_command.opSkills[i - 2]);
		dat->down = (i >= 8 ? battle.gui.choice_command.opSkills[i - 8] : battle.gui.choice_command.opSkills[i + 2]);
	}

	//Item GUI
	
	battle.gui.choice_items.dialogueFrame = gui_window_create(vector2d(4, 76), vector2d(renderSize.x - 128, renderSize.y / 2 - 8 - 28), 0);
	battle.gui.choice_items.optionFrame = gui_window_create(vector2d(4 + renderSize.x - 127, 76-14), vector2d(120, renderSize.y / 2 - 8 - 28+14), 0);
	battle.gui.choice_items.dialogueText = gui_text_create(vector2d(9, 81), "", false, 1);
	battle.gui.choice_items.options = gui_page_list_create(vector2d(4 + renderSize.x - 127, 76 - 14 + 8), max(gfc_list_get_count(inventory_get_items()), 1), 4, 120 - 17, 1);

	//Player Ally GUI
	for (int i = 0; i < party_get_member_count(); i++) {
		FiendData* dat = party_read_member_data(i);
		char* numBuf[4];
		battle.gui.allyGUI[i].frame = gui_window_create(vector2d(0 + (renderSize.x / 4 * i), renderSize.y - 48), vector2d(renderSize.x / 4, 60), 0);
		battle.gui.allyGUI[i].name = gui_text_create(vector2d(6 + (renderSize.x / 4 * i), renderSize.y - 44), dat->name, false, 1);
		sprintf(numBuf, "%i", dat->HP);
		battle.gui.allyGUI[i].hNum = gui_text_create(vector2d(4 + (renderSize.x / 4 * i), renderSize.y - 28), numBuf, false, 2);
		sprintf(numBuf, "%i", dat->MP);
		battle.gui.allyGUI[i].mNum = gui_text_create(vector2d(4 + (renderSize.x / 4 * i), renderSize.y - 15), numBuf, false, 2);
		battle.gui.allyGUI[i].hBar = gui_meter_create(vector2d(5 + 24 + (renderSize.x / 4 * i), renderSize.y - 27), vector2d(renderSize.x / 4 - 32, 8), gfc_color(1, 0, 0, 1), 1);
		battle.gui.allyGUI[i].mBar = gui_meter_create(vector2d(5 + 24 + (renderSize.x / 4 * i), renderSize.y - 14), vector2d(renderSize.x / 4 - 32, 8), gfc_color(0, 0, 0.8, 1), 1);
		((MeterData*)(battle.gui.allyGUI[i].hBar->data))->fill = (float)dat->HP / (float)dat->stats[MHP];
		((MeterData*)(battle.gui.allyGUI[i].mBar->data))->fill = (float)dat->MP / (float)dat->stats[MMP];

		battle.gui.allyGUI[i].frame->visible = true;
		battle.gui.allyGUI[i].name->visible = true;
		battle.gui.allyGUI[i].hNum->visible = true;
		battle.gui.allyGUI[i].mNum->visible = true;
		battle.gui.allyGUI[i].hBar->visible = true;
		battle.gui.allyGUI[i].mBar->visible = true;
	}

	battle.phase = BP_Opening;
	battle.recruitAttempt = 0;
	battle.recruitChance = 0;
	battle.recruitTarget = 0;
	battle.recruiting = false;
	battle.currentFiend = 1;
	battle.itemUsed = -1;
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
	sprintf(sbuf, dungeon);
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
		foe->type = ET_Fiend;
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

		calculate_stats(data);
		data->HP = data->stats[MHP];
		data->MP = data->stats[MMP];

		data->entity = foe;

		fiend_check_new_skills(data);

		num++;
	}

	for (int i = 1; i <= 3; i++) {
		FiendData* foeDat = battle_get_party_member(2, i);
		if (!foeDat) break;
		int dup = 0;
		for (int j = i + 1; j <= 4; j++) {
			FiendData* tgtFoe = battle_get_party_member(2, j);
			if (!tgtFoe) break;
			if (!gfc_line_cmp(foeDat->name, tgtFoe->name)) {
				dup += 1;
				sprintf(tgtFoe->name, "%s %c", tgtFoe->name, 'A' + dup);
			}
		}
		if (dup) {
			sprintf(foeDat->name, "%s %c", foeDat->name, 'A');
		}
	}

	int xOffset = 150 - (pos - 4) / 2;
	for (int i = 0; i < num; i++) {
		vector2d_add(battle.battlers[i]->position, battle.battlers[i]->position, vector2d(xOffset, 0));
	}
	sj_free(battleFile);
	for (int i = 0; i < party_get_member_count(); i++) {
		battle.battlers[num + i] = party_read_member(i);
		((FiendData*)(battle.battlers[num + i]->data))->party = 1;
	}
	battle.active = true;
	load_battle_basics();
	camera_set_position(vector2d(0,0));
	game_set_state(GS_Battle);
	bgm_play_loop(BGM_Battle);
}

SJson* pluck_rival(SJson* fullOptions, TextWord specialty, int* pointsLeft, int* pointCost) {
	TextWord type, to = "toughie", tr = "trickster", ma = "mage";
	gfc_word_cpy(type, specialty);
	if (!gfc_word_cmp(specialty, "none")) {
		int r = min(gfc_random() * 3, 2);
		switch (r) {
		case 0:
			gfc_word_cpy(type, to);
			break;
		case 1:
			gfc_word_cpy(type, tr);
			break;
		case 2:
			gfc_word_cpy(type, ma);
			break;
		}
	}
	slog("%s, %s", specialty, type);
	SJson* options = sj_object_get_value(fullOptions, type);
	int opCount = sj_array_get_count(options);
	if (opCount <= 0) return NULL;
	int pick = min((int)(gfc_random() * opCount), opCount - 1);
	slog("Picking option %i", pick);
	SJson* chosen = sj_array_get_nth(options, pick);
	if (!chosen) {
		slog("Something went wrong with the enemy picker.");
		return NULL;
	}
	int points = 0;
	sj_get_integer_value(sj_object_get_value(chosen, "weight"), &points);
	slog("Weight of option is %i", points);
	*pointCost = points;
	*pointsLeft -= points;
	if (*pointsLeft < 0) return NULL; //went over
	slog("Chose a rando.");
	return chosen;
}
void generate_new_rival_battle(TextWord specialty, int difficulty) {
	SJson* battleFile = sj_load("config/rivalFiends.cfg");
	if (!battleFile) {
		slog("Could not load rivalFiends.cfg!");
		return;
	}
	int qMax = 25, qModLast;
	int num = 0;
	int pos = 0;
	for (int slots = 4, errors = 0; slots > 0 && errors < 3;) {
		slog("Trying new pluck iteration.");
		if (qMax <= 0)
			break;
		SJson* search = pluck_rival(battleFile, specialty, &qMax, &qModLast);
		if (qMax < 0)
			break;
		if (!search) {
			slog("Search came up empty.");
			errors++;
			continue;
		}
		FiendData* data = read_fiend(sj_get_string_value(sj_object_get_value(search, "species")));
		if (data->size > slots) {
			slog("Fiend was too big!");
			free(data);
			errors++;
			continue;
		}
		int level = 1;
		sj_get_integer_value(sj_array_get_nth(sj_object_get_value(search, "level"), difficulty-1), &level);
		if (level == 0) { //Fiend not available at this difficulty.
			slog("Fiend cannot be at level 0.");
			qMax += qModLast;
			free(data);
			continue;
		}
		Entity* foe = entity_new();
		foe->type = ET_Fiend;
		slog("Fiend entity created.");
		slog("Fiend @ level %i.", level);
		int emin, emax;
		sj_get_integer_value(sj_array_get_nth(sj_object_get_value(search, "exp"),0), &emin);
		sj_get_integer_value(sj_array_get_nth(sj_object_get_value(search, "exp"),1), &emax);
		int exp = calculate_stat(emin, emax, level);
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

		calculate_stats(data);
		data->HP = data->stats[MHP];
		data->MP = data->stats[MMP];

		data->entity = foe;

		fiend_check_new_skills(data);

		num++;
	}

	for (int i = 1; i <= 3; i++) {
		FiendData* foeDat = battle_get_party_member(2, i);
		if (!foeDat) break;
		int dup = 0;
		for (int j = i + 1; j <= 4; j++) {
			FiendData* tgtFoe = battle_get_party_member(2, j);
			if (!tgtFoe) break;
			if (!gfc_line_cmp(foeDat->name, tgtFoe->name)) {
				dup += 1;
				sprintf(tgtFoe->name, "%s %c", tgtFoe->name, 'A' + dup);
			}
		}
		if (dup) {
			sprintf(foeDat->name, "%s %c", foeDat->name, 'A');
		}
	}

	int xOffset = 150 - (pos - 4) / 2;
	for (int i = 0; i < num; i++) {
		vector2d_add(battle.battlers[i]->position, battle.battlers[i]->position, vector2d(xOffset, 0));
	}
	sj_free(battleFile);
	for (int i = 0; i < party_get_member_count(); i++) {
		battle.battlers[num + i] = party_read_member(i);
		((FiendData*)(battle.battlers[num + i]->data))->party = 1;
	}
	battle.active = true;
	load_battle_basics();
	((OptionData*)(battle.gui.choice_initial.opFlee->data))->grayed = true;
	((OptionData*)(battle.gui.choice_initial.opRecruit->data))->grayed = true;
	camera_set_position(vector2d(0, 0));
	game_set_state(GS_Battle);
	bgm_play_loop(BGM_Battle);
}
void load_boss_battle(char* bossName) {
	SJson* battleFile = sj_load("config/bosses.cfg");
	if (!battleFile) {
		slog("Could not load battles.cfg!");
		return;
	}
	SJson* bossData = sj_object_get_value(sj_object_get_value(battleFile, bossName), "battle");
	if (!bossData) {
		slog("Could not find battle data in bosses.cfg!");
		sj_free(battleFile);
		return;
	}
	int num = 0;
	int pos = 0;
	for (; num < sj_array_get_count(bossData);) {
		SJson* particular = sj_array_get_nth(bossData, num);
		FiendData* data = read_fiend(sj_get_string_value(sj_object_get_value(particular, "species")));
		Entity* foe = entity_new();
		foe->type = ET_Fiend;
		slog("Fiend entity created.");
		int level = 1;
		sj_get_integer_value(sj_object_get_value(particular, "level"), &level);
		slog("Fiend @ level %i.", level);
		int exp = 0;
		sj_get_integer_value(sj_object_get_value(particular, "exp"), &exp);
		slog("Fiend has %i exp worth.", exp);
		data->exp = exp;
		data->level = level;
		data->party = 2;
		sj_get_integer_value(sj_object_get_value(particular, "boss sound"), &(data->hasBossSound));
		gfc_line_cpy(data->species, sj_get_string_value(sj_object_get_value(particular, "species")));
		gfc_line_cpy(data->name, sj_get_string_value(sj_object_get_value(particular, "name")));
		slog("Copied boss name.");
		foe->data = data;
		slog("Copied reference to fiend data.");
		battle.battlers[num] = foe;
		slog("Fiend added to list of battlers.");
		foe->sprite = data->sprite;

		foe->position = vector2d(pos, 8);
		pos += foe->sprite->frame_w + 4;

		//calculate_stats(data);
		for (int i = 0; i < 6; i++) {
			sj_get_integer_value(sj_array_get_nth(sj_object_get_value(particular, "stats"), i), &(data->stats[i]));
		}
		data->HP = data->stats[MHP];
		data->MP = data->stats[MMP];

		TextWord tactic;
		gfc_word_cpy(tactic, sj_get_string_value(sj_object_get_value(particular, "tactic")));
		if (!gfc_word_cmp(tactic, "destruction")) {
			data->tactic = Destruction;
			data->plan = plan_destruction;
			slog("Plan set to destruction.");
		}
		if (!gfc_word_cmp(tactic, "chaos")) {
			data->tactic = Chaos;
			data->plan = plan_chaos;
		}
		if (!gfc_word_cmp(tactic, "support")) {
			data->tactic = Support;
		}
		if (!gfc_word_cmp(tactic, "magicless")) {
			data->tactic = Magicless;
		}
		SJson* skills = sj_object_get_value(particular, "skills");
		if (skills && sj_is_array(skills)) {
			for (int i = 0; i < sj_array_get_count(skills); i++) {
				data->skills[i + 2] = get_skill(sj_get_string_value(sj_array_get_nth(skills, i)));
			}
		}

		data->entity = foe;

		num++;
	}

	int xOffset = 150 - (pos - 4) / 2;
	for (int i = 0; i < num; i++) {
		vector2d_add(battle.battlers[i]->position, battle.battlers[i]->position, vector2d(xOffset, 0));
	}
	for (int i = 0; i < party_get_member_count(); i++) {
		battle.battlers[num + i] = party_read_member(i);
		((FiendData*)(battle.battlers[num + i]->data))->party = 1;
	}
	battle.active = true;
	load_battle_basics();
	battle_set_main_dialogue(sj_get_string_value(sj_object_get_value(sj_object_get_value(battleFile, bossName), "dialogue")));
	((OptionData*)(battle.gui.choice_initial.opFlee->data))->grayed = true;
	((OptionData*)(battle.gui.choice_initial.opRecruit->data))->grayed = true;
	sj_free(battleFile);
	camera_set_position(vector2d(0, 0));
	game_set_state(GS_Battle);
	bgm_play_loop(BGM_Boss);
}

int battle_handle_recruit_outcome(int* phase) {
	switch (*phase) {
	case 0:
	{
		slog("Attempting to recruit.");
		TextBlock t;
		sprintf(t, "%s is trying to decide.", battle_get_party_member(2, battle.recruitTarget)->name);
		battle_set_main_dialogue(t);
		if (fabsf(battle.recruitChance - 0.5) < 0.15)
			battle_wait(600);
		else if (fabsf(battle.recruitChance - 0.5) < 0.4)
			battle_wait(400);
		else
			battle_wait(200);
		*phase += 1;
		return 0;
	}
	case 1:
	{
		TextBlock t;
		sprintf(t, "%s is trying to decide..", battle_get_party_member(2, battle.recruitTarget)->name);
		battle_set_main_dialogue(t);
		if (fabsf(battle.recruitChance - 0.5) < 0.15)
			battle_wait(600);
		else if (fabsf(battle.recruitChance - 0.5) < 0.4)
			battle_wait(400);
		else
			battle_wait(200);
		*phase += 1;
		return 0;
	}
	case 2:
	{
		TextBlock t;
		sprintf(t, "%s is trying to decide...", battle_get_party_member(2, battle.recruitTarget)->name);
		battle_set_main_dialogue(t);
		battle_wait(800);
		*phase += 1;
		return 0;
	}
	case 3:
	{
		float decision = gfc_random();
		if (decision >= 1)
			decision = 0.999999f;
		if (decision < battle.recruitChance)
			battle.recruited = true;
		TextBlock t;
		if(battle.recruited)
			sprintf(t, "%s has decided to join you!", battle_get_party_member(2, battle.recruitTarget)->name);
		else
			sprintf(t, "Recruitment failed.");
		battle_set_main_dialogue(t);
		battle_wait(1000);
		*phase = 0;
		return 1;
	}
	}
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
	case BP_PreppingRecruit: 
		if (gfc_input_controller_button_pressed(0, "circle"))
			switchPhase(BP_RoundPrep);
		break;
	case BP_PreppingTactic:
		if (!battle_get_party_member(1, battle.currentFiend))
			switchPhase(BP_PreAction);
		if (gfc_input_controller_button_pressed(0, "circle")) {
			if (battle.currentFiend > 1) {
				battle.currentFiend--;
				switchPhase(BP_PreppingTactic);
			}
			else
				switchPhase(BP_RoundPrep);
		}
		break;
	case BP_PreppingAction:
		if (gfc_input_controller_button_pressed(0, "circle"))
			switchPhase(BP_PreppingTactic);
		break;
	case BP_PreAction:
	{
		battle.recruiting = false;
		int foe = 1, ally = 1, moveNum = 0;
		for (int i = 0; i < 8; i++) {
			if (!battle.battlers[i]) continue;
			if(((FiendData*)(battle.battlers[i]->data))->HP <= 0) {
				if (((FiendData*)(battle.battlers[i]->data))->party == 1)
					ally++;
				if (((FiendData*)(battle.battlers[i]->data))->party == 2)
					foe++;
				continue;
			}
			if (((FiendData*)(battle.battlers[i]->data))->party == 2) {
				if (((FiendData*)(battle.battlers[i]->data))->plan) {
					((FiendData*)(battle.battlers[i]->data))->plan(battle.battlers[i]->data);
					battle.moveOrder[moveNum] = ((FiendData*)(battle.battlers[i]->data))->selectedSkill;
					battle.moveOrderUT[moveNum] = vector4d(2, foe, ((FiendData*)(battle.battlers[i]->data))->skillTarget.x, ((FiendData*)(battle.battlers[i]->data))->skillTarget.y);
				}
				else {
					battle.moveOrder[moveNum] = get_skill("Attack");
					battle.moveOrderUT[moveNum] = vector4d(2, foe, 1, 0);
				}
				battle.moveOrderDeterminer[moveNum] = ((FiendData*)(battle.battlers[i]->data))->stats[AGL] * (1 + gfc_crandom() * 0.16) + battle.moveOrder[moveNum].turnBoost;
				slog("Foe %i chose a move on %f, %f", foe, battle.moveOrderUT[moveNum].z, battle.moveOrderUT[moveNum].w);
				foe++;
			}
			if (((FiendData*)(battle.battlers[i]->data))->party == 1) {
				if (((FiendData*)(battle.battlers[i]->data))->selectedSkill.chosen) {
					battle.moveOrder[moveNum] = ((FiendData*)(battle.battlers[i]->data))->selectedSkill;
					battle.moveOrderUT[moveNum] = vector4d(1, ally, ((FiendData*)(battle.battlers[i]->data))->skillTarget.x, ((FiendData*)(battle.battlers[i]->data))->skillTarget.y);
				}
				else if (((FiendData*)(battle.battlers[i]->data))->plan) {
					((FiendData*)(battle.battlers[i]->data))->plan(battle.battlers[i]->data);
					battle.moveOrder[moveNum] = ((FiendData*)(battle.battlers[i]->data))->selectedSkill;
					battle.moveOrderUT[moveNum] = vector4d(1, ally, ((FiendData*)(battle.battlers[i]->data))->skillTarget.x, ((FiendData*)(battle.battlers[i]->data))->skillTarget.y);
				}
				else {
					battle.moveOrder[moveNum] = get_skill("Attack");
					battle.moveOrderUT[moveNum] = vector4d(1, ally, 2, 0);
				}
				/*if (((FiendData*)(battle.battlers[i]->data)) == battle_get_party_member(1,1)) {
					battle.moveOrder[moveNum] = get_skill("umbraboom");
					battle.moveOrderUT[moveNum] = vector4d(1, ally, 2, 0);
				}*/
				battle.moveOrderDeterminer[moveNum] = ((FiendData*)(battle.battlers[i]->data))->stats[AGL] * (1 + gfc_crandom() * 0.16) + battle.moveOrder[moveNum].turnBoost;
				slog("Ally %i chose a move on %f, %f", ally, battle.moveOrderUT[moveNum].z, battle.moveOrderUT[moveNum].w);
				ally++;
			}
			moveNum++;
		}
		slog("Got chosen moves.");
		for (Bool sorted = false; !sorted;) {
			sorted = true;
			for (int i = 1; i < 8; i++) {
				if (!battle.battlers[i]) break;
				if (battle.moveOrderDeterminer[i] > battle.moveOrderDeterminer[i - 1]) {
					sorted = false;
					Skill tmpS;
					tmpS = skill_copy(battle.moveOrder[i]);
					battle.moveOrder[i] = skill_copy(battle.moveOrder[i-1]);
					battle.moveOrder[i - 1] = skill_copy(tmpS);
					Vector4D tmpV;
					vector4d_copy(tmpV, battle.moveOrderUT[i]);
					vector4d_copy(battle.moveOrderUT[i], battle.moveOrderUT[i-1]);
					vector4d_copy(battle.moveOrderUT[i-1], tmpV);
					int tmpI = battle.moveOrderDeterminer[i];
					battle.moveOrderDeterminer[i] = battle.moveOrderDeterminer[i - 1];
					battle.moveOrderDeterminer[i - 1] = tmpI;
				}
			}
		}
		slog("Move order:");
		for (int i = 0; i < moveNum; i++) {
			if(battle.moveOrder[i].inUse && battle_get_party_member(battle.moveOrderUT[i].x, battle.moveOrderUT[i].y))
				slog("   %i: %s %s on %i-%i", i, battle_get_party_member(battle.moveOrderUT[i].x,battle.moveOrderUT[i].y)->name, battle.moveOrder[i].name,
				(int)(battle.moveOrderUT[i].z), (int)(battle.moveOrderUT[i].w));
		}
		battle.turn = 0;
		switchPhase(BP_Acting);
		slog("READY TO ACT NOW!");
		break;
	}
	case BP_Acting:
		if (battle.timeForMove <= SDL_GetTicks64()) {
			if (battle.recruiting && (battle.turn >= 8 || !battle.moveOrder[battle.turn].chosen || battle.moveOrder[battle.turn].flag != SF_Recruiting) && battle_handle_recruit_outcome(&battle.turnPhase)) {
				if (battle.recruited) {
					switchPhase(BP_Ending);
					bgm_pause();
					Sound* victoryTune = gfc_sound_load("audio/sfx/Victory.mp3", 1, 2);
					gfc_sound_play(victoryTune, 0, 1, -1, -1);
					gfc_sound_free(victoryTune);
				}
				else {
					battle.recruiting = false;
				}
			}
			else if ((battle.moveOrder[battle.turn].chosen && battle.moveOrder[battle.turn].flag == SF_Recruiting) || !(battle.recruiting && (battle.turn >= 8 || !battle.moveOrder[battle.turn].chosen || battle.moveOrder[battle.turn].flag != SF_Recruiting))) {
				if (battle.turn >= 8 || !battle.battlers[battle.turn]) {
					switchPhase(BP_PostAction);
					break;
				}
				slog("Turn %i-%i.", battle.turn, battle.turnPhase);
				if (!battle.moveOrder[battle.turn].chosen || 
					((battle.turnPhase == 0 && battle_get_party_member(battle.moveOrderUT[battle.turn].x, battle.moveOrderUT[battle.turn].y)->HP == 0) ||
					battle.moveOrder[battle.turn].perform((int)(battle.moveOrderUT[battle.turn].x), (int)(battle.moveOrderUT[battle.turn].y),
						(int)(battle.moveOrderUT[battle.turn].z), (int)(battle.moveOrderUT[battle.turn].w), &battle.turnPhase)))
				{
					battle.turn++;
					battle_handle_outcome();
				}
				if (!(battle.turn < 8 && battle.battlers[battle.turn])) {
					battle_wait(1000);
				}
			}
			
		}
		break;
	case BP_PostAction:
		for (int i = 0; i < 8; i++) {
			if (battle.battlers[i]) {
				((FiendData*)(battle.battlers[i]->data))->selectedSkill = get_skill(NULL);
			}
			battle.moveOrder[i] = get_skill(NULL);
			battle.moveOrderDeterminer[i] = 0;
			battle.moveOrderUT[i] = vector4d(0, 0, 0, 0);
		}
		battle.recruiting = false;
		switchPhase(BP_RoundPrep);
		break;
	case BP_Ending:
		if (battle.timeForMove <= SDL_GetTicks64()) {
			if (battle.victorious) {
				if (gfc_input_controller_button_pressed_by_index(0, 0))
				{
					switch (battle.turn) {
					case 0: {
						TextBlock disp = "";
						sprintf(disp, "Your party gains %i exp.", battle.accumulatedEXP);
						battle_set_main_dialogue(disp);
						battle.turn++;
						battle.turnPhase = 0;
						break;
					}
					case 1:
					case 2:
					case 3:
					case 4: 
						if (!battle_get_party_member(1, battle.turn)) {
							battle.turn++;
							battle.turnPhase = 0;
						}
						else {
							static Bool leveledUp = false;
							if (battle.turnPhase == 0)
								leveledUp = false;
							int doneStep = fiend_check_level_up(battle_get_party_member(1, battle.turn), &(((TextData*)(battle.gui.textDisplay.text->data))->text), &(battle.turnPhase), &leveledUp);
							if (battle.turnPhase == 1 && leveledUp) {
								battle_wait(600);
								//Play sound here.
							}
							if (doneStep) {
								battle.turn++;
								battle.turnPhase = 0;
							}
							if (leveledUp)
								break;
						}
					case 5:
						//Monetary gain here!
						battle.turn++;
						//break;
					case 6:
						kill_battle();
						return;
					}
				}
			}
			else {
				if (gfc_input_controller_button_pressed_by_index(0, 0))
				{
					kill_battle();
					return;
				}
			}
		}
	}
	for (int i = 0; i < party_get_member_count(); i++) {
		FiendData* dat = party_read_member_data(i);
		char* numBuf[4];
		sprintf(numBuf, "%i", dat->HP);
		memcpy(((TextData*)(battle.gui.allyGUI[i].hNum->data))->text, numBuf, sizeof(char) * (dat->HP >= 100 ? 4 : (dat->HP >= 10 ? 3 : 2)));
		sprintf(numBuf, "%i", dat->MP);
		memcpy(((TextData*)(battle.gui.allyGUI[i].mNum->data))->text, numBuf, sizeof(char) * (dat->MP >= 100 ? 4 : (dat->MP >= 10 ? 3 : 2)));
		((MeterData*)(battle.gui.allyGUI[i].hBar->data))->fill = (float)dat->HP / (float)dat->stats[MHP];
		((MeterData*)(battle.gui.allyGUI[i].mBar->data))->fill = (float)dat->MP / (float)dat->stats[MMP];
	}
}


float battle_get_recruit_chance() {
	return battle.recruitChance;
}

void battle_add_exp(int exp) {
	battle.accumulatedEXP += exp;
}