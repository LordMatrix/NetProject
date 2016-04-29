enum Direction { NONE, UP, RIGHT, DOWN, LEFT };

typedef struct {
  int player_id;
  Direction direction;
} Movement;

typedef struct {
	float x,y;
} Point2;

typedef struct {
  unsigned char r;
  unsigned char g;
  unsigned char b;
  unsigned char a;
} Color;

typedef struct {
  int id;
  int ip;
  char name[50];
  int x, y;
  Color color;
  Direction direction;
} Player;

typedef struct {
  int num_players;
  Player players[64];
} GameState;

typedef struct {
  int id; //type of package
  union { Movement movement; GameState gamestate; Player player; }; 
} Package;
//id = 0 -> ERROR
//id = 1 -> Direction
//id = 2 -> GameState
//id = 3 -> Player