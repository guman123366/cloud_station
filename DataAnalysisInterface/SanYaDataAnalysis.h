#pragma once
#include <QObject>
#include "DataAnalysisInterface.h"
#include "dataanalysisinterface_global.h"

class SanYaData;

class DATAANALYSISINTERFACE_EXPORT SanYaDataAnalysis : public DataAnalysisInterface
{
	Q_OBJECT

public:
	SanYaDataAnalysis(QObject *parent=NULL);
	~SanYaDataAnalysis();

	DataDefineInterface* AnalyseData(QByteArray ary, int nLength) override;

private:
	SanYaData* m_SanYaData;
};
