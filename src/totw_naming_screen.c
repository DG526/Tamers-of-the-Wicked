#include "simple_logger.h"
#include "gfc_types.h"
#include "totw_naming_screen.h"
#include "totw_game_status.h"

Entity* naming_system_new(FiendData* namingTarget) {
	slog("attempting to create naming system");
	if (!namingTarget) return NULL;
	Entity* sys = entity_new();
	if (!sys) return NULL;
	NamingSystemData* data = gfc_allocate_array(sizeof(NamingSystemData),1);
	if (!data) {
		entity_free(sys);
		return NULL;
	}
	data->isNamingFiend = true;
	data->namingTarget = namingTarget;
	TextLine* toWriteTo = NULL;
	toWriteTo = &(namingTarget->name);
	gfc_line_cpy(*toWriteTo, "");
	char* toDisplay = "Please input a name: __________";
	data->name = toWriteTo;

	data->nameDisplay = gui_text_create(vector2d(45, 35), toDisplay, false, 0);
	data->nameDisplay->visible = true;

	data->shifted = true;
	char charList[] = "1234567890abcdefghijklmnopqrstuvwxyz-\'";
	char shiftList[] = "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ-\'";
	slog("creating letters");
	int charNum = 0;
	int xOffsetBase = 30, xOffsetInterval = 20, yOffsetBase = 80, yOffsetInterval = 20;
	for (int r = 0; charNum < 38; r++) {
		for (int c = 0; c < 10 && charNum < 38; c++) {
			data->choices[charNum] = gui_letter_create(vector2d(xOffsetBase + xOffsetInterval * c, yOffsetBase + yOffsetInterval * r), charList[charNum], shiftList[charNum],
				charNum == 10, data->name, 0);
			//((LetterData*)(data->choices[charNum]->data))->shiftLetter = shiftList[charNum];
			((LetterData*)(data->choices[charNum]->data))->shifted = data->shifted;
			data->choices[charNum]->visible = true;
			charNum++;
		}
	}
	slog("creating option");
	data->choices[38] = gui_option_create(vector2d(xOffsetBase + xOffsetInterval * 8, yOffsetBase + yOffsetInterval * 3), "Enter", false, 0);
	data->choices[38]->visible = true;
	((OptionData*)(data->choices[38]->data))->choiceArg1 = sys;
	((OptionData*)(data->choices[38]->data))->onChoose = name_fiend;
	charNum = 0;
	for (int r = 0; charNum < 38; r++) {
		for (int c = 0; c < 10 && charNum < 38; c++) {
			if (r > 0)
				((LetterData*)(data->choices[charNum]->data))->up = data->choices[charNum - 10];
			else
				((LetterData*)(data->choices[charNum]->data))->up = data->choices[min(charNum + 30, 38)];
			if (r < 3)
				((LetterData*)(data->choices[charNum]->data))->down = data->choices[min(charNum + 10, 38)];
			else
				((LetterData*)(data->choices[charNum]->data))->down = data->choices[charNum - 30];
			if (c > 0)
				((LetterData*)(data->choices[charNum]->data))->left = data->choices[charNum - 1];
			else
				((LetterData*)(data->choices[charNum]->data))->left = data->choices[charNum == 30 ? 38 : charNum + 9];
			if (c < 9)
				((LetterData*)(data->choices[charNum]->data))->right = data->choices[charNum + 1];
			else
				((LetterData*)(data->choices[charNum]->data))->right = data->choices[charNum - 9];
			charNum++;
		}
	}
	((OptionData*)(data->choices[38]->data))->up = data->choices[28];
	((OptionData*)(data->choices[38]->data))->down = data->choices[8];
	((OptionData*)(data->choices[38]->data))->left = data->choices[37];
	((OptionData*)(data->choices[38]->data))->right = data->choices[30];

	sys->data = data;
	sys->think = naming_system_think;
	sys->type = ET_Menu;
	return sys;
}

void naming_system_think(Entity* namingSystem) {
	if (!namingSystem) return;
	NamingSystemData* data = (NamingSystemData*)(namingSystem->data);
	char disp[45];
	sprintf(disp, "Please input a name: %s__________", *(data->name));
	disp[31] = '\0';
	gfc_line_cpy(((TextData*)(data->nameDisplay->data))->text, disp);
	if (data->acceptingInput) {
		if (gfc_input_controller_button_pressed_by_index(0, 1)) {
			for (int i = 1; i < 11; i++) {
				if ((*(data->name))[i] == '\0') {
					char* newName = *(data->name);
					newName[i - 1] = '\0';
					strncpy(*(data->name), newName, 11);
					break;
				}
			}
		}
		if (gfc_input_controller_button_pressed(0, "square")) {
			data->capsLock = !data->capsLock;
			data->shifted = data->capsLock;
			for (int i = 0; i < 39; i++) {
				((LetterData*)(data->choices[i]->data))->shifted = data->shifted;
			}
		}
		if (gfc_input_controller_button_released_by_index(0, 0) && data->shifted && !data->capsLock) {
			data->shifted = !data->shifted;
			for (int i = 0; i < 39; i++) {
				((LetterData*)(data->choices[i]->data))->shifted = data->shifted;
			}
		}
		else if (gfc_input_controller_button_pressed(0, "triangle")) {
			if (data->capsLock) data->capsLock = false;
			data->shifted = !data->shifted;
			for (int i = 0; i < 39; i++) {
				((LetterData*)(data->choices[i]->data))->shifted = data->shifted;
			}
		}
	}
	else if (gfc_input_controller_button_released_by_index(0, 0)){
		data->acceptingInput = true;
	}
	
}

void naming_system_free(Entity* namingSystem) {
	if (!namingSystem) return;
	slog("Freeing keyboard.");
	NamingSystemData* data = (NamingSystemData*)(namingSystem->data);
	slog("    Freeing letters.");
	for (int i = 0; i < 39; i++) {
		gui_free(data->choices[i]);
	}
	slog("    Freeing name display.");
	gui_free(data->nameDisplay);
	slog("Setting state.");
	if (data->isNamingFiend) {
		game_set_state(GS_Roaming);
	}
	slog("Finishing cleanup.");
	memset(namingSystem->data, 0, sizeof(NamingSystemData));
	entity_free(namingSystem);
	slog("Done cleaning up keyboard.");
}


int name_fiend(void* namingSystem) {
	if (!namingSystem) return 0;
	gfc_input_update();
	NamingSystemData* data = (NamingSystemData*)(((Entity*)namingSystem)->data);
	if (fiend_name_taken(*(data->name))) {
		slog("Name is already taken.");
		((OptionData*)(data->choices[38]->data))->selected = false;
		gui_error_create("Name is already\n      taken.", 1, data->choices[38]);
	}
	else {
		fiend_register_new((FiendData*)(data->namingTarget));
		if(party_get_slots_used() + ((FiendData*)(data->namingTarget))->size <= 4)
			party_add_fiend((FiendData*)(data->namingTarget));
		naming_system_free((Entity*)(namingSystem));
		slog("Fiend named successfully.");
	}
	return 0;
}