#ifndef MEMCARD_H
#define MEMCARD_H

#include <ff.h>

int SD_mount(void);
int SD_init(void);
int SD_createFile(char *filename, unsigned char mode);
int SD_openFile(FIL *file, unsigned char *filename);
int SD_closeFile(FIL *file);
unsigned long int SD_getFileSize(FIL *file);
int SD_writeData(FIL *file, unsigned long int start, unsigned char *data, unsigned int len);
int SD_readData(FIL *file, unsigned long int start, unsigned char *data, unsigned int len);


#endif
