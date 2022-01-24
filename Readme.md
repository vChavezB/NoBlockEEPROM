# NoBlockEEPROM

NoBlockEEPROM is a library intended to avoid blocking code execution when Writing/Reading to the integrated EEPROM peripheral of some AVR MCUs supported by the Arduino SDK.

### Supported Boards

- Arduino Uno/Nano

  Note: In theory any AVR board with an EEPROM peripheral is supported as ATMEL MCUs have typically the same register sets. If you find that any other board works with this library you can add it to the list through a pull-request.

### Motivation

The Arduino SDK uses the avr-libc EEPROM API which in turn are blocking functions. This library uses the interrupt vector to write to the EEPROM asynchronously. Whenever a write operation is done, a callback can be set to notify of this event.



### How to Use

Check `NoBlockEEPROM\examples\WriteRead` for an example which Writes and Reads in non-blocking mode.

### Contribution

If you find any issue or have made improvements to this library you can submit a pull-request.



### License

GPLV3