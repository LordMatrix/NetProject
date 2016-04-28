//Select UDP
#pragma comment(lib, "ws2_32.lib")
#define DrawTextA DrawText

#include "stdio.h"
#include "winsock2.h"

#include "ESAT/window.h"
#include "ESAT/draw.h"
#include "ESAT/input.h"

#include "structs.h"

void drawCube(int size, Color color, Point2 position) {
  float x = position.x;
  float y = position.y;
  
  //Create sube points
  float pathpoints[10] = {-size+x,size+y, size+x,size+y, size+x,-size+y, -size+x,-size+y, -size+x,size+y};
  
  ESAT::DrawSetStrokeColor(color.r,color.g,color.b,color.a);
  ESAT::DrawSetFillColor(color.r,color.g,color.b,color.a);
  ESAT::DrawSolidPath(pathpoints, 5);
}

int ESAT::main(int argc, char** argv) {
  
  
  WSAData wsa;
  SOCKET sock;
  struct sockaddr_in ip;
  char buffer[512];
  int size=0;
  WSAStartup(MAKEWORD(2,0), &wsa);
  sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  ip.sin_family=AF_INET;
  ip.sin_port=htons(9999);
  ip.sin_addr.s_addr=inet_addr("127.0.0.1");
	
  ESAT::WindowInit(1366, 768);
  ESAT::DrawSetTextFont("assets/font/medieval.ttf");
  
  Player* player = new Player();
  Package* pack = new Package();
  
  bool send = false;
  
  //Set placeholder player info
  /**** Placeholders *****/
	player->id = 42;
	player->ip = 25000;
	//player->name = "player";
	

	player->color.r = 100;
	player->color.g = 50;
	player->color.b = 150;
	player->color.a = 255;
  /***********************/
  
  pack->id = 3;
  pack->player = *player;
  
  //Send initial connection with Player info
  sendto(sock, (char*)pack, sizeof(Package), 0, (SOCKADDR*)&ip, sizeof(ip));
  
  
  while(ESAT::WindowIsOpened() && !ESAT::IsSpecialKeyDown(ESAT::kSpecialKey_Escape)) {
  
	ESAT::DrawBegin();
	ESAT::DrawClear(255,255,255);
	
	Package* pack = new Package();
		
	if (ESAT::IsSpecialKeyDown(ESAT::kSpecialKey_Up)) {
		pack->direction = UP;
		send = true;
		printf("arriba\n");
	} else if (ESAT::IsSpecialKeyDown(ESAT::kSpecialKey_Right)) {
		pack->direction = RIGHT;
		send = true;
		printf("derecha\n");
	} else if (ESAT::IsSpecialKeyDown(ESAT::kSpecialKey_Down)) {
		pack->direction = DOWN;
		send = true;
		printf("abajo\n");
	} else if (ESAT::IsSpecialKeyDown(ESAT::kSpecialKey_Left)) {
		pack->direction = LEFT;
		send = true;
		printf("izquierda\n");
	}
	
	
	//Send Package struct with direction
  if (send) {
    printf("sending\n");
    pack->id = 1;
    sendto(sock, (char*)pack, sizeof(Package), 0, (SOCKADDR*)&ip, sizeof(ip));
  }
  send = false;
  delete pack;
	
	//Draw status
	ESAT::DrawSetStrokeColor(0,0,0);
	ESAT::DrawSetFillColor(0,0,0);
	ESAT::DrawText(100.0f, 100.0f, "ola k ase");
	
	ESAT::DrawEnd();
	ESAT::WindowFrame();
  
  }
  
  ESAT::WindowDestroy();
  return 0;
}