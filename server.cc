//Select UDP
#pragma comment(lib, "ws2_32.lib")

#include "stdio.h"
#include "winsock2.h"

#include "time.h"
#include "structs.h"


/****** GLOBALS *****/
Player* g_players[10];
int g_num_clients=0;
int g_max_clients=4;

int g_map_width;
int g_map_height;

Shot* g_shots[50];
int g_num_shots=0;

Hit* g_hits[50];
int g_num_hits=0;
/********************/


///Returns the index of the player collided with
int checkPlayerPlayerCollisions(Player* player) {
  int collided = -1;
  
  //check player collisions
  for (int i=0; i<g_num_clients && collided==-1; i++) {
    if (player->id != i) {
      if ( (player->position.x + g_player_size > g_players[i]->position.x && player->position.x < g_players[i]->position.x + g_player_size) 
        && (player->position.y + g_player_size > g_players[i]->position.y && player->position.y < g_players[i]->position.y + g_player_size)) {
      
        collided = i;
      }
    }
  }
  
  return collided;
}


//Returns the index of the player collided with
int checkShotPlayerCollisions(Shot* shot) {
  int collided = -1;
  
  //check player collisions
  for (int i=0; i<g_num_clients && collided==-1; i++) {
    if ( (shot->position.x + g_shot_size > g_players[i]->position.x && shot->position.x < g_players[i]->position.x + g_player_size) 
      && (shot->position.y + g_shot_size > g_players[i]->position.y && shot->position.y < g_players[i]->position.y + g_player_size)) {
    
      collided = i;
    }
  }
  
  return collided;
}


//Returns the index of the shot collided with
int checkShotShotCollisions(Shot* shot) {
  int collided = -1;
  
  //check player collisions
  for (int i=0; i<g_num_shots && collided==-1; i++) {
    if (shot != g_shots[i]) {
      if ( (shot->position.x + g_shot_size > g_shots[i]->position.x && shot->position.x < g_shots[i]->position.x + g_shot_size) 
        && (shot->position.y + g_shot_size > g_shots[i]->position.y && shot->position.y < g_shots[i]->position.y + g_shot_size)) {
      
        collided = i;
      }
    }
  }
  
  return collided;
}


///Creates a Hit object and increases the buffer count
void createHit(Point2 position) {
  Hit* hit = new Hit();
  hit->position = position;
  hit->age = 0.0f;
  
  g_hits[g_num_hits] = hit;
  g_num_hits++;
}


///Apply damage to a player
void damagePlayer(Player* player, float amount) {
  if (player->health >= 0.0f)
    player->health -= amount;
  else
    player->alive = false;
}


///Moves a player in the map, given the new position is in-bounds and empty
void move(int player_id, Direction direction) {
  Point2 prev = g_players[player_id]->position;
  float speed = g_player_speed;
  
  g_players[player_id]->direction = direction;
  
  if (g_players[player_id]->blocking)
    speed = g_player_speed/3;
    
  //Update positions
  switch (direction) {
    case UP:
      if (g_players[player_id]->position.y > 0)
        g_players[player_id]->position.y -= speed;
      break;
    case RIGHT:
      if (g_players[player_id]->position.x < kWinWidth)
        g_players[player_id]->position.x += speed;
      break;
    case DOWN:
      if (g_players[player_id]->position.y < kWinHeight)
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
  
  
  int collider = checkPlayerPlayerCollisions(g_players[player_id]);
  if (collider != -1 && g_players[collider]->alive) {
    
    //Create a hit if any of the collider players is NOT blocking
    if (!g_players[player_id]->blocking || !g_players[collider]->blocking) {
      
      //Push colliding players backwards
      g_players[collider]->position.x += (g_players[player_id]->position.x - prev.x) * g_strength;
      g_players[collider]->position.y += (g_players[player_id]->position.y - prev.y) * g_strength;
      
      g_players[player_id]->position.x -= (g_players[player_id]->position.x - prev.x) * g_strength;
      g_players[player_id]->position.y -= (g_players[player_id]->position.y - prev.y) * g_strength;
    
      createHit(g_players[collider]->position);
    }
    
    //Damage colliding players if neither of them is blocking
    if (!g_players[player_id]->blocking && !g_players[collider]->blocking) {
      damagePlayer(g_players[player_id], 1.0f);
      damagePlayer(g_players[collider], 1.0f);
    }
  }
}


///Creates a new player in the first available spot
bool createPlayer() {

  if (g_num_clients < g_max_clients) {
    Player* player = new Player();
    player->id = g_num_clients;
    player->health = g_player_health;
    
    Color color;
    switch (g_num_clients) {
      case 0:
        color = {255,0,0,255};
        player->position.x = g_player_size;
        player->position.y = g_player_size;
        break;
      case 1:
        color = {0,255,0,255};
        player->position.x = kWinWidth - g_player_size;
        player->position.y = g_player_size;
        break;
      case 2:
        color = {0,0,255,255};
        player->position.x = g_player_size;
        player->position.y = kWinHeight - g_player_size;
        break;
      case 3:
        color = {255,255,0,255};
        player->position.x = kWinWidth - g_player_size;
        player->position.y = kWinHeight - g_player_size;
        break;
      default:
        color = {0,0,0,255};
        break;
    }
    
    player->color = color;
    player->alive = true;
    player->blocking = false;
    player->last_shot_time = 0.0f;
    
    g_players[g_num_clients] = player;

    g_num_clients++;
    return true;
  } else {
    return false;
  }
}


///Creates a shot given the player direction of movement
void createShot(Player* player) {
  Shot* shot = new Shot();
  
  shot->player_id = player->id;
  shot->position = player->position;
  
  switch (player->direction) {
    case 1:
      shot->velocity = {0.0f, -1.0f};
      break;
    case 2:
      shot->velocity = {1.0f, 0.0f};
      break;
    case 3:
      shot->velocity = {0.0f, 1.0f};
      break;
    case 4:
      shot->velocity = {-1.0f, 0.0f};
      break;
    default:
      break;
  }
  
  //Adjust velocity
  shot->velocity.x *= g_shot_speed;
  shot->velocity.y *= g_shot_speed;
  
  g_shots[g_num_shots] = shot;
  g_num_shots++;
}


///Remove a shot from the buffer
void destroyShot(Shot* shot) {
  for (int i=0; i<g_num_shots; i++) {
    if (g_shots[i] == shot) {
      g_num_shots--;
      
      if (i < g_num_shots) {
        g_shots[i] = g_shots[g_num_shots];
        g_shots[g_num_shots] = nullptr;
      }
    }
  }
}


///Removes a Hit effect from the buffer
void destroyHit(Hit* hit) {
  for (int i=0; i<g_num_hits; i++) {
    if (g_hits[i] == hit) {
      g_num_hits--;
      
      if (i < g_num_hits) {
        g_hits[i] = g_hits[g_num_hits];
        g_hits[g_num_hits] = nullptr;
      }
    }
  }
}


///Moves a shot accordingly to its velocity
void moveShot(Shot* shot) {
  //Advance shot position
  shot->position.x += shot->velocity.x;
  shot->position.y += shot->velocity.y;
  
  //Check screen boundaries
  if (shot->position.y < 0 || shot->position.x > kWinWidth
      || shot->position.y > kWinHeight || shot->position.x < 0) {

    destroyShot(shot);
  }
}


///Deletes a player and removes it from the buffer
void removePlayer(int id) {
  printf("Client %d disconnected from : %s\n", id, inet_ntoa(g_players[id]->ip));
  delete g_players[id];
  g_players[id] = nullptr;
  g_num_clients--;
}


/********** SERVER CONSOLE ***********/
DWORD WINAPI console(LPVOID data) {
  char cmd[10];
  
  while (1) {
	  gets_s(&cmd[0], 10);

	  if (strcmp(cmd, "clients") == 0)
		  printf("There are %d clients connected\n", g_num_clients);
	  else if (strcmp(cmd, "players") == 0) {
		  for (int i=0; i<g_num_clients; i++) {
			  printf("%s has %fHP left\n", g_players[i]->name, g_players[i]->health);
		  }
		  printf("\n");
	  } else
		  printf("Unknown command\n");
  }
  
  return 0;
}
/*************************************/


int main(int argc, char** argv) {

  //Winsock stuff
  struct timeval utime;
  WSAData wsa;
  SOCKET sock;
  fd_set SOCK_IN;
  struct sockaddr_in ip, ipc[64];
  int size=sizeof(ip);
  char buffer[sizeof(Package)];
  WSAStartup(MAKEWORD(2,0), &wsa);
  sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  ip.sin_family=AF_INET;
  ip.sin_port=htons(9999);
  ip.sin_addr.s_addr=inet_addr("0.0.0.0");
  bind(sock, (SOCKADDR*)&ip, sizeof(ip));
  FD_ZERO(&SOCK_IN);
  utime.tv_sec=0;
  utime.tv_usec= g_refresh_time;
  
  Package* pack_out = new Package();
  
  
  //Console runs in a new thread
  DWORD ThreadID;
  HANDLE ThreadHandle;
  ThreadHandle = CreateThread(NULL,0,console,NULL,0,&ThreadID);
  
  if(ThreadHandle == NULL) {
    printf("\nError creating console thread\n");
  }
  
  
  //Main Loop
  while (1) {
    memset(buffer, 0, sizeof(Package));
    FD_SET(sock, &SOCK_IN);
    select(g_num_clients, &SOCK_IN, NULL, NULL, &utime);
    if (FD_ISSET(sock, &SOCK_IN)) {
      if (recvfrom(sock, buffer, sizeof(Package), 0, (SOCKADDR*)&ipc[g_num_clients], &size)) {
        
        Package* pack_in = new Package();
        memcpy(pack_in, buffer, sizeof(Package));
        
        switch (pack_in->id) {
          case 1:
            //Direction
            if (g_players[pack_in->movement.player_id]->alive) {
              move(pack_in->movement.player_id, pack_in->movement.direction);
              
              if (pack_in->movement.shooting && g_num_shots < g_max_shots) {
                //Create shot if the recharge time has passed
                double current_tick;
                current_tick = clock();
             
                if (current_tick - g_players[pack_in->movement.player_id]->last_shot_time > g_shot_delay) {
                  createShot(g_players[pack_in->movement.player_id]);
                  g_players[pack_in->movement.player_id]->last_shot_time = current_tick;
                  
                }
              }
              
              //Set/Unset blocking
              g_players[pack_in->movement.player_id]->blocking = pack_in->movement.blocking;
            }
            break;
          case 2:
            //GameState
            break;
          case 3:
            //Player
            if (createPlayer())  {
              printf("Player created with ID %d\n",pack_in->player.avatar);
              g_players[g_num_clients - 1]->avatar = pack_in->player.avatar;
              
              //Default avatar
              if (!pack_in->player.avatar)
                g_players[g_num_clients - 1]->avatar = 1;
              
              g_players[g_num_clients - 1]->ip = ipc[g_num_clients - 1].sin_addr;
              memcpy(g_players[g_num_clients - 1]->name, pack_in->player.name, 50);
              
              //Send player info back
              pack_out->id = 3;
              pack_out->player.id = g_players[g_num_clients - 1]->id;
              printf("New player with id %d connected from %s\n", g_players[g_num_clients - 1]->id, inet_ntoa(g_players[g_num_clients - 1]->ip));
              sendto(sock, (char*)pack_out, sizeof(Package), 0, (SOCKADDR*)&ipc[g_num_clients - 1], sizeof(ipc[g_num_clients - 1]));
            } else {
              pack_out->id = 8;
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
    memset(pack_out, 0, sizeof(Package));
    
    
    //Copy all player data to package_out
    for (int i=0; i<g_num_clients; i++) {
      if (g_players[i] != nullptr)
        pack_out->gamestatus.players[i] = *g_players[i];
    }
    
    
    //Copy all shots data to package_out
    for (int i=0; i<g_num_shots; i++) {
      if (g_shots[i] != nullptr) {
        //Update shot
        moveShot(g_shots[i]);
        int player_hit = checkShotPlayerCollisions(g_shots[i]);
        
        if (player_hit != -1) {
          if (player_hit != g_shots[i]->player_id) {
            
            //Add hit effect
            createHit(g_shots[i]->position);
            
            destroyShot(g_shots[i]);
            g_players[player_hit]->position.x += g_shots[i]->velocity.x * g_strength;
            g_players[player_hit]->position.y += g_shots[i]->velocity.y * g_strength;
            
            //Damage player is he¡s not blocking
            if (!g_players[player_hit]->blocking) {
              damagePlayer(g_players[player_hit], 10);
              
              //Add score
              if (g_players[player_hit]->alive) {
                g_players[g_shots[i]->player_id]->score += 10.0f;
              }
            }
            
          }
        } else {     
          int shot_hit = checkShotShotCollisions(g_shots[i]);
          if (shot_hit != -1) {
            //Add hit effect
            createHit(g_shots[shot_hit]->position);
            //Destroy shot
            destroyShot(g_shots[i]);
            destroyShot(g_shots[shot_hit]);
          }
        }
        pack_out->gamestatus.shots[i] = *g_shots[i];
        }
    }
    //Copy array lengths
    pack_out->gamestatus.num_players = g_num_clients;
    pack_out->gamestatus.num_shots = g_num_shots;
    pack_out->gamestatus.num_hits = g_num_hits;

    //Update hits
    for (int i=0; i<g_num_hits; i++) {
      g_hits[i]->age += 0.1f;
      
      if (g_hits[i]->age > 100.0f)
        destroyHit(g_hits[i]);
    }

    //Copy hits to package out
    for (int i=0; i<g_num_hits; i++) {
      pack_out->gamestatus.hits[i] = *g_hits[i];
    }

    //Send data to all players
    for (int i=0; i<g_num_clients; i++) {
      sendto(sock, (char*)pack_out, sizeof(Package), 0, (SOCKADDR*)&ipc[i], sizeof(ipc[i]));
    }
  }
  
  //Destroy child process
  WaitForSingleObject(ThreadHandle, INFINITE);
  return 0;
 
}