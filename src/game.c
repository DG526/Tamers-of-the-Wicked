#include <SDL.h>
#include "gf2d_graphics.h"
#include "gf2d_sprite.h"
#include "gf2d_font.h"
#include "gfc_input.h"
#include "simple_logger.h"
#include "totw_game_status.h"
#include "totw_entity.h"
#include "totw_fiend.h"
#include "totw_gui.h"
#include "totw_bgm.h"
#include "totw_battle.h"
#include "totw_level.h"
#include "totw_player.h"
#include "totw_fiend_management.h"

static Bool quitNow = false;
static Bool changed = false;
static TextBlock battleDialogue = "";

void quit() {
    slog("Quitting...");
    quitNow = true;
}
void startGame() {
    gui_free_all();

    player_new();
    if (player_get()) slog("Successfully created player.");

    ally_list_load();

    level_set_active_level(level_load("maps/base.map"));
    //level_set_active_level(level_load("maps/dungeon-1-1.map"));
    if (!level_get_active_level()) slog("Couldn't create level.");

    game_set_state(GS_Roaming);
}
void loadMain() {
    Sprite* logoSpr = gf2d_sprite_load_image("images/Logo.png");
    GUI* logo = gui_sprite_create(vector2d(0, -24), logoSpr, gfc_color(1, 1, 1, 1), 0);
    GUI* opStart = gui_option_create(vector2d(100, 120), "Start", true, 0);
    GUI* opQuit = gui_option_create(vector2d(160, 120), "Quit", false, 0);
    opStart->color = gfc_color(1, 1, 1, 1);
    opQuit->color = gfc_color(1, 0.8, 0.8, 1);
    ((OptionData*)(opStart->data))->right = opQuit;
    ((OptionData*)(opQuit->data))->left = opStart;
    ((OptionData*)(opStart->data))->onChoose = startGame;
    ((OptionData*)(opQuit->data))->onChoose = game_quit;
    logo->visible = true;
    opStart->visible = true;
    opQuit->visible = true;
    game_set_state(GS_Title);
}
void exitToMain() {
    slog("Starting to exit to Main Menu.");
    entity_free_all();
    slog("Entities freed.");
    party_close();
    slog("Party closed.");
    gui_free_all();
    slog("GUIs freed.");
    level_free(level_get_active_level());
    slog("Level freed.");
    loadMain();
}

int main(int argc, char * argv[])
{
    /*variable declarations*/
    int done = 0;
    const Uint8 * keys;
    //Sprite *sprite;
    
    int mx,my;
    float mf = 0;
    Sprite *mouse;
    Sprite *panel;
    Sprite* cursor;
    Sprite* barFrame, * barFill;
    Color mouseColor = gfc_color8(255,100,255,200);
    Vector2D renderSize = vector2d(300, 180);
    Vector2D renderScale = vector2d(5, 5);
    game_set_resolution(renderSize.x, renderSize.y);
    Level* currentMap;
    
    /*program initializtion*/
    init_logger("gf2d.log");
    slog("---==== BEGIN ====---");
    gf2d_graphics_initialize(
        "Tamers of the Wicked",
        renderSize.x * renderScale.x,
        renderSize.y * renderScale.y,
        renderSize.x,
        renderSize.y,
        vector4d(200,200,200,255),
        0);
    gf2d_graphics_set_frame_delay(16);
    gfc_audio_init(32, 8, 0, 2, 1, 1);
    gf2d_sprite_init(1024);
    gf2d_font_init("config/font.cfg");
    SDL_ShowCursor(SDL_ENABLE);

    entity_manager_init(256);
    gui_manager_init(200);


    gfc_input_init("config/input_dualshock.cfg");
    /*demo setup*/
    //sprite = gf2d_sprite_load_image("images/backgrounds/bg_flat.png");
    mouse = gf2d_sprite_load_all("images/pointer.png",32,32,16,0);
    
    panel = gf2d_sprite_load_all("images/window_frame.png", 8, 8, 3, 0);
    cursor = gf2d_sprite_load_all("images/selection_cursor.png", 8, 7, 2, 0);
    barFrame = gf2d_sprite_load_all("images/bar_outline.png", 2, 2, 3, 0);
    barFill = gf2d_sprite_load_all("images/bar_fill.png", 4, 4, 1, 0);
    int spacing = 4;
    int hudStartY = 2 * spacing + 64;
    

    loadMain();
    
    //party_manager_new();
    int canQuit = 1;
    slog("Starting game loop.");
    /*main game loop*/
    while(!done)
    {
        SDL_PumpEvents();   // update SDL's internal event structures
        keys = SDL_GetKeyboardState(NULL); // get the keyboard state for this frame
        /*update things here*/
        SDL_GetMouseState(&mx,&my);
        mf+=0.1;
        if (mf >= 16.0)mf = 0;
        gfc_input_update();
        
        if (game_get_state() == GS_Title) {
            gui_update_all();
        }
        else {

            entity_think_all();
            entity_update_all();
            if (gfc_input_controller_button_pressed(0,"Share")) {
                if (game_get_wait_speed() >= 1.0) {
                    game_set_wait_speed(0.4);
                }
                else {
                    game_set_wait_speed(game_get_wait_speed() + 0.2);
                }
                slog("Set wait speed multiplier to %f.", game_get_wait_speed());
            }
            if (gfc_input_controller_button_pressed(0,"Options")) {
                if (game_get_state() == GS_Battle)
                    slog("You can't save in a battle, silly! What, do you think this is Undertale?");
                else
                    game_save();
            }
            switch (game_get_state()) {
            case GS_Battle:
                battle_update();
                break;
            case GS_Roaming:
                /*if (gfc_input_controller_button_pressed(0, "triangle")) {
                    if (in_battle())
                        kill_battle();
                    new_battle();
                }
                else if (gfc_input_controller_button_pressed(0, "square")) {
                    if (in_battle())
                        kill_battle();
                    new_hard_battle();
                }*/
                break;
            }
            /*
            if (changed) {
                gfc_block_cpy(((TextData*)(bDialogue)->data)->text, battleDialogue);
                ((TextData*)(bDialogue)->data)->currentChar = 0;
                changed = false;
            }
            */
            gui_update_all();

        }

        gf2d_graphics_clear_screen();// clears drawing buffers
        // all drawing should happen betweem clear_screen and next_frame
            //backgrounds drawn first
            //gf2d_sprite_draw_image(sprite,vector2d(0,0));

            if (game_get_state() == GS_Title) {
                Rect full = gfc_shape_rect(0, 0, game_get_resolution_x(), game_get_resolution_y()).s.r;
                gf2d_draw_rect_filled(full, gfc_color(0, 0, 0, 1));
            }
            //UI elements last
            if (game_get_state() == GS_Roaming && level_get_active_level()) {
                level_draw(level_get_active_level());
            }
            entity_draw_all();
            gui_draw_all();
            //gf2d_font_draw_line_tag("Fiends approach!\nCourse of action?", FT_Normal, gfc_color(0, 0, 0, 1), vector2d(12, renderSize.y / 2 + 12));
        //Vector2D frac = vector2d(1 / renderScale.x, 1 / renderScale.y);
            /*gf2d_sprite_draw(
                mouse,
                vector2d(mx / renderScale.x,my / renderScale.y),
                &frac,
                NULL,
                NULL,
                NULL,
                &mouseColor,
                (int)mf);*/
        gf2d_graphics_next_frame();// render current draw frame and skip to the next frame
        bgm_update();
        if ((canQuit && game_get_state() == GS_Title && keys[SDL_SCANCODE_ESCAPE]) || game_get_quitting())done = 1; // exit condition
        else if (game_get_state() != GS_Title && keys[SDL_SCANCODE_ESCAPE]) {
            exitToMain();
            canQuit = 0;
        }
        else if (!keys[SDL_SCANCODE_ESCAPE])
            canQuit = 1;
        //slog("Rendering at %f FPS",gf2d_graphics_get_frames_per_second());
    }
    slog("---==== END ====---");
    return 0;
}
/*eol@eof*/
