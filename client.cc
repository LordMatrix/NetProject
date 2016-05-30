//Select UDP
#pragma comment(lib, "ws2_32.lib")
#define DrawTextA DrawText

#include "stdio.h"
#include "winsock2.h"

#include <string>

#include "ESAT/window.h"
#include "ESAT/draw.h"
#include "ESAT/input.h"
#include "ESAT/sprite.h"

#include "structs.h"


void drawCube(int size, Color color, Point2 position) {
  float x = position.x;
  float y = position.y;
  
  //Create cube points
  float pathpoints[10] = {x,y, x+size,y, x+size,y+size, x,y+size, x,y};
  
  ESAT::DrawSetStrokeColor(color.r,color.g,color.b,color.a);
  ESAT::DrawSetFillColor(color.r,color.g,color.b,color.a);
  ESAT::DrawSolidPath(pathpoints, 5);
}


void drawLifeBar(float health, Color color, Point2 position) {
  float x = position.x;
  float y = position.y;
  int size = 25;
  float width = size*6;
  
  //Create border points
  float border_points[10] = {x,y, x+width,y, x+width,y+size, x,y+size, x,y};
  ESAT::DrawSetStrokeColor(0,0,0,255);
  ESAT::DrawSetFillColor(0,0,0,0);
  ESAT::DrawSolidPath(border_points, 5);
  
  //Create healthbar points
  width = (width/g_player_health) * health;
  float health_points[10] = {x,y, x+width,y, x+width,y+size, x,y+size, x,y};
  ESAT::DrawSetStrokeColor(0,0,0,0);
  ESAT::DrawSetFillColor(color.r,color.g,color.b,color.a);
  ESAT::DrawSolidPath(health_points, 5);
}


void drawHit(Hit hit) {
  float x,y;
  int num_particles = 20;
  float angle = (360/num_particles) * 0.0174533f;

  for (int i=0; i<num_particles; i++) {
    x = hit.position.x + cos(angle*i) * hit.age/2;
    y = hit.position.y + sin(angle*i) * hit.age/2;

    ESAT::DrawSetFillColor(0,0,0,255);
    ESAT::DrawSetStrokeColor(0,0,0,255);
    ESAT::DrawLine(x,y,x+2,y+2);
  }
}


void game() {
  
}


void setText() {
  ESAT::DrawSetStrokeColor(0,0,0);
  ESAT::DrawSetFillColor(0,0,0);
  ESAT::DrawSetTextSize(30.0f);
}


int ESAT::main(int argc, char** argv) {
  
  bool game_started = false;
  struct timeval time;
  WSAData wsa;
  SOCKET sock, socks;
  fd_set SOCK_IN;
  struct sockaddr_in ip, ips;
  char* buffer = (char*)malloc(sizeof(Package));
  int size=sizeof(ip);
  WSAStartup(MAKEWORD(2,0), &wsa);
  sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  ip.sin_family=AF_INET;
  ip.sin_port=htons(8888);
  ip.sin_addr.s_addr=inet_addr("127.0.0.1");
	bind(sock, (SOCKADDR*)&ip, sizeof(ip));
  FD_ZERO(&SOCK_IN);
  time.tv_sec=0;
  time.tv_usec=g_refresh_time;
  
  socks = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  ips.sin_family=AF_INET;
  ips.sin_port=htons(9999);
  ips.sin_addr.s_addr=inet_addr("127.0.0.1");
  
  ESAT::WindowInit(kWinWidth, kWinHeight);
  ESAT::DrawSetTextFont("assets/font/medieval.ttf");
  
  Player* player = new Player();
  Package* pack = new Package();
  Package* pack_in = new Package();
  
  bool send = false;
  bool exit = false;
  bool disconnect_quietly = false;
  
  std::string name = "";
  ESAT::SpriteHandle avatars[4];
  ESAT::SpriteHandle skull;
  ESAT::SpriteHandle block;
  ESAT::SpriteHandle score;
  
  for (int i=0; i<4; i++) {
    avatars[i] = ESAT::SpriteFromFile(("assets/img/"+std::to_string(i+1)+".png").c_str());
  }
  
  skull = ESAT::SpriteFromFile("assets/img/skull.png");
  score = ESAT::SpriteFromFile("assets/img/score.png");
  block = ESAT::SpriteFromFile("assets/img/block.png");
  
  
  while(ESAT::WindowIsOpened() && !exit) {
    
    if (ESAT::IsSpecialKeyDown(ESAT::kSpecialKey_Escape))
      exit = true;
    
    ESAT::DrawBegin();
    ESAT::DrawClear(255,255,255);
    
    setText();
      
    if (!game_started) {
      
      char key = ESAT::GetNextPressedKey();
      if (key) {
        //Check 1-4 numbers
        if (key>=49 && key<=52) {
          player->avatar = key -48;
        } else {
          //Append to player name
          name += key;
        }
      }
      
      ESAT::DrawText(100.0f,30.0f,("Nickname:  "+name).c_str());
      
      for (int i=0; i<4; i++) {
        int x = 50.0f+320.0f*i;
        ESAT::DrawSprite(avatars[i], x, 100.0f);
        
        
        if (player->avatar == i+1) {
          ESAT::DrawSetFillColor(0,0,255);
          ESAT::DrawSetTextSize(70.0f);
        } else {
          setText();
        }
        
        ESAT::DrawText(x+130, 580.0f, std::to_string(i+1).c_str());

        
        setText();
        
        x = 50.0f;
        ESAT::DrawText(x, 660.0f, "Type to introduce your nickname.");
        ESAT::DrawText(x, 690.0f, "Press 1-4 to select your avatar.");
        ESAT::DrawText(x, 720.0f, "Press ENTER to connect to the server.");
      }
      
      if (ESAT::IsSpecialKeyPressed(ESAT::kSpecialKey_Enter)) {
        
        //Set player info  
        memcpy(player->name, name.c_str(), sizeof(name));
        pack->id = 3;
        pack->player = *player;
        
        
        //Send initial connection with Player info
        sendto(sock, (char*)pack, sizeof(Package), 0, (SOCKADDR*)&ips, sizeof(ip));
        
        //Get player info back
        memset(buffer, 0, sizeof(Package));
        recvfrom(sock, buffer, sizeof(Package), 0, (SOCKADDR*)&ip, &size);
        
        memcpy(pack_in, buffer, sizeof(Package));
        printf("PACKIN   %d\n",pack_in->id);
        if (pack_in->id) {
          player->id = pack_in->player.id;
          player->ip = pack_in->player.ip;
          player->color.r = pack_in->player.color.r;
          player->color.g = pack_in->player.color.g;
          player->color.b = pack_in->player.color.b;
          player->color.a = pack_in->player.color.a;
        } else {
          printf("Max players reached.Connection refused.\n");
          exit = true;
          disconnect_quietly = true;
        }
        game_started = true;
      }
    } else {
      Package* pack = new Package();
        
      if (ESAT::IsSpecialKeyPressed(ESAT::kSpecialKey_Up)) {
        pack->movement.direction = UP;
        send = true;
      } else if (ESAT::IsSpecialKeyPressed(ESAT::kSpecialKey_Right)) {
        pack->movement.direction = RIGHT;
        send = true;
      } else if (ESAT::IsSpecialKeyPressed(ESAT::kSpecialKey_Down)) {
        pack->movement.direction = DOWN;
        send = true;
      } else if (ESAT::IsSpecialKeyPressed(ESAT::kSpecialKey_Left)) {
        pack->movement.direction = LEFT;
        send = true;
      }

      if (ESAT::IsSpecialKeyPressed(ESAT::kSpecialKey_Control)) {
        pack->movement.blocking = true;
        send = true;
      } else if (ESAT::IsSpecialKeyPressed(ESAT::kSpecialKey_Space)) {
        pack->movement.shooting = true;
        send = true;
      }
        
      //Send Package struct with direction
      if (send) {
        pack->id = 1;
        pack->movement.player_id = player->id;
        sendto(sock, (char*)pack, sizeof(Package), 0, (SOCKADDR*)&ips, sizeof(ip));
      }
      send = false;
      delete pack;
      
      
      //Receive game status
      memset(buffer, 0, sizeof(Package));
      FD_SET(sock, &SOCK_IN);
      select(1, &SOCK_IN, NULL, NULL, &time);
      
      if (FD_ISSET(sock, &SOCK_IN)) {
        if (recvfrom(sock, buffer, sizeof(Package), 0, (SOCKADDR*)&ip, &size)) {
          
          memset (pack_in, 0, sizeof(Package));
          memcpy(pack_in, buffer, sizeof(Package));
          
          GameStatus status = pack_in->gamestatus;
          
          for (int i=0; i<status.num_players; i++) {
            drawCube(g_player_size/2, status.players[i].color, status.players[i].position);
            
            if (!status.players[i].alive) {
              ESAT::DrawSprite(skull, status.players[i].position.x, status.players[i].position.y);
            } else if (status.players[i].blocking) {
              ESAT::DrawSprite(block, status.players[i].position.x, status.players[i].position.y);
            }
            
            
            //Draw player info
            int x,y,lx,ly;
            switch (status.players[i].id) {
              case 0:
                x = 0;
                y= 20;
                lx = x+50.0f;
                ly = y+20.0f;
                break;
              case 1:
                x = kWinWidth - 100;
                y = 20;
                lx = x-170.0f;
                ly = y+20.0f;
                break;
              case 2:
                x = 0;
                y = kWinHeight - 60;
                lx = x+50.0f;
                ly = y+20.0f;
                break;
              case 3:
                x = kWinWidth - 100;
                y = kWinHeight - 60;
                lx = x-170.0f;
                ly = y+20.0f;
                break;
            }
            
            //Draw avatar
            ESAT::Mat3 scale, translate, transform;
            float factor = 0.2;
            
            scale = ESAT::Mat3Scale(factor, factor);
            translate = ESAT::Mat3Translate(x,y);
            transform = ESAT::Mat3Multiply(translate, scale);
            ESAT::DrawSpriteWithMatrix(avatars[status.players[i].avatar - 1], transform);
            
            //Draw nickname
            ESAT::DrawSetFillColor(status.players[i].color.r,status.players[i].color.g,status.players[i].color.b);
            ESAT::DrawSetTextSize(20.0f);
            ESAT::DrawText(lx, ly-20.0f, status.players[i].name);
            
            //Draw score
            ESAT::DrawSprite(score, lx, ly-10);
            setText();
            ESAT::DrawText(lx+40, ly+15, std::to_string(status.players[i].score).c_str());
            
            //Draw lifebar
            Point2 pos = {lx, ly+25};
            drawLifeBar(status.players[i].health, status.players[i].color, pos);
          }
          
          //Draw shots
          for (int i=0; i<status.num_shots; i++) {
            drawCube(g_shot_size, status.players[status.shots[i].player_id].color, status.shots[i].position);
          }
          
          //Draw hits
          for (int i=0; i<status.num_hits; i++) {
            drawHit(status.hits[i]);
          }
        }
      }
      
      
      
      
    
    }
    
    //Draw status
      
    ESAT::DrawEnd();
    ESAT::WindowFrame();
  }
  
  //Send disconnection signal
  if (!disconnect_quietly) {
    memset (pack, 0, sizeof(Package));
    pack->id = 4;
    pack->player = *player;
    
    //Send initial connection with Player info
    sendto(sock, (char*)pack, sizeof(Package), 0, (SOCKADDR*)&ips, sizeof(ip));
  }
  
  // cleanup
  closesocket(sock);
  closesocket(socks);
  WSACleanup();
  ESAT::WindowDestroy();
  
  return 0;
}