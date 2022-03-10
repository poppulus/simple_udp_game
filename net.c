#include "net.h"

void closeNet(NET *net)
{
    net->pquit = 1;
    net->lost = 1;
    printf("NET: wait on thread if any\n");
    // wait on thread and free
    if (net->thread != NULL) 
    {
        SDL_WaitThread(net->thread, NULL);
        net->thread = NULL;
        printf("THREAD: kill\n");
    }
    net->pquit = 0;
    net->lost = 0;
    printf("NET: close socket\n");
    // free socket
    SDLNet_UDP_Close(net->connection.sd);
    net->connection.sd = NULL;
    printf("NET: free udp packets\n");
    unsigned char i;
    // free packet pointers
    for (i = 0; i < 2; i++) 
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
    printf("NET: reset local user\n");
    net->localplayer = NULL;
    net->localuser.address.host = 0;
    net->localuser.address.port = 0;
    net->localuser.status = 0;
    net->localuser.timeout = 0;
    net->localuser.id = 0;
    printf("NET: reset other values\n");
    net->numplayers = 0;
    net->numusers = 0;
    net->join = 0;
    net->left = 0;
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

        net->players_net[0].id = 1;
        net->numplayers = 1;

        net->localplayer = &net->players_net[0];

        if ((net->thread = SDL_CreateThread(host_thread, "host", net)) == NULL)
        {
            printf("ERROR: SDL_CreateThread, %s\n", SDL_GetError());
            net->pquit = 1;
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
            net->pquit = 1;
            net->lost = 1;
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

    net->lost = 0;
    net->pquit = 0;
    net->join = 0;
    net->left = 0;

    net->thread = NULL;
    net->users = NULL;
    net->players_net = NULL;
    net->localplayer = NULL;

    net->localuser.id = 0;
    net->localuser.address.host = 0;
    net->localuser.address.port = 0;
    net->localuser.status = 0;
    net->localuser.timeout = 0;

    net->puck.x = 0;
    net->puck.y = 0;

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

    if (!(pks[HOST_PACKET] = SDLNet_AllocPacket(64))) 
    { 
        fprintf(stderr, "SDLNet_AllocPacket: %s\n", SDLNet_GetError()); 
        return 0;
    }

    if (!(pks[PEER_PACKET] = SDLNet_AllocPacket(64))) 
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
            n++;
            port = atoi(&string[i + 1]);
            host = calloc(i, 1);
            memcpy(host, string, i);
            break;
        }
    }

    if (n > 1 || n == 0)
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

    if (!(net->connection.pks[PEER_PACKET] = SDLNet_AllocPacket(64))) 
    { 
        fprintf(stderr, "SDLNet_AllocPacket: %s\n", SDLNet_GetError()); 
        return 0;
    }

    if (!(net->connection.pks[HOST_PACKET] = SDLNet_AllocPacket(64))) 
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

    printf("plrs %d usrs %d\n", net->numplayers, net->numusers);

    while (!quit && !net->pquit)
    {
        timer = SDL_GetTicks64();

        if ((recv = SDLNet_UDP_Recv(net->connection.sd, net->connection.pks[PEER_PACKET])) > 0) 
        {
            /*
            printf("UDP Packet incoming\n"); 
            printf("\tChan: %d\n", net->connection.pks[PEER_PACKET]->channel); 
            printf("\tData STATUS: %d\n", net->connection.pks[PEER_PACKET]->data[NET_STATUS]); 
            printf("\tData ID: %d\n", net->connection.pks[PEER_PACKET]->data[NET_ID]); 
            printf("\tData STRING: %s\n", (char *)&net->connection.pks[PEER_PACKET]->data); 
            printf("\tLen: %d\n", net->connection.pks[PEER_PACKET]->len); 
            printf("\tMaxlen: %d\n", net->connection.pks[PEER_PACKET]->maxlen); 
            printf("\tStatus: %d\n", net->connection.pks[PEER_PACKET]->status); 
            printf(
                "\tAddress: %x %x\n", 
                net->connection.pks[PEER_PACKET]->address.host, 
                net->connection.pks[PEER_PACKET]->address.port);
            */
            
            switch (net->connection.pks[PEER_PACKET]->data[NET_STATUS])
            {
                case N_CONNECT:
                    //printf("RECEIVE: connect\n");
                    hostHandleConnect(
                        net->users, 
                        net->connection.pks, 
                        &net->numusers, 
                        &player_id);
                break;
                case N_DISCONNECT:
                    //printf("RECEIVE: disconnecting, id %d\n", net->connection.pks[PEER_PACKET]->data[NET_ID]);
                    hostHandleDisconnect(
                        net->users, 
                        net->connection.pks[PEER_PACKET]->data[NET_ID], 
                        net->players_net, 
                        &net->numusers, 
                        &net->numplayers);
                break;
                case N_NEWID:
                    //printf("RECEIVE: new id, user %d\n", net->connection.pks[PEER_PACKET]->data[NET_ID]);
                    hostHandleNewId(
                        net->users, 
                        net->players_net, 
                        net->connection.pks[PEER_PACKET]->data[NET_ID], 
                        &net->numplayers);
                break;
                case N_PLAY:
                    //printf("RECEIVE: play\n");
                    hostHandlePlay(
                        net->users, 
                        net->players_net, 
                        net->connection.pks[PEER_PACKET]);
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
                            net->users[i].address,
                            net->numplayers,
                            net->players_net,
                            net->puck);

                        send = SDLNet_UDP_Send(net->connection.sd, -1, net->connection.pks[HOST_PACKET]);
                    break;
                }
            }
        }

        if (recv == -1)
        {
            printf("NET: error on receive\n");
            net->pquit = 1;
        }
        else if (!send)
        {
            printf("NET: error on send\n");
            net->pquit = 1;
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

void addPlayerNet(P_NET *p, int id)
{
    p->id = id;
    p->x = 0;
    p->y = 0;
}

void removePlayerNet(P_NET *p)
{
    p->id = 0;
    p->x = 0;
    p->y = 0;
}

int client_thread(void *ptr)
{
    NET *net = ptr;

    printf("THREAD: start\n");

    int delta = 0, response = 0, send = -1, pkg_len = 0;
    unsigned long timer = 0, response_timer = SDL_GetTicks64();
    long timeout = 0;
    int *buffer = NULL;

    while (!net->lost && !net->pquit)
    {
        timer = SDL_GetTicks64();

        if ((response = SDLNet_UDP_Recv(net->connection.sd, net->connection.pks[HOST_PACKET])) > 0)
        {
            response_timer = SDL_GetTicks64();
            /*
            printf("UDP Packet incoming\n"); 
            printf("\tChan: %d\n", net->connection.pks[HOST_PACKET]->channel); 
            printf("\tData STATUS: %d\n", net->connection.pks[HOST_PACKET]->data[NET_STATUS]); 
            printf("\tData ID: %d\n", net->connection.pks[HOST_PACKET]->data[NET_ID]); 
            printf("\tData STRING: %s\n", (char *)&net->connection.pks[HOST_PACKET]->data[NET_USERDATA]);
            printf("\tLen: %d\n", net->connection.pks[HOST_PACKET]->len); 
            printf("\tMaxlen: %d\n", net->connection.pks[HOST_PACKET]->maxlen); 
            printf("\tStatus: %d\n", net->connection.pks[HOST_PACKET]->status); 
            printf(
                "\tAddress: %x %x\n",
                 net->connection.pks[HOST_PACKET]->address.host, 
                 net->connection.pks[HOST_PACKET]->address.port);
            */
            switch (net->connection.pks[HOST_PACKET]->data[NET_STATUS])
            {
                case N_CONNECT:
                break;
                case N_DISCONNECT:
                    //printf("RECEIVE: disconnect\n");
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
                    //printf("RECEIVE: new id\n");
                    if (net->localuser.status != N_NEWID)
                    {
                        net->localuser.id = net->connection.pks[HOST_PACKET]->data[NET_USERDATA];
                        net->localuser.status = N_NEWID;
                    }
                break;
                case N_PLAY:
                    //printf("RECEIVE: play\n");
                    if (net->localuser.status != N_PLAY) 
                    {
                        net->localuser.status = N_PLAY;

                        setupClientPlay(
                            net->connection.pks[PEER_PACKET], 
                            net->localuser.id, 
                            net->connection.pks[HOST_PACKET]->address);
                    }
                    else 
                    {
                        // handle incoming data 

                        buffer = (int *)&net->connection.pks[HOST_PACKET]->data[NET_USERDATA];
                        pkg_len = (net->connection.pks[HOST_PACKET]->len - NET_BUFFER_PRESET) >> 2;

                        net->puck.x = (int)net->connection.pks[HOST_PACKET]->data[NET_PUCKDATA];
                        net->puck.y = (int)net->connection.pks[HOST_PACKET]->data[NET_PUCKDATA + 4];

                        if (net->connection.pks[HOST_PACKET]->data[NET_NUSERS] > net->numplayers)
                        {
                            clientHandleNewUser(net->players_net, buffer, pkg_len, &net->numplayers);
                            net->join = 1;
                        }
                        else if (net->connection.pks[HOST_PACKET]->data[NET_NUSERS] < net->numplayers)
                        {
                            clientHandleLostUser(net->players_net, buffer, pkg_len, &net->numplayers);
                            net->left = 1;
                        }

                        if (net->localplayer == NULL)
                        {
                            printf("NET: local user id %d\n", net->localuser.id);

                            for (unsigned char i = 0; i < net->numplayers; i++)
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

                        for (unsigned char i = 0; i < net->numplayers; i++)
                        {
                            if (net->players_net[i].id == buffer[i * 3] 
                            && net->players_net[i].id != net->localuser.id)
                            {
                                net->players_net[i].x = buffer[(i * 3) + 1];
                                net->players_net[i].y = buffer[(i * 3) + 2];
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

                printf("NET: pkg len:%d\n", net->connection.pks[PEER_PACKET]->len);

                send = (
                    SDLNet_UDP_Send(
                        net->connection.sd, -1, net->connection.pks[PEER_PACKET]));
            break;
            case N_DISCONNECT:
                setupClientDisconnect(
                    net->connection.pks[PEER_PACKET], 
                    net->localuser.id, 
                    net->connection.hostaddr);

                send = (SDLNet_UDP_Send(net->connection.sd, -1, net->connection.pks[PEER_PACKET]));
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
                setPackageAddress(
                    &net->connection.pks[PEER_PACKET]->address, 
                    net->connection.pks[HOST_PACKET]->address);

                net->connection.pks[PEER_PACKET]->data[NET_STATUS] = N_PLAY;
                net->connection.pks[PEER_PACKET]->data[NET_ID] = net->localuser.id;

                if (net->localplayer != NULL)
                {
                    memcpy(
                        (int*)&net->connection.pks[PEER_PACKET]->data[NET_USERDATA], 
                        net->localplayer, 
                        sizeof(P_NET));
                }

                net->connection.pks[PEER_PACKET]->len = sizeof(P_NET) + NET_BUFFER_PRESET;

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

void setPackageAddress(IPaddress *send_addr, IPaddress recv_addr)
{
    send_addr->host = recv_addr.host;
    send_addr->port = recv_addr.port;
}

void setupClientConnect(UDPpacket *p, IPaddress ip)
{
    p->address.host = ip.host;
    p->address.port = ip.port;

    p->data[NET_STATUS] = N_CONNECT;
    p->data[NET_ID] = 0;

    p->len = strlen((char *)&p->data[NET_USERDATA]) + NET_BUFFER_PRESET;
}

void setupClientNewId(UDPpacket *p, char id, IPaddress ip)
{
    p->address.host = ip.host;
    p->address.port = ip.port;

    p->data[NET_STATUS] = N_NEWID;
    p->data[NET_ID] = id;

    p->len = strlen((char *)&p->data[NET_USERDATA]) + NET_BUFFER_PRESET;
}

void setupClientDisconnect(UDPpacket *p, char id, IPaddress ip)
{
    p->address.host = ip.host;
    p->address.port = ip.port;

    p->data[NET_STATUS] = N_DISCONNECT;
    p->data[NET_ID] = id;

    p->len = strlen((char *)&p->data[NET_USERDATA]) + NET_BUFFER_PRESET;
}

void setupClientPlay(UDPpacket *p, char id, IPaddress ip)
{
    p->address.host = ip.host;
    p->address.port = ip.port;

    p->data[NET_STATUS] = N_PLAY;
    p->data[NET_ID] = id;

    p->len = strlen((char *)&p->data[NET_USERDATA]) + NET_BUFFER_PRESET;
}

/*
UDPconnection *createNewConnection(UDPpacket *pks[], UDPsocket sd, UDPuser *usr, P_NET plrs[], const char *type)
{
    static UDPconnection c;

    c.pks = pks;
    c.sd = &sd;
    c.players = plrs;
    c.lost = 0;
    c.numplayers = 1;

    if (type == "client") 
    {
        c.type = "client";
        c.localuser = usr;
    }
    else if (type == "host")
    {
        c.type = "host";
        c.users = usr;
        c.numusers = 1;
    }

    return &c;
}
*/

int connectClient(UDPconnection *c, UDPuser *user)
{
    Uint64 timer = 0, res_timer = SDL_GetTicks64();
    int status = 0, response = 0;
    long timeout = 0;

    while (!status)
    {
        timer = SDL_GetTicks64();

        // ping server
        if (SDLNet_UDP_Send(c->sd, -1, c->pks[PEER_PACKET])) 
            printf("SEND: ping\n"); 
        else status = -1;
        
        // wait on response
        if ((response = SDLNet_UDP_Recv(c->sd, c->pks[HOST_PACKET])) > 0)
        {
            res_timer = SDL_GetTicks64();
            
            printf("UDP Packet incoming\n"); 
            printf("\tChan: %d\n", c->pks[HOST_PACKET]->channel); 
            printf("\tData STATUS: %d\n", c->pks[HOST_PACKET]->data[NET_STATUS]); 
            printf("\tData ID: %d\n", c->pks[HOST_PACKET]->data[NET_ID]); 
            printf("\tData STRING: %s\n", (char *)&c->pks[HOST_PACKET]->data[NET_USERDATA]);
            printf("\tLen: %d\n", c->pks[HOST_PACKET]->len); 
            printf("\tMaxlen: %d\n", c->pks[HOST_PACKET]->maxlen); 
            printf("\tStatus: %d\n", c->pks[HOST_PACKET]->status); 
            printf("\tAddress: %x %x\n", c->pks[HOST_PACKET]->address.host, c->pks[HOST_PACKET]->address.port);
            
            switch (c->pks[HOST_PACKET]->data[NET_STATUS])
            {
                case N_DISCONNECT:
                    status = -1;
                    printf("RECEIVE: disconnect\n");
                break;
                case N_NEWID:
                    user->id = c->pks[HOST_PACKET]->data[NET_USERDATA];
                    status = 1;
                    printf("RECEIVE: new id: %d\n", c->pks[HOST_PACKET]->data[NET_USERDATA]);
                break;
            }
        }

        if (response < 0) status = -1;
        else if (!response)
        {
            timeout = timer - res_timer;

            if (timeout >= 3000)
            {
                printf("RECEIVE: host timeout\n");
                status = -1;
            }
        }

        SDL_Delay(100);
    }

    return status;
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
    if (packet->data[NET_NUSERS] < (*numplayers))
    {
        /*
        unsigned char n = 0;

        for (unsigned char i = 0; i < (*numplayers); i++)
        {
            for (unsigned char j = 0; j < packet->data[NET_NUSERS]; j++)
            {
                if (players[i].id == (int)packet->data[NET_USERDATA + (j * 3)])
                    n = 1;
            }
            // if player id is NOT in data, remove from list
            if (!n)
            {
                printf("NET: removed user %d\n", players[i].id);
                removePlayer(&players[i]);
                (*numplayers)--;
            }

            n = 0;
        }
        */

        printf("NET: lost player(s)\n");
    }
    else if (packet->data[NET_NUSERS] > (*numplayers))
    {
        /*
        unsigned char n = 0;

        for (unsigned char j = 0; j < packet->data[NET_NUSERS]; j++)
        {
            for (unsigned char i = 0; i < (*numplayers); i++)
            {
                if ((int)packet->data[NET_USERDATA + (j * 3)] == players[i].id)
                    n = 1;
            }
            // if player id is NOT in data, add to list
            if (!n)
            {
                for (unsigned char p = 0; p < (*numplayers); p++)
                {
                    if (!players[p].id)
                    {
                        printf("NET: added user %d\n", (int)packet->data[NET_USERDATA + (j * 3)]);
                        addPlayer(&players[p], (int)packet->data[NET_USERDATA + (j * 3)]);
                        (*numplayers)++;
                    }
                }
            }

            n = 0;
        }
        */

        for (unsigned char j = 0; j < packet->data[NET_NUSERS]; j++)
        {
            printf("USER: id:%d\n", (int)packet->data[NET_USERDATA + (j * 3)]);
        }

        printf("NET: new player(s)\n");
    }

    for (unsigned char i = 0; i < (*numplayers); i++)
    {
        if (players[i].id == (int)packet->data[NET_USERDATA + (i * 3)])
        {
            players[i].x = (int)packet->data[NET_USERDATA + (i * 3) + 1];
            players[i].y = (int)packet->data[NET_USERDATA + (i * 3) + 2];
        }
    }
}

void clientHandleNewUser(P_NET *plrs, int *buf, int pkg_len, unsigned char *numplayers)
{
    unsigned char n = 0;
    
    for (unsigned char i = 0; i < pkg_len; i += 3)
    {
        for (unsigned j = 0; j < (*numplayers); j++)
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
            (*numplayers)++;
            for (unsigned p = 0; p < (*numplayers); p++)
            {
                if (!plrs[p].id)
                {
                    addPlayerNet(&plrs[p], buf[i]);
                    break;
                }
            }
        }
        n = 0;
    }
}

void clientHandleLostUser(P_NET *plrs, int *buf, int pkg_len, unsigned char *numplayers)
{
    unsigned char n = 0;
    
    for (unsigned char i = 0; i < (*numplayers); i++)
    {
        for (unsigned j = 0; j < pkg_len; j += 3)
        {
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
        }
        if (!n)
        {
            printf("USER: found lost id:%d\n", plrs[i].id);
            removePlayerNet(&plrs[i]);
            (*numplayers)--;
        }
        n = 0;
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
                if ((*numplayers) < MAX_NET_USERS)
                {
                    users[i].timeout = SDL_GetTicks64();
                    users[i].status = N_PLAY;

                    printf("USER: user %d set status to play\n", users[i].id);

                    addPlayerNet(&players[*numplayers], users[i].id);

                    (*numplayers)++;

                    printf("USER: add user %d to player list\n", users[i].id);
                }
            }
            break;
        }
    }
}

void hostHandlePlay(UDPuser *users, P_NET *players, UDPpacket *pack)
{
    int *buffer = (int *)&pack->data[NET_USERDATA];

    for (unsigned char i = 0; i < MAX_NET_USERS; i++)
    {
        if (users[i].id == pack->data[NET_ID] && pack->data[NET_ID] > 0)
        {
            users[i].timeout = SDL_GetTicks64();

            // handle player data here
            for (unsigned char j = 0; j < MAX_NET_USERS; j++)
            {
                if (players[j].id == users[i].id)
                {
                    players[j].x = buffer[1];
                    players[j].y = buffer[2];

                    //printf("USER: new x:%d y:%d\n", players[j].x, players[j].y);
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

    p->data[NET_STATUS] = N_CONNECT;
    p->data[NET_NUSERS] = nplrs;

    p->len = strlen((char *)&p->data[NET_USERDATA]) + NET_BUFFER_PRESET;
}

void hostSendDisconnect(UDPpacket *p, IPaddress ip, unsigned char nplrs)
{
    p->address.host = ip.host;
    p->address.port = ip.port;

    p->data[NET_STATUS] = N_DISCONNECT;
    p->data[NET_NUSERS] = nplrs;

    p->len = strlen((char *)&p->data[NET_USERDATA]) + NET_BUFFER_PRESET;
}

void hostSendNewId(UDPpacket *p, IPaddress ip, unsigned char nplrs, int id)
{
    p->address.host = ip.host;
    p->address.port = ip.port;

    p->data[NET_STATUS] = N_NEWID;
    p->data[NET_NUSERS] = nplrs;
    p->data[NET_USERDATA] = id;

    p->len = strlen((char *)&p->data[NET_USERDATA]) + NET_BUFFER_PRESET;
}

void hostSendPlay(UDPpacket *p, IPaddress ip, unsigned char nplrs, P_NET *plrs, PUCK_NET puck)
{
    p->address.host = ip.host;
    p->address.port = ip.port;

    p->data[NET_STATUS] = N_PLAY;
    p->data[NET_NUSERS] = nplrs;

    memcpy((int *)&p->data[NET_PUCKDATA], &puck, 8);

    memcpy(
        (int *)&p->data[NET_USERDATA], 
        plrs, 
        sizeof(P_NET) * nplrs);

    p->len = (sizeof(P_NET) * nplrs) + NET_BUFFER_PRESET;
}
