#ifndef _BEACHJUDGE_SOCKET_H_
#define _BEACHJUDGE_SOCKET_H_

namespace beachjudge
{
	class Socket
	{
		static int ms_count;

		bool m_isBlocking;
		int m_socket;

		Socket();

	public:
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

		static Socket *Create(Domain domain, Type type, Protocol protocol, bool isBlocking = true);
		~Socket();

		Socket *Accept();
		bool Bind(unsigned int port);
		bool Listen(unsigned int backlog);
		bool Connect(const char *address, unsigned int port);
		bool Shutdown();
		int Read(char *buffer, unsigned int length);
		int Write(char *buffer, unsigned int length);
		bool SetBlocking(bool isBlocking);
		void GetPeerIP4Info(unsigned long *addr, unsigned short *port);
	};
}

#endif
