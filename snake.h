
#ifndef MULTIPLAYER_SNAKE_H
#define MULTIPLAYER_SNAKE_H
typedef struct {
    float x;
    float y;
} coords;
struct segment {
    coords pos;
    struct segment *next;
};
struct snake {
    char name[16];
    int length;
    int digestingFood;
    coords dir;
    float speed;
    struct segment *head;
    struct segment *tail;
    UT_hash_handle hh;
};

struct food {
    coords pos;
    struct food *next;
};

coords get_random_start();
int ate_food();
int spawn_snake();
void slither();
void spawn_food();
int kill_snake();

#endif //MULTIPLAYER_SNAKE_H