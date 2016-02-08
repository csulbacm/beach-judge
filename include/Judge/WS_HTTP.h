#ifndef _JUDGE_WS_HTTP_H_
#define _JUDGE_WS_HTTP_H_

#include <string>

// lws
#include <libwebsockets.h>

// beachJudge
#include <Judge/Types.h>
#include <Judge/Session.h>

namespace judge {

extern const char *g_resourcePath;

typedef struct psd_http {
	int fd;
	u64 t;
	User *user;
	Session *session;
	char sessionID[65];
} psd_http;

int ws_http(libwebsocket_context *context,
	libwebsocket *wsi,
	libwebsocket_callback_reasons reason, void *user,
	void *in, size_t len);


}

#endif
