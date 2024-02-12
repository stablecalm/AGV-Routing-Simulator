#include "StdAfx.h"
#include "ALVJobStatistics.h"

CALVJobStatistics::CALVJobStatistics(void)
{
	nAANN = 0;
	nANNA = 0;
	nANAN = 0;
	nAIAI = 0;
	nAIAN = 0;
	nAINA = 0;
	nANAI = 0;
	m_totalDelay = 0;
	m_totalDelayOfDischarge = 0;
	m_totalDelayOfLoad = 0;
}

CALVJobStatistics::~CALVJobStatistics(void)
{	
	m_ALVJobs.clear();
}

void CALVJobStatistics::AddCompleteALVJob(CALVJob* pALVJob )
{
	CALVJob completeALVJob(pALVJob);
	switch( completeALVJob.pattern )
	{
	case JP_AANN: ++nAANN; break;
	case JP_ANNA: ++nANNA; break;
	case JP_ANAN: ++nANAN; break;
	case JP_AIAI: ++nAIAI; break;
	case JP_AIAN: ++nAIAN; break;
	case JP_AINA: ++nAINA; break;
	case JP_ANAI: ++nANAI; break;
	}
	m_totalDelay += completeALVJob.delayTime;
	if( completeALVJob.prsType == PrsType_Discharging ){
		m_totalDelayOfDischarge += completeALVJob.delayTime;
	}
	else if( completeALVJob.prsType == PrsType_Loading ){
		m_totalDelayOfLoad += completeALVJob.delayTime;
	}
	m_ALVJobs.push_back(completeALVJob);
	return;
}

void CALVJobStatistics::WriteResult()
{
	ofstream ofs;
	CString resultFileName;

	//수행한 Dual-load Type 결과 기록
	resultFileName.Format("RESULT_USING_PATTERN.txt");
	ofs.open(resultFileName, ios_base::app);
	if(ofs.fail()){
		CString msg;
		msg.Format(resultFileName+" 결과 파일 읽기 실패");
		AfxMessageBox(_T(msg),MB_OK,0);
		return;
	}
	ofs.seekp(ios_base::end);
	ofs << "AANN:" << " " << nAANN << " ";
	ofs << "ANNA:" << " " << nANNA << " ";
	ofs << "ANAN:" << " " << nANAN << " ";
	ofs << "AIAI:" << " " << nAIAI << " ";
	ofs << "AIAN:" << " " << nAIAN << " ";
	ofs << "AINA:" << " " << nAINA << " ";
	ofs << "ANAI:" << " " << nANAI << " ";
	ofs << endl;
	
	ofs.close();
	ofs.clear();

	//ALVJob Log 기록
	resultFileName.Format("RESULT_LOG_OF_ALVJob.txt");
	ofs.open(resultFileName, ios_base::app);
	if(ofs.fail()){
		CString msg;
		msg.Format(resultFileName+" 결과 파일 읽기 실패");
		AfxMessageBox(_T(msg),MB_OK,0);
		return;
	}
	ofs.seekp(ios_base::end);	

	vector<CALVJob>::iterator index;
	for( index = m_ALVJobs.begin(); index != m_ALVJobs.end(); ++index ){
		ofs << "ALV_" << g_SimulationSpec.nVehicle << " ";
		ofs << index->jobID << " ";
		ofs << (index->finishTime - index->startTime) << " ";

		if( index->prsType == PrsType_Discharging ){
			ofs << index->QuayHPID << " ";
			ofs << index->YardHPID << " ";
			ofs << (index->travelToQuaysideEndTime - index->travelToQuaysideBeginTime) << " ";
			ofs << (index->waitQCPermissionEndTime - index->waitQCPermissionBeginTime) << " ";
			ofs << (index->travelToQCEndTime - index->travelToQCBeginTime) << " ";
			ofs << (index->transferAtQCEndTime - index->transferAtQCBeginTime) << " ";
			ofs << (index->travelToYardsideEndTime - index->travelToYardsideBeginTime) << " ";
			ofs << (index->waitBlockPermissionEndTime - index->waitBlockPermissionBeginTime) << " ";
			ofs << (index->travelToBlockEndTime - index->travelToBlockBeginTime) << " ";
			ofs << (index->transferAtBlockEndTime - index->transferAtBlockBeginTime) << " ";
		}
		else if( index->prsType == PrsType_Loading ){
			ofs << index->YardHPID << " ";
			ofs << index->QuayHPID << " ";
			ofs << (index->travelToYardsideEndTime - index->travelToYardsideBeginTime) << " ";
			ofs << (index->waitBlockPermissionEndTime - index->waitBlockPermissionBeginTime) << " ";
			ofs << (index->travelToBlockEndTime - index->travelToBlockBeginTime) << " ";
			ofs << (index->transferAtBlockEndTime - index->transferAtBlockBeginTime) << " ";
			ofs << (index->travelToQuaysideEndTime - index->travelToQuaysideBeginTime) << " ";
			ofs << (index->waitQCPermissionEndTime - index->waitQCPermissionBeginTime) << " ";
			ofs << (index->travelToQCEndTime - index->travelToQCBeginTime) << " ";
			ofs << (index->transferAtQCEndTime - index->transferAtQCBeginTime) << " ";
		}
		ofs << index->delayTime << " ";
		ofs	<< endl;
	}		
	
	ofs.close();
	ofs.clear();
	
	//ALVJob 수행 시 평균
	index = m_ALVJobs.begin();

	long TotalElapsedTime		= index->finishTime - index->startTime;
	long TotalTravelToQuayside	 = index->travelToQuaysideEndTime - index->travelToQuaysideBeginTime;
	long TotalwWaitQCPermission  = index->waitQCPermissionEndTime - index->waitQCPermissionBeginTime;
	long TotalTravelToQC		= index->travelToQCEndTime - index->travelToQCBeginTime;
	long TotalTransferAtQC		= index->transferAtQCEndTime - index->transferAtQCBeginTime;
	long TotalTravelToYardside	= index->travelToYardsideEndTime - index->travelToYardsideBeginTime;
	long TotalWaitBlockPermisson= index->waitBlockPermissionEndTime - index->waitBlockPermissionBeginTime;
	long TotalTravelToBlock		= index->travelToBlockEndTime - index->travelToBlockBeginTime;
	long TotalTransferAtBlock	= index->transferAtBlockEndTime - index->transferAtBlockBeginTime;
	
	++index;
	for( ; index != m_ALVJobs.end(); ++index){
		TotalElapsedTime		+= index->finishTime - index->startTime;
		TotalTravelToQuayside	 += index->travelToQuaysideEndTime - index->travelToQuaysideBeginTime;
		TotalwWaitQCPermission  += index->waitQCPermissionEndTime - index->waitQCPermissionBeginTime;
		TotalTravelToQC  += index->travelToQCEndTime - index->travelToQCBeginTime;
		TotalTransferAtQC  += index->transferAtQCEndTime - index->transferAtQCBeginTime;
		TotalTravelToYardside	+= index->travelToYardsideEndTime - index->travelToYardsideBeginTime;
		TotalWaitBlockPermisson  += index->waitBlockPermissionEndTime - index->waitBlockPermissionBeginTime;
		TotalTravelToBlock  += index->travelToBlockEndTime - index->travelToBlockBeginTime;
		TotalTransferAtBlock += index->transferAtBlockEndTime - index->transferAtBlockBeginTime;
	}
	
	long processSize = (long)m_ALVJobs.size();
	double AvgTotalElapsedTime		= (double)TotalElapsedTime/processSize;
	double AvgTravelToQuayside		= (double)TotalTravelToQuayside/processSize;
	double AvgwWaitQCPermission		= (double)TotalwWaitQCPermission/processSize;
	double AvgTravelToQC			= (double)TotalTravelToQC/processSize;
	double AvgTransferAtQC			= (double)TotalTransferAtQC/processSize;
	double AvgTravelToYardside		= (double)TotalTravelToYardside/processSize;
	double AvgWaitBlockPermisson	= (double)TotalWaitBlockPermisson/processSize;
	double AvgTravelToBlock			= (double)TotalTravelToBlock/processSize;
	double AvgTransferAtBlock		= (double)TotalTransferAtBlock/processSize;

	//Job 지연의 평균과 분산. 총작업평균/분산 양하작업평균/분산 적하작업평균/분산
	double totalMean = m_totalDelay/processSize;
	double dischargeMean = m_totalDelayOfDischarge/processSize;
	double loadMean = m_totalDelayOfLoad/processSize;

	double totalVar = 0;
	double dischargeVar = 0;
	double loadVar = 0;
	for( index = m_ALVJobs.begin(); index != m_ALVJobs.end(); ++index){
		totalVar += ((double)index->delayTime - totalMean)*((double)index->delayTime - totalMean);
		if( index->prsType == PrsType_Discharging ){
			dischargeVar += ((double)index->delayTime - dischargeMean)*((double)index->delayTime - dischargeMean);
		}
		else if( index->prsType == PrsType_Loading ){
			loadVar += ((double)index->delayTime - loadMean)*((double)index->delayTime - loadMean);
		}
	}
	totalVar /= (double)processSize;
	dischargeVar /= (double)processSize;
	loadVar /= (double)processSize;

	resultFileName.Format("RESULT_AVG_OF_ALVJOB.txt");
	ofs.open(resultFileName, ios_base::app);
	if(ofs.fail()){
		CString msg;
		msg.Format(resultFileName+" 결과 파일 읽기 실패");
		AfxMessageBox(_T(msg),MB_OK,0);
		return;
	}

	//결과 파일 
	ofs.seekp(ios_base::end);
	ofs << "ALV_" << g_SimulationSpec.nVehicle << " ";
	ofs << "Avg ";
	ofs << (int)AvgTotalElapsedTime << " ";
	ofs << (int)AvgTravelToQuayside << " ";
	ofs << (int)AvgwWaitQCPermission << " ";
	ofs << (int)AvgTransferAtQC << " ";
	ofs << (int)AvgTravelToYardside << " ";
	ofs << (int)AvgWaitBlockPermisson << " ";
	ofs << (int)AvgTravelToBlock << " ";
	ofs << (int)AvgTransferAtBlock << " ";
	ofs << "TOTAL " << totalMean << " " << totalVar << " ";
	ofs << "D " << dischargeMean << " " << dischargeVar << " ";
	ofs << "L " << loadMean << " " << loadVar << " ";
	ofs	<< endl;
	ofs.close();
	ofs.clear();

	//통계정보 처리가 끝난 프로세스를 삭제한다.
	m_ALVJobs.clear();

	return;
}

