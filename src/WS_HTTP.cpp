#include <sys/stat.h>

// beachJudge
#include <Judge/Base.h>
#include <Judge/WS_HTTP.h>

namespace judge {

//TODO: Simplify this function
int ws_http(lws *wsi,
	lws_callback_reasons reason, void *cl,
	void *in, size_t len)
{
	timeval tv;
	int n, m;
	unsigned char *p;
	char *other_headers = 0;
	unsigned char buffer[4096];
	psd_http *pss = (psd_http *)cl;
	unsigned char *end;
	switch (reason) {
	case LWS_CALLBACK_PROTOCOL_INIT: // once
	case LWS_CALLBACK_PROTOCOL_DESTROY:
	case LWS_CALLBACK_WSI_CREATE: // per connection
		break;
	case LWS_CALLBACK_WSI_DESTROY:
		pss->Purge();
		break;
	case LWS_CALLBACK_FILTER_HTTP_CONNECTION: {
		// Initialize PSS
		*pss = psd_http();
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
			//TODO: Remove Debug
			//printf("|%s|: |%s|\n", c, buf);
			if (memcmp(c, "cookie:", 7) == 0) {
				char sessID[65];
				sessID[64] = 0;
				if (sscanf(buf, "JUDGESESSID=%[0-9a-zA-Z]", sessID) == 1) {
					string sessionID = string(sessID);
					if (Session::s_sessionMap.count(sessionID) != 0) {
						pss->session = &Session::s_sessionMap[sessionID];
						pss->session->Reset();
						pss->session->SQL_Sync();
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
		pss->target = new char[len + 1];
		memcpy(pss->target, in, len);
		pss->target[len] = 0;
		pss->targetLen = len;

		if (len < 1) {
			lws_return_http_status(wsi, HTTP_STATUS_BAD_REQUEST, NULL);
			goto try_to_reuse;
		}

		bool hasSlash = false;
		if (strchr((const char *)in + 1, '/'))
			hasSlash = true;

		/* if a legal POST URL, let it continue and accept data */
		if (lws_hdr_total_length(wsi, WSI_TOKEN_POST_URI)) {
			printf("%p: POST %s\n", pss, (char *)in);

			if (pss->user != 0) {
				if (memcmp(in, "/logout", 7) == 0) {
					printf("  %p: Logout %s\n", pss, pss->user->name.c_str());
					Session::s_sessionMap.erase(string(pss->sessionID));

					//TODO: Handle invalid pw error
					char response[128];
					sprintf(response, "HTTP/1.0 200 OK\r\n"
						"Connection: close\r\n"
						"Content-Type: text/html; charset=UTF-8\r\n"
						"Content-Length: %d\r\n\r\n"
						"%s",
						2, "OK");
					lws_write(wsi,
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
			lws_return_http_status(wsi, HTTP_STATUS_NO_CONTENT, NULL);
			return -1;
		}

		// Send File
		char buf[256];
		strcpy(buf, g_resourcePath);

		//TODO: Patch login access on logged in
		//TODO: Restrict content loading to respective user groups
//		if (lws_is_ssl(wsi) == 0) {
//			strcat(buf,"/nonssl.html");
		//} else if (strstr((char *)in, ".") == NULL) {
		
		// Routing
		// TODO: Check if file, otherwise send appropriate package
		if (strstr((char *)in, ".") == NULL) {
			if (pss->user == 0)
				strcat(buf, "/login");
			else if(pss->user->level == User::Judge)
				strcat(buf, "/judge");
			else if(pss->user->level == User::Admin)
				strcat(buf, "/admin");
			else
				strcat(buf, "/user");
			strcat(buf, ".html");
		} else if (strcmp((const char *)in, "/")) {
			if (*((const char *)in) != '/')
				strcat(buf, "/");
			if (hasSlash) {
				lws_return_http_status(wsi, HTTP_STATUS_FORBIDDEN, NULL);
				goto try_to_reuse;
			}
			strncat(buf, (const char *)in, sizeof(buf) - strlen(g_resourcePath));
		}

		buf[sizeof(buf) - 1] = '\0';

		// Handle file not found
		{
			struct stat fileExist;
			if (stat(buf, &fileExist) != 0) {
				printf("%p: GET %s %d\n", pss, (char *)in, HTTP_STATUS_NOT_FOUND);
				lws_return_http_status(wsi, HTTP_STATUS_NOT_FOUND, NULL);
				return -1;
			}
		}

		/* refuse to serve files we don't understand */
		const char *mimetype = getMimeType(buf);
		if (!mimetype) {
			lwsl_err("Unknown mimetype for %s\n", buf);
			printf("%p: GET %s %d\n", pss, (char *)in, HTTP_STATUS_UNSUPPORTED_MEDIA_TYPE);
			lws_return_http_status(wsi, HTTP_STATUS_UNSUPPORTED_MEDIA_TYPE, NULL);
			return -1;
		}

		n = lws_serve_http_file(wsi, buf, mimetype, other_headers, n);

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
	case LWS_CALLBACK_HTTP_BODY: {
		// Concat body
		if (pss->bodyLen) {
			char *old = pss->body;
			pss->body = new char[pss->bodyLen + len + 1];
			memcpy(pss->body, old, pss->bodyLen);
			memcpy(pss->body + pss->bodyLen, in, len);
			pss->bodyLen += len;
			pss->body[pss->bodyLen] = 0;
			delete [] old;
		} else {
			pss->body = new char[len + 1];
			memcpy(pss->body, in, len);
			pss->body[len] = 0;
			pss->bodyLen = len;
		}
		lws_return_http_status(wsi, 100, NULL);
		return 0;
	}
	case LWS_CALLBACK_HTTP_BODY_COMPLETION: {
		printf("%p: Received data for |%s| size: %ld\n", pss, pss->target, pss->bodyLen);
		if (strcmp(pss->target, "/login") == 0) {
			char nameBuff[65];
			char pwBuff[65];
			nameBuff[64] = 0;
			pwBuff[64] = 0;
			char response[192];
			//TODO: Minimize response buffer
			{
				if (sscanf(pss->body, "username=%64[a-zA-Z0-9]&password=%64[a-zA-Z0-9]", nameBuff, pwBuff) == 2) {
					// Login Attempt
					printf("  %p: Login %s\n", pss, nameBuff);
					string name(nameBuff);
					if (User::s_byName.count(name) != 0) {
						User *user = User::s_byName[name];
						printf("U: %p\n", user);
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
							Session::s_sessionMap[sessID] =
								Session(sessID.c_str(), user);
							Session::s_sessionMap[sessID].SQL_Insert();

							// Send Key to Client
							sprintf(response, "HTTP/1.0 200 OK\r\n"
								"Connection: close\r\n"
								"Content-Type: text/html; charset=UTF-8\r\n"
								"Content-Length: %ld\r\n\r\n"
								"%s",
								strlen(sessionKey), sessionKey);
							lws_write(wsi,
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
			lws_write(wsi, (unsigned char *)response, strlen(response), LWS_WRITE_HTTP);
			goto try_to_reuse;
		}

		lws_return_http_status(wsi, HTTP_STATUS_OK, NULL);
		goto try_to_reuse;
	}
	case LWS_CALLBACK_HTTP_FILE_COMPLETION:
//		lwsl_info("LWS_CALLBACK_HTTP_FILE_COMPLETION seen\n");
		/* kill the connection after we sent one file */
		goto try_to_reuse;

	case LWS_CALLBACK_HTTP_WRITEABLE: {
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
			m = lws_write(wsi,
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
				lws_set_timeout(wsi,
					PENDING_TIMEOUT_HTTP_CONTENT, 5);
			
			/* if we have indigestion, let him clear it before eating more */
			if (lws_partial_buffered(wsi))
				break;

		} while (!lws_send_pipe_choked(wsi));

later:
		lws_callback_on_writable(wsi);
		break;
flush_bail:
		if (lws_partial_buffered(wsi)) {
			lws_callback_on_writable(wsi);
			break;
		}
		close(pss->fd);
		goto try_to_reuse;

bail:
		close(pss->fd);
		return -1;
	}
	default:
		break;
	}

	return 0;
	
try_to_reuse:
	if (lws_http_transaction_completed(wsi))
		return -1;
	return 0;
}


}
