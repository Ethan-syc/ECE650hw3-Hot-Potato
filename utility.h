#ifndef UTILITY_H
#define UTILITY_H
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
int send_all(int fd, char *buf, int len);
int send_all(int fd, const char *buf, int len);
int recv_all(int fd, char *buf, int len);
int recv_all(int fd, const char *buf, int len);
void send_int(int fd, int data_to_send);
int recv_int(int fd);
#endif
