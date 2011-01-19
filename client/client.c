/*
 *	client.c
 *	
 * 	By Shawn Dooley
 *
 *
 *
 *	This program is designed to serve as the control input for a
 *	arduino controlled toy car. It uses the arrow keys, and spacebar
 *	to move and stop. press escape or 'q' to exit the program.
 *
 */


#include <arpa/inet.h>
#include <ctype.h>
#include <ncurses.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "client.h"



//String that we send to the WiShield

char buf[MAXDATASIZE];      


//Storage for teh last info sent to the WiShield

char lastSent[MAXDATASIZE];	
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
		
		if (ch == ' ')	 // Check for a stop command first.
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

/*
I store a copy of the last sent string in a variable named lastSent. 
before sending the next string I compare the new string to this.
If they match, I skip sending the new one.
I found this fixed some of the delay that resulted from key mashing that 
people are inclined to do
*/
				memcpy(lastSent, buf, MAXDATASIZE);
  			
			}
		}
 

 		// Loop Until the escape key  or 'Q' is pressed.  
	} while(ch != 27 && ch != 'Q' );  

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
	
	build_string();

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
	


	build_string();

	


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
	

	
	build_string();


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
	

	
	build_string();

}


void build_string(void)
{

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


	
