#include "ets_sys.h"
#include "mem.h"
#include "osapi.h"
#include "user_interface.h"
#include "user_global.h"
#include "espconn.h"

#define NTP_PACKET_SIZE 48

uint32_t unixSeconds = 0;
int32_t ntpTimeOffset = 2 * 3600;
bool ntp_invalid = true;
uint8_t ntp_server_counter = 0;
os_timer_t ntp_timer;

void ntp_send(struct espconn *ptr_conn, char *data, uint16_t len);

ICACHE_FLASH_ATTR
void ntp_set_time_offset(int32_t offset){
	ntpTimeOffset = offset;
}

ICACHE_FLASH_ATTR
uint32_t ntp_get_current_time(void){
	return unixSeconds + ntpTimeOffset;
}

ICACHE_FLASH_ATTR
uint8_t ntp_get_day_of_week(void){
	uint8_t weekday = (ntp_get_current_time() / 86400) % 7;
	weekday = weekday + 3; //we have to account that 1.1.1970 is thursday
	return weekday % 7;
}

ICACHE_FLASH_ATTR
void ntp_get_time_string(char *buffer) {
	uint32_t unixTime = ntp_get_current_time();
	uint8_t hour = (unixTime % 86400L) / 3600;
	uint8_t min = (unixTime % 3600) / 60;
	uint8_t sec = (unixTime % 60);
	os_sprintf(buffer, "%02d:%02d:%02d", hour, min, sec);
}

ICACHE_FLASH_ATTR
void getNtpTime(struct espconn *conn) {
	uint8_t packetBuffer[NTP_PACKET_SIZE];
	os_memset(packetBuffer, 0, NTP_PACKET_SIZE);
	// set all bytes in the buffer to 0
	memset(packetBuffer, 0, NTP_PACKET_SIZE);
	// Initialize values needed to form NTP request
	// (see URL above for details on the packets)
	packetBuffer[0] = 0b11100011;   // LI, Version, Mode
	packetBuffer[1] = 0;     // Stratum, or type of clock
	packetBuffer[2] = 6;     // Polling Interval
	packetBuffer[3] = 0xEC;  // Peer Clock Precision
	// 8 bytes of zero for Root Delay & Root Dispersion
	packetBuffer[12] = 49;
	packetBuffer[13] = 0x4E;
	packetBuffer[14] = 49;
	packetBuffer[15] = 52;

	ntp_send(conn, packetBuffer, NTP_PACKET_SIZE);
}

ICACHE_FLASH_ATTR
void ntp_timeout_handler(struct espconn *conn){
	os_printf("[NTP] timeout... retrying...\n");
	getNtpTime(conn);
}

ICACHE_FLASH_ATTR
void ntp_send(struct espconn *conn, char *data, uint16_t len) {
	espconn_sendto(conn, data, len);
	os_printf("[NTP] send request...\n");
}

ICACHE_FLASH_ATTR
void ntp_receive(void *ptr_conn, char *data, unsigned short len) {
	os_timer_disarm(&ntp_timer);
	os_printf("[NTP] got response...\n");
	struct espconn *conn = ptr_conn;

	uint32_t secondsFrom1900 = (data[40] << 24) | (data[41] << 16)
			| (data[42] << 8) | data[43];
	unixSeconds = secondsFrom1900 - 2208988800UL;

	char buffer[10];
	ntp_get_time_string(buffer);
	os_printf("[NTP] current time: %s\n", buffer);

	espconn_delete(conn);
}

ICACHE_FLASH_ATTR
void ntp_connect(void){
	static esp_udp espudp;

	static struct espconn esp_conn;
	esp_conn.type = ESPCONN_UDP;
	esp_conn.state = ESPCONN_NONE;
	esp_conn.proto.udp = &espudp;
	esp_conn.proto.udp->remote_ip[0] = 129;
	esp_conn.proto.udp->remote_ip[1] = 6;
	esp_conn.proto.udp->remote_ip[2] = 15;
	esp_conn.proto.udp->remote_ip[3] = 30;
	ntp_server_counter %= 3;

	esp_conn.proto.udp->remote_port = 123;
	esp_conn.proto.udp->local_port = 2390;

	espconn_create(&esp_conn);

	espconn_regist_recvcb(&esp_conn, ntp_receive);
	getNtpTime(&esp_conn);

	os_timer_disarm(&ntp_timer);
	os_timer_setfn(&ntp_timer, ntp_timeout_handler, &esp_conn);
	os_timer_arm(&ntp_timer, 2000, 1);
}

ICACHE_FLASH_ATTR
void ntp_init(void) {
	os_timer_disarm(&ntp_timer);
	os_timer_setfn(&ntp_timer, ntp_connect, NULL);
	os_timer_arm(&ntp_timer, 5000, 0);
}
