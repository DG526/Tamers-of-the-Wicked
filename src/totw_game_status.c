#include "simple_json.h"
#include "simple_logger.h"
#include "totw_game_status.h"
#include "totw_fiend.h"

static GameState state = GS_Title;
static Bool quitting = false;
static int resX = 0, resY = 0;
static float waitSpeed = 1;

Bool game_get_quitting() {
	return quitting;
}
GameState game_get_state() {
	return state;
}

void game_quit() {
	quitting = true;
}
void game_set_state(GameState status) {
	state = status;
}

void game_set_resolution(int x, int y) {
	resX = x;
	resY = y;
}
int game_get_resolution_x() {
	return resX;
}
int game_get_resolution_y() {
	return resY;
}

float game_get_wait_speed() {
	return waitSpeed;
}
void game_set_wait_speed(float newSpeed) {
	waitSpeed = newSpeed;
}

void game_save() {
	slog("Saving...");
	SJson* file = sj_object_new();
	SJson* menagerie = sj_new();
	SJson* party = sj_array_new();
	if (!file || !menagerie || !party) {
		slog("Saving error: Could not initialize JSON.");
		return;
	}
	menagerie = menagerie_get_as_json();
	for (int i = 0; i < party_get_member_count(); i++) {
		sj_array_append(party, sj_new_str(party_read_member_data(i)->name));
	}
	sj_object_insert(file, "fiend list", menagerie);
	sj_object_insert(file, "party", party);
	sj_save(file, "save/savegame.dat");
	slog("Game saved successfully.");
}