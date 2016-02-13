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
	u32 targetLen, bodyLen;
	char *target, *body;

	psd_http() :
		fd(0),
		t(0),
		user(0),
		session(0),
		target(0),
		body(0),
		targetLen(0),
		bodyLen(0)
	{
	}

	void Purge()
	{
		delete [] target;
		delete [] body;
	}
} psd_http;

int ws_http(lws *wsi,
	lws_callback_reasons reason, void *user,
	void *in, size_t len);

extern lws_context *g_lws_context;

}

#endif
