/*
 ============================================================================
 Name        : supla_esp_cfgmode.c
 Author      : Przemyslaw Zygmunt przemek@supla.org
 Version     : 1.0
 Copyright   : GPLv2
 ============================================================================
*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>


#include <ip_addr.h>
#include <user_interface.h>
#include <espconn.h>
#include <spi_flash.h>
#include <osapi.h>
#include <mem.h>

#include "supla_esp.h"
#include "supla_esp_cfg.h"
#include "supla_esp_cfgmode.h"
#include "supla_esp_devconn.h"
#include "supla_esp_gpio.h"

#include "supla-dev/log.h"

#define TYPE_UNKNOWN  0
#define TYPE_GET      1
#define TYPE_POST     2

#define STEP_TYPE        0
#define STEP_GET         2
#define STEP_POST        3
#define STEP_PARSE_VARS  4
#define STEP_DONE        10

#define VAR_NONE         0
#define VAR_SID          1
#define VAR_WPW          2
#define VAR_SVR          3
#define VAR_LID          4
#define VAR_PWD          5
#define VAR_CFGBTN       6
#define VAR_BTN1         7
#define VAR_BTN2         8

typedef struct {
	
	char step;
	char type;
	char current_var;
	
	short matched;
	
	char *pbuff;
	int buff_size;
	int offset;
	char intval[12];
	
}TrivialHttpParserVars;


static char supla_esp_cfgmode = 0;

void ICACHE_FLASH_ATTR 
supla_esp_http_send_response(struct espconn *pespconn, const char *code, const char *html) {

	int len = strlen(html)+26;
	char *buff = os_malloc(strlen(code)+len+110);
	ets_snprintf(buff, len+150, "HTTP/1.1 %s\r\nAccept-Ranges: bytes\r\nContent-Length: %i\r\nConnection: close\r\nContent-Type: text/html\r\n\r\n<html><body>%s</body></html>", code, len, html);

	espconn_sent(pespconn, buff, strlen(buff));
	os_free(buff);
}

void ICACHE_FLASH_ATTR 
supla_esp_http_ok(struct espconn *pespconn, const char *html) {
	supla_esp_http_send_response(pespconn, "200 OK", html);
}

void ICACHE_FLASH_ATTR 
supla_esp_http_404(struct espconn *pespconn) {
	supla_esp_http_send_response(pespconn, "404 Not Found", "Not Found");
}

void ICACHE_FLASH_ATTR 
supla_esp_http_error(struct espconn *pespconn) {
	supla_esp_http_send_response(pespconn, "500 Internal Server Error", "Error");
}

int ICACHE_FLASH_ATTR
Power(int x, int y) {
	
    int result = 1;
    while (y)
    {
        if (y & 1)
            result *= x;
        
        y >>= 1;
        x *= x;
    }

    return result;
}

int ICACHE_FLASH_ATTR
HexToInt(char *str, int len) {

	int a, n, p;
	int result = 0;

	if ( len%2 != 0 )
		return 0;

	p = len - 1;

	for(a=0;a<len;a++) {
		n = 0;

		if ( str[a] >= 'A' && str[a] <= 'F' )
			n = str[a]-55;
		else if ( str[a] >= 'a' && str[a] <= 'f' )
			n = str[a]-87;
		else if ( str[a] >= '0' && str[a] <= '9' )
			n = str[a]-48;


		result+=Power(16, p)*n;
		p--;
	}

	return result;


};

void ICACHE_FLASH_ATTR
supla_esp_parse_request(TrivialHttpParserVars *pVars, char *pdata, unsigned short len, SuplaEspCfg *cfg) {
	
	if ( len == 0 ) 
		return;
	
	int a, p;
	
	//for(a=0;a<len;a++)
	//	os_printf("%c", pdata[a]);

	if ( pVars->step == STEP_TYPE ) {
		
		char get[] = "GET";
		char post[] = "POST";
		char url[] = " / HTTP";
		
		if ( len >= 3
			 && memcmp(pdata, get, 3) == 0
			 && len >= 10
			 && memcmp(&pdata[3], url, 7) == 0 ) {
			
			pVars->step = STEP_GET;
			pVars->type = TYPE_GET;
			
		} else if ( len >= 4
				 && memcmp(pdata, post, 4) == 0
				 && len >= 11
				 && memcmp(&pdata[4], url, 7) == 0 )  {
			
			pVars->step = STEP_POST;
			pVars->type = TYPE_POST;
		}
		
	}
	
	p = 0;
	
	if ( pVars->step == STEP_POST ) {
		
		char header_end[4] = { '\r', '\n', '\r', '\n' };
		
		for(a=p;a<len;a++) {
		
			if ( len-a >= 4
				 && memcmp(header_end, &pdata[a], 4) == 0 ) {

				pVars->step = STEP_PARSE_VARS;
				p+=3;
			}

		}
		
	}
	
	if ( pVars->step == STEP_PARSE_VARS ) {
		
		for(a=p;a<len;a++) {
		
			if ( pVars->current_var == VAR_NONE ) {
				
				char sid[3] = { 's', 'i', 'd' };
				char wpw[3] = { 'w', 'p', 'w' };
				char svr[3] = { 's', 'v', 'r' };
				char lid[3] = { 'l', 'i', 'd' };
				char pwd[3] = { 'p', 'w', 'd' };
				char btncfg[3] = { 'c', 'f', 'g' };
				char btn1[3] = { 'b', 't', '1' };
				char btn2[3] = { 'b', 't', '2' };
				
				if ( len-a >= 4
					 && pdata[a+3] == '=' ) {

					if ( memcmp(sid, &pdata[a], 3) == 0 ) {
						
						pVars->current_var = VAR_SID;
						pVars->buff_size = WIFI_SSID_MAXSIZE;
						pVars->pbuff = cfg->WIFI_SSID;
						
					} else if ( memcmp(wpw, &pdata[a], 3) == 0 ) {
						
						pVars->current_var = VAR_WPW;
						pVars->buff_size = WIFI_PWD_MAXSIZE;
						pVars->pbuff = cfg->WIFI_PWD;
						
					} else if ( memcmp(svr, &pdata[a], 3) == 0 ) {
						
						pVars->current_var = VAR_SVR;
						pVars->buff_size = SERVER_MAXSIZE;
						pVars->pbuff = cfg->Server;
						
					} else if ( memcmp(lid, &pdata[a], 3) == 0 ) {
						
						pVars->current_var = VAR_LID;
						pVars->buff_size = 12;
						pVars->pbuff = pVars->intval;
						
					} else if ( memcmp(pwd, &pdata[a], 3) == 0 ) {
						
						pVars->current_var = VAR_PWD;
						pVars->buff_size = SUPLA_LOCATION_PWD_MAXSIZE;
						pVars->pbuff = cfg->LocationPwd;
						
					} else if ( memcmp(btncfg, &pdata[a], 3) == 0 ) {

						pVars->current_var = VAR_CFGBTN;
						pVars->buff_size = 12;
						pVars->pbuff = pVars->intval;

					} else if ( memcmp(btn1, &pdata[a], 3) == 0 ) {

						pVars->current_var = VAR_BTN1;
						pVars->buff_size = 12;
						pVars->pbuff = pVars->intval;

					} else if ( memcmp(btn2, &pdata[a], 3) == 0 ) {

						pVars->current_var = VAR_BTN2;
						pVars->buff_size = 12;
						pVars->pbuff = pVars->intval;

					}
					
					a+=4;
					pVars->offset = 0;
				}
				
			}
			
			if ( pVars->current_var != VAR_NONE ) {
				
				if ( pVars->offset < pVars->buff_size
					 && a < len
					 && pdata[a] != '&' ) {
					
					if ( pdata[a] == '%' && a+2 < len ) {
						
						pVars->pbuff[pVars->offset] = HexToInt(&pdata[a+1], 2);
						pVars->offset++;
						a+=2;
						
					} else if ( pdata[a] == '+' ) {

						pVars->pbuff[pVars->offset] = ' ';
						pVars->offset++;

					} else {

						pVars->pbuff[pVars->offset] = pdata[a];
						pVars->offset++;

					}
					
				}

				
				if ( pVars->offset >= pVars->buff_size
					  || a >= len-1
					  || pdata[a] == '&'  ) {
					
					if ( pVars->offset < pVars->buff_size )
						pVars->pbuff[pVars->offset] = 0;
					else
						pVars->pbuff[pVars->buff_size-1] = 0;
					
					
					if ( pVars->current_var == VAR_LID ) {
						
						cfg->LocationID = 0;

						short s=0;
						while(pVars->intval[s]!=0) {

							if ( pVars->intval[s] >= '0' && pVars->intval[s] <= '9' ) {
								cfg->LocationID = cfg->LocationID*10 + pVars->intval[s] - '0';
							}

							s++;
						}
					} else if ( pVars->current_var == VAR_CFGBTN ) {

						cfg->CfgButtonType = pVars->intval[0] - '0';

					} else if ( pVars->current_var == VAR_BTN1 ) {

						cfg->Button1Type = pVars->intval[0] - '0';

					} else if ( pVars->current_var == VAR_BTN2 ) {

						cfg->Button2Type = pVars->intval[0] - '0';
					}
					
					pVars->matched++;
					pVars->current_var = VAR_NONE;

				}
				
			}
			
		}
				
	}
	
}

void ICACHE_FLASH_ATTR
supla_esp_recv_callback (void *arg, char *pdata, unsigned short len)
{	
	struct espconn *conn = (struct espconn *)arg;
	char mac[6];
	char data_saved = 0;

	TrivialHttpParserVars *pVars = (TrivialHttpParserVars *)conn->reverse;

	if ( pdata == NULL || pVars == NULL )
		return;
	
	SuplaEspCfg new_cfg;
	memcpy(&new_cfg, &supla_esp_cfg, sizeof(SuplaEspCfg));
	
	supla_esp_parse_request(pVars, pdata, len, &new_cfg);
	
	if ( pVars->type == TYPE_UNKNOWN ) {
		
		supla_esp_http_404(conn);
		return;
	};
	
	if ( pVars->type == TYPE_POST ) {
		
		//supla_log(LOG_DEBUG, "Matched: %i", pVars->matched);

        #ifdef CFGBTN_TYPE_SELECTION
			if ( pVars->matched < 6 ) {
				return;
			}
		#elif defined(BTN1_2_TYPE_SELECTION)
			if ( pVars->matched < 7 ) {
				return;
			}
        #else
			if ( pVars->matched < 5 ) {
				return;
			}
		#endif
				
		if ( new_cfg.LocationPwd[0] == 0 )
			memcpy(new_cfg.LocationPwd, supla_esp_cfg.LocationPwd, SUPLA_LOCATION_PWD_MAXSIZE);
		
		if ( new_cfg.WIFI_PWD[0] == 0 )
					memcpy(new_cfg.WIFI_PWD, supla_esp_cfg.WIFI_PWD, WIFI_PWD_MAXSIZE);
	    
		if ( 1 == supla_esp_cfg_save(&new_cfg) ) {
					
			memcpy(&supla_esp_cfg, &new_cfg, sizeof(SuplaEspCfg));
			data_saved = 1;
			
		} else {
			supla_esp_http_error(conn);
		}
		
	}

	
	if ( false == wifi_get_macaddr(STATION_IF, mac) ) {
		supla_esp_http_error(conn);
		return;
	}
	
	int bufflen = 830+strlen(supla_esp_cfg.WIFI_SSID)+strlen(supla_esp_cfg.Server);
	char *buffer = (char*)os_malloc(bufflen);

    #ifdef CFGBTN_TYPE_SELECTION

	ets_snprintf(buffer, 
			bufflen,
 			"<h1>SUPLA ESP8266</h1>GUID: %02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X<br>MAC: %02X:%02X:%02X:%02X:%02X:%02X<br>LAST STATE: %s<br><br><form method=\"post\">WiFi SSID: <input type=\"text\" name=\"sid\" value=\"%s\"><br>WiFi Password: <input type=\"text\" name=\"wpw\" value=\"\"><br><br>Server: <input type=\"text\" name=\"svr\" value=\"%s\"><br>Location ID:<input type=\"number\" name=\"lid\" value=\"%i\"><br>Location password:<input type=\"text\" name=\"pwd\" value=\"\"><br><br>Button type:<select name=\"cfg\"><option value=\"0\" %s>button</option><option value=\"1\" %s>switch</option></select><br><br><input type=\"submit\" value=\"Save\"></form>%s",
			(unsigned char)supla_esp_cfg.GUID[0],
			(unsigned char)supla_esp_cfg.GUID[1],
			(unsigned char)supla_esp_cfg.GUID[2],
			(unsigned char)supla_esp_cfg.GUID[3],
			(unsigned char)supla_esp_cfg.GUID[4],
			(unsigned char)supla_esp_cfg.GUID[5],
			(unsigned char)supla_esp_cfg.GUID[6],
			(unsigned char)supla_esp_cfg.GUID[7],
			(unsigned char)supla_esp_cfg.GUID[8],
			(unsigned char)supla_esp_cfg.GUID[9],
			(unsigned char)supla_esp_cfg.GUID[10],
			(unsigned char)supla_esp_cfg.GUID[11],
			(unsigned char)supla_esp_cfg.GUID[12],
			(unsigned char)supla_esp_cfg.GUID[13],
			(unsigned char)supla_esp_cfg.GUID[14],
			(unsigned char)supla_esp_cfg.GUID[15],
			(unsigned char)mac[0],
			(unsigned char)mac[1],
			(unsigned char)mac[2],
			(unsigned char)mac[3],
			(unsigned char)mac[4],
			(unsigned char)mac[5], 
			supla_esp_devconn_laststate(),
			supla_esp_cfg.WIFI_SSID,
			supla_esp_cfg.Server,
			supla_esp_cfg.LocationID,
			supla_esp_cfg.CfgButtonType == BTN_TYPE_BUTTON ? "selected" : "",
			supla_esp_cfg.CfgButtonType == BTN_TYPE_SWITCH ? "selected" : "",
			data_saved == 1 ? "Data saved!" : "");

	#elif defined(BTN1_2_TYPE_SELECTION)

	ets_snprintf(buffer,
			bufflen,
 			"<h1>SUPLA ESP8266</h1>GUID: %02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X<br>MAC: %02X:%02X:%02X:%02X:%02X:%02X<br>LAST STATE: %s<br><br><form method=\"post\">WiFi SSID: <input type=\"text\" name=\"sid\" value=\"%s\"><br>WiFi Password: <input type=\"text\" name=\"wpw\" value=\"\"><br><br>Server: <input type=\"text\" name=\"svr\" value=\"%s\"><br>Location ID:<input type=\"number\" name=\"lid\" value=\"%i\"><br>Location password:<input type=\"text\" name=\"pwd\" value=\"\"><br><br>Input1 type:<select name=\"bt1\"><option value=\"0\" %s>button</option><option value=\"1\" %s>switch</option></select><br>Input2 type:<select name=\"bt2\"><option value=\"0\" %s>button</option><option value=\"1\" %s>switch</option></select><br><br><input type=\"submit\" value=\"Save\"></form>%s",
			(unsigned char)supla_esp_cfg.GUID[0],
			(unsigned char)supla_esp_cfg.GUID[1],
			(unsigned char)supla_esp_cfg.GUID[2],
			(unsigned char)supla_esp_cfg.GUID[3],
			(unsigned char)supla_esp_cfg.GUID[4],
			(unsigned char)supla_esp_cfg.GUID[5],
			(unsigned char)supla_esp_cfg.GUID[6],
			(unsigned char)supla_esp_cfg.GUID[7],
			(unsigned char)supla_esp_cfg.GUID[8],
			(unsigned char)supla_esp_cfg.GUID[9],
			(unsigned char)supla_esp_cfg.GUID[10],
			(unsigned char)supla_esp_cfg.GUID[11],
			(unsigned char)supla_esp_cfg.GUID[12],
			(unsigned char)supla_esp_cfg.GUID[13],
			(unsigned char)supla_esp_cfg.GUID[14],
			(unsigned char)supla_esp_cfg.GUID[15],
			(unsigned char)mac[0],
			(unsigned char)mac[1],
			(unsigned char)mac[2],
			(unsigned char)mac[3],
			(unsigned char)mac[4],
			(unsigned char)mac[5],
			supla_esp_devconn_laststate(),
			supla_esp_cfg.WIFI_SSID,
			supla_esp_cfg.Server,
			supla_esp_cfg.LocationID,
			supla_esp_cfg.Button1Type == BTN_TYPE_BUTTON ? "selected" : "",
			supla_esp_cfg.Button1Type == BTN_TYPE_SWITCH ? "selected" : "",
			supla_esp_cfg.Button2Type == BTN_TYPE_BUTTON ? "selected" : "",
			supla_esp_cfg.Button2Type == BTN_TYPE_SWITCH ? "selected" : "",
			data_saved == 1 ? "Data saved!" : "");

    #else

	ets_snprintf(buffer,
			bufflen,
 			"<h1>SUPLA ESP8266</h1>GUID: %02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X<br>MAC: %02X:%02X:%02X:%02X:%02X:%02X<br>LAST STATE: %s<br><br><form method=\"post\">WiFi SSID: <input type=\"text\" name=\"sid\" value=\"%s\"><br>WiFi Password: <input type=\"text\" name=\"wpw\" value=\"\"><br><br>Server: <input type=\"text\" name=\"svr\" value=\"%s\"><br>Location ID:<input type=\"number\" name=\"lid\" value=\"%i\"><br>Location password:<input type=\"text\" name=\"pwd\" value=\"\"><br><br><input type=\"submit\" value=\"Save\"></form>%s",
			(unsigned char)supla_esp_cfg.GUID[0],
			(unsigned char)supla_esp_cfg.GUID[1],
			(unsigned char)supla_esp_cfg.GUID[2],
			(unsigned char)supla_esp_cfg.GUID[3],
			(unsigned char)supla_esp_cfg.GUID[4],
			(unsigned char)supla_esp_cfg.GUID[5],
			(unsigned char)supla_esp_cfg.GUID[6],
			(unsigned char)supla_esp_cfg.GUID[7],
			(unsigned char)supla_esp_cfg.GUID[8],
			(unsigned char)supla_esp_cfg.GUID[9],
			(unsigned char)supla_esp_cfg.GUID[10],
			(unsigned char)supla_esp_cfg.GUID[11],
			(unsigned char)supla_esp_cfg.GUID[12],
			(unsigned char)supla_esp_cfg.GUID[13],
			(unsigned char)supla_esp_cfg.GUID[14],
			(unsigned char)supla_esp_cfg.GUID[15],
			(unsigned char)mac[0],
			(unsigned char)mac[1],
			(unsigned char)mac[2],
			(unsigned char)mac[3],
			(unsigned char)mac[4],
			(unsigned char)mac[5],
			supla_esp_devconn_laststate(),
			supla_esp_cfg.WIFI_SSID,
			supla_esp_cfg.Server,
			supla_esp_cfg.LocationID,
			data_saved == 1 ? "Data saved!" : "");
    #endif
	
	supla_esp_http_ok((struct espconn *)arg, buffer);
	os_free(buffer);
	
	
}


void ICACHE_FLASH_ATTR
supla_esp_discon_callback(void *arg) {
	
    struct espconn *conn = (struct espconn *)arg;
	
    if ( conn->reverse != NULL ) {
    	os_free(conn->reverse);
    	conn->reverse = NULL;
    }
}

void ICACHE_FLASH_ATTR
supla_esp_connectcb(void *arg)
{
    struct espconn *conn = (struct espconn *)arg;
    
    TrivialHttpParserVars *pVars = os_malloc(sizeof(TrivialHttpParserVars));
    memset(pVars, 0, sizeof(TrivialHttpParserVars));
    conn->reverse = pVars;

    espconn_regist_recvcb(conn, supla_esp_recv_callback );
    espconn_regist_disconcb(conn, supla_esp_discon_callback);
}


void ICACHE_FLASH_ATTR
supla_esp_cfgmode_start(void) {
	
	char APSSID[] = AP_SSID;
	char mac[6];

	wifi_get_macaddr(SOFTAP_IF, mac);

	struct softap_config apconfig;
	struct espconn *conn;
	
	if ( supla_esp_cfgmode == 1 ) 
		return;
	
	supla_esp_cfgmode = 1;
	
	supla_log(LOG_DEBUG, "ENTER CFG MODE");
	supla_esp_gpio_state_cfgmode();

	int apssid_len = strlen(APSSID);

	if ( apssid_len+14 > 32 )
		apssid_len = 18;

	memcpy(apconfig.ssid, APSSID, apssid_len);

	ets_snprintf(&apconfig.ssid[apssid_len],
			14,
 			"-%02X%02X%02X%02X%02X%02X",
			(unsigned char)mac[0],
			(unsigned char)mac[1],
			(unsigned char)mac[2],
			(unsigned char)mac[3],
			(unsigned char)mac[4],
			(unsigned char)mac[5]);

	apconfig.password[0] = 0;
	apconfig.ssid_len = apssid_len+13;
	apconfig.channel = 1;
	apconfig.authmode = AUTH_OPEN;
	apconfig.ssid_hidden = 0;
	apconfig.max_connection = 4;
	apconfig.beacon_interval = 100;
	
	wifi_set_opmode(SOFTAP_MODE);
	wifi_softap_set_config(&apconfig);

	conn = (struct espconn *)os_malloc(sizeof(struct espconn));
	memset( conn, 0, sizeof( struct espconn ) );

	espconn_create(conn);
	espconn_regist_time(conn, 5, 0);
	
	conn->type = ESPCONN_TCP;
	conn->state = ESPCONN_NONE;

	conn->proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
	conn->proto.tcp->local_port = 80;

	espconn_regist_connectcb(conn, supla_esp_connectcb);
	espconn_accept(conn);

}

char ICACHE_FLASH_ATTR
supla_esp_cfgmode_started(void) {

	return supla_esp_cfgmode == 1 ? 1 : 0;
}

