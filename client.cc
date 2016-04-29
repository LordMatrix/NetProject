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
  SOCKET sock, socks;
  struct sockaddr_in ip, ips;
  char buffer[512];
  int size=sizeof(ip);
  WSAStartup(MAKEWORD(2,0), &wsa);
  sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  ip.sin_family=AF_INET;
  ip.sin_port=htons(8888);
  ip.sin_addr.s_addr=inet_addr("127.0.0.1");
	bind(sock, (SOCKADDR*)&ip, sizeof(ip));
  
  socks = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  ips.sin_family=AF_INET;
  ips.sin_port=htons(9999);
  ips.sin_addr.s_addr=inet_addr("127.0.0.1");
  
  ESAT::WindowInit(1366, 768);
  ESAT::DrawSetTextFont("assets/font/medieval.ttf");
  
  Player* player = new Player();
  Package* pack = new Package();
  
  bool send = false;
  
  //Set placeholder player info
  /**** Placeholders *****/
  //player->name = "player";
	
  /***********************/
  
  pack->id = 3;
  pack->player = *player;
  
  //Send initial connection with Player info
  sendto(sock, (char*)pack, sizeof(Package), 0, (SOCKADDR*)&ips, sizeof(ip));
  
  //Get player info back
  memset(buffer, 0, 512);
  recvfrom(sock, buffer, 512, 0, (SOCKADDR*)&ip, &size);
  Package* pack_in = new Package();
  memcpy(pack_in, buffer, sizeof(Package));
  
  player->id = pack_in->player.id;
  player->ip = pack_in->player.ip;
  player->color.r = pack_in->player.color.r;
	player->color.g = pack_in->player.color.g;
	player->color.b = pack_in->player.color.b;
	player->color.a = pack_in->player.color.a;
  
  printf("pack_in id: %d\n", pack_in->id);
  printf("My id is: %s\n", player->id);
  
  while(ESAT::WindowIsOpened() && !ESAT::IsSpecialKeyDown(ESAT::kSpecialKey_Escape)) {
  
	ESAT::DrawBegin();
	ESAT::DrawClear(255,255,255);
	
	Package* pack = new Package();
		
	if (ESAT::IsSpecialKeyDown(ESAT::kSpecialKey_Up)) {
		pack->movement.direction = UP;
		send = true;
		printf("arriba\n");
	} else if (ESAT::IsSpecialKeyDown(ESAT::kSpecialKey_Right)) {
		pack->movement.direction = RIGHT;
		send = true;
		printf("derecha\n");
	} else if (ESAT::IsSpecialKeyDown(ESAT::kSpecialKey_Down)) {
		pack->movement.direction = DOWN;
		send = true;
		printf("abajo\n");
	} else if (ESAT::IsSpecialKeyDown(ESAT::kSpecialKey_Left)) {
		pack->movement.direction = LEFT;
		send = true;
		printf("izquierda\n");
	}
	
	
	//Send Package struct with direction
  if (send) {
    printf("sending\n");
    pack->id = 1;
    pack->movement.player_id = player->id;
    sendto(sock, (char*)pack, sizeof(Package), 0, (SOCKADDR*)&ips, sizeof(ip));
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