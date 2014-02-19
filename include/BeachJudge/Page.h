#ifndef _BEACHJUDGE_PAGE_H_
#define _BEACHJUDGE_PAGE_H_

//- Standard Library -
#include <string>
#include <sstream>
#include <map>

//- Beach Judge -
#include <BeachJudge/Session.h>
#include <BeachJudge/Socket.h>

namespace beachjudge
{
	class Page
	{
		std::string m_fileSource, m_html;
		bool m_isTemp;

		Page();

	public:
		static Page *Create(std::string file);
		static Page *CreateFromHTML(std::string html);
		static void Cleanup();
		static void RegisterTemplate(std::string entry, void (*func)(std::stringstream &, Socket *, Session *, std::string, std::map<std::string, std::string> *));
		static void RegisterDefaultTemplates();

		~Page();

		void AddToStream(std::stringstream &stream, Socket *client, Session *session, std::map<std::string, std::string> *GETMap, std::map<std::string, std::string> *masterLocalVars = 0);
	};
}

#endif
