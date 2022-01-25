#include <EEPROM.h>
#include "NoBlockEEPROM.h"

bool EEPROM_WriteFinished = false;

void EEPROMFinishedCB(void * args)
{
  EEPROM_WriteFinished = true;
}
constexpr int buffer_size = 100;
uint8_t write_buffer[buffer_size];
constexpr uint8_t write_addr = 0;
uint32_t write_start;

void setup() {
  Serial.begin(115200);
  NoBlkEEPROM.Begin();
  NoBlkEEPROM.SetCallback(EEPROMFinishedCB, nullptr);
  //Initialize buffer before writing with random values
  randomSeed(analogRead(0));
  for (auto i = 0; i < buffer_size; i++)
  {
    write_buffer[i] =  random(0, 0xFF);
  }
  Serial.println("Write started");
  write_start = millis();

  NoBlkEEPROMClass::EEPROMResult result;
  result = NoBlkEEPROM.Write(write_addr, write_buffer, buffer_size);
  if (result != NoBlkEEPROMClass::eeprom_ok)
  {
    Serial.print("1st EEPROM Write failed: ");
    Serial.println(result, HEX);
    while(1);
  }
  //Second write should fail as eeprom is busy
  result = NoBlkEEPROM.Write(write_addr, random(0, 0xFF));
  if (result != NoBlkEEPROMClass::eeprom_busy)
  {
    Serial.print("2nd EEPROM Write failed: ");
    Serial.println(result, HEX);
    while(1);
  }
}

void loop() {

  if (EEPROM_WriteFinished == true)
  {
    EEPROM_WriteFinished = false;
    uint32_t write_end = millis();
    Serial.print("Operation took :");
    Serial.print(write_end - write_start);
    Serial.println(" ms");
    uint8_t read_buff[buffer_size];
    Serial.println("Reading buffer");
    NoBlkEEPROM.Read(0, read_buff, buffer_size);
    int compare_res = memcmp(read_buff, write_buffer, buffer_size);
    if (compare_res != 0)
    {
      Serial.println("Error Write Operation failed");
    }
    else
    {
      Serial.println("Write/Read Ok");
    }
  }

}
