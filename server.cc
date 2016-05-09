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


bool checkCollisions(Player* player) {
  bool collided = false;
  
  //check player collisions
  for (int i=0; i<g_num_clients && ! collided; i++) {
    if (player->id != i) {
      if ( (player->position.x + g_player_size > g_players[i]->position.x && player->position.x < g_players[i]->position.x + g_player_size) 
        && (player->position.y + g_player_size > g_players[i]->position.y && player->position.y < g_players[i]->position.y + g_player_size))
      
        collided = true;
    }
  }
  
  return collided;
}


void move(int player_id, Direction direction) {
  Point2 prev = g_players[player_id]->position;
  
  //Update positions
  switch (direction) {
    case UP:
      if (g_players[player_id]->position.y > 0)
        g_players[player_id]->position.y -= g_speed;
      break;
    case RIGHT:
      if (g_players[player_id]->position.x < kWinWidth)
        g_players[player_id]->position.x += g_speed;
      break;
    case DOWN:
      if (g_players[player_id]->position.y < kWinHeight)
        g_players[player_id]->position.y += g_speed;
      break;
    case LEFT:
      if (g_players[player_id]->position.x > 0)
        g_players[player_id]->position.x -= g_speed;
      break;
    case NONE:
    default:
      break;
  }
  
  if (checkCollisions(g_players[player_id]))
    g_players[player_id]->position = prev;
}


bool createPlayer() {
  if (g_num_clients < g_max_clients) {
    Player* player = new Player();
    player->id = g_num_clients;
    player->health = 1000.0f;
    
    Color color;
    switch (g_num_clients) {
      case 0:
        color = {255,0,0,255};
        player->position.x = g_player_size;
        player->position.y = g_player_size;
        memcpy(player->name,"DOGE", 15);
        break;
      case 1:
        color = {0,255,0,255};
        player->position.x = kWinWidth - g_player_size;
        player->position.y = g_player_size;
        memcpy(player->name,"LONGCAT", 15);
        break;
      case 2:
        color = {0,0,255,255};
        player->position.x = g_player_size;
        player->position.y = kWinHeight - g_player_size;
        memcpy(player->name,"SMIUCH", 15);
        break;
      case 3:
        color = {255,255,0,255};
        player->position.x = kWinWidth - g_player_size;
        player->position.y = kWinHeight - g_player_size;
        memcpy(player->name,"LITTLEMARMOT", 15);
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