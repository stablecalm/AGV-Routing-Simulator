// iTLC_AHVView.h : iCiTLC_AHVView Ŭ������ �������̽�
//
#pragma once

#include "ALVManager.h"
#include "YTEmulator/ALVEmulator.h"

#define	DC_WIDTH	1600		//ȭ���� �ʺ�
#define DC_HEIGHT	1000		//ȭ���� ����

class CiTLC_AHVView : public CScrollView
{
protected: // serialization������ ��������ϴ�.
	CiTLC_AHVView();
	DECLARE_DYNCREATE(CiTLC_AHVView)

// Ư��
public:
	//classes
	CiTLC_AHVDoc* GetDocument() const;

	CExperiments*		m_pExperiments;
	CALVManager*	m_pEquipmentManager;
	CALVEmulator*		m_pALVEmulator;

	int					m_iTimerID;
	double				m_dZoom;		//view�� ������ �����Ѵ�.
	HBITMAP				m_hBitmapShip;	//���� �̹����� �ε��� BITMAP ����

// �۾�
private:
	//void SetExperimentsDC( CDC& dc);
	
	//drawing
	void DrawBackground( CDC& dc);
	void DrawShip( CDC& dc );	

private:
	bool m_bDrawRoute;

// ������
	public:
	virtual void OnDraw(CDC* pDC);  // �� �並 �׸��� ���� �����ǵǾ����ϴ�.
virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void OnInitialUpdate(); // ���� �� ó�� ȣ��Ǿ����ϴ�.
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// ����
public:
	virtual ~CiTLC_AHVView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// �޽��� �� �Լ��� �����߽��ϴ�.
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

#ifndef _DEBUG  // iTLC_AHVView.cpp�� ����� ����
inline CiTLC_AHVDoc* CiTLC_AHVView::GetDocument() const
   { return reinterpret_cast<CiTLC_AHVDoc*>(m_pDocument); }
#endif

