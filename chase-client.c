#include <stdlib.h>
#include <ncurses.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include "chase.h"

WINDOW * message_win;

void new_player (player_position_t * player, char c, int pos_x,  int pos_y){
    player->x = pos_x;
    player->y = pos_y;
    player->c = c;
}

void draw_player(WINDOW *win, player_position_t * player, int delete){
    int ch;
    if(delete){
        ch = player->c;
    }else{
        ch = ' ';
    }
    int p_x = player->x;
    int p_y = player->y;
    wmove(win, p_x, p_y);
    waddch(win,ch);
    wrefresh(win);
}

void move_player (player_position_t * player, int direction){
    if (direction == KEY_UP){
        if (player->x  != 1){
            player->x --;
        }
    }
    if (direction == KEY_DOWN){
        if (player->x  != WINDOW_SIZE-2){
            player->x ++;
        }
    }
    if (direction == KEY_LEFT){
        if (player->y  != 1){
            player->y --;
        }
    }
    if (direction == KEY_RIGHT)
        if (player->y  != WINDOW_SIZE-2){
            player->y ++;
    }
}

direction find_direction(int key){
    
    direction i;
    switch (key){
        case KEY_UP:
            i = UP;
            break;
        case KEY_DOWN:
            i = DOWN;
            break;
        case KEY_LEFT:
            i = LEFT;
            break;
        case KEY_RIGHT:
            i = RIGHT;
            break;
    }

    return i;
}   

player_position_t p1;

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


    /* Send connect message*/
    char ch;
    remote_char_t msg_send, msg_rcv;

    do{
        printf("What is your character (a...z)?: ");
        ch = getchar();
        ch = tolower(ch);  
    }while(!isalpha(ch));

    /* Send Connection Message*/
    msg_send.type = 0; 
    msg_send.ch = ch;    

    sendto(sock_fd, &msg_send, sizeof(remote_char_t), 0, 
            (const struct sockaddr *)&server_addr, sizeof(server_addr));

    /* Receives Ball Information */       
    do{
        recv(sock_fd, &msg_rcv, sizeof(remote_char_t), 0);
        if(msg_rcv.type == 5){
            printf("\n Received disconnect.\n\n");
            exit(0);
        }
    }while(msg_rcv.type != 1);


	initscr();		    	/* Start curses mode 		*/
	cbreak();				/* Line buffering disabled	*/
    keypad(stdscr, TRUE);   /* We get F1, F2 etc..		*/
	noecho();			    /* Don't echo() while we do getch */


    /* Creates a window and draws a border */
    WINDOW * my_win = newwin(WINDOW_SIZE, WINDOW_SIZE, 0, 0);
    box(my_win, 0 , 0);	
	wrefresh(my_win);
    keypad(my_win, true);
    /* Creates a window for health of players and draws a border  */
    message_win = newwin(5, WINDOW_SIZE, WINDOW_SIZE, 0);
    box(message_win, 0 , 0);	
	wrefresh(message_win);
    


    new_player(&p1, ch, msg_rcv.player_position.x, msg_rcv.player_position.y);
    draw_player(my_win, &p1, true);
    // mvwprintw(message_win, 4,1,"%d %d", p1.x, p1.y);
    // wrefresh(message_win);

    int key = -1;
    while(key != 27 && key!= 'q'){
        key = wgetch(my_win);	
        if (key == KEY_LEFT || key == KEY_RIGHT || key == KEY_UP || key == KEY_DOWN){
            
            /* Sends Ball Movement mesage*/
            msg_send.type = 2;
            msg_send.ch = ch;
            msg_send.direction = find_direction(key);
            sendto(sock_fd, &msg_send, sizeof(remote_char_t), 0, 
                (const struct sockaddr *)&server_addr, sizeof(server_addr));
            recv(sock_fd, &msg_rcv, sizeof(remote_char_t), 0);
            if(msg_rcv.type == 3){
                wclear(my_win);
                box(my_win, 0 , 0);
                wrefresh(message_win);
                wclear(message_win);
                box(message_win, 0 , 0);
                wrefresh(message_win);
                for(int i = 0 ; i < MAX_ARRAY; i++){
                    if(msg_rcv.clients[i].id !=-1 && msg_rcv.clients[i].ch!=ch){
                        wmove(my_win, msg_rcv.clients[i].pos_x, msg_rcv.clients[i].pos_y);
                        waddch(my_win,msg_rcv.clients[i].ch);
                        mvwprintw(message_win, i+2,1,"%c %d ", msg_rcv.clients[i].ch, msg_rcv.clients[i].health);
                        wrefresh(my_win);
                    }else if(msg_rcv.clients[i].ch==ch){
                        // p1.x=msg_rcv.clients[i].pos_x;
                        // p1.y=msg_rcv.clients[i].pos_y;
                        
                        wmove(my_win, msg_rcv.clients[i].pos_x, msg_rcv.clients[i].pos_y);
                        waddch(my_win,msg_rcv.clients[i].ch);
                        mvwprintw(message_win, 1,1,"%c %d ", msg_rcv.clients[i].ch, msg_rcv.clients[i].health);
                        wrefresh(my_win);
                    }
                    if(msg_rcv.bots[i].id !=-1){
                        wmove(my_win, msg_rcv.bots[i].pos_x, msg_rcv.bots[i].pos_y);
                        waddch(my_win,msg_rcv.bots[i].ch);
                        wrefresh(my_win);
                    }
                    if(msg_rcv.prizes[i].id !=-1){
                        wmove(my_win, msg_rcv.prizes[i].pos_x, msg_rcv.prizes[i].pos_y);
                        waddch(my_win,msg_rcv.prizes[i].ch);
                        wrefresh(my_win);
                    }
                }
            }else if(msg_rcv.type == 4){
                endwin();
                printf("\n You have reached 0 Health.\n\n");
                break;
            }
        }

        /* Draw new position*/
        // draw_player(my_win, &p1, false);
        // move_player (&p1, key);
        // draw_player(my_win, &p1, true);
        wrefresh(message_win);	
    }
    if(key == 'q'){
        msg_send.type = 5;
        sendto(sock_fd, &msg_send, sizeof(remote_char_t), 0, 
            (const struct sockaddr *)&server_addr, sizeof(server_addr));
        endwin();
    }

    exit(0);
}