#include <stdio.h>
#include <hidapi/hidapi.h>

#define BUF_SIZE 64

#define FRAME_SOH           0x01
#define FRAME_EOT           0x04
#define FRAME_DLE           0x10

#define CMD_READ_VERSION    0x01	// 33, FRAME_DLE, 16
#define CMD_ERASE_FLASH     0x02	// 66, 32
#define CMD_PROGRAM_FLASH   0x03
#define CMD_READ_CRC        0x04
#define CMD_JUMP_APP        0x05	// 165, 80

#define PFM_START   0x1d000000
#define PFM_END     0x1d01c000
#define EXCPT_START 0x1d01f000
#define EXCPT_END   0x1d020000
#define BFM_START   0x1fc00400
#define BFM_END     0x1fc00bf0

unsigned verify_hex(unsigned char *buf);
unsigned pop_row_crc(unsigned &crc);	// return address

hid_device *handle;

unsigned char cmd_read_version[] = { CMD_READ_VERSION, 33, 16 };
unsigned char cmd_erase_flash[] = { CMD_ERASE_FLASH, 66, 32 };
unsigned char cmd_jump_app[] = { CMD_JUMP_APP, 165, 80 };

unsigned char buf[BUF_SIZE];
unsigned char raw[BUF_SIZE];

const unsigned short crc_table[16] = {
  0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
  0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef
};

int frame(unsigned char* dest, unsigned char* src, unsigned length) {
  int j = 1;
  dest[0] = FRAME_SOH;
  for (int i = 0; i < length; i++) {
    if (src[i] == FRAME_SOH) dest[j++] = FRAME_DLE;
    if (src[i] == FRAME_EOT) dest[j++] = FRAME_DLE;
    if (src[i] == FRAME_DLE) dest[j++] = FRAME_DLE;
    dest[j++] = src[i];
  }
  dest[j++] = FRAME_EOT;
  return j;
}

unsigned unframe(unsigned char* dest, unsigned char* src) {
  int j = 1;
  if (src[0] == FRAME_SOH)
    for (int i = 0; i < BUF_SIZE; i++) {
      dest[i] = src[j++];
      if (dest[i] == FRAME_EOT) return i - 2;
      if (dest[i] == FRAME_DLE) dest[i] = src[j++];
    }
  return 0;
}

unsigned calculate_crc(unsigned crc, unsigned char *data, unsigned nbytes) {
  unsigned i;
  while (nbytes--) {
    i = (crc >> 12) ^ (*data >> 4);
    crc = crc_table[i & 15] ^ (crc << 4);
    i = (crc >> 12) ^ *data++;
    crc = crc_table[i & 15] ^ (crc << 4);
  }
  return crc & 0xffff;
}

bool erase_flash(void) {
  unsigned length, crc;
  frame(buf, cmd_erase_flash, 3);
  hid_write(handle, buf, BUF_SIZE);
  hid_read(handle, buf, BUF_SIZE);
  length = unframe(raw, buf);
  if (length) {
    crc = calculate_crc(0, raw, length); 
    if (crc != (raw[length] | (raw[length + 1] << 8))) length = 0;
  }
  return length;
}

bool check_address(unsigned addr) {
  if (addr < PFM_START) return false;
  if (addr < PFM_END) return true;
  if (addr < EXCPT_START) return false;
  if (addr < EXCPT_END) return true;
  if (addr < BFM_START) return false;
  if (addr < BFM_END) return true;
  return false;
}

int main(int argc, char* argv[]) {
  int res;
  unsigned length, temp;
  FILE *fp = NULL;
  printf("\nBootloader Application for STICK250\n");
  printf("--PIC32MX250F128D: switch at B8, LED at B9\n");
  printf("  Copyright: (C) 2023 Lams Workshop\n");
  if (argc > 1) {
    fp = fopen(argv[1], "r");
    if (!fp) {
      printf("cannot open file %s\n", argv[1]);
      return 1;
    }
  }
  res = hid_init();
  handle = hid_open(0x4d8, 0x3c, NULL);
  if (!handle) {
    printf("Unable to open STICK250\n");
    printf("--Press switch and then click reset button\n");
    hid_exit();
    if (fp) fclose(fp);
    return 1;
  }
  frame(buf, cmd_read_version, 3);
  res = hid_write(handle, buf, BUF_SIZE);
  res = hid_read(handle, buf, BUF_SIZE);
  length = unframe(raw, buf);
  printf("AN1388 HID Bootloader Version %d.%d\n", raw[1], raw[2]);
  if (length) {
    temp = calculate_crc(0, raw, length); 
    if (temp != (raw[length] | (raw[length + 1] << 8))) length = 0;
  }
  if (!length) {
    printf("Unable to read version\n");
    hid_close(handle);
    hid_exit();
    if (fp) fclose(fp);
    return 1;
  }
  if (fp) {

//*** ERASE FLASH *****************************************************
    printf("Erase flash: "); fflush(stdout);
    if (!erase_flash()) {
      printf("Unable to erase flash\n");
      hid_close(handle);
      hid_exit();
      fclose(fp);
      return 1;
    }
    printf("done\n");
// ** ERASE FLASH ****************************************************/

    printf("Program flash: "); fflush(stdout);
    raw[0] = CMD_PROGRAM_FLASH;
    while(fgets((char*)&raw[1], BUF_SIZE - 1, fp)) {
      length = verify_hex(&raw[1]);
      if (length) {
        temp = calculate_crc(0, raw, ++length);
        raw[length++] = temp & 255;
        raw[length++] = temp >> 8;
        frame(buf, raw, length);
//*** PROGRAM FLASH ***************************************************
        res = hid_write(handle, buf, BUF_SIZE);
        res = hid_read(handle, buf, BUF_SIZE);
        printf("."); fflush(stdout);
        length = unframe(raw, buf);
        if (length) {
          temp = calculate_crc(0, raw, length); 
          if (temp != (raw[length] | (raw[length + 1] << 8))) length = 0;
        }
        if (!length) {
          printf("Unable to write flash\n");
          erase_flash();
          hid_close(handle);
          hid_exit();
          fclose(fp);
          return 1;
        }
// ** PROGRAM FLASH **************************************************/
      } else {
        printf("Not an HEX file\n");
        hid_close(handle);
        hid_exit();
        fclose(fp);
        return 1;
      }
    }
    printf("done\n");

//*** READ CRC ********************************************************
    printf("Verify flash: "); fflush(stdout);
    raw[0] = CMD_READ_CRC;
    unsigned crc;
    while ((temp = pop_row_crc(crc))) {
      if (!check_address(temp)) {
        printf("\nCodes outside usable zone are not written");
        continue;
      }
      raw[1] = temp & 255; temp >>=8;
      raw[2] = temp & 255; temp >>=8;
      raw[3] = temp & 255; temp >>=8;
      raw[4] = temp & 255;
      raw[5] = 128; raw[6] = raw[7] = raw[8] = 0;
      temp = calculate_crc(0, raw, 9);
      raw[9] = temp & 255;
      raw[10] = temp >> 8;
      frame(buf, raw, 11);
      res = hid_write(handle, buf, BUF_SIZE);
      res = hid_read(handle, buf, BUF_SIZE);
      length = unframe(raw, buf);
      if (length) {
        temp = calculate_crc(0, raw, length); 
        if (temp != (raw[length] | (raw[length + 1] << 8))) length = 0;
      }
      if (!length) {
        printf("Unable to read crc\n");
        erase_flash();
        hid_close(handle);
        hid_exit();
        fclose(fp);
        return 1;
      }
      if (crc != (raw[1] | (raw[2] << 8))) {
        printf("crc not match\n");
        erase_flash();
        hid_close(handle);
        hid_exit();
        fclose(fp);
        return 1;
      }
      printf("."); fflush(stdout);
    }
    printf("done\n");
// ** READ CRC *******************************************************/

//*** JUMP APP ********************************************************
    frame(buf, cmd_jump_app, 3);
    res = hid_write(handle, buf, BUF_SIZE);
    printf("Start application\n");
// ** JUMP APP *******************************************************/

  }
//  for (int i = 0; i < length; i++) printf("raw[%d]: %d\n", i, raw[i]);
  hid_close(handle);
  res = hid_exit();
  if (fp) fclose(fp);
  return 0;
}

//sudo apt install libhidapi-dev
//gcc main.c -lhidapi-libusb
