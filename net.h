#include <SDL2/SDL_net.h>

#define TICKRATE 16

#define HOST_PACKET 0
#define PEER_PACKET 1

#define HOST_ID 1

#define MAX_NET_USERS 2
#define NET_BUFFER_PRESET 7
#define NET_BUFFER_SIZE 64

enum NET_STATUS
{
    N_CONNECT,
    N_DISCONNECT,
    N_NEWID,
    N_PLAY
};

enum NET_DATA
{
    NET_STATUS,
    NET_ID,
    NET_NUSERS,
    NET_PUCKDATA,
    NET_USERID = 7,
    NET_USERSTATUS,
    NET_USERPOS
};

enum P_NET_STATE
{
    PNET_NONE,
    PNET_GRAB,
    PNET_SHOOT
};

typedef struct Puck_Net
{
    short x, y;
} PUCK_NET;

typedef struct Player_Net
{
    unsigned char id, state;
    short x, y;
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
    PUCK_NET puck;
    unsigned char numusers, numplayers;
    int lost:1, pquit:1, join:1, left:1, pgrab:1;
} NET;

void closeNet(NET *net);
int startNetHost(NET *net, int port);
int startNetClient(NET *net, const char *string);

void initNet(NET *net);
int initNetHost(UDPsocket *sd, UDPpacket **pks, unsigned short port);
int initNetClient(NET *net, const char *string);

int host_thread(void *ptr);
int client_thread(void *ptr);

UDPconnection *createNewConnection(
    UDPpacket *pks[], 
    UDPsocket sd, 
    UDPuser *usr, 
    P_NET plrs[], 
    const char *type
);

void addUser(UDPuser *user, char id, int host, unsigned short port);
void removeUser(UDPuser *user);

void addPlayerNet(P_NET *p, int id, int grab);
void removePlayerNet(P_NET *p);

void resetUser(UDPuser *u);

void setPackageAddress(IPaddress *send_addr, IPaddress recv_addr);

void setupClientConnect(UDPpacket *p, IPaddress ip);
void setupClientNewId(UDPpacket *p, char id, IPaddress ip);
void setupClientDisconnect(UDPpacket *p, char id, IPaddress ip);
void setupClientPlay(UDPpacket *p, char id, IPaddress ip);

int connectClient(UDPconnection *c, UDPuser *user);

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
void hostSendPlay(UDPpacket *p, IPaddress ip, unsigned char nplrs, P_NET *plrs, PUCK_NET puck);
