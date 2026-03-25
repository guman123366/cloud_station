#pragma once

#include <QObject>
#include <QThread>

class RTMPStreamTranslation : public QThread
{
	Q_OBJECT

public:
	RTMPStreamTranslation(QObject *parent);
	~RTMPStreamTranslation();

protected:
	void run() override;

private:
	//QString 
};
