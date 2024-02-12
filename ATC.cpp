#include "StdAfx.h"
#include ".\atc.h"

CATC::CATC(void)
{
}

CATC::CATC( int id, CPoint& initialPos )
:m_ID(id), m_ptLocation(initialPos), m_iThroughput(0), m_idleTime(0), m_clockTick(0),
m_travelTimer(0), m_CWS(CWS_Idle), m_pProcess(NULL)
{
	ostringstream sout;
	sout << "YD00B0" << id;
	m_StringID = sout.str();
	sout.str("");
	sout.clear();

	//ATC에 종속되어있는 TP의 위치를 결정한다.
	m_HPs.clear();

	m_ptTrolley = initialPos;
	m_ptSpreader = initialPos;
}

CATC::~CATC(void)
{
	m_HPs.clear();				//TP정보를 삭제한다.
	m_HPIDList.clear();
	m_DischargingSequence.clear();
}

//drawing operation--------------------------------------------------------------------------------------------
void CATC::DrawHPs( CDC& dc, double dZoom)
{
	//ATC의 TP를 그린다.
	int index;
	for( index=0; index < (int)m_HPs.size(); ++index){
		m_HPs[index].DrawTP(dc, dZoom);
	}
	return;
}
void CATC::DrawCrane(CDC& dc, double dZoom)
{
	CPen pen, *pOldPen;
	CBrush brush_container, *pOldBrush;

	//Block을 그린다.
	if( pen.CreatePen( PS_SOLID, 2, RGB( 0, 100, 100 ) ) ){
		pOldPen = dc.SelectObject( &pen );
		CPoint ptBlock = m_ptLocation;
		dc.MoveTo( (int)(ptBlock.x * dZoom), (int)((ptBlock.y + g_CraneSpec.blockLength)*dZoom) );
		dc.LineTo( (int)(ptBlock.x * dZoom), (int)(ptBlock.y*dZoom) );
		dc.LineTo( (int)((ptBlock.x + g_CraneSpec.blockWidth)*dZoom), (int)(ptBlock.y*dZoom) );
		dc.LineTo( (int)((ptBlock.x + g_CraneSpec.blockWidth)*dZoom), (int)((ptBlock.y + g_CraneSpec.blockLength)*dZoom) );
		dc.SelectObject(pOldPen);
		pen.DeleteObject();
	}

	//Block의 ID를 표시한다.
	CString strBlockID;
	strBlockID.Format(_T("Block %d"), m_ID);
	dc.TextOut((int)((m_ptLocation.x+400)*dZoom), (int)((m_ptLocation.y+g_CraneSpec.blockLength-100)*dZoom), strBlockID);

	//QC가 현재 처리 중인 Process ID
	if( m_pProcess != NULL ){
		strBlockID.Format( _T("Prs ID: %s"), m_pProcess->prsID.c_str() );
		dc.TextOut((int)((m_ptLocation.x+400)*dZoom), 20 + (int)((m_ptLocation.y+g_CraneSpec.blockLength-100)*dZoom), strBlockID);
	}
	else{
		strBlockID.Format( _T("Prs ID: NULL") );
		dc.TextOut((int)((m_ptLocation.x+400)*dZoom), 20 + (int)((m_ptLocation.y+g_CraneSpec.blockLength-100)*dZoom), strBlockID);
	}

	//ATC를 그린다.
	if( m_CWS == CWS_LoadedTravel || m_CWS == CWS_WaitingforDropoff || m_CWS == CWS_Dropoff || m_CWS == CWS_Pickup ){
		//스프레더를 그린다.
		if( pen.CreatePen( PS_SOLID, 1, COLOR_BLACK ) && brush_container.CreateSolidBrush( COLOR_CONTAINER_HIGH ) ){
			pOldPen = dc.SelectObject( &pen );
			pOldBrush = dc.SelectObject( &brush_container);

			CPoint ptATC = m_ptTrolley;
			//스프레더 부분을 그린다.
			CPoint buffer_area[4];
			CPoint ptTP = m_ptSpreader;
			buffer_area[0].x = (int)( (ptTP.x - g_CraneSpec.Container40ftHalfWidth) * dZoom);
			buffer_area[0].y = (int)( (ptATC.y + g_CraneSpec.Container40ftHalfLength) * dZoom);
			buffer_area[1].x = (int)( (ptTP.x + g_CraneSpec.Container40ftHalfWidth) * dZoom);
			buffer_area[1].y = (int)( (ptATC.y + g_CraneSpec.Container40ftHalfLength) * dZoom);
			buffer_area[2].x = (int)( (ptTP.x + g_CraneSpec.Container40ftHalfWidth) * dZoom);
			buffer_area[2].y = (int)( (ptATC.y - g_CraneSpec.Container40ftHalfLength) * dZoom);
			buffer_area[3].x = (int)( (ptTP.x - g_CraneSpec.Container40ftHalfWidth) * dZoom);
			buffer_area[3].y = (int)( (ptATC.y - g_CraneSpec.Container40ftHalfLength) * dZoom);			
			
			//dc.Polygon(buffer_area, 4);

			dc.MoveTo(buffer_area[3]);
			int i;
			for(i=0; i<4; ++i){
				dc.LineTo(buffer_area[i]);
			}			

			dc.SelectObject(pOldPen);
			pen.DeleteObject();
			dc.SelectObject(pOldBrush);
			brush_container.DeleteObject();
		}
	}
	
	//Crane을 그린다.
	if( pen.CreatePen( PS_SOLID, 2, COLOR_ATC )){
		pOldPen = dc.SelectObject( &pen );
		CPoint ptATC = m_ptTrolley;
		//크레인의 중심선을 그린다.
		ptATC.x -= 150;
		dc.MoveTo( (int)(ptATC.x * dZoom), (int)(ptATC.y*dZoom) );
		dc.LineTo( (int)((ptATC.x+g_CraneSpec.blockWidth+300) * dZoom), (int)(ptATC.y*dZoom) );
		//크레인의 양다리를 그린다.
		dc.MoveTo( (int)(ptATC.x * dZoom), (int)((ptATC.y+g_CraneSpec.Container40ftLength)*dZoom) );
		dc.LineTo( (int)(ptATC.x * dZoom), (int)((ptATC.y-g_CraneSpec.Container40ftLength)*dZoom) );
		dc.MoveTo( (int)((ptATC.x+g_CraneSpec.blockWidth+300) * dZoom), (int)((ptATC.y+g_CraneSpec.Container40ftLength)*dZoom) );
		dc.LineTo( (int)((ptATC.x+g_CraneSpec.blockWidth+300) * dZoom), (int)((ptATC.y-g_CraneSpec.Container40ftLength)*dZoom) );
		dc.SelectObject(pOldPen);
		pen.DeleteObject();
	}
	
}


//public operations---------------------------------------------------------------------------
void CATC::SetHP( CHP hp )
{ 
	m_HPs.push_back(hp); 
	m_HPIDList.insert( make_pair( hp.GetStringID(), hp.GetID())); 
	return;
}

int CATC::GetPairTP( int tp )
{
	int pairTP = tp;	//Block 앞에 Row가 하나 밖에 없는 경우, PAIR_TP는 자기 자신을 가리킨다.
	if( g_CraneSpec.nRowperBlock == 2 ){
		if( tp < g_CraneSpec.nHPperBlock ){
			pairTP = tp + g_CraneSpec.nHPperBlock;
		}
		else{
			pairTP = tp - g_CraneSpec.nHPperBlock;
		}
	}
	return pairTP;
}

int CATC::EstimateTimeofSingleCycle()
{
	int prob = rand()%1000;
	int travelTime = 0;
	//측정한 cycle-time에서 차량과 hoisting 작업에 걸리는 20sec을 뺀 분포.
	if( m_pProcess->prsType == PrsType_Discharging ){
		if( 0 <= prob && prob < 3 )		   {	travelTime = 40  + rand()%20;}		
		else if( 3   <= prob && prob < 27 ){	travelTime = 60  + rand()%20;}	
		else if( 27  <= prob && prob < 82 ){	travelTime = 80  + rand()%20;}	
		else if( 82  <= prob && prob < 167){	travelTime = 100 + rand()%20;}	
		else if( 167 <= prob && prob < 306){	travelTime = 120 + rand()%20;}	
		else if( 306 <= prob && prob < 475){	travelTime = 140 + rand()%20;}	
		else if( 475 <= prob && prob < 672){	travelTime = 160 + rand()%20;}	
		else if( 672 <= prob && prob < 812){	travelTime = 180 + rand()%20;}	
		else if( 812 <= prob && prob < 908){	travelTime = 200 + rand()%20;}	
		else if( 908 <= prob && prob < 967){	travelTime = 220 + rand()%20;}	
		else if( 967 <= prob && prob < 987){	travelTime = 240 + rand()%20;}	
		else if( 987 <= prob && prob < 998){	travelTime = 260 + rand()%20;}	
		else if( 998 <= prob && prob <1000){	travelTime = 280 + rand()%20;}	
	}
	else if( m_pProcess->prsType == PrsType_Loading ){
		if( 0 <= prob && prob < 21 )	   {	travelTime = 40  + rand()%20;}		
		else if( 21  <= prob && prob < 96 ){	travelTime = 60  + rand()%20;}	
		else if( 96  <= prob && prob < 186){	travelTime = 80  + rand()%20;}	
		else if( 186 <= prob && prob < 306){	travelTime = 100 + rand()%20;}	
		else if( 306 <= prob && prob < 442){	travelTime = 120 + rand()%20;}	
		else if( 442 <= prob && prob < 505){	travelTime = 140 + rand()%20;}	
		else if( 505 <= prob && prob < 610){	travelTime = 160 + rand()%20;}	
		else if( 610 <= prob && prob < 718){	travelTime = 180 + rand()%20;}	
		else if( 718 <= prob && prob < 819){	travelTime = 200 + rand()%20;}	
		else if( 819 <= prob && prob < 857){	travelTime = 220 + rand()%20;}	
		else if( 857 <= prob && prob < 889){	travelTime = 240 + rand()%20;}	
		else if( 889 <= prob && prob < 923){	travelTime = 260 + rand()%20;}	
		else if( 923 <= prob && prob < 942){	travelTime = 280 + rand()%20;}	
		else if( 942 <= prob && prob < 986){	travelTime = 300 + rand()%80;}	
		else if( 986 <= prob && prob < 994){	travelTime = 380 + rand()%80;}	
		else if( 994 <= prob && prob <1000){	travelTime = 460 + rand()%80;}	
	}		
	return travelTime;
}

int CATC::GetReservableHPforPassPoint()
{
	int hpID;
	for( hpID = 0; hpID < g_CraneSpec.nHPperBlock; ++hpID ){
		if( m_HPs[hpID].CheckReservation() == false || m_HPs[GetPairTP(hpID)].CheckReservation() == false ){//둘 중 한 TP가 비어있을 때
			if( m_HPs[hpID].CheckReservation() == false ){	//크레인측 TP를 우선적으로 반환한다.
				return hpID;
			}
			else{
				return GetPairTP(hpID);
			}
		}
	}
	return -1;	//예약 가능한 TP가 하나도 없음.
}

int	CATC::GetReservableHPfor( CContainer container )
{
	int needCapacity;
	if( container.conType == ConType_General40ft ){
		needCapacity = 2;
	}
	else if( container.conType == ConType_General20ft ){
		needCapacity = 1;
	}
	int hpID;
	for( hpID = 0; hpID < g_CraneSpec.nHPperBlock; ++hpID ){
		if( m_HPs[hpID].CheckReservation() == false && m_HPs[GetPairTP(hpID)].CheckReservation() == false ){//예약 가능한 것
			if( m_HPs[hpID].GetRemainCapacity() >= needCapacity ){	//크레인측 TP를 우선적으로 반환한다.
				return hpID;
			}
			else if( m_HPs[GetPairTP(hpID)].GetRemainCapacity() >= needCapacity ){
				return GetPairTP(hpID);
			}
		}		
	}
	return -1;	//예약 가능하고 컨테이너가 놓여있지 않은 TP가 하나도 없음.
}

int	CATC::GetReservableHPfor( int needCapacity )
{
	if( needCapacity < 0 || needCapacity > 2 ){
		Assert(false, "Invalid Capacity");
	}
	int hpID;
	for( hpID = 0; hpID < g_CraneSpec.nHPperBlock * g_CraneSpec.nRowperBlock; ++hpID ){
		if( m_HPs[hpID].CheckReservation() == false && m_HPs[GetPairTP(hpID)].CheckReservation() == false ){//예약 가능한 것
			int remainCapacity = m_HPs[hpID].GetRemainCapacity();
			if( remainCapacity >= needCapacity ){	//크레인측 TP를 우선적으로 반환한다.
				return hpID;
			}
		}		
	}
	return -1;	//예약 가능하고 컨테이너가 놓여있지 않은 TP가 하나도 없음.
}

void CATC::SetDischargingSequence( vector<CProcess*> &prsSequence )
{
	m_DischargingSequence = prsSequence;
	return;
}

void CATC::SetLoadingSequence( vector<CProcess*> &prsSequence )
{
	CProcess* prs;
	vector<CProcess*>::iterator index;
	for( index = prsSequence.begin(); index != prsSequence.end(); ++index ){
		prs = (*index);
		m_LoadingSeqPerQC[prs->iQCID].push_back(prs);
	}

	return;
}

int CATC::DecideHPforLoadingContainer(CProcess *pProcess)
{
	int workingTP = -1;

	int needCapacity;
	if( pProcess->ALVJob->container.conType == ConType_General20ft ){
		needCapacity = 1;
	}
	else if( pProcess->ALVJob->container.conType == ConType_General40ft ){
		needCapacity = 2;
	}

	int tp;
	int maxTP = g_CraneSpec.nRowperBlock * g_CraneSpec.nHPperBlock;
	if( needCapacity == 1 ){//20ft 컨테이너를 놓는 경우, 먼저 20ft 컨테이너가 놓여있는 것부터 선택해서 HP를 꽉 채운다.
		for( tp = 0; tp < maxTP; ++tp ){
			if( m_HPs[tp].GetRemainCapacity() == 1 ){//20ft만 놓여있는 HP 중
				if( m_HPs[tp].CheckReservation() == false && m_HPs[GetPairTP(tp)].CheckReservation() == false ){
					workingTP = tp;
					break;
				}
			}
		}
		if( workingTP == -1 ){	//위 과정에서 사용 가능한 HP를 찾지 못했다면
			for( tp = 0; tp < maxTP; ++tp ){
				if( m_HPs[tp].GetRemainCapacity() == 2 ){//완전히 비어있는 HP 중
					if( m_HPs[tp].CheckReservation() == false && m_HPs[GetPairTP(tp)].CheckReservation() == false ){
						workingTP = tp;
						break;
					}
				}
			}
		}
	}
	else if( needCapacity == 2 ){	//40ft 컨테이너를 놓는 경우
		for( tp = 0; tp < maxTP; ++tp ){
			if( m_HPs[tp].GetRemainCapacity() == 2 ){//완전히 비어있는 HP 중
				if( m_HPs[tp].CheckReservation() == false && m_HPs[GetPairTP(tp)].CheckReservation() == false ){
					workingTP = tp;
					break;
				}
			}
		}
	}
	return workingTP;
}

//public operations---------------------------------------------------------------------------
void CATC::DoSingleIteration()
{
	++m_clockTick;
	if( m_pProcess == NULL ){
		DecideCurrentProcess();	//수행할 작업을 결정한다.
	}
	if( m_pProcess != NULL ){
		if(m_pProcess->prsType == PrsType_Loading ){
			DoLoad();		
		}
		else if(m_pProcess->prsType == PrsType_Discharging ){
			DoDischarge();
		}
	}	
	return;	
}

void CATC::DoLoad()
{
	//적하 작업 - ProcessSequence 순서를 지키면서 
	switch( m_CWS)
	{
	case CWS_Idle:	
		{
			m_ptTrolley.y = m_ptLocation.y;
			m_ptSpreader.x = m_HPs[m_workingHP].GetPtCenter().x;
			
			//확률 분포를 참조하여 ATC Cycle Time 생성
			m_travelTime = (int)(EstimateTimeofSingleCycle()/g_CraneSpec.unitTime);	//Time을 Clock으로 변환

			//이동 소요 시간 설정
			m_travelTimer = (int)m_travelTime/2;
			m_CWS = CWS_EmptyTravel;			
		}		
		break;
	case CWS_EmptyTravel:
		{
			--m_travelTimer;
			m_ptTrolley.y += g_CraneSpec.ASCTravelInterval;

			if( m_travelTimer <= 0 ){
				//이동 소요 시간 설정
				m_travelTimer = (int)m_travelTime/2;
				m_CWS = CWS_LoadedTravel;
			}
		}
		break;
	case CWS_LoadedTravel:
		{
			--m_travelTimer;
			m_ptTrolley.y -= g_CraneSpec.ASCTravelInterval;

			if( m_travelTimer <= 0 ){
				m_CWS = CWS_WaitingforDropoff;			//컨테이너 적하 대기
			}
		}
		break;
	case CWS_WaitingforDropoff:
		{
			//적하 HP를 작업 시작과 동시에 예약 (데드락 방지)
			//프로세스 정보 업데이트
			string hpID = m_HPs[m_workingHP].GetStringID();
			m_pProcess->yardHP = hpID;
			m_pProcess->ALVJob->YardHPID = hpID;

			//트롤리 위치 지정
			m_ptTrolley.y = m_HPs[m_workingHP].GetPtCenter().y;
			m_hoistingTimer = (int)(g_CraneSpec.ASCUnloadingTime/g_CraneSpec.unitTime);	//Time을 Clock으로 변환
			m_CWS = CWS_Dropoff;		
		}
		break;
	case CWS_Dropoff:
		{
			--m_hoistingTimer;

			if( m_hoistingTimer <= 0 ){
				//TP의 상태를 변경한다.
				m_HPs[m_workingHP].SetContainer(m_pProcess->ALVJob->container);
				m_HPs[m_workingHP].ReleaseReservation();

				//적하 작업 완료
				m_LoadingSeqPerQC[m_pProcess->iQCID].pop_front();	
				
				m_pProcess = NULL;
				++m_iThroughput;

				m_CWS = CWS_Idle;
			}
		}
		break;

	default:
		break;
	}
}

void CATC::DoDischarge()
{
	//양하 작업 - AGV의 경우, 도착한 순서대로 처리
	switch( m_CWS)
	{
	case CWS_Idle:	
		{
			m_ptTrolley.y = m_ptLocation.y;
			m_ptSpreader.x = m_HPs[m_workingHP].GetPtCenter().x;
			m_CWS = CWS_WaitingforPickup;
		}
		break;
	case CWS_WaitingforPickup:
		{	
			m_ptTrolley.y = m_HPs[m_workingHP].GetPtCenter().y;
			m_hoistingTimer = (int)(g_CraneSpec.ASCLoadingTime/g_CraneSpec.unitTime);	//Time을 Clock으로 변환
			m_CWS = CWS_Pickup;
		}
		break;
	case CWS_Pickup:
		{			
			--m_hoistingTimer;
			if( m_hoistingTimer <= 0 ){
				m_HPs[m_workingHP].ReleaseContainer(m_pProcess->ALVJob->container);
				m_HPs[m_workingHP].ReleaseReservation();

				//작업소요시간 설정
				m_travelTime = (int)(EstimateTimeofSingleCycle()/g_CraneSpec.unitTime);	//Time을 Clock으로 변환
				m_travelTimer = (int)m_travelTime/2;
				m_CWS = CWS_LoadedTravel;
			}
		}
		break;
	case CWS_LoadedTravel:
		{
			--m_travelTimer;
			m_ptTrolley.y += g_CraneSpec.ASCTravelInterval;
			if( m_travelTimer <= 0 ){
				m_travelTimer = (int)m_travelTime/2;				
				m_CWS = CWS_EmptyTravel;				
			}
		}
		break;	
	case CWS_EmptyTravel:
		{
			--m_travelTimer;
			m_ptTrolley.y -= g_CraneSpec.ASCTravelInterval;
			if( m_travelTimer <= 0 ){
				++m_iThroughput;

				//양하 작업 완료를 PM에게 알림
				m_pEquipmentManager->ProcessFinished(m_pProcess);
				m_DischargingSequence.erase(m_indexDischarge);
				m_pProcess = NULL;
				m_CWS = CWS_Idle;
			}
		}
		break;
	default:
		break;
	}
}

void CATC::DecideCurrentProcess()
{
	if( g_SimulationSpec.cargoType == CargoType_Discharge ){//양하 작업만 처리하는 경우
		m_pProcess = SelectDischargingProcess();
	}
	else if( g_SimulationSpec.cargoType == CargoType_Load ){//적하 작업만 처리하는 경우
		m_pProcess = SelectLoadingProcess();
	}
	else if( g_SimulationSpec.cargoType == CargoType_Mixed || g_SimulationSpec.cargoType == CargoType_DoubleCycle ){//양적하 작업이 섞여있는 경우
		bool noLoad = true;
		int qc;
		for( qc = 0; qc < g_SimulationSpec.nQC; ++qc ){
			if( m_LoadingSeqPerQC[qc].empty() == false ){
				noLoad = false;
				break;
			}
		}

		if( noLoad ){
			m_pProcess = SelectDischargingProcess();
		}
		else if( m_DischargingSequence.empty() ){
			m_pProcess = SelectLoadingProcess();
		}
		else{
			//양하 작업과 적하 작업이 섞여있는 경우 또는 적하 작업만 있는 경우
			//1. HP의 사용률이 75% 이상인지 확인. HP 8개 기준, 12 TEU가 놓여있으면 사용률이 75%
			int nUsed = 0;
			int hp;
			for( hp = 0; hp < (g_CraneSpec.nHPperBlock * g_CraneSpec.nRowperBlock); ++hp ){
				nUsed += m_HPs[hp].GetCurrentCapacity();
			}
			if( nUsed >= 9 ){
				m_pProcess = SelectDischargingProcess();
			}
			else{
				m_pProcess = SelectLoadingProcess();
			}
		}
		
	}

	//해당 작업의 원활한 수행을 위하여 HP를 미리 예약한다.
	if(m_pProcess != NULL){
		if( m_pProcess->prsType == PrsType_Discharging ){
			//HP 결정
			int workingHP = -1;
			if( m_pProcess->ALVJob->YardHPID != "" ){//양하 컨테이너의 HP가 정해져있지 않다면 PASS
				string hpID = m_pProcess->ALVJob->YardHPID;
				if( m_HPs[m_HPIDList[hpID]].CheckReservation() == false && m_HPs[GetPairTP(m_HPIDList[hpID])].CheckReservation() == false ){
					//방해되는 AHV가 없을 때
					workingHP = m_HPIDList[hpID];
				}
			}
			if( workingHP != -1){
				m_workingHP = workingHP;
				m_HPs[m_workingHP].SetReservation();
			}
			else{//HP예약을 못했다면
				m_pProcess = NULL;
			}
		}
		else{
			//적하 작업을 위한 HP를 미리 예약한다.
			int workingHP = DecideHPforLoadingContainer(m_pProcess);
			if( workingHP != -1 ){
				m_workingHP = workingHP;
				m_HPs[m_workingHP].SetReservation();
			}
			else{//HP예약을 못했다면
				m_pProcess = NULL;
			}
		}
	}

	return;
}

CProcess* CATC::SelectDischargingProcess()
{
	CProcess* pProcess = NULL;
	//HP에 놓여있는 컨테이너 중 양하 컨테이너가 있다면 작업 수행
	//HP를 완전히 비울 수 있는 것을 우선적으로 선택 (- 해당 작업에 소요될 시간은 미리 알 수 없음)
	int hp;
	for( hp = 0; hp < (g_CraneSpec.nHPperBlock * g_CraneSpec.nRowperBlock); ++hp ){
		if( m_HPs[hp].GetnContainer() == 1 ){			//A. HP 하나를 완전히 비울 수 있는 것부터 처리한다.
			string prsID = m_HPs[hp].GetProcessID();
			if( prsID.substr(0,1) == "D" ){				//B. 양하 작업이 맞는지 확인한다.
														//C. 예약 가능하다면 해당 작업 처리
				if( m_HPs[hp].CheckReservation() == false && CheckHPReservation(GetPairTP(hp)) == false ){														
					for( m_indexDischarge = m_DischargingSequence.begin(); m_indexDischarge != m_DischargingSequence.end(); ++m_indexDischarge ){
						if( prsID == (*m_indexDischarge)->prsID ){
							pProcess = (*m_indexDischarge);
							break;
						}
					}
					break;
				}		
			}
		}				
	}
	if( pProcess == NULL ){//HP를 완전히 비울 수 있는 양하 작업이 없다면
		for( hp = 0; hp < (g_CraneSpec.nHPperBlock * g_CraneSpec.nRowperBlock); ++hp ){
			if( m_HPs[hp].GetnContainer() == 2 ){
				string prsID = m_HPs[hp].GetProcessID();
				if( prsID.substr(0,1) == "D" ){	//B. 양하 작업이 맞는지 확인한다.
					if( m_HPs[hp].CheckReservation() == false && CheckHPReservation(GetPairTP(hp)) == false ){	//C. 예약 가능하다면 해당 작업 처리
						for( m_indexDischarge = m_DischargingSequence.begin(); m_indexDischarge != m_DischargingSequence.end(); ++m_indexDischarge ){
							if( prsID == (*m_indexDischarge)->prsID ){
								pProcess = (*m_indexDischarge);
								break;
							}
						}
						break;
					}		
				}
			}				
		}
	}
	return pProcess;
}

CProcess* CATC::SelectLoadingProcess()
{
	CProcess* pProcess = NULL;

	int minDeadline = -1;
	int qc, urgentQC = -1;
	for( qc = 0; qc < g_SimulationSpec.nQC; ++qc ){
		if( m_LoadingSeqPerQC[qc].empty() == false ){
			if( minDeadline < 0 ){
				minDeadline = m_LoadingSeqPerQC[qc][0]->deadline;
				urgentQC = qc;
			}
			else if( minDeadline > m_LoadingSeqPerQC[qc][0]->deadline ){
				minDeadline = m_LoadingSeqPerQC[qc][0]->deadline;
				urgentQC = qc;
			}
			else if( minDeadline == m_LoadingSeqPerQC[qc][0]->deadline ){
				if( rand()%100 > 50 ){
					minDeadline = m_LoadingSeqPerQC[qc][0]->deadline;
					urgentQC = qc;
				}						
			}
		}
	}
	if( urgentQC != -1 ){
		pProcess = m_LoadingSeqPerQC[urgentQC][0];
	}

	return pProcess;
}

bool CATC::CheckPairHPReservation( string hpStringID)
{
	int hpID = m_HPIDList[hpStringID];
	if( m_HPs[hpID].CheckReservation() == false && m_HPs[GetPairTP(hpID)].CheckReservation() == false ){
		return true;
	}
	return false;
}

bool CATC::CheckPairHPReservation( int hpID)
{
	if( m_HPs[hpID].CheckReservation() == false && m_HPs[GetPairTP(hpID)].CheckReservation() == false ){
		return true;
	}
	return false;
}

void CATC::GetProcessIDList(deque<string>& jobIDList)
{
	//HP에 놓여있는 적하 작업의 ID를 모은다.
	int hp;
	string prsID;
	deque<string> loadingIDList;
	for(hp=0;hp<g_SimulationSpec.nHPperBlock*g_SimulationSpec.nRowperBlock;++hp){
		for( int j=0; j<2; ++j){
			prsID = m_HPs[hp].GetPrsIDofContainer(j);
			if( prsID != "" ){
				if( prsID.substr(0,1) == "L" ){
					jobIDList.push_back(prsID);
				}
				else{
					loadingIDList.push_back(prsID);
				}
			}
		}
	}
	//QC와 중복되는 작업의 ID는 ProcessManager에서 처리
	return;
}