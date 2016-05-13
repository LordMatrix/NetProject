enum Direction { NONE, UP, RIGHT, DOWN, LEFT };

typedef struct {
  int player_id;
  Direction direction;
  bool shooting;
} Movement;

typedef struct {
	float x,y;
} Point2, Vec2;

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
  Point2 position;
  Color color;
  Direction direction;
  float health;
} Player;

typedef struct {
  int player_id;
  Point2 position;
  Vec2 velocity;
} Shot;

typedef struct {
  int num_players;
  int num_shots;
  Player players[10];
  Shot shots[50];
} GameStatus;

typedef struct {
  int id; //type of package
  union { Movement movement; GameStatus gamestatus; Player player; }; 
} Package;
//id = 0 -> ERROR
//id = 1 -> Direction
//id = 2 -> GameState
//id = 3 -> Player
//id = 4 -> Disconnection

int g_refresh_time = 50;

const int kWinWidth = 800;
const int kWinHeight = 600;

float g_player_speed = 5.0f;
float g_shot_speed = 1.5f;
float g_strength = 10.0f;

float g_player_size = 20.0f;
float g_shot_size = 5.0f;