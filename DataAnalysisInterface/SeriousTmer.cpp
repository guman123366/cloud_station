#include "SeriousTmer.h"

void WINAPI CALLBACK time_proc(uint, uint, DWORD_PTR user, DWORD_PTR, DWORD_PTR)
{
	SeriousTmer *t = reinterpret_cast<SeriousTmer*>(user);
	emit t->si_timeout();
}

SeriousTmer::SeriousTmer(int interval, QObject *parent)
	: QObject(parent), m_interval(interval), m_id(0)
{
}

SeriousTmer::~SeriousTmer()
{
	stopTimer();
}

void SeriousTmer::startTimer()
{
	if (m_id == 0)
	{
		m_id = timeSetEvent(m_interval, 1, time_proc, (DWORD_PTR)this, TIME_CALLBACK_FUNCTION | TIME_PERIODIC | TIME_KILL_SYNCHRONOUS);
	}
}

void SeriousTmer::stopTimer()
{
	if (m_id)
	{
		timeKillEvent(m_id);
		m_id = 0;
	}
}


