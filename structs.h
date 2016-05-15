enum Direction { NONE, UP, RIGHT, DOWN, LEFT };

typedef struct {
  int player_id;
  Direction direction;
  bool shooting;
  bool blocking;
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
  int avatar;
  in_addr ip;
  char name[50];
  Point2 position;
  Color color;
  Direction direction;
  float health;
  bool alive;
  bool blocking;
  int score;
  double last_shot_time;
} Player;

typedef struct {
  int player_id;
  Point2 position;
  Vec2 velocity;
} Shot;

typedef struct {
  Point2 position;
  float age;
} Hit;

typedef struct {
  int num_players;
  int num_shots;
  int num_hits;
  Player players[10];
  Shot shots[50];
  Hit hits[50];
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

const int kWinWidth = 1366;
const int kWinHeight = 768;

float g_player_speed = 0.2f;
float g_shot_speed = 0.5f;
float g_strength = 10.0f;
int g_player_health = 20.0f;
int g_max_shots = 50;
double g_shot_delay = 300.0f;

float g_player_size = 60.0f;
float g_shot_size = 10.0f;