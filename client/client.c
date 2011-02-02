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
	int port;

	

	/**
	If there are no arguements, use the default values for
	IP address and port number.
	*/
	
	if(argc == 1)
	{
		strcpy(ip_address, IP_ADDRESS);
		port = PORT;
	}
	/** 
	If there are more than 2 additional arguements, inform the user
	that they are stupid, and exit.
	*/
	if( argc > 3)
	{	
		error( "Wrong number of arguements.");

		exit (2);
		
	}
	
	/**
	If there are arguements
	Test the format of the IP address to ensure it is in the correct 
	format.
	*/
		
	if( argc > 1 && !validIP( argv[1])  )
	{
		/** 
		Announce invalid IP address format, and exit.
		*/
		
		error( "Invalid IP format.");
		exit(3);
	}
	else
	{
		// If the IP address is valid, store it in the ip_address variable
		
		strcpy( ip_address, argv[1]);
	}
	
	/**
	If a 3rd arguement is present, check to make sure it could be a
	valid port numnber, otherwise print usage, and exit.
	*/
	
	if(argc == 3 )
	{
		if( atoi(argv[2]) > 65535  || atoi(argv[2]) <= 0 )
		{
			error("Invalid arguement for port.");
			exit(4);
		}
		else
		{
			port = atoi(argv[2]);
		}
	}
	else
	{
		port = PORT;
	}
		
		
		
		
		int ch;					//Variable used to store keyboard input
		int sockfd;				//descriptor for socket
		struct sockaddr_in srv; //Structure for socket address info
		
		
		//Clear any stray info in the srv structure.
		memset(&srv, 0, sizeof(srv));

		// setup address type
		srv.sin_family = AF_INET;
		//convert port info to network byte order.
		srv.sin_port = htons(port);


		inet_pton(AF_INET, ip_address, &srv.sin_addr);

		// check socket, exit on error.
    	if ((sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) 
		{
     	   perror("socket");
     	   exit(0);
    	}
		// Attempt conenction, exit on failure.
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
		//Dont print keyboard input.
		noecho();
		
		//Set simple flags for vehicle state.
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


		/**
			Only send a new update string to the car if it is different than the last
			one that was sent. This reduces the chances of flooding the queue and 
			causing delays in response.
					
		*/

		if( strcmp(buf, lastSent))
		{		
    
				// Send user input.
	    	if (send(sockfd, buf, MAXDATASIZE, 0) == -1) 
			{
	    	 	perror("send");
      			close(sockfd);
      			exit(1);
    		}
			else
			{
				//
				memcpy(lastSent, buf, MAXDATASIZE);
			}
		}
 

 		// Loop Until the 'Esc'  or 'Q' is pressed.  
	} while(ch != 27 && ch != 'Q' );  

	/** 
	After exiting, send one last "STOP" string to make sure you dont
	break the connection with the car moving, which is funny, but not a
	really good idea.
	*/
	
	if (send(sockfd, "AA\0" , MAXDATASIZE, 0) == -1) 
	{
		perror("send");
		close(sockfd);
		exit(1);
	}


	//Close the connection and exit ncurses.
	close(sockfd);
	endwin();
  	return 0;


}  //End of main.

/*****************************************************************

Each of the following function is pretty straightforward.
They change the value of the buffer string based on user input, and 
vehicle state.

******************************************************************/




void stopped_keys(int keypress)
{
	switch(keypress)
	{
		case KEY_UP:
			
			/**
			Increase the speed of both left and right wheels
			if the up key is pressed when the vehicle is stopped
			*/
			
			reversed = FALSE;				
			left_reversed = FALSE;
			right_reversed = FALSE;	
			lmSpeed += STEPSIZE;
			rmSpeed += STEPSIZE;
			break;
				
		case KEY_DOWN:
			
			/**
			Increment the speed of both left and right wheels, AND
			change the flag for reverse.
			
			*/
			
			reversed = TRUE;				
			left_reversed = TRUE;
			right_reversed = TRUE;
			
			lmSpeed += STEPSIZE;
			rmSpeed += STEPSIZE;
			break;
						
		case KEY_LEFT:
			/**
			
			Increase the speed of both left and right wheels, but 
			only reverse the left one.
			
			this causes the vehicle to turn in place counterclockwise.
			
			*/
			
			reversed = FALSE;			
			left_reversed = TRUE;
			right_reversed = FALSE;
			lmSpeed += STEPSIZE;
			rmSpeed += STEPSIZE;
			break;		
		
		case KEY_RIGHT:

			/**
			Increment the speed of both left and right wheels, but only
			reverse the right one.
			
			this causes the vehicle to turn in place clockwise.
			*/
			
			reversed = FALSE;			
			left_reversed = FALSE;
			left_reversed = TRUE;
			lmSpeed += STEPSIZE;
			rmSpeed += STEPSIZE;
			break;		
		
	}
	
	/** Use the build string function to actually make the buffer string
	that gets sent to the car.
	*/
	build_string();

}


void reverse_keys(int keypress)
{

	switch(keypress)
	{
		case KEY_UP:
			/**
			If the car is moving backwards, pressing up causes the car to slow down 
			*/
			
			
			lmSpeed -= STEPSIZE;
			rmSpeed -= STEPSIZE;
			break;
				
		case KEY_DOWN:
			/**
			When reveresed, the down key causes teh vehicle to accelerate backwards.
			*/
			lmSpeed += STEPSIZE;
			rmSpeed += STEPSIZE;
			break;
						
		case KEY_LEFT:
			/**
			In reverse, the left key causes the left motor to slow down, and the
			right motor to speed up causing it to turn in an arc twards the left 
			side of the vehicle.
			*/
			
		
			lmSpeed -= STEPSIZE;
			rmSpeed += STEPSIZE;
			break;		
		
		case KEY_RIGHT:
			/**
			In reverse the right key causes teh right motor to slow down, and
			the left motor to speed up causing it to turn an arc twards
			the right side of the vehicle.
			*/
		
			lmSpeed += STEPSIZE;
			rmSpeed -= STEPSIZE;
			break;		
		
	}

	
	/**
	All these checks just fix any problems we may have caused by stepping
	over our bounds. This is very necessary if you changed the STEPSIZE
	constant.
	
	*/
	
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
	

	/**
	Use build_string to make the string to send to the car.
	*/
	build_string();

	


}


void forward_keys(int keypress)
{

	switch(keypress)
	{
		case KEY_UP:
			/**
			Up causes the car to accelerate if the car is already moving.
			
			*/
			
			lmSpeed += STEPSIZE;
			rmSpeed += STEPSIZE;
			break;
				
		case KEY_DOWN:
			/**
				
			Down causes the car to slow down.
			
			*/
			lmSpeed -= STEPSIZE;
			rmSpeed -= STEPSIZE;
			break;
						
		case KEY_LEFT:
			/**
			Left causes the left wheels to slow down, and the right wheels 
			to speed up, causing the car to turn in an arc to the left.
			*/
		
			lmSpeed -= STEPSIZE;
			rmSpeed += STEPSIZE;
			break;
	
		case KEY_RIGHT:
			/**
			The right arrow key causes the right wheel to slow down, and the 
			left wheel to speed up causing the car to turn in an arc to the right.
			*/
			lmSpeed += STEPSIZE;
			rmSpeed -= STEPSIZE;
			break;		
	}

		/**
	All these checks just fix any problems we may have caused by stepping
	over our bounds. This is very necessary if you changed the STEPSIZE
	constant.
	
	*/

	
	if( lmSpeed <= 65)
		lmSpeed = 65;
	if( rmSpeed <= 65)
		rmSpeed = 65;
	
	if( lmSpeed >= 90)
		lmSpeed = 90;
	if( rmSpeed >= 90)
		rmSpeed = 90;
	

	/**
	Use build_string to make the string to send to the car.
	*/
	build_string();


}


void opposite_direction_keys(int keypress)
{

	switch(keypress)
	{
		case KEY_UP:
			/**
			If the car is turning in place, the up key makes it 
			spin in place faster.
			*/
			lmSpeed += STEPSIZE;
			rmSpeed += STEPSIZE;
			break;
				
		case KEY_DOWN:
			/**
			If the car is turning in place, the down key makes it 
			spin in place slower until it stops.
			*/


			lmSpeed -= STEPSIZE;
			rmSpeed -= STEPSIZE;
			break;
						
		case KEY_LEFT:
			/**
			I couldnt think of a good desired behavoir for this condition
			so I decided to go with slowing down the left wheel, and speeding
			up the right. I dont have a good reason for this.
			
			*/
		
			lmSpeed -= STEPSIZE;
			rmSpeed += STEPSIZE;
			break;		
		
		case KEY_RIGHT:
			/**
			I couldnt think of a good desired behavoir for this condition
			for this either, so it is the opposite of above.
			*/

			lmSpeed += STEPSIZE;
			rmSpeed -= STEPSIZE;
			break;		
		
	}

	/**
	All these checks just fix any problems we may have caused by stepping
	over our bounds. This is very necessary if you changed the STEPSIZE
	constant.
	
	*/

	if( lmSpeed <= 65)
		lmSpeed = 65;
	if( rmSpeed <= 65)
		rmSpeed = 65;
	
	if( lmSpeed >= 90)
		lmSpeed = 90;
	if( rmSpeed >= 90)
		rmSpeed = 90;
	


	/**
	Use build_string to make the string to send to the car.
	*/

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




/**

	This is a simple function I found for validating an IP address.
	I found it online in a forum.
	

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

/**
This is just a function to print an error message and the correct
usage if someone messes up the arguements.
*/

void error( char *message)
{
		printf("\nError: %s", message );
		printf("\n\nUsage:\n\n ./client[.exe] [ip.add.re.ss] [port]" );
		printf("\n\nIf no arguements are given, a default address and port are used");
		printf("\nDefault address is %s. \nDefault port is %i.", IP_ADDRESS, PORT );
	
}