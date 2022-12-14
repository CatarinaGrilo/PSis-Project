#define WINDOW_SIZE 20
#define MAX_ARRAY 10
#define SOCKET_NAME "/tmp/sock_Gamechase"


/*  Message type:
    0 - client        
    1 - Bots
    2 - Prizes
 */
typedef struct ch_info_t{
    int id;
    int ch;
    int pos_x, pos_y;
    int health;
} ch_info_t;
typedef enum direction{
    UP,
    DOWN,
    LEFT,
    RIGHT
} direction;

typedef struct player_position_t{
    int x, y;
    char c;
} player_position_t;

/*  Message type:
    0 - Connect:            client -> server
    1 - Ball information:   server -> client
    2 - Ball movement:      client -> server
    3 - Field Status:       server -> client
    4 - Health 0:           server -> client
    5 - Disconnect:         client -> server
 */
typedef struct remote_char_t{
    int type;
    char ch;
    int id;
    player_position_t player_position;
    direction direction;
    ch_info_t clients[MAX_ARRAY];
    ch_info_t bots[MAX_ARRAY];
    ch_info_t prizes[MAX_ARRAY];
} remote_char_t;

