#include <stdlib.h>
#include "ets_sys.h"
#include "mem.h"
#include "osapi.h"
#include "user_interface.h"
#include "espconn.h"
#include "user_global.h"
#include "time.h"
#include "ip_addr.h"

#define MAX_ALLOWED_LENGTH  2000

typedef struct {
    UrlFrame frame;
    int lengthRemaining;
    struct ip_addr remote_ip;
    int remote_port;
} request_frame_t;

webserver_handler handler;

ICACHE_FLASH_ATTR
void free_frame(UrlFrame *frame){
    os_free(frame->body);
    os_free(frame->url);
    os_free(frame->requestHeaders);
    os_free(frame->requestMethods);
}

ICACHE_FLASH_ATTR
char *get_header(char *body, const char *header){
    char *headerStartPos = (char *)os_strstr(body, header) + os_strlen(header);
    char *headerEndPos = (char *)os_strstr(headerStartPos, "\r\n");
    char *headerValue = (char *)os_zalloc(headerEndPos - headerStartPos + 1);
    os_strncpy(headerValue, headerStartPos, headerEndPos - headerStartPos);
    headerValue[headerEndPos - headerStartPos] = '\0';
    os_printf("HEADER %s: %s\n", header, headerValue);
    return headerValue;
}

/******************************************************************************
 * FunctionName : parse_url
 * Description  : parse the received data from the server
 * Parameters   : precv -- the received data
 *                purl_frame -- the result of parsing the url
 * Returns      : none
*******************************************************************************/
ICACHE_FLASH_ATTR
void parse_url(char *data, UrlFrame *frame, int dataLength)
{
    char *str = NULL;
    uint8 length = 0;
    char *currentPos = NULL;
    char *buffer = NULL;

    currentPos = (char *)os_strstr(data, " HTTP");

    if (currentPos != NULL) {
        length = currentPos - data;
        buffer = (char *)os_zalloc(length + 1);
        os_strncpy(buffer, data, length);

        //parse method (GET / POST)
        currentPos = buffer;
        if (os_strncmp(currentPos, "GET ", 4) == 0) {
            frame->method = GET;
            currentPos += 4;
        } else if (os_strncmp(currentPos, "POST ", 5) == 0) {
            frame->method = POST;
            currentPos += 5;
        } else if(os_strncmp(currentPos, "OPTIONS ", 8) == 0) {
            frame->method = OPTIONS;
            currentPos += 8;
        }else {
        	os_printf("error parsing http request, unknown method %s\n", currentPos);
        }

        //parse url
        str = (char *)os_zalloc(os_strlen(currentPos) + 1);
        os_strcpy(str, currentPos);
        frame->url = str;
        os_free(buffer);

        //parse Content-Length
        if(frame->method == POST){
            char *contentLength = get_header(data, "Content-Length: ");
            os_printf("post header: %s\n", contentLength);
            frame->content_length = atoi(contentLength);
            os_free(contentLength);
        }

        if(frame->method == OPTIONS){
            frame->requestHeaders = get_header(data, "Access-Control-Request-Headers: ");
            frame->requestMethods = get_header(data, "Access-Control-Request-Method: ");
        }

        //parse body
        currentPos = (char *)os_strstr(data, "\r\n\r\n") + 4;
        if(currentPos == NULL)
        {
        	os_printf("did not find body\n");
        	return ;
        }
        uint16_t strSize = dataLength - (currentPos - data);
        str = (char *)os_zalloc(strSize + 1);
        os_strncpy(str, currentPos, strSize);
        frame->body = str;
        frame->bodyLength = strSize + 1;

    } else {
        return;
    }
}

ICACHE_FLASH_ATTR
void data_send(void *arg, bool responseOK, char *psend, const char *type, uint16_t code) {
	uint16 length = 0;
	char *pbuf = NULL;
	char httphead[256];
	struct espconn *ptrespconn = arg;
	os_memset(httphead, 0, 256);

	if (responseOK) {
		os_sprintf(httphead,
				"HTTP/1.0 %d OK\r\nContent-Length: %d\r\nServer: lwIP/1.4.0\r\nAccess-Control-Allow-Origin: *\r\n", code,
				psend ? os_strlen(psend) : 0);

		if (psend) {
			os_sprintf(httphead + os_strlen(httphead),
					"Content-type: %s\r\nPragma: no-cache\r\n\r\n", type);
			length = os_strlen(httphead) + os_strlen(psend);
			pbuf = (char *) os_zalloc(length + 1);
			os_memcpy(pbuf, httphead, os_strlen(httphead));
			os_memcpy(pbuf + os_strlen(httphead), psend, os_strlen(psend));
			espconn_send(ptrespconn, pbuf, length);
		} else {
			os_sprintf(httphead + os_strlen(httphead), "\n");
			length = os_strlen(httphead);
			espconn_send(ptrespconn, httphead, length);
		}

	} else {
	    if(psend){
	        os_sprintf(httphead, "HTTP/1.0 %d %s\r\nContent-Length: 0\r\nServer: lwIP/1.4.0\r\n\r\n\n", code, psend);
	    }
	    else {
            os_sprintf(httphead, "HTTP/1.0 %d BadRequest\r\nContent-Length: 0\r\nServer: lwIP/1.4.0\r\n\r\n\n", code);
	    }
	    length = os_strlen(httphead);
	    espconn_send(ptrespconn, httphead, length);
	}

	if (pbuf) {
		os_free(pbuf);
		pbuf = NULL;
	}
}

ICACHE_FLASH_ATTR
void data_send_cors(struct espconn *conn, char *headers){
    char str[1024];
    os_sprintf(str, "HTTP/1.0 200 OK\r\nContent-Length: 0\r\n%s\r\n", headers);
    espconn_send(conn, str, os_strlen(str));
}

ICACHE_FLASH_ATTR
void data_send_ok(struct espconn *conn){
    data_send(conn, true, NULL, NULL, 200);
}

ICACHE_FLASH_ATTR
void data_send_fail(struct espconn *conn, uint16_t code, char *statusMessage){
    data_send(conn, false, statusMessage, NULL, code);
}

ICACHE_FLASH_ATTR
void data_send_json(struct espconn *conn, char *psend){
    data_send(conn, true, psend, "application/json", 200);
}

ICACHE_FLASH_ATTR
void data_send_text(struct espconn *conn, char *psend){
    data_send(conn, true, psend, "text/plain", 200);
}

/******************************************************************************
 * FunctionName : webserver_recon
 * Description  : the connection has been err, reconnection
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
 *******************************************************************************/
ICACHE_FLASH_ATTR
void webserver_recon(void *arg, sint8 err) {
	struct espconn *pesp_conn = arg;

	os_printf("webserver's %d.%d.%d.%d:%d err %d reconnect\n",
			pesp_conn->proto.tcp->remote_ip[0],
			pesp_conn->proto.tcp->remote_ip[1],
			pesp_conn->proto.tcp->remote_ip[2],
			pesp_conn->proto.tcp->remote_ip[3],
			pesp_conn->proto.tcp->remote_port, err);
}

/******************************************************************************
 * FunctionName : webserver_recon
 * Description  : the connection has been err, reconnection
 * Parameters   : arg -- Additional argument to pass to the callback function
 * Returns      : none
 *******************************************************************************/
ICACHE_FLASH_ATTR
void webserver_discon(void *arg) {
	struct espconn *pesp_conn = arg;

	os_printf("webserver's %d.%d.%d.%d:%d disconnect\n",
			pesp_conn->proto.tcp->remote_ip[0],
			pesp_conn->proto.tcp->remote_ip[1],
			pesp_conn->proto.tcp->remote_ip[2],
			pesp_conn->proto.tcp->remote_ip[3],
			pesp_conn->proto.tcp->remote_port);


    request_frame_t *request = (request_frame_t *)pesp_conn->reverse;
    if(request != NULL){
        free_frame(&(request->frame));
        os_free(request);
        pesp_conn->reverse = NULL;
    }
}

/******************************************************************************
 * FunctionName : webserver_recv
 * Description  : Processing the received data from the server
 * Parameters   : arg -- Additional argument to pass to the callback function
 *                pusrdata -- The received data (or NULL when the connection has been closed!)
 *                length -- The length of received data
 * Returns      : none
 *******************************************************************************/
ICACHE_FLASH_ATTR
void webserver_recv(void *arg, char *pusrdata, unsigned short length) {
	struct espconn *ptrespconn = arg;

	os_printf("webserver's %d.%d.%d.%d:%d receive\n",
	          ptrespconn->proto.tcp->remote_ip[0],
	          ptrespconn->proto.tcp->remote_ip[1],
	          ptrespconn->proto.tcp->remote_ip[2],
	          ptrespconn->proto.tcp->remote_ip[3],
	          ptrespconn->proto.tcp->remote_port);

//	os_printf("RECEIVED:\n%s\n\n\n\n\n\nEND RECEIVED\n", pusrdata);
//	char response[64];
	ip_addr_t remote_ip;
	int remote_port = ptrespconn->proto.tcp->remote_port;
	uint8_t *ip = ptrespconn->proto.tcp->remote_ip;
	IP4_ADDR(&remote_ip, ip[0], ip[1], ip[2], ip[3]);

//	os_printf("got receive event: %d\n", remote_port);
	request_frame_t *request = (request_frame_t *) ptrespconn->reverse;
	if(request == NULL){
	    request = (request_frame_t *)os_zalloc(sizeof(request_frame_t));
	    request->remote_ip = remote_ip;
	    request->remote_port = remote_port;
	    request->lengthRemaining = -1;
        ptrespconn->reverse = (void *) request;
	}

	if(pusrdata != NULL && length > 0){
		//this is header
		if(request->lengthRemaining == -1){
		    parse_url(pusrdata, &(request->frame), length);
		    if(request->frame.method == GET || request->frame.method == OPTIONS){
		        request->lengthRemaining = 0;
		    }

		    if(request->frame.method == POST){
		        request->lengthRemaining = request->frame.content_length - request->frame.bodyLength + 1;
		        os_printf("clen: %d\n", request->frame.content_length);
		        os_printf("blen: %d\n", request->frame.bodyLength);
		    }
		}
		else{
		    //check if request too big
		    if(request->frame.content_length > MAX_ALLOWED_LENGTH){
		        data_send_fail(ptrespconn, 400, "request too big");
                return;
            }

		    //reallocate body
		    char *newBody = (char *)os_realloc(request->frame.body, request->frame.bodyLength + length);
		    request->frame.body = newBody;

		    //check if reallocation successful
		    if(request->frame.body == NULL){
		        os_printf("out of memory!!!!!!!!!!!! free [%d] requested [%d]\n", system_get_free_heap_size(), request->frame.bodyLength + length);
		        data_send_fail(ptrespconn, 500, "out of memory");
		        return;
		    }

		    //append new data to body
		    os_strncpy(request->frame.body + request->frame.bodyLength - 1, pusrdata, length);

		    request->frame.bodyLength += length;
		    request->frame.body[request->frame.bodyLength - 1] = '\0';
            request->lengthRemaining -= length;
		}

		if(request->lengthRemaining == 0)
		{
		    if(request->frame.method == OPTIONS){
		        os_printf("-------- CORS REQUEST\n");
		        os_printf("%s\n", request->frame.body);

		        char str[1024];
		        os_sprintf(str, "Access-Control-Allow-Origin: *\r\nAccess-Control-Allow-Headers: %s\r\nAccess-Control-Allow-Methods: %s\r\n", request->frame.requestHeaders, request->frame.requestMethods);
		        data_send_cors(ptrespconn, str);
		        return;
		    }

		    bool handeled = false;
		    if(handler != NULL){
		        handeled = handler(ptrespconn, &(request->frame));
		    }

		    if(!handeled){
		        data_send_fail(ptrespconn, 404, "NOT FOUND");
		    }
		}
	}
}

ICACHE_FLASH_ATTR
void webserver_listen(void* arg) {
	struct espconn *pesp_conn = arg;

	os_printf("webserver's %d.%d.%d.%d:%d listen\n",
	            pesp_conn->proto.tcp->remote_ip[0],
	            pesp_conn->proto.tcp->remote_ip[1],
	            pesp_conn->proto.tcp->remote_ip[2],
	            pesp_conn->proto.tcp->remote_ip[3],
	            pesp_conn->proto.tcp->remote_port);
	espconn_regist_recvcb(pesp_conn, webserver_recv);
	espconn_regist_reconcb(pesp_conn, webserver_recon);
	espconn_regist_disconcb(pesp_conn, webserver_discon);
}

ICACHE_FLASH_ATTR
void webserver_init(void) {
	LOCAL struct espconn esp_conn;
	LOCAL esp_tcp esptcp;

	esp_conn.type = ESPCONN_TCP;
	esp_conn.state = ESPCONN_NONE;
	esp_conn.proto.tcp = &esptcp;
	esp_conn.proto.tcp->local_port = 80;
	espconn_regist_connectcb(&esp_conn, webserver_listen);
	espconn_accept(&esp_conn);
}

ICACHE_FLASH_ATTR
void webserver_register_handler(webserver_handler user_handler){
	handler = user_handler;
}
