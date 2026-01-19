#include <stdio.h>
#include <stdlib.h>
#include <uv.h>
#include "snake.h"
#include <string.h>
#include <math.h>

const int PORT =  8080;

uv_loop_t *loop;
uv_udp_t send_socket;
uv_udp_t recv_socket;

struct snake *SNAKES = NULL;
struct food *FOODS = NULL;
int NUM_FOOD = 0;

coords get_random_start() {
    return (coords){(float)rand() / RAND_MAX, (float)rand() / RAND_MAX};
}
double distance(coords *d1, coords *d2) {
    return sqrt(pow(d2->x - d1->x, 2) + pow(d2->y - d1->y, 2));
}

void detect_collision_food(struct snake *snake, struct food **foods) {
    struct food *tmpFood = *foods;
    struct food *prev = NULL;
    while (tmpFood) {
        if (distance(&tmpFood->pos, &snake->head->pos) <= 0.01) {
            snake->digestingFood +=1;
            struct food *eatenFood = tmpFood;
            if (prev) {
                prev->next = tmpFood->next;
            }else {
                *foods = tmpFood->next;
            }
            tmpFood = tmpFood->next;
            free(eatenFood);
            NUM_FOOD -=1;
            continue;
        }
        prev = tmpFood;
        tmpFood = tmpFood->next;
    }
}
int spawn_snake(struct snake **snakes, char name[16]) { // NOT thread safe when inserting a new snake, inserts at start of list
    struct snake *tmpSnake = *snakes;
    while (tmpSnake) {
        if (strcmp(tmpSnake->name, name) == 0) {
            return -1;
        }
        tmpSnake = tmpSnake->next;
    }
    struct segment *head = malloc(sizeof (struct segment));
    struct segment *tail = malloc(sizeof (struct segment));
    struct snake *snake = malloc(sizeof(struct snake));

    head->pos = get_random_start();
    head->next = tail;
    tail->pos.x = head->pos.x - snake->dir.x * snake->speed;
    tail->pos.y = head->pos.y - snake->dir.y * snake->speed;

    snake->length = 10;
    snake->dir = (coords){0.5f, 0.5f};
    snake->head = head;
    snake->next = *snakes;
    snake->speed = 0.1f;
    snake->digestingFood = 0;

    strncpy(snake->name, name, sizeof(snake->name) - 1);
    snake->name[sizeof(snake->name) - 1] = '\0';

    *snakes = snake;
    return 0;
}
void slither(struct snake *snake, struct food **foods) {
    printf("Num of food: %d\n", NUM_FOOD);
    struct segment* newSegment = malloc(sizeof(struct segment));

    newSegment->pos.x = snake->head->pos.x + snake->dir.x * snake->speed;
    newSegment->pos.y = snake->head->pos.y + snake->dir.y * snake->speed;
    newSegment->next = snake->head;

    snake->head = newSegment;
    detect_collision_food(snake, foods);
    if (snake->digestingFood > 0) {
        snake->digestingFood -= 1;
        snake->length += 1;
    }
    else{
        struct segment *tmp = snake->head;
        while (tmp) {
            if (tmp->next->next == NULL) {
                free(tmp->next);
                tmp->next = NULL;
            }
            tmp = tmp->next;
        }
    }
}
void spawn_food(struct food **foods, int *num_food) { // Update food list is NOT thread safe, implement safety when adding threads
    struct food *food = malloc(sizeof(struct food));
    food->pos = get_random_start();
    food->next = *foods;
    *foods = food;
    *num_food += 1;
}
int kill_snake(struct snake **snakes, char name[16]) {
    struct snake *tmpSnake = *snakes;
    struct snake *prev = NULL;
    while (tmpSnake != NULL && strcmp(tmpSnake->name, name) != 0) {
        prev = tmpSnake;
        tmpSnake = tmpSnake->next;
    }
    if (tmpSnake == NULL)  return -1;

    if (prev) {
        prev->next = tmpSnake->next;
    }else {
        *snakes = tmpSnake->next;
    }
    struct segment* tmp;
    while (tmpSnake->head) {
        tmp = tmpSnake->head;
        tmpSnake->head = tmpSnake->head->next;
        free(tmp);
    }
    free(tmpSnake);
    return 0;
}


void on_read(uv_udp_t *req, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr, unsigned flags) {
    if (nread < 0) {
        fprintf(stderr, "Read error %s\n", uv_err_name(nread));
        uv_close((uv_handle_t*) req, NULL);
        free(buf->base);
        return;
    }
    char sender[17] = { 0 };
    uv_ip4_name((const struct sockaddr_in*) addr, sender, 16);
    fprintf(stderr, "Recv from %s\n", sender);
    unsigned int *as_integer = (unsigned int*)buf->base;
    unsigned int ipbin = ntohl(as_integer[4]);
    unsigned char ip[4] = {0};
    int i;
    for (i = 0; i < 4; i++)
        ip[i] = (ipbin >> i*8) & 0xff;
    fprintf(stderr, "Offered IP %d.%d.%d.%d\n", ip[3], ip[2], ip[1], ip[0]);
    free(buf->base);
    uv_udp_recv_stop(req);
}

void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    buf->base = malloc(suggested_size);
    buf->len = suggested_size;
}
void create_uv_loop(int port) {
    loop = uv_default_loop();
    uv_udp_init(loop, &recv_socket);

    struct sockaddr_in recv_addr;
    uv_ip4_addr("0.0.0.0", port, &recv_addr);
    uv_udp_bind(&recv_socket, (const struct sockaddr *)&recv_addr, UV_UDP_REUSEADDR);
    uv_udp_recv_start(&recv_socket, alloc_buffer, on_read);

    uv_udp_init(loop, &send_socket);

}

void game_loop() {
    struct snake* tmpSnake;
    tmpSnake = SNAKES;
    while (tmpSnake) {
        slither(tmpSnake,&FOODS);
        tmpSnake = tmpSnake->next;
    }
    
}
int main() {
    char snakeName[20];
    for (int i = 0; i < 2000; i++) {
        snprintf(snakeName, 16, "snake%d", i);
        spawn_snake(&SNAKES, snakeName);
        spawn_food(&FOODS, &NUM_FOOD);
    }
    create_uv_loop(PORT);

    uv_timer_t timer_req;
    uv_timer_init(loop, &timer_req);
    uv_timer_start(&timer_req, game_loop, 0, 33);

    uv_run(loop, UV_RUN_DEFAULT);

    uv_loop_close(loop);
    return 0;
}