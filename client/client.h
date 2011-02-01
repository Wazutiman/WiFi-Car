/*
 *	Constants and prototypes used by client.c
 *
 *	by Shawn Dooley
 *
 *
 */



// the port WiShield is listening on

#define PORT 1000 

#define IP_ADDRESS "192.168.1.10"


//A constant to distinguish between a forward and reverse motor command.

#define REVERSE 32


// max number of bytes we can get at once from WiShield

#define MAXDATASIZE 3   


// A larger number here makes the car speed up faster. 
//Default is 10.		

#define STEPSIZE 10	



//Prototypes


void adjust_motor_speed(int keypress);  //Obsolete, being removed.

/*

This function is designed to process the changes based on a keypress when the wheels are stopped. 

*/

void stopped_keys(int keypress);
/*

This function is designed to process the changes based on a keypress when the wheels are both moving backwards. 

*/

void reverse_keys(int keypress);
/*

This function is designed to process the changes based on a keypress when the wheels are both moving forward. 

*/

void forward_keys(int keypress);

/*

This function is designed to process the changes based on a keypress when the wheels are spinning in opposite directions. 

*/

void opposite_direction_keys(int keypress);


/*

This function is designed to be called after the key press has been processed, it brings back variables that may have stepped over their limits, and creates the string that is to be sent to the Wi-Shield.

*/
void build_string(void);

/*

Simple function to determine if the IP address supplied as an arguement is a valid format.

*/


bool validIP( char *ip_add);


/*

Convenient method to report an error, and print usage
*/


void error( char *message);


 
