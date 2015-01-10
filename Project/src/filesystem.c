#include "filesystem.h"
#include "ff.h"
#include "spi_flash.h"
#include "common.h"

static char test_buffer[sFLASH_SPI_SECTORSIZE];
static FATFS fs; //fs object must be kept, because buffer inside will be used in further operation

void FileSystem_Init()
{

  DBG_MSG("Try to mount...");
  f_mount(&fs, "", 0);


}

void FileSystem_MkFs()
{
  FRESULT res;
  DWORD cnt;
  FATFS *fsptr;

  DBG_MSG("Try to mkfs...");
  res = f_mkfs("", 0, 0);
  DBG_MSG("f_mkfs()=%d", res);

  res = f_getfree("", &cnt, &fsptr);
  DBG_MSG("f_getfree()=%d cnt=%d", res, cnt);

}

void FileSystem_TestFileIO()
{
  FRESULT res;
  UINT cnt;
  FIL file;

  res = f_open(&file, "logfile.txt", FA_WRITE | FA_CREATE_ALWAYS);
  DBG_MSG("f_open()=%d", res);
  res = f_write(&file, "Hello", 5, &cnt);
  DBG_MSG("f_write()=%d", res);
  res = f_close(&file);
  DBG_MSG("f_close()=%d", res);

  char read_buf[16]={0};
  res = f_open(&file, "logfile.txt", FA_READ);
  DBG_MSG("f_open()=%d", res);
  res = f_read(&file, read_buf, 5, &cnt);
  DBG_MSG("f_read()=%d", res);
  DBG_MSG("Read: %s", read_buf);
  res = f_close(&file);
  DBG_MSG("f_close()=%d", res);

}

void FileSystem_TestFlash()
{
  sFLASH_Init();
  sFLASH_ReadBuffer(test_buffer, 0, sFLASH_SPI_SECTORSIZE);
  for (int i = 0; i < sFLASH_SPI_SECTORSIZE; ++i)
  {
    if(i%32==0)
      printf("\r\n");
    printf("%x,", test_buffer[i]);
  }
  printf("\r\n\n");
  sFLASH_EraseSector(0);
  for (int i = 0; i < sFLASH_SPI_SECTORSIZE; ++i)
  {
    test_buffer[i] = i&0xff;
  }
  sFLASH_WriteBuffer(test_buffer, 0, sFLASH_SPI_SECTORSIZE);
  sFLASH_ReadBuffer(test_buffer, 0, sFLASH_SPI_SECTORSIZE);
  for (int i = 0; i < sFLASH_SPI_SECTORSIZE; ++i)
  {
    if(i%32==0)
      printf("\r\n");
    printf("%x,", test_buffer[i]);
  }
  printf("\r\n");
}

void FileSystem_Dump(DWORD sector, UINT bytes)
{
  sFLASH_ReadBuffer(test_buffer, sector*sFLASH_SPI_SECTORSIZE, bytes);
  for (int i = 0; i < bytes; ++i)
  {
    if(i%32==0)
      printf("\r\n");
    printf("%x,", test_buffer[i]);
  }
}


