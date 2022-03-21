#include "util.h"

int main(int argc, const char *argv[])
{
    SDL_Window *window = NULL; 
    SDL_Renderer *renderer = NULL;
    Mix_Chunk *audio_chunks[5];

    if (initSdl(&window, &renderer))
    {
        //SDL_SetWindowResizable(window, true); // testing window resize
        //SDL_SetRelativeMouseMode(SDL_ENABLE);
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

            T playerTexture;

            printf("loading assets ...\n");

            if (initTexture(renderer, &playerTexture, "assets/warrior-Sheet.png"))
            {
                printf("texture assets loaded!\n");

                if (initAudio(audio_chunks))
                {
                    printf("audio assets loaded!\n");

                    SDL_Event e;
                    P_TEST play_test;
                    P players[2];
                    P_G goalie[2];
                    SDL_Rect buttons[3], p_clips[12], goal_r[2], gk_r[2];
                    Uint64 timer; 
                    int delta;

                    play_test.mix_chunks = audio_chunks;
                    play_test.gunTexture = NULL;
                    play_test.controller = NULL;

                    play_test.channel_volume = 32;

                    Mix_Volume(-1, 32);

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

                    printf("loading map ...\n");

                    play_test.m_buffer = NULL;
                    play_test.level.map = NULL;
                    play_test.level.collision = NULL;

                    if (loadMap(renderer, &play_test, "maps/pucko"))
                    {
                        initLevel(&play_test);
                        initMap(&play_test);

                        printf("map loaded!\n");

                        printf("init/setup game\n");

                        initButtons(buttons);
                        initPlayerClips(p_clips);

                        setupGoals(goal_r);
                        setupGoalKeepers(gk_r, goalie);
                        setupGame(&play_test, goal_r, gk_r, goalie);

                        initNet(&play_test.network);

                        play_test.buttons = buttons;

                        printf("game setup done!\n");

                        printf("init player\n");

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

                        // program loop
                        while (!play_test.quit)
                        {
                            timer = SDL_GetTicks64();

                            SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
                            SDL_RenderClear(renderer);

                            // ONLY for testing !!!
                            switch (play_test.g_state)
                            {
                                case G_JOIN:
                                    inputsJoin(&play_test, e);
                                break;
                                case G_HOST:
                                    inputsHost(&play_test, e);
                                break;
                                case G_HOSTING:
                                case G_JOINING:
                                    while (SDL_PollEvent(&e) != 0)
                                    {
                                        switch (e.type)
                                        {
                                            case SDL_QUIT:
                                                play_test.quit = true;
                                            break;
                                            case SDL_KEYDOWN:
                                                if (e.key.keysym.sym == SDLK_ESCAPE)
                                                {
                                                    play_test.network.pquit = true;
                                                }
                                            break;
                                        }
                                    }
                                break;
                                default: 
                                    if (play_test.is_net) inputsNetGame(&play_test, e);
                                    else inputsGame(&play_test, e);
                                break;
                            }

                            updateGame(&play_test, players);
                            renderGame(renderer, font, &play_test, players);
                            
                            SDL_RenderPresent(renderer);

                            // limit framerate to ~60 
                            delta = SDL_GetTicks64() - timer;
                            if (delta < TICKS) SDL_Delay(TICKS - delta);
                        }

                        printf("NET: close if online\n");
                        play_test.network.pquit = true;
                        play_test.network.lost = true;
                        closeNet(&play_test.network);
                    }
                    else printf("Failed to load map!\n");

                    printf("GAME: shutting down ...\n");

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

        printf("free font\n");
    }
    else printf("Failed to initialize SDL, %s\n", SDL_GetError());    

    closeSdl(&window, &renderer, audio_chunks);

    printf("program exited normally\n");

    return 0;
}
