#include <stdlib.h>
#include <ncurses.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include "../chase.h"


WINDOW * message_win;

/* Creates new player */
void new_player (player_position_t * player, char c){
    player->x = WINDOW_SIZE/2;
    player->y = WINDOW_SIZE/2;
    player->c = c;
}

/* Calculates a random direction for the bot*/
direction randomDirection(){

    int randomnumber = random() % 4;
    //printf("%d\n", randomnumber);

    if(randomnumber == 0)
        return UP;
    else if(randomnumber == 1)
        return DOWN;
    else if(randomnumber == 2)
        return LEFT;
    else if(randomnumber == 3)
        return RIGHT;
    
    return -1;
}

/* Prints direction of the bot*/
void print_direction(direction direction){
    
    switch (direction)   {
        case UP: 
            printf("UP\n");
            break;
        case DOWN:
            printf("DOWN\n");
            break;
        case LEFT:
            printf("LEFT\n");
            break;
        case RIGHT:
            printf("RIGHT\n");
            break;
   }
   printf("What?");
   return;
}

int main(){

    /* Open and link socket */
    int sock_fd;
    char ADDRESS[20];
    
    printf("Put the Address of server: ");
    scanf("%s",ADDRESS); 
    sock_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sock_fd == -1){
	    perror("socket: ");
	    exit(-1);
    }
    struct sockaddr_un local_client_addr;
    local_client_addr.sun_family = AF_UNIX;
    sprintf(local_client_addr.sun_path,"%s_%d", ADDRESS, getpid());

    unlink(local_client_addr.sun_path);
    int err = bind(sock_fd, (const struct sockaddr *) &local_client_addr, sizeof(local_client_addr));
    if(err == -1){
        perror("bind");
        exit(-1);
    }
    struct sockaddr_un server_addr;
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, ADDRESS);


    char ch = '*';
    int number_bots=0, i = 0;
    remote_char_t msg_send, msg_rcv;

    do{
        printf("\nNumber of bots: ");
        scanf("%d",&number_bots); 
        if(number_bots < 1 || number_bots > 10){
          printf("\nNumber of bots must be between 1-10.");  
        }
    }while(number_bots < 1 || number_bots > 10);

    /* Send Connection Message*/
    msg_send.type = 0; 
    msg_send.ch = '*';    
    for (i = 0; i < number_bots; i++){
        msg_send.id = i;
        sendto(sock_fd, &msg_send, sizeof(remote_char_t), 0, 
                (const struct sockaddr *)&server_addr, sizeof(server_addr));  
        /* Wait to receive ball information*/
        do{
            //printf("Waiting\n");
            recv(sock_fd, &msg_rcv, sizeof(remote_char_t), 0);
        }while(msg_rcv.type != 1);
    }
    //printf("Done\n");

    while (1){
    
        usleep(BOT_SLEEP);
        
        /* Send Ball Movement Message */
        for(i = 0; i < number_bots; i++){
            //printf("Sending information\n");
            msg_send.type = 2;
            msg_send.ch = ch;
            msg_send.direction = randomDirection();
            //print_direction(msg_send.direction);
            msg_send.id = i;
            
            sendto(sock_fd, &msg_send, sizeof(remote_char_t), 0, 
                (const struct sockaddr *) &server_addr, sizeof(server_addr));
        }
    }
    exit(0);
}