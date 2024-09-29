#include <DHT.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>

#define DHTPIN D3 // DHT11 Data Pin
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

const char *ssid = "project";              // Your WiFi SSID
const char *password = "project007";       // Your WiFi Password
const char *server = "api.thingspeak.com"; // ThingSpeak server

const char *apiKey = "6RK2WUNZAO40RLG3"; // Replace with your ThingSpeak API key
const long channelID = 2674941;          // Replace with your ThingSpeak channel ID

int irInPin = D1;  // IR sensor for in
int irOutPin = D2; // IR sensor for out
int smokePin = A0; // MQ-2 sensor

int personCount = 0;    // Variable to track the number of persons inside
int lastInState = LOW;  // Last state of the IN IR sensor
int lastOutState = LOW; // Last state of the OUT IR sensor

void setup()
{
    Serial.begin(115200);
    WiFi.begin(ssid, password);

    // Wait for connection
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }

    Serial.println("Connected to WiFi");

    dht.begin();
    pinMode(irInPin, INPUT);
    pinMode(irOutPin, INPUT);
}

void loop()
{
    // Read DHT11
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    // Read MQ-2
    int smokeLevel = analogRead(smokePin);

    // Read IR Sensors
    int inState = digitalRead(irInPin);
    int outState = digitalRead(irOutPin);

    // Check for person entering
    if (inState == HIGH && lastInState == LOW)
    {
        personCount++;
        delay(100); // Debounce delay
    }

    // Check for person exiting
    if (outState == HIGH && lastOutState == LOW)
    {
        if (personCount > 0)
        {
            personCount--;
        }
        delay(100); // Debounce delay
    }

    lastInState = inState;
    lastOutState = outState;

    // Send data to ThingSpeak
    if (WiFi.status() == WL_CONNECTED)
    {
        WiFiClient client;
        String url = String("/update?api_key=") + apiKey +
                     "&field1=" + String(temperature) +
                     "&field2=" + String(humidity) +
                     "&field3=" + String(smokeLevel) +
                     "&field4=" + String(personCount) +
                     "&field5=" + String(channelID);

        // Make a GET request
        if (client.connect(server, 80))
        {
            client.print("GET " + url + " HTTP/1.1\r\n");
            client.print("Host: " + String(server) + "\r\n");
            client.print("Connection: close\r\n");
            client.print("\r\n");

            // Wait for response
            while (client.available())
            {
                String line = client.readStringUntil('\n');
                Serial.println(line);
            }
            client.stop();
        }
        else
        {
            Serial.println("Connection failed");
        }
    }

    delay(3000); // Send data every 3 seconds
}