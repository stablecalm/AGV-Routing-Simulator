// iTLC_AHVDoc.h : CiTLC_AHVDoc 클래스의 인터페이스
//
#pragma once

#include "Experiments.h"

class CiTLC_AHVDoc : public CDocument
{
protected: // serialization에서만 만들어집니다.
	CiTLC_AHVDoc();
	DECLARE_DYNCREATE(CiTLC_AHVDoc)

// 특성
// data 속성들
public:
	CExperiments		m_Experiments;

// 작업
public:

// 재정의
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// 구현
public:
	virtual ~CiTLC_AHVDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 메시지 맵 함수를 생성했습니다.
protected:
	DECLARE_MESSAGE_MAP()
};


