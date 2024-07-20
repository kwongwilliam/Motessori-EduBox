

/*
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 *
 * More pin layouts for other boards can be found here: https://github.com/miguelbalboa/rfid#pin-layout
 */

#include <SPI.h>
#include <MFRC522.h>
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

// Use pins 2 and 3 to communicate with DFPlayer Mini
static const uint8_t PIN_MP3_TX = 2;  // Connects to module's RX
static const uint8_t PIN_MP3_RX = 3;  // Connects to module's TX
SoftwareSerial softwareSerial(PIN_MP3_RX, PIN_MP3_TX);

#define RST_PIN 9  // Configurable, see typical pin layout above
#define SS_PIN 10  // Configurable, see typical pin layout above
#define BUSY_PIN 4
#define volume_up 6
#define volume_down 8
#define switch_mode 5
#define next_song 7


// Create the Player object
DFRobotDFPlayerMini player;

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance
String accessUID[] = { "", "53 31 13 5C 41 00 01", "53 B3 29 5B 41 00 01", "53 A3 25 5B 41 00 01", "53 9F 21 5B 41 00 01", "53 C9 2D 5B 41 00 01", "53 7A 3A 5B 41 00 01" };


int main_len = sizeof(accessUID) / sizeof(accessUID[0]);

int mode = 1;

void setup() {
  pinMode(BUSY_PIN, INPUT);
  pinMode(volume_up, INPUT);
  pinMode(volume_down, INPUT);
  pinMode(switch_mode, INPUT);
  pinMode(next_song, INPUT);


  Serial.begin(9600);  // Initialize serial communications with the PC
  softwareSerial.begin(9600);
  while (!Serial)
    ;  // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  while (!player.begin(softwareSerial)) {
    delay(300);
  }
  SPI.begin();         // Init SPI bus
  mfrc522.PCD_Init();  // Init MFRC522
  delay(4);            // Optional delay. Some board do need more time after init to be ready, see Readme
  // mfrc522.PCD_DumpVersionToSerial();	// Show details of PCD - MFRC522 Card Reader details
  Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));
  player.play(7);
  mode = 1;
  Serial.println(mode);
}

void loop() {

  Serial.println(digitalRead(switch_mode));
  if ((digitalRead(switch_mode)) == 1) {
    if (mode < 3) {
      mode++;
    } else if (mode == 3) {
      mode = 1;
    }
    Serial.println(mode);
    if (mode == 1) {
      player.play(7);
      delay(3000);
    }
    if (mode == 2) {
      player.play(8);
      delay(3000);
      player.loopFolder(2);  //loop all mp3 files in folder SD:/02.
    } else if (mode == 3) {
      player.play(9);
      delay(3000);
      player.loopFolder(3);  //loop all mp3 files in folder SD:/03.
    }
  }


  if ((digitalRead(volume_up)) == 1) {
    player.volumeUp();  //Volume Up
  }
  if ((digitalRead(volume_down)) == 1) {
    player.volumeDown();  //Volume Down
  }
  // Animal mode
  if (mode == 1) {
    // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
    if (!mfrc522.PICC_IsNewCardPresent()) {
      return;
    }

    // Select one of the cards
    if (!mfrc522.PICC_ReadCardSerial()) {
      return;
    }

    String animal = "";
    int play_index = 0;

    // read animals
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
      Serial.print(mfrc522.uid.uidByte[i], HEX);
      animal.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
      animal.concat(String(mfrc522.uid.uidByte[i], HEX));
    }

    // convert to uppercase
    animal.toUpperCase();

    for (int i = 0; i < main_len; i++) {
      if (animal.substring(1) == accessUID[i]) {
        play_index = i;
        Serial.println(play_index);
      }
    }

    Serial.println("OK");
    for (int i = 0; i < 10; i++) {
      player.volume(23);
      player.EQ(DFPLAYER_EQ_JAZZ);

      player.play(play_index);
      delay(500);
      if (digitalRead(BUSY_PIN) == 0) {
        // Serial.println(digitalRead(BUSY_PIN));
        break;
      }
    }

    Serial.println("");

    // PICC_HaltA() is automatically called
    mfrc522.PICC_HaltA();
    play_index = 0;
    animal = "";
  }
  // Happy mode
  else if (mode == 2) {
    if ((digitalRead(next_song)) == 1) {
      player.next();
    }
  }
  // sleep mode
  else if (mode == 3) {
    if ((digitalRead(next_song)) == 1) {
      player.next();
    }
  }
  delay(1000);
}