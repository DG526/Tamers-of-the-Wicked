#include "simple_logger.h"

#include "totw_game_status.h"
#include "totw_fiend_management.h"

int party_manager_close(Entity* partyManager) {
	game_set_state(((PartyManagerData*)(partyManager->data))->returnState);
	party_manager_free(partyManager);
	return 0;
}

int changeFiendToSwap(Entity* partyManager, int changeTo);
int chooseFiendToSwap(Entity* partyManager);
int swapFiend(Entity* partyManager, FiendData* newMember);
int swapSprite2(Entity* partyManager, Sprite* newSprite);
int close_party_manager(Entity* partyManager);

Entity* party_manager_new() {
	Entity* sys = entity_new();
	if (!sys) return NULL;
	PartyManagerData* data = gfc_allocate_array(sizeof(PartyManagerData), 1);
	if (!data) {
		entity_free(sys);
		return NULL;
	}
	sys->data = data;

	data->returnState = game_get_state();

	data->panels[0] = gui_window_create(vector2d(-8, -8), vector2d(game_get_resolution_x() + 16, game_get_resolution_y() + 16), 0);
	data->panels[1] = gui_window_create(vector2d(game_get_resolution_x() / 20, 20), vector2d(game_get_resolution_x() / 10 * 4, game_get_resolution_y() - 20), 1);
	data->panels[2] = gui_window_create(vector2d(game_get_resolution_x() / 20*11, 20), vector2d(game_get_resolution_x() / 10 * 4, game_get_resolution_y() - 20), 1);
	data->panels[3] = gui_window_create(vector2d(0, 0), vector2d(game_get_resolution_x() / 2, 76), 2);
	data->panels[4] = gui_window_create(vector2d(game_get_resolution_x() / 2, 0), vector2d(game_get_resolution_x() / 2, 76), 2);
	for (int i = 0; i < 5; i++) {
		data->panels[i]->visible = true;
	}
	TextWord slots = "";
	sprintf(slots, "Slots: %i/4", party_get_slots_used());
	data->slotDisplay = gui_text_create(vector2d(game_get_resolution_x() / 20 + game_get_resolution_x() / 5, 76 + 3), slots, false, 3);
	data->slotDisplay->visible = true;
	data->slotDisplay->color = (party_get_slots_used() > 0 && party_get_slots_used() <= 4) ? gfc_color(0, 0.8, 0, 1) : gfc_color(0.8, 0, 0, 1);
	for (int i = 0; i < 4; i++) {
		data->partyOptions[i] = gui_option_create(vector2d(game_get_resolution_x() / 20 + 4, 76 + 17 + i * 14), party_read_member_data(i) ? party_read_member_data(i)->name : "(None)", i == 0, 3);
		((OptionData*)(data->partyOptions[i]->data))->onHover = changeFiendToSwap;
		((OptionData*)(data->partyOptions[i]->data))->hoverArg1 = sys;
		((OptionData*)(data->partyOptions[i]->data))->hoverArg2 = i;
		((OptionData*)(data->partyOptions[i]->data))->onChoose = chooseFiendToSwap;
		((OptionData*)(data->partyOptions[i]->data))->choiceArg1 = sys;
		((OptionData*)(data->partyOptions[i]->data))->grayed = i > party_get_member_count();
	}
	data->partyOptions[4] = gui_option_create(vector2d(game_get_resolution_x() / 20 + 4, 76 + 17 + 4 * 14 + 6), "Confirm Party", false, 3);
	((OptionData*)(data->partyOptions[4]->data))->onChoose = close_party_manager;
	((OptionData*)(data->partyOptions[4]->data))->choiceArg1 = sys;
	((OptionData*)(data->partyOptions[4]->data))->grayed = !(party_get_slots_used() > 0 && party_get_slots_used() <= 4);
	for (int i = 0; i < 5; i++) {
		data->partyOptions[i]->visible = true;
		OptionData* oData = data->partyOptions[i]->data;
		oData->down = i < 4 ? data->partyOptions[i + 1] : data->partyOptions[0];
		oData->up = i > 0 ? data->partyOptions[i - 1] : data->partyOptions[4];
	}
	data->menagerieList = gui_page_list_create(vector2d(game_get_resolution_x() / 20 * 11 + 8, 78), menagerie_get_member_count() + 1, 7, game_get_resolution_x() / 10 * 4 - 17, 4);
	slog("Created Menagerie List.");
	PageListData* pages = (PageListData*)(data->menagerieList->data);
	//pages->options->count = menagerie_get_member_count() + 1;
	gfc_list_append(pages->options, gui_option_create(vector2d(0, 0), "(Remove)", false, 3));
	((OptionData*)((GUI*)(gfc_list_get_nth(pages->options, 0)))->data)->choiceArg1 = sys;
	((OptionData*)((GUI*)(gfc_list_get_nth(pages->options, 0)))->data)->choiceArg2 = NULL;
	((OptionData*)((GUI*)(gfc_list_get_nth(pages->options, 0)))->data)->choiceArg3 = gfc_list_get_nth(pages->options, 0);
	((OptionData*)((GUI*)(gfc_list_get_nth(pages->options, 0)))->data)->onChoose = swapFiend;
	((OptionData*)((GUI*)(gfc_list_get_nth(pages->options, 0)))->data)->hoverArg1 = sys;
	((OptionData*)((GUI*)(gfc_list_get_nth(pages->options, 0)))->data)->hoverArg2 = NULL;
	((OptionData*)((GUI*)(gfc_list_get_nth(pages->options, 0)))->data)->onHover = swapSprite2;
	if (gfc_list_get_nth(pages->options, 0))
		slog("Added removal option.");
	for (int i = 0, j = 0; j < menagerie_get_member_count(); i++) {
		if (!menagerie_read_member_data_by_index(i)) {
			continue;
		}
		TextWord name = "";
		sprintf(name, "%s - %i", menagerie_read_member_data_by_index(i)->name, menagerie_read_member_data_by_index(i)->size);
		gfc_list_append(pages->options, gui_option_create(vector2d(0, 0), name, false, 3));
		((OptionData*)((GUI*)(gfc_list_get_nth(pages->options, j+1)))->data)->choiceArg1 = sys;
		((OptionData*)((GUI*)(gfc_list_get_nth(pages->options, j+1)))->data)->choiceArg2 = menagerie_read_member_data_by_index(i);
		((OptionData*)((GUI*)(gfc_list_get_nth(pages->options, j+1)))->data)->choiceArg3 = gfc_list_get_nth(pages->options, j+1);
		((OptionData*)((GUI*)(gfc_list_get_nth(pages->options, j+1)))->data)->onChoose = swapFiend;
		((OptionData*)((GUI*)(gfc_list_get_nth(pages->options, j+1)))->data)->hoverArg1 = sys;
		((OptionData*)((GUI*)(gfc_list_get_nth(pages->options, j+1)))->data)->hoverArg2 = menagerie_read_member_data_by_index(i)->sprite;
		((OptionData*)((GUI*)(gfc_list_get_nth(pages->options, j+1)))->data)->onHover = swapSprite2;
		j++;
	}
	slog("Added options to list.");
	data->menagerieList->visible = true;
	gui_page_list_refresh(data->menagerieList);

	data->leftSprite = gui_sprite_create(vector2d(game_get_resolution_x() * 0.25 - party_read_member_data(0)->sprite->frame_w / 2, 8), party_read_member_data(0)->sprite, gfc_color(1, 1, 1, 1), 4);
	slog("Left inited.");
	data->rightSprite = gui_sprite_create(vector2d(game_get_resolution_x() * 0.75, 8), NULL, gfc_color(1, 1, 1, 1), 4);
	slog("Right inited.");
	data->leftSprite->visible = true;
	data->rightSprite->visible = true;

	sys->think = party_manager_think;
	game_set_state(GS_Managing);
	return sys;
}
void party_manager_think(Entity* partyManager) {
	PartyManagerData* data = partyManager->data;

	if (data->replacing) {
		if (gfc_input_controller_button_pressed_by_index(0, 1)) {
			slog("Aborting swap.");
			swapSprite2(partyManager, NULL);
			((OptionData*)(data->partyOptions[data->fiendToReplace]->data))->isCurrentlyActive = true;
			((PageListData*)(data->menagerieList->data))->isCurrentlyActive = false;
			((OptionData*)(((GUI*)(gfc_list_get_nth(((PageListData*)(data->menagerieList->data))->options, ((PageListData*)(data->menagerieList->data))->selectedItemIndex)))->data))->selected = false;
			data->replacing = false;
			gfc_input_update();
		}
	}
}
void party_manager_free(Entity* partyManager) {
	PartyManagerData* data = partyManager->data;

	slog("Freeing GUIs...");
	gui_free(data->menagerieList);
	for (int i = 0; i < 5; i++) {
		gui_free(data->panels[i]);
		gui_free(data->partyOptions[i]);
	}
	gui_free(data->leftSprite);
	gui_free(data->rightSprite);
	gui_free(data->message);
	gui_free(data->slotDisplay);
	slog("Freeing party manager data...");
	//free(partyManager->data);
	slog("Freeing party manager...");
	entity_free(partyManager);
	slog("Party manager freed.");
}
int close_party_manager(Entity* partyManager) {
	GameState state = ((PartyManagerData*)partyManager->data)->returnState;
	party_manager_free(partyManager);
	game_set_state(state);
	return 0;
}

int changeFiendToSwap(Entity* partyManager, int changeTo) {
	PartyManagerData* data = partyManager->data;
	data->fiendToReplace = changeTo;
	slog("Swapping sprite to fiend %i.", changeTo);
	vector2d_add(data->leftSprite->position, data->leftSprite->position, vector2d(!data->leftSprite->sprite ? 0 : data->leftSprite->sprite->frame_w / 2, 0));
	if (party_read_member_data(changeTo))
		data->leftSprite->sprite = party_read_member_data(changeTo)->sprite;
	else
		data->leftSprite->sprite = NULL;
	vector2d_sub(data->leftSprite->position, data->leftSprite->position, vector2d(!data->leftSprite->sprite ? 0 : data->leftSprite->sprite->frame_w / 2, 0));
	slog("Sprite swapped.");
	return 0;
}
int chooseFiendToSwap(Entity* partyManager) {
	PartyManagerData* data = partyManager->data;
	data->replacing = true;
	((OptionData*)(data->partyOptions[data->fiendToReplace]->data))->isCurrentlyActive = false;
	((PageListData*)(data->menagerieList->data))->isCurrentlyActive = true;
	((PageListData*)(data->menagerieList->data))->selectedItemIndex = 0;
	(((OptionData*)((GUI*)(gfc_list_get_nth(((PageListData*)(data->menagerieList->data))->options, 0)))->data))->selectedNow = true;
	return 0;
}
int swapFiend(Entity* partyManager, FiendData* newMember, GUI* option) {
	PartyManagerData* data = partyManager->data;
	if (!newMember) {
		if (party_read_member_data(data->fiendToReplace)) {
			party_remove_fiend(data->fiendToReplace);
		}
	}
	else if (data->fiendToReplace >= party_get_member_count()) {
		party_add_fiend(newMember);
		if (data->fiendToReplace < 3) {
			((OptionData*)(data->partyOptions[data->fiendToReplace+1]->data))->grayed = false;
		}
	}
	else {
		party_replace_fiend(data->fiendToReplace, newMember);
	}
	for (int i = 0; i < 4; i++) {
		if(party_read_member_data(i))
			gfc_line_cpy(((OptionData*)(data->partyOptions[i]->data))->text, party_read_member_data(i)->name);
		else
			gfc_line_cpy(((OptionData*)(data->partyOptions[i]->data))->text, "(None)");
	}
	for (int i = 0, nulls = 0; i < 4; i++) {
		if (!party_read_member_data(i)) {
			nulls++;
			if (nulls > 1)
				((OptionData*)(data->partyOptions[i]->data))->grayed = true;
		}
	}
	((OptionData*)(data->partyOptions[data->fiendToReplace]->data))->isCurrentlyActive = true;
	((PageListData*)(data->menagerieList->data))->isCurrentlyActive = false;
	((OptionData*)(option->data))->selected = false;
	swapSprite2(partyManager, NULL);
	vector2d_add(data->leftSprite->position, data->leftSprite->position, vector2d(!data->leftSprite->sprite ? 0 : data->leftSprite->sprite->frame_w / 2, 0));
	if (newMember)
		data->leftSprite->sprite = party_read_member_data(data->fiendToReplace)->sprite;
	else if(party_read_member_data(data->fiendToReplace))
		data->leftSprite->sprite = party_read_member_data(data->fiendToReplace)->sprite;
	else
		data->leftSprite->sprite = NULL;
	vector2d_sub(data->leftSprite->position, data->leftSprite->position, vector2d(!data->leftSprite->sprite ? 0 : data->leftSprite->sprite->frame_w / 2, 0));


	sprintf(((TextData*)(data->slotDisplay->data))->text, "Slots: %i/4", party_get_slots_used());
	data->slotDisplay->color = (party_get_slots_used() > 0 && party_get_slots_used() <= 4) ? gfc_color(0, 0.8, 0, 1) : gfc_color(0.8, 0, 0, 1);
	((OptionData*)(data->partyOptions[4]->data))->grayed = !(party_get_slots_used() > 0 && party_get_slots_used() <= 4);
	((OptionData*)(data->partyOptions[data->fiendToReplace]->data))->isCurrentlyActive = true;
	((PageListData*)(data->menagerieList->data))->isCurrentlyActive = false;
	data->replacing = false;
	gfc_input_update();

	return 0;
}
int swapSprite2(Entity* partyManager, Sprite* newSprite) {
	PartyManagerData* data = partyManager->data;
	vector2d_add(data->rightSprite->position, data->rightSprite->position, vector2d(!data->rightSprite->sprite ? 0 : data->rightSprite->sprite->frame_w / 2, 0));
	data->rightSprite->sprite = newSprite;
	vector2d_sub(data->rightSprite->position, data->rightSprite->position, vector2d(!data->rightSprite->sprite ? 0 : data->rightSprite->sprite->frame_w / 2, 0));
	return 0;
}