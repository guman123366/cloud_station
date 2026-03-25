#pragma once

#include <QObject>
#include <qt_windows.h>
#include <windows.h>
#include <mmsystem.h>

class SeriousTmer : public QObject
{
	Q_OBJECT

public:
	explicit SeriousTmer(int interval,QObject *parent);
	~SeriousTmer();

	int m_id;
signals:
	void si_timeout();
public:
	void startTimer();
	void stopTimer();

	friend void WINAPI CALLBACK time_proc(uint, uint, DWORD_PTR, DWORD_PTR, DWORD_PTR);
private:
	int m_interval;
};
