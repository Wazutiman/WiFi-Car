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

//roushstg3@yahoo.com

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


int main( int argc, char *argv[])
{

	/**
	Make the IP address string large enough to handle any 
	standard IPv4 address.
	set the standdard port to the one defined in the header
	*/
	
	char ip_address[16];
	int port = PORT;
	
	
	//	Set the standard IP address to the one defined in the header.
	
	strcpy(ip_address, IP_ADDRESS);

	/* Make sure there are the correct number of arguements*/
	
	if( argc > 3)
	{	
		error( "Wrong number of arguements.");

		exit (2);
		
	}
	if( argc > 1 && !validIP( argv[1])  )
	{
		error( "Invalid IP format.");
		exit(3);
	}
	else
	{
		strcpy( ip_address, argv[1]);
	}
	
	if(argc == 3 )
	{
		if( atoi(argv[2]) > 65535  || atoi(argv[2]) <= 0 )
		{
			error("Invalid port.");
			exit(4);
		}
		else
		{
			port = atoi(argv[2]);
		}
	}

		// Variables used for socket.	
		int ch;
		int sockfd;
		struct sockaddr_in srv;
		
		
		memset(&srv, 0, sizeof(srv));

		// setup IP address of WiShield
		srv.sin_family = AF_INET;
		srv.sin_port = htons(PORT);


		inet_pton(AF_INET, ip_address, &srv.sin_addr);

    // setup socket and connect
    	if ((sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) 
		{
     	   perror("socket");
     	   exit(0);
    	}

    	if (connect(sockfd, (const void*)&srv, sizeof(srv)) == -1) 
		{
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
				reversed = TRUE;
			else
				reversed = FALSE;			
			
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
			strcpy(buf, "AA");
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

/*



*/

bool validIP(char *ip_add)
{
	unsigned b1, b2, b3, b4;
	unsigned char c;

	if (sscanf(ip_add, "%3u.%3u.%3u.%3u%c", &b1, &b2, &b3, &b4, &c) != 4)
		return false;

	if ((b1 | b2 | b3 | b4) > 255) 
		return false;
	
	if (strspn(ip_add, "0123456789.") < strlen(ip_add)) 
		return false;
	
	
	//If it passed all that it is probably valid.
	return true;
}

void error( char *message)
{
		printf("\nError: %s", message );
		printf("\n\nUsage:\n\n ./client[.exe] [ip.add.re.ss] [port]" );
		printf("\n\nIf no arguements are given, a default address and port are used");
		printf("\nDefault address is %s. \nDefault port is %i.", IP_ADDRESS, PORT );
	
}