#ifndef DEBUG_H
#define DEBUG_H

#include <stdint.h>

void debug_req(void);
void printDebug(flash char *fmtstr, ...);   
void print_hex_ascii_line(const unsigned char *payload, int len, int offset);    
void print_payload(const unsigned char *payload, int len);
#endif 