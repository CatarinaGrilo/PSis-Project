#include <stdlib.h>
#include <ncurses.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include "chase.h"

WINDOW * message_win;


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


    char ch;
    int number_bots = 0;
    remote_char_t msg_send, msg_rcv;


    /* Send Connection Message*/
    msg_send.type = 0; 
    msg_send.ch = '*';    
    for (int i = 0; i < number_bots; i++){
        sendto(sock_fd, &msg_send, sizeof(remote_char_t), 0, 
                (const struct sockaddr *)&server_addr, sizeof(server_addr));
        do{
            recv(sock_fd, &msg_rcv, sizeof(remote_char_t), 0);
        }while(msg_rcv.type != 1);
    }

    /* Receives Ball Information */       


    new_player(&p1, ch); // acho q nao é assim

    // while (1)
    // {
    //     n++;
    //     sleep_delay = random()%700000;
    //     usleep(sleep_delay);
    //     direction = random()%4;
    //     switch (direction)
    //     {
    //     case LEFT:
    //        printf("%d Going Left   \n", n);
    //         break;
    //     case RIGHT:
    //         printf("%d Going Right   \n", n);
    //        break;
    //     case DOWN:
    //         printf("%d Going Down   \n", n);
    //         break;
    //     case UP:
    //         printf("%d Going Up    \n", n);
    //         break;
    //     }

    //     //TODO_9
    //     // prepare the movement message
    //     m.direction = direction;
    //     m.msg_type = 1;

    //     //TODO_10
    //     //send the movement message
    //     sendto(sock_fd, &m, sizeof(remote_char_t), 0, 
    //         (const struct sockaddr *) &server_addr, sizeof(server_addr));
    //     recv(sock_fd, &m_receive, sizeof(remote_char_t), 0);
    //     if(m_receive.msg_type==3){
    //         printf("in the same position\n");
    //     }
    // }

    // int key = -1;
    // while(key != 27 && key!= 'q' && key != 'h'){
    //     key = wgetch(my_win);	
    //     if (key == KEY_LEFT || key == KEY_RIGHT || key == KEY_UP || key == KEY_DOWN){
            
    //         /* Sends Ball Movement mesage*/
    //         msg_send.type = 2;
    //         msg_send.ch = ch;
    //         msg_send.direction = find_direction(key);
    //         sendto(sock_fd, &msg_send, sizeof(remote_char_t), 0, 
    //             (const struct sockaddr *)&server_addr, sizeof(server_addr));

    //         /* Wait for answer */
    //         recv(sock_fd, &msg_rcv, sizeof(remote_char_t), 0);
    //     }
    // }
    // if(key == 'q'){
    //     msg_send.type = 5;
    //     sendto(sock_fd, &msg_send, sizeof(remote_char_t), 0, 
    //         (const struct sockaddr *)&server_addr, sizeof(server_addr));
    //     endwin();
    // }

    exit(0);
}