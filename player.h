#ifndef PLAYER_H
#define PLAYER_H
int connect_to_ringmaster(const char* ringmaster_hostname, const char* ringmaster_port);
int get_player_id(int ringmaster_fd);
int listen_and_send_hostinfo(int ringmaster_fd);
#endif
