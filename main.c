#include <stdio.h>
#include <stdlib.h>
#include <uv.h>
#include <sys/time.h>

const suseconds_t TICK_RATE = 16666666;
const int PORT =  8080;

uv_loop_t *loop;
uv_udp_t send_socket;
uv_udp_t recv_socket;


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
void spawn_snake(struct Snake *snake) {
    struct segment *segment = malloc(sizeof(struct segment));
    segment->pos = (coords){10.0f, 20.0f};
    segment->next = NULL;
    *snake = (struct Snake){"pablito", 10, {0.5f, 0.5f}, segment, NULL};
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
suseconds_t get_us_diff(struct timeval* t1, struct timeval* t2) {

}

int main() {
    create_uv_loop(PORT);
    struct Snake* snake = malloc(sizeof(struct Snake));
    spawn_snake(snake);
    struct timeval t1, t2;
    while (1) {
        gettimeofday(&t1, NULL);
        t2 = t1;
        struct Snake* tmpSnake = snake;
        while (tmpSnake) {
            printf("Snake: %s\n", snake->name);
            tmpSnake = tmpSnake->next;
        }
        while (get_us_diff(&t1, &t2) <= TICK_RATE) {
            printf("%ld\n", (long)(t2.tv_usec - t1.tv_usec));
            uv_run(loop, UV_RUN_NOWAIT);
            gettimeofday(&t2, NULL);
        }
    }

    uv_loop_close(loop);
    return 0;
}