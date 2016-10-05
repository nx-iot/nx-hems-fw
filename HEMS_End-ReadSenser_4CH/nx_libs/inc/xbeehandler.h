// Modem Status
#define HWRST       0x00        // Hardware reset
#define WDRST       0x01        // Watchdog timer reset
#define JOINNET     0x02        // Joined network (for routers and end devices)
#define DISAS       0x03        // Disassociated (from network)
#define COSTART     0x06        // Coordinator started
#define KEYUPDT     0x07        // Security Key was updated
#define VOLTEXC     0x0D        // Voltage supply limit exceeded (for PRO S2B only)
#define MODCHNG     0x11        // Modem Config Changed while joining (shouldn't occur)
#define STCKERR     0x80        // Stack Error (shouldn't occur)

// Command status of AT Command Response
#define ATCMDOK     0x00        // OK
#define ATCMDERR    0x01        // Error
#define BADCMD      0x02        // Invalid Command
#define BADPARAM    0x03        // Invalid Parameter
#define TXFAIL      0x04        // Tx Failure

// AT Command
#define DH          0x4448
#define DL          0x444C
#define SH          0x5348
#define SL          0x534C
#define NI          0x4E49
#define OI          0x4F49
#define JN          0x4A4E
#define DB          0x4442
#define VS          0x2556      // %V (Supply Voltage)
#define VP          0x562B      // V+ Voltage Supply Monitoring
#define TP          0x5450
#define AC          0x4143
#define ND          0x4E44
#define NR          0x4E52
#define AI          0x4149
#define DJ          0x444A   



// Globle variable use in
// - here
// - main.c
// - xbee.c
extern char _xbee_join;
extern char _xbee_Assoc;      
extern char _xbee_chkAsso_res;

int xbee_processZTS(char *buf);
int xbee_processMDS(char *buf);
int xbee_processATCMR(char *buf, int len);

int process_cmdData(unsigned int atCmd, char *cmdData, int len);