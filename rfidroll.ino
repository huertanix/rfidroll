/**************************************************************************/
/*! 
    @file     rfidroll.pde
    @author   David Huerta but mostly Adafruit Industries
    @license  MIT (see license.txt)

    This sketch will turn Mifare classic tags into weapons of mass tROLLing
    Based pretty much completely on Adafruit's sample mifare classic format 
    ndef example sketch, which can let you do more than rickroll Android users:
    https://github.com/adafruit/Adafruit_NFCShield_I2C/blob/master/examples/mifareclassic_formatndef/mifareclassic_formatndef.pde
    
    This is an example sketch for the Adafruit PN532 NFC/RFID breakout boards
    This library works with the Adafruit NFC Shield 
      ----> https://www.adafruit.com/products/789
 
    Adafruit invests time and resources providing this open source code, 
    please support Adafruit and open-source hardware by purchasing 
    products from Adafruit!

*/
/**************************************************************************/

#include <Wire.h>
#include <Adafruit_NFCShield_I2C.h>

#define IRQ   (2)
#define RESET (3)  // Not connected by default on the NFC Shield

Adafruit_NFCShield_I2C nfc(IRQ, RESET);

const char * url = "youtube.com/watch?v=dQw4w9WgXcQ";
uint8_t ndefprefix = NDEF_URIPREFIX_HTTP_WWWDOT;

void setup(void) {
  Serial.begin(115200);
  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
    
  // configure board to read RFID tags
  nfc.SAMConfig();
}

void loop(void) {
  uint8_t success;                          // Flag to check if there was an error with the PN532
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  bool authenticated = false;               // Flag to indicate if the sector is authenticated

  // Use the default key
  uint8_t keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

  Serial.println("");
  Serial.println("Place tag near reader");
  Serial.println("and press any key.");
  // Wait for user input before proceeding
  while (!Serial.available());
  // a key was pressed1
  while (Serial.available()) Serial.read();
    
  // Wait for an ISO14443A type card (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  
  if (success) {
    // Display some basic information about the card
    Serial.println("Found a tag");
    Serial.print("  UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
    Serial.print("  UID Value: ");
    nfc.PrintHex(uid, uidLength);
    Serial.println("");
    
    // Make sure this is a Mifare Classic card
    if (uidLength != 4) {
      Serial.println("Oops ... this doesn't seem to be a Mifare Classic card!"); 
      return;
    }
    
    // We probably have a Mifare Classic card ... 
    Serial.println("Seems to be a Mifare Classic card (4 byte UID)");

    // Try to format the card for NDEF data
    success = nfc.mifareclassic_AuthenticateBlock (uid, uidLength, 0, 0, keya);
    if (!success) {
      Serial.println("Unable to authenticate block 0 to enable card formatting!");
      return;
    }
    
    success = nfc.mifareclassic_FormatNDEF();
    if (!success) {
      Serial.println("Unable to format the card for NDEF");
      return;
    }
    
    // Try to authenticate block 4 (first block of sector 1) using our key
    success = nfc.mifareclassic_AuthenticateBlock (uid, uidLength, 4, 0, keya);

    // Make sure the authentification process didn't fail    
    if (!success) {
      Serial.println("Authentication fail.");
      return;
    }

    // URI is within size limits ... write it to the card and report success/failure
    success = nfc.mifareclassic_WriteNDEFURI(1, ndefprefix, url);
    if (success) {
      Serial.println("Card has been formatted to rickroll for great justice");
    }
    else {
      Serial.println("FAIL");
    }
  }
}
