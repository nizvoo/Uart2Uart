/*
  Copyright (C) 2000-2014
  nizvoo (AT) gmail.com
*/

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <stdint.h>

#include "inc/comm/ComPortUtils.h"

#define BUF_LEN (2048)

struct user_st
{
  TCHAR src_port_name[MAX_PATH + 1];
  TCHAR dst_port_name[MAX_PATH + 1];
  uint32_t src_baudrate;
  uint32_t dst_baudrate;
};

static
void show_user_helper(void)
{
  fprintf(stdout, "Uart2Uart.exe COMx COMy COMxbaudrate COMybaudrate");
}

static
void parse_entry_param(int argc, char * argv[], struct user_st* user_data)
{
  TCHAR full_port_name[MAX_PATH + 1] = {0};
  // Setup comm port for user query
  char* ascii_src_port_name = "COM10";
  char* ascii_dst_port_name = "COM16";
  uint32_t src_baudrate = 57600;
  uint32_t dst_baudrate = 57600;
  
  if( argc > 1 ) {
    ascii_src_port_name = argv[1];
  }
  
  if (argc > 2) {
    ascii_dst_port_name = argv[2];
  }
  
  if (argc > 3) {
	src_baudrate = atoi(argv[3]);
  }

  if (argc > 4) {
	dst_baudrate = atoi(argv[4]);
  }
  
  user_data->src_baudrate = src_baudrate;
  user_data->dst_baudrate = dst_baudrate;
  
  mbstowcs(user_data->src_port_name, ascii_src_port_name, MAX_PATH);
  mbstowcs(user_data->dst_port_name, ascii_dst_port_name, MAX_PATH);  
}

static
void show_user_buf(const char* buf, int len, int dir)
{
  int i;
  if (dir == 0)
    fprintf(stdout, "--->");
  else
    fprintf(stdout, "<---");
  for (i = 0; i < len; ++i)
    fprintf(stdout, "%02X ", (BYTE)buf[i]);
    
  fprintf(stdout, " :%d\n", len);
}

static HANDLE sh;
static HANDLE dh;

static void ForwardToUART(HANDLE h, const char* buf, int len)
{  
  WriteRSData(h, buf, len);  
}

int main(int argc, char * argv[])
{
  BOOL running = TRUE;
  int len;
  
  int i;
  DWORD wstm, rstm;
  BYTE buf[BUF_LEN];
  struct user_st user_data;
  TCHAR full_port_name[MAX_PATH + 1] = {0};
  
  fprintf(stdout, "Uart2Uart version beta 0.91\n");
  fprintf(stdout, "  Copyright (C) 2000-2013\n");
  fprintf(stdout, "   nizvoo (AT) gmail.com\n\n");  
  
  if (argc < 2) {
    show_user_helper();
    return;
  }
  
  parse_entry_param(argc, argv, &user_data);
  _ftprintf(stdout, _T("SRC %s:%d\n"),
    user_data.src_port_name, user_data.src_baudrate);
	
  _ftprintf(stdout, _T("DST %s:%d \n"),
	user_data.dst_port_name, user_data.dst_baudrate);
 
  sh = ConnctComPort(user_data.src_port_name, user_data.src_baudrate); 
  if (0 == sh) {
    _sntprintf(full_port_name, MAX_PATH, _T("\\\\.\\%s"), user_data.src_port_name);
    sh = ConnctComPort(full_port_name, user_data.src_baudrate);
  }
  
  if (0 == sh) {
    _tprintf(_T("Open %s failed\n"), user_data.src_port_name);
    return 0;
  }
  
  
  dh = ConnctComPort(user_data.dst_port_name, user_data.dst_baudrate); 
  if (0 == dh) {
    _sntprintf(full_port_name, MAX_PATH, _T("\\\\.\\%s"), user_data.dst_port_name);
    dh = ConnctComPort(full_port_name, user_data.dst_baudrate);
  }
  
  if (0 == dh) {
    _tprintf(_T("Open %s failed\n"), user_data.dst_port_name);
    return 0;
  }  
   
  while (running) {
    len = ReadRSData(sh, buf, BUF_LEN);
    if (len > 0) {
      ForwardToUART(dh, buf, len);
      show_user_buf(buf, len, 0);
      fprintf(stdout, "Forward to DST UART %d\n", len);      
    }
    
    len = ReadRSData(dh, buf, BUF_LEN);
    if (len > 0) {
      ForwardToUART(sh, buf, len);
      show_user_buf(buf, len, 1);
      fprintf(stdout, "Forward to SRC UART %d\n", len);      
    }    

    if (_kbhit()) break;    
  }

  CloseComPort(sh);
  CloseComPort(dh);
  
  return 0;
}
