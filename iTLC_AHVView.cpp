// iTLC_AHVView.cpp : CiTLC_AHVView 클래스의 구현
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
	// 표준 인쇄 명령입니다.
	ON_COMMAND(ID_FILE_PRINT, CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CScrollView::OnFilePrintPreview)
	ON_COMMAND(ID_SIMSTART, &CiTLC_AHVView::OnSimstart)
	ON_COMMAND(ID_SIMPAUSE, &CiTLC_AHVView::OnSimpause)
	ON_WM_TIMER()
	ON_COMMAND(ID_VIEW_ROUTE, &CiTLC_AHVView::OnViewRoute)
END_MESSAGE_MAP()

// CiTLC_AHVView 생성/소멸

CiTLC_AHVView::CiTLC_AHVView()
:m_iTimerID(0), m_bDrawRoute(false)
{
	// TODO: 여기에 생성 코드를 추가합니다.
	
}

CiTLC_AHVView::~CiTLC_AHVView()
{
	m_pEquipmentManager->PauseSimulation();
}

BOOL CiTLC_AHVView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: CREATESTRUCT cs를 수정하여 여기에서
	// Window 클래스 또는 스타일을 수정합니다.

	return CScrollView::PreCreateWindow(cs);
}

// CiTLC_AHVView 그리기

void CiTLC_AHVView::OnDraw(CDC* pDC)
{
	CiTLC_AHVDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: 여기에 원시 데이터에 대한 그리기 코드를 추가합니다.
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

	//초기화 작업이 끝나면 Draw 시작
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
	// TODO: 이 뷰의 전체 크기를 계산합니다.

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


// CiTLC_AHVView 인쇄

BOOL CiTLC_AHVView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 기본적인 준비
	return DoPreparePrinting(pInfo);
}

void CiTLC_AHVView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 인쇄하기 전에 추가 초기화 작업을 추가합니다.
}

void CiTLC_AHVView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 인쇄 후 정리 작업을 추가합니다.
}


// CiTLC_AHVView 진단

#ifdef _DEBUG
void CiTLC_AHVView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CiTLC_AHVView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

CiTLC_AHVDoc* CiTLC_AHVView::GetDocument() const // 디버그되지 않은 버전은 인라인으로 지정됩니다.
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CiTLC_AHVDoc)));
	return (CiTLC_AHVDoc*)m_pDocument;
}
#endif //_DEBUG

//drawing operations-----------------------------------------------------------------------------------------------
void CiTLC_AHVView::DrawBackground( CDC& dc)
{
	//바다 그리기
	dc.FillSolidRect(0, 0, DC_WIDTH, (int)(g_SimulationSpec.berthWidth * m_dZoom), RGB(150, 200, 255) );

	//에이프런 영역 그리기
	dc.FillSolidRect(0, (int)(g_SimulationSpec.berthWidth * m_dZoom), DC_WIDTH, (int)(g_SimulationSpec.quayWidth * m_dZoom), RGB(230, 230, 230) );

	//분할선 긋기
	CPen pen, *pOldPen;
	if( pen.CreatePen( PS_SOLID, 1, COLOR_BLACK ) ){
		pOldPen = dc.SelectObject( &pen );

		CPoint ptStart, ptEnd;
		//바다-에이프런 영역 분할선
		ptStart	= CPoint( 0, (int)(g_SimulationSpec.berthWidth * m_dZoom));
		ptEnd	= CPoint( DC_WIDTH, (int)(g_SimulationSpec.berthWidth * m_dZoom));
		dc.MoveTo( ptStart );
		dc.LineTo( ptEnd );
		//에이프런-장치장 영역 분할선
		ptStart	= CPoint( 0, (int)((g_SimulationSpec.berthWidth+g_SimulationSpec.quayWidth) * m_dZoom));
		ptEnd	= CPoint( DC_WIDTH, (int)((g_SimulationSpec.berthWidth+g_SimulationSpec.quayWidth) * m_dZoom));
		dc.MoveTo( ptStart );
		dc.LineTo( ptEnd );
		dc.SelectObject(pOldPen);
		pen.DeleteObject();
	}

	//화면 좌측 상단에 시뮬레이션 진행 시간, 시뮬레이션 배속, 시간당 평균 생산량을 표시한다.
	CString backMsg;
	//시뮬레이션 진행 시간
	int simTotalSec = (int)(m_pEquipmentManager->GetClockTick()*g_SimulationSpec.unitTime);	//Clock을 Time으로 변환
	int simHour = (int)(simTotalSec/3600);
	int simMin  = (int)(simTotalSec%3600/60);
	int simSec  = (int)(simTotalSec%60);

	backMsg.Format("Time:  %3dh %3dm %3ds", simHour, simMin, simSec );
	dc.TextOut( 10, 20, backMsg );

	//시뮬레이션 스피드 (delay = 100일 때, 1배속)
	int simSpeed;
	if(g_SimulationSpec.delay > 0){
		simSpeed = (int)(100/g_SimulationSpec.delay);
	}
	else{//delay를 주지 않은 경우, 이 경우 배속은 CPU에 dependent함.
		simSpeed = 300;
	}
	backMsg.Format("Speed:  x%3d", simSpeed );
	dc.TextOut( 10, 40, backMsg );

	backMsg.Format("Clock:  %d", m_pEquipmentManager->GetClockTick() );
	dc.TextOut( 10, 60, backMsg );

	/*
	//시간당 QC 평균 처리량 표시
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


// CiTLC_AHVView 메시지 처리기

void CiTLC_AHVView::OnSimstart(){
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
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
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	if( m_iTimerID != 0 ){
		KillTimer( m_iTimerID );
		m_iTimerID = 0;
	}
	m_pEquipmentManager->PauseSimulation();
}

void CiTLC_AHVView::OnTimer(UINT_PTR nIDEvent){
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	Invalidate(false);
	CScrollView::OnTimer(nIDEvent);
}

void CiTLC_AHVView::OnViewRoute(){
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	if( m_bDrawRoute ){
		m_bDrawRoute = false;
	}
	else{
		m_bDrawRoute = true;
	}
}