// This #include statement was automatically added by the Particle IDE.
#include <WebServer.h>

/* Function prototypes -------------------------------------------------------*/
int tinkerDigitalRead(String pin);
int tinkerDigitalWrite(String command);
int tinkerAnalogRead(String pin);
int tinkerAnalogWrite(String command);

int fireplaceSwitch = D0;
int relaySwitch = A0;

#define PREFIX ""
WebServer webserver(PREFIX, 80);


void helloCmd(WebServer &server, WebServer::ConnectionType type, char *, bool)
{
    /* this line sends the standard "we're all OK" headers back to the
     browser */
    server.httpSuccess();

    /* if we're handling a GET or POST, we can output our data here.
     For a HEAD request, we just stop after outputting headers. */
    if (type != WebServer::HEAD)
    {
        /* this defines some HTML text in read-only memory aka PROGMEM.
         * This is needed to avoid having the string copied to our limited
         * amount of RAM. */
        P(helloMsg) = "<h1>fireplace!</h1>";
        
        /* this is a special form of print that outputs from PROGMEM */
        server.printP(helloMsg);
    }
}


/* This function is called once at start up ----------------------------------*/
void setup() {
 
	Particle.function("rlySwitchOn",rlySwitchOn);
    Particle.function("rlySwitchOff",rlySwitchOff);
    
    Particle.function("rlySwitch15",rlySwitch15);
    Particle.function("rlySwitch60",rlySwitch60);
    Particle.function("rlySwitch120",rlySwitch120);
    
    Particle.function("digitalread", tinkerDigitalRead);
	Particle.function("digitalwrite", tinkerDigitalWrite);
	Particle.function("analogread", tinkerAnalogRead);
	Particle.function("analogwrite", tinkerAnalogWrite);
    
    pinMode(fireplaceSwitch, INPUT_PULLDOWN);
    pinMode(relaySwitch, OUTPUT);

    digitalWrite(relaySwitch, HIGH);  

    Time.zone(-6);

    int curHour = Time.hour();
    char str[15];
    sprintf(str, "%d", curHour);
    webserver.setDefaultCommand(&helloCmd);
    RGB.control(true); 
    RGB.brightness(0);
    webserver.addCommand("index.html", &helloCmd);
}

int delayMins = 1;
unsigned long lastTime = 0;
unsigned long lastTimeCheck = 0;
bool relaySwitchNeedsToTurnOn = false;
bool relaySwitchTimedNeedsToTurnOff = false;
bool fireplaceSwitchNeedsToTurnOn = false;
bool alertIfOn = true;

void loop(){
    char buff[64];
    int len = 64;
    
    if (digitalRead(fireplaceSwitch) == LOW){
        if(!fireplaceSwitchNeedsToTurnOn){
            pinOff(relaySwitch);
            fireplaceSwitchNeedsToTurnOn = true;
        }
    } else if(digitalRead(fireplaceSwitch) == HIGH) {
        if(fireplaceSwitchNeedsToTurnOn){
            pinOn(relaySwitch);
            fireplaceSwitchNeedsToTurnOn = false;
        }
    }
    
    unsigned long nowCheck = millis();
    if ((nowCheck - lastTimeCheck) >=  30 * 1000) {
    	lastTimeCheck = nowCheck;
        if (digitalRead(relaySwitch) == HIGH){
            if(!alertIfOn){
                Particle.publish("fireplace-off", NULL, 60, PRIVATE);
                alertIfOn = true;
            }
        } else if(digitalRead(relaySwitch) == LOW) {
            if(alertIfOn){
                Particle.publish("fireplace-on", NULL, 60, PRIVATE);
                alertIfOn = false;
            }
        }
    }
    

    /* process incoming connections one at a time forever */
    webserver.processConnection(buff, &len);
    
    unsigned long now = millis();
    if ((now - lastTime) >=  delayMins * 60 * 1000) {
    	lastTime = now;
        if(relaySwitchTimedNeedsToTurnOff){
            relaySwitchTimedNeedsToTurnOff = false;
            pinOff(relaySwitch);
        }
    }
}

int pinOn(int pin){
    if (digitalRead(pin) == HIGH){
        // Particle.publish("fireplace-on", NULL, 60, PRIVATE);    
        digitalWrite(pin, LOW);  
    }
    return 1;
}

int pinOff(int pin){
    if (digitalRead(pin) == LOW){
        // Particle.publish("fireplace-off", NULL, 60, PRIVATE);    
        digitalWrite(pin, HIGH); 
    }
    return 1;
}


int rlySwitchOn(String command){
    pinOn(relaySwitch);
    relaySwitchTimedNeedsToTurnOff = false;
    return 1;
}

int rlySwitchOff(String command){
    pinOff(relaySwitch);
    return 1;
}

int rlySwitch15(String command){
    pinOn(relaySwitch);
    lastTime = millis();
    delayMins = 15;
    relaySwitchTimedNeedsToTurnOff = true;
    return 1;
}

int rlySwitch60(String command){
    pinOn(relaySwitch);
    lastTime = millis();
    delayMins = 60;
    relaySwitchTimedNeedsToTurnOff = true;
    return 1;
}

int rlySwitch120(String command){
    pinOn(relaySwitch);
    lastTime = millis();
    delayMins = 120;
    relaySwitchTimedNeedsToTurnOff = true;
    return 1;
}

/*******************************************************************************
 * Function Name  : tinkerDigitalRead
 * Description    : Reads the digital value of a given pin
 * Input          : Pin
 * Output         : None.
 * Return         : Value of the pin (0 or 1) in INT type
                    Returns a negative number on failure
 *******************************************************************************/
int tinkerDigitalRead(String pin)
{
	//convert ascii to integer
	int pinNumber = pin.charAt(1) - '0';
	//Sanity check to see if the pin numbers are within limits
	if (pinNumber< 0 || pinNumber >7) return -1;

	if(pin.startsWith("D"))
	{
		pinMode(pinNumber, INPUT_PULLDOWN);
		return digitalRead(pinNumber);
	}
	else if (pin.startsWith("A"))
	{
		pinMode(pinNumber+10, INPUT_PULLDOWN);
		return digitalRead(pinNumber+10);
	}
	return -2;
}

/*******************************************************************************
 * Function Name  : tinkerDigitalWrite
 * Description    : Sets the specified pin HIGH or LOW
 * Input          : Pin and value
 * Output         : None.
 * Return         : 1 on success and a negative number on failure
 *******************************************************************************/
int tinkerDigitalWrite(String command)
{
	bool value = 0;
	//convert ascii to integer
	int pinNumber = command.charAt(1) - '0';
	//Sanity check to see if the pin numbers are within limits
	if (pinNumber< 0 || pinNumber >7) return -1;

	if(command.substring(3,7) == "HIGH") value = 1;
	else if(command.substring(3,6) == "LOW") value = 0;
	else return -2;

	if(command.startsWith("D"))
	{
		pinMode(pinNumber, OUTPUT);
		digitalWrite(pinNumber, value);
		return 1;
	}
	else if(command.startsWith("A"))
	{
		pinMode(pinNumber+10, OUTPUT);
		digitalWrite(pinNumber+10, value);
		return 1;
	}
	else return -3;
}

/*******************************************************************************
 * Function Name  : tinkerAnalogRead
 * Description    : Reads the analog value of a pin
 * Input          : Pin
 * Output         : None.
 * Return         : Returns the analog value in INT type (0 to 4095)
                    Returns a negative number on failure
 *******************************************************************************/
int tinkerAnalogRead(String pin)
{
	//convert ascii to integer
	int pinNumber = pin.charAt(1) - '0';
	//Sanity check to see if the pin numbers are within limits
	if (pinNumber< 0 || pinNumber >7) return -1;

	if(pin.startsWith("D"))
	{
		return -3;
	}
	else if (pin.startsWith("A"))
	{
		return analogRead(pinNumber+10);
	}
	return -2;
}

/*******************************************************************************
 * Function Name  : tinkerAnalogWrite
 * Description    : Writes an analog value (PWM) to the specified pin
 * Input          : Pin and Value (0 to 255)
 * Output         : None.
 * Return         : 1 on success and a negative number on failure
 *******************************************************************************/
int tinkerAnalogWrite(String command)
{
	//convert ascii to integer
	int pinNumber = command.charAt(1) - '0';
	//Sanity check to see if the pin numbers are within limits
	if (pinNumber< 0 || pinNumber >7) return -1;

	String value = command.substring(3);

	if(command.startsWith("D"))
	{
		pinMode(pinNumber, OUTPUT);
		analogWrite(pinNumber, value.toInt());
		return 1;
	}
	else if(command.startsWith("A"))
	{
		pinMode(pinNumber+10, OUTPUT);
		analogWrite(pinNumber+10, value.toInt());
		return 1;
	}
	else return -2;
}
