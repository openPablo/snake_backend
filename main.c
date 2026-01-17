#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>
#include <sys/time.h>

const suseconds_t TICK_RATE = 33333;
const int PORT =  8080;
const float STEP_SIZE = 0.01;

uv_loop_t *loop;
uv_udp_t send_socket;
uv_udp_t recv_socket;
struct Snake* snakes;

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
    struct segment *head;
    struct Snake *next;
};
coords get_random_start() {
    return (coords){(float)rand() / RAND_MAX, (float)rand() / RAND_MAX};
}
void spawn_snake(char name[16]) {
    struct segment *segment = malloc(sizeof *segment);
    segment->pos = get_random_start();
    segment->next = NULL;
    struct Snake *snake = malloc(sizeof(struct Snake));
    snake->length = 10;
    snake->dir = (coords){0.5f, 0.5f};
    snake->head = segment;
    snake->next = NULL;

    strncpy(snake->name, name, sizeof(snake->name) - 1);
    snake->name[sizeof(snake->name) - 1] = '\0';
    snakes->next = snake;
}

void slither(struct Snake *snake) {

}
void kill_snake(struct Snake *snake) {
    struct segment* tmp;
    while (snake->head) {
        tmp = snake->head;
        snake->head = snake->head->next;
        free(tmp);
    }
    struct Snake* tmpSnake;
    while (snake) {
        tmpSnake = snake;
        snake = snake->next;
        free(tmpSnake);
    }
}
suseconds_t get_microsecond_diff(const struct timeval* t1, const struct timeval* t2) {
    return (t2->tv_sec - t1->tv_sec) * 1000000 + (t2->tv_usec - t1->tv_usec);
}

int main() {
    create_uv_loop(PORT);
    spawn_snake("Pablito");
    spawn_snake("Ornellita");

    struct timeval t1, t2;
    while (1) {
        gettimeofday(&t1, NULL);
        t2 = t1;
        struct Snake* tmpSnake = snakes;
        while (tmpSnake) {
            printf("Snake: %s\n", snakes->name);
            tmpSnake = tmpSnake->next;
        }
        while (get_microsecond_diff(&t1, &t2) <= TICK_RATE) {
            uv_run(loop, UV_RUN_NOWAIT);
            gettimeofday(&t2, NULL);
        }
    }

    uv_loop_close(loop);
    return 0;
}