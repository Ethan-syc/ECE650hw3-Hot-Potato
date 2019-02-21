#ifndef RINGMASTER_H
#define RINGMASTER_H
#include <string>
#include <utility>
#include <vector>
using namespace std;
int listen_to_port(const char *hostname, const char *ringmaster_listen_port);
// send to prev, 2 send to 1, 1 send to last
void send_player_info_to_neighbor(
    vector<int> player_fd_vector,
    vector<pair<string, string>> player_host_port_vector);
int start_game(const vector<int> &player_fd_vector,
               const int num_players, const int num_hops);
#endif
