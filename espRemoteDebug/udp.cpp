/*-----------------------------------------------------------------------------
/
/
/----------------------------------------------------------------------------*/
#include "udp.h"

/*---------------------------------------------------------------------------*/
#if UDP_DEBUG
#define DBG(format, ...) do { xprintf(format, ## __VA_ARGS__); } while(0)
#else
#define DBG(format, ...) do { } while(0)
#endif

/*---------------------------------------------------------------------------*/
#define PROJECT_NAME "espRemoteDebug"
#define VERSION_MAJOR 0
#define VERSION_MINOR 1

/*---------------------------------------------------------------------------*/
static struct espconn* udpServer;
static int32_t aliveCounter_sec;
static uint8_t ipAddressString[32];
static uint8_t broadcastString[128];
static uint8_t macAddressString[32];
static os_timer_t udpServerTimer;

/*---------------------------------------------------------------------------*/
static void ICACHE_FLASH_ATTR
udpServerTimer_2000ms(void *arg)
{
  uint16_t length;
  uint8_t hwaddr[6];

  aliveCounter_sec += 1;

  struct ip_info ipconfig;

  wifi_get_ip_info(STATION_IF, &ipconfig);
  wifi_get_macaddr(STATION_IF, hwaddr);

  os_sprintf((char*)broadcastString, MACSTR ", " IPSTR ", %s, v%d.%d, %s, %s, %d sec\n",
   MAC2STR(hwaddr), IP2STR(&ipconfig.ip), PROJECT_NAME,
   VERSION_MAJOR, VERSION_MINOR, __DATE__, __TIME__,
   aliveCounter_sec
  );

  espconn_sent(udpServer, broadcastString, os_strlen((char*)broadcastString));

  os_timer_arm(&udpServerTimer, 1000, 0);
}

/*---------------------------------------------------------------------------*/
void ICACHE_FLASH_ATTR
udp_init()
{
  // Allocate memory for the control block
  udpServer = (struct espconn*)calloc(sizeof(struct espconn),1);

  // Create the connection from block
  espconn_create(udpServer);

  // Set connection to UDP
  udpServer->type = ESPCONN_UDP;

  // Alloc memory for the udp object in the server context and assign port
  udpServer->proto.udp = (esp_udp *)calloc(sizeof(esp_udp),1);
  udpServer->proto.udp->local_port = UDP_BROADCAST_PORT;

  // Create the object
  espconn_create(udpServer);

  // Enable UDP mode
  udpServer->type = ESPCONN_UDP;
  udpServer->proto.udp = (esp_udp *)calloc(sizeof(esp_udp),1);
  udpServer->proto.udp->local_port = UDP_BROADCAST_PORT;
  udpServer->proto.udp->remote_port = UDP_BROADCAST_PORT;

  // Set IP address to broadcast
  uint8_t broadcastIP[] = { 255, 255, 255, 255 };
  os_memcpy(udpServer->proto.udp->remote_ip, broadcastIP, 4);

  // Create the timer ...
  os_timer_disarm(&udpServerTimer);
  os_timer_setfn(&udpServerTimer, (os_timer_func_t*)udpServerTimer_2000ms, NULL);
  os_timer_arm(&udpServerTimer, 1000, 0);

  aliveCounter_sec = 0;
}
