/*
Copyright (C) 2022 Victor Chavez
This file is part of NoBlockEEPROM

NoBlockEEPROM is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
IOLink Device Generator is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with IOLink Device Generator.  If not, see <https://www.gnu.org/licenses/>.

*/

#include "NoBlockEEPROM.hpp"
#include "Arduino.h"
#include <util/atomic.h>

#define MAX_ADR (1023) /*!<Max EEPROM Address for Arduino UNO */

NoBlkEEPROMClass& NoBlkEEPROM = NoBlkEEPROMClass::instance();

void NoBlkEEPROMClass::Begin()
{
  EECR = (static_cast<uint8_t>(EraseWrite) << EEPM0); //Programming mode Write+Erase

}
void NoBlkEEPROMClass::SetCallback(WriteCallback cb, void * args)
{
  if (cb != nullptr)
  {
    write_cb = cb;
    cb_args = args;
  }
}

void NoBlkEEPROMClass::WriteByte(uint16_t addr, uint8_t data)
{
  EEAR = addr ;
  EEDR = data;
  EECR |= (1 << EEMPE); //Write Enable
  EECR |= ( (1U << EEPE)   //Start eeprom write
            | (1U << EERIE) ); //Enable Interrupt
}

uint8_t NoBlkEEPROMClass::ReadByte(uint16_t addr)
{
  EEAR = addr ;
  /* Start eeprom read by writing EERE */
  EECR |= (1 << EERE);
  return EEDR;
}

NoBlkEEPROMClass::EEPROMResult NoBlkEEPROMClass::Write(uint16_t addr, const uint8_t * buffer, uint8_t len)
{
  EEPROMResult result;
  do
  {
    ATOMIC_BLOCK(ATOMIC_FORCEON) //Do not allow concurrent calls
    {
      if (addr > MAX_ADR || (addr + len > MAX_ADR))
      {
        result = EEPROMResult::eeprom_addr_ovflw;
        break;
      }
      if ( EECR & (1 << EEPE) )
      {
        result = EEPROMResult::eeprom_busy;
        break;
      }
      transfer.len = len;
      transfer.cnt = 0;
      transfer.buff = buffer;
      transfer.addr = addr;
      WriteByte(transfer.addr, *transfer.buff);
      result = EEPROMResult::eeprom_ok;
    }
  } while (0);
  return result;
}


NoBlkEEPROMClass::EEPROMResult NoBlkEEPROMClass::Write(uint16_t addr, uint8_t data)
{
  return Write(addr, &data, 1);
}


NoBlkEEPROMClass::EEPROMResult NoBlkEEPROMClass::Read(uint16_t addr, uint8_t * buff_out)
{
  return Read(addr, buff_out, 1);
}

NoBlkEEPROMClass::EEPROMResult NoBlkEEPROMClass::Read(uint16_t addr, uint8_t * buff_out, uint8_t len)
{

  EEPROMResult result;
  do
  {
    ATOMIC_BLOCK(ATOMIC_FORCEON) //Do not allow concurrent calls
    {
      if (addr > MAX_ADR || (addr + len > MAX_ADR))
      {
        result = EEPROMResult::eeprom_addr_ovflw;
        break;
      }
      if ( EECR & (1 << EEPE) )
      {
        result = EEPROMResult::eeprom_busy;
        break;
      }
      for (auto i = 0; i < len; i++)
      {
        buff_out[i] =  ReadByte(addr + i);
      }
      result = EEPROMResult::eeprom_ok;
    }
  } while (0);
  return result;
}

ISR(EE_READY_vect)
{
  NoBlkEEPROM.transfer.cnt++;
  if (NoBlkEEPROM.transfer.cnt == NoBlkEEPROM.transfer.len)
  {
    //Transfer finished
    EECR &= ~(1U << EERIE);  //Disable interrupt
    if (NoBlkEEPROM.write_cb != nullptr)
    {
      NoBlkEEPROM.write_cb(NoBlkEEPROM.cb_args);
    }
  }
  else
  {
    //Continue writing to eeprom
    NoBlkEEPROM.transfer.addr++;
    NoBlkEEPROM.transfer.buff++;
    NoBlkEEPROM.WriteByte(NoBlkEEPROM.transfer.addr, *NoBlkEEPROM.transfer.buff);
  }
}
