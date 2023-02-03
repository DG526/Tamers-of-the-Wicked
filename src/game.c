#include <SDL.h>
#include "gf2d_graphics.h"
#include "gf2d_sprite.h"
#include "gf2d_font.h"
#include "gfc_input.h"
#include "simple_logger.h"
#include "totw_entity.h"
#include "totw_fiend.h"
#include "totw_gui.h"
#include "totw_battle.h"

static Bool quitNow = false;
static Bool changed = false;
static TextBlock battleDialogue = "";

void quit() {
    slog("Quitting...");
    quitNow = true;
}
void new_battle() {
    generate_new_battle("dummy",1);
    changed = true;
    gfc_block_cpy(battleDialogue,"Fiends approach!\nCourse of action?");
}
void new_hard_battle() {
    generate_new_battle("stupid",1);
    changed = true;
    gfc_block_cpy(battleDialogue,"Fiends approach!\nCourse of action?");
}
void fight() {
    if (!in_battle()) return;
    changed = true;
    gfc_block_cpy(battleDialogue,"Hey, wait a sec...\nyou can't do that yet...");
}
void flee() {
    if (!in_battle()) return;
    changed = true;
    gfc_block_cpy(battleDialogue,"Couldn't get away!");
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
    gf2d_sprite_init(1024);
    gf2d_font_init("config/font.cfg");
    SDL_ShowCursor(SDL_ENABLE);

    entity_manager_init(256);
    gui_manager_init(128);

    ally_list_load();

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
    /*
    GUI* bDialogueFrame = gui_window_create(vector2d(4, hudStartY + 4), vector2d(renderSize.x - 8, renderSize.y / 2 - 8 - 28), 0, panel);
    GUI* bDialogue = gui_text_create(vector2d(10, hudStartY + 10), "", 1, 1);
    
    GUI* bO_Fight = gui_option_create(vector2d(renderSize.x / 2, hudStartY + 10), "Fight", true, 1, cursor);
    GUI* bO_Flee = gui_option_create(vector2d(renderSize.x / 2, hudStartY + 10 + 12), "Flee", false, 1, cursor);
    GUI* bO_Die = gui_option_create(vector2d(renderSize.x / 2, hudStartY + 10 + 24), "Pass Out", false, 1, cursor);
    */
    /*
    Entity* p1 = party_read_member(0);
    slog("Basic fiend data read.");
    FiendData* dat = party_read_member_data(0);
    slog("Successfully loaded fiend, with size of %i, and stats of %i, %i, %i, %i, %i, %i.",dat->size,dat->stats[MHP],dat->stats[MMP],dat->stats[ATK],dat->stats[DEF],dat->stats[PWR],dat->stats[AGL]);

    char* numBuf[4];
    GUI* allyFrame1 = gui_window_create(vector2d(0, renderSize.y - 48), vector2d(renderSize.x / 4, 60), 0, panel);
    GUI* allyName1 = gui_text_create(vector2d(6, renderSize.y - 44), dat->name,false,1);
    sprintf(numBuf, "%i", dat->HP);
    GUI* hNum1 = gui_text_create(vector2d(4, renderSize.y - 28), numBuf, false, 2);
    sprintf(numBuf, "%i", dat->MP);
    GUI* mNum1 = gui_text_create(vector2d(4, renderSize.y - 15), numBuf, false, 2);
    GUI* hbar1 = gui_meter_create(vector2d(5 + 24, renderSize.y - 27), vector2d(renderSize.x / 4 - 32, 8), gfc_color(1, 0, 0, 1), 1, barFrame, barFill);
    GUI* mbar1 = gui_meter_create(vector2d(5 + 24, renderSize.y - 14), vector2d(renderSize.x / 4 - 32, 8), gfc_color(0, 0, 0.8, 1), 1, barFrame, barFill);
    ((MeterData*)(hbar1->data))->fill = (float)dat->HP / (float)dat->stats[MHP];
    ((MeterData*)(mbar1->data))->fill = (float)dat->MP / (float)dat->stats[MMP];

    Entity* p2 = party_read_member(1);
    slog("Basic fiend data read.");
    dat = party_read_member_data(1);
    slog("Successfully loaded fiend, with size of %i, and stats of %i, %i, %i, %i, %i, %i.", dat->size, dat->stats[MHP], dat->stats[MMP], dat->stats[ATK], dat->stats[DEF], dat->stats[PWR], dat->stats[AGL]);

    GUI* allyFrame2 = gui_window_create(vector2d(renderSize.x / 4, renderSize.y - 48), vector2d(renderSize.x / 4, 60), 0, panel);
    GUI* allyName2 = gui_text_create(vector2d(renderSize.x / 4 + 6, renderSize.y - 44), dat->name,false,1);
    sprintf(numBuf, "%i", dat->HP);
    GUI* hNum2 = gui_text_create(vector2d(renderSize.x / 4 + 4, renderSize.y - 28), numBuf, false, 2);
    sprintf(numBuf, "%i", dat->MP);
    GUI* mNum2 = gui_text_create(vector2d(renderSize.x / 4 + 4, renderSize.y - 15), numBuf, false, 2);
    GUI* hbar2 = gui_meter_create(vector2d(renderSize.x / 4 + 5 + 24, renderSize.y - 27), vector2d(renderSize.x / 4 - 32, 8), gfc_color(1, 0, 0, 1), 1, barFrame, barFill);
    GUI* mbar2 = gui_meter_create(vector2d(renderSize.x / 4 + 5 + 24, renderSize.y - 14), vector2d(renderSize.x / 4 - 32, 8), gfc_color(0, 0, 0.8, 1), 1, barFrame, barFill);
    ((MeterData*)(hbar2->data))->fill = (float)dat->HP / (float)dat->stats[MHP];
    ((MeterData*)(mbar2->data))->fill = (float)dat->MP / (float)dat->stats[MMP];

    allyFrame1->visible = true;
    allyName1->visible = true;
    hNum1->visible = true;
    mNum1->visible = true;
    hbar1->visible = true;
    mbar1->visible = true;
    allyFrame2->visible = true;
    allyName2->visible = true;
    hNum2->visible = true;
    mNum2->visible = true;
    hbar2->visible = true;
    mbar2->visible = true;
    */
    /*
    GUI* allyFrame3 = gui_window_create(vector2d(renderSize.x / 2, renderSize.y - 48), vector2d(renderSize.x / 4, 60), 0, panel);
    GUI* allyName3 = gui_text_create(vector2d(renderSize.x / 2 + 6, renderSize.y - 44), "Nialliv",false,1);
    sprintf(numBuf, "%i", 32);
    GUI* hNum3 = gui_text_create(vector2d(renderSize.x / 2 + 4, renderSize.y - 28), numBuf, false, 2);
    sprintf(numBuf, "%i", 75);
    GUI* mNum3 = gui_text_create(vector2d(renderSize.x / 2 + 4, renderSize.y - 15), numBuf, false, 2);
    GUI* hbar3 = gui_meter_create(vector2d(renderSize.x / 2 + 5 + 24, renderSize.y - 27), vector2d(renderSize.x / 4 - 32, 8), gfc_color(1, 0, 0, 1), 1, barFrame, barFill);
    GUI* mbar3 = gui_meter_create(vector2d(renderSize.x / 2 + 5 + 24, renderSize.y - 14), vector2d(renderSize.x / 4 - 32, 8), gfc_color(0, 0, 0.8, 1), 1, barFrame, barFill);
    ((MeterData*)(hbar3->data))->fill = ((float)32 / (float)170);
    ((MeterData*)(mbar3->data))->fill = ((float)75 / (float)236);

    GUI* allyFrame4 = gui_window_create(vector2d(3 * renderSize.x / 4, renderSize.y - 48), vector2d(renderSize.x / 4, 60), 0, panel);
    GUI* allyName4 = gui_text_create(vector2d(3 * renderSize.x / 4 + 6, renderSize.y - 44), "Dnegel", false, 1);
    sprintf(numBuf, "%i", 723);
    GUI* hNum4 = gui_text_create(vector2d(3 * renderSize.x / 4 + 4, renderSize.y - 28), numBuf, false, 2);
    sprintf(numBuf, "%i", 523);
    GUI* mNum4 = gui_text_create(vector2d(3 * renderSize.x / 4 + 4, renderSize.y - 15), numBuf, false, 2);
    GUI* hbar4 = gui_meter_create(vector2d(3 * renderSize.x / 4 + 5 + 24, renderSize.y - 27), vector2d(renderSize.x / 4 - 32, 8), gfc_color(1, 0, 0, 1), 1, barFrame, barFill);
    GUI* mbar4 = gui_meter_create(vector2d(3 * renderSize.x / 4 + 5 + 24, renderSize.y - 14), vector2d(renderSize.x / 4 - 32, 8), gfc_color(0, 0, 0.8, 1), 1, barFrame, barFill);
    ((MeterData*)(hbar4->data))->fill = ((float)723 / (float)999);
    ((MeterData*)(mbar4->data))->fill = ((float)523 / (float)999);
    
    if (bO_Fight && bO_Flee && bO_Die) {
        ((OptionData*)(bO_Fight->data))->up = bO_Die;
        ((OptionData*)(bO_Fight->data))->down = bO_Flee;
        ((OptionData*)(bO_Fight->data))->onSelect = fight;
        
        ((OptionData*)(bO_Flee->data))->up = bO_Fight;
        ((OptionData*)(bO_Flee->data))->down = bO_Die;
        ((OptionData*)(bO_Flee->data))->onSelect = flee;
        
        ((OptionData*)(bO_Die->data))->up = bO_Flee;
        ((OptionData*)(bO_Die->data))->down = bO_Fight;
        ((OptionData*)(bO_Die->data))->onSelect = quit;
    }
    */
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

        entity_think_all();
        entity_update_all();
        battle_update();
        if (gfc_input_controller_button_pressed(0, "triangle")) {
            if (in_battle())
                kill_battle();
            new_battle();
        }
        else if (gfc_input_controller_button_pressed(0, "square")) {
            if (in_battle())
                kill_battle();
            new_hard_battle();
        }
        /*
        if (changed) {
            gfc_block_cpy(((TextData*)(bDialogue)->data)->text, battleDialogue);
            ((TextData*)(bDialogue)->data)->currentChar = 0;
            changed = false;
        }
        */
        gui_update_all();

        gf2d_graphics_clear_screen();// clears drawing buffers
        // all drawing should happen betweem clear_screen and next_frame
            //backgrounds drawn first
            //gf2d_sprite_draw_image(sprite,vector2d(0,0));
            
            //UI elements last
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
        
        if (keys[SDL_SCANCODE_ESCAPE] || quitNow)done = 1; // exit condition
        //slog("Rendering at %f FPS",gf2d_graphics_get_frames_per_second());
    }
    slog("---==== END ====---");
    return 0;
}
/*eol@eof*/
