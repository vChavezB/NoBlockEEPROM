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

#ifndef NoBlkEEPROM_H
#define NoBlkEEPROM_H

#include <stdint.h>
#include <avr/interrupt.h>

extern "C" void EE_READY_vect(void);

class NoBlkEEPROMClass
{
  public:
    /*!< Results for using eeprom public operation API */
    enum EEPROMResult
    {
      eeprom_ok,                /*!< EEPROM operation ok */
      eeprom_busy,              /*!< EEPROM is already busy with a transaction */
      eeprom_addr_ovflw,        /*!< Combination of address + len would cause overflow */
      eeprom_addr_range_err     /*!< Addres not within range */
    };
    /*!< Allow only one instance */
    static NoBlkEEPROMClass & instance()
    {
      static NoBlkEEPROMClass NoBlkEEPROM;
      return NoBlkEEPROM;
    }
    /*!< Callback function type*/
    typedef void (*WriteCallback)(void *);
    /* @brief Initializes the EEPROM Peripheral
       @note Should be only called once
    */
    void Begin();
    /* @brief Sets callback for asynch events from EEPROM
       @details Whenever a EEPROM operation is finished,
                the callback that has been passed to this function
                will be called asynchronously.
                @param[in] cb The callback function to be set
                @param[in] args Pointer to address of arguments to be passed to the callback
      @note Keep in mind that the callback is called in an ISR, so all
            best practices regarding code execution in an ISR should be followed.
    */
    void SetCallback(WriteCallback cb, void * args);
    /* @brief Write a byte to the EEPROM
                @param[in] addr EEPROM Address
                @param[in] data 8 bit data to write to eeprom 
      @retval eeprom_ok: Operation was succesful, operation done notified via callback @ref SetCallback
    */
    EEPROMResult Write(uint16_t addr, uint8_t data);
    /* @brief Write a buffer to the EEPROM
                @param[in] addr   EEPROM Address
                @param[in] buffer Pointer to data buffer to write
                @param[in] len    Length of the buffer
      @note The buffer used to intiate the write operation shall not change while the operation is not finished.
            The reason is that there is no internal buffer implemented
      @retval eeprom_ok: Operation was succesful, operation done notified via callback @ref SetCallback
    */
    EEPROMResult Write(uint16_t addr, const uint8_t * buffer, uint8_t len);
    /* @brief Read n bytes from the EEPROM
                @param[in] addr       EEPROM Address to read
                @param[out] buff_out  Pointer to buffer where read operations will be saved
                @param[in] len        Total bytes to read
      @note This function is non-blocking, in case the eeprom is busy it will return eeprom_busy
      @retval eeprom_ok: Operation was succesful, operation done notified via callback @ref SetCallback
    */
    EEPROMResult Read(uint16_t addr,uint8_t * buff_out,uint8_t len);
    /* @brief Read one byte of data from the EEPROM
                @param[in] addr       EEPROM Address to read
                @param[out] buff_out  Pointer to to 8 bit buffer to store read operation
      @note This function is non-blocking, in case the eeprom is busy it will return eeprom_busy
      @retval eeprom_ok: Operation was succesful, operation done notified via callback @ref SetCallback
    */
    EEPROMResult Read(uint16_t addr,uint8_t * buff_out);
  private:
    NoBlkEEPROMClass() {};
    /* @brief Write a byte to the EEPROM peripheralusing the MCU registers
                @param[in] addr   EEPROM Address
                @param[in] data   8 bit data to write to eeprom 
    */
    void WriteByte(uint16_t addr, uint8_t data);

    uint8_t ReadByte(uint16_t addr);
    /*!< Pointer to callback arguments */
    void * cb_args;
    /*!< Callback function used when write operations are done */
    WriteCallback write_cb;
    /*!< Data required for EEPROM non blocking transfers */
    struct Transfer
    {
      const uint8_t * buff; /*!< Pointer to address that holds the buffer to write */
      uint16_t addr;        /*!< EEPROM */
      uint8_t cnt;
      uint8_t len;
    };
    Transfer transfer;
    /*!<EEPROM Programming modes according to Table 7-1 datasheet */
    enum PRGMode 
    {
      EraseWrite = 0,
      EraseOnly = 1,
      WriteOnly = 2
    };
    /*!< Vector table function for EEPROM Ready interrupt */
    friend void EE_READY_vect(void);
};

extern NoBlkEEPROMClass & NoBlkEEPROM;
#endif
