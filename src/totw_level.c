#include "simple_logger.h"
#include "simple_json.h"

#include "gfc_config.h"
#include "gfc_list.h"

#include "gf2d_graphics.h"

#include "totw_camera.h"
#include "totw_level.h"
#include "totw_overworld_ents.h";
#include "totw_player.h";

void level_build(Level* level);

static Level* activeLevel = NULL;

Level* level_get_active_level()
{
    return activeLevel;
}

void level_set_active_level(Level* level)
{
    activeLevel = level;
}

Level* level_load(const char* filename)
{
    int tile;
    int i, c;
    int j, d;
    int tileFPL;
    const char* str;
    SJson* json, * lj, * list, * zList, * row, * item, * encounterSteps, *encounterVariance;
    Level* level;
    if (!filename)return NULL;
    json = sj_load(filename);
    if (!json)return NULL;
    level = level_new();
    if (!level)
    {
        slog("failed to allocate new level");
        sj_free(json);
        return NULL;
    }
    lj = sj_object_get_value(json, "map");
    if (!lj)
    {
        slog("file %s missing level object", filename);
        sj_free(json);
        level_free(level);
        return NULL;
    }
    str = sj_object_get_value_as_string(lj, "name");
    if (str)gfc_line_cpy(level->name, str);
    //sj_value_as_vector2d(sj_object_get_value(lj, "tileSize"), &level->tileSize);
    //sj_object_get_value_as_int(lj, "tileFPL", &tileFPL);
    //str = sj_object_get_value_as_string(lj, "tileSet");
    //if (str)
    //{
    //    level->tileSet = gf2d_sprite_load_all(str, (Sint32)level->tileSize.x, (Sint32)level->tileSize.y, tileFPL, 1);
    //}
    level->tileSet = tileset_load(sj_object_get_value_as_string(lj, "tileSet"));
    list = sj_object_get_value(lj, "tileMap");
    zList = sj_object_get_value(lj, "zoneMap");
    c = sj_array_get_count(list);
    row = sj_array_get_nth(list, 0);
    d = sj_array_get_count(row);
    if ((c * d) == 0)
    {
        slog("corrupt row or column count for %s level", filename);
        level_free(level);
        sj_free(json);
        return NULL;
    }
    level->mapSize.x = d;
    level->mapSize.y = c;
    level->tileMap = gfc_allocate_array(sizeof(int), c * d);
    level->tileInfoMap = gfc_allocate_array(sizeof(TileInfo), c * d);
    if (!level->tileMap || !level->tileInfoMap)
    {
        slog("failed to allocate tileMap for level %s", filename);
        level_free(level);
        sj_free(json);
        return NULL;
    }
    for (i = 0; i < c; i++)// i is row
    {
        row = sj_array_get_nth(list, i);
        SJson* zRow = sj_array_get_nth(zList, i);
        if (!row)continue;
        d = sj_array_get_count(row);
        for (j = 0; j < d; j++)// j is column
        {
            item = sj_array_get_nth(row, j);
            SJson* zItem = sj_array_get_nth(zRow, j);
            if (!item)continue;
            tile = 0;//default
            int zid = 0;
            sj_get_integer_value(item, &tile);
            sj_get_integer_value(zItem, &zid);
            level->tileMap[(i * (int)level->mapSize.x) + j] = tile;
            level->tileInfoMap[(i * (int)level->mapSize.x) + j] = level->tileSet.templates[tile];
            level->tileInfoMap[(i * (int)level->mapSize.x) + j].encounterZone = zid;
        }
    }

    level->encounterSteps = -1;
    sj_get_integer_value(sj_object_get_value(lj, "encounterSteps"), &(level->encounterSteps));
    sj_get_integer_value(sj_object_get_value(lj, "esVariance"), &(level->encounterVariance));

    SJson* spawnList = sj_object_get_value(lj, "spawnList");
    for (int i = 0; i < sj_array_get_count(spawnList); i++) {
        SJson* inst = sj_array_get_nth(spawnList, i);
        if (!strcmp(sj_get_string_value(sj_object_get_value(inst, "name")), "boss")) {
            int x, y;
            sj_get_integer_value(sj_array_get_nth(sj_object_get_value(inst, "tilePos"), 0), &x);
            sj_get_integer_value(sj_array_get_nth(sj_object_get_value(inst, "tilePos"), 1), &y);
            slog("Attempting to create a boss at %i, %i.", x, y);
            Entity* boss = owe_boss_new(sj_get_string_value(sj_object_get_value(inst, "boss")), vector2d(x,y));
            char* dir = sj_get_string_value(sj_object_get_value(inst, "direction"));
            if (!gfc_word_cmp(dir, "North"))
                ((OWE_BossData*)(boss->data))->direction = North;
            else if (!gfc_word_cmp(dir, "East"))
                ((OWE_BossData*)(boss->data))->direction = East;
            else if (!gfc_word_cmp(dir, "West"))
                ((OWE_BossData*)(boss->data))->direction = West;
            slog("Placed Boss");
            tile_set_occupier(level, x, y, boss);
        }
        if (!strcmp(sj_get_string_value(sj_object_get_value(inst, "name")), "player_start")) {
            int x, y;
            sj_get_integer_value(sj_array_get_nth(sj_object_get_value(inst, "tilePos"), 0), &x);
            sj_get_integer_value(sj_array_get_nth(sj_object_get_value(inst, "tilePos"), 1), &y);
            slog("Attempting to place player at %i, %i.", x, y);
            char* dir = sj_get_string_value(sj_object_get_value(inst, "direction"));
            if (!gfc_word_cmp(dir, "North"))
                ((PlayerData*)(player_get()->data))->direction = North;
            else if (!gfc_word_cmp(dir, "South"))
                ((PlayerData*)(player_get()->data))->direction = South;
            else if (!gfc_word_cmp(dir, "East"))
                ((PlayerData*)(player_get()->data))->direction = East;
            else if (!gfc_word_cmp(dir, "West"))
                ((PlayerData*)(player_get()->data))->direction = West;
            player_set_coords(vector2d(x, y));
            slog("Placed Player");
            //tile_set_occupier(level, x, y, boss);
        }
        if (!strcmp(sj_get_string_value(sj_object_get_value(inst, "name")), "portal")) {
            int x, y, r, g, b;
            sj_get_integer_value(sj_array_get_nth(sj_object_get_value(inst, "tilePos"), 0), &x);
            sj_get_integer_value(sj_array_get_nth(sj_object_get_value(inst, "tilePos"), 1), &y);
            sj_get_integer_value(sj_array_get_nth(sj_object_get_value(inst, "color"), 0), &r);
            sj_get_integer_value(sj_array_get_nth(sj_object_get_value(inst, "color"), 1), &g);
            sj_get_integer_value(sj_array_get_nth(sj_object_get_value(inst, "color"), 2), &b);
            Color col = gfc_color8(r, g, b, 255);
            const char* t = sj_get_string_value(sj_object_get_value(inst, "targetMap"));
            Entity* portal = owe_portal_new(t, vector2d(x, y), col);
            slog("Placed Portal");
            tile_set_occupier(level, x, y, portal);
        }
    }
    SJson* rivals = sj_object_get_value(lj, "rivals");
    if (rivals) {
        SJson* quantities = sj_object_get_value(rivals, "quantities");
        int quantity;
        sj_get_integer_value(sj_array_get_nth(quantities, min(gfc_random() * sj_array_get_count(quantities), sj_array_get_count(quantities) - 1)), &quantity);
        for (int i = 0; i < quantity; i++) {
            slog("Getting rival coordinates...");
            int x = min(gfc_random() * d, d - 1);
            int y = min(gfc_random() * c, c - 1);
            while (!tile_is_available(level, x, y) || !level->tileInfoMap[(y * (int)level->mapSize.x) + x].encounterZone) {
                slog("Recalculating rival coordinates...");
                x = min(gfc_random() * d, d - 1);
                y = min(gfc_random() * c, c - 1);
            }
            slog("Getting rival difficulty...");
            SJson* difficulties = sj_object_get_value(rivals, "difficulties");
            int difficulty;
            sj_get_integer_value(sj_array_get_nth(difficulties, min(gfc_random() * sj_array_get_count(difficulties), sj_array_get_count(difficulties) - 1)), &difficulty);
            slog("Getting rival specialty...");
            int r = min(gfc_random() * 5, 4);
            TextWord specialty;
            switch (r) {
            case 0:
            case 1:
                gfc_word_cpy(specialty, "none");
                break;
            case 2:
                gfc_word_cpy(specialty, "toughie");
                break;
            case 3:
                gfc_word_cpy(specialty, "trickster");
                break;
            case 4:
                gfc_word_cpy(specialty, "mage");
                break;
            }
            slog("Rival specialty: %s.", specialty);
            Entity* rival = owe_rival_new(specialty, difficulty, vector2d(x, y));
            tile_set_occupier(level, x, y, rival);
        }
        slog("Successfully created %i rivals.", quantity);
    }

    ((PlayerData*)(player_get()->data))->battleSteps = level->encounterSteps + gfc_crandom() * level->encounterVariance;
    sj_free(json);
    level_build(level);
    slog("Level created successfully.");
    return level;
}
/*
int level_shape_clip(Level* level, Shape shape)
{
    int i, c;
    Shape* clip;
    if (!level)return 0;
    c = gfc_list_get_count(level->clips);
    for (i = 0; i < c; i++)
    {
        clip = gfc_list_get_nth(level->clips, i);
        if (!clip)continue;
        if (gfc_shape_overlap(*clip, shape))return 1;
    }
    return 0;
}
void level_build_clip_space(Level* level)
{
    Shape* shape;
    int i, j;
    if (!level)return;
    for (j = 0; j < level->mapSize.y; j++)//j is row
    {
        for (i = 0; i < level->mapSize.x; i++)// i is column
        {
            if (level->tileMap[(j * (int)level->mapSize.x) + i] <= 0)continue;//skip zero
            shape = gfc_allocate_array(sizeof(Shape), 1);
            if (!shape)continue;
            *shape = gfc_shape_rect(i * level->tileSize.x, j * level->tileSize.y, level->tileSize.x, level->tileSize.y);
            gfc_list_append(level->clips, shape);
        }
    }
}
*/

void level_build(Level* level)
{
    int i, j;
    if (!level)return;
    if (level->tileLayer)gf2d_sprite_free(level->tileLayer);
    level->tileLayer = gf2d_sprite_new();
    if (!level->tileLayer)
    {
        slog("failed to create sprite for tileLayer");
        return;
    }
    // if there is a default surface, free it
    if (level->tileLayer->surface)SDL_FreeSurface(level->tileLayer->surface);
    //create a surface the size we need it
    level->tileLayer->surface = gf2d_graphics_create_surface(level->tileSet.tileSize.x * level->mapSize.x, level->tileSet.tileSize.y * level->mapSize.y);
    if (!level->tileLayer->surface)
    {
        slog("failed to create tileLayer surface");
        return;
    }
    //make sure the surface is compatible with our graphics settings
    level->tileLayer->surface = gf2d_graphics_screen_convert(&level->tileLayer->surface);
    if (!level->tileLayer->surface)
    {
        slog("failed to create surface for tileLayer");
        return;
    }
    //draw the tile sprite to the surface
    for (j = 0; j < level->mapSize.y; j++)//j is row
    {
        for (i = 0; i < level->mapSize.x; i++)// i is column
        {
            if (level->tileMap[(j * (int)level->mapSize.x) + i] < 0)continue;//skip negatives
            gf2d_sprite_draw_to_surface(
                level->tileSet.sprite,
                vector2d(i * level->tileSet.tileSize.x, j * level->tileSet.tileSize.y),
                NULL,
                NULL,
                level->tileMap[(j * (int)level->mapSize.x) + i],
                level->tileLayer->surface);
        }
    }
    //convert it to a texture
    level->tileLayer->texture = SDL_CreateTextureFromSurface(gf2d_graphics_get_renderer(), level->tileLayer->surface);
    SDL_SetTextureBlendMode(level->tileLayer->texture, SDL_BLENDMODE_BLEND);
    SDL_UpdateTexture(level->tileLayer->texture,
        NULL,
        level->tileLayer->surface->pixels,
        level->tileLayer->surface->pitch);
    level->tileLayer->frame_w = level->tileLayer->surface->w;
    level->tileLayer->frame_h = level->tileLayer->surface->h;
    level->tileLayer->frames_per_line = 1;
    camera_set_world_size(vector2d(level->tileLayer->frame_w, level->tileLayer->frame_h));
    //level_build_clip_space(level);
    slog("Level built.");
}

void level_draw(Level* level)
{
    if (!level)return;
    if (!level->tileLayer)return;
    gf2d_sprite_draw_image(level->tileLayer, camera_get_draw_offset());
}

Level* level_new()
{
    Level* level;
    level = gfc_allocate_array(sizeof(Level), 1);
    //level->clips = gfc_list_new();
    return level;
}

void level_free(Level* level)
{
    if (!level)return;
    if (level->tileSet.sprite)gf2d_sprite_free(level->tileSet.sprite);
    if (level->tileLayer)gf2d_sprite_free(level->tileLayer);
    if (level->tileMap)free(level->tileMap);
    //gfc_list_foreach(level->clips, free);
    //gfc_list_delete(level->clips);
    free(level);
}

//TileInfo* tile_get(int column, int row);

Tileset tileset_load(const char* filename) {
    Tileset ts;
    if (!filename) return ts;
    SJson* file;
    file = sj_load(filename);
    if (!file) return ts;
    sj_get_integer_value(sj_object_get_value(file, "tilesPerRow"), &(ts.fpl));
    int x, y;
    sj_get_integer_value(sj_array_get_nth(sj_object_get_value(file, "tileSize"), 0), &x);
    sj_get_integer_value(sj_array_get_nth(sj_object_get_value(file, "tileSize"), 1), &y);
    ts.tileSize = vector2d(x, y);
    ts.sprite = gf2d_sprite_load_all(sj_get_string_value(sj_object_get_value(file, "sprite")), x, y, ts.fpl, 1);
    if (!ts.sprite) slog("Invalid sprite in %s", filename);
    ts.tiles = sj_array_get_count(sj_object_get_value(file, "tiles"));
    for (int i = 0; i < ts.tiles && i < 64; i++) {
        TileInfo templ;
        templ.tileFrame = i;
        int passable;
        sj_get_integer_value(sj_object_get_value(sj_array_get_nth(sj_object_get_value(file, "tiles"), i), "passable"), &passable);
        templ.solid = !passable;
        templ.occupier = 0;
        ts.templates[i] = templ;
    }
    sj_free(file);
    return ts;
}

TileInfo tile_get(int column, int row) {
    int width = activeLevel->mapSize.x;
    return activeLevel->tileInfoMap[column + row * width];
}
void tile_set_occupier(Level* level, int column, int row, Entity* occupier) {
    int width = level->mapSize.x;
    level->tileInfoMap[column + row * width].occupier = occupier;
}
Entity* tile_get_occupier(Level* level, int column, int row) {
    int width = level->mapSize.x;
    return level->tileInfoMap[column + row * width].occupier;
}
int tile_is_available(Level* level, int column, int row) {
    int width = level->mapSize.x;
    if(level->tileInfoMap[column + row * width].occupier || level->tileInfoMap[column + row * width].solid)
        return false;
    return true;
}

void tile_cpy(TileInfo* dst, TileInfo* src) {
    dst->coords = src->coords;
    dst->solid = src->solid;
    dst->tileFrame = src->tileFrame;
}