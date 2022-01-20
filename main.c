#include "util.h"

int main(int argc, const char *argv[])
{
    SDL_Window *window = NULL; 
    SDL_Renderer *renderer = NULL;
    Mix_Chunk *audio_chunks[2];

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

            if (initTexture(renderer, &playerTexture, "./assets/warrior-Sheet.png")
            & initTexture(renderer, &gunTexture, "./assets/tiny_gun_icons/pack.png"))
            {
                for (int i = 0; i < 2; i++) audio_chunks[i] = NULL;

                if (!(audio_chunks[0] = Mix_LoadWAV("./assets/low.wav")))
                    printf("Failed to load sound effect! SDL_mixer Error: %s\n", Mix_GetError());
                else
                {
                    printf("assets loaded!\n");

                    SDL_Event e;
                    P_TEST play_test;
                    P player;
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

                    play_test.style = P_TOPDOWN_SHOOT;
                    play_test.GUN_speed = 16;
                    play_test.GUN_delay = 20;
                    play_test.GUN_angle = 4;
                    play_test.GUN_deg = 0;
                    play_test.GUN_radx = 0;
                    play_test.GUN_rady = 0;
                    play_test.channel_volume = 64;
                    play_test.JOY_deg = 0;
                    play_test.JOY_use = false;
                    play_test.JOY_vel = 0;
                    play_test.JOY_xdir = 0;
                    play_test.JOY_ydir = 0;

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

                    play_test.crosshair.r.w = 4;
                    play_test.crosshair.r.h = 4;
                    play_test.crosshair.show = false;

                    play_test.club_r.w = 16;
                    play_test.club_r.h = 16;

                    play_test.goal_r.w = 48;
                    play_test.goal_r.h = 64;
                    play_test.goal_r.x = 48;
                    play_test.goal_r.y = 224;

                    initPlayerClips(p_clips);

                    player.texture = &playerTexture;
                    player.t_clips = p_clips;
                    player.dir = &play_test.input_q[0];
                    player.gvel = STANDARD_VELOCITY;
                    player.vel = STANDARD_VELOCITY;
                    player.shoot = false;
                    player.sprint = false;
                    player.sprint_timer = 0;
                    player.sprint_cdown = false;
                    player.sprint_cdown_timer = 0;

                    play_test.screen.w = W_WIDTH;
                    play_test.screen.h = W_HEIGHT;
                    play_test.screen.x = 0;
                    play_test.screen.y = 0;

                    play_test.camera.w = W_WIDTH;
                    play_test.camera.h = W_HEIGHT;
                    play_test.camera.x = 0;
                    play_test.camera.y = 0;

                    /* testing bullets */
                    for (int i = 0; i < 10; i++)
                    {
                        player.bullets[i].r.w = 4;
                        player.bullets[i].r.h = 4;
                        player.bullets[i].shoot = false;
                    }

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

                        initPlayer(&player, play_test.level, play_test.m_buffer);
                        setupPlay(&play_test, &player);

                        printf("init player and play\nstarting game loop\n");

                        // program loop
                        while (!play_test.quit)
                        {
                            timer = SDL_GetTicks();

                            SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
                            SDL_RenderClear(renderer);

                            playTopDownShooter(&play_test, e, &player);
                            updateTopDownShoot(&play_test, &player);
                            playRender(renderer, font, &play_test, &player);
                            
                            SDL_RenderPresent(renderer);

                            // limit framerate to ~60 
                            delta = SDL_GetTicks() - timer;
                            if (delta < TICKS) SDL_Delay(TICKS - delta);
                        }
                    }
                    else printf("Failed to load map!\n");

                    freeBuffers(&play_test);
                    freePlayer(&player);
                    freePlayTest(&play_test);
                }
            }
            else printf("Failed to init assets!\n");
        }
        else printf("Failed to load font!\n");
    }
    else printf("Failed to initialize SDL, %s\n", SDL_GetError());         

    closeSdl(&window, &renderer, audio_chunks);

    printf("program exited normally\n");

    return 0;
}
