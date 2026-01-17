#include <stdio.h>
#include <stdlib.h>
#include <uv.h>

#include <time.h>

#define TICK_RATE_NS 16666666
#define PORT 8080

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
    int x;
    int y;
} coords;
struct Snake {
    char name[16];
    int length;
    coords dir;
    struct Snake *next;
};

int main() {
    create_uv_loop(PORT);
    struct Snake snake =
    while (1) {
        uv_run(loop, UV_RUN_NOWAIT);
        while (snake) {

            snake = snake->next;
        }
    }


    uv_loop_close(loop);
    return 0;
}