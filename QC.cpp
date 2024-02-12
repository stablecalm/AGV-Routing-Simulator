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
	m_HPs.clear();				//TP������ �����Ѵ�.
	m_HPIDList.clear();
	m_DischargingSequence.clear();	//���� �۾� ����
	m_LoadingSequence.clear();		//���� �۾� ����
}

//drawing operation--------------------------------------------------------------------------------------------
void CQC::DrawHPs( CDC& dc, double dZoom)
{
	//QC�� TP�� �׸���.
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

	if( pen.CreatePen( PS_SOLID, 2, COLOR_QC ) ){		//QC�� �׸���.
		pOldPen = dc.SelectObject( &pen );
		//QC��ٸ��� �׸���
		CPoint ptQC = m_ptLocation;//left top ��ǥ
		CPoint legPos;
		legPos.x = ptQC.x;
		legPos.y = ptQC.y + (int)(g_CraneSpec.QCfrontReach) + 1000;
		//�����ٸ�
		dc.MoveTo( (int)(legPos.x * dZoom), (int)(legPos.y * dZoom) );
		dc.LineTo( (int)((legPos.x + g_CraneSpec.QClegWidth) * dZoom), (int)(legPos.y * dZoom) );
		//�����ٸ�
		legPos.y += (int)(g_CraneSpec.QClegGap);
		dc.MoveTo( (int)(legPos.x * dZoom), (int)(legPos.y * dZoom) );
		dc.LineTo( (int)((legPos.x + g_CraneSpec.QClegWidth) * dZoom), (int)(legPos.y * dZoom) );

		//QC���� �׸���
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

	//Trolley�� �����̳ʸ� �׸���.
	if( pen.CreatePen(PS_SOLID, 1, COLOR_BLACK) && brush_container.CreateSolidBrush( COLOR_CONTAINER_HIGH ) ){
		pOldPen = dc.SelectObject( &pen );
		pOldBrush = dc.SelectObject( &brush_container );

		CPoint ptTrolley = m_ptTrolley;//Trolley ��ǥ

		//Ʈ�Ѹ��� �����̳� �κ��� �׸���.
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

		CPoint ptTrolley = m_ptTrolley;//Trolley ��ǥ
		ptTrolley.x += (int)( (g_CraneSpec.QClegWidth - g_CraneSpec.QCwidth)/2.0 ) - g_CraneSpec.QCTrolleyLegGap;

		//Ʈ�Ѹ��� �߽ɼ� �׸���
		dc.MoveTo( (int)(ptTrolley.x * dZoom), (int)(ptTrolley.y* dZoom) );
		dc.LineTo( (int)((ptTrolley.x + g_CraneSpec.QCwidth + 2*g_CraneSpec.QCTrolleyLegGap) * dZoom), (int)(ptTrolley.y*dZoom) );
		//Ʈ�Ѹ��� ��ٸ� �׸���
		dc.MoveTo( (int)(ptTrolley.x * dZoom), (int)((ptTrolley.y - g_CraneSpec.Container40ftWidth) * dZoom) );
		dc.LineTo( (int)(ptTrolley.x * dZoom), (int)((ptTrolley.y + g_CraneSpec.Container40ftWidth) * dZoom) );		

		ptTrolley.x += (int)(g_CraneSpec.QCwidth + 2*g_CraneSpec.QCTrolleyLegGap);
		dc.MoveTo( (int)(ptTrolley.x * dZoom), (int)((ptTrolley.y - g_CraneSpec.Container40ftWidth) * dZoom) );
		dc.LineTo( (int)(ptTrolley.x * dZoom), (int)((ptTrolley.y + g_CraneSpec.Container40ftWidth) * dZoom) );
		dc.SelectObject(pOldPen);
		pen.DeleteObject();
	}

	//���� QC�� ���¸� ǥ���Ѵ�.
	CString str;	
	CPoint ptStr = m_ptLocation;//left top ��ǥ
	ptStr.x += g_CraneSpec.QClegWidth + 200;
	ptStr.y += g_CraneSpec.QCfrontReach + 1200;

	//QC�� Throughput ǥ��
	str.Format( _T("Throughput: %d"), m_iThroughput );
	dc.TextOut((int)(ptStr.x * dZoom), (int)((ptStr.y) * dZoom), str );

	//QC�� �� Idle Time
	str.Format( _T("Idle Time: %d"), m_idleTime );
	dc.TextOut((int)(ptStr.x * dZoom), (int)((ptStr.y+1000) * dZoom), str );

	//QC�� ���� ó�� ���� Process ID
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
	return -1;	//���� ������ TP�� �ϳ��� ����.
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
			if( m_HPs[hpID].CheckReservation() == false ){//���� ������ ��
				if( m_HPs[hpID].GetRemainCapacity() == 2 ){	//ũ������ TP�� �켱������ ��ȯ�Ѵ�.
					return hpID;
				}
			}		
		}
	}
	
	return -1;	//���� �����ϰ� �����̳ʰ� �������� ���� TP�� �ϳ��� ����.
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
			DecideCurrentProcess();	//������ �۾��� �����Ѵ�.
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
	else{//������ �۾��� ���ٸ�
		m_worksDone = true;
	}

	return;
}

int CQC::GetHPofDischargingProcess(CProcess* pProcess)
{
	int workingHP = -1;
	int startHP;
	if( m_CargoType == CargoType_DoubleCycle && !m_DischargingSequence.empty() && !m_LoadingSequence.empty()){//�� ��� HP�� ���� ���� �۾��� ���� ���� �۾��� ���
		startHP = 2;
	}
	else{
		startHP = 0;
	}

	int hpID;
	//���� �۾��� ������ Ȯ���Ѵ�.
	vector<pair<ERelativePosition, string>> jobList = pProcess->ALVJob->connectedJobList;
	if( jobList.empty() == false ){//JobList�� ������� ���� ���

		//���� ���μ����� ����� ��ġ Ȯ��
		if( pProcess->ALVJob->relativePosition == ERP_Left ){//20x20ft �۾�
			//�� ��� HP�� RemainCapacity�� 2 �̻��̾����.
			for( hpID = startHP; hpID < g_CraneSpec.nHPperQC; ++hpID ){
				if( m_HPs[hpID].GetRemainCapacity() == 2 ){
					if( m_HPs[hpID].CheckReservation() == false ){
						workingHP = hpID;
						break;
					}
				}
			}
		}
		//������ ��� ���� ���μ����� �����ϰ��� �ϴ� HP Pair�� (0,1) �Ǵ� (2,3). ��� RemainCapacity�� 2 �̻��̾����.
		else if( pProcess->ALVJob->relativePosition == ERP_Top){//40x40ft �۾�
			for( hpID = startHP; hpID < g_CraneSpec.nHPperQC; hpID+=2 ){
				if( m_HPs[hpID].GetRemainCapacity() == 2 && m_HPs[hpID+1].GetRemainCapacity() == 2 ){
					if( m_HPs[hpID].CheckReservation() == false && m_HPs[hpID+1].CheckReservation() == false ){
						workingHP = hpID;
						break;
					}
				}
			}
		}
		else if(pProcess->ALVJob->relativePosition == ERP_LeftTop){//20x20x20x20ft �۾�
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
	else{//Single-20ft �Ǵ� Single-40ft �۾�					
		if( pProcess->ALVJob->container.conType == ConType_General20ft ){
			//Single-20ft�� ���, HP�� capacity�� 1�� ���� ã�� �� ä���ִ� �������� �������´�.
			for( hpID = startHP; hpID < g_CraneSpec.nHPperQC; ++hpID ){
				if( m_HPs[hpID].GetRemainCapacity() == 1 ){
					if( m_HPs[hpID].CheckReservation() == false ){
						workingHP = hpID;
						break;
					}
				}
			}
			if( workingHP == -1){//20ft �ϳ��� �����ִ� HP�� ��ã�Ҵٸ�
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
		else{//Single-40ft �۾�					
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
		if( jobList.empty() == false ){//JobList�� ������� ���� ���
			//���� ���μ����� ����� ��ġ Ȯ��
			if( m_HPs[hpID].GetPrsIDofContainer(0) == pProcess->prsID && m_HPs[hpID].GetPrsIDofContainer(1) == m_LoadingSequence[1]->prsID ){
				if( m_HPs[hpID].CheckReservation() == false ){			//HP�� ����Ǿ��ִ� ���°� �ƴ�
					workingHP = hpID;
				}
			}
			else if( m_HPs[hpID].GetPrsIDofContainer(1) == pProcess->prsID && m_HPs[hpID].GetPrsIDofContainer(0) == m_LoadingSequence[1]->prsID ){
				if( m_HPs[hpID].CheckReservation() == false ){			//HP�� ����Ǿ��ִ� ���°� �ƴ�
					workingHP = hpID;
				}
			}

			//���� ���μ����� ����� ��ġ Ȯ��
			if( pProcess->ALVJob->relativePosition == ERP_Left ){//20x20ft �۾�
				if( m_HPs[hpID].GetPrsIDofContainer(0) == pProcess->prsID && m_HPs[hpID].GetPrsIDofContainer(1) == m_LoadingSequence[1]->prsID ){
					if( m_HPs[hpID].CheckReservation() == false ){			//HP�� ����Ǿ��ִ� ���°� �ƴ�
						workingHP = hpID;
					}
				}
				else if( m_HPs[hpID].GetPrsIDofContainer(1) == pProcess->prsID && m_HPs[hpID].GetPrsIDofContainer(0) == m_LoadingSequence[1]->prsID ){
					if( m_HPs[hpID].CheckReservation() == false ){			//HP�� ����Ǿ��ִ� ���°� �ƴ�
						workingHP = hpID;
					}
				}
			}
			//������ ��� ���� ���μ����� �����ϰ��� �ϴ� HP Pair�� (0,1) �Ǵ� (2,3). ��� RemainCapacity�� 2 �̻��̾����.
			else if( pProcess->ALVJob->relativePosition == ERP_Top){//40x40ft �۾�
				if( m_HPs[hpID].GetPrsIDofContainer(0) == pProcess->prsID && m_HPs[hpID+1].GetPrsIDofContainer(0) == m_LoadingSequence[1]->prsID){//���� �����̳ʰ� ��� �����ߴ°�
					if( m_HPs[hpID].CheckReservation() == false && m_HPs[hpID+1].CheckReservation() == false){//HP�� ����Ǿ��ִ� ���°� �ƴ�
						workingHP = hpID;
					}
				}					
			}
			else if(pProcess->ALVJob->relativePosition == ERP_LeftTop){//20x20x20x20ft �۾�
				bool checkUpperBound = false;
				//��� �� ���� �����̳ʰ� �����ִ��� Ȯ���Ѵ�.
				if( m_HPs[hpID].GetPrsIDofContainer(0) == pProcess->prsID && m_HPs[hpID].GetPrsIDofContainer(1) == m_LoadingSequence[1]->prsID ){
					if( m_HPs[hpID].CheckReservation() == false ){			//HP�� ����Ǿ��ִ� ���°� �ƴ�
						checkUpperBound =true;
					}
				}
				else if( m_HPs[hpID].GetPrsIDofContainer(1) == pProcess->prsID && m_HPs[hpID].GetPrsIDofContainer(0) == m_LoadingSequence[1]->prsID ){
					if( m_HPs[hpID].CheckReservation() == false ){			//HP�� ����Ǿ��ִ� ���°� �ƴ�
						checkUpperBound =true;
					}
				}
				//�ϴ� �� ���� �����̳ʰ� �����ִ��� Ȯ���Ѵ�.
				if( checkUpperBound ){
					if( m_HPs[hpID+1].GetPrsIDofContainer(0) == m_LoadingSequence[2]->prsID && m_HPs[hpID+1].GetPrsIDofContainer(1) == m_LoadingSequence[3]->prsID ){
						if( m_HPs[hpID+1].CheckReservation() == false ){			//HP�� ����Ǿ��ִ� ���°� �ƴ�
							workingHP = hpID;
						}
					}
					else if( m_HPs[hpID+1].GetPrsIDofContainer(1) == m_LoadingSequence[3]->prsID && m_HPs[hpID+1].GetPrsIDofContainer(0) == m_LoadingSequence[2]->prsID ){
						if( m_HPs[hpID+1].CheckReservation() == false ){			//HP�� ����Ǿ��ִ� ���°� �ƴ�
							workingHP = hpID;
						}
					}					
				}
			}

		}
		//Single-20ft �Ǵ� Single-40ft �۾�					
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

	if( jobList.empty() == false ){//JobList�� ������� ���� ���

		if( m_pProcess->ALVJob->relativePosition == ERP_Left ){//20x20ft �۾�
			m_HPs[hpID].SetReservation();			
		}
		else if( m_pProcess->ALVJob->relativePosition == ERP_Top){//40x40ft �۾�
			m_HPs[hpID].SetReservation();
			m_HPs[hpID+1].SetReservation();
		}
		else if(m_pProcess->ALVJob->relativePosition == ERP_LeftTop){//20x20x20x20ft �۾�
			m_HPs[hpID].SetReservation();
			m_HPs[hpID+1].SetReservation();
		}
	}
	else{	//Single-20ft �Ǵ� Single-40ft �۾�					
		m_HPs[hpID].SetReservation();
	}				
	return;
}

void CQC::HPReleaseofProcess( int hpID )
{
	vector<pair<ERelativePosition, string>> jobList = m_pProcess->ALVJob->connectedJobList;

	if( jobList.empty() == false ){//JobList�� ������� ���� ���

		if( m_pProcess->ALVJob->relativePosition == ERP_Left ){//20x20ft �۾�
			m_HPs[hpID].ReleaseReservation();			
		}
		else if( m_pProcess->ALVJob->relativePosition == ERP_Top){//40x40ft �۾�
			m_HPs[hpID].ReleaseReservation();
			m_HPs[hpID+1].ReleaseReservation();
		}
		else if(m_pProcess->ALVJob->relativePosition == ERP_LeftTop){//20x20x20x20ft �۾�
			m_HPs[hpID].ReleaseReservation();
			m_HPs[hpID+1].ReleaseReservation();
		}
	}
	else{	//Single-20ft �Ǵ� Single-40ft �۾�					
		m_HPs[hpID].ReleaseReservation();
	}				
	return;
}

void CQC::SetDelayTime(int dTime)
{
	if( m_pProcess->prsType == PrsType_Discharging  ){
		vector<pair<ERelativePosition, string>> jobList = m_pProcess->ALVJob->connectedJobList;

		if( jobList.empty() == false ){//JobList�� ������� ���� ���
			//����� ��� �۾��� delay�� �����ϴ�.
			if( m_pProcess->ALVJob->relativePosition == ERP_Left ){//20x20ft �۾�
				m_pProcess->ALVJob->delayTime = dTime;
				m_DischargingSequence[1]->ALVJob->delayTime = dTime;
			}
			else if( m_pProcess->ALVJob->relativePosition == ERP_Top){//40x40ft �۾�
				m_pProcess->ALVJob->delayTime = dTime;
				m_DischargingSequence[1]->ALVJob->delayTime = dTime;
			}
			else if(m_pProcess->ALVJob->relativePosition == ERP_LeftTop){//20x20x20x20ft �۾�
				m_pProcess->ALVJob->delayTime = dTime;
				m_DischargingSequence[1]->ALVJob->delayTime = dTime;
				m_DischargingSequence[2]->ALVJob->delayTime = dTime;
				m_DischargingSequence[3]->ALVJob->delayTime = dTime;
			}
		}
		else{	//Single-20ft �Ǵ� Single-40ft �۾�
			m_pProcess->ALVJob->delayTime = dTime;
		}
	}
	else if( m_pProcess->prsType == PrsType_Loading ){
		vector<pair<ERelativePosition, string>> jobList = m_pProcess->ALVJob->connectedJobList;
		if( m_pProcess->ALVJob->YardHPID == "" ){
			if( jobList.empty() == false ){//JobList�� ������� ���� ���
				//�������� ���� �۾��� delay�� �����Ѵ�.
				if( m_pProcess->ALVJob->relativePosition == ERP_Left ){//20x20ft �۾�
					m_pProcess->ALVJob->delayTime = dTime;
					m_LoadingSequence[1]->ALVJob->delayTime = dTime;
				}
				//������ ��� ���� ���μ����� �����ϰ��� �ϴ� HP Pair�� (0,1) �Ǵ� (2,3). ��� RemainCapacity�� 2 �̻��̾����.
				else if( m_pProcess->ALVJob->relativePosition == ERP_Top){//40x40ft �۾�
					m_pProcess->ALVJob->delayTime = dTime;
					m_LoadingSequence[1]->ALVJob->delayTime = dTime;
				}
				else if(m_pProcess->ALVJob->relativePosition == ERP_LeftTop){//20x20x20x20ft �۾�
					m_pProcess->ALVJob->delayTime = dTime;
					m_LoadingSequence[1]->ALVJob->delayTime = dTime;
					m_LoadingSequence[2]->ALVJob->delayTime = dTime;
					m_LoadingSequence[3]->ALVJob->delayTime = dTime;
				}
			}
			//Single-20ft �Ǵ� Single-40ft �۾�					
			else{
				m_pProcess->ALVJob->delayTime = dTime;
			}		
		}
		else{
			if( jobList.empty() == false ){//JobList�� ������� ���� ���
				//�������� ���� �۾��� delay�� �����Ѵ�.
				if( m_pProcess->ALVJob->relativePosition == ERP_Left ){//20x20ft �۾�
					if( m_pProcess->prsID != m_HPs[m_HPIDList[m_pProcess->ALVJob->YardHPID]].GetPrsIDofContainer(0) && m_pProcess->prsID != m_HPs[m_HPIDList[m_pProcess->ALVJob->YardHPID]].GetPrsIDofContainer(1) ){
						m_pProcess->ALVJob->delayTime = dTime;
					}
					if( m_LoadingSequence[1]->prsID != m_HPs[m_HPIDList[m_pProcess->ALVJob->YardHPID]].GetPrsIDofContainer(0) && m_LoadingSequence[1]->prsID != m_HPs[m_HPIDList[m_pProcess->ALVJob->YardHPID]].GetPrsIDofContainer(1) ){
						m_LoadingSequence[1]->ALVJob->delayTime = dTime;
					}
				}
				//������ ��� ���� ���μ����� �����ϰ��� �ϴ� HP Pair�� (0,1) �Ǵ� (2,3). ��� RemainCapacity�� 2 �̻��̾����.
				else if( m_pProcess->ALVJob->relativePosition == ERP_Top){//40x40ft �۾�
					if( m_pProcess->prsID != m_HPs[m_HPIDList[m_pProcess->ALVJob->YardHPID]].GetPrsIDofContainer(0) ){
						m_pProcess->ALVJob->delayTime = dTime;
					}
					if( m_LoadingSequence[1]->prsID != m_HPs[m_HPIDList[m_pProcess->ALVJob->YardHPID]+1].GetPrsIDofContainer(0) ){
						m_LoadingSequence[1]->ALVJob->delayTime = dTime;
					}	
				}
				else if(m_pProcess->ALVJob->relativePosition == ERP_LeftTop){//20x20x20x20ft �۾�
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
			//Single-20ft �Ǵ� Single-40ft �۾�					
			else{
				m_pProcess->ALVJob->delayTime = dTime;
			}
		}

		
						
	}
	return;
}

void CQC::SetContainerofProcess( int hpID )
{
	//����
	vector<pair<ERelativePosition, string>> jobList = m_pProcess->ALVJob->connectedJobList;

	if( jobList.empty() == false ){//JobList�� ������� ���� ���

		if( m_pProcess->ALVJob->relativePosition == ERP_Left ){//20x20ft �۾�
			m_HPs[hpID].SetContainer(m_pProcess->ALVJob->container);
			m_HPs[hpID].SetContainer(m_DischargingSequence[1]->ALVJob->container);
		}
		else if( m_pProcess->ALVJob->relativePosition == ERP_Top){//40x40ft �۾�
			m_HPs[hpID].SetContainer(m_pProcess->ALVJob->container);
			m_HPs[hpID+1].SetContainer(m_DischargingSequence[1]->ALVJob->container);
		}
		else if(m_pProcess->ALVJob->relativePosition == ERP_LeftTop){//20x20x20x20ft �۾�
			m_HPs[hpID].SetContainer(m_pProcess->ALVJob->container);
			m_HPs[hpID].SetContainer(m_DischargingSequence[1]->ALVJob->container);
			m_HPs[hpID+1].SetContainer(m_DischargingSequence[2]->ALVJob->container);
			m_HPs[hpID+1].SetContainer(m_DischargingSequence[3]->ALVJob->container);
		}
	}
	else{	//Single-20ft �Ǵ� Single-40ft �۾�					
		m_HPs[hpID].SetContainer(m_pProcess->ALVJob->container);
	}				
	return;
}

void CQC::ReleaseContainerofProcess( int hpID )
{
	//����
	vector<pair<ERelativePosition, string>> jobList = m_pProcess->ALVJob->connectedJobList;

	if( jobList.empty() == false ){//JobList�� ������� ���� ���

		if( m_pProcess->ALVJob->relativePosition == ERP_Left ){//20x20ft �۾�
			m_HPs[hpID].ReleaseContainer(m_pProcess->ALVJob->container);
			m_HPs[hpID].ReleaseContainer(m_LoadingSequence[1]->ALVJob->container);
		}
		else if( m_pProcess->ALVJob->relativePosition == ERP_Top){//40x40ft �۾�
			m_HPs[hpID].ReleaseContainer(m_pProcess->ALVJob->container);
			m_HPs[hpID+1].ReleaseContainer(m_LoadingSequence[1]->ALVJob->container);
		}
		else if(m_pProcess->ALVJob->relativePosition == ERP_LeftTop){//20x20x20x20ft �۾�
			m_HPs[hpID].ReleaseContainer(m_pProcess->ALVJob->container);
			m_HPs[hpID].ReleaseContainer(m_LoadingSequence[1]->ALVJob->container);
			m_HPs[hpID+1].ReleaseContainer(m_LoadingSequence[2]->ALVJob->container);
			m_HPs[hpID+1].ReleaseContainer(m_LoadingSequence[3]->ALVJob->container);
		}
	}
	else{	//Single-20ft �Ǵ� Single-40ft �۾�					
		m_HPs[hpID].ReleaseContainer(m_pProcess->ALVJob->container);
	}
	return;
}

void CQC::DeclareProcessFinished( int hpID )
{
	if( m_pProcess->prsType == PrsType_Discharging  ){
		vector<pair<ERelativePosition, string>> jobList = m_pProcess->ALVJob->connectedJobList;

		if( jobList.empty() == false ){//JobList�� ������� ���� ���

			if( m_pProcess->ALVJob->relativePosition == ERP_Left ){//20x20ft �۾�//���μ��� ���� ������Ʈ
				int i;
				for( i=0; i<2; ++i ){
					m_DischargingSequence[0]->quayHP			= m_HPs[hpID].GetStringID();
					m_DischargingSequence[0]->ALVJob->QuayHPID	= m_HPs[hpID].GetStringID();
					m_DischargingSequence.pop_front();
				}
			}
			else if( m_pProcess->ALVJob->relativePosition == ERP_Top){//40x40ft �۾�//���μ��� ���� ������Ʈ
				m_DischargingSequence[0]->quayHP = m_HPs[hpID].GetStringID();
				m_DischargingSequence[0]->ALVJob->QuayHPID = m_HPs[hpID].GetStringID();
				m_DischargingSequence[1]->quayHP = m_HPs[hpID+1].GetStringID();
				m_DischargingSequence[1]->ALVJob->QuayHPID = m_HPs[hpID+1].GetStringID();
				m_DischargingSequence.pop_front();
				m_DischargingSequence.pop_front();
			}
			else if(m_pProcess->ALVJob->relativePosition == ERP_LeftTop){//20x20x20x20ft �۾�//���μ��� ���� ������Ʈ
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
		else{	//Single-20ft �Ǵ� Single-40ft �۾�//���μ��� ���� ������Ʈ
			m_DischargingSequence[0]->quayHP			= m_HPs[hpID].GetStringID();
			m_DischargingSequence[0]->ALVJob->QuayHPID	= m_HPs[hpID].GetStringID();
			m_DischargingSequence.pop_front();
		}
	}
	else if( m_pProcess->prsType == PrsType_Loading ){
		deque<CProcess*>::iterator index;

		vector<pair<ERelativePosition, string>> jobList = m_pProcess->ALVJob->connectedJobList;		
		if( jobList.empty() == false ){//JobList�� ������� ���� ���

			if( m_pProcess->ALVJob->relativePosition == ERP_Left ){//20x20ft �۾�
				for( int i=0; i<2; ++i ){
					m_pEquipmentManager->ProcessFinished(m_LoadingSequence[0]);
					m_LoadingSequence.pop_front();
				}
			}
			else if( m_pProcess->ALVJob->relativePosition == ERP_Top){//40x40ft �۾�
				for( int i=0; i<2; ++i ){
					m_pEquipmentManager->ProcessFinished(m_LoadingSequence[0]);
					m_LoadingSequence.pop_front();
				}
			}
			else if(m_pProcess->ALVJob->relativePosition == ERP_LeftTop){//20x20x20x20ft �۾�
				for( int i=0; i<4; ++i ){
					m_pEquipmentManager->ProcessFinished(m_LoadingSequence[0]);
					m_LoadingSequence.pop_front();
				}
			}
		}
		else{	//Single-20ft �Ǵ� Single-40ft �۾�					
			m_pEquipmentManager->ProcessFinished(m_pProcess);
			m_LoadingSequence.pop_front();
		}		
	}

	m_pProcess = NULL;

	return;
}

void CQC::RecalculateDeadlineOfLoadingProcess()
{
	int nSet = 1;//QC�� �� ���� ó���ϴ� �����̳� ��
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
			++index;//���� ó�� ���� ���μ����� ��ŵ
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

void CQC::DoLoad()	//���� �� QC�� �۾� ��û�� ���� ����
{
	switch( m_CWS)
	{
	case CWS_Idle://�۾� �½�ũ�� �����ϴ� TP ����.
		{
			m_ptTrolley.y = m_ptLocation.y + g_CraneSpec.QCfrontReach + g_CraneSpec.QClegGap + 1000; //for Drawing
			m_CWS = CWS_WaitingforPickup;
		}		
		break;
	
	case CWS_WaitingforPickup://�۾� �½�ũ�� �����ϴ� TP�� �۾� �������� ������ ���
		{
			m_ptTrolley.y = m_ptLocation.y + g_CraneSpec.QCfrontReach + g_CraneSpec.QClegGap + 1000; //for Drawing			

			m_workingHP = GetHPofLoadingProcess(m_pProcess);
			if( m_workingHP == -1 ){
				if( m_worksDone == false ){//traveling�� �����µ� hoisting�� �������� ���Ѵٸ� -> QC ����
					++m_delayTime;
					SetDelayTime((long)(m_delayTime*g_SimulationSpec.unitTime));
				}
			}
			else{
				m_idleTime += m_delayTime;
				m_delayTime = 0;
				HPReservationofProcess( m_workingHP );
				m_CWS = CWS_TravelforPickup;

				//�����̳��� ��ġ�� �����´�.
				m_ConRowLength = (int)((double)m_pProcess->shipLoc.height * 2.39);
				m_ConColLength = (int)((double)m_pProcess->shipLoc.slot * 2.35);				
			}
		}
		break;

	case CWS_TravelforPickup://�۾� �½�ũ�� �����ϴ� TP���� �̵� �簳.
		{
			m_ptTrolley.y += m_IntervalOfMovement; //for Drawing

			if( m_ptTrolley.y >= m_HPs[m_workingHP].GetPtCenter().y ){
				//�����̳ʸ� ������ �ٽ� ���ø��µ� �ɸ��� �ð�
				double distance = (26.6)/2;
				double da = (m_emptyLoadSpeed*m_emptyLoadSpeed)/(2*m_accemptyLoadSpeed);
				if( distance >= da ){
					m_hoistingTimer = (int)((m_emptyLoadSpeed/m_accemptyLoadSpeed+ distance/m_emptyLoadSpeed- m_emptyLoadSpeed/(2*m_accemptyLoadSpeed))/g_CraneSpec.unitTime);	//Time�� Clock���� ��ȯ
				}
				else{
					m_hoistingTimer = (int)((sqrt(2*distance/m_accemptyLoadSpeed))/g_CraneSpec.unitTime);	//Time�� Clock���� ��ȯ
				}
				da = (m_fullLoadSpeed*m_fullLoadSpeed)/(2*m_accfullLoadSpeed);
				if( distance >= da ){
					m_hoistingTimer += (int)((m_fullLoadSpeed/m_accfullLoadSpeed+ distance/m_fullLoadSpeed- m_fullLoadSpeed/(2*m_accfullLoadSpeed))/g_CraneSpec.unitTime);	//Time�� Clock���� ��ȯ
				}
				else{
					m_hoistingTimer += (int)((sqrt(2*distance/m_accfullLoadSpeed))/g_CraneSpec.unitTime);	//Time�� Clock���� ��ȯ
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
				//hoisting�� ������ �۾� ���� TP�� �����̳� ����
				ReleaseContainerofProcess( m_workingHP );
				HPReleaseofProcess( m_workingHP );
				RecalculateDeadlineOfLoadingProcess();
				
				//Ʈ�Ѹ� �̵� �ð� ���
				double distance = (52 + m_ConColLength)/2;
				double da = (m_trolleySpeed*m_trolleySpeed)/(2*m_acctrolleySpeed);
				if( distance >= da ){
					m_travelTimer = (int)((m_trolleySpeed/m_acctrolleySpeed + distance/m_trolleySpeed - m_trolleySpeed/(2*m_acctrolleySpeed))/g_CraneSpec.unitTime);	//Time�� Clock���� ��ȯ
				}
				else{
					m_travelTimer = (int)((sqrt(2*distance/m_acctrolleySpeed ))/g_CraneSpec.unitTime);	//Time�� Clock���� ��ȯ
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
					m_hoistingTimer = (int)((m_emptyLoadSpeed/m_accemptyLoadSpeed+ distance/m_emptyLoadSpeed- m_emptyLoadSpeed/(2*m_accemptyLoadSpeed))/g_CraneSpec.unitTime);	//Time�� Clock���� ��ȯ
				}
				else{
					m_hoistingTimer = (int)((sqrt(2*distance/m_accemptyLoadSpeed))/g_CraneSpec.unitTime);	//Time�� Clock���� ��ȯ
				}
				da = (m_fullLoadSpeed*m_fullLoadSpeed)/(2*m_accfullLoadSpeed);
				if( distance >= da ){
					m_hoistingTimer += (int)((m_fullLoadSpeed/m_accfullLoadSpeed+ distance/m_fullLoadSpeed- m_fullLoadSpeed/(2*m_accfullLoadSpeed))/g_CraneSpec.unitTime);	//Time�� Clock���� ��ȯ
				}
				else{
					m_hoistingTimer += (int)((sqrt(2*distance/m_accfullLoadSpeed))/g_CraneSpec.unitTime);	//Time�� Clock���� ��ȯ
				}
				m_hoistingTimer *= 2;
				//���ڰ��� �۾� �� ������ �߻��� �� �ִ�.
				m_hoistingTimer += abs((int)(115 - g_CraneSpec.normalRandom( 115, 13.7 )));
				m_CWS = CWS_Dropoff;
			}
		}
		break;
	case CWS_Dropoff:
		{
			--m_hoistingTimer;
			if( m_hoistingTimer <= 0 ){
				//Ʈ�Ѹ� �̵� �ð� ���
				if( m_CargoType == CargoType_DoubleCycle ){
					m_travelTimer = 0;
				}
				else{
					double distance = (52 + m_ConColLength)/2;
					double da = (m_trolleySpeed*m_trolleySpeed)/(2*m_acctrolleySpeed);
					if( distance >= da ){
						m_travelTimer = (int)((m_trolleySpeed/m_acctrolleySpeed + distance/m_trolleySpeed - m_trolleySpeed/(2*m_acctrolleySpeed))/g_CraneSpec.unitTime);	//Time�� Clock���� ��ȯ
					}
					else{
						m_travelTimer = (int)((sqrt(2*distance/m_acctrolleySpeed ))/g_CraneSpec.unitTime);	//Time�� Clock���� ��ȯ
					}
					m_travelTimer *= 2;
				}				
				m_CWS = CWS_EmptyTravel;				
			}
		}
		break;
	case CWS_EmptyTravel://�鸮ġ ������������ �̵�
		{
			m_ptTrolley.y += m_IntervalOfMovement; //for Drawing

			--m_travelTimer;
			if( m_travelTimer <= 0 ){
				if( m_worksDone == false ){
					++m_iThroughput;								//QC ó���� ����				
				}				
				//���� �۾� �ϷḦ PM���� �˸�
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
			
			//�����̳��� ��ġ�� �����´�.
			m_ConRowLength = (int)((double)m_pProcess->shipLoc.height * 2.39);
			m_ConColLength = (int)((double)m_pProcess->shipLoc.slot * 2.35);

			//Ʈ�Ѹ� �̵� �ð� ���
			if( m_CargoType == CargoType_DoubleCycle ){
				m_travelTimer = 0;					 
			}
			else{
				double distance = (52 + m_ConColLength)/2;
				double da = (m_trolleySpeed*m_trolleySpeed)/(2*m_acctrolleySpeed);
				if( distance >= da ){
					m_travelTimer = (int)((m_trolleySpeed/m_acctrolleySpeed + distance/m_trolleySpeed - m_trolleySpeed/(2*m_acctrolleySpeed))/g_CraneSpec.unitTime);	//Time�� Clock���� ��ȯ
				}
				else{
					m_travelTimer = (int)((sqrt(2*distance/m_acctrolleySpeed ))/g_CraneSpec.unitTime);	//Time�� Clock���� ��ȯ
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
				//�����̳ʸ� ���ø��µ� �ɸ��� �ð�
				double distance = (5.6+m_ConRowLength)/2;
				double da = (m_emptyLoadSpeed*m_emptyLoadSpeed)/(2*m_accemptyLoadSpeed);
				if( distance >= da ){
					m_hoistingTimer = (int)((m_emptyLoadSpeed/m_accemptyLoadSpeed+ distance/m_emptyLoadSpeed- m_emptyLoadSpeed/(2*m_accemptyLoadSpeed))/g_CraneSpec.unitTime);	//Time�� Clock���� ��ȯ
				}
				else{
					m_hoistingTimer = (int)((sqrt(2*distance/m_accemptyLoadSpeed))/g_CraneSpec.unitTime);	//Time�� Clock���� ��ȯ
				}
				da = (m_fullLoadSpeed*m_fullLoadSpeed)/(2*m_accfullLoadSpeed);
				if( distance >= da ){
					m_hoistingTimer += (int)((m_fullLoadSpeed/m_accfullLoadSpeed+ distance/m_fullLoadSpeed- m_fullLoadSpeed/(2*m_accfullLoadSpeed))/g_CraneSpec.unitTime);	//Time�� Clock���� ��ȯ
				}
				else{
					m_hoistingTimer += (int)((sqrt(2*distance/m_accfullLoadSpeed))/g_CraneSpec.unitTime);	//Time�� Clock���� ��ȯ
				}
				m_hoistingTimer *= 2;

				//���ڰ��� �۾� �� ������ �߻��� �� �ִ�.
				m_hoistingTimer += abs((int)(115- g_CraneSpec.normalRandom( 115, 13.7 )));
				m_CWS = CWS_Pickup;
			}			
		}
		break;	
	case CWS_Pickup:
		{
			--m_hoistingTimer;
			if( m_hoistingTimer <= 0 ){
				//Ʈ�Ѹ� �̵� �ð� ���
				double distance = (52 + m_ConColLength)/2;
				double da = (m_trolleySpeed*m_trolleySpeed)/(2*m_acctrolleySpeed);
				if( distance >= da ){
					m_travelTimer = (int)((m_trolleySpeed/m_acctrolleySpeed + distance/m_trolleySpeed - m_trolleySpeed/(2*m_acctrolleySpeed))/g_CraneSpec.unitTime);	//Time�� Clock���� ��ȯ
				}
				else{
					m_travelTimer = (int)((sqrt(2*distance/m_acctrolleySpeed ))/g_CraneSpec.unitTime);	//Time�� Clock���� ��ȯ
				}
				m_travelTimer *= 2;
				//Idle ���¿��� Ʈ�Ѹ� ��ġ���� TP �ձ����� �Ÿ� = QC�� frontreach ���� + QC legGap + ???
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
					++m_delayTime;//traveling�� �����µ� hoisting�� �������� ���Ѵٸ� -> QC ����
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
				//�����̳ʸ� ������ �ٽ� ���ø��µ� �ɸ��� �ð�
				double distance = (26.6)/2;
				double da = (m_emptyLoadSpeed*m_emptyLoadSpeed)/(2*m_accemptyLoadSpeed);
				if( distance >= da ){
					m_hoistingTimer = (int)((m_emptyLoadSpeed/m_accemptyLoadSpeed+ distance/m_emptyLoadSpeed- m_emptyLoadSpeed/(2*m_accemptyLoadSpeed))/g_CraneSpec.unitTime);	//Time�� Clock���� ��ȯ
				}
				else{
					m_hoistingTimer = (int)((sqrt(2*distance/m_accemptyLoadSpeed))/g_CraneSpec.unitTime);	//Time�� Clock���� ��ȯ
				}
				da = (m_fullLoadSpeed*m_fullLoadSpeed)/(2*m_accfullLoadSpeed);
				if( distance >= da ){
					m_hoistingTimer += (int)((m_fullLoadSpeed/m_accfullLoadSpeed+ distance/m_fullLoadSpeed- m_fullLoadSpeed/(2*m_accfullLoadSpeed))/g_CraneSpec.unitTime);	//Time�� Clock���� ��ȯ
				}
				else{
					m_hoistingTimer += (int)((sqrt(2*distance/m_accfullLoadSpeed))/g_CraneSpec.unitTime);	//Time�� Clock���� ��ȯ
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
					++m_iThroughput;						//QC ó���� ����
				}

				//���μ��� ���� �� HP �����̳� ������Ʈ
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
	//Deadline�� ����ϰ� HP�� �������� ��.
	long deadline;
	int remainHP = CheckRemainHP();
	//deadline = (int)(m_clockTick* g_CraneSpec.unitTime)+remainHP*g_CraneSpec.halfCycletime*2;//�ʷ� ȯ��
	deadline = (int)(m_clockTick* g_CraneSpec.unitTime);//�ʷ� ȯ��
	

	/*
	if( m_CargoType == CargoType_Discharge ){
		int remainHP = CheckRemainHP();
		deadline = (int)(m_clockTick* g_CraneSpec.unitTime)+g_CraneSpec.halfCycletime*2;//�ʷ� ȯ��
		deadline *= remainHP;
	}
	else if( m_CargoType == CargoType_DoubleCycle ){
		//HP�� �����ִ� ���� �����̳� ����� �����´�.
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

		//���� QC�� �۾���ȹ�� �Ϻθ� �����´�.
		int nCount = 0;
		vector<string> qcSchedule;

		int nSet = 1;//QC�� �� ���� ó���ϴ� �����̳� ��

		//int maxCount = 2*(g_SimulationSpec.nHPperQC-1);//�̻����� ��� ������ �� ó�� ������ �۾� ��
		//��� ó���� ���� �۾��� ���� HP�� ������ ������ ��� HP�� QC �����쿡 ���� ���� �۾��� �����ִٰ� �����ϸ�
		//�� HP ���� ���� ����Ŭ���� �����ϹǷ� 2*(g_SimulationSpec.nHPperQC-1) �� �ȴ�.
		//���� ������ ������� ���ϱ� ���� HP�� ���� ���� �۾��� ���� ���� �۾��� ����ϰ� �����Ƿ� ���� �Ʒ��� ���� ���� ���� ���� (HP 4�� ����)
		int maxCount = 3;//�̻����� ��� ������ �� ó�� ������ �۾� ��

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

		for(i=0; i < nSet; ++i ){//���� ó�� ���� ���� �۾��� ����
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
		deadline = (long)(m_clockTick* g_CraneSpec.unitTime) + estimatedTime;//�ʷ� ȯ��
	}
	*/
	
	//deadline ����
	vector<pair<ERelativePosition, string>> jobList = m_pProcess->ALVJob->connectedJobList;
	if( jobList.empty() == false ){//JobList�� ������� ���� ���
		if( m_pProcess->ALVJob->relativePosition == ERP_Left ){//20x20ft �۾�//���μ��� ���� ������Ʈ				
			for( int i=0; i<2; ++i){
				m_DischargingSequence[i]->deadline = deadline;
				m_DischargingSequence[i]->ALVJob->deadline = deadline;
			}
		}
		else if( m_pProcess->ALVJob->relativePosition == ERP_Top){//40x40ft �۾�//���μ��� ���� ������Ʈ
			for( int i=0; i<2; ++i){
				m_DischargingSequence[i]->deadline = deadline;
				m_DischargingSequence[i]->ALVJob->deadline = deadline;
			}
		}
		else if(m_pProcess->ALVJob->relativePosition == ERP_LeftTop){//20x20x20x20ft �۾�//���μ��� ���� ������Ʈ
			for( int i=0; i<4; ++i){
				m_DischargingSequence[i]->deadline = deadline;
				m_DischargingSequence[i]->ALVJob->deadline = deadline;
			}
		}
	}
	else{	//Single-20ft �Ǵ� Single-40ft �۾�
		//���μ��� ���� ������Ʈ
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

	if( m_DischargingSequence.empty() == true ){//�� �̻� ���� �۾��� ���ٸ�
		if( nSequence <= 8){
			return true;
		}
		return false;
	}
	else{//�����ϰ� ȥ�����ִ� ��Ȳ����
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
	nRemainHP = CheckRemainHP();//���� HP ���� ����
	if( nRemainHP - nSequence >= 0){
		return true;
	}
	else{
		return false;
	}
}

void CQC::DecideCurrentProcess()
{
	if( m_CargoType == CargoType_Discharge ){//���� �۾��� ó���ϴ� ���
		m_pProcess = m_DischargingSequence[0];
	}
	else if( m_CargoType == CargoType_Load ){//���� �۾��� ó���ϴ� ���
		m_pProcess = m_LoadingSequence[0];
	}
	else if( m_CargoType == CargoType_Mixed ){//���� �۾��� ó���ϴ� ���
		if( m_LoadingSequence.empty() ){
			m_pProcess = m_DischargingSequence[0];
		}
		else if( m_DischargingSequence.empty() ){
			m_pProcess = m_LoadingSequence[0];
		}		
	}
	else if( m_CargoType == CargoType_DoubleCycle ){//- DoubleCycling;
		if( m_PreviousDischarge ){//���� �۾��� �����ߴٸ� ���� �۾�
			m_pProcess = m_LoadingSequence[0];
			m_PreviousDischarge = false;
		}
		else{//���� �۾��� �����ߴٸ� ���� �۾�
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
			if( m_BayType == BayType_20ft ){	//�� ��� ���� �� ���� �ֿ켱���� �۾���
				if( count < 4 ){
					return true;
				}
			}
			else if(m_BayType == BayType_40ft ){//�� ��� ���� �� ���� �ֿ켱���� �۾���
				if( count < 2 ){
					return true;
				}
			}		
		}
		else{
			if( m_SpreaderType == Spreader_Twin && m_BayType == BayType_20ft ){	//�� ��� ���� �� ���� �ֿ켱���� �۾���
				if( count < 2 ){
					return true;
				}
			}
			else{//���� �� ���� �ֿ켱���� �۾�
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
	//���� HP�� �����ִ� ���� �۾��� ID�� ������.
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

	//�׸��� QC�� ���� �۾� Set�� ID�� ������.
	if( g_SimulationSpec.nVehicle < 5 ){
		if( m_CargoType == CargoType_Load ){//Only Load��� �� Set�� ID��
			nSet *= 2;	
		}
		else if( m_CargoType == CargoType_DoubleCycle ){//Double-cycling�̶�� �� Set�� ID��
			nSet *= 1;
		}
	}
	else if( g_SimulationSpec.nVehicle < 12 ){
		if( m_CargoType == CargoType_Load ){//Only Load��� �� Set�� ID��
			nSet *= 4;	
		}
		else if( m_CargoType == CargoType_DoubleCycle ){//Double-cycling�̶�� �� Set�� ID��
			nSet *= 2;
		}
	}
	else if( g_SimulationSpec.nVehicle <= 15 ){
		if( m_CargoType == CargoType_Load ){//Only Load��� �� Set�� ID��
			nSet *= 6;	
		}
		else if( m_CargoType == CargoType_DoubleCycle ){//Double-cycling�̶�� �� Set�� ID��
			nSet *= 3;
		}
	}
	else{
		if( m_CargoType == CargoType_Load ){//Only Load��� �� Set�� ID��
			nSet *= 8;	
		}
		else if( m_CargoType == CargoType_DoubleCycle ){//Double-cycling�̶�� �� Set�� ID��
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
	//���ϰ� �ƴ� ����̴�.
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