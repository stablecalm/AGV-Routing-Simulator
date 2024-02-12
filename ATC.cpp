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

	//ATC�� ���ӵǾ��ִ� TP�� ��ġ�� �����Ѵ�.
	m_HPs.clear();

	m_ptTrolley = initialPos;
	m_ptSpreader = initialPos;
}

CATC::~CATC(void)
{
	m_HPs.clear();				//TP������ �����Ѵ�.
	m_HPIDList.clear();
	m_DischargingSequence.clear();
}

//drawing operation--------------------------------------------------------------------------------------------
void CATC::DrawHPs( CDC& dc, double dZoom)
{
	//ATC�� TP�� �׸���.
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

	//Block�� �׸���.
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

	//Block�� ID�� ǥ���Ѵ�.
	CString strBlockID;
	strBlockID.Format(_T("Block %d"), m_ID);
	dc.TextOut((int)((m_ptLocation.x+400)*dZoom), (int)((m_ptLocation.y+g_CraneSpec.blockLength-100)*dZoom), strBlockID);

	//QC�� ���� ó�� ���� Process ID
	if( m_pProcess != NULL ){
		strBlockID.Format( _T("Prs ID: %s"), m_pProcess->prsID.c_str() );
		dc.TextOut((int)((m_ptLocation.x+400)*dZoom), 20 + (int)((m_ptLocation.y+g_CraneSpec.blockLength-100)*dZoom), strBlockID);
	}
	else{
		strBlockID.Format( _T("Prs ID: NULL") );
		dc.TextOut((int)((m_ptLocation.x+400)*dZoom), 20 + (int)((m_ptLocation.y+g_CraneSpec.blockLength-100)*dZoom), strBlockID);
	}

	//ATC�� �׸���.
	if( m_CWS == CWS_LoadedTravel || m_CWS == CWS_WaitingforDropoff || m_CWS == CWS_Dropoff || m_CWS == CWS_Pickup ){
		//���������� �׸���.
		if( pen.CreatePen( PS_SOLID, 1, COLOR_BLACK ) && brush_container.CreateSolidBrush( COLOR_CONTAINER_HIGH ) ){
			pOldPen = dc.SelectObject( &pen );
			pOldBrush = dc.SelectObject( &brush_container);

			CPoint ptATC = m_ptTrolley;
			//�������� �κ��� �׸���.
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
	
	//Crane�� �׸���.
	if( pen.CreatePen( PS_SOLID, 2, COLOR_ATC )){
		pOldPen = dc.SelectObject( &pen );
		CPoint ptATC = m_ptTrolley;
		//ũ������ �߽ɼ��� �׸���.
		ptATC.x -= 150;
		dc.MoveTo( (int)(ptATC.x * dZoom), (int)(ptATC.y*dZoom) );
		dc.LineTo( (int)((ptATC.x+g_CraneSpec.blockWidth+300) * dZoom), (int)(ptATC.y*dZoom) );
		//ũ������ ��ٸ��� �׸���.
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
	int pairTP = tp;	//Block �տ� Row�� �ϳ� �ۿ� ���� ���, PAIR_TP�� �ڱ� �ڽ��� ����Ų��.
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
	//������ cycle-time���� ������ hoisting �۾��� �ɸ��� 20sec�� �� ����.
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
		if( m_HPs[hpID].CheckReservation() == false || m_HPs[GetPairTP(hpID)].CheckReservation() == false ){//�� �� �� TP�� ������� ��
			if( m_HPs[hpID].CheckReservation() == false ){	//ũ������ TP�� �켱������ ��ȯ�Ѵ�.
				return hpID;
			}
			else{
				return GetPairTP(hpID);
			}
		}
	}
	return -1;	//���� ������ TP�� �ϳ��� ����.
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
		if( m_HPs[hpID].CheckReservation() == false && m_HPs[GetPairTP(hpID)].CheckReservation() == false ){//���� ������ ��
			if( m_HPs[hpID].GetRemainCapacity() >= needCapacity ){	//ũ������ TP�� �켱������ ��ȯ�Ѵ�.
				return hpID;
			}
			else if( m_HPs[GetPairTP(hpID)].GetRemainCapacity() >= needCapacity ){
				return GetPairTP(hpID);
			}
		}		
	}
	return -1;	//���� �����ϰ� �����̳ʰ� �������� ���� TP�� �ϳ��� ����.
}

int	CATC::GetReservableHPfor( int needCapacity )
{
	if( needCapacity < 0 || needCapacity > 2 ){
		Assert(false, "Invalid Capacity");
	}
	int hpID;
	for( hpID = 0; hpID < g_CraneSpec.nHPperBlock * g_CraneSpec.nRowperBlock; ++hpID ){
		if( m_HPs[hpID].CheckReservation() == false && m_HPs[GetPairTP(hpID)].CheckReservation() == false ){//���� ������ ��
			int remainCapacity = m_HPs[hpID].GetRemainCapacity();
			if( remainCapacity >= needCapacity ){	//ũ������ TP�� �켱������ ��ȯ�Ѵ�.
				return hpID;
			}
		}		
	}
	return -1;	//���� �����ϰ� �����̳ʰ� �������� ���� TP�� �ϳ��� ����.
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
	if( needCapacity == 1 ){//20ft �����̳ʸ� ���� ���, ���� 20ft �����̳ʰ� �����ִ� �ͺ��� �����ؼ� HP�� �� ä���.
		for( tp = 0; tp < maxTP; ++tp ){
			if( m_HPs[tp].GetRemainCapacity() == 1 ){//20ft�� �����ִ� HP ��
				if( m_HPs[tp].CheckReservation() == false && m_HPs[GetPairTP(tp)].CheckReservation() == false ){
					workingTP = tp;
					break;
				}
			}
		}
		if( workingTP == -1 ){	//�� �������� ��� ������ HP�� ã�� ���ߴٸ�
			for( tp = 0; tp < maxTP; ++tp ){
				if( m_HPs[tp].GetRemainCapacity() == 2 ){//������ ����ִ� HP ��
					if( m_HPs[tp].CheckReservation() == false && m_HPs[GetPairTP(tp)].CheckReservation() == false ){
						workingTP = tp;
						break;
					}
				}
			}
		}
	}
	else if( needCapacity == 2 ){	//40ft �����̳ʸ� ���� ���
		for( tp = 0; tp < maxTP; ++tp ){
			if( m_HPs[tp].GetRemainCapacity() == 2 ){//������ ����ִ� HP ��
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
	return;	
}

void CATC::DoLoad()
{
	//���� �۾� - ProcessSequence ������ ��Ű�鼭 
	switch( m_CWS)
	{
	case CWS_Idle:	
		{
			m_ptTrolley.y = m_ptLocation.y;
			m_ptSpreader.x = m_HPs[m_workingHP].GetPtCenter().x;
			
			//Ȯ�� ������ �����Ͽ� ATC Cycle Time ����
			m_travelTime = (int)(EstimateTimeofSingleCycle()/g_CraneSpec.unitTime);	//Time�� Clock���� ��ȯ

			//�̵� �ҿ� �ð� ����
			m_travelTimer = (int)m_travelTime/2;
			m_CWS = CWS_EmptyTravel;			
		}		
		break;
	case CWS_EmptyTravel:
		{
			--m_travelTimer;
			m_ptTrolley.y += g_CraneSpec.ASCTravelInterval;

			if( m_travelTimer <= 0 ){
				//�̵� �ҿ� �ð� ����
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
				m_CWS = CWS_WaitingforDropoff;			//�����̳� ���� ���
			}
		}
		break;
	case CWS_WaitingforDropoff:
		{
			//���� HP�� �۾� ���۰� ���ÿ� ���� (����� ����)
			//���μ��� ���� ������Ʈ
			string hpID = m_HPs[m_workingHP].GetStringID();
			m_pProcess->yardHP = hpID;
			m_pProcess->ALVJob->YardHPID = hpID;

			//Ʈ�Ѹ� ��ġ ����
			m_ptTrolley.y = m_HPs[m_workingHP].GetPtCenter().y;
			m_hoistingTimer = (int)(g_CraneSpec.ASCUnloadingTime/g_CraneSpec.unitTime);	//Time�� Clock���� ��ȯ
			m_CWS = CWS_Dropoff;		
		}
		break;
	case CWS_Dropoff:
		{
			--m_hoistingTimer;

			if( m_hoistingTimer <= 0 ){
				//TP�� ���¸� �����Ѵ�.
				m_HPs[m_workingHP].SetContainer(m_pProcess->ALVJob->container);
				m_HPs[m_workingHP].ReleaseReservation();

				//���� �۾� �Ϸ�
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
	//���� �۾� - AGV�� ���, ������ ������� ó��
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
			m_hoistingTimer = (int)(g_CraneSpec.ASCLoadingTime/g_CraneSpec.unitTime);	//Time�� Clock���� ��ȯ
			m_CWS = CWS_Pickup;
		}
		break;
	case CWS_Pickup:
		{			
			--m_hoistingTimer;
			if( m_hoistingTimer <= 0 ){
				m_HPs[m_workingHP].ReleaseContainer(m_pProcess->ALVJob->container);
				m_HPs[m_workingHP].ReleaseReservation();

				//�۾��ҿ�ð� ����
				m_travelTime = (int)(EstimateTimeofSingleCycle()/g_CraneSpec.unitTime);	//Time�� Clock���� ��ȯ
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

				//���� �۾� �ϷḦ PM���� �˸�
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
	if( g_SimulationSpec.cargoType == CargoType_Discharge ){//���� �۾��� ó���ϴ� ���
		m_pProcess = SelectDischargingProcess();
	}
	else if( g_SimulationSpec.cargoType == CargoType_Load ){//���� �۾��� ó���ϴ� ���
		m_pProcess = SelectLoadingProcess();
	}
	else if( g_SimulationSpec.cargoType == CargoType_Mixed || g_SimulationSpec.cargoType == CargoType_DoubleCycle ){//������ �۾��� �����ִ� ���
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
			//���� �۾��� ���� �۾��� �����ִ� ��� �Ǵ� ���� �۾��� �ִ� ���
			//1. HP�� ������ 75% �̻����� Ȯ��. HP 8�� ����, 12 TEU�� ���������� ������ 75%
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

	//�ش� �۾��� ��Ȱ�� ������ ���Ͽ� HP�� �̸� �����Ѵ�.
	if(m_pProcess != NULL){
		if( m_pProcess->prsType == PrsType_Discharging ){
			//HP ����
			int workingHP = -1;
			if( m_pProcess->ALVJob->YardHPID != "" ){//���� �����̳��� HP�� ���������� �ʴٸ� PASS
				string hpID = m_pProcess->ALVJob->YardHPID;
				if( m_HPs[m_HPIDList[hpID]].CheckReservation() == false && m_HPs[GetPairTP(m_HPIDList[hpID])].CheckReservation() == false ){
					//���صǴ� AHV�� ���� ��
					workingHP = m_HPIDList[hpID];
				}
			}
			if( workingHP != -1){
				m_workingHP = workingHP;
				m_HPs[m_workingHP].SetReservation();
			}
			else{//HP������ ���ߴٸ�
				m_pProcess = NULL;
			}
		}
		else{
			//���� �۾��� ���� HP�� �̸� �����Ѵ�.
			int workingHP = DecideHPforLoadingContainer(m_pProcess);
			if( workingHP != -1 ){
				m_workingHP = workingHP;
				m_HPs[m_workingHP].SetReservation();
			}
			else{//HP������ ���ߴٸ�
				m_pProcess = NULL;
			}
		}
	}

	return;
}

CProcess* CATC::SelectDischargingProcess()
{
	CProcess* pProcess = NULL;
	//HP�� �����ִ� �����̳� �� ���� �����̳ʰ� �ִٸ� �۾� ����
	//HP�� ������ ��� �� �ִ� ���� �켱������ ���� (- �ش� �۾��� �ҿ�� �ð��� �̸� �� �� ����)
	int hp;
	for( hp = 0; hp < (g_CraneSpec.nHPperBlock * g_CraneSpec.nRowperBlock); ++hp ){
		if( m_HPs[hp].GetnContainer() == 1 ){			//A. HP �ϳ��� ������ ��� �� �ִ� �ͺ��� ó���Ѵ�.
			string prsID = m_HPs[hp].GetProcessID();
			if( prsID.substr(0,1) == "D" ){				//B. ���� �۾��� �´��� Ȯ���Ѵ�.
														//C. ���� �����ϴٸ� �ش� �۾� ó��
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
	if( pProcess == NULL ){//HP�� ������ ��� �� �ִ� ���� �۾��� ���ٸ�
		for( hp = 0; hp < (g_CraneSpec.nHPperBlock * g_CraneSpec.nRowperBlock); ++hp ){
			if( m_HPs[hp].GetnContainer() == 2 ){
				string prsID = m_HPs[hp].GetProcessID();
				if( prsID.substr(0,1) == "D" ){	//B. ���� �۾��� �´��� Ȯ���Ѵ�.
					if( m_HPs[hp].CheckReservation() == false && CheckHPReservation(GetPairTP(hp)) == false ){	//C. ���� �����ϴٸ� �ش� �۾� ó��
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
	//HP�� �����ִ� ���� �۾��� ID�� ������.
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
	//QC�� �ߺ��Ǵ� �۾��� ID�� ProcessManager���� ó��
	return;
}