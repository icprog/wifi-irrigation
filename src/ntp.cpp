#include "ntp.h"

// Create an instance of the WiFiUDP class to send and receive
WiFiUDP UDP;
// time.nist.gov NTP server address
IPAddress timeServerIP;
// Address of time server
const char* NTPServerName = "pool.ntp.org";
// buffer to hold incoming and outgoing packets
byte NTPBuffer[NTP_PACKET_SIZE];

// British Summer Time
TimeChangeRule BST = {"BST", Last, Sun, Mar, 1, 60};
// Standard Time        
TimeChangeRule GMT = {"GMT", Last, Sun, Oct, 2, 0};
// United Kingdom (London, Belfast)
Timezone UK(BST, GMT);
// Base pointer
TimeChangeRule *tcr;
// Store time with DST/Timezone compensation
time_t time_local;

// Data recieved from getTime() function - may be invalid
static uint32_t time_ntp = 0;
// As above, but checked for valididty
static uint32_t time_unix = 0;

// Time converted to instantaneous second count
time_t time_utc = 0;
// As above, but previous reading
static time_t time_utc_prev = 0;

// Time since last ntp request was sent
static uint32_t last_ntp_request = 0;
// Time since last ntp response was actually recieved
static uint32_t last_ntp_response = millis();
// Current time in ms
static uint32_t millis_current = millis();

static uint32_t getTime();
static void sendNTPpacket(IPAddress& address);

void setup_NTP(){
  // Return instantly if no internet available
  if(WiFi.status() != WL_CONNECTED){ return; }

	UDP.begin(123);
	// Get the IP address of the NTP server
	if(!WiFi.hostByName(NTPServerName, timeServerIP)) {
    	Serial.println(F("DNS lookup failed. Rebooting."));
    	Serial.flush();
    	ESP.reset();
  	}
  	Serial.print(F("Time server IP:\t"));
  	Serial.println(timeServerIP);
  
  	Serial.println(F("\r\nSending NTP request ..."));
  	sendNTPpacket(timeServerIP); 
}

String time_string(time_t time){
	return String("") + \
		hour(time) + ":" + minute(time) + ":" + second(time);
}


bool NTP_loop() {
  	millis_current = millis();
 	// If a minute has passed since last NTP request and WiFi is connected
  	if (TIME_TO_REFRESH && !WLAN_DISCONNECTED) {
    	last_ntp_request = millis_current;
    	Serial.println(F("\r\nSending NTP request ..."));
    	// Send an NTP request
    	sendNTPpacket(timeServerIP);               
  	}

  	// Check if an NTP response has arrived and get the (UNIX) time
  	time_ntp = getTime();
  	 // If a new timestamp has been received
  	if (time_ntp) {                                 
    	time_unix = time_ntp;
    	Serial.print(F("NTP response:\t"));
    	Serial.println(time_unix);
    	last_ntp_response = millis_current;
  	}
  	else if (UPDATE_TIMEOUT) {
    	Serial.println(F("More than 1 hour since last NTP response. Rebooting."));
    	Serial.flush();
    	ESP.reset();
  	}
  	time_utc = CALC_ACTUAL_TIME;
    setTime(UK.toLocal(time_utc, &tcr));
	// If a second has passed since last print
	if (time_utc != time_utc_prev && time_unix != 0) { 
	    time_utc_prev = time_utc;
	    return true;
	}
	else { return false; }
}

static uint32_t getTime() {
	// If there's no response (yet)
  if (UDP.parsePacket() == 0) { return 0; }
  // read the packet into the buffer
	UDP.read(NTPBuffer, NTP_PACKET_SIZE);
	// Combine the 4 timestamp bytes into one 32-bit number
	uint32_t NTPTime = (NTPBuffer[40] << 24) | (NTPBuffer[41] << 16) | (NTPBuffer[42] << 8) | NTPBuffer[43];
	// Convert NTP time to a UNIX timestamp:
	// Unix time starts on Jan 1 1970. That's 2208988800 seconds in NTP time:
	const uint32_t seventyYears = 2208988800UL;
	// subtract seventy years:
	uint32_t UNIXTime = NTPTime - seventyYears;
	return UNIXTime;
}

static void sendNTPpacket(IPAddress& address) {
	// set all bytes in the buffer to 0	
  	memset(NTPBuffer, 0, NTP_PACKET_SIZE);  
  	// Initialize values needed to form NTP request
  	// LI, Version, Mode
  	NTPBuffer[0] = 0b11100011;   
  	// send a packet requesting a timestamp:
  	// NTP requests are to port 123
  	UDP.beginPacket(address, 123);
  	UDP.write(NTPBuffer, NTP_PACKET_SIZE);
  	UDP.endPacket();
}