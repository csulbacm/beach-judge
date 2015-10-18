#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <map>

// sqlite
#include <sqlite3.h>

// lws
#include <libwebsockets.h>

// beachJudge
#include <Judge/Judge.h>

using namespace judge;
using namespace std;

map<string, Session> Session::s_sessionMap = map<string, Session>();

#define MAX_MESSAGE_QUEUE 32
#define MAX_RESPONSE 2047

struct a_message {
	void *payload;
	u16 len;
};

struct per_session_data__judge {
	libwebsocket *wsi;
	u16 ringbuffer_tail;
	a_message ringbuffer[MAX_MESSAGE_QUEUE];
	u16 ringbuffer_head;

	struct Service *service;
	u16 width;
	u16 height;
	u64 lastSendMS;
	char buffer[MAX_RESPONSE + LWS_SEND_BUFFER_PRE_PADDING + LWS_SEND_BUFFER_POST_PADDING];
	char *msg;
	Session *session;
	User *user;
};


//----------------------------------------------
//------------- Message Handling ---------------

map<string, void (*)(libwebsocket *wsi,
	per_session_data__judge *pss, char *msgIn)> g_msgHandlerMap;

void msg_populate(libwebsocket *wsi, per_session_data__judge *pss, char *msgIn)
{
	// Populate User Session Data
	sprintf(pss->msg, ""
		"\"msg\":\"POP\","
		"\"name\":\"%s\"",
		pss->user->username.c_str());
}

void msg_teamList(libwebsocket *wsi, per_session_data__judge *pss, char *msgIn)
{
	// Populate Team Data
	stringstream users;
	map<string, User *>::iterator it = User::s_usersByName.begin();
	map<string, User *>::iterator end = User::s_usersByName.end();
	char entry[64];
	while (it != end) {
		sprintf(entry, "{\"i\":\"%04x\",\"n\":\"%s\"}",
			it->second->id, it->second->username.c_str());
		users << entry;
		++it;
		if (it != end)
			users << "\",\"";
	}
	sprintf(pss->msg, ""
		"\"msg\":\"TL\","
		"\"teams\":[%s]",
		users.str().c_str());
}

void msg_createTeam(libwebsocket *wsi, per_session_data__judge *pss, char *msgIn)
{
	// Restrict action to judge
	if (pss->user->isJudge == false) {
		sprintf(pss->msg, "\"msg\": \"ERR\"");
		return;
	}

	//TODO: Define name/password length requirements
	//TODO: Split username and display name
	char name[16], p1[16], p2[16];
	i16 r = sscanf(msgIn, "n=%[a-zA-Z0-9]&p1=%[a-zA-Z0-9]&p2=%[a-zA-Z0-9]", name, p1, p2);
	if (r != 3) {
		sprintf(pss->msg, ""
			"\"msg\":\"CT\","
			"\"err\":\"I\"");
		return;
	}
	if (strcmp(p1, p2) != 0) {
		//TODO: memcpy prebuilt errors
		sprintf(pss->msg, ""
			"\"msg\":\"CT\","
			"\"err\":\"P\"");
		return;
	}

	// Set name to lower case
	{
		i16 l = strlen(name);
		for (i16 a = 0; a < l; ++a)
			name[a] = tolower(name[a]);
	}

	if (User::s_usersByName.count(name) != 0) {
		sprintf(pss->msg, ""
			"\"msg\":\"CT\","
			"\"err\":\"N\"");
		return;
	}

	User *newUser = new User(name, p1, false);
	sprintf(pss->msg, ""
		"\"msg\":\"CT\","
		"\"i\":\"%04x\","
		"\"n\":\"%s\"",
		newUser->id,
		newUser->username.c_str());
}

void populateMsgHandlerMap(map<string, void (*)(libwebsocket *wsi,
	per_session_data__judge *pss, char *msgIn)> &m)
{
	m["POP"] = msg_populate;
	m["TL"] = msg_teamList;
	m["CT"] = msg_createTeam;
}

libwebsocket_context *context;
volatile int force_exit = 0;

enum lws_protocols {
	PROTOCOL_HTTP = 0,
	PROTOCOL_LWS_MIRROR,
	LWS_PROTOCOL_COUNT
};

const char *resource_path = "../res";

struct per_session_data__http {
	int fd;
	u64 t;
	User *user;
	Session *session;
	char sessionID[65];
};

/* this protocol server (always the first one) just knows how to do HTTP */

//TODO: Simplify this function
int callback_http(libwebsocket_context *context,
	libwebsocket *wsi,
	libwebsocket_callback_reasons reason, void *user,
	void *in, size_t len)
{
	char buf[256];
	char leaf_path[1024];
	char b64[64];
	timeval tv;
	int n, m;
	unsigned char *p;
	char *other_headers = 0;
	unsigned char buffer[4096];
	struct stat stat_buf;
	per_session_data__http *pss =
			(per_session_data__http *)user;
	const char *mimetype;
	unsigned char *end;
	switch (reason) {
	case LWS_CALLBACK_FILTER_HTTP_CONNECTION: {
		i16 n = 0;
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
					if (Session::s_sessionMap.count(sessionID) != 0) {
						pss->session = &Session::s_sessionMap[sessionID];
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
		pss->t = getRealTimeMS();

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
			printf("%p: POST %s\n", pss, (char *)in);

			if (pss->user != 0) {
				if (memcmp(in, "/logout", 7) == 0) {
					printf("  %p: Logout %s\n", pss, pss->user->username.c_str());
					Session::s_sessionMap.erase(string(pss->sessionID));

					//TODO: Handle invalid pw error
					char response[128];
					sprintf(response, "HTTP/1.0 200 OK\r\n"
						"Connection: close\r\n"
						"Content-Type: text/html; charset=UTF-8\r\n"
						"Content-Length: %d\r\n\r\n"
						"%s",
						2, "OK");
					libwebsocket_write(wsi,
						(unsigned char *)response,
						strlen(response), LWS_WRITE_HTTP);

					goto try_to_reuse;
				}
			}

			return 0;
		}

#if JUDGE_STATIC_ASSETS
#else
		//TODO: Make favicon
		if (strcmp((const char *)in, "/favicon.ico") == 0) {
			printf("%p: GET %s %d\n", pss, (char *)in, HTTP_STATUS_NO_CONTENT);
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

		// Handle file not found
		{
			struct stat fileExist;
			if (stat(buf, &fileExist) != 0) {
				printf("%p: GET %s %d\n", pss, (char *)in, HTTP_STATUS_NOT_FOUND);
				libwebsockets_return_http_status(context, wsi,
					HTTP_STATUS_NOT_FOUND, NULL);
				return -1;
			}
		}

		/* refuse to serve files we don't understand */
		mimetype = getMimeType(buf);
		if (!mimetype) {
			lwsl_err("Unknown mimetype for %s\n", buf);
			printf("%p: GET %s %d\n", pss, (char *)in, HTTP_STATUS_UNSUPPORTED_MEDIA_TYPE);
			libwebsockets_return_http_status(context, wsi,
				HTTP_STATUS_UNSUPPORTED_MEDIA_TYPE, NULL);
			return -1;
		}

		n = libwebsockets_serve_http_file(context, wsi, buf,
						mimetype, other_headers, n);

		printf("%p: GET %s 200 - %ld ms\n", pss, (char *)in, getRealTimeMS() - pss->t);

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
		char response[128];

		{
			char body[len + 1];
			body[len] = 0;
			memcpy(body, in, len);
			body[len] = 0;
			if (sscanf(body, "username=%64[a-zA-Z0-9]&password=%64[a-zA-Z0-9]", nameBuff, pwBuff) == 2) {
	
				// Login Attempt
				printf("  %p: Login %s\n", pss, nameBuff);
				string name(nameBuff);
				if (User::s_usersByName.count(name) != 0) {
					User *user = User::s_usersByName[name];
					if (user->TestPassword(pwBuff)) {
						// Generate Session Key
						u16 l = SHA256_DIGEST_LENGTH * 2;
						char sessionKey[l + 1];
						char randKey[21];
						sessionKey[l] = 0;
						randKey[20] = 0;
						string sessID;
						do {
							sprintf(randKey, "%010d%010d", rand(), rand());
							unsigned char hash[SHA256_DIGEST_LENGTH];
							SHA256_CTX sha256;
							SHA256_Init(&sha256);
							SHA256_Update(&sha256, randKey, strlen(randKey));
							SHA256_Final(hash, &sha256);
							for (i16 a = 0; a < SHA256_DIGEST_LENGTH; ++a)
								sprintf(sessionKey + (a << 1), "%02x", hash[a]);
							sessID = string(sessionKey);
						} while (Session::s_sessionMap.count(sessID));
	
						// Update Session Map
						Session::s_sessionMap[sessID] = Session(user);
	
						// Send Key to Client
						sprintf(response, "HTTP/1.0 200 OK\r\n"
							"Connection: close\r\n"
							"Content-Type: text/html; charset=UTF-8\r\n"
							"Content-Length: %ld\r\n\r\n"
							"%s",
							strlen(sessionKey), sessionKey);
						libwebsocket_write(wsi,
							(unsigned char *)response,
							strlen(response), LWS_WRITE_HTTP);
					} else goto login_failed;
				} else goto login_failed;
			} else goto login_failed;
		}
		goto try_to_reuse;

login_failed:
		sprintf(response, "HTTP/1.0 200 OK\r\n"
			"Connection: close\r\n"
			"Content-Type: text/html; charset=UTF-8\r\n"
			"Content-Length: %d\r\n\r\n"
			"%s",
			3, "ERR");
		libwebsocket_write(wsi,
			(unsigned char *)response,
			strlen(response), LWS_WRITE_HTTP);

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
			printf("R: %d\n", n);
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
	default:
		break;
	}

	return 0;
	
try_to_reuse:
	if (lws_http_transaction_completed(wsi))
		return -1;

	return 0;
}

int callback_judge(libwebsocket_context *context,
	libwebsocket *wsi,
	libwebsocket_callback_reasons reason,
	void *user, void *in, size_t len)
{
	i16 n;
	per_session_data__judge *pss = (per_session_data__judge *)user;

	switch (reason) {

	case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION: {
		i16 n = 0;
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
					if (Session::s_sessionMap.count(sessionID) != 0) {
						pss->session = &Session::s_sessionMap[sessionID];
						pss->session->Reset();
						pss->user = pss->session->user;
					}
				}
			}

			n++;
		} while (c);
		
		// Block connections without valid session
		if (pss->user == 0)
			return -1;

		break;
	}

	case LWS_CALLBACK_ESTABLISHED:
		lwsl_info("callback_judge: LWS_CALLBACK_ESTABLISHED\n");
		pss->ringbuffer_tail = pss->ringbuffer_head;
		pss->wsi = wsi;

		// Initialize Session
		pss->width = 0;
		pss->height = 0;
		pss->lastSendMS = 0;
		pss->msg = pss->buffer + LWS_SEND_BUFFER_PRE_PADDING;

		printf("%p: Connected\n", pss);
		break;

	case LWS_CALLBACK_CLOSED:
		printf("%p: Disconnected\n", pss);
		break;

	case LWS_CALLBACK_PROTOCOL_DESTROY:
		for (n = 0; n < sizeof pss->ringbuffer / sizeof pss->ringbuffer[0]; n++)
			if (pss->ringbuffer[n].payload)
				free(pss->ringbuffer[n].payload);
		break;

	case LWS_CALLBACK_SERVER_WRITEABLE:
		while (pss->ringbuffer_tail != pss->ringbuffer_head) {
			// Input Processing
			char *msgIn = (char *)pss->ringbuffer[pss->ringbuffer_tail].payload + LWS_SEND_BUFFER_PRE_PADDING;
			char msgType[5];
			msgType[4] = 0;
			sscanf(msgIn, "%4[a-zA-Z]:", msgType);
			if (g_msgHandlerMap.count(msgType))
				(*g_msgHandlerMap[msgType])(wsi, pss, msgIn + strlen(msgType) + 1);
			else
				sprintf(pss->msg, "\"msg\": \"ERR\"");
			libwebsocket_write(wsi,
				(unsigned char *)pss->msg,
				strlen(pss->msg), LWS_WRITE_TEXT);

			if (pss->ringbuffer_tail == (MAX_MESSAGE_QUEUE - 1))
				pss->ringbuffer_tail = 0;
			else
				pss->ringbuffer_tail++;

			if (((pss->ringbuffer_head - pss->ringbuffer_tail) &
					(MAX_MESSAGE_QUEUE - 1)) == (MAX_MESSAGE_QUEUE - 15))
				libwebsocket_rx_flow_allow_all_protocol(
								 libwebsockets_get_protocol(wsi));

//			lwsl_debug("tx fifo %d\n", (pss->ringbuffer_head - pss->ringbuffer_tail) & (MAX_MESSAGE_QUEUE - 1));

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
		if (((pss->ringbuffer_head - pss->ringbuffer_tail) &
					(MAX_MESSAGE_QUEUE - 1)) == (MAX_MESSAGE_QUEUE - 1)) {
			lwsl_err("dropping!\n");
			goto choke;
		}

		if (pss->ringbuffer[pss->ringbuffer_head].payload)
			free(pss->ringbuffer[pss->ringbuffer_head].payload);

		pss->ringbuffer[pss->ringbuffer_head].payload =
				malloc(LWS_SEND_BUFFER_PRE_PADDING + len +
							LWS_SEND_BUFFER_POST_PADDING);
		pss->ringbuffer[pss->ringbuffer_head].len = len;
		memcpy((char *)pss->ringbuffer[pss->ringbuffer_head].payload +
						LWS_SEND_BUFFER_PRE_PADDING, in, len + 1);
		if (pss->ringbuffer_head == (MAX_MESSAGE_QUEUE - 1))
			pss->ringbuffer_head = 0;
		else
			pss->ringbuffer_head++;

		if (((pss->ringbuffer_head - pss->ringbuffer_tail) &
					(MAX_MESSAGE_QUEUE - 1)) != (MAX_MESSAGE_QUEUE - 2))
			goto done;

choke:
		lwsl_debug("LWS_CALLBACK_RECEIVE: throttling %p\n", wsi);
		libwebsocket_rx_flow_control(wsi, 0);

//		lwsl_debug("rx fifo %d\n", (pss->ringbuffer_head - pss->ringbuffer_tail) & (MAX_MESSAGE_QUEUE - 1));
done:
		libwebsocket_callback_on_writable_all_protocol(
								 libwebsockets_get_protocol(wsi));
		break;

	default:
		break;
	}

	return 0;
}

libwebsocket_protocols protocols[] = {
	{ // HTTP Handler
		"http-only",		/* name */
		callback_http,		/* callback */
		sizeof (per_session_data__http),	/* per_session_data_size */
		0,			/* max frame size / rx buffer */
	},
	{ // beachJudge Protocol
		"judge-protocol",
		callback_judge,
		sizeof(per_session_data__judge),
		128,
	},
	{ NULL, NULL, 0, 0 } /* terminator */
};

volatile i16 exiting = 0;
sigjmp_buf jmpenv;
void sigHandler(int signo, siginfo_t *info, void *context) {
	if (exiting++)
		_exit(1);
	siglongjmp(jmpenv, 1);
}

int main(int argc, char *argv[])
{
	srand(time(NULL));

	// Populate Msg Handler Map
	populateMsgHandlerMap(g_msgHandlerMap);

	// Initialize LWS
	lws_set_log_level(0, NULL);
	lws_context_creation_info info;
	memset(&info, 0, sizeof info);
	{
		info.port = 8081;
		info.protocols = protocols;
		info.gid = -1;
		info.uid = -1;
#if JUDGE_SSL
		char cert_path[1024];
		char key_path[1024];
		u16 plen_check = strlen(resource_path) + 32;
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

	// Load beachJudge Data
	printf("Loading beachJudge data...\n");
	loadJudgeData();


	// SQLite Test
	{
		printf("Opening SQLIte Database...\n");
		sqlite3 *db;
		int c = sqlite3_open(".data", &db);
		printf("   C: %d\n", c);
		sqlite3_close(db);
	}

	// Start Server
	printf("Server is running.\n");
	if (!sigsetjmp(jmpenv, 1)) {
		// Clean up upon orderly shut down. Do _not_ cleanup if we die
		// unexpectedly, as we cannot guarantee if we are still in a valid
		// static. This means, we should never catch SIGABRT.
		static const i16 signals[] = { SIGHUP, SIGINT, SIGQUIT, SIGTERM };
		struct sigaction sa;
		memset(&sa, 0, sizeof(sa));
		sa.sa_sigaction = sigHandler;
		sa.sa_flags	 = SA_SIGINFO | SA_RESETHAND;
		for (u16 i = 0; i < sizeof(signals) / sizeof(*signals); ++i)
			sigaction(signals[i], &sa, NULL);

		// Main Loop
		i16 n = 0;
		while (n >= 0 && !force_exit)
			n = libwebsocket_service(context, 50);
	}

	// Cleanup
	//TODO: Debug seg fault on from this line
	printf("Server is shutting down...\n");
	libwebsocket_context_destroy(context);
	printf("Saving beachJudge data...\n");
	saveJudgeData();
	User::Cleanup();

	return 0;
}
