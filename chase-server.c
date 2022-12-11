#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include "chase.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>  
#include <sys/socket.h>
#include <sys/un.h>
#include <time.h>

void new_position(int *x, int *y, direction direction){
    switch (direction)
        {
        case UP:
            (*x) --;
            if(*x ==0)
                *x = 2;
            break;
        case DOWN:
            (*x) ++;
            if(*x ==WINDOW_SIZE-1)
                *x = WINDOW_SIZE-3;
            break;
        case LEFT:
            (*y) --;
            if(*y ==0)
                *y = 2;
            break;
        case RIGHT:
            (*y) ++;
            if(*y ==WINDOW_SIZE-1)
                *y = WINDOW_SIZE-3;
            break;
        default:
            break;
        }
}

int find_ch_info(ch_info_t client_data[], int ch){
    for (int i = 0 ; i < MAX_ARRAY; i++){
        if(client_data[i].ch == ch){
            return i;
        }
    }
    return -1;
}

int find_free_spot(ch_info_t char_data[]){
    for (int i = 0 ; i < MAX_ARRAY; i++){
        if(char_data[i].id == -1){
            return i;
        }
    }
    return -1;
}

void remove_from_game(ch_info_t char_data[], int i){
    char_data[i].id= -1;
    char_data[i].ch = ' ';
    char_data[i].pos_x = 0;
    char_data[i].pos_y = 0;
}

int main(){	
    
    ch_info_t client_data[MAX_ARRAY];
    ch_info_t bot_data[MAX_ARRAY];
    ch_info_t prizes_data[MAX_ARRAY];
    int num_clients = 0, num_bots = 0;
    clock_t begin = clock();
    clock_t end;
    double time_spent;

    srand(time(NULL));

    for (int i = 0 ; i < MAX_ARRAY; i++){
        client_data[i].id = -1;
        client_data[i].ch = ' ';
        bot_data[i].id = -1;
        bot_data[i].ch = ' ';
        prizes_data[i].id = -1;
        prizes_data[i].ch = ' ';
    }

    /* Open and link socket */
    int sock_fd;
    sock_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sock_fd == -1){
	    perror("socket: ");
	    exit(-1);
    }
    struct sockaddr_un local_addr;
    local_addr.sun_family = AF_UNIX;
    strcpy(local_addr.sun_path, SOCKET_NAME);

    unlink(SOCKET_NAME);
    int err = bind(sock_fd, 
            (const struct sockaddr *)&local_addr, sizeof(local_addr));
    if(err == -1) {
	    perror("bind");
	    exit(-1);
    }


	initscr();		    	/* Start curses mode 		*/
	cbreak();				/* Line buffering disabled	*/
    keypad(stdscr, TRUE);   /* We get F1, F2 etc..		*/
	noecho();			    /* Don't echo() while we do getch */


    /* Creates a window and draws a border */
    WINDOW * my_win = newwin(WINDOW_SIZE, WINDOW_SIZE, 0, 0);
    box(my_win, 0 , 0);	
	wrefresh(my_win);
    /* Creates a window for health of players and draws a border  */
    WINDOW * message_win = newwin(5, WINDOW_SIZE, WINDOW_SIZE, 0);
    box(message_win, 0 , 0);	
	wrefresh(message_win);

    /*inicialize prizes*/
    int i=0, j=0;
    while(i<MAX_ARRAY && j<5){
        if(prizes_data[i].id==-1){
            prizes_data[i].id = 1;
            prizes_data[i].ch = rand() % 5 + 49;
            prizes_data[i].pos_x = rand() % (WINDOW_SIZE-2) + 1;
            prizes_data[i].pos_y = rand() % (WINDOW_SIZE-2) + 1;
            j++;
            //mvwprintw(my_win, i+1,1,"%d %d %d ", prizes_data[i].ch, prizes_data[i].pos_x, prizes_data[i].pos_y);
        }
        i++;
    } 
    for(i=0; i<MAX_ARRAY; i++){
        if(prizes_data[i].id!=-1){
            wmove(my_win, prizes_data[i].pos_x, prizes_data[i].pos_y);
            waddch(my_win,prizes_data[i].ch| A_BOLD);
            wrefresh(my_win);
        }
    } 


    int ch;
    int pos_x;
    int pos_y;
    int n_bytes;
    int ch_pos;
    int indice;
    remote_char_t msg;

    direction  direction;
    struct sockaddr_un client_addr;
    socklen_t client_addr_size = sizeof(struct sockaddr_un);

    while (1){

        //EVETY 5SEC PUT ONE MORE PRIZE : NOT WORKING I DONT KNOW WHY, THE CLOCK_PER_SEC
        /*end = clock();
        time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
        mvwprintw(my_win, 2,1,"here:%ld %ld %ld", begin, end, (end-begin)/CLOCKS_PER_SEC);
        if(time_spent > 5){
            i=0; j=0;
            while(i<MAX_ARRAY && j<1){
                if(prizes_data[i].id==-1){
                    prizes_data[i].id = 1;
                    prizes_data[i].ch = rand() % 5 + 49;
                    prizes_data[i].pos_x = rand() % (WINDOW_SIZE-2) + 1;
                    prizes_data[i].pos_y = rand() % (WINDOW_SIZE-2) + 1;
                    wmove(my_win, prizes_data[i].pos_x, prizes_data[i].pos_y);
                    waddch(my_win,prizes_data[i].ch| A_BOLD);
                    wrefresh(my_win);
                    j++;
                }
                i++;
            }
            begin = clock();
        }*/

        n_bytes = recvfrom(sock_fd, &msg, sizeof(remote_char_t), 0, 
                        (struct sockaddr *)&client_addr, &client_addr_size);
        if (n_bytes != sizeof(remote_char_t)){
            continue;
        }
        /* Connect Message */
        if(msg.type == 0){
            if(num_clients <= 10){
                ch = msg.ch;
                pos_x = WINDOW_SIZE/2;
                pos_y = WINDOW_SIZE/2;

                /*Client is not a bot*/
                if(ch != '*'){
                    /* Stores Information */
                    indice = find_free_spot(client_data);
                    
                    //if (indice == -1){}
                        /* Quando já há 10 clientes, decidir o que fazer*/
                    
                    client_data[indice].id= num_clients;
                    client_data[indice].ch = ch;
                    client_data[indice].pos_x = pos_x;
                    client_data[indice].pos_y = pos_y;
                    client_data[indice].health = 10;
                    num_clients++;
                    /* draw mark on new position */
                    wmove(my_win, pos_x, pos_y);
                    waddch(my_win,ch| A_BOLD);
                    mvwprintw(message_win, indice+1,1,"%c %d", client_data[indice].ch, client_data[indice].health);

                /*Client is a bot*/
                }else if(ch == '*'){
                    /* Stores Information */
                    indice = find_free_spot(bot_data);
                    //if (indice == -1){}
                        /* Quando já há 10 clientes, decidir o que fazer*/

                    bot_data[indice].id = num_bots;
                    bot_data[indice].ch = ch;
                    bot_data[indice].pos_x = pos_x;
                    bot_data[indice].pos_y = pos_y;
                    num_bots++;
                    /* draw mark on new position */
                    wmove(my_win, pos_x, pos_y);
                    waddch(my_win,ch| A_BOLD);

                }
                /* Send Ball Information Message*/
                msg.type = 1; 
                sendto(sock_fd, &msg, sizeof(remote_char_t), 0, 
                        (const struct sockaddr *) &client_addr, client_addr_size);
            }
        }
        /* Ball Movement message */
        else if(msg.type == 2){
            ch_pos = find_ch_info(client_data, msg.ch);
            if(ch_pos != -1 && client_data[ch_pos].health > 0){
                pos_x = client_data[ch_pos].pos_x;
                pos_y = client_data[ch_pos].pos_y;
                ch = client_data[ch_pos].ch;
                /*deletes old place */
                wmove(my_win, pos_x, pos_y);
                waddch(my_win,' ');

                /* Calculates new direction */
                direction = msg.direction;

                /* Calculates new mark position */
                new_position(&pos_x, &pos_y, direction);
                client_data[ch_pos].pos_x = pos_x;
                client_data[ch_pos].pos_y = pos_y;
                //mvwprintw(my_win, 1,1,"HERE %d, %d\n", client_data[ch_pos].pos_y, client_data[ch_pos].pos_x);

                /* Check if a ball moved onto another object */
                for(i = 0 ; i < MAX_ARRAY; i++){
                    /* Ball rammed into another ball*/
                    if(client_data[i].pos_x == pos_x && client_data[i].pos_y == pos_y && client_data[i].ch != ch){
                        if(client_data[i].health > 0)
                            client_data[i].health--;
                        if(client_data[ch_pos].health < 10)
                            client_data[ch_pos].health++;
                        mvwprintw(message_win, i+1,1,"%c %d ", client_data[i].ch, client_data[i].health);
                    }
                    /* Bot rammed into a bot */
                        // TO DO
                    /* Ball rammed into a prize */
                    if(prizes_data[i].pos_x == pos_x && prizes_data[i].pos_y == pos_y){
                        client_data[ch_pos].health += (prizes_data[i].ch-48);
                        remove_from_game(prizes_data, i);
                        if (client_data[ch_pos].health > 10)
                            client_data[ch_pos].health = 10;
                    }
                }


                /* Send Field Status Message*/
                msg.type = 3;
                for (i = 0 ; i < MAX_ARRAY; i++){
                    msg.clients[i].id = client_data[i].id;
                    msg.clients[i].ch = client_data[i].ch;
                    msg.clients[i].pos_x = client_data[i].pos_x;
                    msg.clients[i].pos_y = client_data[i].pos_y;
                    msg.clients[i].health = client_data[i].health;

                    msg.bots[i].id = bot_data[i].id;
                    msg.bots[i].ch = bot_data[i].ch;
                    msg.bots[i].pos_x = bot_data[i].pos_x;
                    msg.bots[i].pos_y = bot_data[i].pos_y;
                    msg.bots[i].health = bot_data[i].health;

                    msg.prizes[i].id = prizes_data[i].id;
                    msg.prizes[i].ch = prizes_data[i].ch;
                    msg.prizes[i].pos_x = prizes_data[i].pos_x;
                    msg.prizes[i].pos_y = prizes_data[i].pos_y;
                    msg.prizes[i].health = prizes_data[i].health;
                }

                //mvwprintw(my_win, 3,1,"BLA %d, %d\n", msg.clients[0].pos_y, msg.clients[0].pos_x);
                sendto(sock_fd, &msg, sizeof(remote_char_t), 0, 
                    (const struct sockaddr *) &client_addr, client_addr_size);

                wmove(my_win, pos_x, pos_y);
                waddch(my_win,ch| A_BOLD);
                mvwprintw(message_win, ch_pos+1,1,"%c %d ", client_data[ch_pos].ch, client_data[ch_pos].health);
            }
            /* Send Health 0 Message*/
            else if(client_data[ch_pos].health == 0){
                /* Send Healt 0 Message to client*/
                msg.type = 4;
                sendto(sock_fd, &msg, sizeof(remote_char_t), 0, 
                    (const struct sockaddr *) &client_addr, client_addr_size);
                /* Clean up array in server */
                pos_x = client_data[ch_pos].pos_x;
                pos_y = client_data[ch_pos].pos_y;
                ch = client_data[ch_pos].ch;
                remove_from_game(client_data, ch_pos);
                num_clients--;
                /* Deletes player from screen */
                wmove(my_win, pos_x, pos_y);
                waddch(my_win,' ');
                mvwprintw(message_win, ch_pos+1,1,"         ");
            }
            
        }
        /* Disconnect Message*/
        else if(msg.type == 5){
            ch_pos = find_ch_info(client_data, msg.ch);
            if(ch_pos != -1){
                pos_x = client_data[ch_pos].pos_x;
                pos_y = client_data[ch_pos].pos_y;
                ch = client_data[ch_pos].ch;
                /*deletes old place */
                wmove(my_win, pos_x, pos_y);
                waddch(my_win,' ');
                wrefresh(my_win);
                mvwprintw(message_win, ch_pos+1,1,"         ");
                remove_from_game(client_data, ch_pos);
                num_clients--;
            }
        }
        wrefresh(my_win);
        wrefresh(message_win);
    }
  	endwin();			/* End curses mode*/

	return 0;
}