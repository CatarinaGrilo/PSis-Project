#include <stdlib.h>
#include <ncurses.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include "chase.h"

WINDOW * message_win;
int sleep_delay = 3000000;


typedef struct player_position_t{
    int x, y;
    char c;
} player_position_t;

void new_player (player_position_t * player, char c){
    player->x = WINDOW_SIZE/2;
    player->y = WINDOW_SIZE/2;
    player->c = c;
}

player_position_t p1;

int main(){

    /* Open and link socket */
    int sock_fd;
    sock_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sock_fd == -1){
	    perror("socket: ");
	    exit(-1);
    }
    struct sockaddr_un local_client_addr;
    local_client_addr.sun_family = AF_UNIX;
    sprintf(local_client_addr.sun_path,"%s_%d", SOCKET_NAME, getpid());

    unlink(local_client_addr.sun_path);
    int err = bind(sock_fd, (const struct sockaddr *) &local_client_addr, sizeof(local_client_addr));
    if(err == -1){
        perror("bind");
        exit(-1);
    }
    struct sockaddr_un server_addr;
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, SOCKET_NAME);


    char ch = '*';
    int number_bots = 1, i = 0;
    remote_char_t msg_send, msg_rcv;


    /* Send Connection Message*/
    msg_send.type = 0; 
    msg_send.ch = '*';    
    for (i = 0; i < number_bots; i++){
        msg_send.bots->id = i;
        sendto(sock_fd, &msg_send, sizeof(remote_char_t), 0, 
                (const struct sockaddr *)&server_addr, sizeof(server_addr));  
        /* Wait to receive ball information*/
        do{
            //printf("Waiting\n");
            recv(sock_fd, &msg_rcv, sizeof(remote_char_t), 0);
        }while(msg_rcv.type != 1);
    }
    //printf("Done\n");

    new_player(&p1, ch); // acho q nao é assim
    direction dir = LEFT;

    while (1){
    
        usleep(sleep_delay);
        //random()%4)
        for(i = 0; i < number_bots; i++){
            //direction dir = direction[i];
            /* Send Ball Movement Message */
            msg_send.type = 2;
            msg_send.ch = '*';
            msg_send.direction = dir;

            sendto(sock_fd, &msg_send, sizeof(remote_char_t), 0, 
                (const struct sockaddr *) &server_addr, sizeof(server_addr));
        }
    }


    exit(0);
}