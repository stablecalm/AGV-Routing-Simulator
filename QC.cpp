#include "StdAfx.h"
#include ".\qc.h"

//constructor, destructor-------------------------------------------------------------------------------------
CQC::CQC(void)
{
}

CQC::CQC( ETrolleyType trolleyType, ESpreaderType spreaderType, int id, CPoint& initialPos )
:m_ID(id), m_workingHP(0), m_ptLocation(initialPos), m_iThroughput(0), m_idleTime(0), m_ptTrolley(m_ptLocation),
m_hoistingTime(0), m_travelTime(0), m_CWS(CWS_Idle), m_travelTimer(0), 
m_clockTick(0), m_numberBayWorks(0), m_worksDone(false), m_pProcess(NULL), 
m_CountDoubleCycle(0), m_PreviousDischarge(false),m_delayTime(0),
m_TrolleyType(trolleyType), m_SpreaderType(spreaderType)
{
	m_DeadlineMethod = Loose_Deadline;

	ostringstream sout;
	sout << "QC00E0" << id;
	m_StringID = sout.str();
	sout.str("");
	sout.clear();
		
	m_HPs.clear();

	//for drawing
	m_ptTrolley.y = m_ptLocation.y + g_CraneSpec.QCfrontReach + g_CraneSpec.QClegGap + 1000; //for Drawing
	m_IntervalOfMovement = 25;

	m_trolleySpeed = 4.0;
	m_emptyLoadSpeed = 3.0;
	m_fullLoadSpeed = 1.5;
	m_acctrolleySpeed = 0.8;
	m_accemptyLoadSpeed = 0.75;
	m_accfullLoadSpeed = 0.75;

	
}

CQC::~CQC(void)
{
	m_HPs.clear();				//TP정보를 삭제한다.
	m_HPIDList.clear();
	m_DischargingSequence.clear();	//양하 작업 순서
	m_LoadingSequence.clear();		//적하 작업 순서
}

//drawing operation--------------------------------------------------------------------------------------------
void CQC::DrawHPs( CDC& dc, double dZoom)
{
	//QC의 TP를 그린다.
	int index;
	for( index=0; index < (int)m_HPs.size(); ++index){
		m_HPs[index].DrawTP(dc, dZoom);
	}
	return;
}

void CQC::DrawCrane( CDC& dc, double dZoom )
{

	CPen pen, *pOldPen;
	CBrush brush_container, *pOldBrush;

	if( pen.CreatePen( PS_SOLID, 2, COLOR_QC ) ){		//QC를 그린다.
		pOldPen = dc.SelectObject( &pen );
		//QC양다리를 그리고
		CPoint ptQC = m_ptLocation;//left top 좌표
		CPoint legPos;
		legPos.x = ptQC.x;
		legPos.y = ptQC.y + (int)(g_CraneSpec.QCfrontReach) + 1000;
		//해측다리
		dc.MoveTo( (int)(legPos.x * dZoom), (int)(legPos.y * dZoom) );
		dc.LineTo( (int)((legPos.x + g_CraneSpec.QClegWidth) * dZoom), (int)(legPos.y * dZoom) );
		//육측다리
		legPos.y += (int)(g_CraneSpec.QClegGap);
		dc.MoveTo( (int)(legPos.x * dZoom), (int)(legPos.y * dZoom) );
		dc.LineTo( (int)((legPos.x + g_CraneSpec.QClegWidth) * dZoom), (int)(legPos.y * dZoom) );

		//QC몸통 그리고
		CPoint body;
		body.x = ptQC.x + (int)( (g_CraneSpec.QClegWidth - g_CraneSpec.QCwidth)/2.0 );
		body.y = ptQC.y;
		dc.MoveTo( (int)(body.x * dZoom), (int)(body.y * dZoom) );
		dc.LineTo( (int)(body.x * dZoom), (int)((body.y + g_CraneSpec.QClength)*dZoom) );
		body.x += (int)(g_CraneSpec.QCwidth);
		dc.MoveTo( (int)(body.x * dZoom), (int)(body.y * dZoom) );
		dc.LineTo( (int)(body.x * dZoom), (int)((body.y + g_CraneSpec.QClength)*dZoom) );
		dc.SelectObject(pOldPen);
		pen.DeleteObject();				
	}

	//Trolley의 컨테이너를 그린다.
	if( pen.CreatePen(PS_SOLID, 1, COLOR_BLACK) && brush_container.CreateSolidBrush( COLOR_CONTAINER_HIGH ) ){
		pOldPen = dc.SelectObject( &pen );
		pOldBrush = dc.SelectObject( &brush_container );

		CPoint ptTrolley = m_ptTrolley;//Trolley 좌표

		//트롤리의 컨테이너 부분을 그린다.
		CPoint ptTP = m_HPs[0].GetPtCenter();

		CPoint buffer_area[4];
		buffer_area[0].x = (int)( (ptTP.x - g_CraneSpec.Container40ftHalfLength) * dZoom);
		buffer_area[0].y = (int)( (ptTrolley.y + g_CraneSpec.Container40ftHalfWidth) * dZoom);
		buffer_area[1].x = (int)( (ptTP.x + g_CraneSpec.Container40ftHalfLength) * dZoom);
		buffer_area[1].y = (int)( (ptTrolley.y + g_CraneSpec.Container40ftHalfWidth) * dZoom);
		buffer_area[2].x = (int)( (ptTP.x + g_CraneSpec.Container40ftHalfLength) * dZoom);
		buffer_area[2].y = (int)( (ptTrolley.y - g_CraneSpec.Container40ftHalfWidth) * dZoom);
		buffer_area[3].x = (int)( (ptTP.x - g_CraneSpec.Container40ftHalfLength) * dZoom);
		buffer_area[3].y = (int)( (ptTrolley.y - g_CraneSpec.Container40ftHalfWidth) * dZoom);			

		dc.MoveTo(buffer_area[3]);
		int i;
		for(i=0; i<4; ++i){
			dc.LineTo(buffer_area[i]);
		}

		bool DrawingContainerRequired = false;
		if( m_CWS == CWS_LoadedTravel			||	m_CWS == CWS_WaitingforDropoff	|| m_CWS == CWS_TravelforDropoff	||	m_CWS == CWS_Dropoff ){	
				DrawingContainerRequired = true;
		}
		if( DrawingContainerRequired ){
			dc.Polygon(buffer_area, 4);
		}
		dc.SelectObject(pOldPen);
		pen.DeleteObject();
		dc.SelectObject(pOldBrush);
		pen.DeleteObject();
	}

	//Drawing the motion of the trolley
	if( pen.CreatePen(PS_SOLID,2, COLOR_QC) ){
		pOldPen = dc.SelectObject( &pen );

		CPoint ptTrolley = m_ptTrolley;//Trolley 좌표
		ptTrolley.x += (int)( (g_CraneSpec.QClegWidth - g_CraneSpec.QCwidth)/2.0 ) - g_CraneSpec.QCTrolleyLegGap;

		//트롤리의 중심선 그리기
		dc.MoveTo( (int)(ptTrolley.x * dZoom), (int)(ptTrolley.y* dZoom) );
		dc.LineTo( (int)((ptTrolley.x + g_CraneSpec.QCwidth + 2*g_CraneSpec.QCTrolleyLegGap) * dZoom), (int)(ptTrolley.y*dZoom) );
		//트롤리의 양다리 그리기
		dc.MoveTo( (int)(ptTrolley.x * dZoom), (int)((ptTrolley.y - g_CraneSpec.Container40ftWidth) * dZoom) );
		dc.LineTo( (int)(ptTrolley.x * dZoom), (int)((ptTrolley.y + g_CraneSpec.Container40ftWidth) * dZoom) );		

		ptTrolley.x += (int)(g_CraneSpec.QCwidth + 2*g_CraneSpec.QCTrolleyLegGap);
		dc.MoveTo( (int)(ptTrolley.x * dZoom), (int)((ptTrolley.y - g_CraneSpec.Container40ftWidth) * dZoom) );
		dc.LineTo( (int)(ptTrolley.x * dZoom), (int)((ptTrolley.y + g_CraneSpec.Container40ftWidth) * dZoom) );
		dc.SelectObject(pOldPen);
		pen.DeleteObject();
	}

	//현재 QC의 상태를 표시한다.
	CString str;	
	CPoint ptStr = m_ptLocation;//left top 좌표
	ptStr.x += g_CraneSpec.QClegWidth + 200;
	ptStr.y += g_CraneSpec.QCfrontReach + 1200;

	//QC의 Throughput 표시
	str.Format( _T("Throughput: %d"), m_iThroughput );
	dc.TextOut((int)(ptStr.x * dZoom), (int)((ptStr.y) * dZoom), str );

	//QC의 총 Idle Time
	str.Format( _T("Idle Time: %d"), m_idleTime );
	dc.TextOut((int)(ptStr.x * dZoom), (int)((ptStr.y+1000) * dZoom), str );

	//QC가 현재 처리 중인 Process ID
	if( m_pProcess != NULL ){
		str.Format( _T("Prs ID: %s"), m_pProcess->prsID.c_str() );
		dc.TextOut((int)(ptStr.x * dZoom), (int)((ptStr.y+1500) * dZoom), str );
	}
	else{
		str.Format( _T("Prs ID: NULL") );
		dc.TextOut((int)(ptStr.x * dZoom), (int)((ptStr.y+1500) * dZoom), str );
	}
	
}

//public operation--------------------------------------------------------------------------------------------
void CQC::Initialize( int id, int workingTP, CPoint& initialPos ){
	m_iThroughput = 0;
	m_ID = id;
	m_workingHP = workingTP;
	m_ptLocation = initialPos;
}

int CQC::GetReservableHPforPassPoint()
{
	int hpID;
	for( hpID = 0; hpID < g_CraneSpec.nHPperQC; ++hpID ){
		if( m_HPs[hpID].CheckReservation() == false ){
			return hpID;
		}
	}
	return -1;	//예약 가능한 TP가 하나도 없음.
}

int	CQC::GetReservableHPfor( int Capacity )
{
	int endHP;
	if( m_CargoType == CargoType_DoubleCycle && !m_DischargingSequence.empty() && !m_LoadingSequence.empty()){
		endHP = 2;
	}
	else{
		endHP = g_CraneSpec.nHPperQC;
	}

	int needCapacity = Capacity;
	int hpID;
	/*
	if( m_TrolleyType == Trolley_Tandem || m_TrolleyType == Trolley_TandemDouble ){

	}
	else{
	}
	*/
	if( m_SpreaderType == Spreader_Single || m_SpreaderType == Spreader_Twin){
		for( hpID = 0; hpID < endHP; ++hpID ){
			if( m_HPs[hpID].CheckReservation() == false ){//예약 가능한 것
				if( m_HPs[hpID].GetRemainCapacity() == 2 ){	//크레인측 TP를 우선적으로 반환한다.
					return hpID;
				}
			}		
		}
	}
	
	return -1;	//예약 가능하고 컨테이너가 놓여있지 않은 TP가 하나도 없음.
}

//public operation--------------------------------------------------------------------------------------------
void CQC::SetHP( CHP hp )
{ 
	m_HPs.push_back(hp); 
	m_HPIDList.insert( make_pair( hp.GetStringID(), hp.GetID())); 
	return;
}

void CQC::SetDischargingSequence( deque<CProcess*> &prsSequence )
{
	m_DischargingSequence = prsSequence;
	return;
}

void CQC::SetLoadingSequence( deque<CProcess*> &prsSequence )
{
	m_LoadingSequence = prsSequence;
	return;
}

void CQC::DoSingleIteration(){
	if( m_worksDone == false ){
		++m_clockTick;
	}
	if( m_DischargingSequence.empty() == false || m_LoadingSequence.empty() == false ){
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
	}
	else{//수행할 작업이 없다면
		m_worksDone = true;
	}

	return;
}

int CQC::GetHPofDischargingProcess(CProcess* pProcess)
{
	int workingHP = -1;
	int startHP;
	if( m_CargoType == CargoType_DoubleCycle && !m_DischargingSequence.empty() && !m_LoadingSequence.empty()){//이 경우 HP의 반은 양하 작업에 반은 적하 작업에 사용
		startHP = 2;
	}
	else{
		startHP = 0;
	}

	int hpID;
	//현재 작업의 종류를 확인한다.
	vector<pair<ERelativePosition, string>> jobList = pProcess->ALVJob->connectedJobList;
	if( jobList.empty() == false ){//JobList가 비어있지 않은 경우

		//현재 프로세스의 상대적 위치 확인
		if( pProcess->ALVJob->relativePosition == ERP_Left ){//20x20ft 작업
			//이 경우 HP의 RemainCapacity가 2 이상이어야함.
			for( hpID = startHP; hpID < g_CraneSpec.nHPperQC; ++hpID ){
				if( m_HPs[hpID].GetRemainCapacity() == 2 ){
					if( m_HPs[hpID].CheckReservation() == false ){
						workingHP = hpID;
						break;
					}
				}
			}
		}
		//다음의 경우 현재 프로세스가 예약하고자 하는 HP Pair는 (0,1) 또는 (2,3). 모두 RemainCapacity가 2 이상이어야함.
		else if( pProcess->ALVJob->relativePosition == ERP_Top){//40x40ft 작업
			for( hpID = startHP; hpID < g_CraneSpec.nHPperQC; hpID+=2 ){
				if( m_HPs[hpID].GetRemainCapacity() == 2 && m_HPs[hpID+1].GetRemainCapacity() == 2 ){
					if( m_HPs[hpID].CheckReservation() == false && m_HPs[hpID+1].CheckReservation() == false ){
						workingHP = hpID;
						break;
					}
				}
			}
		}
		else if(pProcess->ALVJob->relativePosition == ERP_LeftTop){//20x20x20x20ft 작업
			for( hpID = startHP; hpID < g_CraneSpec.nHPperQC; hpID+=2 ){
				if( m_HPs[hpID].GetRemainCapacity() == 2 && m_HPs[hpID+1].GetRemainCapacity() == 2 ){
					if( m_HPs[hpID].CheckReservation() == false && m_HPs[hpID+1].CheckReservation() == false ){
						workingHP = hpID;
						break;
					}
				}
			}
		}
	}
	else{//Single-20ft 또는 Single-40ft 작업					
		if( pProcess->ALVJob->container.conType == ConType_General20ft ){
			//Single-20ft인 경우, HP의 capacity가 1인 것을 찾아 꽉 채워주는 방향으로 내려놓는다.
			for( hpID = startHP; hpID < g_CraneSpec.nHPperQC; ++hpID ){
				if( m_HPs[hpID].GetRemainCapacity() == 1 ){
					if( m_HPs[hpID].CheckReservation() == false ){
						workingHP = hpID;
						break;
					}
				}
			}
			if( workingHP == -1){//20ft 하나만 놓여있는 HP를 못찾았다면
				for( hpID = startHP; hpID < g_CraneSpec.nHPperQC; ++hpID ){
					if( m_HPs[hpID].GetRemainCapacity() == 2 ){
						if( m_HPs[hpID].CheckReservation() == false ){
							workingHP = hpID;
							break;
						}
					}
				}
			}
		}
		else{//Single-40ft 작업					
			for( hpID = startHP; hpID < g_CraneSpec.nHPperQC; ++hpID ){
				if( m_HPs[hpID].GetRemainCapacity() == 2 ){
					if( m_HPs[hpID].CheckReservation() == false ){
						workingHP = hpID;
						break;
					}
				}
			}		
		}			
	}

	return workingHP;
}

int	CQC::GetHPofLoadingProcess(CProcess* pProcess)
{
	int workingHP = -1;

	if( pProcess->ALVJob->QuayHPID != "" ){
		int hpID = m_HPIDList[pProcess->ALVJob->QuayHPID];

		vector<pair<ERelativePosition, string>> jobList = pProcess->ALVJob->connectedJobList;
		if( jobList.empty() == false ){//JobList가 비어있지 않은 경우
			//현재 프로세스의 상대적 위치 확인
			if( m_HPs[hpID].GetPrsIDofContainer(0) == pProcess->prsID && m_HPs[hpID].GetPrsIDofContainer(1) == m_LoadingSequence[1]->prsID ){
				if( m_HPs[hpID].CheckReservation() == false ){			//HP가 예약되어있는 상태가 아님
					workingHP = hpID;
				}
			}
			else if( m_HPs[hpID].GetPrsIDofContainer(1) == pProcess->prsID && m_HPs[hpID].GetPrsIDofContainer(0) == m_LoadingSequence[1]->prsID ){
				if( m_HPs[hpID].CheckReservation() == false ){			//HP가 예약되어있는 상태가 아님
					workingHP = hpID;
				}
			}

			//현재 프로세스의 상대적 위치 확인
			if( pProcess->ALVJob->relativePosition == ERP_Left ){//20x20ft 작업
				if( m_HPs[hpID].GetPrsIDofContainer(0) == pProcess->prsID && m_HPs[hpID].GetPrsIDofContainer(1) == m_LoadingSequence[1]->prsID ){
					if( m_HPs[hpID].CheckReservation() == false ){			//HP가 예약되어있는 상태가 아님
						workingHP = hpID;
					}
				}
				else if( m_HPs[hpID].GetPrsIDofContainer(1) == pProcess->prsID && m_HPs[hpID].GetPrsIDofContainer(0) == m_LoadingSequence[1]->prsID ){
					if( m_HPs[hpID].CheckReservation() == false ){			//HP가 예약되어있는 상태가 아님
						workingHP = hpID;
					}
				}
			}
			//다음의 경우 현재 프로세스가 예약하고자 하는 HP Pair는 (0,1) 또는 (2,3). 모두 RemainCapacity가 2 이상이어야함.
			else if( pProcess->ALVJob->relativePosition == ERP_Top){//40x40ft 작업
				if( m_HPs[hpID].GetPrsIDofContainer(0) == pProcess->prsID && m_HPs[hpID+1].GetPrsIDofContainer(0) == m_LoadingSequence[1]->prsID){//목적 컨테이너가 모두 도착했는가
					if( m_HPs[hpID].CheckReservation() == false && m_HPs[hpID+1].CheckReservation() == false){//HP가 예약되어있는 상태가 아님
						workingHP = hpID;
					}
				}					
			}
			else if(pProcess->ALVJob->relativePosition == ERP_LeftTop){//20x20x20x20ft 작업
				bool checkUpperBound = false;
				//상단 두 개의 컨테이너가 놓여있는지 확인한다.
				if( m_HPs[hpID].GetPrsIDofContainer(0) == pProcess->prsID && m_HPs[hpID].GetPrsIDofContainer(1) == m_LoadingSequence[1]->prsID ){
					if( m_HPs[hpID].CheckReservation() == false ){			//HP가 예약되어있는 상태가 아님
						checkUpperBound =true;
					}
				}
				else if( m_HPs[hpID].GetPrsIDofContainer(1) == pProcess->prsID && m_HPs[hpID].GetPrsIDofContainer(0) == m_LoadingSequence[1]->prsID ){
					if( m_HPs[hpID].CheckReservation() == false ){			//HP가 예약되어있는 상태가 아님
						checkUpperBound =true;
					}
				}
				//하단 두 개의 컨테이너가 놓여있는지 확인한다.
				if( checkUpperBound ){
					if( m_HPs[hpID+1].GetPrsIDofContainer(0) == m_LoadingSequence[2]->prsID && m_HPs[hpID+1].GetPrsIDofContainer(1) == m_LoadingSequence[3]->prsID ){
						if( m_HPs[hpID+1].CheckReservation() == false ){			//HP가 예약되어있는 상태가 아님
							workingHP = hpID;
						}
					}
					else if( m_HPs[hpID+1].GetPrsIDofContainer(1) == m_LoadingSequence[3]->prsID && m_HPs[hpID+1].GetPrsIDofContainer(0) == m_LoadingSequence[2]->prsID ){
						if( m_HPs[hpID+1].CheckReservation() == false ){			//HP가 예약되어있는 상태가 아님
							workingHP = hpID;
						}
					}					
				}
			}

		}
		//Single-20ft 또는 Single-40ft 작업					
		else{
			if( m_HPs[hpID].CheckReservation() == false ){
				workingHP = hpID;
			}					
		}				
	}
	return workingHP;
}

void CQC::HPReservationofProcess( int hpID )
{
	vector<pair<ERelativePosition, string>> jobList = m_pProcess->ALVJob->connectedJobList;

	if( jobList.empty() == false ){//JobList가 비어있지 않은 경우

		if( m_pProcess->ALVJob->relativePosition == ERP_Left ){//20x20ft 작업
			m_HPs[hpID].SetReservation();			
		}
		else if( m_pProcess->ALVJob->relativePosition == ERP_Top){//40x40ft 작업
			m_HPs[hpID].SetReservation();
			m_HPs[hpID+1].SetReservation();
		}
		else if(m_pProcess->ALVJob->relativePosition == ERP_LeftTop){//20x20x20x20ft 작업
			m_HPs[hpID].SetReservation();
			m_HPs[hpID+1].SetReservation();
		}
	}
	else{	//Single-20ft 또는 Single-40ft 작업					
		m_HPs[hpID].SetReservation();
	}				
	return;
}

void CQC::HPReleaseofProcess( int hpID )
{
	vector<pair<ERelativePosition, string>> jobList = m_pProcess->ALVJob->connectedJobList;

	if( jobList.empty() == false ){//JobList가 비어있지 않은 경우

		if( m_pProcess->ALVJob->relativePosition == ERP_Left ){//20x20ft 작업
			m_HPs[hpID].ReleaseReservation();			
		}
		else if( m_pProcess->ALVJob->relativePosition == ERP_Top){//40x40ft 작업
			m_HPs[hpID].ReleaseReservation();
			m_HPs[hpID+1].ReleaseReservation();
		}
		else if(m_pProcess->ALVJob->relativePosition == ERP_LeftTop){//20x20x20x20ft 작업
			m_HPs[hpID].ReleaseReservation();
			m_HPs[hpID+1].ReleaseReservation();
		}
	}
	else{	//Single-20ft 또는 Single-40ft 작업					
		m_HPs[hpID].ReleaseReservation();
	}				
	return;
}

void CQC::SetDelayTime(int dTime)
{
	if( m_pProcess->prsType == PrsType_Discharging  ){
		vector<pair<ERelativePosition, string>> jobList = m_pProcess->ALVJob->connectedJobList;

		if( jobList.empty() == false ){//JobList가 비어있지 않은 경우
			//연결된 모든 작업의 delay가 동일하다.
			if( m_pProcess->ALVJob->relativePosition == ERP_Left ){//20x20ft 작업
				m_pProcess->ALVJob->delayTime = dTime;
				m_DischargingSequence[1]->ALVJob->delayTime = dTime;
			}
			else if( m_pProcess->ALVJob->relativePosition == ERP_Top){//40x40ft 작업
				m_pProcess->ALVJob->delayTime = dTime;
				m_DischargingSequence[1]->ALVJob->delayTime = dTime;
			}
			else if(m_pProcess->ALVJob->relativePosition == ERP_LeftTop){//20x20x20x20ft 작업
				m_pProcess->ALVJob->delayTime = dTime;
				m_DischargingSequence[1]->ALVJob->delayTime = dTime;
				m_DischargingSequence[2]->ALVJob->delayTime = dTime;
				m_DischargingSequence[3]->ALVJob->delayTime = dTime;
			}
		}
		else{	//Single-20ft 또는 Single-40ft 작업
			m_pProcess->ALVJob->delayTime = dTime;
		}
	}
	else if( m_pProcess->prsType == PrsType_Loading ){
		vector<pair<ERelativePosition, string>> jobList = m_pProcess->ALVJob->connectedJobList;
		if( m_pProcess->ALVJob->YardHPID == "" ){
			if( jobList.empty() == false ){//JobList가 비어있지 않은 경우
				//도착하지 않은 작업의 delay만 갱신한다.
				if( m_pProcess->ALVJob->relativePosition == ERP_Left ){//20x20ft 작업
					m_pProcess->ALVJob->delayTime = dTime;
					m_LoadingSequence[1]->ALVJob->delayTime = dTime;
				}
				//다음의 경우 현재 프로세스가 예약하고자 하는 HP Pair는 (0,1) 또는 (2,3). 모두 RemainCapacity가 2 이상이어야함.
				else if( m_pProcess->ALVJob->relativePosition == ERP_Top){//40x40ft 작업
					m_pProcess->ALVJob->delayTime = dTime;
					m_LoadingSequence[1]->ALVJob->delayTime = dTime;
				}
				else if(m_pProcess->ALVJob->relativePosition == ERP_LeftTop){//20x20x20x20ft 작업
					m_pProcess->ALVJob->delayTime = dTime;
					m_LoadingSequence[1]->ALVJob->delayTime = dTime;
					m_LoadingSequence[2]->ALVJob->delayTime = dTime;
					m_LoadingSequence[3]->ALVJob->delayTime = dTime;
				}
			}
			//Single-20ft 또는 Single-40ft 작업					
			else{
				m_pProcess->ALVJob->delayTime = dTime;
			}		
		}
		else{
			if( jobList.empty() == false ){//JobList가 비어있지 않은 경우
				//도착하지 않은 작업의 delay만 갱신한다.
				if( m_pProcess->ALVJob->relativePosition == ERP_Left ){//20x20ft 작업
					if( m_pProcess->prsID != m_HPs[m_HPIDList[m_pProcess->ALVJob->YardHPID]].GetPrsIDofContainer(0) && m_pProcess->prsID != m_HPs[m_HPIDList[m_pProcess->ALVJob->YardHPID]].GetPrsIDofContainer(1) ){
						m_pProcess->ALVJob->delayTime = dTime;
					}
					if( m_LoadingSequence[1]->prsID != m_HPs[m_HPIDList[m_pProcess->ALVJob->YardHPID]].GetPrsIDofContainer(0) && m_LoadingSequence[1]->prsID != m_HPs[m_HPIDList[m_pProcess->ALVJob->YardHPID]].GetPrsIDofContainer(1) ){
						m_LoadingSequence[1]->ALVJob->delayTime = dTime;
					}
				}
				//다음의 경우 현재 프로세스가 예약하고자 하는 HP Pair는 (0,1) 또는 (2,3). 모두 RemainCapacity가 2 이상이어야함.
				else if( m_pProcess->ALVJob->relativePosition == ERP_Top){//40x40ft 작업
					if( m_pProcess->prsID != m_HPs[m_HPIDList[m_pProcess->ALVJob->YardHPID]].GetPrsIDofContainer(0) ){
						m_pProcess->ALVJob->delayTime = dTime;
					}
					if( m_LoadingSequence[1]->prsID != m_HPs[m_HPIDList[m_pProcess->ALVJob->YardHPID]+1].GetPrsIDofContainer(0) ){
						m_LoadingSequence[1]->ALVJob->delayTime = dTime;
					}	
				}
				else if(m_pProcess->ALVJob->relativePosition == ERP_LeftTop){//20x20x20x20ft 작업
					if( m_pProcess->prsID != m_HPs[m_HPIDList[m_pProcess->ALVJob->YardHPID]].GetPrsIDofContainer(0) && m_pProcess->prsID != m_HPs[m_HPIDList[m_pProcess->ALVJob->YardHPID]].GetPrsIDofContainer(1) ){
						m_pProcess->ALVJob->delayTime = dTime;
					}
					if( m_LoadingSequence[1]->prsID != m_HPs[m_HPIDList[m_pProcess->ALVJob->YardHPID]].GetPrsIDofContainer(0) && m_LoadingSequence[1]->prsID != m_HPs[m_HPIDList[m_pProcess->ALVJob->YardHPID]].GetPrsIDofContainer(1) ){
						m_LoadingSequence[1]->ALVJob->delayTime = dTime;
					}
					if( m_LoadingSequence[2]->prsID != m_HPs[m_HPIDList[m_pProcess->ALVJob->YardHPID]+1].GetPrsIDofContainer(0) && m_LoadingSequence[2]->prsID != m_HPs[m_HPIDList[m_pProcess->ALVJob->YardHPID]+1].GetPrsIDofContainer(1) ){
						m_LoadingSequence[2]->ALVJob->delayTime = dTime;
					}
					if( m_LoadingSequence[3]->prsID != m_HPs[m_HPIDList[m_pProcess->ALVJob->YardHPID]+1].GetPrsIDofContainer(0) && m_LoadingSequence[3]->prsID != m_HPs[m_HPIDList[m_pProcess->ALVJob->YardHPID]+1].GetPrsIDofContainer(1) ){
						m_LoadingSequence[3]->ALVJob->delayTime = dTime;
					}
				}
			}
			//Single-20ft 또는 Single-40ft 작업					
			else{
				m_pProcess->ALVJob->delayTime = dTime;
			}
		}

		
						
	}
	return;
}

void CQC::SetContainerofProcess( int hpID )
{
	//양하
	vector<pair<ERelativePosition, string>> jobList = m_pProcess->ALVJob->connectedJobList;

	if( jobList.empty() == false ){//JobList가 비어있지 않은 경우

		if( m_pProcess->ALVJob->relativePosition == ERP_Left ){//20x20ft 작업
			m_HPs[hpID].SetContainer(m_pProcess->ALVJob->container);
			m_HPs[hpID].SetContainer(m_DischargingSequence[1]->ALVJob->container);
		}
		else if( m_pProcess->ALVJob->relativePosition == ERP_Top){//40x40ft 작업
			m_HPs[hpID].SetContainer(m_pProcess->ALVJob->container);
			m_HPs[hpID+1].SetContainer(m_DischargingSequence[1]->ALVJob->container);
		}
		else if(m_pProcess->ALVJob->relativePosition == ERP_LeftTop){//20x20x20x20ft 작업
			m_HPs[hpID].SetContainer(m_pProcess->ALVJob->container);
			m_HPs[hpID].SetContainer(m_DischargingSequence[1]->ALVJob->container);
			m_HPs[hpID+1].SetContainer(m_DischargingSequence[2]->ALVJob->container);
			m_HPs[hpID+1].SetContainer(m_DischargingSequence[3]->ALVJob->container);
		}
	}
	else{	//Single-20ft 또는 Single-40ft 작업					
		m_HPs[hpID].SetContainer(m_pProcess->ALVJob->container);
	}				
	return;
}

void CQC::ReleaseContainerofProcess( int hpID )
{
	//적하
	vector<pair<ERelativePosition, string>> jobList = m_pProcess->ALVJob->connectedJobList;

	if( jobList.empty() == false ){//JobList가 비어있지 않은 경우

		if( m_pProcess->ALVJob->relativePosition == ERP_Left ){//20x20ft 작업
			m_HPs[hpID].ReleaseContainer(m_pProcess->ALVJob->container);
			m_HPs[hpID].ReleaseContainer(m_LoadingSequence[1]->ALVJob->container);
		}
		else if( m_pProcess->ALVJob->relativePosition == ERP_Top){//40x40ft 작업
			m_HPs[hpID].ReleaseContainer(m_pProcess->ALVJob->container);
			m_HPs[hpID+1].ReleaseContainer(m_LoadingSequence[1]->ALVJob->container);
		}
		else if(m_pProcess->ALVJob->relativePosition == ERP_LeftTop){//20x20x20x20ft 작업
			m_HPs[hpID].ReleaseContainer(m_pProcess->ALVJob->container);
			m_HPs[hpID].ReleaseContainer(m_LoadingSequence[1]->ALVJob->container);
			m_HPs[hpID+1].ReleaseContainer(m_LoadingSequence[2]->ALVJob->container);
			m_HPs[hpID+1].ReleaseContainer(m_LoadingSequence[3]->ALVJob->container);
		}
	}
	else{	//Single-20ft 또는 Single-40ft 작업					
		m_HPs[hpID].ReleaseContainer(m_pProcess->ALVJob->container);
	}
	return;
}

void CQC::DeclareProcessFinished( int hpID )
{
	if( m_pProcess->prsType == PrsType_Discharging  ){
		vector<pair<ERelativePosition, string>> jobList = m_pProcess->ALVJob->connectedJobList;

		if( jobList.empty() == false ){//JobList가 비어있지 않은 경우

			if( m_pProcess->ALVJob->relativePosition == ERP_Left ){//20x20ft 작업//프로세스 정보 업데이트
				int i;
				for( i=0; i<2; ++i ){
					m_DischargingSequence[0]->quayHP			= m_HPs[hpID].GetStringID();
					m_DischargingSequence[0]->ALVJob->QuayHPID	= m_HPs[hpID].GetStringID();
					m_DischargingSequence.pop_front();
				}
			}
			else if( m_pProcess->ALVJob->relativePosition == ERP_Top){//40x40ft 작업//프로세스 정보 업데이트
				m_DischargingSequence[0]->quayHP = m_HPs[hpID].GetStringID();
				m_DischargingSequence[0]->ALVJob->QuayHPID = m_HPs[hpID].GetStringID();
				m_DischargingSequence[1]->quayHP = m_HPs[hpID+1].GetStringID();
				m_DischargingSequence[1]->ALVJob->QuayHPID = m_HPs[hpID+1].GetStringID();
				m_DischargingSequence.pop_front();
				m_DischargingSequence.pop_front();
			}
			else if(m_pProcess->ALVJob->relativePosition == ERP_LeftTop){//20x20x20x20ft 작업//프로세스 정보 업데이트
				int i;
				for( i=0; i<2; ++i ){
					m_DischargingSequence[0]->quayHP			= m_HPs[hpID].GetStringID();
					m_DischargingSequence[0]->ALVJob->QuayHPID	= m_HPs[hpID].GetStringID();
					m_DischargingSequence.pop_front();
				}
				for( i=0; i<2; ++i ){
					m_DischargingSequence[0]->quayHP			= m_HPs[hpID+1].GetStringID();
					m_DischargingSequence[0]->ALVJob->QuayHPID	= m_HPs[hpID+1].GetStringID();
					m_DischargingSequence.pop_front();
				}
			}
		}
		else{	//Single-20ft 또는 Single-40ft 작업//프로세스 정보 업데이트
			m_DischargingSequence[0]->quayHP			= m_HPs[hpID].GetStringID();
			m_DischargingSequence[0]->ALVJob->QuayHPID	= m_HPs[hpID].GetStringID();
			m_DischargingSequence.pop_front();
		}
	}
	else if( m_pProcess->prsType == PrsType_Loading ){
		deque<CProcess*>::iterator index;

		vector<pair<ERelativePosition, string>> jobList = m_pProcess->ALVJob->connectedJobList;		
		if( jobList.empty() == false ){//JobList가 비어있지 않은 경우

			if( m_pProcess->ALVJob->relativePosition == ERP_Left ){//20x20ft 작업
				for( int i=0; i<2; ++i ){
					m_pEquipmentManager->ProcessFinished(m_LoadingSequence[0]);
					m_LoadingSequence.pop_front();
				}
			}
			else if( m_pProcess->ALVJob->relativePosition == ERP_Top){//40x40ft 작업
				for( int i=0; i<2; ++i ){
					m_pEquipmentManager->ProcessFinished(m_LoadingSequence[0]);
					m_LoadingSequence.pop_front();
				}
			}
			else if(m_pProcess->ALVJob->relativePosition == ERP_LeftTop){//20x20x20x20ft 작업
				for( int i=0; i<4; ++i ){
					m_pEquipmentManager->ProcessFinished(m_LoadingSequence[0]);
					m_LoadingSequence.pop_front();
				}
			}
		}
		else{	//Single-20ft 또는 Single-40ft 작업					
			m_pEquipmentManager->ProcessFinished(m_pProcess);
			m_LoadingSequence.pop_front();
		}		
	}

	m_pProcess = NULL;

	return;
}

void CQC::RecalculateDeadlineOfLoadingProcess()
{
	int nSet = 1;//QC가 한 번에 처리하는 컨테이너 수
	if( m_SpreaderType == Spreader_Single ){
		nSet = 1;
	}
	else if( m_SpreaderType == Spreader_Twin ){
		if( m_BayType == BayType_20ft ){
			nSet = 2;
		}
		else if( m_BayType == BayType_40ft ){
			nSet = 1;
		}				
	}
	else if( m_SpreaderType == Spreader_TandemTwin ){
		if( m_BayType == BayType_20ft ){
			nSet = 4;
		}
		else if( m_BayType == BayType_40ft ){
			nSet = 2;
		}		
	}

	int i;
	deque<CProcess*>::iterator index = m_LoadingSequence.begin();
	if( m_pProcess->prsType == PrsType_Loading ){
		for( i=0; i<nSet; ++i){
			++index;//현재 처리 중인 프로세스는 스킵
		}		
	}	
	int count = 1;
	for( ; index != m_LoadingSequence.end(); ){
		for( i=0; i<nSet; ++i){
			(*index)->deadline = (long)(m_clockTick*g_SimulationSpec.unitTime) + count*2*g_CraneSpec.halfCycletime ;
			++index;
		}		
		++count;
	}
	
	return;
}

void CQC::DoLoad()	//적하 시 QC는 작업 요청을 하지 않음
{
	switch( m_CWS)
	{
	case CWS_Idle://작업 태스크가 존재하는 TP 선택.
		{
			m_ptTrolley.y = m_ptLocation.y + g_CraneSpec.QCfrontReach + g_CraneSpec.QClegGap + 1000; //for Drawing
			m_CWS = CWS_WaitingforPickup;
		}		
		break;
	
	case CWS_WaitingforPickup://작업 태스크가 존재하는 TP가 작업 가능해질 때까지 대기
		{
			m_ptTrolley.y = m_ptLocation.y + g_CraneSpec.QCfrontReach + g_CraneSpec.QClegGap + 1000; //for Drawing			

			m_workingHP = GetHPofLoadingProcess(m_pProcess);
			if( m_workingHP == -1 ){
				if( m_worksDone == false ){//traveling이 끝났는데 hoisting을 시작하지 못한다면 -> QC 지연
					++m_delayTime;
					SetDelayTime((long)(m_delayTime*g_SimulationSpec.unitTime));
				}
			}
			else{
				m_idleTime += m_delayTime;
				m_delayTime = 0;
				HPReservationofProcess( m_workingHP );
				m_CWS = CWS_TravelforPickup;

				//컨테이너의 위치를 가져온다.
				m_ConRowLength = (int)((double)m_pProcess->shipLoc.height * 2.39);
				m_ConColLength = (int)((double)m_pProcess->shipLoc.slot * 2.35);				
			}
		}
		break;

	case CWS_TravelforPickup://작업 태스크가 존재하는 TP까지 이동 재개.
		{
			m_ptTrolley.y += m_IntervalOfMovement; //for Drawing

			if( m_ptTrolley.y >= m_HPs[m_workingHP].GetPtCenter().y ){
				//컨테이너를 내리고 다시 들어올리는데 걸리는 시간
				double distance = (26.6)/2;
				double da = (m_emptyLoadSpeed*m_emptyLoadSpeed)/(2*m_accemptyLoadSpeed);
				if( distance >= da ){
					m_hoistingTimer = (int)((m_emptyLoadSpeed/m_accemptyLoadSpeed+ distance/m_emptyLoadSpeed- m_emptyLoadSpeed/(2*m_accemptyLoadSpeed))/g_CraneSpec.unitTime);	//Time을 Clock으로 변환
				}
				else{
					m_hoistingTimer = (int)((sqrt(2*distance/m_accemptyLoadSpeed))/g_CraneSpec.unitTime);	//Time을 Clock으로 변환
				}
				da = (m_fullLoadSpeed*m_fullLoadSpeed)/(2*m_accfullLoadSpeed);
				if( distance >= da ){
					m_hoistingTimer += (int)((m_fullLoadSpeed/m_accfullLoadSpeed+ distance/m_fullLoadSpeed- m_fullLoadSpeed/(2*m_accfullLoadSpeed))/g_CraneSpec.unitTime);	//Time을 Clock으로 변환
				}
				else{
					m_hoistingTimer += (int)((sqrt(2*distance/m_accfullLoadSpeed))/g_CraneSpec.unitTime);	//Time을 Clock으로 변환
				}
				m_hoistingTimer *= 2;
				m_CWS = CWS_Pickup;
			}
		}
		break;
	case CWS_Pickup:
		{
			m_ptTrolley.y = m_HPs[m_workingHP].GetPtCenter().y;//for Drawing

			--m_hoistingTimer;
			if( m_hoistingTimer <= 0 ){
				//hoisting이 끝나면 작업 중인 TP에 컨테이너 해제
				ReleaseContainerofProcess( m_workingHP );
				HPReleaseofProcess( m_workingHP );
				RecalculateDeadlineOfLoadingProcess();
				
				//트롤리 이동 시간 계산
				double distance = (52 + m_ConColLength)/2;
				double da = (m_trolleySpeed*m_trolleySpeed)/(2*m_acctrolleySpeed);
				if( distance >= da ){
					m_travelTimer = (int)((m_trolleySpeed/m_acctrolleySpeed + distance/m_trolleySpeed - m_trolleySpeed/(2*m_acctrolleySpeed))/g_CraneSpec.unitTime);	//Time을 Clock으로 변환
				}
				else{
					m_travelTimer = (int)((sqrt(2*distance/m_acctrolleySpeed ))/g_CraneSpec.unitTime);	//Time을 Clock으로 변환
				}
				m_travelTimer *= 2;
				m_IntervalOfMovement = (int)((m_ptTrolley.y - m_ptLocation.y)/m_travelTimer);
				m_CWS = CWS_LoadedTravel;
			}			
		}
		break;

	case CWS_LoadedTravel:
		{
			m_ptTrolley.y -= m_IntervalOfMovement; //for Drawing

			--m_travelTimer;
			if( m_travelTimer <= 0 ){
				double distance = (5.6+m_ConRowLength)/2;
				double da = (m_emptyLoadSpeed*m_emptyLoadSpeed)/(2*m_accemptyLoadSpeed);
				if( distance >= da ){
					m_hoistingTimer = (int)((m_emptyLoadSpeed/m_accemptyLoadSpeed+ distance/m_emptyLoadSpeed- m_emptyLoadSpeed/(2*m_accemptyLoadSpeed))/g_CraneSpec.unitTime);	//Time을 Clock으로 변환
				}
				else{
					m_hoistingTimer = (int)((sqrt(2*distance/m_accemptyLoadSpeed))/g_CraneSpec.unitTime);	//Time을 Clock으로 변환
				}
				da = (m_fullLoadSpeed*m_fullLoadSpeed)/(2*m_accfullLoadSpeed);
				if( distance >= da ){
					m_hoistingTimer += (int)((m_fullLoadSpeed/m_accfullLoadSpeed+ distance/m_fullLoadSpeed- m_fullLoadSpeed/(2*m_accfullLoadSpeed))/g_CraneSpec.unitTime);	//Time을 Clock으로 변환
				}
				else{
					m_hoistingTimer += (int)((sqrt(2*distance/m_accfullLoadSpeed))/g_CraneSpec.unitTime);	//Time을 Clock으로 변환
				}
				m_hoistingTimer *= 2;
				//선박과의 작업 시 오차가 발생할 수 있다.
				m_hoistingTimer += abs((int)(115 - g_CraneSpec.normalRandom( 115, 13.7 )));
				m_CWS = CWS_Dropoff;
			}
		}
		break;
	case CWS_Dropoff:
		{
			--m_hoistingTimer;
			if( m_hoistingTimer <= 0 ){
				//트롤리 이동 시간 계산
				if( m_CargoType == CargoType_DoubleCycle ){
					m_travelTimer = 0;
				}
				else{
					double distance = (52 + m_ConColLength)/2;
					double da = (m_trolleySpeed*m_trolleySpeed)/(2*m_acctrolleySpeed);
					if( distance >= da ){
						m_travelTimer = (int)((m_trolleySpeed/m_acctrolleySpeed + distance/m_trolleySpeed - m_trolleySpeed/(2*m_acctrolleySpeed))/g_CraneSpec.unitTime);	//Time을 Clock으로 변환
					}
					else{
						m_travelTimer = (int)((sqrt(2*distance/m_acctrolleySpeed ))/g_CraneSpec.unitTime);	//Time을 Clock으로 변환
					}
					m_travelTimer *= 2;
				}				
				m_CWS = CWS_EmptyTravel;				
			}
		}
		break;
	case CWS_EmptyTravel://백리치 시작지점까지 이동
		{
			m_ptTrolley.y += m_IntervalOfMovement; //for Drawing

			--m_travelTimer;
			if( m_travelTimer <= 0 ){
				if( m_worksDone == false ){
					++m_iThroughput;								//QC 처리량 갱신				
				}				
				//적하 작업 완료를 PM에게 알림
				DeclareProcessFinished(m_workingHP);				
				m_CWS = CWS_Idle;
			}		
		}
		break;
	default:
		break;
	}
}

void CQC::DoDischarge(){
	
	switch( m_CWS)
	{
	case CWS_Idle:	
		{
			m_ptTrolley.y = m_ptLocation.y + g_CraneSpec.QCfrontReach + g_CraneSpec.QClegGap + 1000; //for Drawing
			
			//컨테이너의 위치를 가져온다.
			m_ConRowLength = (int)((double)m_pProcess->shipLoc.height * 2.39);
			m_ConColLength = (int)((double)m_pProcess->shipLoc.slot * 2.35);

			//트롤리 이동 시간 계산
			if( m_CargoType == CargoType_DoubleCycle ){
				m_travelTimer = 0;					 
			}
			else{
				double distance = (52 + m_ConColLength)/2;
				double da = (m_trolleySpeed*m_trolleySpeed)/(2*m_acctrolleySpeed);
				if( distance >= da ){
					m_travelTimer = (int)((m_trolleySpeed/m_acctrolleySpeed + distance/m_trolleySpeed - m_trolleySpeed/(2*m_acctrolleySpeed))/g_CraneSpec.unitTime);	//Time을 Clock으로 변환
				}
				else{
					m_travelTimer = (int)((sqrt(2*distance/m_acctrolleySpeed ))/g_CraneSpec.unitTime);	//Time을 Clock으로 변환
				}
				m_travelTimer *= 2;
			}			
			m_CWS = CWS_EmptyTravel;
		}		
		break;
	case CWS_EmptyTravel:
		{
			m_ptTrolley.y -= m_IntervalOfMovement; //for Drawing

			--m_travelTimer;
			if( m_travelTimer <= 0 ){
				//컨테이너를 들어올리는데 걸리는 시간
				double distance = (5.6+m_ConRowLength)/2;
				double da = (m_emptyLoadSpeed*m_emptyLoadSpeed)/(2*m_accemptyLoadSpeed);
				if( distance >= da ){
					m_hoistingTimer = (int)((m_emptyLoadSpeed/m_accemptyLoadSpeed+ distance/m_emptyLoadSpeed- m_emptyLoadSpeed/(2*m_accemptyLoadSpeed))/g_CraneSpec.unitTime);	//Time을 Clock으로 변환
				}
				else{
					m_hoistingTimer = (int)((sqrt(2*distance/m_accemptyLoadSpeed))/g_CraneSpec.unitTime);	//Time을 Clock으로 변환
				}
				da = (m_fullLoadSpeed*m_fullLoadSpeed)/(2*m_accfullLoadSpeed);
				if( distance >= da ){
					m_hoistingTimer += (int)((m_fullLoadSpeed/m_accfullLoadSpeed+ distance/m_fullLoadSpeed- m_fullLoadSpeed/(2*m_accfullLoadSpeed))/g_CraneSpec.unitTime);	//Time을 Clock으로 변환
				}
				else{
					m_hoistingTimer += (int)((sqrt(2*distance/m_accfullLoadSpeed))/g_CraneSpec.unitTime);	//Time을 Clock으로 변환
				}
				m_hoistingTimer *= 2;

				//선박과의 작업 시 오차가 발생할 수 있다.
				m_hoistingTimer += abs((int)(115- g_CraneSpec.normalRandom( 115, 13.7 )));
				m_CWS = CWS_Pickup;
			}			
		}
		break;	
	case CWS_Pickup:
		{
			--m_hoistingTimer;
			if( m_hoistingTimer <= 0 ){
				//트롤리 이동 시간 계산
				double distance = (52 + m_ConColLength)/2;
				double da = (m_trolleySpeed*m_trolleySpeed)/(2*m_acctrolleySpeed);
				if( distance >= da ){
					m_travelTimer = (int)((m_trolleySpeed/m_acctrolleySpeed + distance/m_trolleySpeed - m_trolleySpeed/(2*m_acctrolleySpeed))/g_CraneSpec.unitTime);	//Time을 Clock으로 변환
				}
				else{
					m_travelTimer = (int)((sqrt(2*distance/m_acctrolleySpeed ))/g_CraneSpec.unitTime);	//Time을 Clock으로 변환
				}
				m_travelTimer *= 2;
				//Idle 상태에서 트롤리 위치에서 TP 앞까지의 거리 = QC의 frontreach 길이 + QC legGap + ???
				m_CWS = CWS_LoadedTravel;
			}
		}
		break;

	case CWS_LoadedTravel:
		{
			m_ptTrolley.y += m_IntervalOfMovement; //for Drawing

			--m_travelTimer;
			if( m_travelTimer <= 0 ){
				m_CWS = CWS_WaitingforDropoff;
			}			
		}
		break;

	case CWS_WaitingforDropoff:
		{
			m_ptTrolley.y = m_ptLocation.y + g_CraneSpec.QCfrontReach + g_CraneSpec.QClegGap + 1000; //for Drawing
			
			m_workingHP = GetHPofDischargingProcess(m_pProcess);
			
			if(m_workingHP == -1 ){
				if( m_worksDone == false ){
					++m_delayTime;//traveling이 끝났는데 hoisting을 시작하지 못한다면 -> QC 지연
				}
			}
			else{
				if( m_delayTime > 0 ){
					SetDelayTime( (long)(m_delayTime*g_SimulationSpec.unitTime) );
					m_idleTime += m_delayTime;
					m_delayTime = 0;
				}				
				HPReservationofProcess( m_workingHP );
				m_CWS = CWS_TravelforDropoff;
			}			
		}
		break;

	case CWS_TravelforDropoff:
		{
			m_ptTrolley.y += m_IntervalOfMovement; //for Drawing

			if( m_ptTrolley.y >= m_HPs[m_workingHP].GetPtCenter().y ){
				//컨테이너를 내리고 다시 들어올리는데 걸리는 시간
				double distance = (26.6)/2;
				double da = (m_emptyLoadSpeed*m_emptyLoadSpeed)/(2*m_accemptyLoadSpeed);
				if( distance >= da ){
					m_hoistingTimer = (int)((m_emptyLoadSpeed/m_accemptyLoadSpeed+ distance/m_emptyLoadSpeed- m_emptyLoadSpeed/(2*m_accemptyLoadSpeed))/g_CraneSpec.unitTime);	//Time을 Clock으로 변환
				}
				else{
					m_hoistingTimer = (int)((sqrt(2*distance/m_accemptyLoadSpeed))/g_CraneSpec.unitTime);	//Time을 Clock으로 변환
				}
				da = (m_fullLoadSpeed*m_fullLoadSpeed)/(2*m_accfullLoadSpeed);
				if( distance >= da ){
					m_hoistingTimer += (int)((m_fullLoadSpeed/m_accfullLoadSpeed+ distance/m_fullLoadSpeed- m_fullLoadSpeed/(2*m_accfullLoadSpeed))/g_CraneSpec.unitTime);	//Time을 Clock으로 변환
				}
				else{
					m_hoistingTimer += (int)((sqrt(2*distance/m_accfullLoadSpeed))/g_CraneSpec.unitTime);	//Time을 Clock으로 변환
				}
				m_hoistingTimer *= 2;
				m_CWS = CWS_Dropoff;
			}
		}
		break;

	case CWS_Dropoff:
		{
			m_ptTrolley.y = m_HPs[m_workingHP].GetPtCenter().y;//for Drawing

			--m_hoistingTimer;
			if( m_hoistingTimer <= 0 ){
				if( m_worksDone == false ){
					++m_iThroughput;						//QC 처리량 갱신
				}

				//프로세스 정보 및 HP 컨테이너 업데이트
				DecideDeadlineofDischarge();
				if( m_CargoType == CargoType_DoubleCycle ){
					RecalculateDeadlineOfLoadingProcess();
				}
				SetContainerofProcess( m_workingHP );
				HPReleaseofProcess( m_workingHP );
				DeclareProcessFinished( m_workingHP );
				m_CWS = CWS_Idle;
			}			
		}
		break;

	
	default:
		break;
	}
}


void CQC::DecideDeadlineofDischarge()
{
	//Deadline을 계산하고 HP에 내려놓을 것.
	long deadline;
	int remainHP = CheckRemainHP();
	//deadline = (int)(m_clockTick* g_CraneSpec.unitTime)+remainHP*g_CraneSpec.halfCycletime*2;//초로 환산
	deadline = (int)(m_clockTick* g_CraneSpec.unitTime);//초로 환산
	

	/*
	if( m_CargoType == CargoType_Discharge ){
		int remainHP = CheckRemainHP();
		deadline = (int)(m_clockTick* g_CraneSpec.unitTime)+g_CraneSpec.halfCycletime*2;//초로 환산
		deadline *= remainHP;
	}
	else if( m_CargoType == CargoType_DoubleCycle ){
		//HP에 놓여있는 적하 컨테이너 목록을 가져온다.
		vector<string> setHP;
		int i;
		for( i = 0; i < g_CraneSpec.nHPperQC; ++i ){
			if( m_HPs[i].GetnContainer() > 0 ){
				string prsID;
				prsID = m_HPs[i].GetPrsIDofContainer(0);
				if( prsID != "" ){
					if( prsID.substr(0,1) == "L" ){
						setHP.push_back( prsID );
					}					
				}
				prsID = m_HPs[i].GetPrsIDofContainer(1);
				if( prsID != "" ){	
					if( prsID.substr(0,1) == "L" ){
						setHP.push_back( prsID );
					}
				}
			}
		}

		//현재 QC의 작업계획의 일부를 가져온다.
		int nCount = 0;
		vector<string> qcSchedule;

		int nSet = 1;//QC가 한 번에 처리하는 컨테이너 수

		//int maxCount = 2*(g_SimulationSpec.nHPperQC-1);//이상적인 경우 스케쥴 상 처리 가능한 작업 수
		//방금 처리한 양하 작업이 놓인 HP를 제외한 나머지 모든 HP에 QC 스케쥴에 따른 적하 작업이 놓여있다고 가정하면
		//각 HP 마다 더블 사이클링이 가능하므로 2*(g_SimulationSpec.nHPperQC-1) 이 된다.
		//현재 양적하 데드락을 피하기 위해 HP의 반은 양하 작업에 반은 적하 작업에 사용하고 있으므로 식을 아래와 같이 값을 직접 지정 (HP 4개 기준)
		int maxCount = 3;//이상적인 경우 스케쥴 상 처리 가능한 작업 수

		if( m_SpreaderType == Spreader_Single ){
			nSet = 1;
			maxCount = 3;
		}
		else if( m_SpreaderType == Spreader_Twin ){
			if( m_BayType == BayType_20ft ){
				nSet = 2;
				maxCount = 3;
			}
			else if( m_BayType == BayType_40ft ){
				nSet = 1;
				maxCount = 3;
			}				
		}
		else if( m_SpreaderType == Spreader_TandemTwin ){
			if( m_BayType == BayType_20ft ){
				nSet = 4;
				maxCount = 1;
			}
			else if( m_BayType == BayType_40ft ){
				nSet = 2;
				maxCount = 1;
			}		
		}		

		deque<CProcess*> ::iterator indexDJob;
		deque<CProcess*> ::iterator indexLJob;

		indexLJob = m_LoadingSequence.begin();
		indexDJob = m_DischargingSequence.begin();

		for(i=0; i < nSet; ++i ){//현재 처리 중인 양하 작업은 제외
			++indexDJob;
		}
		
		while( nCount < maxCount ){
			if( indexLJob != m_LoadingSequence.end() ){
				for(i=0; i < nSet; ++i ){
					qcSchedule.push_back((*indexLJob)->prsID);					
					++indexLJob;
				}				
			}
			if( indexDJob != m_DischargingSequence.end() ){
				for(i=0; i < nSet; ++i ){
					qcSchedule.push_back((*indexDJob)->prsID);
					++indexDJob;
				}				
			}
			++nCount;
		}			

		long estimatedTime = m_pEquipmentManager->EstimateDeadline( m_StringID, nSet, setHP, qcSchedule );
		deadline = (long)(m_clockTick* g_CraneSpec.unitTime) + estimatedTime;//초로 환산
	}
	*/
	
	//deadline 갱신
	vector<pair<ERelativePosition, string>> jobList = m_pProcess->ALVJob->connectedJobList;
	if( jobList.empty() == false ){//JobList가 비어있지 않은 경우
		if( m_pProcess->ALVJob->relativePosition == ERP_Left ){//20x20ft 작업//프로세스 정보 업데이트				
			for( int i=0; i<2; ++i){
				m_DischargingSequence[i]->deadline = deadline;
				m_DischargingSequence[i]->ALVJob->deadline = deadline;
			}
		}
		else if( m_pProcess->ALVJob->relativePosition == ERP_Top){//40x40ft 작업//프로세스 정보 업데이트
			for( int i=0; i<2; ++i){
				m_DischargingSequence[i]->deadline = deadline;
				m_DischargingSequence[i]->ALVJob->deadline = deadline;
			}
		}
		else if(m_pProcess->ALVJob->relativePosition == ERP_LeftTop){//20x20x20x20ft 작업//프로세스 정보 업데이트
			for( int i=0; i<4; ++i){
				m_DischargingSequence[i]->deadline = deadline;
				m_DischargingSequence[i]->ALVJob->deadline = deadline;
			}
		}
	}
	else{	//Single-20ft 또는 Single-40ft 작업
		//프로세스 정보 업데이트
		m_pProcess->deadline = deadline;
		m_pProcess->ALVJob->deadline = deadline;
	}
	
				
	return;
}

bool CQC::CheckAdmissible( string jobID )
{
	int nSequence = 0;
	deque<CProcess*>::iterator index;
	for( index = m_LoadingSequence.begin(); index != m_LoadingSequence.end(); ++index ){
		++nSequence;
		if( jobID == (*index)->prsID ){
			break;
		}
	}

	if( m_DischargingSequence.empty() == true ){//더 이상 양하 작업이 없다면
		if( nSequence <= 8){
			return true;
		}
		return false;
	}
	else{//양적하가 혼재해있는 상황에서
		if( nSequence <= 4){
			return true;
		}
		return false;
	}
	
}

bool CQC::CheckExistOnHP( string prsID ){
	int hp;
	for( hp = 0; hp < g_CraneSpec.nHPperQC; ++hp ){
		if( m_HPs[hp].GetPrsIDofContainer(0) == prsID || m_HPs[hp].GetPrsIDofContainer(1) == prsID ){
			return true;
		}
	}
	return false;
}

int CQC::CheckRemainHP()
{
	int count = 0;
	int checkCapacity = 2;

	int hp;
	if( m_CargoType == CargoType_DoubleCycle ){
		for( hp = 0; hp < (int)g_CraneSpec.nHPperQC/2; ++hp ){
			if( m_HPs[hp].GetRemainCapacity() >= checkCapacity ){
				count += m_HPs[hp].GetRemainCapacity();
			}
		}
	}
	else{
		for( hp = 0; hp < g_CraneSpec.nHPperQC; ++hp ){
			if( m_HPs[hp].GetRemainCapacity() >= checkCapacity ){
				count += m_HPs[hp].GetRemainCapacity();
			}
		}
	}	

	return (int)(count/checkCapacity);
	/*
	int count = 0;
	int checkCapacity = 2;

	if( m_SpreaderType == Spreader_Single || m_SpreaderType == Spreader_Twin ){
		if( m_BayType == BayType_20ft && m_SpreaderType == Spreader_Twin ){
			checkCapacity = 1;
		}
	}

	int hp;
	if( m_CargoType == CargoType_DoubleCycle ){
		for( hp = 0; hp < (int)g_CraneSpec.nHPperQC/2; ++hp ){
			if( m_HPs[hp].GetRemainCapacity() >= checkCapacity ){
				count += m_HPs[hp].GetRemainCapacity();
			}
		}
	}
	else{
		for( hp = 0; hp < g_CraneSpec.nHPperQC; ++hp ){
			if( m_HPs[hp].GetRemainCapacity() >= checkCapacity ){
				count += m_HPs[hp].GetRemainCapacity();
			}
		}
	}	

	return (int)(count/checkCapacity);
	*/
}


bool CQC::CheckAdmissibleLoad( string jobID )
{
	int nSequence = 0;
	int nRemainHP;
	deque<CProcess*>::iterator index;
	
	for( index = m_LoadingSequence.begin(); index != m_LoadingSequence.end(); ++index ){
		if( CheckExistOnHP((*index)->prsID) == false ){
			++nSequence;
		}		
		if( jobID == (*index)->prsID ){
			break;
		}
	}
	nRemainHP = CheckRemainHP();//현재 HP 여유 개수
	if( nRemainHP - nSequence >= 0){
		return true;
	}
	else{
		return false;
	}
}

void CQC::DecideCurrentProcess()
{
	if( m_CargoType == CargoType_Discharge ){//양하 작업만 처리하는 경우
		m_pProcess = m_DischargingSequence[0];
	}
	else if( m_CargoType == CargoType_Load ){//적하 작업만 처리하는 경우
		m_pProcess = m_LoadingSequence[0];
	}
	else if( m_CargoType == CargoType_Mixed ){//적하 작업만 처리하는 경우
		if( m_LoadingSequence.empty() ){
			m_pProcess = m_DischargingSequence[0];
		}
		else if( m_DischargingSequence.empty() ){
			m_pProcess = m_LoadingSequence[0];
		}		
	}
	else if( m_CargoType == CargoType_DoubleCycle ){//- DoubleCycling;
		if( m_PreviousDischarge ){//양하 작업을 수행했다면 적하 작업
			m_pProcess = m_LoadingSequence[0];
			m_PreviousDischarge = false;
		}
		else{//적하 작업을 수행했다면 양하 작업
			m_pProcess = m_DischargingSequence[0];
			m_PreviousDischarge = true;
		}
	}

	return;
}

int CQC::GetThroughputPerHour()
{ 
	return (int)((double)m_iThroughput/(m_clockTick*g_CraneSpec.unitTime/3600)); 
}

bool CQC::CheckTopPriority( string jobID )
{
	int count = -1;
	bool bFind = false;
	deque<CProcess*>::iterator index;
	if( m_DischargingSequence.empty() == false ){		
		for( index = m_DischargingSequence.begin(); index != m_DischargingSequence.end(); ++index ){
			++count;
			if( jobID == (*index)->prsID ){
				bFind = true;
				break;
			}			
		}
	}
	if( bFind == false ){
		count = -1;
		if( m_LoadingSequence.empty() == false ){
			for( index = m_LoadingSequence.begin(); index != m_LoadingSequence.end(); ++index ){
				++count;
				if( jobID == (*index)->prsID ){
					bFind = true;
					break;
				}				
			}
		}
	}	
	if( bFind ){
		if( m_SpreaderType == Spreader_TandemTwin ){
			if( m_BayType == BayType_20ft ){	//이 경우 상위 네 개가 최우선순위 작업들
				if( count < 4 ){
					return true;
				}
			}
			else if(m_BayType == BayType_40ft ){//이 경우 상위 두 개가 최우선순위 작업들
				if( count < 2 ){
					return true;
				}
			}		
		}
		else{
			if( m_SpreaderType == Spreader_Twin && m_BayType == BayType_20ft ){	//이 경우 상위 두 개가 최우선순위 작업들
				if( count < 2 ){
					return true;
				}
			}
			else{//상위 한 개만 최우선순위 작업
				if( count == 0 ){
					return true;
				}
			}
		}
	}
	
	return false;

}


void CQC::GetProcessIDList(deque<string>& jobIDList)
{
	//먼저 HP에 놓여있는 양하 작업의 ID를 모은다.
	int hp;
	string prsID;
	for(hp=0;hp<g_SimulationSpec.nHPperQC;++hp){
		for( int j=0; j<2; ++j){
			prsID = m_HPs[hp].GetPrsIDofContainer(j);
			if( prsID != "" ){
				if( prsID.substr(0,1) == "D" ){
					jobIDList.push_back(prsID);
				}
			}
		}
	}
	/*
	int nCount = (int)(g_SimulationSpec.nVehicle/g_SimulationSpec.nQC);
	if( m_BayType == BayType_20ft ){
		nCount *= 2;
	}
	if( m_LoadingSequence.empty() == false ){
		deque<CProcess*>::iterator index;
		index = m_LoadingSequence.begin();
		int h;
		for( h = 0; h < nCount; ++h ){
			jobIDList.push_back((*index)->prsID);
			++index;
			if( index == m_LoadingSequence.end() ){
				break;
			}
		}		
	}
	*/

	///*	
	int nSet = 0;
	if( m_SpreaderType == Spreader_Single ){
		nSet = 1;
	}
	else if( m_SpreaderType == Spreader_Twin ){
		if( m_BayType == BayType_20ft )
			nSet = 2;
		else
			nSet = 1;
	}
	else if( m_SpreaderType == Spreader_TandemTwin ){
		if( m_BayType == BayType_20ft )
			nSet = 4;
		else
			nSet = 2;
	}

	//그리고 QC의 적하 작업 Set의 ID를 모은다.
	if( g_SimulationSpec.nVehicle < 5 ){
		if( m_CargoType == CargoType_Load ){//Only Load라면 세 Set의 ID를
			nSet *= 2;	
		}
		else if( m_CargoType == CargoType_DoubleCycle ){//Double-cycling이라면 두 Set의 ID를
			nSet *= 1;
		}
	}
	else if( g_SimulationSpec.nVehicle < 12 ){
		if( m_CargoType == CargoType_Load ){//Only Load라면 세 Set의 ID를
			nSet *= 4;	
		}
		else if( m_CargoType == CargoType_DoubleCycle ){//Double-cycling이라면 두 Set의 ID를
			nSet *= 2;
		}
	}
	else if( g_SimulationSpec.nVehicle <= 15 ){
		if( m_CargoType == CargoType_Load ){//Only Load라면 세 Set의 ID를
			nSet *= 6;	
		}
		else if( m_CargoType == CargoType_DoubleCycle ){//Double-cycling이라면 두 Set의 ID를
			nSet *= 3;
		}
	}
	else{
		if( m_CargoType == CargoType_Load ){//Only Load라면 세 Set의 ID를
			nSet *= 8;	
		}
		else if( m_CargoType == CargoType_DoubleCycle ){//Double-cycling이라면 두 Set의 ID를
			nSet *= 3;
		}
	}
	
	int nCount = 0, nAvailable = 0;
	if( m_LoadingSequence.empty() == false ){
		deque<CProcess*>::iterator index;
		index = m_LoadingSequence.begin();
		for( nCount = 0; nCount < nSet; ++nCount ){
			jobIDList.push_back((*index)->prsID);
			++index;
			if( index == m_LoadingSequence.end() ){
				break;
			}
		}		
	}
	//*/

	return;
}

bool CQC::CheckKeepingSchedule(string jobID)
{
	deque<CProcess*>::iterator index;
	for( index = m_LoadingSequence.begin(); index != m_LoadingSequence.end(); ++index ){
		if( (*index)->prsID == jobID ){
			if( index == m_LoadingSequence.begin() ){
				return true;
			}
			else{
				--index;
				if( (*index)->ALVJob->assignedEquipID != "" ){
					return true;
				}
				else{
					return false;
				}
			}
		}
	}
	//적하가 아닌 경우이다.
	return true;
}

bool CQC::CheckReservable( string hpStringID, int needCapacity)
{
	int hpID = m_HPIDList[hpStringID];
	if( m_HPs[hpID].GetRemainCapacity() >= needCapacity ){
		return true;
	}
	else{
		return false;
	}
}