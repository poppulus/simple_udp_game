#include "net.h"

void closeNet(NET *net)
{
    net->tquit = 1;
    net->lost = 1;

    printf("NET: wait on thread if any\n");
    // wait on thread and free
    if (net->thread != NULL) 
    {
        SDL_WaitThread(net->thread, NULL);
        net->thread = NULL;
        printf("THREAD: kill\n");
    }

    net->tquit = 0;
    net->lost = 0;

    printf("NET: close socket\n");
    // free socket
    SDLNet_UDP_Close(net->connection.sd);
    net->connection.sd = NULL;

    printf("NET: free udp packets\n");
    // free packet pointers
    for (unsigned char i = 0; i < 2; i++) 
    {
        if (net->connection.pks[i] != NULL) 
        {
            SDLNet_FreePacket(net->connection.pks[i]);
            net->connection.pks[i] = NULL;
        }
    }

    printf("NET: free users\n");
    // free users
    if (net->users != NULL)
    {
        free(net->users);
        net->users = NULL;
    }

    printf("NET: free players\n");
    // free players
    if (net->players_net != NULL)
    {
        free(net->players_net);
        net->players_net = NULL;
    }

    net->puck.id = 0;
    net->puck.x = 0;
    net->puck.y = 0;

    net->goalkeepers.gk1_status = 0;
    net->goalkeepers.gk2_status = 0;

    printf("NET: reset local user\n");
    net->localplayer = NULL;
    resetUser(&net->localuser);

    printf("NET: reset other values\n");
    net->numplayers = 0;
    net->numusers = 0;
    net->play_state = GNET_DROP;
    net->type = 0;
    net->join = 0;
    net->left = 0;
    net->ok = 0;

    printf("NET: exit\n");
    SDLNet_Quit();
}

int startNetHost(NET *net, int port)
{
    int success = 1;

    if (initNetHost(&net->connection.sd, net->connection.pks, port))
    {
        printf("NET: host connection established\n");

        net->users = calloc(MAX_NET_USERS, sizeof(UDPuser));
        net->players_net = calloc(MAX_NET_USERS, sizeof(P_NET));

        net->players_net[0].id = HOST_ID;
        net->numplayers = 1;

        net->localplayer = &net->players_net[0];

        if ((net->thread = SDL_CreateThread(host_thread, "host", net)) == NULL)
        {
            printf("ERROR: SDL_CreateThread, %s\n", SDL_GetError());
            net->tquit = 1;
            net->lost = 1;
            closeNet(net);
            success = 0;
        }
        else printf("NET: host thread starting\n");
    } 
    else success = 0;

    return success;
}

int startNetClient(NET *net, const char *string)
{
    int success = 1;

    // setup connection
    if (initNetClient(net, string))
    {
        printf(
            "NET: client connection established on: %d:%d\n", 
            net->connection.hostaddr.host, 
            net->connection.hostaddr.port);

        net->players_net = calloc(MAX_NET_USERS, sizeof(P_NET));

        memset(
            net->connection.pks[PEER_PACKET]->data, 
            0, 
            net->connection.pks[PEER_PACKET]->maxlen);

        setupClientConnect(
            net->connection.pks[PEER_PACKET], net->connection.hostaddr);

        if ((net->thread = SDL_CreateThread(client_thread, "client", net)) == NULL)
        {
            printf("ERROR: SDL_CreateThread, %s\n", SDL_GetError());
            closeNet(net);
            success = 0;
        }
        else printf("NET: client thread starting\n");
    }
    else success = 0;

    return success;
}

void initNet(NET *net)
{
    net->numusers = 0;
    net->numplayers = 0;
    net->play_state = GNET_DROP;
    net->type = 0;

    net->lost = 0;
    net->tquit = 0;
    net->join = 0;
    net->left = 0;
    net->ok = 0;

    net->thread = NULL;
    net->users = NULL;
    net->players_net = NULL;
    net->localplayer = NULL;

    net->localuser.id = 0;
    net->localuser.address.host = 0;
    net->localuser.address.port = 0;
    net->localuser.status = 0;
    net->localuser.timeout = 0;

    net->puck.id = 0;
    net->puck.x = 0;
    net->puck.y = 0;

    net->goalkeepers.gk1_status = 0;
    net->goalkeepers.gk2_status = 0;

    net->connection.sd = NULL;
    net->connection.pks[HOST_PACKET] = NULL;
    net->connection.pks[PEER_PACKET] = NULL;
}

int initNetHost(UDPsocket *sd, UDPpacket **pks, unsigned short port)
{
    if (SDLNet_Init() < 0)
	{
		fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
		return 0;
	}	

	if (!(*sd = SDLNet_UDP_Open(port)))
	{
		fprintf(stderr, "SDLNet_UDP_Open: %s\n", SDLNet_GetError());
		return 0;
	}

    printf("NET: UDP port %d\n", port);

    if (!(pks[HOST_PACKET] = SDLNet_AllocPacket(NET_BUFFER_SIZE))) 
    { 
        fprintf(stderr, "SDLNet_AllocPacket: %s\n", SDLNet_GetError()); 
        return 0;
    }

    if (!(pks[PEER_PACKET] = SDLNet_AllocPacket(NET_BUFFER_SIZE))) 
    { 
        fprintf(stderr, "SDLNet_AllocPacket: %s\n", SDLNet_GetError()); 
        return 0;
    }

    return 1;
}

int initNetClient(NET *net, const char *string)
{
    char *host = NULL;
    Uint16 port = 0;
    int len = 0;

    printf("NET: init\n");
    
    if (SDLNet_Init() < 0)
	{
		fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
		return 0;
	}	

    printf("NET: open udp on socket\n");

    if (!(net->connection.sd = SDLNet_UDP_Open(0)))
	{
		fprintf(stderr, "SDLNet_UDP_Open: %s\n", SDLNet_GetError());
		return 0;
	}

    len = strlen(string);

    unsigned char n = 0;

    for (unsigned char i = len - 1; i > 0; i--)
    {
        if (string[i] == ':' && (i + 1) < 64)
        {
            n = 1;
            port = atoi(&string[i + 1]);
            host = calloc(i, 1);
            memcpy(host, string, i);
            break;
        }
    }

    if (!n)
    {
        printf("NET: incorrect IP, example: %s\n", "123.123.123.123:9999");
    }

    printf("host %s, port %d\n", host, port);
    
    if (SDLNet_ResolveHost(&net->connection.hostaddr, host, port))
    { 
        fprintf(
            stderr, 
            "SDLNet_ResolveHost(%d %d): %s\n", 
            net->connection.hostaddr.host, net->connection.hostaddr.port, SDLNet_GetError()); 
        if (host != NULL) free(host);
        return 0;
    }

    if (host != NULL) free(host);

    printf("NET: allocate packets\n");

    if (!(net->connection.pks[PEER_PACKET] = SDLNet_AllocPacket(NET_BUFFER_SIZE))) 
    { 
        fprintf(stderr, "SDLNet_AllocPacket: %s\n", SDLNet_GetError()); 
        return 0;
    }

    if (!(net->connection.pks[HOST_PACKET] = SDLNet_AllocPacket(NET_BUFFER_SIZE))) 
    { 
        fprintf(stderr, "SDLNet_AllocPacket: %s\n", SDLNet_GetError()); 
        return 0;
    }

    return 1;
}

int host_thread(void *ptr)
{
    NET *net = ptr;
    int quit = 0, player_id = 2, recv = 0, send = -1, delta = 0;
    unsigned long timer = 0;
    long timeout = 0;

    printf("THREAD: start\nNET: UDP server now listening ...\n");

    //net->localuser.id = HOST_ID;
    //net->localuser.status = N_PLAY;
    //net->localuser.timeout = SDL_GetTicks64();
    
    net->ok = 1;

    while (!quit && !net->tquit)
    {
        timer = SDL_GetTicks64();

        if ((recv = SDLNet_UDP_Recv(net->connection.sd, net->connection.pks[PEER_PACKET])) > 0) 
        {
            switch (net->connection.pks[PEER_PACKET]->data[NET_CLIENT_STATUS])
            {
                case N_CONNECT:
                    hostHandleConnect(
                        net->users, 
                        net->connection.pks, 
                        &net->numusers, 
                        &player_id);
                break;
                case N_DISCONNECT:
                    hostHandleDisconnect(
                        net->users, 
                        net->connection.pks[PEER_PACKET]->data[NET_CLIENT_ID], 
                        net->players_net, 
                        &net->numusers, 
                        &net->numplayers);

                    net->left = 1;
                break;
                case N_NEWID:
                    hostHandleNewId(
                        net->users, 
                        net->players_net, 
                        net->connection.pks[PEER_PACKET]->data[NET_CLIENT_ID], 
                        &net->numplayers);

                    net->join = 1;
                break;
                case N_PLAY:
                    hostHandlePlay(
                        net->users, 
                        net->players_net, 
                        net->connection.pks[PEER_PACKET],
                        net->play_state);
                break;
            }
        } 
        
        for (unsigned char i = 0; i < MAX_NET_USERS; i++)
        {
            if (net->users[i].id > 0)
            {
                timeout = timer - net->users[i].timeout;

                if (timeout >= 3000)
                {
                    for (unsigned char j = 0; j < MAX_NET_USERS; j++)
                    {
                        if (net->players_net[j].id == net->users[i].id)
                        {
                            printf(
                                "TIMEOUT: no player %d data\nUSER: current players: %d\n", 
                                net->players_net[j].id, 
                                net->numplayers - 1);

                            removePlayerNet(&net->players_net[j]);
                            net->numplayers--;
                            net->left = 1;
                            break;
                        }
                    }

                    // remove user
                    removeUser(&net->users[i]);
                    net->numusers--;

                    if (!net->numusers) printf("USER: no connected users!\n");
                    else printf("USER: user count: %d\n", net->numusers);
                    continue;
                }

                switch (net->users[i].status)
                {
                    case N_CONNECT:
                        hostSendConnect(
                            net->connection.pks[HOST_PACKET], 
                            net->users[i].address,
                            net->numplayers);

                        send = SDLNet_UDP_Send(net->connection.sd, -1, net->connection.pks[HOST_PACKET]);
                    break;
                    case N_DISCONNECT:
                        hostSendDisconnect(
                            net->connection.pks[HOST_PACKET], 
                            net->users[i].address,
                            net->numplayers);

                        send = SDLNet_UDP_Send(net->connection.sd, -1, net->connection.pks[HOST_PACKET]);
                    break;
                    case N_NEWID:
                        hostSendNewId(
                            net->connection.pks[HOST_PACKET], 
                            net->users[i].address,
                            net->numplayers,
                            net->users[i].id);

                        send = SDLNet_UDP_Send(net->connection.sd, -1, net->connection.pks[HOST_PACKET]);
                    break;
                    case N_PLAY:
                        hostSendPlay(
                            net->connection.pks[HOST_PACKET], 
                            net->users[i].address, *net);

                        send = SDLNet_UDP_Send(net->connection.sd, -1, net->connection.pks[HOST_PACKET]);
                    break;
                }
            }
        }

        if (recv == -1)
        {
            printf("NET: error on receive\n");
            net->tquit = 1;
        }
        else if (!send)
        {
            printf("NET: error on send\n");
            net->tquit = 1;
        }
        
        memset(
            net->connection.pks[PEER_PACKET]->data, 
            0, 
            net->connection.pks[PEER_PACKET]->maxlen);

        memset(
            net->connection.pks[HOST_PACKET]->data, 
            0, 
            net->connection.pks[HOST_PACKET]->maxlen);

        // set tickrate to ~60/second
        delta = SDL_GetTicks64() - timer;
        if (delta < TICKRATE) SDL_Delay(TICKRATE - delta);
    }

    return 0;
}

void addUser(UDPuser *user, char id, int host, unsigned short port)
{
    user->id = id;
    user->status = N_NEWID;
    user->timeout = SDL_GetTicks64();
    user->address.host = host;
    user->address.port = port;
}

void removeUser(UDPuser *user)
{
    user->id = 0;
    user->status = 0;
    user->timeout = 0;
    user->address.host = 0;
    user->address.port = 0;
}

void addPlayerNet(P_NET *p, int id, int state)
{
    p->id = id;
    p->state = state;
    p->x = 0;
    p->y = 0;
}

void removePlayerNet(P_NET *p)
{
    p->id = 0;
    p->state = 0;
    p->x = 0;
    p->y = 0;
}

void resetNet(NET *net)
{
    net->numusers = 0;
    net->numplayers = 0;
    net->play_state = GNET_DROP;

    net->lost = 0;
    net->tquit = 0;
    net->join = 0;
    net->left = 0;
    net->ok = 0;

    net->thread = NULL;
    net->users = NULL;
    net->players_net = NULL;
    net->localplayer = NULL;

    net->localuser.id = 0;
    net->localuser.address.host = 0;
    net->localuser.address.port = 0;
    net->localuser.status = 0;
    net->localuser.timeout = 0;

    net->puck.id = 0;
    net->puck.x = 0;
    net->puck.y = 0;

    net->connection.sd = NULL;
    net->connection.pks[HOST_PACKET] = NULL;
    net->connection.pks[PEER_PACKET] = NULL;
}

void resetUser(UDPuser *u)
{
    u->address.host = 0;
    u->address.port = 0;
    u->status = 0;
    u->timeout = 0;
    u->id = 0;
}

int client_thread(void *ptr)
{
    NET *net = ptr;

    printf("THREAD: start\n");

    int delta = 0, response = 0, send = -1, pkg_len = 0;
    unsigned long timer = 0, response_timer = SDL_GetTicks64();
    long timeout = 0;

    short   *puck_buffer = NULL, 
            *user_pos_buf = NULL;

    float   *user_angle = NULL;

    unsigned char *user_buffer = NULL;

    while (!net->lost && !net->tquit)
    {
        timer = SDL_GetTicks64();

        if ((response = SDLNet_UDP_Recv(net->connection.sd, net->connection.pks[HOST_PACKET])) > 0)
        {
            response_timer = SDL_GetTicks64();
            
            switch (net->connection.pks[HOST_PACKET]->data[NET_HOST_STATUS])
            {
                case N_CONNECT:
                break;
                case N_DISCONNECT:
                    if (net->localuser.status != N_DISCONNECT)
                    {
                        net->localuser.status = N_DISCONNECT;
                    }
                    else 
                    {
                        // break out of loop
                        net->lost = 1;
                    }
                break;
                case N_NEWID:
                    if (net->localuser.status == N_CONNECT)
                    {
                        net->localuser.id = net->connection.pks[HOST_PACKET]->data[NET_HOST_USERID];
                        net->localuser.status = N_NEWID;
                    }
                break;
                case N_PLAY:
                    if (net->localuser.status == N_NEWID) 
                    {
                        net->localuser.status = N_PLAY;

                        //setupClientPlay(
                        //    net->connection.pks[PEER_PACKET], 
                        //    net->connection.pks[HOST_PACKET]->address, 
                        //    net->localuser.id, net->localplayer);

                        net->ok = 1;
                        printf("NET: client ok\n");
                    }
                    else if (net->localuser.status == N_PLAY)
                    {
                        // handle incoming data 
                        puck_buffer = (short *)&net->connection.pks[HOST_PACKET]->data[NET_HOST_PUCKDATA];
                        user_buffer = (unsigned char *)&net->connection.pks[HOST_PACKET]->data[NET_HOST_USERANGLE];
                        pkg_len = (net->connection.pks[HOST_PACKET]->len - NET_BUFFER_PRESET);

                        net->play_state = net->connection.pks[HOST_PACKET]->data[NET_HOST_STATE];

                        net->goalkeepers.gk1_status = net->connection.pks[HOST_PACKET]->data[NET_HOST_GK1];
                        net->goalkeepers.gk2_status = net->connection.pks[HOST_PACKET]->data[NET_HOST_GK2];

                        net->puck.id = net->connection.pks[HOST_PACKET]->data[NET_HOST_PUCKID];
                        net->puck.x = puck_buffer[0];
                        net->puck.y = puck_buffer[1];

                        if (net->connection.pks[HOST_PACKET]->data[NET_HOST_NUSERS] > net->numplayers)
                        {
                            printf("NET: handle new user(s)\n");
                            clientHandleNewUser(
                                net->players_net, 
                                &user_buffer[8], 
                                pkg_len, 
                                &net->numplayers);

                            net->join = 1;
                        }
                        else if (net->connection.pks[HOST_PACKET]->data[NET_HOST_NUSERS] < net->numplayers)
                        {
                            printf("NET: handle lost user(s)\n");
                            clientHandleLostUser(
                                net->players_net, 
                                &user_buffer[8], 
                                pkg_len, 
                                &net->numplayers);
                                
                            net->left = 1;
                        }

                        if (net->localplayer == NULL)
                        {
                            printf("NET: local user id %d\n", net->localuser.id);

                            for (unsigned char i = 0; i < MAX_NET_USERS; i++)
                            {
                                if (net->players_net[i].id == net->localuser.id)
                                {
                                    net->localplayer = &net->players_net[i];
                                    break;
                                }
                            }

                            if (net->localplayer)
                                printf("PLAYER: local id: %d\n", net->localplayer->id);
                            else 
                                printf("PLAYER: could not set local player!\n");
                        }

                        for (unsigned char i = 0; i < MAX_NET_USERS; i++)
                        {
                            if (net->players_net[i].id == user_buffer[(i * NET_BUFFER_PLAYER) + 8])
                            {
                                user_angle = (float *)&user_buffer[(i * NET_BUFFER_PLAYER)];
                                user_pos_buf = (short *)&user_buffer[(i * NET_BUFFER_PLAYER) + 4];

                                if (net->players_net[i].id != net->localuser.id)
                                {
                                    net->players_net[i].angle = *user_angle;
                                    net->players_net[i].x = user_pos_buf[0];
                                    net->players_net[i].y = user_pos_buf[1];
                                    net->players_net[i].state = user_buffer[(i * NET_BUFFER_PLAYER) + 9];
                                }
                                else 
                                {
                                    if (net->play_state == GNET_DROP)
                                    {
                                        net->players_net[i].x = user_pos_buf[0];
                                        net->players_net[i].y = user_pos_buf[1];
                                    }
                                }
                            }
                        }
                    }
                break;
            }
        }

        switch (net->localuser.status)
        {
            case N_CONNECT:
                setupClientConnect(
                    net->connection.pks[PEER_PACKET], 
                    net->connection.hostaddr);

                send = (
                    SDLNet_UDP_Send(
                        net->connection.sd, -1, net->connection.pks[PEER_PACKET]));
            break;
            case N_DISCONNECT:
                setupClientDisconnect(
                    net->connection.pks[PEER_PACKET], 
                    net->localuser.id, 
                    net->connection.hostaddr);

                send = (
                    SDLNet_UDP_Send(
                        net->connection.sd, -1, net->connection.pks[PEER_PACKET]));
            break;
            case N_NEWID:
                setupClientNewId(
                    net->connection.pks[PEER_PACKET], 
                    net->localuser.id, 
                    net->connection.hostaddr);

                send = (
                    SDLNet_UDP_Send(
                        net->connection.sd, -1, net->connection.pks[PEER_PACKET]));
            break;
            case N_PLAY:
                setupClientPlay(
                    net->connection.pks[PEER_PACKET], 
                    net->connection.hostaddr, 
                    net->localuser.id, net->localplayer);

                send = (
                    SDLNet_UDP_Send(
                        net->connection.sd, -1, net->connection.pks[PEER_PACKET]));
            break;
        }

        if (!response)
        {
            timeout = timer - response_timer;

            if (timeout >= 3000)
            {
                printf("RECEIVE: no response from host for 3 seconds\n");
                net->lost = 1;
            }
        }
        else if (response < 0) 
        {
            printf("ERROR: no response from host\n");
            net->lost = 1;
        }
        else timeout = 0;

        if (!send)
        {
            printf("ERROR: no send to host\n");
            net->lost = 1;
        }

        memset(
            net->connection.pks[HOST_PACKET]->data, 
            0, 
            net->connection.pks[HOST_PACKET]->maxlen);

        memset(
            net->connection.pks[PEER_PACKET]->data, 
            0, 
            net->connection.pks[PEER_PACKET]->maxlen);

        // set tickrate to ~60
        delta = SDL_GetTicks64() - timer;
        if (delta < TICKRATE) SDL_Delay(TICKRATE - delta);
    }

    printf("THREAD: exit\n");

    return 0;
}

void setupClientConnect(UDPpacket *p, IPaddress ip)
{
    p->address.host = ip.host;
    p->address.port = ip.port;

    p->data[NET_CLIENT_STATUS] = N_CONNECT;
    p->data[NET_CLIENT_ID] = 0;

    p->len = NET_CLIENT_SEND_SIZE;
}

void setupClientNewId(UDPpacket *p, char id, IPaddress ip)
{
    p->address.host = ip.host;
    p->address.port = ip.port;

    p->data[NET_CLIENT_STATUS] = N_NEWID;
    p->data[NET_CLIENT_ID] = id;

    p->len = NET_CLIENT_SEND_SIZE;
}

void setupClientDisconnect(UDPpacket *p, char id, IPaddress ip)
{
    p->address.host = ip.host;
    p->address.port = ip.port;

    p->data[NET_CLIENT_STATUS] = N_DISCONNECT;
    p->data[NET_CLIENT_ID] = id;

    p->len = NET_CLIENT_SEND_SIZE;
}

void setupClientPlay(UDPpacket *p, IPaddress ip, char id, P_NET *localplayer)
{
    p->address.host = ip.host;
    p->address.port = ip.port;

    p->data[NET_CLIENT_STATUS] = N_PLAY;
    p->data[NET_CLIENT_ID] = id;

    if (localplayer != NULL)
    {
        memcpy(
            (unsigned char *)&p->data[NET_CLIENT_USERANGLE], 
            localplayer, NET_BUFFER_PLAYER);
    }

    p->len = NET_CLIENT_SEND_SIZE;
}

void printPacketInfo(UDPpacket *pkt)
{
    printf("UDP Packet incoming\n"); 
    printf("\tChan: %d\n", pkt->channel); 
    printf("\tData STATUS: %d\n", pkt->data[NET_CLIENT_STATUS]); 
    printf("\tData ID: %d\n", pkt->data[NET_CLIENT_ID]); 
    printf("\tData STRING: %s\n", (char *)&pkt->data[NET_CLIENT_USERID]);
    printf("\tLen: %d\n", pkt->len); 
    printf("\tMaxlen: %d\n", pkt->maxlen); 
    printf("\tStatus: %d\n", pkt->status); 
    printf("\tAddress: %x %x\n", pkt->address.host, pkt->address.port);
}

void clientHandleConnect()
{

}

void clientHandleDisconnect()
{

}

void clientHandleNewId()
{

}

void clientHandlePlay(P_NET *players, UDPpacket *packet, unsigned char *numplayers)
{
    
}

void clientHandleNewUser(P_NET *plrs, unsigned char *buf, int pkg_len, unsigned char *numplayers)
{
    unsigned char n = 0;
    
    for (unsigned char i = 0; i < pkg_len; i += NET_BUFFER_PLAYER)
    {
        n = i / 12;
        if (buf[i] && !plrs[n].id)
        {
            printf("USER: found new id:%d\n", buf[i]);
            addPlayerNet(&plrs[n], buf[i], buf[i + 1]);
            (*numplayers)++;
        }
        /*
        for (unsigned j = 0; j < MAX_NET_USERS; j++)
        {
            if (buf[i] == plrs[j].id) 
            {
                n = 1;
                break;
            }
            else if (!plrs[j].id) 
            {
                n = 1; 
                break;
            }
        }
        if (!n)
        {
            printf("USER: found new id:%d\n", buf[i]);
            for (unsigned p = 0; p < MAX_NET_USERS; p++)
            {
                if (!plrs[p].id)
                {
                    addPlayerNet(&plrs[p], buf[i], buf[i + 1]);
                    (*numplayers)++;
                    break;
                }
            }
        }
        n = 0;
        */
    }
}

void clientHandleLostUser(P_NET *plrs, unsigned char *buf, int pkg_len, unsigned char *numplayers)
{
    unsigned char n = 0;

    for (unsigned i = 0; i < pkg_len; i += NET_BUFFER_PLAYER)
    {
        n = i / 12;
        if (!buf[i] && plrs[n].id)
        {
            printf("USER: found lost id: %d\n", plrs[n].id);
            removePlayerNet(&plrs[n]);
            (*numplayers)--;
        }
        /*
        if (plrs[i].id == buf[j]) 
        {
            n = 1;
            break;
        }
        else if (!plrs[i].id) 
        {
            n = 1; 
            break;
        }
    
    if (!n)
    {
        printf("USER: found lost id: %d\n", plrs[i].id);
        removePlayerNet(&plrs[i]);
        (*numplayers)--;
    }
    n = 0;
    */
    }
    
}

void clientSendConnect()
{

}

void clientSendDisconnect()
{

}

void clientSendNewId()
{

}

void clientSendPlay()
{

}

void hostHandleConnect(UDPuser *users, UDPpacket **pks, unsigned char *numusers, int *newid)
{
    for (unsigned char i = 0; i < MAX_NET_USERS; i++)
    {
        if (users[i].address.host == pks[PEER_PACKET]->address.host 
        && users[i].address.port == pks[PEER_PACKET]->address.port) break;

        else if (!users[i].id)
        {
            if ((*numusers) < (MAX_NET_USERS - 1))
            {
                printf("USER: new id: %d\n", (*newid));

                addUser(
                    &users[i], 
                    (*newid), 
                    pks[PEER_PACKET]->address.host, 
                    pks[PEER_PACKET]->address.port);

                (*newid)++;
                (*numusers)++;

                printf("USER: user count: %d\n", (*numusers));
            }
            break;
        }
    }
}

void hostHandleDisconnect(
    UDPuser *users, 
    unsigned char peerid, 
    P_NET *players, 
    unsigned char *numusers, 
    unsigned char *numplayers)
{
    for (unsigned char i = 0; i < MAX_NET_USERS; i++)
    {
        if (users[i].id == peerid)
        {
            users[i].timeout = SDL_GetTicks64();

            if (users[i].status != N_DISCONNECT) users[i].status = N_DISCONNECT;
            else 
            {
                for (unsigned char j = 0; j < MAX_NET_USERS; j++)
                {
                    if (players[j].id == users[i].id)
                    {
                        removePlayerNet(&players[j]);
                        (*numplayers)--;
                    }
                }

                printf("USER: user %d removed\n", users[i].id);

                removeUser(&users[i]);
                (*numusers)--;

                if (*numusers < 1) printf("USER: no connected users!\n");
                else printf("USER: user count: %d\n", *numusers);
            }
            break;
        }
    }
}

void hostHandleNewId(UDPuser *users, P_NET *players, unsigned char peerid, unsigned char *numplayers)
{
    for (unsigned char i = 0; i < MAX_NET_USERS; i++)
    {
        if (users[i].id == peerid)
        {
            if (users[i].status == N_NEWID)
            {
                users[i].timeout = SDL_GetTicks64();
                users[i].status = N_PLAY;

                printf("USER: user %d set status to play\n", users[i].id);

                for (unsigned char j = 0; j < MAX_NET_USERS; j++)
                {
                    if (!players[j].id)
                    {
                        addPlayerNet(&players[j], users[i].id, PNET_NONE);
                        (*numplayers)++;
                        printf("USER: add user %d to player list\n", users[i].id);
                        break;
                    }
                }
            }
            break;
        }
    }
}

void hostHandlePlay(UDPuser *users, P_NET *players, UDPpacket *pack, unsigned char state)
{
    float *angle = (float *)&pack->data[NET_CLIENT_USERANGLE];
    short *buffer = (short *)&pack->data[NET_CLIENT_USERPOS];

    for (unsigned char i = 0; i < MAX_NET_USERS; i++)
    {
        if (users[i].id && users[i].id == pack->data[NET_CLIENT_ID])
        {
            users[i].timeout = SDL_GetTicks64();

            // handle player data here
            for (unsigned char j = 0; j < MAX_NET_USERS; j++)
            {
                if (players[j].id == users[i].id)
                {
                    players[j].angle = *angle;

                    if (state != GNET_DROP)
                    {
                        players[j].x = buffer[0];
                        players[j].y = buffer[1];
                    }
                    
                    players[j].state = pack->data[NET_CLIENT_USERSTATUS];
                    break;
                }
            }
            break;
        }
    }
}

void hostSendConnect(UDPpacket *p, IPaddress ip, unsigned char nplrs)
{
    p->address.host = ip.host;
    p->address.port = ip.port;

    p->data[NET_HOST_STATUS] = N_CONNECT;
    p->data[NET_HOST_NUSERS] = nplrs;

    p->len = NET_HOST_SEND_SIZE;
}

void hostSendDisconnect(UDPpacket *p, IPaddress ip, unsigned char nplrs)
{
    p->address.host = ip.host;
    p->address.port = ip.port;

    p->data[NET_HOST_STATUS] = N_DISCONNECT;
    p->data[NET_HOST_NUSERS] = nplrs;

    p->len = NET_HOST_SEND_SIZE;
}

void hostSendNewId(UDPpacket *p, IPaddress ip, unsigned char nplrs, int id)
{
    p->address.host = ip.host;
    p->address.port = ip.port;

    p->data[NET_HOST_STATUS] = N_NEWID;
    p->data[NET_HOST_NUSERS] = nplrs;
    p->data[NET_HOST_USERID] = id;

    p->len = NET_HOST_SEND_SIZE;
}

void hostSendPlay(UDPpacket *p, IPaddress ip, NET net)
{
    p->address.host = ip.host;
    p->address.port = ip.port;

    p->data[NET_HOST_STATUS] = N_PLAY;
    p->data[NET_HOST_NUSERS] = net.numplayers;
    p->data[NET_HOST_STATE] = net.play_state;
    
    memcpy((unsigned char *)&p->data[NET_HOST_GK1], &net.goalkeepers.gk1_status, 2);

    p->data[NET_HOST_PUCKID] = net.puck.id;
    memcpy((short *)&p->data[NET_HOST_PUCKDATA], &net.puck.x, 4);

    memcpy(
        (unsigned char *)&p->data[NET_HOST_USERANGLE], 
        net.players_net, NET_BUFFER_PLAYER * net.numplayers);

    p->len = (NET_BUFFER_PLAYER * net.numplayers) + NET_BUFFER_PRESET;
}
