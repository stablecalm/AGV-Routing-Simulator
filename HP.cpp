#include "StdAfx.h"
#include ".\HP.h"

//constructor, destructor--------------------------------------------------------------------------------
CHP::CHP(void)
{
}

CHP::CHP( int id, CPoint& ptCenter, EHPAlign HPalign )
:m_ID(id), m_ptCenter(ptCenter), m_HPAlign(HPalign), m_bReserved(false), m_bHoisting(false),
m_nContainer(0), m_Capacity(0), m_MaximumCapacity(2)
{
	m_Container[0].conType = ConType_Undefined;
	m_Container[1].conType = ConType_Undefined;

	if( HPalign == HPA_Horizontal ){
		//QC에 놓이는 TP
		//Container가 놓일 곳의 좌표 설정
		m_pt40ftContainerArea[0].x = (int)(m_ptCenter.x-(g_SimulationSpec.Container40ftHalfLength));
		m_pt40ftContainerArea[0].y = (int)(m_ptCenter.y-(g_SimulationSpec.Container40ftHalfWidth ));
		m_pt40ftContainerArea[1].x = (int)(m_ptCenter.x+(g_SimulationSpec.Container40ftHalfLength ));
		m_pt40ftContainerArea[1].y = (int)(m_ptCenter.y-(g_SimulationSpec.Container40ftHalfWidth ));
		m_pt40ftContainerArea[2].x = (int)(m_ptCenter.x+(g_SimulationSpec.Container40ftHalfLength ));
		m_pt40ftContainerArea[2].y = (int)(m_ptCenter.y+(g_SimulationSpec.Container40ftHalfWidth));
		m_pt40ftContainerArea[3].x = (int)(m_ptCenter.x-(g_SimulationSpec.Container40ftHalfLength ));
		m_pt40ftContainerArea[3].y = (int)(m_ptCenter.y+(g_SimulationSpec.Container40ftHalfWidth));

		//좌측 20ft 컨테이너가 놓일 곳의 좌표 설정
		m_ptFront20ftContainerArea[0].x = (int)(m_ptCenter.x-(g_SimulationSpec.Container20ftLength+g_SimulationSpec.i20ftHalfGap));
		m_ptFront20ftContainerArea[0].y = (int)(m_ptCenter.y-(g_SimulationSpec.Container20ftHalfWidth ));
		m_ptFront20ftContainerArea[1].x = (int)(m_ptCenter.x-(g_SimulationSpec.i20ftHalfGap ));
		m_ptFront20ftContainerArea[1].y = (int)(m_ptCenter.y-(g_SimulationSpec.Container20ftHalfWidth ));
		m_ptFront20ftContainerArea[2].x = (int)(m_ptCenter.x-(g_SimulationSpec.i20ftHalfGap ));
		m_ptFront20ftContainerArea[2].y = (int)(m_ptCenter.y+(g_SimulationSpec.Container20ftHalfWidth));
		m_ptFront20ftContainerArea[3].x = (int)(m_ptCenter.x-(g_SimulationSpec.Container20ftLength+g_SimulationSpec.i20ftHalfGap ));
		m_ptFront20ftContainerArea[3].y = (int)(m_ptCenter.y+(g_SimulationSpec.Container20ftHalfWidth));

		//우측 20ft 컨테이너가 놓일 곳의 좌표 설정
		m_ptBack20ftContainerArea[0].x = (int)(m_ptCenter.x+(g_SimulationSpec.i20ftHalfGap ));
		m_ptBack20ftContainerArea[0].y = (int)(m_ptCenter.y-(g_SimulationSpec.Container20ftHalfWidth ));
		m_ptBack20ftContainerArea[1].x = (int)(m_ptCenter.x+(g_SimulationSpec.Container20ftLength+g_SimulationSpec.i20ftHalfGap));
		m_ptBack20ftContainerArea[1].y = (int)(m_ptCenter.y-(g_SimulationSpec.Container20ftHalfWidth ));
		m_ptBack20ftContainerArea[2].x = (int)(m_ptCenter.x+(g_SimulationSpec.Container20ftLength+g_SimulationSpec.i20ftHalfGap ));
		m_ptBack20ftContainerArea[2].y = (int)(m_ptCenter.y+(g_SimulationSpec.Container20ftHalfWidth));
		m_ptBack20ftContainerArea[3].x = (int)(m_ptCenter.x+(g_SimulationSpec.i20ftHalfGap ));
		m_ptBack20ftContainerArea[3].y = (int)(m_ptCenter.y+(g_SimulationSpec.Container20ftHalfWidth));
		
	}
	else //TPA_Vertical
	{
		//Block에 놓이는 TP
		//Container가 놓일 곳의 좌표 설정
		m_pt40ftContainerArea[0].x = (int)(m_ptCenter.x-(g_SimulationSpec.Container40ftHalfWidth));
		m_pt40ftContainerArea[0].y = (int)(m_ptCenter.y-(g_SimulationSpec.Container40ftHalfLength));
		m_pt40ftContainerArea[1].x = (int)(m_ptCenter.x+(g_SimulationSpec.Container40ftHalfWidth));
		m_pt40ftContainerArea[1].y = (int)(m_ptCenter.y-(g_SimulationSpec.Container40ftHalfLength));
		m_pt40ftContainerArea[2].x = (int)(m_ptCenter.x+(g_SimulationSpec.Container40ftHalfWidth));
		m_pt40ftContainerArea[2].y = (int)(m_ptCenter.y+(g_SimulationSpec.Container40ftHalfLength));
		m_pt40ftContainerArea[3].x = (int)(m_ptCenter.x-(g_SimulationSpec.Container40ftHalfWidth));
		m_pt40ftContainerArea[3].y = (int)(m_ptCenter.y+(g_SimulationSpec.Container40ftHalfLength));

		//상측 20ft 컨테이너가 놓일 곳의 좌표 설정
		m_ptFront20ftContainerArea[0].x = (int)(m_ptCenter.x-(g_SimulationSpec.Container20ftHalfWidth ));
		m_ptFront20ftContainerArea[0].y = (int)(m_ptCenter.y-(g_SimulationSpec.Container20ftLength+g_SimulationSpec.i20ftHalfGap));
		m_ptFront20ftContainerArea[1].x = (int)(m_ptCenter.x+(g_SimulationSpec.Container20ftHalfWidth ));
		m_ptFront20ftContainerArea[1].y = (int)(m_ptCenter.y-(g_SimulationSpec.Container20ftLength+g_SimulationSpec.i20ftHalfGap ));
		m_ptFront20ftContainerArea[2].x = (int)(m_ptCenter.x+(g_SimulationSpec.Container20ftHalfWidth));
		m_ptFront20ftContainerArea[2].y = (int)(m_ptCenter.y-(g_SimulationSpec.i20ftHalfGap ));
		m_ptFront20ftContainerArea[3].x = (int)(m_ptCenter.x-(g_SimulationSpec.Container20ftHalfWidth));
		m_ptFront20ftContainerArea[3].y = (int)(m_ptCenter.y-(g_SimulationSpec.i20ftHalfGap ));

		//하측 20ft 컨테이너가 놓일 곳의 좌표 설정
		m_ptBack20ftContainerArea[0].x = (int)(m_ptCenter.x-(g_SimulationSpec.Container20ftHalfWidth ));
		m_ptBack20ftContainerArea[0].y = (int)(m_ptCenter.y+(g_SimulationSpec.i20ftHalfGap ));
		m_ptBack20ftContainerArea[1].x = (int)(m_ptCenter.x+(g_SimulationSpec.Container20ftHalfWidth ));
		m_ptBack20ftContainerArea[1].y = (int)(m_ptCenter.y+(g_SimulationSpec.i20ftHalfGap ));
		m_ptBack20ftContainerArea[2].x = (int)(m_ptCenter.x+(g_SimulationSpec.Container20ftHalfWidth));
		m_ptBack20ftContainerArea[2].y = (int)(m_ptCenter.y+(g_SimulationSpec.Container20ftLength+g_SimulationSpec.i20ftHalfGap ));
		m_ptBack20ftContainerArea[3].x = (int)(m_ptCenter.x-(g_SimulationSpec.Container20ftHalfWidth));
		m_ptBack20ftContainerArea[3].y = (int)(m_ptCenter.y+(g_SimulationSpec.Container20ftLength+g_SimulationSpec.i20ftHalfGap));
	}
}

CHP::~CHP(void)
{
}
//constructor, destructor--------------------------------------------------------------------------------

//public operations--------------------------------------------------------------------------------------
void CHP::DrawTP(CDC& dc, double m_dZoom)
{
	if( m_Capacity > 0 ){//영역은 Waypoint가 그리므로, HP는 컨테이너만 그려주면 된다.
		CPen pen, *pOldPen;
		CBrush brush_container, *pOldBrush;
		if( pen.CreatePen(PS_SOLID,1, COLOR_BLACK) ){
			pOldPen = dc.SelectObject(&pen);
			
			if( m_Container[0].conType == ConType_General40ft ){
				CPoint buffer_area[4];
				int i;		
				for(i=0; i<4; ++i){
					buffer_area[i].x = (int)(m_pt40ftContainerArea[i].x *m_dZoom);
					buffer_area[i].y = (int)(m_pt40ftContainerArea[i].y *m_dZoom);			
				}
				//컨테이너 영역의 테두리를 그려준다.
				dc.MoveTo(buffer_area[3]);
				for(i=0;i<4;++i){
					dc.LineTo(buffer_area[i]);
				}
				if( m_Container[0].containerID.substr(0,1) == "D" ){//컨테이너 영역을 칠한다.
					if( brush_container.CreateSolidBrush(COLOR_CONTAINER40FTD) ){
						pOldBrush = dc.SelectObject(&brush_container);
						dc.Polygon( buffer_area, 4 );
						dc.SelectObject(pOldBrush);
						brush_container.DeleteObject();
					}
				}
				else{//컨테이너 영역을 칠한다.
					if( brush_container.CreateSolidBrush(COLOR_CONTAINER40FTL) ){
						pOldBrush = dc.SelectObject(&brush_container);
						dc.Polygon( buffer_area, 4 );
						dc.SelectObject(pOldBrush);
						brush_container.DeleteObject();
					}
				}
				
			}
			else if(m_Container[0].conType == ConType_General20ft ){
				CPoint buffer_area[4];
				int i;		
				for(i=0; i<4; ++i){
					buffer_area[i].x = (int)(m_ptFront20ftContainerArea[i].x *m_dZoom);
					buffer_area[i].y = (int)(m_ptFront20ftContainerArea[i].y *m_dZoom);			
				}
				//컨테이너 영역의 테두리를 그려준다.
				dc.MoveTo(buffer_area[3]);
				for(i=0;i<4;++i){
					dc.LineTo(buffer_area[i]);
				}
				if( m_Container[0].containerID.substr(0,1) == "D" ){//컨테이너 영역을 칠한다.
					if( brush_container.CreateSolidBrush(COLOR_CONTAINER20FTD) ){
						pOldBrush = dc.SelectObject(&brush_container);
						dc.Polygon( buffer_area, 4 );
						dc.SelectObject(pOldBrush);
						brush_container.DeleteObject();
					}
				}
				else{
					if( brush_container.CreateSolidBrush(COLOR_CONTAINER20FTL) ){
						pOldBrush = dc.SelectObject(&brush_container);
						dc.Polygon( buffer_area, 4 );
						dc.SelectObject(pOldBrush);
						brush_container.DeleteObject();
					}
				}
				
			}

			if( m_Container[1].conType == ConType_General20ft ){
				CPoint buffer_area[4];
				int i;
				for(i=0; i<4; ++i){
					buffer_area[i].x = (int)(m_ptBack20ftContainerArea[i].x *m_dZoom);
					buffer_area[i].y = (int)(m_ptBack20ftContainerArea[i].y *m_dZoom);			
				}
				//컨테이너 영역의 테두리를 그려준다.
				dc.MoveTo(buffer_area[3]);
				for(i=0;i<4;++i){
					dc.LineTo(buffer_area[i]);
				}
				if( m_Container[1].containerID.substr(0,1) == "D" ){//컨테이너 영역을 칠한다.
					if( brush_container.CreateSolidBrush(COLOR_CONTAINER20FTD) ){
						pOldBrush = dc.SelectObject(&brush_container);
						dc.Polygon( buffer_area, 4 );
						dc.SelectObject(pOldBrush);
						brush_container.DeleteObject();
					}
				}
				else{
					if( brush_container.CreateSolidBrush(COLOR_CONTAINER20FTL) ){
						pOldBrush = dc.SelectObject(&brush_container);
						dc.Polygon( buffer_area, 4 );
						dc.SelectObject(pOldBrush);
						brush_container.DeleteObject();
					}
				}
			}			
			dc.SelectObject(pOldPen);
			pen.DeleteObject();
		}
	}
	
	//TP의 상태를 표시한다.
	CPoint ptTP = GetPtCenter();
	CString str;
	if( m_bHoisting ){		//크레인이 TP를 사용 중일 때
		str.Format( "H%d", m_Capacity );
	}
	else if( m_bReserved ){			//AHV가 TP를 사용 중일 때
		str.Format( "R%d", m_Capacity );
	}	
	else{						//TP 미사용 상태 - Available
		str.Format( "A%d", m_Capacity );
	}
	dc.TextOut((int)((ptTP.x) * m_dZoom), (int)((ptTP.y) * m_dZoom), str );
	return;
}

bool CHP::SetReservation(){ 
	if(m_bReserved){
		return false;
	}
	else{
		m_bReserved = true;
		return true;
	}
}

void CHP::SetContainer( CContainer container )
{
	if( container.conType == ConType_General20ft){
		m_Capacity += 1;
	}
	else if( container.conType == ConType_General40ft ){
		m_Capacity += 2;
	}

	if( m_Capacity > m_MaximumCapacity ){
		Assert(false, "CHP::Excessive Capacity");
	}
	
	if( m_Container[0].containerID == "" ){
		m_Container[0].conType = container.conType;
		m_Container[0].containerID = container.containerID;
	}
	else if( m_Container[1].containerID == "" ){
		m_Container[1].conType = container.conType;
		m_Container[1].containerID = container.containerID;
	}
	++m_nContainer;
	
	return;
}

void CHP::ReleaseContainer( CContainer container )
{
	if( container.conType == ConType_General20ft){
		m_Capacity -= 1;
	}
	else if( container.conType == ConType_General40ft ){
		m_Capacity -= 2;
	}
	if( m_Container[0].containerID == container.containerID ){
		m_Container[0].conType = ConType_Undefined;
		m_Container[0].containerID = "";
	}
	else if( m_Container[1].containerID == container.containerID ){
		m_Container[1].conType = ConType_Undefined;
		m_Container[1].containerID = "";
	}
	--m_nContainer;

	return;
}

string	CHP::GetProcessID()
{
	if( m_Container[0].containerID != "")
		return m_Container[0].containerID;
	else if( m_Container[1].containerID != "")
		return m_Container[1].containerID;
	else
		return "";
}

//public operations--------------------------------------------------------------------------------------