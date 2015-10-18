#ifndef _JUDGE_WS_JUDGE_H_
#define _JUDGE_WS_JUDGE_H_

// lws
#include <libwebsockets.h>

// beachJudge
#include <Judge/Config.h>
#include <Judge/Session.h>
#include <Judge/Types.h>

namespace judge {

struct a_message {
	void *payload;
	u16 len;
};

typedef struct psd_judge {
	libwebsocket *wsi;
	u16 ringbuffer_tail;
	a_message ringbuffer[JUDGE_MAX_MESSAGE_QUEUE];
	u16 ringbuffer_head;

	struct Service *service;
	u16 width;
	u16 height;
	u64 lastSendMS;
	char buffer[JUDGE_MAX_RESPONSE + LWS_SEND_BUFFER_PRE_PADDING + LWS_SEND_BUFFER_POST_PADDING];
	char *msg;
	Session *session;
	User *user;
} psd_judge;

typedef void (*func_judge)(libwebsocket *wsi, psd_judge *pss, char *msgIn);

int ws_judge(libwebsocket_context *context,
	libwebsocket *wsi,
	libwebsocket_callback_reasons reason,
	void *user, void *in, size_t len);

}

#endif
