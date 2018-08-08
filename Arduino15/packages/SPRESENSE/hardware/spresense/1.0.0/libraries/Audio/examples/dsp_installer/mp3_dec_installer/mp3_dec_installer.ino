/*
 *  mp3_dec_installer.ino - MP3 decoder dsp installer
 *  Copyright 2018 Sony Semiconductor Solutions Corporation
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *  The DSP installer is used to load code for the Audio DSP into SD Card or SPI-Flash.
 *  This needs to be done ahead of using the Audio DSP functions in any sketch.
 *  A range of Audio DSP functions are available. This skecth loads MP3 decoder.
 *
 *  Run the sketch and you should see
 *    Select where to install MP3DEC?
 *     [1]: SD Card (Insert SD Card on the extension board)
 *     [2]: SPI-Flash
 *  After the above message appeared, input a numeral number from Serial Monitor.
 *  If '1' is input,
 *    Install: /mnt/sd0/BIN/MP3DEC Done.
 *  If '2' is input,
 *    Install: /mnt/spif/BIN/MP3DEC Done.
 *    Finished.
 *
 *  The loading process only needs to be done once as the DSP code is retained when up upload a new sketch.
 *
 *  To check what DSP files are loaded is a board you have already programmed you can use NuttShell
 *  - To list the files installed in the SPI-Flash. Check the directory /mnt/spif/BIN.
 *  - To list the files installed in the SD Card. Check the directory /mnt/sd0/BIN
 *    or put the SD card in a PC and check the SD card /BIN directory.
 *
 *  You can use dsp_inataller with a range of encoders and decoders as they become available.
 *
 *  This example code is in the public domain.
 */

#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

// Include each file generated by bin2c
#include "MP3DEC.h"

#define DSP_MOUNTPT_SDCARD   "/mnt/sd0/BIN"
#define DSP_MOUNTPT_SPIFLASH "/mnt/spif/BIN"

#define _FILEELEM(elem) { \
  .name = #elem, \
  .addr = elem##_start, \
  .size = &elem##_size, \
}

struct fileinfo_s {
  const char *name;
  const unsigned char *addr;
  const size_t *size;
};

struct fileinfo_s dsplist[] =
{
  // Add each file here
  _FILEELEM(MP3DEC),
};

void setup()
{
  unsigned int i;
  int ret;
  FILE *fp;
  char dirpath[64] = {0};
  char filepath[64] = {0};
  enum {INTO_SD, INTO_FLASH} where;

  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect.
  }

  // Display Menu
  Serial.println("Select where to install MP3DEC?");
  Serial.println(" [1]: SD Card (Insert SD Card on the extension board)");
  Serial.println(" [2]: SPI-Flash");

  // User Menu Selection
  int c;
  while (true) {
    Serial.println("");
    Serial.print("Please input number. [1-2] ? ");
    while (!Serial.available());
    c = Serial.read();
    while (Serial.available()) {
      Serial.read(); // Discard inputs except for 1st character
    }

    if (c == '1') {
      Serial.println("1 -> SD Card");
      where = INTO_SD;
      break;
    } else if (c == '2') {
      Serial.println("2 -> SPI-Flash");
      where = INTO_FLASH;
      break;
    } else {
      if ((c != '\n') && (c != '\r'))
        Serial.write(c);
    }
  }

  // If install to SD Card, check whether the SD Card is inserted.
  if (where == INTO_SD) {
    struct stat buf;

    strncpy(dirpath, DSP_MOUNTPT_SDCARD, sizeof(dirpath));
    for (;;) {
      ret = stat("/mnt/sd0", &buf);
      if (ret) {
        Serial.println("Please insert formatted SD Card.");
        sleep(1);
      } else {
        break;
      }
    }
  } else if (where == INTO_FLASH) {
    strncpy(dirpath, DSP_MOUNTPT_SPIFLASH, sizeof(dirpath));
  }

  mkdir(dirpath, 0777);

  for (i = 0; i < sizeof(dsplist) / sizeof(dsplist[0]); i++)
    {
      snprintf(filepath, sizeof(filepath), "%s/%s",
               dirpath, dsplist[i].name);

      Serial.print("Install: ");
      Serial.print(filepath);

      unlink(filepath);

      fp = fopen(filepath, "wb");

      ret = fwrite(dsplist[i].addr, *dsplist[i].size, 1, fp);

      Serial.println((ret) ? " Done." : " Fail.");

      fclose(fp);
    }

  Serial.println("Finished.");
}

void loop()
{
  // nothing happens after setup finishes.
}