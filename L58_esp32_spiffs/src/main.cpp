#include "Arduino.h"
#include "SPIFFS.h"

const char* testFileName = "/test.txt";

void setup()
{
    Serial.begin(9600);

    if (!SPIFFS.begin(true))
    {
        Serial.println("File system initialization failed");
        ESP.restart();
    }
    Serial.println("File system initialization successfully");

    auto file = SPIFFS.open(testFileName, FILE_WRITE, true);
    if (!file)
    {
        Serial.println("File opening failed in write mode");
        ESP.restart();
    }
    Serial.println("The file is successfully opened in write mode");

    file.println("Line 1: xxxxxxx");
    file.println("Line 2: yyyyyy");
    file.println("Line 3: zzzzzzz");

    file.flush();
    file.close();


    file = SPIFFS.open(testFileName, FILE_READ, false);
    if (!file)
    {
        Serial.println("File opening failed in read mode");
        ESP.restart();
    }
    Serial.println("The file is successfully opened in read mode");
    Serial.printf("All contents of the file: %s\n", file.readString().c_str());

    file.seek(0);
    int line = 1;
    while (file.available())
    {
        Serial.printf("%d line content: %s\n", line++, file.readStringUntil('\n').c_str());
    }
    file.close();

    file = SPIFFS.open(testFileName, FILE_APPEND, false);
    if (!file)
    {
        Serial.println("File failed to open in append mode");
        ESP.restart();
    }
    Serial.println("The file is successfully opened in append mode");

    file.println("Line 4: aaaaa");
    file.flush();
    file.close();


    file = SPIFFS.open(testFileName, FILE_READ, false);
    if (!file)
    {
        Serial.println("File opening failed in read mode");
        ESP.restart();
    }
    Serial.println("The file is successfully opened in read mode");
    Serial.printf("All contents of the file: %s\n", file.readString().c_str());

    file = SPIFFS.open("/rose.jpg", FILE_READ, false);
    if (!file)
    {
        Serial.println("File opening failed in read mode");
        ESP.restart();
    }
    Serial.println("The file is successfully opened in read mode");
    Serial.printf("File size: %d\n", file.size());
}

void loop()
{
}
