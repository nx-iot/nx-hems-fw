#ifndef XBEE_H
#define XBEE_H

#define ATCMD       0x08
#define ATCMQ       0x09
#define ZTR         0x10
#define EAZCF       0x11
#define RCR         0x17
#define CSR         0x21
#define ATCMR       0x88
#define MDS         0x8A
#define ZTS         0x8B
#define ZRPKT       0x90
#define ZERI        0x91
#define ZIORXI      0x92
#define NI_IND      0x95    
#define RCRES       0x97

#define XBEE_TIMEOUT 0            
                                   
#endif

void xbee_receivePacket(void);
int xbee_processPacket(char *buf, int len);

int xbee_sendAPI(char *frame, int lenght, int timeout);
int xbee_sendZTR(char *destAddr, char *destNetAddr, char *data, int dataLen);
int xbee_sendATCommand(char frameType, int atc, char *param, int paramLen);
void xbee_reset(void);
void xbee_readSerialNumber(void);
void xbee_leaveNetwork(void);
void xbee_checkAssociation(void);
void xbee_disableJoinScan(void);

int xbee_checksum(char buf[],int len);
extern char sr_addr[8];
extern int (*funcProcessZTS)(char*);
extern int (*funcProcessZRPKT)(char*, int); 
extern int (*funcProcessMDS)(char*);
extern int (*funcProcessATCMR)(char*, int);
//extern int (*funcProcessRCRES)(char*, int);
//extern int (*funcProcessZIORXI)(char*, int);
//extern int (*funcProcessNI_IND)(char*, int);

