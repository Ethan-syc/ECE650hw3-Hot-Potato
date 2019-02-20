#ifndef PLAYER_H
#define PLAYER_H
#include <string>
#include <utility>
using namespace std;
int accept_from_client(int fd);
int connect_to_server(const char *hostname,
                          const char *port);
int get_player_id(int ringmaster_fd);
int listen_and_send_hostinfo(int ringmaster_fd);
pair<string, string> recv_neighbor_info(int ringmaster_fd);
pair<int, int> form_ring(int my_listen_fd,
                         pair<string, string> neighbor_host_port_pair,
                         int player_id);
#endif
