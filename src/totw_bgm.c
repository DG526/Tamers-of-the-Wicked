#include "simple_logger.h";
#include "gfc_types.h"
#include "totw_bgm.h"

static BGMTrack ct = 0;
static Sound* currentTrack = { 0 };
static float trackLoopPoint;
static int timeLoopPoint, currentLayer;

void bgm_play_loop(BGMTrack track) {
	gfc_sound_free(currentTrack);
	switch (track) {
	case BGM_Battle:
		currentTrack = gfc_sound_load("audio/bgm/Fiend Fight.mp3", 1, 0);
		trackLoopPoint = 72;
		break;
	case BGM_Boss:
		currentTrack = gfc_sound_load("audio/bgm/Monstrous Mayhem.mp3", 1, 0);
		trackLoopPoint = 48;
		break;
	}
	if (!currentTrack) {
		slog("Error finding audio file.");
		return;
	}
	gfc_sound_play(currentTrack, 0, 0.8, 0, -1);
	Mix_Pause(1);
	timeLoopPoint = SDL_GetTicks64() + (int)(trackLoopPoint * 1000);
	currentLayer = 0;
	ct = track;
}
/*
void bgm_play_loop_at(BGMTrack track, float startTime) {
	gfc_sound_free(currentTrack);
	switch (track) {
	case BGM_Battle:
		currentTrack = gfc_sound_load("audio/bgm/Fiend Fight.mp3", 1, 0);
		trackLoopPoint = 72;
		break;
	case BGM_Boss:
		currentTrack = gfc_sound_load("audio/bgm/Monstrous Mayhem.mp3", 1, 0);
		trackLoopPoint = 48;
		break;
	}
	gfc_sound_play(currentTrack, 0, 0.8, 0, -1);
	timeLoopPoint = SDL_GetTicks64() + (int)(trackLoopPoint * 1000);
	currentLayer = 0;
}
*/
BGMTrack bgm_get_current_track() {
	return ct;
}
float bgm_pause() {
	timeLoopPoint = 0;
	currentLayer = 0;
	gfc_sound_free(currentTrack);
	currentTrack = 0;
	Mix_Pause(0);
	Mix_Pause(1);
}
void bgm_update() {
	if (!currentTrack) return;
	if (timeLoopPoint <= SDL_GetTicks64()) {
		timeLoopPoint = SDL_GetTicks64() + (int)(trackLoopPoint * 1000);
		currentLayer = (currentLayer + 1) % 2;
		gfc_sound_play(currentTrack, 0, 0.8, currentLayer, -1);
	}
}