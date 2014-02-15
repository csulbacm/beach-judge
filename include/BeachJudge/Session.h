#ifndef _BEACHJUDGE_SESSION_H_
#define _BEACHJUDGE_SESSION_H_

namespace beachjudge
{
	class Session
	{
		unsigned long m_address, m_expireTimeMS;
		unsigned short m_id, m_port, m_userID;

		Session();

	public:
		static void Cleanup(bool deleteAll = false);
		static Session *Create(unsigned long address, unsigned short port, unsigned short userID);
		static Session *Lookup(unsigned long address);

		unsigned short GetID() const;
		unsigned short GetUserID() const;
		unsigned long GetExpireTimeMS() const;

		~Session();
	};
}

#endif
