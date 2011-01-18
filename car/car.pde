/*
 * Socket App
 *
 * A simple socket application example using the WiShield 1.0
 */

#include <WiShield.h>

#define WIRELESS_MODE_INFRA	1
#define WIRELESS_MODE_ADHOC	2

//Prototypes
void update_motor_speed( void );

//Motor control pins.
int lSpeedPin = 6;
int lMotorPin = 7;
int rSpeedPin = 5;
int rMotorPin = 4;

int lmotrspd = 65; 
int rmotrspd = 65;



// Wireless configuration parameters ----------------------------------------
unsigned char local_ip[] = {192,168,1,10};	// IP address of WiShield
unsigned char gateway_ip[] = {192,168,1,1};	// router or gateway IP address
unsigned char subnet_mask[] = {255,255,255,0};	// subnet mask for the local network
const prog_char ssid[] PROGMEM = {"Wazutiman"};		// max 32 bytes

unsigned char security_type = 0;	// 0 - open; 1 - WEP; 2 - WPA; 3 - WPA2

// WPA/WPA2 passphrase
const prog_char security_passphrase[] PROGMEM = {"Happy"};	// max 64 characters

// WEP 128-bit keys
// sample HEX keys
prog_uchar wep_keys[] PROGMEM = {	0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d,	// Key 0
									0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00,	// Key 1
									0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00,	// Key 2
									0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00	// Key 3
								};

// setup the wireless mode
// infrastructure - connect to AP
// adhoc - connect to another WiFi device
unsigned char wireless_mode = WIRELESS_MODE_INFRA;

unsigned char ssid_len;
unsigned char security_passphrase_len;
//---------------------------------------------------------------------------
char buffer[3];

void setup()
{
  pinMode( lMotorPin, OUTPUT);
  pinMode( rMotorPin, OUTPUT);
  
  Serial.begin(9600);	
  WiFi.init();
}

void loop()
{
  WiFi.run();
  
  //Only update the motor speed if we get new instructions.
  if(buffer[0] != '\0')
  {
    update_motor_speed();
    buffer[0] = '\0';        
    buffer[1] = '\0';
    buffer[2] = '\0';
  }
  
}//end of main




void update_motor_speed( void )
{

 Serial.print(buffer[0]);
 
  if(buffer[0] < 100)
  {
    lmotrspd = (buffer[0] % 65) * 10 ;  
    digitalWrite(lMotorPin,HIGH);
    analogWrite(lSpeedPin, lmotrspd);
    Serial.print(lmotrspd);
  }
  else
  {
   //32 characters difference for lower case 
   lmotrspd = ((buffer[0] % 65) % 32) * 10 ; 

    digitalWrite(lMotorPin,LOW);
    analogWrite(lSpeedPin, lmotrspd);
    Serial.print(lmotrspd);
    Serial.println( (int)(buffer[0]));
  }
 
 
  if(buffer[1] < 100)
  {
    rmotrspd = (buffer[1] % 65) * 10 ;  
    digitalWrite(lMotorPin,HIGH);
    analogWrite(rSpeedPin, rmotrspd);
    

  }
  else
  {
       //32 characters difference for lower case 
   rmotrspd = ((buffer[1] % 65) % 32) * 10 ; 
   
   digitalWrite(lMotorPin,LOW);
   analogWrite(rSpeedPin, rmotrspd);
 
  }
} 


