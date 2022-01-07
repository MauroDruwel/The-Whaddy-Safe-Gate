#include <Adafruit_Fingerprint.h>

#include <Servo.h> //Servo library

Servo myservo;  //Servo name is myservo

// Create a variable to store the servo position:
int pos = 0;
int oldState = 0;
int newState = 90;

// HC-SR05
#define  TRIGGER_PIN  12
#define  ECHO_PIN     13


#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
// For UNO and others without hardware serial, we must use software serial...
// pin #4 is IN from sensor (GREEN wire)
// pin #7 is OUT from arduino  (WHITE wire)
// Set up the serial port to use softwareserial..
SoftwareSerial mySerial(4, 7);

#else
// On Leonardo/M0/etc, others with hardware serial, use hardware serial!
// #0 is green wire, #1 is white
#define mySerial Serial1

#endif


Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

void setup()
{
	Serial.begin(9600);
	while (!Serial);  // For Yun/Leo/Micro/Zero/...
	delay(100);
	Serial.println("\n\nAdafruit finger detect test");
	myservo.attach(9);
	// HC-SR05
	pinMode(TRIGGER_PIN, OUTPUT);
	pinMode(ECHO_PIN, INPUT);

	// set the data rate for the sensor serial port	
	finger.begin(57600);
	delay(5);
	if (finger.verifyPassword()) {
		Serial.println("Found fingerprint sensor!");
	}
	else {
		Serial.println("Did not find fingerprint sensor :(");
		while (1) { delay(1); }
	}

	Serial.println(F("Reading sensor parameters"));
	finger.getParameters();
	Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
	Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
	Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
	Serial.print(F("Security level: ")); Serial.println(finger.security_level);
	Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
	Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
	Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);

	finger.getTemplateCount();

	if (finger.templateCount == 0) {
		Serial.print("Sensor doesn't contain any fingerprint data. Please run the 'enroll' example.");
	}
	else {
		Serial.println("Waiting for valid finger...");
		Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
	}
}

void loop()                     // run over and over again
{
	getFingerprintIDez();
	if (oldState != newState) {
		myservo.attach(9);
		delay(15);
		Sweep();
	}
	else {
		myservo.detach();//myservo.write(oldState);
	}
	delay(50);
}

uint8_t getFingerprintID() {
	uint8_t p = finger.getImage();
	switch (p) {
	case FINGERPRINT_OK:
		//Serial.println("Image taken");
		break;
	case FINGERPRINT_NOFINGER:
		//Serial.println("No finger detected");
		return p;
	case FINGERPRINT_PACKETRECIEVEERR:
		//Serial.println("Communication error");
		return p;
	case FINGERPRINT_IMAGEFAIL:
		//Serial.println("Imaging error");
		return p;
	default:
		//Serial.println("Unknown error");
		return p;
	}

	// OK success!

	p = finger.image2Tz();
	switch (p) {
	case FINGERPRINT_OK:
		//Serial.println("Image converted");
		break;
	case FINGERPRINT_IMAGEMESS:
		Serial.println("Image too messy");
		return p;
	case FINGERPRINT_PACKETRECIEVEERR:
		//Serial.println("Communication error");
		return p;
	case FINGERPRINT_FEATUREFAIL:
		//Serial.println("Could not find fingerprint features");
		return p;
	case FINGERPRINT_INVALIDIMAGE:
		//Serial.println("Could not find fingerprint features");
		return p;
	default:
		Serial.println("Unknown error");
		return p;
	}

	// OK converted!
	p = finger.fingerSearch();
	if (p == FINGERPRINT_OK) {
		//Serial.println("Found a print match!");
	}
	else if (p == FINGERPRINT_PACKETRECIEVEERR) {
		//Serial.println("Communication error");
		return p;
	}
	else if (p == FINGERPRINT_NOTFOUND) {
		//Serial.println("Did not find a match");
		return p;
	}
	else {
		//Serial.println("Unknown error");
		return p;
	}

	// found a match!
	Serial.print("#"); Serial.print(finger.fingerID);
	Serial.print(","); Serial.println(finger.confidence);

	return finger.fingerID;
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
	uint8_t p = finger.getImage();
	if (p != FINGERPRINT_OK)  return -1;

	p = finger.image2Tz();
	if (p != FINGERPRINT_OK)  return -1;

	p = finger.fingerFastSearch();
	if (p != FINGERPRINT_OK)  return -1;

	// found a match!
	Serial.print("#"); Serial.print(finger.fingerID);
	Serial.print(","); Serial.println(finger.confidence);
	if (finger.fingerID == 1 || finger.fingerID == 2) {
		Serial.println("Finger Detected!");
		if (oldState == 90) {
			float cm = GetDistance();
			Serial.println(cm);
			if (cm >= 15.5/*155*/) {
				newState = 0;
			}
			else {
				Serial.println("Poort kan niet open. Uw auto staat te dicht!");
			}

		}
		else {
			newState = 90;
		}
	}
	return finger.fingerID;
}

int GetDistance() // returns the distance (cm)
{
	long duration; // variable for the duration of sound wave travel
	float distance; // variable for the distance measurement
	// Clears the trigPin condition
	digitalWrite(TRIGGER_PIN, LOW);
	delayMicroseconds(2);
	// Sets the trigPin HIGH (ACTIVE) for 10 microseconds
	digitalWrite(TRIGGER_PIN, HIGH);
	delayMicroseconds(10);
	digitalWrite(TRIGGER_PIN, LOW);
	// Reads the echoPin, returns the sound wave travel time in microseconds
	duration = pulseIn(ECHO_PIN, HIGH);
	// Calculating the distance
	distance = duration * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)

	return distance;
}

void Sweep() {
	if (newState == 90) {
		for (pos = 0; pos <= 90; pos += 1) { // goes from 0 degrees to 180 degrees
			myservo.write(pos);              // tell servo to go to position in variable 'pos'
			delay(30);                       // waits 15ms for the servo to reach the position
		}
	}
	if (newState == 0) {
		for (pos = 90; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
			myservo.write(pos);              // tell servo to go to position in variable 'pos'
			delay(30);                       // waits 15ms for the servo to reach the position
		}
	}
	oldState = newState;
	delay(1000);
}
