/* -------------------- COMMAND -------------------- */
#define STATREQC        0x1A        // -- Status Request COMMAND      
#define STATREPC        0x2A        // -- Status Report COMMAND     
#define EVENREPC        0x3A        // -- Event Report COMMAND     
#define CONFMANC        0x4A        // -- Configuration Management COMMAND   
#define DATAREPC        0x5A        // -- Data Report COMMAND                
#define LOGMANIC        0x6A        // -- Log Manipulation COMMAND    
#define NETMAINC        0x7A        // -- Network Maintenance COMMAND    
#define RMDCTRLC        0x8A        // -- Remote Device Control COMMAND    
#define CLRLISTC        0x9A        // -- Clear List COMMAND
#define FMWUPLDC        0xFA        // -- Firmware Upload COMMAND (Reserved)   
                   
/* -------------------- ACKNOWLEDGE -------------------- */
#define STATREQA        0x1B        // -- Status Request ACKNOWLEDGE
#define STATREPA        0x2B        // -- Status Report ACKNOWLEDGE
#define EVENREPA        0x3B        // -- Event Report ACKNOWLEDGE
#define CONFMANA        0x4B        // -- Configuration Management ACKNOWLEDGE
#define DATAREPA        0x5B        // -- Data Report ACKNOWLEDGE
#define LOGMANIA        0x6B        // -- Log Manipulation ACKNOWLEDGE
#define NETMAINA        0x7B        // -- Network Maintenance ACKNOWLEDGE
#define RMDCTRLA        0x8B        // -- Remote Device Control ACKNOWLEDGE        
#define CLRLISTA        0x9B        // -- Clear List ACKNOWLEDGE
#define PROTOERROR      0xEB        // -- Protocol Error Feedback 
#define FMWUPLDA        0xFB        // -- Firmware Upload ACKNOWLEDGE (Reserved)



/* --------------- NETWORK MAINTENANCE COMMAND --------------- */
#define HEARTBEAT       0x11        // Heartbeat
#define JOINREQ         0x30        // Join Request

/* --------------- DATA REPORT COMMAND --------------- */
// - Record Type
#define RLT             0x00        // Real time record
#define LOG             0x01        // Logged record
#define SMART_PLUG      0x20        // Fixed for Smart_Plug
#define SMART_THERMO    0x21        // Fixed for Smart_Thermostat
// - Data ID
#define SEN1            0x01        // Sensor1
#define SEN2            0x02        // Sensor2
#define SEN3            0x03        // Sensor3
#define SEN4            0x04        // Sensor4
#define SEN5            0x05        // Sensor5
// - Data Type
#define TEMP            0x01        // Temperature (C)
#define PRES            0x02        // Pressure
#define HUMI            0x03        // Humidity
#define VOLT            0x10        // Voltage (V)
#define AMP             0x11        // Ampere (mA)
#define PWR             0x12        // Power (KW)
#define KWH             0x13        // Energy (KWH)

/* --------------- PROTOCOL ERROR FEEDBACK ACK --------------- */
// - Error ID
#define INVALID_LEN         0x10    // Invalid Length
#define INVALID_FTYPE       0x20    // Invalid Frame type
#define INVALID_PARAM       0x30    // Invalid Parameter
#define PARAM_OUT_OF_RANGE  0x40    // Parameter Out of Range
#define END_NOT_READY       0x50    // End Point not Ready
#define UNKNOWN_ERR         0xFF    // Unknown Error

/* --------------- Relay CH --------------- */
#define RELAY_CH1 					0x20
#define RELAY_CH2 					0x21
#define RELAY_CH3 					0x22
#define RELAY_CH4 					0x23
#define RELAY_CH5 					0x24
#define RELAY_CH6 					0x25
#define RELAY_CH7 					0x26
#define RELAY_CH8 					0x27
#define RELAY_CH9 					0x28
#define RELAY_CH10 					0x29
#define RELAY_ALL 					0x2f


// DATASET use in Data Report Cmd Packet
typedef struct {
    char dataID;
    char dataType;
    float value;
}DATASET;
int INT_sendStatusReportCmd(unsigned long int timeStamp, char accm, char *param, int paramLen, int timeout);
int INT_sendDataReportCmd(unsigned long int timeStamp, char recordType, char numSet, DATASET *data, int timeout);
int INT_sendNetworkMaintenanceCmd(char cmd, char *specData, int specDataLen, int timeout);
int INT_processPacket(char *buf, int len);
int INT_sendEventReportCmd(unsigned long int timeStamp, char recordType, char eventID, char *status, int timeout);
void Remote_Relay(char *buf);