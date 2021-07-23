/**
 * @file
 * @brief BSD Sockets compatible API definitions
 *
 * An API for applications to use BSD Sockets like API.
 */

/*
 * Copyright (c) 2017 Linaro Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __NET_SOCKET_H
#define __NET_SOCKET_H

/**
 * @brief BSD Sockets compatible API
 * @defgroup bsd_sockets BSD Sockets compatible API
 * @{
 */

#include <sys/types.h>
#include <zephyr/types.h>
#include <net/net_ip.h>

#ifdef __cplusplus
extern "C" {
#endif

struct zsock_pollfd
{
    int fd;
    short events;
    short revents;
};

/* Values are compatible with Linux */
#define ZSOCK_POLLIN 1
#define ZSOCK_POLLOUT 4

int zsock_socket(int family, int type, int proto);
int zsock_close(int sock);
int zsock_bind(int sock, const struct sockaddr *addr, socklen_t addrlen);
int zsock_connect(int sock, const struct sockaddr *addr, socklen_t addrlen);
int zsock_listen(int sock, int backlog);
int zsock_accept(int sock, struct sockaddr *addr, socklen_t *addrlen);
ssize_t zsock_send(int sock, const void *buf, size_t len, int flags);
ssize_t zsock_recv(int sock, void *buf, size_t max_len, int flags);
int zsock_fcntl(int sock, int cmd, int flags);
int zsock_poll(struct zsock_pollfd *fds, int nfds, int timeout);
int zsock_inet_pton(sa_family_t family, const char *src, void *dst);

#if defined(CONFIG_NET_SOCKETS_POSIX_NAMES)
#define socket zsock_socket
#define close zsock_close
#define bind zsock_bind
#define connect zsock_connect
#define listen zsock_listen
#define accept zsock_accept
#define send zsock_send
#define recv zsock_recv
#define fcntl zsock_fcntl

#define poll zsock_poll
#define pollfd zsock_pollfd
#define POLLIN ZSOCK_POLLIN
#define POLLOUT ZSOCK_POLLOUT

#define inet_ntop net_addr_ntop
#define inet_pton zsock_inet_pton
#endif

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* __NET_SOCKET_H */
