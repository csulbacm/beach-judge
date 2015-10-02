//- POSIX -
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>

#include <stdio.h>
#include <iostream>

using namespace std;

/**	@brief Socket Data Structure Class
 */
typedef struct Socket
{
	static int s_count;

	bool isBlocking;

	int socketFD;

	Socket();

	typedef enum Domain
	{
		IP4,
		IP6
	} Domain;

	typedef enum Protocol
	{
		TCP,
		UDP
	} Protocol;

	typedef enum Type
	{
		Raw,
		Stream,
		Datagram,
		SequencedPacket
	} Type;

	/**	@brief Constructor.
	 *	@param domain Domain of the Socket.
	 *	@param type Type of the Socket.
	 *	@param protocol Protocol of the Socket.
	 *	@return Socket object or 0 if creation failed.
	 */
	static Socket *Create(Domain domain, Type type, Protocol protocol, bool isBlocking = true);

	/**	@brief Destructor.
	 */
	~Socket();

	/**	@brief Returns the Socket of a newly accepted connection.
	 *	@return Socket of newly accepted connection.
	 */
	Socket *Accept();

	/**	@brief Attempts to bind the socket to a port.
	 *	@param port Target port of the binding.
	 *	@return Whether or not the binding was successful.
	 */
	bool Bind(unsigned int port);

	/**	@brief Configures the socket to listen to the port the socket is bound to.
	 *	@param backlog Maximum number of connections queued for acceptance.
	 *	@return Whether or not configuration was successful.
	 */
	bool Listen(unsigned int backlog);

	/**	@brief Attempts to connect to a location.
	 *	@param address Target address of connection.
	 *	@param port Target port of connection.
	 *	@return Whether or not the connection was successful.
	 */
	bool Connect(const char *address, unsigned int port);

	/**	@brief Attempts to shutdown the socket connection.
	 *	@return Whether or not the shutdown was successful.
	 */
	bool Shutdown();

	/**	@brief Reads data from the socket connection.
	 *	@param buffer Buffer for storing the result of the read request.
	 *	@param length Length of the read request.
	 */
	int Read(char *buffer, unsigned int length);

	/**	@brief Writes data to the socket connection.
	 *	@param buffer Buffer of the write request.
	 *	@param length Length of the write request.
	 */
	int Write(char *buffer, unsigned int length);

	bool SetBlocking(bool isBlocking);

	void GetPeerIP4Info(unsigned long *addr, unsigned short *port);

	bool Close();

	int HasRead();

	int RecvFrom(char *buffer, unsigned int buffSize, void *sockAddr, int *sockAddrLen);

	int SendTo(char *buffer, unsigned int buffSize, void *sockAddr, int sockAddrLen);


} Socket;


int Socket::s_count = 0;

Socket *Socket::Create(Domain domain, Type type, Protocol protocol, bool isBlocking)
{
	int sDomain = -1, sType = -1, sProtocol = -1;

	switch (domain) {
	case IP4:
		sDomain = PF_INET;
		break;
	case IP6:
		sDomain = PF_INET6;
		break;
	}


	switch (type) {
	case Raw:
		sType = SOCK_RAW;
		break;
	case Stream:
		sType = SOCK_STREAM;
		break;
	case Datagram:
		sType = SOCK_DGRAM;
		break;
	case SequencedPacket:
		sType = SOCK_SEQPACKET;
		break;
	}

	switch (protocol) {
	case TCP:
		sProtocol = IPPROTO_TCP;
		break;
	case UDP:
		sProtocol = IPPROTO_UDP;
	}

	if (sDomain == -1 || sType == -1 || sProtocol == -1) {
		printf("[Error] Socket::Create - Unrecognized socket configuration.\n");
		return 0;
	}

	int sSocket = socket(sDomain, sType, sProtocol);

	if (sSocket == -1) {
		close(sSocket);

		printf("[Error] Socket::Create - Failed to create Socket.\n");
	}

	int reuse = 1;
	setsockopt(sSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
	Socket *sock = new Socket();
	sock->socketFD = sSocket;
	if (!isBlocking)
		sock->SetBlocking(isBlocking);
	return sock;
}

Socket::Socket()
{
	socketFD = 0;
	isBlocking = true;
	s_count++;
}

Socket::~Socket()
{
	close(socketFD);

	s_count--;
}

Socket *Socket::Accept()
{
	int ret = accept(socketFD, NULL, NULL);

	if (ret == -1) {
		if (isBlocking)
			printf("[Error] Socket::Accept - Unable to accept incomming connection.\n");
		return 0;
	}

	Socket *client = new Socket();
	client->socketFD = ret;
	return client;
}

bool Socket::Bind(unsigned int port)
{
	struct sockaddr_in stSockAddr;
	memset(&stSockAddr, 0, sizeof(stSockAddr));
	
	stSockAddr.sin_family = AF_INET;
	stSockAddr.sin_port = htons(port);
	stSockAddr.sin_addr.s_addr = INADDR_ANY;
	if (bind(socketFD, (struct sockaddr *)&stSockAddr, sizeof(stSockAddr)) != 0) {
		printf("[Error] Socket::Bind - Unable to bind to port %d.\n", port);
		return false;
	}

	return true;
}

bool Socket::Connect(const char *address, unsigned int port)
{
	struct sockaddr_in stSockAddr;
	memset(&stSockAddr, 0, sizeof(stSockAddr));

	stSockAddr.sin_family = AF_INET;
	stSockAddr.sin_port = htons(port);
	int ret = inet_pton(AF_INET, address, (char *)&stSockAddr.sin_addr);

	if (ret < 0) {
		printf("[Error] Socket::Connect - \"%s\" is not a valid address family.\n", address);
		return false;
	} else if (ret == 0) {
		printf("[Error] Socket::Connect - \"%s:%d\" is not a valid ip address.\n", address, port);
		return false;
	}

	if (connect(socketFD, (struct sockaddr *)&stSockAddr, sizeof(stSockAddr)) == -1) {
		printf("[Error] Socket::Connect - Unable to connect to \"%s:%d\"\n", address, port);
		return false;
	}

	return true;
}

bool Socket::Listen(unsigned int backlog)
{
	if (listen(socketFD, backlog) == -1) {
		printf("[Error] Socket::Listen - Unable to listen.\n");
		return false;
	}

	return true;
}

int Socket::Read(char *buffer, unsigned int length)
{
	return read(socketFD, buffer, length);
}

bool Socket::Shutdown()
{
	SetBlocking(true);

	if (shutdown(socketFD, SHUT_RDWR) == -1) {
		printf("[Error] Socket::Listen - Unable to shutdown socket.\n");
		return false;
	}

	return true;
}

bool Socket::Close()
{
	SetBlocking(true);

	if (close(socketFD) == -1) {
		printf("[Error] Socket::Listen - Unable to close socket.\n");
		return false;
	}

	return true;
}

int Socket::Write(char *buffer, unsigned int length)
{
	return send(socketFD, buffer, length, 0);
}

int Socket::HasRead()
{
	fd_set readFlags, writeFlags;
	struct timeval waitd = {0, 0};
	FD_ZERO(&readFlags);
	FD_ZERO(&writeFlags);
	FD_SET(socketFD, &readFlags);

	int ret = select(socketFD + 1, &readFlags, &writeFlags, (fd_set *)0, &waitd);
	
	int hasRead = 0;
	hasRead = FD_ISSET(socketFD, &readFlags) ? 1 : 0;
		
	if (hasRead > 0) {
		char c = 0;
		ret = recv(socketFD, &c, 1, MSG_PEEK);

		if (ret == 0) //TODO: Verify this means client terminated connection
			hasRead = -1;
	}

	return hasRead;
}

bool Socket::SetBlocking(bool blocking)
{
	int flags = blocking ? 0 : SOCK_NONBLOCK;
	fcntl(socketFD, F_SETFL, flags);

	isBlocking = blocking;
	return true;
}

void Socket::GetPeerIP4Info(unsigned long *addr, unsigned short *port)
{
	struct sockaddr_in Addr;
	socklen_t AddrLen = sizeof(Addr);
	getpeername(socketFD, (sockaddr *)&Addr, &AddrLen);
	*addr = Addr.sin_addr.s_addr;
	*port = Addr.sin_port;
}

int Socket::RecvFrom(char *buffer, unsigned int buffSize, void *sockAddr, int *sockAddrLen)
{
	return recvfrom(socketFD, buffer, buffSize, 0, (sockaddr *)sockAddr, (socklen_t *)sockAddrLen);
}

int Socket::SendTo(char *buffer, unsigned int buffSize, void *sockAddr, int sockAddrLen)
{
	return sendto(socketFD, buffer, buffSize, 0, (sockaddr *)sockAddr, sockAddrLen);
}


int main(int argc, char *argv[])
{
	Socket *serv = Socket::Create(Socket::IP4,
		Socket::Stream, Socket::TCP, true);

	serv->Bind(28888);
	serv->Listen(16);

	while (true) {
		Socket *cl = serv->Accept();
		if (!cl)
			break;
		cout << "B" << endl;
	}

	delete serv;

	return 0;
}
