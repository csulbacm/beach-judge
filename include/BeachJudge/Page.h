#ifndef _BEACHJUDGE_PAGE_H_
#define _BEACHJUDGE_PAGE_H_

//- Standard Library -
#include <string>
#include <sstream>

//- Beach Judge -
#include <BeachJudge/Session.h>
#include <BeachJudge/Socket.h>

namespace beachjudge
{
	class Page
	{
		std::string m_fileSource, m_html;

		Page();

	public:
		static Page *Create(std::string file);
		static void Cleanup();
		static void RegisterTemplate(std::string entry, void (*func)(std::stringstream &, Socket *, Session *, std::string));

		~Page();

		void AddToStream(std::stringstream &stream, Socket *client, Session *session);
	};
}

#endif
