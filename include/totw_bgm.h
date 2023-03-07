#ifndef __BGM_H__
#define __BGM_H__

#include "gfc_audio.h"
#include "gfc_text.h"

typedef enum {
	BGM_Title,
	BGM_Base,
	BGM_Dungeon,
	BGM_Battle,
	BGM_Boss
}BGMTrack;

void bgm_play_loop(BGMTrack track);
void bgm_play_loop_at(BGMTrack track, float startTime);
BGMTrack bgm_get_current_track();
float bgm_pause();
void bgm_update();

#endif