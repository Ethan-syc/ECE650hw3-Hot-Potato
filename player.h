#ifndef PLAYER_H
#define PLAYER_H
#include <string>
#include <utility>
#include <vector>
using namespace std;
int accept_from_client(int socket_fd);
int connect_to_server(const char *hostname, const char *port);
int get_player_id(int ringmaster_fd);
int listen_and_send_hostinfo(int ringmaster_fd);
pair<string, string> recv_neighbor_info(int ringmaster_fd);
pair<int, int> form_ring(int my_listen_fd,
                         pair<string, string> neighbor_host_port_pair,
                         int player_id);

int send_to_neighbor(int hops, pair<int, int> neighbor_fd_pair,
                     string curr_trace, int player_id, int player_num);
#endif
