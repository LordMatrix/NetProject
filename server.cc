//Select UDP
#pragma comment(lib, "ws2_32.lib")

#include "stdio.h"
#include "winsock2.h"

#include "structs.h"


//Map where the players live
int g_board[8][8];

Player* g_players[10];
int g_num_clients=0;

int g_map_width;
int g_map_height;


void update() {
  //Update positions
  for (int i=0; i<g_num_clients; i++) {
    switch (g_players[i]->direction) {
      case UP:
        g_players[i]->y--;
        break;
      case RIGHT:
        g_players[i]->x++;
        break;
      case DOWN:
        g_players[i]->y++;
        break;
      case LEFT:
        g_players[i]->x--;
        break;
      case NONE:
      default:
        break;
    }
  }
}


void createPlayer() {
	Player* player = new Player();
	player->id = g_num_clients;
	player->x = 0;
	player->y = 0;
	
	player->color.r = 100;
	player->color.g = 100;
	player->color.b = 100;
	player->color.a = 255;
	
	g_players[g_num_clients] = player;
	g_num_clients++;
}


int main(int argc, char** argv) {

  struct timeval time;
  WSAData wsa;
  SOCKET sock;
  fd_set SOCK_IN;
  struct sockaddr_in ip, ipc[64];
  int size=sizeof(ip);
  char buffer[512];
  WSAStartup(MAKEWORD(2,0), &wsa);
  sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  ip.sin_family=AF_INET;
  ip.sin_port=htons(9999);
  ip.sin_addr.s_addr=inet_addr("127.0.0.1");
  bind(sock, (SOCKADDR*)&ip, sizeof(ip));
  FD_ZERO(&SOCK_IN);
  time.tv_sec=1;
  time.tv_usec=0;
  
  
  while (1) {
    memset(buffer, 0, 512);
    FD_SET(sock, &SOCK_IN);
    select(g_num_clients, &SOCK_IN, NULL, NULL, &time);
    if (FD_ISSET(sock, &SOCK_IN)) {
      if (recvfrom(sock, buffer, 512, 0, (SOCKADDR*)&ipc, &size)) {
        
        Package* pack_in = new Package();
        Package* pack_out = new Package();
        memcpy(pack_in, buffer, sizeof(Package));
        
        switch (pack_in->id) {
          case 1:
            //Direction
            printf("Direction is:%d\n",pack_in->movement.direction);
            break;
          case 2:
            //GameState
            break;
          case 3:
            //Player
            printf("\nNueva conexion en case 3\n");
            createPlayer();
            
            //Send player info back
            pack_out->id = 5;
            pack_out->player = *g_players[g_num_clients - 1];
            printf("New player id is:  %d", pack_out->player.id);
            sendto(sock, (char*)pack_out, sizeof(Package), 0, (SOCKADDR*)&ipc, sizeof(ipc));
            printf("\nClientes conectados:%d\n", g_num_clients);
            
            break;
          default:
            //ERROR
            break;
        }
        
        
        delete pack_in;
      } else {
        printf("It went the other way\n");
      }
    } else {
      printf("\nTimeout\n");
    }
  }
  
  return 0;
 
}