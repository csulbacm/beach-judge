#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>

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

static unsigned long long sessionExpireTimeMS = 1000 * 60 * 60 * 1;
typedef struct Session {
	User *user;
	unsigned long long expireTimeMS;

	Session() :
		user(0),
		expireTimeMS(0)
	{
	}

	Session(User *user) :
		user(user)
	{
		Reset();
	}

	Session(User *user, unsigned long long timeMS) :
		user(user),
		expireTimeMS(timeMS)
	{
	}

	void Reset()
	{
		expireTimeMS = getTimeMS() + sessionExpireTimeMS;
	}
} Session;

static map<string, Session> g_sessionMap = map<string, Session>();

void loadJudgeData()
{
	//- Create default judge account -
	if (User::s_usersByName.count("judge") == 0) {
		printf("Creating judge account...\n");
		new User("judge", "test", true);
	}

	printf("Loading user sessions...\n");
	{
		ifstream file(".sessions");
		if (file.is_open()) {
			string sessID, name;
			unsigned long long expireTimeMS;
			while (file >> sessID >> name >> expireTimeMS) {
				if (User::s_usersByName.count(name) == 0)
					continue;
				g_sessionMap[sessID] = Session(User::s_usersByName[name], expireTimeMS);
			}
			file.close();
		}
	}
}
void saveJudgeData()
{
	printf("Saving user sessions...\n");
	{
		ofstream file;
		file.open(".sessions");
		std::map<std::string, Session>::iterator it = g_sessionMap.begin();
		std::map<std::string, Session>::iterator end = g_sessionMap.end();
		while (it != end) {
			file << it->first
				<< ' ' << it->second.user->username.c_str() 
				<< ' ' << it->second.expireTimeMS
				<< '\n';
			++it;
		}
		file.close();
	}
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
	User *user;
	Session *session;
	char sessionID[65];
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

	if (!strcmp(&file[n - 3], ".js"))
		return "text/javascript";

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
	case LWS_CALLBACK_FILTER_HTTP_CONNECTION: {
		int n = 0;
		char buf[256];
		const unsigned char *c;
		do {
			c = lws_token_to_string((lws_token_indexes)n);
			if (!c) {
				n++;
				continue;
			}
	
			if (!lws_hdr_total_length(wsi, (lws_token_indexes)n)) {
				n++;
				continue;
			}
	
			lws_hdr_copy(wsi, buf, sizeof buf, (lws_token_indexes)n);
			if (memcmp(c, "cookie:", 7) == 0) {
				char sessID[65];
				sessID[64] = 0;
				if (sscanf(buf, "JUDGESESSID=%[0-9a-zA-Z]", sessID) == 1) {
					string sessionID = string(sessID);
					if (g_sessionMap.count(sessionID) != 0) {
						pss->session = &g_sessionMap[sessionID];
						pss->session->Reset();
						pss->user = pss->session->user;
						memcpy(pss->sessionID, sessID, 65);
					}
				}
			}

			n++;
		} while (c);
		break;
	}
	case LWS_CALLBACK_CLOSED_HTTP:
		break;
	case LWS_CALLBACK_HTTP: {
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
		if (lws_hdr_total_length(wsi, WSI_TOKEN_POST_URI)) {
			printf("%lx: POST %s\n", (unsigned long)pss, (char *)in);

			if (pss->user != 0) {
				if (memcmp(in, "/logout", 7) == 0) {
					printf("  %lx: Logout %s\n", (unsigned long)pss, pss->user->username.c_str());
					g_sessionMap.erase(string(pss->sessionID));

					//TODO: Handle invalid pw error
					char response[128];
					sprintf(response, "HTTP/1.0 200 OK\r\n"
						"Connection: close\r\n"
						"Content-Type: text/html; charset=UTF-8\r\n"
						"Content-Length: %d\r\n\r\n"
						"%s\r\n",
						2, "OK");
					libwebsocket_write(wsi,
						(unsigned char *)response,
						strlen(response), LWS_WRITE_HTTP);

					goto try_to_reuse;
				}
			}

			return 0;
		}

#if USE_STATIC_ASSETS
#else
		//TODO: Make favicon
		if (strcmp((const char *)in, "/favicon.ico") == 0) {
			printf("%lx: GET %s %d\n", (unsigned long)pss, (char *)in, HTTP_STATUS_NO_CONTENT);
			libwebsockets_return_http_status(context, wsi,
				      HTTP_STATUS_NO_CONTENT, NULL);
			return -1;
		}

		/* if not, send a file the easy way */
		strcpy(buf, resource_path);

		//TODO: Patch login access on logged in
		//TODO: Restrict content loading to respective user groups
//		if (lws_is_ssl(wsi) == 0) {
//			strcat(buf,"/nonssl.html");
		//} else if (strstr((char *)in, ".") == NULL) {
		if (strstr((char *)in, ".") == NULL) {
			if (pss->user == 0)
				strcat(buf, "/login");
			else if(pss->user->isJudge)
				strcat(buf, "/judge");
			else
				strcat(buf, "/team");
			strcat(buf, ".html");
		} else if (strcmp((const char *)in, "/")) {
			if (*((const char *)in) != '/')
				strcat(buf, "/");
			strncat(buf, (const char *)in, sizeof(buf) - strlen(resource_path));
		}

		buf[sizeof(buf) - 1] = '\0';

		//- Handle file not found -
		{
			struct stat fileExist;
			if (stat(buf, &fileExist) != 0) {
				printf("%lx: GET %s %d\n", (unsigned long)pss, (char *)in, HTTP_STATUS_NOT_FOUND);
				libwebsockets_return_http_status(context, wsi,
					      HTTP_STATUS_NOT_FOUND, NULL);
				return -1;
			}
		}

		/* refuse to serve files we don't understand */
		mimetype = get_mimetype(buf);
		if (!mimetype) {
			lwsl_err("Unknown mimetype for %s\n", buf);
			printf("%lx: GET %s %d\n", (unsigned long)pss, (char *)in, HTTP_STATUS_UNSUPPORTED_MEDIA_TYPE);
			libwebsockets_return_http_status(context, wsi,
				      HTTP_STATUS_UNSUPPORTED_MEDIA_TYPE, NULL);
			return -1;
		}

		n = libwebsockets_serve_http_file(context, wsi, buf,
						mimetype, other_headers, n);

		printf("%lx: GET %s 200 - %lld ms\n", (unsigned long)pss, (char *)in, getTimeMS() - pss->t);

		if (n < 0 || ((n > 0) && lws_http_transaction_completed(wsi)))
			return -1; /* error or can't reuse connection: close the socket */
#endif

		/*
		 * notice that the sending of the file completes asynchronously,
		 * we'll get a LWS_CALLBACK_HTTP_FILE_COMPLETION callback when
		 * it's done
		 */

		break;
	}
	case LWS_CALLBACK_HTTP_BODY:
		char nameBuff[65];
		char pwBuff[65];
		nameBuff[64] = 0;
		pwBuff[64] = 0;

		{
			char body[len + 1];
			body[len] = 0;
			memcpy(body, in, len);
			body[len] = 0;
			if (sscanf(body, "username=%64[a-zA-Z0-9]&password=%64[a-zA-Z0-9]", nameBuff, pwBuff) == 2) {
				char response[128];
	
				//- Login Attempt -
				printf("  %lx: Login %s\n", (unsigned long)pss, nameBuff);
				string name(nameBuff);
				if (User::s_usersByName.count(name) != 0) {
					User *user = User::s_usersByName[name];
					if (user->TestPassword(pwBuff)) {
						//- Generate Session Key -
						unsigned int l = SHA256_DIGEST_LENGTH * 2;
						char sessionKey[l + 1];
						char randKey[21];
						sessionKey[l] = 0;
						randKey[20] = 0;
						string sessID;
						do
						{
							sprintf(randKey, "%010d%010d", rand(), rand());
							unsigned char hash[SHA256_DIGEST_LENGTH];
							SHA256_CTX sha256;
							SHA256_Init(&sha256);
							SHA256_Update(&sha256, randKey, strlen(randKey));
							SHA256_Final(hash, &sha256);
							for (int a = 0; a < SHA256_DIGEST_LENGTH; ++a)
								sprintf(sessionKey + (a << 1), "%02x", hash[a]);
							sessID = string(sessionKey);
						} while (g_sessionMap.count(sessID));
	
						//- Update Session Map -
						g_sessionMap[sessID] = Session(user);
	
						//- Send Key to Client -
						sprintf(response, "HTTP/1.0 200 OK\r\n"
							"Connection: close\r\n"
							"Content-Type: text/html; charset=UTF-8\r\n"
							"Content-Length: %ld\r\n\r\n"
							"%s\r\n",
							strlen(sessionKey), sessionKey);
						libwebsocket_write(wsi,
							(unsigned char *)response,
							strlen(response), LWS_WRITE_HTTP);
					} else {
						//TODO: Handle invalid pw error
						sprintf(response, "HTTP/1.0 200 OK\r\n"
							"Connection: close\r\n"
							"Content-Type: text/html; charset=UTF-8\r\n"
							"Content-Length: %d\r\n\r\n"
							"%s\r\n",
							3, "ERR");
						libwebsocket_write(wsi,
							(unsigned char *)response,
							strlen(response), LWS_WRITE_HTTP);
					}
				} else {
					//TODO: Handle user does not exist
					sprintf(response, "HTTP/1.0 200 OK\r\n"
						"Connection: close\r\n"
						"Content-Type: text/html; charset=UTF-8\r\n"
						"Content-Length: %d\r\n\r\n"
						"%s\r\n",
						3, "ERR");
					libwebsocket_write(wsi,
						(unsigned char *)response,
						strlen(response), LWS_WRITE_HTTP);
				}
			}
		}

		goto try_to_reuse;

	case LWS_CALLBACK_HTTP_BODY_COMPLETION:

		/* the whole of the sent body arrived, close or reuse the connection */
//		libwebsockets_return_http_status(context, wsi,
//						HTTP_STATUS_OK, NULL);
//		goto try_to_reuse;
		return 0;

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

#define MAX_MESSAGE_QUEUE 32
#define MAX_RESPONSE 2047

struct per_session_data__judge {
	struct libwebsocket *wsi;
	int								 ringbuffer_tail;
	struct Service			*service;
	int		 pty;
	int			 width;
	int		 height;
	long long		 lastSendMS;
	char		buffer[MAX_RESPONSE + LWS_SEND_BUFFER_PRE_PADDING + LWS_SEND_BUFFER_POST_PADDING];
	char			*msg;
	Session *session;
	User *user;
};

struct a_message {
	void *payload;
	size_t len;
};

static struct a_message ringbuffer[MAX_MESSAGE_QUEUE];
static int ringbuffer_head;

static int
callback_judge(struct libwebsocket_context *context,
			struct libwebsocket *wsi,
			enum libwebsocket_callback_reasons reason,
								 void *user, void *in, size_t len)
{
	int n;
	struct per_session_data__judge *pss = (struct per_session_data__judge *)user;

	switch (reason) {

	case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION: {
		int n = 0;
		char buf[256];
		const unsigned char *c;
		do {
			c = lws_token_to_string((lws_token_indexes)n);
			if (!c) {
				n++;
				continue;
			}
	
			if (!lws_hdr_total_length(wsi, (lws_token_indexes)n)) {
				n++;
				continue;
			}
	
			lws_hdr_copy(wsi, buf, sizeof buf, (lws_token_indexes)n);
			if (memcmp(c, "cookie:", 7) == 0) {
				char sessID[65];
				sessID[64] = 0;
				if (sscanf(buf, "JUDGESESSID=%[0-9a-zA-Z]", sessID) == 1) {
					string sessionID = string(sessID);
					if (g_sessionMap.count(sessionID) != 0) {
						pss->session = &g_sessionMap[sessionID];
						pss->session->Reset();
						pss->user = pss->session->user;
					}
				}
			}

			n++;
		} while (c);
		break;
	}

	case LWS_CALLBACK_ESTABLISHED:
		lwsl_info("callback_judge: LWS_CALLBACK_ESTABLISHED\n");
		pss->ringbuffer_tail = ringbuffer_head;
		pss->wsi = wsi;

		//- Initialize Session -
//		pss->pty = -1;
		pss->width = 0;
		pss->height = 0;
		pss->lastSendMS = 0;
		pss->msg = pss->buffer + LWS_SEND_BUFFER_PRE_PADDING;
//		printf("%lx: Connected\n", (unsigned long)pss);
		break;

	case LWS_CALLBACK_CLOSED:
//		if (pss->pty >= 0)
//			NOINTR(close(pss->pty));
//		printf("%lx: Disconnected\n", (unsigned long)pss);
		break;

	case LWS_CALLBACK_PROTOCOL_DESTROY:
		lwsl_notice("mirror protocol cleaning up\n");
		for (n = 0; n < sizeof ringbuffer / sizeof ringbuffer[0]; n++)
			if (ringbuffer[n].payload)
				free(ringbuffer[n].payload);
		break;

	case LWS_CALLBACK_SERVER_WRITEABLE:
		while (pss->ringbuffer_tail != ringbuffer_head) {

			//- Input Processing -
			char response[1024];
			memset(response, 0, 1024);
			//TODO: Handle buffer len
			char *msgIn = (char *)ringbuffer[pss->ringbuffer_tail].payload + LWS_SEND_BUFFER_PRE_PADDING;
			char *msgOut = response + LWS_SEND_BUFFER_PRE_PADDING;
			if (memcmp(msgIn, "POP", 3) == 0) {
				//- Populate User Session Data -
				sprintf(msgOut, ""
					"\"msg\": \"POP\","
					"\"name\": \"%s\"",
					pss->user->username.c_str());
			} else if (memcmp(msgIn, "TL", 2) == 0) {
				//- Populate Team Data -
				stringstream users;
				map<string, User *>::iterator it = User::s_usersByName.begin();
				map<string, User *>::iterator end = User::s_usersByName.end();
				while (it != end) {
					users << it->second->username.c_str();
					++it;
					if (it != end)
						users << ", ";
				}
				sprintf(msgOut, ""
					"\"msg\": \"TL\","
					"\"teams\": [ \"%s\" ]",
					users.str().c_str());
			} else {
				sprintf(msgOut, ""
					"\"msg\": \"ERR\"");
			}
			libwebsocket_write(wsi,
				(unsigned char *)msgOut,
				strlen(msgOut), LWS_WRITE_TEXT);

			if (pss->ringbuffer_tail == (MAX_MESSAGE_QUEUE - 1))
				pss->ringbuffer_tail = 0;
			else
				pss->ringbuffer_tail++;

			if (((ringbuffer_head - pss->ringbuffer_tail) &
					(MAX_MESSAGE_QUEUE - 1)) == (MAX_MESSAGE_QUEUE - 15))
				libwebsocket_rx_flow_allow_all_protocol(
								 libwebsockets_get_protocol(wsi));

			// lwsl_debug("tx fifo %d\n", (ringbuffer_head - pss->ringbuffer_tail) & (MAX_MESSAGE_QUEUE - 1));

			if (lws_partial_buffered(wsi) || lws_send_pipe_choked(wsi)) {
				libwebsocket_callback_on_writable(context, wsi);
				break;
			}
			/*
			 * for tests with chrome on same machine as client and
			 * server, this is needed to stop chrome choking
			 */
#ifdef _WIN32
			Sleep(1);
#else
			usleep(1);
#endif
		}

		break;

	case LWS_CALLBACK_RECEIVE:
		if (((ringbuffer_head - pss->ringbuffer_tail) &
					(MAX_MESSAGE_QUEUE - 1)) == (MAX_MESSAGE_QUEUE - 1)) {
			lwsl_err("dropping!\n");
			goto choke;
		}

		if (ringbuffer[ringbuffer_head].payload)
			free(ringbuffer[ringbuffer_head].payload);

		ringbuffer[ringbuffer_head].payload =
				malloc(LWS_SEND_BUFFER_PRE_PADDING + len +
							LWS_SEND_BUFFER_POST_PADDING);
		ringbuffer[ringbuffer_head].len = len;
		memcpy((char *)ringbuffer[ringbuffer_head].payload +
						LWS_SEND_BUFFER_PRE_PADDING, in, len);
		if (ringbuffer_head == (MAX_MESSAGE_QUEUE - 1))
			ringbuffer_head = 0;
		else
			ringbuffer_head++;

		if (((ringbuffer_head - pss->ringbuffer_tail) &
					(MAX_MESSAGE_QUEUE - 1)) != (MAX_MESSAGE_QUEUE - 2))
			goto done;

choke:
		lwsl_debug("LWS_CALLBACK_RECEIVE: throttling %p\n", wsi);
		libwebsocket_rx_flow_control(wsi, 0);

//		lwsl_debug("rx fifo %d\n", (ringbuffer_head - pss->ringbuffer_tail) & (MAX_MESSAGE_QUEUE - 1));
done:
		libwebsocket_callback_on_writable_all_protocol(
								 libwebsockets_get_protocol(wsi));
		break;

	default:
		break;
	}

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
	{
		"judge-protocol",
		callback_judge,
		sizeof(struct per_session_data__judge),
		128,
	},
	{ NULL, NULL, 0, 0 } /* terminator */
};

static volatile int   exiting;
static sigjmp_buf     jmpenv;
static void sigHandler(int signo, siginfo_t *info, void *context) {
	if (exiting++) {
		_exit(1);
	}
	siglongjmp(jmpenv, 1);
}

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
		info.options = LWS_SERVER_OPTION_ALLOW_NON_SSL_ON_SSL_PORT;
#endif
		context = libwebsocket_create_context(&info);
	}
	if (context == NULL) {
		lwsl_err("libwebsocket init failed\n");
		return -1;
	}

	//- Load beachJudge Data  -
	printf("Loading beachJudge data...\n");
	loadJudgeData();

	//- Start Server -
	printf("Server is running.\n");
	if (!sigsetjmp(jmpenv, 1)) {
		// Clean up upon orderly shut down. Do _not_ cleanup if we die
		// unexpectedly, as we cannot guarantee if we are still in a valid
		// static. This means, we should never catch SIGABRT.
		static const int signals[] = { SIGHUP, SIGINT, SIGQUIT, SIGTERM };
		struct sigaction sa;
		memset(&sa, 0, sizeof(sa));
		sa.sa_sigaction = sigHandler;
		sa.sa_flags	 = SA_SIGINFO | SA_RESETHAND;
		for (int i = 0; i < sizeof(signals)/sizeof(*signals); ++i) {
			sigaction(signals[i], &sa, NULL);
		}

		n = 0;
		while (n >= 0 && !force_exit) {
			n = libwebsocket_service(context, 50);
		}
	}

	libwebsocket_context_destroy(context);
	lwsl_notice("libwebsockets-test-server exited cleanly\n");

	printf("Saving beachJudge data...\n");
	saveJudgeData();
	User::Cleanup();

	return 0;
}
