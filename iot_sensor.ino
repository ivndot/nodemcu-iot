/*
  ================================
           LIBRARIES
  ================================
*/
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
/*
  ================================
         WIFI CONFIGURATION
  ================================
*/
const String SERVER_URL = "http://api-iot-maria.herokuapp.com/";
#define SSID "SSID_EXAMPLE"
#define PASSWD  "PASSWD_EXAMPLE"
/*
  ================================
      PIN LOGIC nodeMCU ESP8266
  ================================
*/
/*static const uint8_t D0 = 16;
  static const uint8_t D1 = 5;
  static const uint8_t D2= 4;
  static const uint8_t D3 = 0;
  static const uint8_t D4 = 2;
  static const uint8_t D5 = 14;*/
/*
  ================================
          PIN INPUT/OUTPUT
  ================================
*/
#define SENSOR_TEMP A0
#define LED_SENSOR 4  /* D2 */
#define LED_1 16      /* D0 */
#define LED_2 5       /* D1 */
#define ENA 0         /* D3 */
#define IN1 2         /* D4 */
#define IN2 14        /* D5 */
/*
  ================================
         GLOBAL VARIABLES
  ================================
*/
const int MAX_TEMPERATURE = 50;
int celsius = 0;
String fanMode = "a";
int fanStatus = 0;
String fanSpeed = "l";
int led1Status = 0;
int led2Status = 0;

/*===============================*/

void setup() {
  Serial.begin(115200);
  //SENSOR LED
  pinMode(LED_SENSOR, OUTPUT);
  //LED 1
  pinMode(LED_1, OUTPUT);
  //LED 2
  pinMode(LED_2, OUTPUT);
  //FAN
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  WiFi.begin(SSID, PASSWD);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  delay(5000);
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    // the conecction is established

    // get temperature from the sensor and parse to celsius
    celsius = getCelsiusDegrees(analogRead(SENSOR_TEMP));

    // get mode
    getModeRequest();
    // set temperature23223
    setTemperatureRequest();

  } else {
    // there is no connection to internet
    Serial.println("WIFI disconnected");
  }

  // wait 1 second
  delay(1000);
}

/*===============================*/

/*
  ================================
      GET Request --> /api/mode

     Obtain the mode of the iot
     device from the server and
     update state
  ================================
*/
void getModeRequest() {
  WiFiClient client;
  HTTPClient http;
  // begin connection
  http.begin(client, SERVER_URL + "api/mode");
  // send GET request
  int responseCode = http.GET();

  if (responseCode > 0) {
    // we get a response with the information
    String data = http.getString();
    // destructure the data `mode:fanStatus:fanSpeed:led1Status:led2Status`
    extractData(data);

    // control LEDS
    controlLED();
    // control FAN
    controlFan();

  } else {
    Serial.println("Error code");
    Serial.println(responseCode);
  }

  // free resources
  http.end();
}

/*
  ================================
     PUT Request --> /api/temp

     Send the temperature from
     the lm35 sensor to the
     server
  ================================
*/
void setTemperatureRequest() {
  WiFiClient client;
  HTTPClient http;
  // begin connection
  http.begin(client, SERVER_URL + "api/temp");
  // especify content type
  http.addHeader("Content-Type", "application/json");
  // set content
  String content = "{\"temp\":";
  content += celsius;
  content += "}";
  // send put request
  int responseCode = http.PUT(content);
  // print response code
  Serial.println(responseCode);

  // free resources
  http.end();
}

/*
  ================================
            CONTROL FAN
  ================================
*/
void controlFan() {
  if (fanMode.equals("m")) {
    //manual mode
    Serial.println("Manual");

    //TURN ON/OFF FAN
    if (fanStatus == 0) {
      analogWrite(ENA, 0);
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);
    } else if (fanStatus == 1) {
      //CONTROL SPEED
      if (fanSpeed.equals("l")) {
        //slow speed
        analogWrite(ENA, 100);
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, HIGH);

      } else if (fanSpeed.equals("f")) {
        //fast speed
        analogWrite(ENA, 255);
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, HIGH);
      }
    }
  } else if (fanMode.equals("a")) {
    //automatic mode
    Serial.println("Automatico");

    if (celsius >= MAX_TEMPERATURE) {
      //turn on fan
      turnOnFan();
      //turn on led
      digitalWrite(LED_SENSOR, HIGH);
    } else {
      //turn off fan
      turnOffFan();
      //turn off led
      digitalWrite(LED_SENSOR, LOW);
    }

  }
}

/*
  ================================
            CONTROL LED
  ================================
*/
void controlLED() {
  //TURN ON/OFF LED 1
  if (led1Status == 0) {
    Serial.println("TURN OFF LED 1");
    digitalWrite(LED_1, LOW);
  } else if (led1Status == 1) {
    Serial.println("TURN ON LED 1");
    digitalWrite(LED_1, HIGH);
  }

  //TURN ON/OFF LED 2
  if (led2Status == 0) {
    Serial.println("TURN OFF LED 2");
    digitalWrite(LED_2, LOW);
  } else if (led2Status == 1) {
    Serial.println("TURN ON LED 2");
    digitalWrite(LED_2, HIGH);
  }
}

/*
  ================================
            TURN ON FAN
  ================================
*/
void turnOnFan() {
  analogWrite(ENA, 255);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
}

/*
  ================================
           TURN OFF FAN
  ================================
*/
void turnOffFan() {
  analogWrite(ENA, 0);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
}

/*
  ================================
     CONVERT TO CELSIUS DEGREES
  ================================
*/
int getCelsiusDegrees(float temp) {
  return (temp * 5.0 / 1024) * 100;
}

/*
  ================================
           EXTRACT DATA
  ================================
*/
void extractData(String data) {
  //mode
  fanMode = data.substring(0, 1);
  Serial.print("FAN MODE --> ");
  Serial.println(fanMode);
  //fan status
  fanStatus = data.substring(2, 3).toInt();
  Serial.print("FAN STATUS --> ");
  Serial.println(fanStatus);
  //fan speed
  fanSpeed = data.substring(4, 5);
  Serial.print("FAN SPEED --> ");
  Serial.println(fanSpeed);
  //led 1 status
  led1Status = data.substring(6, 7).toInt();
  Serial.print("LED 1 --> ");
  Serial.println(led1Status);
  //led 2 status
  led2Status = data.substring(8, 9).toInt();
  Serial.print("LED 2 --> ");
  Serial.println(led2Status);
}
