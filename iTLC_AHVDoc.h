// iTLC_AHVDoc.h : CiTLC_AHVDoc Ŭ������ �������̽�
//
#pragma once

#include "Experiments.h"

class CiTLC_AHVDoc : public CDocument
{
protected: // serialization������ ��������ϴ�.
	CiTLC_AHVDoc();
	DECLARE_DYNCREATE(CiTLC_AHVDoc)

// Ư��
// data �Ӽ���
public:
	CExperiments		m_Experiments;

// �۾�
public:

// ������
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// ����
public:
	virtual ~CiTLC_AHVDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// �޽��� �� �Լ��� �����߽��ϴ�.
protected:
	DECLARE_MESSAGE_MAP()
};


