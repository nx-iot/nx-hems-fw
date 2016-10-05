#ifndef XBEE_H
#define XBEE_H

#include "main.h"

//#define ATCMD       0x08
//#define ATCMQ       0x09
//#define ZTR         0x10
//#define EAZCF       0x11
//#define RCR         0x17
//#define CSR         0x21
//#define ATCMR       0x88
//#define MDS         0x8A
//#define ZTS         0x8B
//#define ZRPKT       0x90
//#define ZERI        0x91
//#define ZIORXI      0x92
//#define NI_IND      0x95    
//#define RCRES       0x97

#define ATCMD       (uint8_t)0x08
#define ATCMQ       (uint8_t)0x09
#define ZTR         (uint8_t)0x10
#define EAZCF       (uint8_t)0x11
#define RCR         (uint8_t)0x17
#define CSR         (uint8_t)0x21
#define ATCMR       (uint8_t)0x88
#define MDS         (uint8_t)0x8A
#define ZTS         (uint8_t)0x8B
#define ZRPKT       (uint8_t)0x90	//ZigBee Receive Packet
#define ZERI        (uint8_t)0x91
#define ZIORXI      (uint8_t)0x92
#define NI_IND      (uint8_t)0x95    
#define RCRES       (uint8_t)0x97

#define XBEE_TIMEOUT 0 


int8_t xbee_write(uint8_t *value, uint16_t len);
int8_t xbee_read(uint8_t *value, uint16_t *len);
void xbee_receivePacket(void);
void xbee_checkAssociation(void);
void xbee_readSerialNumber(void);
void xbee_flush();
uint8_t nodeAddress_write(uint8_t *addr);
int xbee_checksum(char buf[],int len);
int xbee_sendATCommand(char frameType, int atc, char *param, int paramLen);
int xbee_processPacket(char *buf, int len);
int xbee_sendAPI(char *frame, int lenght, int timeout);
int8_t xbee_getserial(void);

extern char _xbee_join ;
extern char _xbee_Assoc;     // 0x00 = Successfully joined a network       
extern char _xbee_chkAsso_res;
extern char SENDER_ADDRESS[8];
extern char RECEIVER_ADDRESS[8];
#endif 