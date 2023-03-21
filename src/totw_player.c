#include "simple_logger.h"
#include "totw_camera.h"
#include "gfc_input.h"
#include "totw_game_status.h"
#include "totw_battle.h"
#include "totw_level.h"
#include "totw_player.h"

static Entity* player = { 0 };

Bool canMove(Entity* self, CompassDirection direction);

Entity* player_new() {
	player = entity_new();
	if (!player) {
		slog("No player.");
		return NULL;
	}
	player->sprite = gf2d_sprite_load_all("images/Understudy.png", 32, 32, 1, 0);
	if (!player->sprite) slog("No player sprite.");
	player->think = player_think;
	player->update = player_update;
	player->type = ET_Player;

	PlayerData* data = gfc_allocate_array(sizeof(PlayerData), 1);
	if (!data) slog("No player data.");
	data->focused = true;
	data->state = PS_Idle;
	data->battleSteps = -1;
	player->data = data;
	return player;
}

void player_think(Entity* self) {
	if (game_get_state() != GS_Roaming) return;
	PlayerData* data = (PlayerData*)(self->data);
	switch (data->state) {
	case PS_Inactive:
		return;
	case PS_Idle:
		if (gfc_input_controller_button_pressed_by_index(0, 0)) {
			int x = self->mapPosition.x;
			int y = self->mapPosition.y;
			switch (data->direction) {
			case North:
				y--;
				break;
			case South:
				y++;
				break;
			case East:
				x++;
				break;
			case West:
				x--;
				break;
			}
			slog("Attempting to interact with something at %i, %i...", x, y);
			Entity* occ = tile_get_occupier(level_get_active_level(), x, y);
			if (occ && occ->interact) {
				slog("Interacting with something.");
				occ->interact(occ);
				gfc_input_update();
				return;
			}
		}
		if (gfc_input_controller_button_pressed(0, "D_U")) {
			data->direction = North;
			//slog("Checking movement criterion.");
			if (canMove(self, North)) {
				//slog("Initiating walking.");
				self->mapPosition.y--;
				data->state = PS_Walking;
			}
		}
		else if (gfc_input_controller_button_pressed(0, "D_D")) {
			data->direction = South;
			//slog("Checking movement criterion.");
			if (canMove(self, South)) {
				//slog("Initiating walking.");
				self->mapPosition.y++;
				data->state = PS_Walking;
			}
		}
		else if (gfc_input_controller_button_pressed(0, "D_R")) {
			data->direction = East;
			//slog("Checking movement criterion.");
			if (canMove(self, East)) {
				//slog("Initiating walking.");
				self->mapPosition.x++;
				data->state = PS_Walking;
			}
		}
		else if (gfc_input_controller_button_pressed(0, "D_L")) {
			data->direction = West;
			//slog("Checking movement criterion.");
			if (canMove(self, West)) {
				//slog("Initiating walking.");
				self->mapPosition.x--;
				data->state = PS_Walking;
			}
		}
		break;
	case PS_Walking:
		switch (data->direction) {
		case North:
			self->position.y -= 0.85;
			if (self->position.y <= self->mapPosition.y * 32) {
				self->position.y = self->mapPosition.y * 32;
				data->state = PS_Stopping;
			}
			break;
		case South:
			self->position.y += 0.85;
			if (self->position.y >= self->mapPosition.y * 32) {
				self->position.y = self->mapPosition.y * 32;
				data->state = PS_Stopping;
			}
			break;
		case East:
			self->position.x += 0.85;
			if (self->position.x >= self->mapPosition.x * 32) {
				self->position.x = self->mapPosition.x * 32;
				data->state = PS_Stopping;
			}
			break;
		case West:
			self->position.x -= 0.85;
			if (self->position.x <= self->mapPosition.x * 32) {
				self->position.x = self->mapPosition.x * 32;
				data->state = PS_Stopping;
			}
			break;
		}
		break;
	case PS_Stopping: {
		TileInfo newTile = tile_get(self->mapPosition.x, self->mapPosition.y);
		if (newTile.encounterZone > 0 && !gfc_input_controller_button_held(0,"R1") && !gfc_input_controller_button_held(0,"L1")) {
			data->battleSteps--;
			if (data->battleSteps == 0) {
				slog("Found a monster!");
				data->battleSteps = level_get_active_level()->encounterSteps + gfc_crandom() * level_get_active_level()->encounterVariance;
				generate_new_battle(level_get_active_level()->name, newTile.encounterZone);
				data->state = PS_Idle;
				break;
			}
		}
		if (gfc_input_controller_button_held(0, "D_U")) {
			data->direction = North;
			if (canMove(self, North)) {
				self->mapPosition.y--;
				data->state = PS_Walking;
			}
			else
				data->state = PS_Idle;
		}
		else if (gfc_input_controller_button_held(0, "D_D")) {
			data->direction = South;
			if (canMove(self, South)) {
				self->mapPosition.y++;
				data->state = PS_Walking;
			}
			else
				data->state = PS_Idle;
		}
		else if (gfc_input_controller_button_held(0, "D_R")) {
			data->direction = East;
			if (canMove(self, East)) {
				self->mapPosition.x++;
				data->state = PS_Walking;
			}
			else
				data->state = PS_Idle;
		}
		else if (gfc_input_controller_button_held(0, "D_L")) {
			data->direction = West;
			if (canMove(self, West)) {
				self->mapPosition.x--;
				data->state = PS_Walking;
			}
			else
				data->state = PS_Idle;
		}
		else
			data->state = PS_Idle;
		break;
	}
		
	}
}

void player_update(Entity* self) {
	if (game_get_state() == GS_Battle) return;
	PlayerData* data = (PlayerData*)(self->data);
	self->frame = data->direction;
	if (data->state == PS_Inactive) return;
	if (data->focused) {
		camera_center_at(vector2d(self->position.x + 16, self->position.y + 16));
		camera_world_snap();
	}
}

Bool canMove(Entity* self, CompassDirection direction) {
	if (!self) return false;
	int x = self->mapPosition.x;
	int y = self->mapPosition.y;
	//slog("Attempting to move from %i, %i.", x, y);
	switch (direction) {
	case North:
		y--;
		break;
	case South:
		y++;
		break;
	case East:
		x++;
		break;
	case West:
		x--;
		break;
	}
	if (!level_get_active_level()) {
		slog("Level not found.");
		return false;
	}
	TileInfo tile = tile_get(x, y);
	//if (tile.solid) slog("Tile is solid.");
	if (tile.occupier || tile.solid) { 
		//slog("Tile is occupied or solid.");
		return false; 
	}
	return true;
}

Entity* player_get() {
	return player;
}

void player_set_coords(Vector2D worldCoords) {
	player->mapPosition = worldCoords;
	player->position = vector2d_multiply(worldCoords, vector2d(32, 32));
	slog("Placed player at %i, %i.", (int)(player->mapPosition.x), (int)(player->mapPosition.y));
}
void player_set_dir(CompassDirection dir) {
	((PlayerData*)(player->data))->direction = dir;
	slog("Changed player direction");
}