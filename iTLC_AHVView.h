// iTLC_AHVView.h : iCiTLC_AHVView 클래스의 인터페이스
//
#pragma once

#include "ALVManager.h"
#include "YTEmulator/ALVEmulator.h"

#define	DC_WIDTH	1600		//화면의 너비
#define DC_HEIGHT	1000		//화면의 높이

class CiTLC_AHVView : public CScrollView
{
protected: // serialization에서만 만들어집니다.
	CiTLC_AHVView();
	DECLARE_DYNCREATE(CiTLC_AHVView)

// 특성
public:
	//classes
	CiTLC_AHVDoc* GetDocument() const;

	CExperiments*		m_pExperiments;
	CALVManager*	m_pEquipmentManager;
	CALVEmulator*		m_pALVEmulator;

	int					m_iTimerID;
	double				m_dZoom;		//view의 배율을 결정한다.
	HBITMAP				m_hBitmapShip;	//배의 이미지를 로드할 BITMAP 공간

// 작업
private:
	//void SetExperimentsDC( CDC& dc);
	
	//drawing
	void DrawBackground( CDC& dc);
	void DrawShip( CDC& dc );	

private:
	bool m_bDrawRoute;

// 재정의
	public:
	virtual void OnDraw(CDC* pDC);  // 이 뷰를 그리기 위해 재정의되었습니다.
virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void OnInitialUpdate(); // 생성 후 처음 호출되었습니다.
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// 구현
public:
	virtual ~CiTLC_AHVView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 메시지 맵 함수를 생성했습니다.
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSimstart();
public:
	afx_msg void OnSimpause();
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
public:
	afx_msg void OnViewRoute();
};

#ifndef _DEBUG  // iTLC_AHVView.cpp의 디버그 버전
inline CiTLC_AHVDoc* CiTLC_AHVView::GetDocument() const
   { return reinterpret_cast<CiTLC_AHVDoc*>(m_pDocument); }
#endif

