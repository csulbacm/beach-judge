#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#include <libwebsockets.h>

//- beachJudge -
#include <Judge/Judge.h>

#define USE_STATIC_ASSETS 0
#define USE_SSL 1

using namespace judge;
using namespace std;

struct timespec g_timespec;
inline unsigned long long getTimeMS()
{
  clock_gettime(CLOCK_REALTIME, &g_timespec);
  return g_timespec.tv_sec * 1000 + g_timespec.tv_nsec / 1000000;
}

static struct libwebsocket_context *context;
static volatile int force_exit = 0;

enum lws_protocols {
	/* always first */
	PROTOCOL_HTTP = 0,

	PROTOCOL_LWS_MIRROR,

	/* always last */
	LWS_PROTOCOL_COUNT
};

const char *resource_path = "../res";

struct per_session_data__http {
	int fd;
	unsigned long long t;
};

const char *get_mimetype(const char *file)
{
	int n = strlen(file);

	if (n < 5)
		return NULL;

	if (!strcmp(&file[n - 4], ".ico"))
		return "image/x-icon";

	if (!strcmp(&file[n - 4], ".png"))
		return "image/png";

	if (!strcmp(&file[n - 5], ".html"))
		return "text/html";

	if (!strcmp(&file[n - 4], ".css"))
		return "text/css";

	return NULL;
}

/* this protocol server (always the first one) just knows how to do HTTP */

static int callback_http(struct libwebsocket_context *context,
		struct libwebsocket *wsi,
		enum libwebsocket_callback_reasons reason, void *user,
							   void *in, size_t len)
{
	char buf[256];
	char leaf_path[1024];
	char b64[64];
	struct timeval tv;
	int n, m;
	unsigned char *p;
	char *other_headers = 0;
	static unsigned char buffer[4096];
	struct stat stat_buf;
	struct per_session_data__http *pss =
			(struct per_session_data__http *)user;
	const char *mimetype;
	unsigned char *end;
	switch (reason) {
	case LWS_CALLBACK_CLOSED_HTTP:
		break;
	case LWS_CALLBACK_HTTP:
		pss->t = getTimeMS();

		if (len < 1) {
			libwebsockets_return_http_status(context, wsi,
						HTTP_STATUS_BAD_REQUEST, NULL);
			goto try_to_reuse;
		}

		/* this server has no concept of directories */
		if (strchr((const char *)in + 1, '/')) {
			libwebsockets_return_http_status(context, wsi,
						HTTP_STATUS_FORBIDDEN, NULL);
			goto try_to_reuse;
		}

		/* if a legal POST URL, let it continue and accept data */
		if (lws_hdr_total_length(wsi, WSI_TOKEN_POST_URI))
			return 0;

#if USE_STATIC_ASSETS
#else
		//TODO: Make favicon
		if (strcmp((const char *)in, "/favicon.ico") == 0) {
			printf("%lx: %s %d\n", (unsigned long)pss, (char *)in, HTTP_STATUS_NO_CONTENT);
			libwebsockets_return_http_status(context, wsi,
				      HTTP_STATUS_NO_CONTENT, NULL);
			return -1;
		}

		/* if not, send a file the easy way */
		strcpy(buf, resource_path);
		if (strcmp((const char *)in, "/")) {
			if (*((const char *)in) != '/')
				strcat(buf, "/");
			strncat(buf, (const char *)in, sizeof(buf) - strlen(resource_path));
		} else /* default file to serve */
			strcat(buf, "/index.html");

		if (strstr((char *)in, ".") == NULL)
			strcat(buf, ".html");

		buf[sizeof(buf) - 1] = '\0';

		//- Handle file not found -
		{
			struct stat fileExist;
			if (stat(buf, &fileExist) != 0) {
				printf("%lx: %s %d\n", (unsigned long)pss, (char *)in, HTTP_STATUS_NOT_FOUND);
				libwebsockets_return_http_status(context, wsi,
					      HTTP_STATUS_NOT_FOUND, NULL);
				return -1;
			}
		}

		/* refuse to serve files we don't understand */
		mimetype = get_mimetype(buf);
		if (!mimetype) {
			lwsl_err("Unknown mimetype for %s\n", buf);
			printf("%lx: %s %d\n", (unsigned long)pss, (char *)in, HTTP_STATUS_UNSUPPORTED_MEDIA_TYPE);
			libwebsockets_return_http_status(context, wsi,
				      HTTP_STATUS_UNSUPPORTED_MEDIA_TYPE, NULL);
			return -1;
		}

		n = libwebsockets_serve_http_file(context, wsi, buf,
						mimetype, other_headers, n);

		printf("%lx: %s 200 - %lld ms\n", (unsigned long)pss, (char *)in, getTimeMS() - pss->t);

		if (n < 0 || ((n > 0) && lws_http_transaction_completed(wsi)))
			return -1; /* error or can't reuse connection: close the socket */
#endif

		/*
		 * notice that the sending of the file completes asynchronously,
		 * we'll get a LWS_CALLBACK_HTTP_FILE_COMPLETION callback when
		 * it's done
		 */

		break;

	case LWS_CALLBACK_HTTP_BODY:
		char username[65];
		char password[65];
		username[64] = 0;
		password[64] = 0;
		
		sscanf((char *)in, "username=%64[a-zA-Z0-9]&password=%64[a-zA-Z0-9]", username, password);
		
		if (strlen(username) != 0 && strlen(password) != 0) {
			//- Login Attempt -
			printf("Login: %s %s\n", username, password);
		}

		break;

	case LWS_CALLBACK_HTTP_BODY_COMPLETION:

		/* the whole of the sent body arrived, close or reuse the connection */
		libwebsockets_return_http_status(context, wsi,
						HTTP_STATUS_OK, NULL);
		goto try_to_reuse;

	case LWS_CALLBACK_HTTP_FILE_COMPLETION:
//		lwsl_info("LWS_CALLBACK_HTTP_FILE_COMPLETION seen\n");
		/* kill the connection after we sent one file */
		goto try_to_reuse;

	case LWS_CALLBACK_HTTP_WRITEABLE:
		/*
		 * we can send more of whatever it is we were sending
		 */
		do {
			/* we'd like the send this much */
			n = sizeof(buffer) - LWS_SEND_BUFFER_PRE_PADDING;
			
			/* but if the peer told us he wants less, we can adapt */
			m = lws_get_peer_write_allowance(wsi);

			/* -1 means not using a protocol that has this info */
			if (m == 0)
				/* right now, peer can't handle anything */
				goto later;

			if (m != -1 && m < n)
				/* he couldn't handle that much */
				n = m;
			
			n = read(pss->fd, buffer + LWS_SEND_BUFFER_PRE_PADDING,
									n);
			/* problem reading, close conn */
			if (n < 0)
				goto bail;
			/* sent it all, close conn */
			if (n == 0)
				goto flush_bail;
			/*
			 * To support HTTP2, must take care about preamble space
			 * 
			 * identification of when we send the last payload frame
			 * is handled by the library itself if you sent a
			 * content-length header
			 */
			m = libwebsocket_write(wsi,
					       buffer + LWS_SEND_BUFFER_PRE_PADDING,
					       n, LWS_WRITE_HTTP);
			if (m < 0)
				/* write failed, close conn */
				goto bail;

			/*
			 * http2 won't do this
			 */
			if (m != n)
				/* partial write, adjust */
				if (lseek(pss->fd, m - n, SEEK_CUR) < 0)
					goto bail;

			if (m) /* while still active, extend timeout */
				libwebsocket_set_timeout(wsi,
					PENDING_TIMEOUT_HTTP_CONTENT, 5);
			
			/* if we have indigestion, let him clear it before eating more */
			if (lws_partial_buffered(wsi))
				break;

		} while (!lws_send_pipe_choked(wsi));

later:
		libwebsocket_callback_on_writable(context, wsi);
		break;
flush_bail:
		/* true if still partial pending */
		if (lws_partial_buffered(wsi)) {
			libwebsocket_callback_on_writable(context, wsi);
			break;
		}
		close(pss->fd);
		goto try_to_reuse;

bail:
		close(pss->fd);
		return -1;

	/*
	 * callback for confirming to continue with client IP appear in
	 * protocol 0 callback since no websocket protocol has been agreed
	 * yet.  You can just ignore this if you won't filter on client IP
	 * since the default uhandled callback return is 0 meaning let the
	 * connection continue.
	 */

	case LWS_CALLBACK_FILTER_NETWORK_CONNECTION:

		/* if we returned non-zero from here, we kill the connection */
		break;


	case LWS_CALLBACK_GET_THREAD_ID:
		/*
		 * if you will call "libwebsocket_callback_on_writable"
		 * from a different thread, return the caller thread ID
		 * here so lws can use this information to work out if it
		 * should signal the poll() loop to exit and restart early
		 */

		/* return pthread_getthreadid_np(); */

		break;

	default:
		break;
	}

	return 0;
	
try_to_reuse:
	if (lws_http_transaction_completed(wsi))
		return -1;

	return 0;
}

/* list of supported protocols and callbacks */

static struct libwebsocket_protocols protocols[] = {
	/* first protocol must always be HTTP handler */

	{
		"http-only",		/* name */
		callback_http,		/* callback */
		sizeof (struct per_session_data__http),	/* per_session_data_size */
		0,			/* max frame size / rx buffer */
	},
	{ NULL, NULL, 0, 0 } /* terminator */
};

int main(int argc, char *argv[])
{
	int n = 0;
	struct lws_context_creation_info info;
	memset(&info, 0, sizeof info);
	{
 	 	info.port = 8081;
		info.protocols = protocols;
		info.gid = -1;
		info.uid = -1;
#if USE_SSL
		char cert_path[1024];
		char key_path[1024];
		int plen_check = strlen(resource_path) + 32;
		if (plen_check > sizeof(cert_path) || plen_check > sizeof(key_path)) {
			lwsl_err("resource path too long\n");
			return -1;
		}
		sprintf(cert_path, JUDGE_SSL_CERT);
		sprintf(key_path, JUDGE_SSL_KEY);
		info.ssl_cert_filepath = cert_path;
		info.ssl_private_key_filepath = key_path;
#endif
		context = libwebsocket_create_context(&info);
	}
	if (context == NULL) {
		lwsl_err("libwebsocket init failed\n");
		return -1;
	}

	//- Load user data -
	User admin("judge", "test");

	n = 0;
	while (n >= 0 && !force_exit) {
		/*
		 * If libwebsockets sockets are all we care about,
		 * you can use this api which takes care of the poll()
		 * and looping through finding who needed service.
		 *
		 * If no socket needs service, it'll return anyway after
		 * the number of ms in the second argument.
		 */

		n = libwebsocket_service(context, 50);
	}

	libwebsocket_context_destroy(context);

	lwsl_notice("libwebsockets-test-server exited cleanly\n");

	return 0;
}
