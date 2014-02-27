#ifndef _BEACHJUDGE_COMPETITION_H_
#define _BEACHJUDGE_COMPETITION_H_

//- Standard Library -
#include <string>

namespace beachjudge
{
	class Competition
	{
		unsigned long m_startTimeS, m_endTimeS, m_duration;
		bool m_isRunning;

		Competition();

	public:
		static Competition *GetCurrent();
		static Competition *CreateFromFile(std::string file);
		static Competition *Create(unsigned long duration);

		~Competition();

		void Start();
		void Stop();
		bool IsRunning() const;
		unsigned long GetTimeLeft() const;
		unsigned long GetDuration() const;
		void SetCurrent();
		void SaveToFile(std::string file);
	};
}

#endif
