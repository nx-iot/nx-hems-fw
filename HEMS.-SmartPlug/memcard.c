#include <mega2560.h>
#include <stdio.h>
#include <stdlib.h>
#include <delay.h>
#include "memcard.h"
#include "debug.h"

unsigned long int sdFreeSpace = 0;
                
/*============================================================*/
/* FAT function result */
FRESULT res;
/* will hold the information for logical drive 0: */
FATFS fat;
/* pointer to the FATFS type structure */
FATFS *pfat;
/* root directory path for logical drive 0: */
char root_path[]="0:/";
/*============================================================*/
flash char * flash error_msg[]= {   "", /* not used */
                                    "FR_DISK_ERR",
                                    "FR_INT_ERR",
                                    "FR_NOT_READY",
                                    "FR_NO_FILE",
                                    "FR_NO_PATH",
                                    "FR_INVALID_NAME",
                                    "FR_DENIED",
                                    "FR_EXIST",
                                    "FR_INVALID_OBJECT",
                                    "FR_WRITE_PROTECTED",
                                    "FR_INVALID_DRIVE",
                                    "FR_NOT_ENABLED",
                                    "FR_NO_FILESYSTEM",
                                    "FR_MKFS_ABORTED",
                                    "FR_TIMEOUT"  };      
/*============================================================*/        /* display error message and stop */
void error(FRESULT res) {

    if ((res >= FR_DISK_ERR) && (res <= FR_TIMEOUT))        
        printDebug("<ERR> %p.\r\n", error_msg[res]);     
    return; 

}
/*============================================================*/
int SD_mount(void) {

    FRESULT res; 
    res = f_mount(0, &fat);
    if( res != FR_OK ) {
        printDebug("Logical drive 0: mounted ERROR\r\n");
        error(res);
        return -1;
    }

    printDebug("Logical drive 0: mounted OK\r\n");
    return 0;    
    
}
/*============================================================*/ 
int SD_unmount(void) {

    FRESULT res; 
 
    /* unmount logical drive 0: */
    res = f_mount(0, NULL);
    if( res != FR_OK ){
        printDebug("Logical drive 0: unmounted ERROR\r\n");
        error(res);
        return -1;
    }

    printDebug("Logical drive 0: unmounted OK\r\n");
    return 0;
}             
/*============================================================*/
int SD_readMemoryRemain(unsigned long *free_kbytes)
{
    FRESULT res;
    /* number of free clusters on logical drive 0:*/ 
    unsigned long free_clusters;

    // ------ read available memory space ------- //
    // return as percentage!!! //

    /* get the number of free clusters */
    res = f_getfree(root_path,&free_clusters,&pfat);
    if ( res != FR_OK){                   
       /* an error occured, display it and stop */
       printDebug("<SD_readMemoryRemain> f_getfree(%s) error!!\r\n",root_path);
       error(res);                     
       sdFreeSpace = 0;
       return -1;
    }        
    
    /* calculate the number of free bytes */
    *free_kbytes = free_clusters * pfat->csize / 2;        
    
    return 0;
}       
/*============================================================*/                                              
int SD_init(void) {

    int err;              
    
    printDebug("<SD_init> Start initial memory card\r\n");

    /* point to the FATFS structure that holds
    information for the logical drive 0: */
    pfat=&fat;
                        
    delay_ms(1000);

    err = SD_mount();                     
    if(err < 0){ 
        return -1;
    }                          
    
    printDebug("<SD_init> Check Card Size...\r\n");
    
    delay_ms(1000);
    
    err = SD_readMemoryRemain(&sdFreeSpace);   
    if(err < 0){  
        return -1;
    }             
    
    /* display the number of free kbytes */
    printDebug("<SD_init> Free space on logical drive 0: %lu kbytes\r\n",sdFreeSpace);
    
    return 0;  
    
}
/*============================================================*/

/* Mode */
/* FA_CREATE_NEW       Creates a new file. If the file already exists, the function will fail.                         */
/* FA_CREATE_ALWAYS    Creates a new file. If the file already exists, it will be overwritten and its size set to 0.   */
int SD_createFile(char *filename, unsigned char mode) {

    FIL file;
    FRESULT res;     
    char filepath[50];                     
    
    if(sdFreeSpace == 0) { 
        printDebug("<SD_createFile> SD Card not ready\r\n");      
        return -1;
    }
    
    sprintf(filepath,"%s%s",root_path,filename);
    res = f_open(&file, filepath, mode);

    if(mode == FA_CREATE_ALWAYS) {    // -- replace old file if exist -- //                                               
        printDebug("<SD_createFile> Force re-creating file %s.\r\n",filename);
    }else {
        printDebug("<SD_createFile> Try to creating file %s.\r\n",filename);
    }                                                                          
    
    if( res != FR_OK) {                
         printDebug("<SD_createFile> f_open() failed\r\n");
         error(res);
         return -1;
    }                                    
    
    printDebug("<SD_createFile> '%s' created!!\r\n",filename); 
    
    res = f_close( &file );
    if( res != FR_OK ) {                          
        printDebug("<SD_createFile> f_close() failed\r\n");
        error(res);
        return -1;
    }                 
    
    return 1;   // -- return 1 indicated that it's a new file -- //
}
/*============================================================*/
int SD_openFile(FIL *file, unsigned char *filename)
{
    int     res;
    char    filepath[50];             
    
    if(sdFreeSpace == 0){ printDebug("<SD_openFile> SD Card not ready\r\n");    return -1;}
    
    sprintf(filepath,"%s%s",root_path,filename);                   
    
    res = f_open( file, filepath, FA_WRITE | FA_READ | FA_OPEN_EXISTING);    
    if( res != FR_OK){                
        printDebug("<SD_openFile> f_open() [%s] failed\r\n", filename);
        error(res);                                            
        sdFreeSpace = 0;  
        return -1;
    }             
    
    return 0;
}
/*============================================================*/ 
int SD_closeFile(FIL *file)
{
    int res;                                                                               
    
    if(sdFreeSpace == 0){ printDebug("<SD_closeFile> SD Card not ready\r\n");       return -1;}
    
    res = f_close( file );
    if( res != FR_OK ){                          
        printDebug("<SD_closeFile> f_close() failed\r\n");
        error(res);
        return -1;
    }        
    
    return 0;        
}
/*============================================================*/
unsigned long int SD_getFileSize(FIL *file) {
    return file->fsize;
} 
/*============================================================*/
int SD_writeData(FIL *file, unsigned long int start, unsigned char *data, unsigned int len) {

    FRESULT res;                      
    unsigned int actual_write;                                                              
    
    if(sdFreeSpace == 0){ printDebug("<SD_writeData> SD Card not ready\r\n");       return -1;}                                         
    
    res = f_lseek(file,start);   
    if(res != FR_OK){      
        printDebug("<SD_writeData> f_lseek(%u) failed!!\r\n", start);   
        error(res); 
        return -1;
    }            
    
    res = f_write(file, data, len,  &actual_write);  
    if(res != FR_OK){      
        printDebug("<SD_writeData> f_write() failed\r\n");  
        print_payload(data,len);   
        error(res); 
        return -2;
    }            
    
    if(actual_write != len){
        printDebug("<SD_writeData> actual_write[%u] != len[%u]\r\n",actual_write,len);   
        return -3; // -- the caller should roll back the last data -- //                                                                
    }                                                                     

    return 0;
}
/*============================================================*/
int SD_readData(FIL *file, unsigned long int start, unsigned char *data, unsigned int len)
{
    FRESULT         res;                      
    unsigned int    actual_read;        
    
    if(sdFreeSpace == 0){ printDebug("<SD_readData> SD Card not ready\r\n");       return -1;}     
    
    res = f_lseek(file,start);   
    if(res != FR_OK){      
        printDebug("<SD_readData> f_lseek(%u) failed!!\r\n", start);   
        error(res); 
        return -1;
    }            
    
    res = f_read(file,data,len,&actual_read);  
    if(res != FR_OK){      
        printDebug("<SD_readData> f_read() failed\r\n");     
        error(res); 
        return -2;
    }            
    
    if(actual_read != len){
        printDebug("<SD_writeData> actual_read[%u] != len[%u]\r\n",actual_read,len);   
        return -3; // -- the caller should roll back the last data -- //                                                                
    }                                                                     
     
    return 0;
}
/*============================================================*/

 
