/*================================================================
    * Copyright: 2020 John Jackson
    * Audio

    Audio example.

    Press `esc` to exit the application.
=================================================================*/

#define GS_IMPL
#include <gs/gs.h>

gs_handle(gs_audio_source_t) src_hndl = {0};
gs_handle(gs_audio_source_t) mus_hndl = {0};
gs_handle(gs_audio_instance_t) inst_hndl = {0};

void init()
{
    // Create audio source from file and capture resource handle
    src_hndl = gs_audio_load_from_file("./assets/punch.wav");

    // Persistent audio (music)
    mus_hndl = gs_audio_load_from_file("./assets/music.mp3");

    // Create instance decl, play sound to be looped
    inst_hndl = gs_audio_instance_create(
        &(gs_audio_instance_decl_t){
            .src = mus_hndl,
            .persistent = true,
            .volume = 0.1f,
            .loop = true 
        }
    );
    gs_audio_play(inst_hndl);
}

void update()
{
    if (gs_platform_key_pressed(GS_KEYCODE_ESC)) gs_engine_quit();

    // If press button, play transient, non-persistent audio sample from source
    // with 0.5f volume (volume is normalized 0-1)
    // This sample will be cleaned up automatically by internal resources when done playing.
    if (gs_platform_key_pressed(GS_KEYCODE_O)) {
        gs_audio_play_source(src_hndl, 0.5f);
    }

    // Pause/Play music instance
    if (gs_platform_key_pressed(GS_KEYCODE_P)) {
        if (gs_audio_is_playing(inst_hndl)) gs_audio_pause(inst_hndl);
        else                                gs_audio_play(inst_hndl);
    }

    // Stop music instance
    if (gs_platform_key_pressed(GS_KEYCODE_S)) {
        gs_audio_stop(inst_hndl);
    }

    // Volume up
    if (gs_platform_key_pressed(GS_KEYCODE_UP)) {
        f32 vol = gs_audio_get_volume(inst_hndl);
        gs_audio_set_volume(inst_hndl, vol + 0.1f);
    }

    // Volume down
    if (gs_platform_key_pressed(GS_KEYCODE_DOWN)) {
        f32 vol = gs_audio_get_volume(inst_hndl);
        gs_audio_set_volume(inst_hndl, vol - 0.1f);
    }
}

gs_app_desc_t gs_main(int32_t argc, char** argv)
{
    return (gs_app_desc_t){
        .init = init,
        .update = update
    };
}   




