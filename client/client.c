// client.c
// A stream socket client demo

#include <arpa/inet.h>
#include <ctype.h>
#include <ncurses.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//Prototypes
void adjust_motor_speed(int keypress);  //Obsolete, being removed.

// Functions to determine vehicle behavior based on its current state.
void stopped_keys(int keypress);
void reverse_keys(int keypress);
void forward_keys(int keypress);
void opposite_direction_keys(int keypress);






// Function to make the the vehicle turn in place by activating the motors
// in opposite directions.



#define PORT 1000    // the port WiShield is listening on
#define REVERSE 32

#define MAXDATASIZE 3   // max number of bytes we can get at once from WiShield

					
#define STEPSIZE 5		// A larger number here makes the car speed up faster
						//Default is 5.



	    char buf[MAXDATASIZE];      //String that we send to the WiShield
		char lastSent[MAXDATASIZE];	//Storage for teh last info sent to the WiShield
		char lmSpeed = 65;			
		char rmSpeed = 65;
		
		// flags for the current car state.
		bool moving = FALSE;
		bool reversed = FALSE;
		bool right_reversed = FALSE;
		bool left_reversed = FALSE;


int main( void )
{
		// Variables used for socket.	
	int ch;
    int sockfd;
    struct sockaddr_in srv;
	
	
    memset(&srv, 0, sizeof(srv));

    // setup IP address of WiShield
    srv.sin_family = AF_INET;
    srv.sin_port = htons(PORT);
/*******************Set target IP HERE! **************************/    
	inet_pton(AF_INET, "192.168.1.10", &srv.sin_addr);

    // setup socket and connect
    if ((sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        perror("socket");
        exit(0);
    }

    if (connect(sockfd, (const void*)&srv, sizeof(srv)) == -1) {
        close(sockfd);
        perror("connect");
        exit(0);
    }

		
	//setup curses
	initscr();
	noecho();
    keypad(stdscr, TRUE);

	printw("connected to WiShield\n");
		
//Primary loop. ************************************************************
	do
	{
		noecho();
		
		if(lmSpeed == 65 && rmSpeed == 65)
		{
			reversed = FALSE;			
			moving = FALSE;
			right_reversed = FALSE;
			left_reversed = FALSE;
		}
		else
		{
			moving = TRUE;
		
			if( left_reversed && right_reversed)
			{
				reversed = TRUE;
			}
			else
			{
				reversed = FALSE;			
			}
		}
		
		  // Get input from user
		ch = getch();
		ch = toupper( ch );
		
			//Determine whick method to process the keys based on current state.
		
		if (ch == ' ')	
		{
			lmSpeed = 65;
			rmSpeed = 65;
			moving = FALSE;
			reversed = FALSE;
		
			buf[0] = 'A';
			buf[1] = 'A';	
			buf[2] = '\0';
		}
		else if(!moving)
			stopped_keys(ch);
		else if(reversed)
			reverse_keys(ch);
		else if(left_reversed != right_reversed)
			opposite_direction_keys(ch);
		else
			forward_keys(ch);


			// Only send a string to the car if it is different than the last one sent.
		if( strcmp(buf, lastSent))
		{		
    
				// Send user response
	    	if (send(sockfd, buf, MAXDATASIZE, 0) == -1) 
			{
	    	 	perror("send");
      			close(sockfd);
      			exit(1);
    		}
			else
			{

					//store copy of last sent string to reduce overflow	from button mashing
				memcpy(lastSent, buf, MAXDATASIZE);
  			
			}
		}
 

 		// Loop Until the escape key is pressed.  
	} while(ch != 27  );  

		// Send one last string to stop the car.	  
	if (send(sockfd, "AA\0" , MAXDATASIZE, 0) == -1) 
	{
    perror("send");
    close(sockfd);
    exit(1);
  }


	//Tidy up.
	close(sockfd);
	endwin();
  	return 0;
}  //End of main.



void stopped_keys(int keypress)
{
	switch(keypress)
	{
		case KEY_UP:
			reversed = FALSE;				
			left_reversed = FALSE;
			right_reversed = FALSE;	
		
			lmSpeed += STEPSIZE;
			rmSpeed += STEPSIZE;
			break;
				
		case KEY_DOWN:
			reversed = TRUE;				
			left_reversed = TRUE;
			right_reversed = TRUE;
			lmSpeed += STEPSIZE;
			rmSpeed += STEPSIZE;
			break;
						
		case KEY_LEFT:
			reversed = FALSE;			
			left_reversed = TRUE;
			right_reversed = FALSE;
			lmSpeed += STEPSIZE;
			rmSpeed += STEPSIZE;
			break;		
		
		case KEY_RIGHT:
			reversed = FALSE;			
			left_reversed = FALSE;
			left_reversed = TRUE;
			lmSpeed += STEPSIZE;
			rmSpeed += STEPSIZE;
			break;		
		
	}
	
	//Build string to send.
	if(left_reversed)
		buf[0] = lmSpeed + REVERSE;
	else
		buf[0] = lmSpeed;

	if(right_reversed)
		buf[1] = rmSpeed + REVERSE;
	else
		buf[1] = rmSpeed;
	
	buf[2] = '\0';

}


void reverse_keys(int keypress)
{

	switch(keypress)
	{
		case KEY_UP:
			lmSpeed -= STEPSIZE;
			rmSpeed -= STEPSIZE;
			break;
				
		case KEY_DOWN:
			lmSpeed += STEPSIZE;
			rmSpeed += STEPSIZE;
			break;
						
		case KEY_LEFT:
			lmSpeed -= STEPSIZE;
			rmSpeed += STEPSIZE;
			break;		
		
		case KEY_RIGHT:
			lmSpeed += STEPSIZE;
			rmSpeed -= STEPSIZE;
			break;		
		
	}

	if( lmSpeed <= 65)
	{
		reversed = FALSE;
		left_reversed = FALSE;
		lmSpeed = 65;
	}
	if( rmSpeed <= 65)
	{
		reversed = FALSE;
		right_reversed = FALSE;
		rmSpeed = 65;
	}
	
	if( lmSpeed >= 90)
		lmSpeed = 90;
	if( rmSpeed >= 90)
		rmSpeed = 90;
	

	
	//Build string to send.
	if(left_reversed)
		buf[0] = lmSpeed + REVERSE;
	else
		buf[0] = lmSpeed;

	if(right_reversed)
		buf[1] = rmSpeed + REVERSE;
	else
		buf[1] = rmSpeed;
	
	buf[2] = '\0';


}


void forward_keys(int keypress)
{

	switch(keypress)
	{
		case KEY_UP:
			lmSpeed += STEPSIZE;
			rmSpeed += STEPSIZE;
			break;
				
		case KEY_DOWN:
			lmSpeed -= STEPSIZE;
			rmSpeed -= STEPSIZE;
			break;
						
		case KEY_LEFT:
			lmSpeed -= STEPSIZE;
			rmSpeed += STEPSIZE;
			break;
	
		case KEY_RIGHT:
			lmSpeed += STEPSIZE;
			rmSpeed -= STEPSIZE;
			break;		
	}

	if( lmSpeed <= 65)
		lmSpeed = 65;
	if( rmSpeed <= 65)
		rmSpeed = 65;
	
	if( lmSpeed >= 90)
		lmSpeed = 90;
	if( rmSpeed >= 90)
		rmSpeed = 90;
	

	
	//Build string to send.
	if(left_reversed)
		buf[0] = lmSpeed + REVERSE;
	else
		buf[0] = lmSpeed;

	if(right_reversed)
		buf[1] = rmSpeed + REVERSE;
	else
		buf[1] = rmSpeed;
	
	buf[2] = '\0';


}


void opposite_direction_keys(int keypress)
{

	switch(keypress)
	{
		case KEY_UP:
			lmSpeed += STEPSIZE;
			rmSpeed += STEPSIZE;
			break;
				
		case KEY_DOWN:
			lmSpeed -= STEPSIZE;
			rmSpeed -= STEPSIZE;
			break;
						
		case KEY_LEFT:
			lmSpeed -= STEPSIZE;
			rmSpeed += STEPSIZE;
			break;		
		
		case KEY_RIGHT:
			lmSpeed += STEPSIZE;
			rmSpeed -= STEPSIZE;
			break;		
		
	}

	if( lmSpeed <= 65)
		lmSpeed = 65;
	if( rmSpeed <= 65)
		rmSpeed = 65;
	
	if( lmSpeed >= 90)
		lmSpeed = 90;
	if( rmSpeed >= 90)
		rmSpeed = 90;
	

	
	//Build string to send.
	if(left_reversed)
		buf[0] = lmSpeed + REVERSE;
	else
		buf[0] = lmSpeed;

	if(right_reversed)
		buf[1] = rmSpeed + REVERSE;
	else
		buf[1] = rmSpeed;
	
	buf[2] = '\0';


}


	
