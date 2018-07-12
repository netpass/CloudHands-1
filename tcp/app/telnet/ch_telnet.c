/*
 *
 *      Filename: ch_telnet.c
 *
 *        Author: shajf,csp001314@gmail.com
 *   Description: ---
 *        Create: 2018-07-12 16:21:25
 * Last Modified: 2018-07-12 19:26:59
 */

#include "ch_telnet.h"
#include "ch_config.h"
#include "ch_util.h"
#include "ch_tcp_record.h"
#include "ch_tcp_app_util.h"
#include "ch_log.h"

typedef struct private_telnet_context_t private_telnet_context_t;

#define TELNET_PORTS_MAX 64

struct private_telnet_context_t {

	uint16_t telnet_ports[TELNET_PORTS_MAX];

};

static  private_telnet_context_t tmp_context,*g_tcontext = &tmp_context;


#include "do_telnet_context.c"

static int is_accept_by_port_for_telnet(ch_tcp_app_t *app,ch_packet_tcp_t *tcp_pkt){

	private_telnet_context_t *tcontext = (private_telnet_context_t*)app->context;


    return ch_ports_equal(tcontext->telnet_ports,TELNET_PORTS_MAX,tcp_pkt->src_port,tcp_pkt->dst_port);

}

static int is_accept_by_content_for_telnet(ch_tcp_app_t *app ch_unused,ch_packet_tcp_t *tcp_pkt ch_unused,
	void *data ch_unused,size_t dlen ch_unused){

    return 1;
}


static int request_content_process_for_telnet(ch_tcp_app_t *app ch_unused,ch_shm_format_t *fmt,
	ch_tcp_session_t *tsession,void *data,size_t dlen){

	int rc;

	ch_tcp_record_t tmp_rcd,*tcp_rcd = &tmp_rcd;

	ch_tcp_record_init(tcp_rcd,tsession,PACKET_TYPE_DATA,data,dlen,PROTOCOL_TELNET,SESSION_DIRECT_REQ);

	rc = ch_tcp_record_put(fmt,tcp_rcd);

    return ch_trans_tcp_record_put_returnv(rc);
}

static int response_content_process_for_telnet(ch_tcp_app_t *app ch_unused,ch_shm_format_t *fmt,
	ch_tcp_session_t *tsession,void *data,size_t dlen){

	int rc;

	ch_tcp_record_t tmp_rcd,*tcp_rcd = &tmp_rcd;

	ch_tcp_record_init(tcp_rcd,tsession,PACKET_TYPE_DATA,data,dlen,PROTOCOL_TELNET,SESSION_DIRECT_RES);

	rc = ch_tcp_record_put(fmt,tcp_rcd);

    return ch_trans_tcp_record_put_returnv(rc);
}

static void content_flush_for_telnet(ch_tcp_app_t *app ch_unused,ch_shm_format_t *fmt,
	ch_tcp_session_t *tsession,void *data,size_t dlen){


	ch_tcp_record_t tmp_rcd,*tcp_rcd = &tmp_rcd;

	ch_tcp_record_init(tcp_rcd,tsession,PACKET_TYPE_FLUSH,data,dlen,PROTOCOL_TELNET,SESSION_DIRECT_RES);

	ch_tcp_record_put(fmt,tcp_rcd);

}

static void content_close_for_telnet(ch_tcp_app_t *app ch_unused,ch_shm_format_t *fmt,
	ch_tcp_session_t *tsession,void *data,size_t dlen){


	ch_tcp_record_t tmp_rcd,*tcp_rcd = &tmp_rcd;

	ch_tcp_record_init(tcp_rcd,tsession,PACKET_TYPE_CLOSE,data,dlen,PROTOCOL_TELNET,SESSION_DIRECT_RES);

	ch_tcp_record_put(fmt,tcp_rcd);
}


static ch_tcp_app_t telnet_app = {
    .context = NULL,
	.is_accept_by_port = is_accept_by_port_for_telnet,
	.is_accept_by_content = is_accept_by_content_for_telnet,
	.request_content_process = request_content_process_for_telnet,
	.response_content_process = response_content_process_for_telnet,
	.content_flush = content_flush_for_telnet,
	.content_close = content_close_for_telnet
};



int ch_telnet_init(ch_tcp_app_pool_t *ta_pool,const char *cfname){

	if(do_telnet_context_init(ta_pool->mp,g_tcontext,cfname)){
	
		ch_log(CH_LOG_ERR,"Load TCP APP telnet config file:%s failed!",cfname);
		return -1;
	}

	telnet_app.context = (void*)g_tcontext;

	ch_tcp_app_register(ta_pool,&telnet_app);

	return 0;
}

