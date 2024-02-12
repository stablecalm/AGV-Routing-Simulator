// iTLC_AHVView.cpp : CiTLC_AHVView Ŭ������ ����
//

#include "stdafx.h"
#include "iTLC_AHV.h"

#include "iTLC_AHVDoc.h"
#include "iTLC_AHVView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//DEFINE RGB COLOR

// CiTLC_AHVView

IMPLEMENT_DYNCREATE(CiTLC_AHVView, CScrollView)

BEGIN_MESSAGE_MAP(CiTLC_AHVView, CScrollView)
	// ǥ�� �μ� ����Դϴ�.
	ON_COMMAND(ID_FILE_PRINT, CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CScrollView::OnFilePrintPreview)
	ON_COMMAND(ID_SIMSTART, &CiTLC_AHVView::OnSimstart)
	ON_COMMAND(ID_SIMPAUSE, &CiTLC_AHVView::OnSimpause)
	ON_WM_TIMER()
	ON_COMMAND(ID_VIEW_ROUTE, &CiTLC_AHVView::OnViewRoute)
END_MESSAGE_MAP()

// CiTLC_AHVView ����/�Ҹ�

CiTLC_AHVView::CiTLC_AHVView()
:m_iTimerID(0), m_bDrawRoute(false)
{
	// TODO: ���⿡ ���� �ڵ带 �߰��մϴ�.
	
}

CiTLC_AHVView::~CiTLC_AHVView()
{
	m_pEquipmentManager->PauseSimulation();
}

BOOL CiTLC_AHVView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: CREATESTRUCT cs�� �����Ͽ� ���⿡��
	// Window Ŭ���� �Ǵ� ��Ÿ���� �����մϴ�.

	return CScrollView::PreCreateWindow(cs);
}

// CiTLC_AHVView �׸���

void CiTLC_AHVView::OnDraw(CDC* pDC)
{
	CiTLC_AHVDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: ���⿡ ���� �����Ϳ� ���� �׸��� �ڵ带 �߰��մϴ�.
	CRect rcClient; 
	GetClientRect(&rcClient);
	CBitmap memBmp, *pOldBmp;
	memBmp.CreateCompatibleBitmap( pDC, DC_WIDTH, DC_HEIGHT );

	CDC memDC;
	memDC.CreateCompatibleDC(pDC);
	pOldBmp = memDC.SelectObject(&memBmp);
	memDC.FillSolidRect(0, 0, DC_WIDTH, DC_HEIGHT, RGB(255,255,255) );
	
	memDC.SetBkMode(TRANSPARENT);

	
	DrawBackground( memDC);

	//�ʱ�ȭ �۾��� ������ Draw ����
	if( m_pEquipmentManager->CheckInitComplete() ){
		//m_pALVEmulator->DrawBackground( memDC, m_dZoom );
		m_pALVEmulator->DrawApron( memDC, m_dZoom );
		m_pEquipmentManager->DrawHPs( memDC, m_dZoom );
		m_pALVEmulator->DrawALVs( memDC, m_dZoom, m_bDrawRoute );
		m_pEquipmentManager->DrawCranes( memDC, m_dZoom );
	}
	
	pDC->BitBlt( 0, 0, DC_WIDTH, DC_HEIGHT, &memDC, 0, 0, SRCCOPY );
	memDC.SelectObject(pOldBmp);
	memBmp.DeleteObject();
}

void CiTLC_AHVView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();
	CSize sizeTotal;
	// TODO: �� ���� ��ü ũ�⸦ ����մϴ�.

	m_dZoom = 0.035;

	sizeTotal.cx = DC_WIDTH;
	sizeTotal.cy = DC_HEIGHT;
	CSize sizePage( sizeTotal.cx/2, sizeTotal.cy/2 );
	CSize sizeLine( sizeTotal.cx/50, sizeTotal.cy/50 );
	SetScrollSizes(MM_TEXT, sizeTotal, sizePage, sizeLine );

	m_hBitmapShip = (HBITMAP)LoadImage(NULL, "Vessel.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION | LR_DEFAULTSIZE);

	CiTLC_AHVDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	m_pALVEmulator		= pDoc->m_Experiments.GetALVEmulator();
	m_pEquipmentManager = pDoc->m_Experiments.GetAHVSupervisor();
	
	if( g_SimulationSpec.autostart ){
		AfxGetMainWnd()->SendMessage(WM_COMMAND, ID_SIMSTART);
	}
}


// CiTLC_AHVView �μ�

BOOL CiTLC_AHVView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// �⺻���� �غ�
	return DoPreparePrinting(pInfo);
}

void CiTLC_AHVView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: �μ��ϱ� ���� �߰� �ʱ�ȭ �۾��� �߰��մϴ�.
}

void CiTLC_AHVView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: �μ� �� ���� �۾��� �߰��մϴ�.
}


// CiTLC_AHVView ����

#ifdef _DEBUG
void CiTLC_AHVView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CiTLC_AHVView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

CiTLC_AHVDoc* CiTLC_AHVView::GetDocument() const // ����׵��� ���� ������ �ζ������� �����˴ϴ�.
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CiTLC_AHVDoc)));
	return (CiTLC_AHVDoc*)m_pDocument;
}
#endif //_DEBUG

//drawing operations-----------------------------------------------------------------------------------------------
void CiTLC_AHVView::DrawBackground( CDC& dc)
{
	//�ٴ� �׸���
	dc.FillSolidRect(0, 0, DC_WIDTH, (int)(g_SimulationSpec.berthWidth * m_dZoom), RGB(150, 200, 255) );

	//�������� ���� �׸���
	dc.FillSolidRect(0, (int)(g_SimulationSpec.berthWidth * m_dZoom), DC_WIDTH, (int)(g_SimulationSpec.quayWidth * m_dZoom), RGB(230, 230, 230) );

	//���Ҽ� �߱�
	CPen pen, *pOldPen;
	if( pen.CreatePen( PS_SOLID, 1, COLOR_BLACK ) ){
		pOldPen = dc.SelectObject( &pen );

		CPoint ptStart, ptEnd;
		//�ٴ�-�������� ���� ���Ҽ�
		ptStart	= CPoint( 0, (int)(g_SimulationSpec.berthWidth * m_dZoom));
		ptEnd	= CPoint( DC_WIDTH, (int)(g_SimulationSpec.berthWidth * m_dZoom));
		dc.MoveTo( ptStart );
		dc.LineTo( ptEnd );
		//��������-��ġ�� ���� ���Ҽ�
		ptStart	= CPoint( 0, (int)((g_SimulationSpec.berthWidth+g_SimulationSpec.quayWidth) * m_dZoom));
		ptEnd	= CPoint( DC_WIDTH, (int)((g_SimulationSpec.berthWidth+g_SimulationSpec.quayWidth) * m_dZoom));
		dc.MoveTo( ptStart );
		dc.LineTo( ptEnd );
		dc.SelectObject(pOldPen);
		pen.DeleteObject();
	}

	//ȭ�� ���� ��ܿ� �ùķ��̼� ���� �ð�, �ùķ��̼� ���, �ð��� ��� ���귮�� ǥ���Ѵ�.
	CString backMsg;
	//�ùķ��̼� ���� �ð�
	int simTotalSec = (int)(m_pEquipmentManager->GetClockTick()*g_SimulationSpec.unitTime);	//Clock�� Time���� ��ȯ
	int simHour = (int)(simTotalSec/3600);
	int simMin  = (int)(simTotalSec%3600/60);
	int simSec  = (int)(simTotalSec%60);

	backMsg.Format("Time:  %3dh %3dm %3ds", simHour, simMin, simSec );
	dc.TextOut( 10, 20, backMsg );

	//�ùķ��̼� ���ǵ� (delay = 100�� ��, 1���)
	int simSpeed;
	if(g_SimulationSpec.delay > 0){
		simSpeed = (int)(100/g_SimulationSpec.delay);
	}
	else{//delay�� ���� ���� ���, �� ��� ����� CPU�� dependent��.
		simSpeed = 300;
	}
	backMsg.Format("Speed:  x%3d", simSpeed );
	dc.TextOut( 10, 40, backMsg );

	backMsg.Format("Clock:  %d", m_pEquipmentManager->GetClockTick() );
	dc.TextOut( 10, 60, backMsg );

	/*
	//�ð��� QC ��� ó���� ǥ��
	double totalThroughput;
	if( simHour > 0 ){
		vector<CQC>& QCs = m_pApron->GetQCs();
		int qc;
		totalThroughput = 0.0;
		for( qc=0; qc < g_SimulationSpec.nQCperBerth; ++qc ){
			totalThroughput += (double)QCs[qc].GetThroughput();
		}
		totalThroughput = totalThroughput/(double)g_SimulationSpec.nQCperBerth;
		double avgHour	= (m_pEquipmentManager->GetClockTick()*g_SimSpec.unitTime)/(3600.0);
		totalThroughput = totalThroughput/avgHour;
		backMsg.Format("Avg. Throughput of QCs: %2.2f lift/hr", totalThroughput );
	}
	else{
		backMsg.Format("Avg. Throughput of QCs: Not enough data");
	}
	dc.TextOut( 10, 60, backMsg );
	*/

	return;
}


// CiTLC_AHVView �޽��� ó����

void CiTLC_AHVView::OnSimstart(){
	// TODO: ���⿡ ��� ó���� �ڵ带 �߰��մϴ�.
	if( m_iTimerID == 0 ){
		m_iTimerID = (int)SetTimer( 1, 100, NULL );
	}
	/*
	if( g_SimSpec..experiments ){
		m_pExperiments->StartExperiments();
	}
	else*/
	{
		m_pEquipmentManager->StartSimulation();
	}
}

void CiTLC_AHVView::OnSimpause(){
	// TODO: ���⿡ ��� ó���� �ڵ带 �߰��մϴ�.
	if( m_iTimerID != 0 ){
		KillTimer( m_iTimerID );
		m_iTimerID = 0;
	}
	m_pEquipmentManager->PauseSimulation();
}

void CiTLC_AHVView::OnTimer(UINT_PTR nIDEvent){
	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰� ��/�Ǵ� �⺻���� ȣ���մϴ�.
	Invalidate(false);
	CScrollView::OnTimer(nIDEvent);
}

void CiTLC_AHVView::OnViewRoute(){
	// TODO: ���⿡ ��� ó���� �ڵ带 �߰��մϴ�.
	if( m_bDrawRoute ){
		m_bDrawRoute = false;
	}
	else{
		m_bDrawRoute = true;
	}
}