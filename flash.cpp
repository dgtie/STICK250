#include <xc.h>

#define PFM_START   0x1d000000
#define PFM_END     0x1d01c000
#define EXCPT_START 0x1d01f000
#define EXCPT_END   0x1d020000
#define BFM_START   0x1fc00400
#define BFM_END     0x1fc00bf0

namespace
{

unsigned char row[128] __attribute__((aligned(32)));
unsigned address, r_index;

unsigned NVMUnlock(unsigned nvmop) {
  NVMCON = nvmop;
  NVMKEY = 0xaa996655;
  NVMKEY = 0x556699aa;
  NVMCONSET = 0x8000;
  while (NVMCON & 0x8000);
  NVMCONCLR = 0x4000;
  return NVMCON & 0x3000;
}

unsigned NVMWriteRow(void* addr, void* data) {
  unsigned res;
  NVMADDR = (unsigned)addr;
  NVMSRCADDR = (unsigned)data & 0x1FFFFFFF;
  res = NVMUnlock(0x4003);
  return res;
}

bool check(unsigned addr) {
  if (addr < PFM_START) return false;
  if (addr < PFM_END) return true;
  if (addr < EXCPT_START) return false;
  if (addr < EXCPT_END) return true;
  if (addr < BFM_START) return false;
  if (addr < BFM_END) return true;
  return false;
}

}//anonymous

unsigned NVMErasePage(void* addr) {
  unsigned res;
  NVMADDR = (unsigned)addr;
  res = NVMUnlock(0x4004);
  return res;
}

void clear_row(unsigned addr) {
  address = addr;
  for (int i = 0; i < 128; i++) row[i] = 0xff;
}

unsigned flush_row(unsigned addr) {
  unsigned i = address;
  if (check(address)) NVMWriteRow((void*)address, row);
  clear_row(addr);
  return i;
}

unsigned set_row_address(unsigned a) {
  unsigned ra = a & 0xffffff80;
  r_index = a & 0x7f;
  if (address != ra) return flush_row(ra);
  return 0;
}

unsigned save_row_data(unsigned char c) {
  row[r_index++] = c;
  if (r_index == 128) {
    r_index = 0;
    return flush_row(address + 128);
  }
  return 0;
}
