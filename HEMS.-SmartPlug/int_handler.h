#ifndef INT_HANDLER_H
#define INT_HANDLER_H

// -------------------- HEADER -------------------- //
#define PROTO_VERSION   0x30

// Globle variable use in here, main.c, int_protocol.c
extern char _hb_res;
extern char _hb_stat;
extern char _joinreq_res;
extern char _joinreq_stat;

// Globle variable use in
// - main.c
// - timer.c
extern char _pired;

int8_t INT_sendPacket(char *recvAddr, char *data, int dataLen, int timeout);
int8_t INT_processRMDCTRLC(char *buf, int len);
int8_t INT_processNETMAINA(char *buf, int len);

#endif


