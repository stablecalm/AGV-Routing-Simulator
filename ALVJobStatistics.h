#pragma once
#include "ALVJob.h"

using namespace std;

class CALVJobStatistics
{
private:
	vector<CALVJob> m_ALVJobs;
	int m_I;
	int m_Mq;
	int m_Wq;
	int m_Tq;
	int m_Mb;
	int m_Wb;
	int m_Tb;

	int nAANN;
	int nANNA;
	int nANAN;
	//addition for dual-load ALV
	int nAIAI;
	int nAIAN;
	int nAINA;
	int nANAI;

	long m_totalDelay;
	long m_totalDelayOfDischarge;
	long m_totalDelayOfLoad;

public:
	CALVJobStatistics(void);
	~CALVJobStatistics(void);

	void AddCompleteALVJob(CALVJob* pALVJob);
	void WriteResult();
	int	 GetSize(){ return (int)m_ALVJobs.size(); }
};
