#pragma once

#include "DataAnalysisInterface.h"
#include "dataanalysisinterface_global.h"

class DataDefineInterface;

class DATAANALYSISINTERFACE_EXPORT IntelligentASAdminAnalysis : public DataAnalysisInterface
{
	Q_OBJECT

public:
	IntelligentASAdminAnalysis(QObject *parent=NULL);
	~IntelligentASAdminAnalysis();

	DataDefineInterface* AnalyseData(QByteArray ary, int nLength) override;
};
