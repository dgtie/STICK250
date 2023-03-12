#include <xc.h>
#include "usb.h"
#include "uart.h"
#include "usb_config.h"

#define SERIAL_STATE 0x20

void bd_fill(int index, char *buf, int size, int stat);

bool stop_selection(int);
bool parity_data_selection(int p, int d);
bool baud_selection(unsigned);
bool uart_tx(char *buf, int len);

typedef struct
{
    unsigned   dwDTERate;
    char    bCharFormat;
    char    bParityType;
    char    bDataBits;
} LINE_CODING;

namespace {
    
    struct {
        unsigned char bmRequestType;
        unsigned char bNotification;
        unsigned short wValue;
        unsigned short wIndex;
        unsigned short wLength;
        struct {
            unsigned DCD:1;
            unsigned DSR:1;
            unsigned break_in:1;
            unsigned RI:1;
            unsigned frame_err:1;
            unsigned parity_err:1;
            unsigned overrun_err:1;
            unsigned reserved:1;
            unsigned char nothing;
        };
    } cdc_notice_packet = { 0xa1, SERIAL_STATE, 0, 0, 2, 0 };
    
    LINE_CODING line_coding = { 115200, NUM_STOP_BITS_1, PARITY_NONE, 8 };
    char temp[8];
    char in_buffer[2][USB_EP2_BUFF_SIZE], *rx_buffer;
    char out_buffer[2][USB_EP2_BUFF_SIZE];
    int rx_index, tx_length;
    bool inBusy;
    
}

void CDCInitEndpoint(int config) {
    if (config) {
        U1EP1 = 5;      // EPTXEN, EPHSHK
        U1EP2 = 0x1d;   // EPCONDIS, EPRXEN, EPTXEN, EPHSHK
        rx_buffer = in_buffer[0];
        rx_index = tx_length = 0;
        inBusy = false;
        bd_fill(6, (char*)&cdc_notice_packet, USB_EP1_BUFF_SIZE, 0x80);
        bd_fill(8, out_buffer[0], USB_EP2_BUFF_SIZE, 0x80);
    }
}

int CDCTx(char* &buffer) {
    int length;
    if ((length = tx_length)) {
        if (buffer == out_buffer[0]) {
            bd_fill(8, out_buffer[0], USB_EP2_BUFF_SIZE, 0x80);
            buffer = out_buffer[1];
        } else {
            bd_fill(9, out_buffer[1], USB_EP2_BUFF_SIZE, 0xc0);
            buffer = out_buffer[0];            
        }
        tx_length = 0;
    }
    return length;
}

void CDC_TRN_Handler(int length) {
    char *buf;
    switch (U1STAT >> 2) {
        case 6: bd_fill(7, (char*)&cdc_notice_packet, USB_EP1_BUFF_SIZE, 0xc0);
            break;
        case 7: bd_fill(6, (char*)&cdc_notice_packet, USB_EP1_BUFF_SIZE, 0x80);
            break;
        case 8:
        case 9:
            if (uart_tx(out_buffer[U1STAT >> 2 & 1], tx_length = length))
                CDCTx(buf = out_buffer[(U1STAT >> 2 & 1) ^ 1]);
            break;
        case 10:
        case 11: inBusy = false;
            break;
        default:;
    }
}

char *CDCTrfSetupHandler(setup_packet *SetupPkt) {
    switch (SetupPkt->request) {
        case SET_CONTROL_LINE_STATE:
        case SET_LINE_CODING: return temp;
        case GET_LINE_CODING: return (char*)&line_coding;
        default: return 0;
    }
}

void CDCCtrlTrfSetupComplete(setup_packet *SetupPkt) {
    LINE_CODING *lc = (LINE_CODING*)temp;
    switch (SetupPkt->request) {
        case SET_LINE_CODING:
            if (stop_selection(lc->bCharFormat))
                line_coding.bCharFormat = lc->bCharFormat;
            if (parity_data_selection(lc->bParityType, lc->bDataBits)) {
                line_coding.bDataBits = lc->bDataBits;
                line_coding.bParityType = lc->bParityType;
            }
            if (baud_selection(lc->dwDTERate))
                line_coding.dwDTERate = lc->dwDTERate;
            break;
        default:;
    }
}

void CDCRx(char c) { rx_buffer[rx_index++] = c; }

void CDCTick(void) {
    static int tick;
    if (rx_index) {
        if (++tick > 20) if (!inBusy) {
            if (rx_buffer == in_buffer[0]) {
                rx_buffer = in_buffer[1];
                bd_fill(10, in_buffer[0], rx_index, 0x80);
            } else {
                rx_buffer = in_buffer[0];
                bd_fill(11, in_buffer[1], rx_index, 0xc0);
            }
            rx_index = 0; inBusy = true;
        }
    } else tick = 0;
}

