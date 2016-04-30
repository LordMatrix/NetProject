//Select UDP
#pragma comment(lib, "ws2_32.lib")

#include "stdio.h"
#include "winsock2.h"

#include "structs.h"


//Map where the players live
int g_board[8][8];

Player* g_players[10];
int g_num_clients=0;
int g_max_clients=4;

int g_map_width;
int g_map_height;


void move(int player_id, Direction direction) {
  float speed = 0.1f;
  
  //Update positions
  switch (direction) {
    case UP:
      if (g_players[player_id]->position.y > 0)
        g_players[player_id]->position.y -= speed;
      break;
    case RIGHT:
      if (g_players[player_id]->position.x < g_win_width)
        g_players[player_id]->position.x += speed;
      break;
    case DOWN:
      if (g_players[player_id]->position.y < g_win_height)
        g_players[player_id]->position.y += speed;
      break;
    case LEFT:
      if (g_players[player_id]->position.x > 0)
        g_players[player_id]->position.x -= speed;
      break;
    case NONE:
    default:
      break;
  }
}


bool createPlayer() {
  if (g_num_clients < g_max_clients) {
    Player* player = new Player();
    player->id = g_num_clients;
    player->position.x = 0;
    player->position.y = 0;
    
    Color color;
    switch (g_num_clients) {
      case 0:
        color = {255,0,0,255};
        break;
      case 1:
        color = {0,255,0,255};
        break;
      case 2:
        color = {0,0,255,255};
        break;
      case 3:
        color = {255,255,0,255};
        break;
      default:
        color = {0,0,0,255};
        break;
    }
    
    player->color = color;
    
    g_players[g_num_clients] = player;

    g_num_clients++;
    return true;
  } else {
    return false;
  }
}


void removePlayer(int id) {
  printf("Client disconnected: %d\n", id);
  delete g_players[id];
  g_players[id] = nullptr;
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
  time.tv_sec=0;
  time.tv_usec= g_refresh_time;
  
  Package* pack_out = new Package();
  
  while (1) {
    memset(buffer, 0, 512);
    FD_SET(sock, &SOCK_IN);
    select(g_num_clients, &SOCK_IN, NULL, NULL, &time);
    if (FD_ISSET(sock, &SOCK_IN)) {
      if (recvfrom(sock, buffer, 512, 0, (SOCKADDR*)&ipc[g_num_clients], &size)) {
        
        Package* pack_in = new Package();
        memcpy(pack_in, buffer, sizeof(Package));
        
        switch (pack_in->id) {
          case 1:
            //Direction
            move(pack_in->movement.player_id, pack_in->movement.direction);
            break;
          case 2:
            //GameState
            break;
          case 3:
            //Player
            if (createPlayer())  {            
              //Send player info back
              pack_out->id = 3;
              pack_out->player.id = g_players[g_num_clients - 1]->id;
              printf("New player id is:  %d\n", *g_players[g_num_clients - 1]);
              sendto(sock, (char*)pack_out, sizeof(Package), 0, (SOCKADDR*)&ipc[g_num_clients - 1], sizeof(ipc[g_num_clients - 1]));
            } else {
              pack_out->id = 0;
              sendto(sock, (char*)pack_out, sizeof(Package), 0, (SOCKADDR*)&ipc[g_num_clients - 1], sizeof(ipc[g_num_clients - 1]));
              printf("Max players reached.Connection refused.\n");
            }
            break;
          case 4:
            //Disconnection
            removePlayer(pack_in->player.id);
            break;
          default:
            //ERROR
            break;
        }
        
        
        delete pack_in;
      }
    }
    
    //Send game status
    memset(pack_out, 3, sizeof(Package));
    pack_out->gamestatus.num_players = g_num_clients;
    //Copy all player data to package_out
    for (int i=0; i<g_num_clients; i++) {
      if (g_players[i] != nullptr)
        pack_out->gamestatus.players[i] = *g_players[i];
    }
    
    //Send data to all players
    for (int i=0; i<g_num_clients; i++) {
      sendto(sock, (char*)pack_out, sizeof(Package), 0, (SOCKADDR*)&ipc[i], sizeof(ipc[i]));
    }
  }
  
  return 0;
 
}