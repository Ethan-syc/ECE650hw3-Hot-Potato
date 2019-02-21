#ifndef UTILITY_H
#define UTILITY_H
#include <cstring>
#include <netdb.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
using namespace std;
int send_all(int fd, char *buf, int len);
int send_all(int fd, const char *buf, int len);
int recv_all(int fd, char *buf, int len);
int recv_all(int fd, const char *buf, int len);
int send_int(int fd, int data_to_send);
int recv_int(int fd);
string recv_trace(int fd);
void send_trace(int fd, string curr_trace, int player_id);
int select_active_fd(fd_set readfds, int nfds, const vector<int> &fd_vector);
#endif
