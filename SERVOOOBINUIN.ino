/*
   ESP8266 - 4 Ultrasonic HC-SR04

   SENSOR 1
   TRIG = GPIO 5
   ECHO = GPIO 4

   SENSOR 2
   TRIG = GPIO 14
   ECHO = GPIO 12

   SENSOR 3
   TRIG = GPIO 13
   ECHO = GPIO 16

   SENSOR 4
   TRIG = GPIO 0
   ECHO = GPIO 2
*/

#define TRIG1 5
#define ECHO1 4

#define TRIG2 14
#define ECHO2 12

#define TRIG3 13
#define ECHO3 16

#define TRIG4 0
#define ECHO4 2


float bacaUltrasonik(int trigPin, int echoPin)
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(3);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);

  digitalWrite(trigPin, LOW);

  unsigned long durasi = pulseIn(echoPin, HIGH, 30000);

  if (durasi == 0) {
    return -1;
  }

  float jarak = durasi * 0.0343 / 2.0;

  if (jarak > 400) {
    return -1;
  }

  return jarak;
}


void tampilkan(String nama, float jarak)
{
  Serial.print(nama);
  Serial.print(": ");

  if (jarak < 0) {
    Serial.println("Tidak terbaca");
  }
  else {
    Serial.print(jarak, 1);
    Serial.println(" cm");
  }
}


void setup()
{
  Serial.begin(115200);

  pinMode(TRIG1, OUTPUT);
  pinMode(ECHO1, INPUT);

  pinMode(TRIG2, OUTPUT);
  pinMode(ECHO2, INPUT);

  pinMode(TRIG3, OUTPUT);
  pinMode(ECHO3, INPUT);

  pinMode(TRIG4, OUTPUT);
  pinMode(ECHO4, INPUT);

  digitalWrite(TRIG1, LOW);
  digitalWrite(TRIG2, LOW);
  digitalWrite(TRIG3, LOW);
  digitalWrite(TRIG4, LOW);

  delay(2000);

  Serial.println();
  Serial.println("==============================");
  Serial.println(" ESP8266 4 ULTRASONIC SENSOR");
  Serial.println("==============================");
}


void loop()
{
  float sensor1 = bacaUltrasonik(TRIG1, ECHO1);

  delay(60);

  float sensor2 = bacaUltrasonik(TRIG2, ECHO2);

  delay(60);

  float sensor3 = bacaUltrasonik(TRIG3, ECHO3);

  delay(60);

  float sensor4 = bacaUltrasonik(TRIG4, ECHO4);

  Serial.println("------------------------------");

  tampilkan("Sensor 1", sensor1);
  tampilkan("Sensor 2", sensor2);
  tampilkan("Sensor 3", sensor3);
  tampilkan("Sensor 4", sensor4);

  Serial.println();

  delay(500);
}