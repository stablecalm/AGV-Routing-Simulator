// iTLC_AHVDoc.cpp : CiTLC_AHVDoc Ŭ������ ����
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


// CiTLC_AHVDoc ����/�Ҹ�

CiTLC_AHVDoc::CiTLC_AHVDoc()
{
	// TODO: ���⿡ ��ȸ�� ���� �ڵ带 �߰��մϴ�.
	//����� �ڵ�
	m_Experiments.Initialize();
}

CiTLC_AHVDoc::~CiTLC_AHVDoc()
{
}

BOOL CiTLC_AHVDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: ���⿡ �ٽ� �ʱ�ȭ �ڵ带 �߰��մϴ�.
	// SDI ������ �� ������ �ٽ� ����մϴ�.
	this->SetTitle(_T("3�Ѱ� 2����"));

	return TRUE;
}




// CiTLC_AHVDoc serialization

void CiTLC_AHVDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: ���⿡ ���� �ڵ带 �߰��մϴ�.
	}
	else
	{
		// TODO: ���⿡ �ε� �ڵ带 �߰��մϴ�.
	}
}


// CiTLC_AHVDoc ����

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


// CiTLC_AHVDoc ���
