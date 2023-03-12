#include <stdio.h>

unsigned calculate_crc(unsigned crc, unsigned char*, unsigned);

class Row {
  public:
    Row(unsigned a, Row *n): address(a), next(n)  {
      for (int i = 0; i < 128; i++) data[i] = 0xff;
    }
    Row(unsigned a): Row(a, this) {}
    Row *check_address(unsigned a, Row *last) {
      if (a == address) return this;
      if (next != last) return next->check_address(a, last);
      return next = new Row(a, last);
    }
    void dump(Row *last) {
      printf("address= %08x:\n", address);
      for (int i = 0; i < 128; i+=16) {
        for (int j = 0; j < 16; j++) printf("%02x", data[i + j]);
        printf("\n");
      }
      if (!last || (last == next)) return;
      return next->dump(last);
    }
    Row *get_next(void) { return next; }
    Row *remove_next(void) {
      Row *n = next;
      next = next->next;
      return n;
    }
    unsigned save_data(unsigned char c, unsigned i) {
      data[i] = c;
      return address;
    }
    unsigned get_crc(void) { return calculate_crc(0, data, 128); }
    unsigned get_address(void) { return address; }
  private:
    unsigned address;
    Row *next;
    unsigned char data[128];
};

namespace
{

Row *row;
unsigned row_index;

} //anonymous

void set_row_address(unsigned a) {
  unsigned addr = a & 0xffffff80;
  if (!row) row = new Row(addr);
  row = row->get_next();
  row = row->check_address(addr, row);
  row_index = a & 0x7f;
}

void save_row_data(unsigned char c) {
  unsigned a = row->save_data(c, row_index++);
  if (row_index == 128) set_row_address(a + 128);
}

void dump_row(void) {
  if (row) {
    row = row->get_next();
    row->dump(row);
  }
}

unsigned pop_row_crc(unsigned &crc) {
  if (!row) return 0;
  Row *r = row->remove_next();
  unsigned address = r->get_address();
  crc = r->get_crc();
  if (r == row) row = 0;
  delete r;
  return address;
}

/*** MAIN ******************************************************
int main(void) {
  set_row_address(0x1d000000);
  set_row_address(0x1d000080);
  set_row_address(0x1d000170);
  for (int i = 0; i < 256; i++) save_row_data(i & 255);
  row = row->get_next();
  row->dump(row);
  return 0;
}
// ** MAIN *****************************************************/
