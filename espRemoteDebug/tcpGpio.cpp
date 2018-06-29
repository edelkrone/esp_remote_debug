/*-----------------------------------------------------------------------------
/
/
/----------------------------------------------------------------------------*/
#include "tcpGpio.h"

/*---------------------------------------------------------------------------*/
static uint8_t pinState;
static uint16_t tcpPacketInd;
static uint8_t replyString[32];
static struct espconn tcpServer;
static uint8_t tcpGpio_isConnected;
static uint8_t tcpConnNum;

/*---------------------------------------------------------------------------*/
static void ICACHE_FLASH_ATTR
shell_tcp_disconcb(void *arg)
{
  struct espconn* pespconn = (struct espconn*) arg;

  tcpGpio_isConnected = 0;
}

/*---------------------------------------------------------------------------*/
static void ICACHE_FLASH_ATTR
shell_tcp_recvcb(void* arg, char* data, uint16_t length)
{
  char* lowCmd = "LOW";
  char* highCmd = "HIGH";

  struct espconn* pespconn = (struct espconn*) arg;

  if(strncmp(data, lowCmd, os_strlen(lowCmd)) == 0)
  {
    pinState = 0;
    os_sprintf((char*)replyString,"%s","Ok\r\n");
  }
  else if(strncmp(data, highCmd, os_strlen(highCmd)) == 0)
  {
    pinState = 1;
    os_sprintf((char*)replyString,"%s","Ok\r\n");
  }
  else
  {
    os_sprintf((char*)replyString,"%s","Unknown command\r\n");
  }

  espconn_send(&tcpServer,replyString,os_strlen((char*)replyString));
}

/*---------------------------------------------------------------------------*/
static void ICACHE_FLASH_ATTR
tcpserver_connectcb(void *arg)
{
  struct espconn* pespconn = (struct espconn*) arg;

  tcpGpio_isConnected = 1;

  tcpConnNum++;
  if(tcpConnNum > 4){
    os_printf("system restart\r\n");
    system_restart();
  }
  
  espconn_regist_recvcb(pespconn, shell_tcp_recvcb);
  espconn_regist_disconcb(pespconn, shell_tcp_disconcb);
}

/*---------------------------------------------------------------------------*/
uint8_t ICACHE_FLASH_ATTR
tcpGpio_readState()
{
  return pinState;
}

/*---------------------------------------------------------------------------*/
void ICACHE_FLASH_ATTR
tcpGpio_init()
{
  pinState = 1;
  tcpGpio_isConnected = 0;

  tcpServer.type = ESPCONN_TCP;
  tcpServer.state = ESPCONN_NONE;
  tcpServer.proto.tcp = (esp_tcp *)calloc(sizeof(esp_tcp),1);
  tcpServer.proto.tcp->local_port = TCPGPIO_PORT;

  espconn_regist_connectcb(&tcpServer, tcpserver_connectcb);
  espconn_accept(&tcpServer);
  espconn_regist_time(&tcpServer, 12 * 3600, 0);
}
