#include <stdlib.h>
#include <ncurses.h>
#include "chase.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>  
#include <sys/socket.h>
#include <sys/un.h>

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

void remove_client(ch_info_t char_data[], int i){
    char_data[i].id= -1;
    char_data[i].ch = ' ';
    char_data[i].pos_x = 0;
    char_data[i].pos_y = 0;
}

int main()
{	
    
    ch_info_t client_data[MAX_ARRAY];
    ch_info_t bot_data[MAX_ARRAY];
    ch_info_t prizes_data[MAX_ARRAY];
    int num_clients = 0, num_bots = 0;

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


    /* creates a window and draws a border */
    WINDOW * my_win = newwin(WINDOW_SIZE, WINDOW_SIZE, 0, 0);
    box(my_win, 0 , 0);	
	wrefresh(my_win);


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

    while (1)
    {

        n_bytes = recvfrom(sock_fd, &msg, sizeof(remote_char_t), 0, 
                        (struct sockaddr *)&client_addr, &client_addr_size);
        if (n_bytes!= sizeof(remote_char_t)){
            continue;
        }
        /* Connect Message */
        if(msg.type == 0){
            ch = msg.ch;
            pos_x = WINDOW_SIZE/2;
            pos_y = WINDOW_SIZE/2;
            /*Client is not a bot*/
            if(ch!='*'){
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

            /*Client is a bot*/
            }else if(ch=='*'){
                indice = find_free_spot(bot_data);
                //if (indice == -1){}
                    /* Quando já há 10 clientes, decidir o que fazer*/

                bot_data[indice].id= num_bots;
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
        else if(msg.type == 2){
            ch_pos = find_ch_info(client_data, msg.ch);
            if(ch_pos != -1){
                pos_x = client_data[ch_pos].pos_x;
                pos_y = client_data[ch_pos].pos_y;
                ch = client_data[ch_pos].ch;
                /*deletes old place */
                wmove(my_win, pos_x, pos_y);
                waddch(my_win,' ');

                /* calculates new direction */
                direction = msg.direction;

                /* calculates new mark position */
                new_position(&pos_x, &pos_y, direction);
                client_data[ch_pos].pos_x = pos_x;
                client_data[ch_pos].pos_y = pos_y;
                //mvwprintw(my_win, 1,1,"HERE %d, %d\n", client_data[ch_pos].pos_y, client_data[ch_pos].pos_x);

                /* Send Field Status Message*/
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

                //mvwprintw(my_win, 3,1,"BLA %d, %d\n", msg.clients[0].pos_y, msg.clients[0].pos_x);
                sendto(sock_fd, &msg, sizeof(remote_char_t), 0, 
                    (const struct sockaddr *) &client_addr, client_addr_size);
            }
            wmove(my_win, pos_x, pos_y);
            waddch(my_win,ch| A_BOLD);	
            
        }else if(msg.type == 5){
            ch_pos = find_ch_info(client_data, msg.ch);
            if(ch_pos != -1){
                pos_x = client_data[ch_pos].pos_x;
                pos_y = client_data[ch_pos].pos_y;
                ch = client_data[ch_pos].ch;
                /*deletes old place */
                wmove(my_win, pos_y, pos_x);
                waddch(my_win,' ');
                wrefresh(my_win);
                remove_client(client_data, ch_pos);
                num_clients--;
            }
        }	
        wrefresh(my_win);	
    }
  	endwin();			/* End curses mode*/

	return 0;
}