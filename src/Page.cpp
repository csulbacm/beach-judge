//- Standard Library -
#include <iostream>
#include <fstream>
#include <map>

//- Beach Judge -
#include <BeachJudge/Base.h>
#include <BeachJudge/Page.h>

using namespace std;

namespace beachjudge
{
	map<string, Page *> g_pageMap;
	map<string, void (*)(stringstream &, Socket *, Session *)> g_templateMap;

	void Page::RegisterTemplate(string entry, void (*func)(stringstream &, Socket *, Session *))
	{
		g_templateMap[entry] = func;
	}
	Page *Page::Create(string file)
	{
		if(!fileExists(file.c_str()))
			return 0;

		if(g_pageMap.count(file))
			return g_pageMap[file];

		string html, lineIn;
		ifstream inFile(file.c_str());
		while(getline(inFile, lineIn))
		{
			html = html.append(lineIn);
			html = html.append("\r\n");
		}

		Page *page = new Page();
		page->m_fileSource = file;
		page->m_html = html;
		g_pageMap[file] = page;
		return page;
	}
	void Page::Cleanup()
	{
		while(g_pageMap.size())
			delete g_pageMap.begin()->second;
	}

	Page::Page()
	{
	}
	Page::~Page()
	{
		g_pageMap.erase(m_fileSource);
	}
	void Page::AddToStream(stringstream &stream, Socket *client, Session *session)
	{
		stringstream pageStream(m_html);
		string chunk, varChunk;
		while(getline(pageStream, chunk, '$'))
		{
			stream << chunk;

			if(pageStream.eof())
				break;

			varChunk = "";
			while(true)
			{
				char peek = pageStream.peek();
				if((peek >= 'A' && peek <= 'Z') || (peek >= 'a' && peek <= 'z'))
				{
					varChunk.push_back(peek);
					pageStream.get();
				}
				else
					break;
			}
			cout << "Smart Chunk: |" << varChunk << "|" << endl;
			if(g_templateMap.count(varChunk))
				g_templateMap[varChunk](stream, client, session);
			else
				stream << "$" << varChunk;
		}

		delete this;
	}
}
