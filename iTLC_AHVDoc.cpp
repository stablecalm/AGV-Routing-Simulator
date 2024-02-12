// iTLC_AHVDoc.cpp : CiTLC_AHVDoc 클래스의 구현
//

#include "stdafx.h"
#include "iTLC_AHV.h"

#include "iTLC_AHVDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CiTLC_AHVDoc

IMPLEMENT_DYNCREATE(CiTLC_AHVDoc, CDocument)

BEGIN_MESSAGE_MAP(CiTLC_AHVDoc, CDocument)
END_MESSAGE_MAP()


// CiTLC_AHVDoc 생성/소멸

CiTLC_AHVDoc::CiTLC_AHVDoc()
{
	// TODO: 여기에 일회성 생성 코드를 추가합니다.
	//실험용 코드
	m_Experiments.Initialize();
}

CiTLC_AHVDoc::~CiTLC_AHVDoc()
{
}

BOOL CiTLC_AHVDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: 여기에 다시 초기화 코드를 추가합니다.
	// SDI 문서는 이 문서를 다시 사용합니다.
	this->SetTitle(_T("3총괄 2세부"));

	return TRUE;
}




// CiTLC_AHVDoc serialization

void CiTLC_AHVDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: 여기에 저장 코드를 추가합니다.
	}
	else
	{
		// TODO: 여기에 로딩 코드를 추가합니다.
	}
}


// CiTLC_AHVDoc 진단

#ifdef _DEBUG
void CiTLC_AHVDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CiTLC_AHVDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CiTLC_AHVDoc 명령
