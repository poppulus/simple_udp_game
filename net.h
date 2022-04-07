#include <SDL2/SDL_net.h>

#define TICKRATE 16

#define HOST_PACKET 0
#define PEER_PACKET 1

#define HOST_ID 1

#define MAX_NET_USERS 2
#define NET_BUFFER_PRESET 10
#define NET_BUFFER_SIZE 64
#define NET_BUFFER_PLAYER 12

#define NET_HOST_SEND_SIZE 22
#define NET_CLIENT_SEND_SIZE 12

enum NET_STATUS
{
    N_CONNECT,
    N_DISCONNECT,
    N_NEWID,
    N_PLAY
};
/*
enum NET_DATA
{
    NET_STATUS,
    NET_ID,
    NET_NUSERS,
    NET_GK1,
    NET_GK2,
    NET_PUCKID,
    NET_PUCKDATA,
    NET_USERANGLE = 10,
    NET_USERPOS = 14,
    NET_USERID = 18,
    NET_USERSTATUS
};
*/
enum NET_HOST_DATA
{
    NET_HOST_STATUS,
    NET_HOST_ID,
    NET_HOST_NUSERS,
    NET_HOST_GK1,
    NET_HOST_GK2,
    NET_HOST_PUCKID,
    NET_HOST_PUCKDATA,
    NET_HOST_USERANGLE = 10,
    NET_HOST_USERPOS = 14,
    NET_HOST_USERID = 18,
    NET_HOST_USERSTATUS
};

enum NET_CLIENT_DATA
{
    NET_CLIENT_STATUS,
    NET_CLIENT_ID,
    NET_CLIENT_USERANGLE,
    NET_CLIENT_USERPOS = 6,
    NET_CLIENT_USERID = 10,
    NET_CLIENT_USERSTATUS
};

enum P_NET_STATE
{
    PNET_NONE,
    PNET_GRAB,
    PNET_SHOOT
};

typedef struct GK_Net
{
    unsigned char gk1_status, gk2_status;
} GK_NET;

typedef struct Puck_Net
{
    unsigned char id;
    short x, y;
} PUCK_NET;

typedef struct Player_Net
{
    float angle;
    short x, y;
    unsigned char id, state;
} P_NET;

typedef struct Local_User
{
    unsigned char id, status;
} User;

typedef struct _UDPuser
{
    unsigned char id;
    char status;
    unsigned long timeout;
    IPaddress address;
} UDPuser;

// what even is this
typedef struct _UDPconnection
{
    UDPsocket sd;
    UDPpacket *pks[2];
    IPaddress hostaddr;
} UDPconnection;

typedef struct NETWORKING
{
    SDL_Thread *thread;
    UDPconnection connection;
    UDPuser *users;
    UDPuser localuser;
    P_NET *players_net;
    P_NET *localplayer;
    GK_NET goalkeepers;
    PUCK_NET puck;
    unsigned char numusers, numplayers;
    int lost:1, tquit:1, join:1, left:1;
} NET;

void closeNet(NET *net);
int startNetHost(NET *net, int port);
int startNetClient(NET *net, const char *string);

void initNet(NET *net);
int initNetHost(UDPsocket *sd, UDPpacket **pks, unsigned short port);
int initNetClient(NET *net, const char *string);

int host_thread(void *ptr);
int client_thread(void *ptr);

void printPacketInfo(UDPpacket *pkt);

void addUser(UDPuser *user, char id, int host, unsigned short port);
void removeUser(UDPuser *user);

void addPlayerNet(P_NET *p, int id, int grab);
void removePlayerNet(P_NET *p);

void resetUser(UDPuser *u);

void setupClientConnect(UDPpacket *p, IPaddress ip);
void setupClientNewId(UDPpacket *p, char id, IPaddress ip);
void setupClientDisconnect(UDPpacket *p, char id, IPaddress ip);
void setupClientPlay(UDPpacket *p, IPaddress ip, char id, P_NET *localplayer);

void clientHandleConnect();
void clientHandleDisconnect();
void clientHandleNewId();
void clientHandlePlay(P_NET *players, UDPpacket *packet, unsigned char *numplayers);

void clientHandleNewUser(P_NET *plrs, unsigned char *buf, int pkg_len, unsigned char *numplayers);
void clientHandleLostUser(P_NET *plrs, unsigned char *buf, int pkg_len, unsigned char *numplayers);

void clientSendConnect();
void clientSendDisconnect();
void clientSendNewId();
void clientSendPlay();

void hostHandleConnect(UDPuser *users, UDPpacket **pks, unsigned char *numusers, int *newid);

void hostHandleDisconnect(
    UDPuser *users, 
    unsigned char peerid, 
    P_NET *players, 
    unsigned char *numusers, 
    unsigned char *numplayers
);

void hostHandleNewId(
    UDPuser *users, 
    P_NET *players, 
    unsigned char peerid, 
    unsigned char *numplayers
);

void hostHandlePlay(UDPuser *users, P_NET *players, UDPpacket *pack);

void hostSendConnect(UDPpacket *p, IPaddress ip, unsigned char nplrs);
void hostSendDisconnect(UDPpacket *p, IPaddress ip, unsigned char nplrs);
void hostSendNewId(UDPpacket *p, IPaddress ip, unsigned char nplrs, int id);
void hostSendPlay(UDPpacket *p, IPaddress ip, unsigned char nplrs, P_NET *plrs, PUCK_NET puck, GK_NET gk);
