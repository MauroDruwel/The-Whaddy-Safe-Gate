#define echoPin 4
#define trigPin 2

long duration, distance;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(echoPin,INPUT);
  pinMode(trigPin,OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(trigPin,LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin,HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin,LOW);
  duration = pulseIn(echoPin, HIGH);

  distance = duration*0.034/2;
  Serial.println(distance);
  delay(50);
}
