#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>  
#include <sys/socket.h>
#include <sys/un.h>
#include <time.h>

#include "chase.h"

// 1 is a client
// 2 is a bot
// 3 is a prize
int map[WINDOW_SIZE][WINDOW_SIZE] = {0};

void new_position(int *x, int *y, direction direction){
    switch (direction)
        {
        case UP:
            (*x) --;
            if(*x ==0)
                *x = 1;
            break;
        case DOWN:
            (*x) ++;
            if(*x ==WINDOW_SIZE-1)
                *x = WINDOW_SIZE-2;
            break;
        case LEFT:
            (*y) --;
            if(*y ==0)
                *y = 1;
            break;
        case RIGHT:
            (*y) ++;
            if(*y ==WINDOW_SIZE-1)
                *y = WINDOW_SIZE-2;
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

void init_data(ch_info_t client_data[], ch_info_t bot_data[], ch_info_t prizes_data[]){
    
    for (int i = 0 ; i < MAX_ARRAY; i++){
        client_data[i].id = -1;
        client_data[i].ch = ' ';
        bot_data[i].id = -1;
        bot_data[i].ch = ' ';
        prizes_data[i].id = -1;
        prizes_data[i].ch = ' ';
    }
}

void init_prizes(ch_info_t prizes_data[], int map[WINDOW_SIZE][WINDOW_SIZE], WINDOW * my_win){

    int i = 0, j = 0;
    while(i < MAX_ARRAY &&  j <5){
        if(prizes_data[i].id==-1){
            prizes_data[i].id = 1;
            prizes_data[i].ch = rand() % 5 + 49;
            prizes_data[i].pos_x = rand() % (WINDOW_SIZE-2) + 1;
            prizes_data[i].pos_y = rand() % (WINDOW_SIZE-2) + 1;
            map[prizes_data[i].pos_x][prizes_data[i].pos_y] = 3;
            j++;
        }
        i++;
    } 
    for(i=0; i<MAX_ARRAY; i++){
        if(prizes_data[i].id != -1){
            wmove(my_win, prizes_data[i].pos_x, prizes_data[i].pos_y);
            waddch(my_win,prizes_data[i].ch| A_BOLD);
            wrefresh(my_win);
        }
    } 
}

void add_prizes(clock_t *begin, ch_info_t prizes_data[], int map[WINDOW_SIZE][WINDOW_SIZE], WINDOW * my_win){

    int pos_x = 0, pos_y = 0;
    int prizes_to_add = 0;
    clock_t end = time(NULL);
    double time_spent = end - *begin;
    if(time_spent > 5){
        int i=0; int j=0;
        prizes_to_add = time_spent / 5;
        while(i<MAX_ARRAY && j<prizes_to_add){
            if(prizes_data[i].id == -1){
                prizes_data[i].id = 1;
                prizes_data[i].ch = rand() % 5 + 49;
                do{
                    pos_x = rand() % (WINDOW_SIZE-2) + 1;
                    pos_y = rand() % (WINDOW_SIZE-2) + 1;
                }while(map[pos_x][pos_y] != 0);
                prizes_data[i].pos_x = pos_x;
                prizes_data[i].pos_y = pos_y;
                //prizes_data[i].pos_x = rand() % (WINDOW_SIZE-2) + 1;
                //prizes_data[i].pos_y = rand() % (WINDOW_SIZE-2) + 1;
                wmove(my_win, prizes_data[i].pos_x, prizes_data[i].pos_y);
                waddch(my_win,prizes_data[i].ch| A_BOLD);
                wrefresh(my_win);
                j++;
                map[prizes_data[i].pos_x][prizes_data[i].pos_y] = 3;
            }
            i++;
        }
        *begin = time(NULL);
    }

}


remote_char_t setupFieldStatusmessage(ch_info_t client_data[], ch_info_t bot_data[],  ch_info_t prizes_data[]){
    
    remote_char_t msg;
    msg.type = 3;
    for (int i = 0 ; i < MAX_ARRAY; i++){
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
    
    return msg;
}

int main(){	
    
    ch_info_t client_data[MAX_ARRAY];
    ch_info_t bot_data[MAX_ARRAY];
    ch_info_t prizes_data[MAX_ARRAY];
    int num_clients = 0, num_bots = 0;
    clock_t begin = time(NULL);
    clock_t end;
    double time_spent;
    int ch;
    int pos_x, old_pos_x;
    int pos_y, old_pos_y;
    int n_bytes;
    int ch_pos=-1;
    int indice;
    int i=0, j=0;
    remote_char_t msg;
    direction  direction;
    struct sockaddr_un client_addr;
    socklen_t client_addr_size = sizeof(struct sockaddr_un);

    srand(time(NULL));

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

    /* Initialization of data structures */
    init_data(client_data, bot_data, prizes_data);

    /* Inicialize prizes*/
    init_prizes(prizes_data, map, my_win);

    while (1){

        /* Add prizes every 5 seconds */
        add_prizes(&begin, prizes_data, map, my_win);

        n_bytes = recvfrom(sock_fd, &msg, sizeof(remote_char_t), 0, 
                        (struct sockaddr *)&client_addr, &client_addr_size);
        if (n_bytes != sizeof(remote_char_t)){
            continue;
        }
        /* Connect Message */
        if(msg.type == 0){
            ch_pos = find_ch_info(client_data, msg.ch);
            mvwprintw(message_win, 3,1,"%c %d", msg.ch, ch_pos);
            if(num_clients < 10 && ch_pos ==-1){
                ch = msg.ch;
                do{
                    pos_x = rand() % (WINDOW_SIZE-2) + 1;
                    pos_y = rand() % (WINDOW_SIZE-2) + 1;
                }while(map[pos_x][pos_y] != 0);
                
                /*Client is not a bot*/
                if(ch != '*'){
                    /* Stores Information */
                    indice = find_free_spot(client_data);                    
                    client_data[indice].id= num_clients;
                    client_data[indice].ch = ch;
                    client_data[indice].pos_x = pos_x;
                    client_data[indice].pos_y = pos_y;
                    client_data[indice].health = 10;
                    map[pos_x][pos_y] = 1;
                    num_clients++;
                    /* draw mark on new position */
                    wmove(my_win, pos_x, pos_y);
                    waddch(my_win,ch| A_BOLD);
                    mvwprintw(message_win, indice+1,1,"%c %d",
                        client_data[indice].ch, client_data[indice].health);

                /*Client is a bot*/
                }else if(ch == '*'){
                    mvwprintw(message_win, 2,1,"Bot Connect");
                    /* Stores Information */
                    indice = find_free_spot(bot_data);

                    bot_data[indice].id = msg.id;
                    bot_data[indice].ch = ch;
                    bot_data[indice].pos_x = pos_x;
                    bot_data[indice].pos_y = pos_y;
                    map[pos_x][pos_y] = 2;
                    num_bots++;
                    /* draw mark on new position */
                    wmove(my_win, pos_x, pos_y);
                    waddch(my_win,ch| A_BOLD);

                }
                /* Send Ball Information Message*/
                msg.type = 1; 
                msg.player_position.x = pos_x;
                msg.player_position.y = pos_y;
                sendto(sock_fd, &msg, sizeof(remote_char_t), 0, 
                        (const struct sockaddr *) &client_addr, client_addr_size);

            }else{
                /* Send Disconnect Message to client*/
                msg.type = 5;
                sendto(sock_fd, &msg, sizeof(remote_char_t), 0, 
                    (const struct sockaddr *) &client_addr, client_addr_size);
            }
        }
        /* Ball Movement message */
        else if(msg.type == 2){
            if(msg.ch != '*'){ // Ball

                ch_pos = find_ch_info(client_data, msg.ch);
                if(ch_pos != -1 && client_data[ch_pos].health > 0){
                    /* Old Position*/
                    old_pos_x = client_data[ch_pos].pos_x;
                    old_pos_y = client_data[ch_pos].pos_y;
                    ch = client_data[ch_pos].ch;

                    /* Calculates new direction */
                    direction = msg.direction;

                    /* Calculates new mark position */  
                    pos_x = old_pos_x;
                    pos_y = old_pos_y;
                    new_position(&pos_x, &pos_y, direction);
                
                    /* Check if new place is free */
                    if (map[pos_x][pos_y] == 1){ // Ball is in new position
                        for(i = 0 ; i < MAX_ARRAY; i++){
                        /* Ball rammed into another ball*/
                            if(client_data[i].pos_x == pos_x && 
                               client_data[i].pos_y == pos_y && client_data[i].ch != ch){
                                if(client_data[i].health > 0)
                                    client_data[i].health--;
                                if(client_data[ch_pos].health < 10)
                                    client_data[ch_pos].health++;
                                // Print info about ball rammed into
                                mvwprintw(message_win, i+1,1,"%c %d ", 
                                    client_data[i].ch, client_data[i].health);
                            }
                        }
                    }else if(map[pos_x][pos_y] == 2){ // Bot is in new position
                        // Nothing happens
                    }else if(map[pos_x][pos_y] == 3){ // Prize is in new position
                        for(i = 0 ; i < MAX_ARRAY; i++){
                            if(prizes_data[i].pos_x == pos_x && 
                               prizes_data[i].pos_y == pos_y){
                                client_data[ch_pos].health += (prizes_data[i].ch-48);
                                remove_from_game(prizes_data, i);
                                map[pos_x][pos_y] = 1;
                                if (client_data[ch_pos].health > 10)
                                    client_data[ch_pos].health = 10;
                            }
                        }
                        /* Delete old place*/
                        map[old_pos_x][old_pos_y] = 0;
                        wmove(my_win, old_pos_x, old_pos_y);
                        waddch(my_win,' ');
                        /*Update new position*/
                        client_data[ch_pos].pos_x = pos_x;
                        client_data[ch_pos].pos_y = pos_y;
                        map[pos_x][pos_y] = 1;
                        wmove(my_win, pos_x, pos_y);
                        waddch(my_win,ch| A_BOLD);
                    }
                    else{ // New position is free

                        /* Delete old place*/
                        map[old_pos_x][old_pos_y] = 0;
                        wmove(my_win, old_pos_x, old_pos_y);
                        waddch(my_win,' ');
                        /*Update new position*/
                        client_data[ch_pos].pos_x = pos_x;
                        client_data[ch_pos].pos_y = pos_y;
                        map[pos_x][pos_y] = 1;
                        wmove(my_win, pos_x, pos_y);
                        waddch(my_win,ch| A_BOLD);
                    }

                    /* Send Field Status Message*/
                    msg = setupFieldStatusmessage(client_data, bot_data, prizes_data);
                    sendto(sock_fd, &msg, sizeof(remote_char_t), 0, 
                        (const struct sockaddr *) &client_addr, client_addr_size);

                    mvwprintw(message_win, ch_pos+1,1,"%c %d ", 
                        client_data[ch_pos].ch, client_data[ch_pos].health);
                }
                /* Send Health 0 Message*/
                else if(ch_pos != -1 && client_data[ch_pos].health < 1 ){
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
            else if(msg.ch == '*'){
                ch = msg.ch;
                ch_pos = msg.id;

                /* Old Position*/
                old_pos_x = bot_data[ch_pos].pos_x;
                old_pos_y = bot_data[ch_pos].pos_y;
             
                /* Calculates new direction */
                direction = msg.direction;

                /* Calculates new mark position */
                pos_x = old_pos_x;
                pos_y = old_pos_y;
                new_position(&pos_x, &pos_y, direction);
                

                /* Check if new place is free */
                if (map[pos_x][pos_y] == 1){ // Ball is in new position
                    for(i = 0 ; i < MAX_ARRAY; i++){
                    /* Bot rammed into another ball*/
                        if(client_data[i].pos_x == pos_x && 
                           client_data[i].pos_y == pos_y && client_data[i].ch != ch){
                            if(client_data[i].health > 0)
                                client_data[i].health--;
                            // Print info about ball rammed into
                            mvwprintw(message_win, i+1,1,"%c %d ", 
                            client_data[i].ch, client_data[i].health);
                        }
                    }
                }else if(map[pos_x][pos_y] == 2){ // Bot is in new position
                    // Nothing happens
                
                }else if(map[pos_x][pos_y] == 3){ // Prize is in new position
                    // Nothing happens
                }else{ // New position is completely free
                // int y=0;
                //     if(msg.direction== UP)
                //         y =1;
                //     else if(msg.direction == DOWN)
                //         y=2;
                //     else if(msg.direction == LEFT)
                //         y=3;
                //     else if(msg.direction == RIGHT)
                //         y=4;

                    wrefresh(message_win);

                    /* Delete old place*/
                    map[old_pos_x][old_pos_y] = 0;
                    wmove(my_win, old_pos_x, old_pos_y);
                    waddch(my_win,' ');

                    /* Update new position */
                    bot_data[ch_pos].pos_x = pos_x;
                    bot_data[ch_pos].pos_y = pos_y;
                    map[pos_x][pos_y] = 2;
                    wmove(my_win, pos_x, pos_y);
                    waddch(my_win,ch| A_BOLD);

                }

                wrefresh(my_win);
            }
        }
        /* Disconnect Message*/
        else if(msg.type == 5){
            ch_pos = find_ch_info(client_data, msg.ch);
            if(ch_pos != -1){
                pos_x = client_data[ch_pos].pos_x;
                pos_y = client_data[ch_pos].pos_y;
                ch = client_data[ch_pos].ch;
                map[pos_x][pos_y] = 0;
                
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