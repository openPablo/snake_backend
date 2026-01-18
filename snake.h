
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
struct Snake {
    char name[16];
    int length;
    coords dir;
    float speed;
    struct segment *head;
    struct Snake *next;
};
#endif //MULTIPLAYER_SNAKE_H