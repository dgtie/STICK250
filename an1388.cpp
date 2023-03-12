#define BUF_SIZE 64

#define PFM_START   0x1d000000
#define PFM_END     0x1d01c000
#define EXCPT_START 0x1d01f000
#define EXCPT_END   0x1d020000
#define BFM_START   0x1fc00400
#define BFM_END     0x1fc00bf0

#define FRAME_SOH           0x01
#define FRAME_EOT           0x04
#define FRAME_DLE           0x10

#define CMD_READ_VERSION    0x01
#define CMD_ERASE_FLASH     0x02
#define CMD_PROGRAM_FLASH   0x03
#define CMD_READ_CRC        0x04
#define CMD_JUMP_APP        0x05

bool HIDReportRxd(void), HIDReportTxd(void);
void HIDRxReport(void);
void HIDTxReport(unsigned char *buf);
unsigned NVMErasePage(void*);
unsigned set_row_address(unsigned), save_row_data(unsigned char);
unsigned flush_row(unsigned);
void clear_row(unsigned);
unsigned get_size(void), get_address(void), verify_hex(unsigned char*);
unsigned char *get_data(void);
void jump_app(void);
void toggle_LED(void);

unsigned char inbuffer[BUF_SIZE];	// from host
unsigned char outbuffer[BUF_SIZE];	// to host

namespace
{

unsigned char rawbuffer[BUF_SIZE];
unsigned row[32];
unsigned answer_length, erase_address;

const unsigned short crc_table[16] = {
  0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
  0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef
};

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

unsigned read_inbuffer(void) {
  int j = 1;
  if (inbuffer[0] == FRAME_SOH) 
    for (int i = 0; i < BUF_SIZE; i++) {
      rawbuffer[i] = inbuffer[j++];
      if (rawbuffer[i] == FRAME_EOT) return i - 2;
      if (rawbuffer[i] == FRAME_DLE) rawbuffer[i] = inbuffer[j++];
    }
  return 0;
}

void write_outbuffer(unsigned length) {
  int j = 1;
  outbuffer[0] = FRAME_SOH;
  for (int i = 0; i < length; i++) {
    if (rawbuffer[i] == FRAME_SOH) outbuffer[j++] = FRAME_DLE;
    if (rawbuffer[i] == FRAME_EOT) outbuffer[j++] = FRAME_DLE;
    if (rawbuffer[i] == FRAME_DLE) outbuffer[j++] = FRAME_DLE;
    outbuffer[j++] = rawbuffer[i];
  }
  outbuffer[j] = FRAME_EOT;
}

void program_flash(void) {
  unsigned u = verify_hex(&rawbuffer[1]);
  if (u == 1) flush_row(0);
  if (u > 1) {
    unsigned s = get_size();
    if (s) {
      unsigned char *p = get_data();
      set_row_address(get_address());
      for (int i = 0; i < s; i++) save_row_data(p[i]);
    }
  }
}

unsigned read_crc(void) {
  unsigned *p = (unsigned*)&rawbuffer[1];
  unsigned adrs = *p++ | 0x80000000;
  unsigned nbytes = *p;
  p = (unsigned*)&outbuffer[10];
  *p++ = adrs; *p = nbytes;
  return calculate_crc(0, (unsigned char*)adrs, nbytes);
}

} //anonymous

void ProcessIO(void) {
  unsigned temp;
  if (erase_address) {
    NVMErasePage((void*)erase_address);
    if (erase_address == BFM_START) erase_address = EXCPT_START;
    else erase_address += 0x400;
    if (erase_address == EXCPT_END) erase_address = PFM_START;
    if (erase_address == PFM_END) clear_row(erase_address = 0);
    else return;
  }
  if (answer_length) {
    temp = calculate_crc(0, rawbuffer, answer_length);
    rawbuffer[answer_length++] = temp & 255;
    rawbuffer[answer_length++] = temp >> 8;
    write_outbuffer(answer_length);
    answer_length = 0;
    HIDTxReport(outbuffer);
  }
  if (HIDReportRxd()) {
    unsigned length = read_inbuffer();
    temp = calculate_crc(0, rawbuffer, length);
    if (temp == (rawbuffer[length] | (rawbuffer[length + 1] << 8))) {
      answer_length = 3;
      switch(rawbuffer[0]) {
      case CMD_READ_VERSION: rawbuffer[1] = 1; rawbuffer[2] = 0; break;
      case CMD_ERASE_FLASH: erase_address = BFM_START; answer_length = 1;
                            break;
      case CMD_PROGRAM_FLASH: program_flash(); answer_length = 1; break;
      case CMD_READ_CRC: temp = read_crc(); rawbuffer[1] = temp & 255;
                         rawbuffer[2] = temp >> 8; break;
      case CMD_JUMP_APP: toggle_LED(); jump_app();
      default: answer_length = 0;
      }
    }
    HIDRxReport();
  }
}
