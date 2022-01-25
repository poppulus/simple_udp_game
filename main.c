#include "util.h"

int main(int argc, const char *argv[])
{
    SDL_Window *window = NULL; 
    SDL_Renderer *renderer = NULL;
    Mix_Chunk *audio_chunks[5];

    if (initSdl(&window, &renderer))
    {
        //SDL_SetWindowResizable(window, true); // testing window resize
        SDL_SetRelativeMouseMode(SDL_ENABLE);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);   // testing alpha

        printf("loading font ...\n");

        FC_Font *font = FC_CreateFont();

        FC_LoadFont(
            font, 
            renderer, 
            "early_gameboy.ttf", 
            FONT_SIZE, 
            FC_MakeColor(0, 0, 0, 255), 
            TTF_STYLE_NORMAL);

        if (font != NULL) 
        {
            printf("font loaded!\n");

            T playerTexture, gunTexture;

            printf("loading assets ...\n");

            if (initTexture(renderer, &playerTexture, "assets/graphics/warrior-Sheet.png")
            & initTexture(renderer, &gunTexture, "assets/graphics/tiny_gun_icons/pack.png"))
            {
                bool audio_ok = true;

                for (int i = 0; i < 5; i++) 
                {
                    audio_chunks[i] = NULL;

                    switch (i)
                    {
                        case S_LOW:
                            if (!(audio_chunks[i] = Mix_LoadWAV("assets/sound/low.wav")))
                                audio_ok = false;
                        break;
                        case S_MEDIUM:
                            if (!(audio_chunks[i] = Mix_LoadWAV("assets/sound/medium.wav")))
                                audio_ok = false;
                        break;
                        case S_HIGH:
                            if (!(audio_chunks[i] = Mix_LoadWAV("assets/sound/high.wav")))
                                audio_ok = false;
                        break;
                        case S_SCRATCH:
                            if (!(audio_chunks[i] = Mix_LoadWAV("assets/sound/scratch.wav")))
                                audio_ok = false;
                        break;
                        case S_BEAT:
                            if (!(audio_chunks[i] = Mix_LoadWAV("assets/sound/beat.wav")))
                                audio_ok = false;
                        break;
                    }

                    if (!audio_ok)
                    {
                        printf("Failed to load sound effect! SDL_mixer Error: %s\n", Mix_GetError());
                        break;
                    }
                }

                if (audio_ok)
                {
                     printf("assets loaded!\n");

                    SDL_Event e;
                    P_TEST play_test;
                    P players[2];
                    SDL_Rect p_clips[12];
                    int timer, delta;

                    play_test.mix_chunks = audio_chunks;
                    play_test.gunTexture = &gunTexture;
                    play_test.controller = NULL;

                    for (int i = 0; i < SDL_NumJoysticks(); i++) 
                    {
                        if (SDL_IsGameController(i)) 
                        {
                            play_test.controller = SDL_GameControllerOpen(i);
                            if (play_test.controller) 
                            {
                                printf("Controller %d connected!\n", i);
                                break;
                            }
                            else fprintf(
                                stderr, 
                                "Could not open gamecontroller %i: %s\n", 
                                i, SDL_GetError());
                        }
                    }

                    initPlayerClips(p_clips);

                    play_test.style = P_TOPDOWN_SHOOT;
                    play_test.GUN_speed = 16;
                    play_test.GUN_delay = 20;
                    
                    play_test.channel_volume = 32;

                    Mix_Volume(-1, 32);

                    play_test.PUCK_freeze = false;
                    play_test.PUCK_freeze_timer = 0;

                    int y = 0;
                    for (int i = 0; i < 25; i++)
                    {
                        play_test.gunClips[i].w = 17;
                        play_test.gunClips[i].h = 19;
                        play_test.gunClips[i].x = ((i % 4) << 4) + 1;
                        play_test.gunClips[i].y = (y << 4) + 2;

                        if ((i % 4) == 3) y++;
                    }

                    play_test.bullet_hits.index = 0;

                    for (int i = 0; i < 30; i++)
                    {
                        play_test.bullet_hits.a[i].used = false;
                        play_test.bullet_hits.a[i].x = 0;
                        play_test.bullet_hits.a[i].y = 0;
                    }

                    play_test.goal_r.w = 48;
                    play_test.goal_r.h = 64;
                    play_test.goal_r.x = 48;
                    play_test.goal_r.y = 224;

                    play_test.sprint_hud_r.w = 0;
                    play_test.sprint_hud_r.h = 16;
                    play_test.sprint_hud_r.x = 300;
                    play_test.sprint_hud_r.y = W_HEIGHT - 26;

                    play_test.screen.w = W_WIDTH;
                    play_test.screen.h = W_HEIGHT;
                    play_test.screen.x = 0;
                    play_test.screen.y = 0;

                    play_test.camera.w = W_WIDTH;
                    play_test.camera.h = W_HEIGHT;
                    play_test.camera.x = 0;
                    play_test.camera.y = 0;

                    play_test.m_buffer = NULL;
                    play_test.level.map = NULL;
                    play_test.level.collision = NULL;

                    play_test.w_focus = true;

                    printf("loading map ...\n");

                    if (loadMap(renderer, &play_test, "maps/placeholder"))
                    {
                        initLevel(&play_test);
                        initMap(&play_test);

                        printf("map loaded!\n");

                        for (int i = 0; i < 2; i++)
                        {
                            players[i].texture = &playerTexture;
                            players[i].t_clips = p_clips;

                            initPlayer(
                                &players[i], 
                                play_test.level, 
                                play_test.m_buffer);
                        }

                        setupPlay(&play_test, &players[0]);

                        printf("init player and starting game loop\n");

                        // program loop
                        while (!play_test.quit)
                        {
                            timer = SDL_GetTicks();

                            SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
                            SDL_RenderClear(renderer);

                            playTopDownShooter(&play_test, e);
                            updateTopDownShoot(&play_test, players);
                            playRender(renderer, font, &play_test, players);
                            
                            SDL_RenderPresent(renderer);

                            // limit framerate to ~60 
                            delta = SDL_GetTicks() - timer;
                            if (delta < TICKS) SDL_Delay(TICKS - delta);
                        }
                    }
                    else printf("Failed to load map!\n");

                    freeBuffers(&play_test);
                    freePlayer(players);
                    freePlayTest(&play_test);
                }
                else printf("Failed to init sound assets!\n");
            }
            else printf("Failed to init texture assets!\n");
        }
        else printf("Failed to load font!\n");

        FC_FreeFont(font);
        font = NULL;

        printf("free font\n");
    }
    else printf("Failed to initialize SDL, %s\n", SDL_GetError());         

    closeSdl(&window, &renderer, audio_chunks);

    printf("program exited normally\n");

    return 0;
}
