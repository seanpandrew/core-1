﻿/*
 * (c) Copyright Ascensio System SIA 2010-2017
 *
 * This program is a free software product. You can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License (AGPL)
 * version 3 as published by the Free Software Foundation. In accordance with
 * Section 7(a) of the GNU AGPL its Section 15 shall be amended to the effect
 * that Ascensio System SIA expressly excludes the warranty of non-infringement
 * of any third-party rights.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR  PURPOSE. For
 * details, see the GNU AGPL at: http://www.gnu.org/licenses/agpl-3.0.html
 *
 * You can contact Ascensio System SIA at Lubanas st. 125a-25, Riga, Latvia,
 * EU, LV-1021.
 *
 * The  interactive user interfaces in modified source and object code versions
 * of the Program must display Appropriate Legal Notices, as required under
 * Section 5 of the GNU AGPL version 3.
 *
 * Pursuant to Section 7(b) of the License you must retain the original Product
 * logo when distributing the program. Pursuant to Section 7(e) we decline to
 * grant you any rights under trademark law for use of our trademarks.
 *
 * All the Product's GUI elements, including illustrations and icon sets, as
 * well as technical writing content are licensed under the terms of the
 * Creative Commons Attribution-ShareAlike 4.0 International. See the License
 * terms at http://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
 */

#include "WorksheetSubstream.h"

#include "Biff_records/Uncalced.h"
#include "Biff_records/Index.h"
#include "Biff_unions/GLOBALS.h"
#include "Biff_unions/PAGESETUP.h"
#include "Biff_records/Dimensions.h"
#include "Biff_records/HFPicture.h"
#include "Biff_records/Note.h"
#include "Biff_records/DxGCol.h"
#include "Biff_records/MergeCells.h"
#include "Biff_records/LRng.h"
#include "Biff_records/CodeName.h"
#include "Biff_records/WebPub.h"
#include "Biff_records/Window1.h"
#include "Biff_records/CellWatch.h"
#include "Biff_records/SheetExt.h"
#include "Biff_records/EOF.h"
#include "Biff_records/BOF.h"
#include "Biff_records/DefaultRowHeight.h"
#include "Biff_records/Label.h"

#include "Biff_unions/BACKGROUND.h"
#include "Biff_unions/BIGNAME.h"
#include "Biff_unions/PROTECTION_COMMON.h" 
#include "Biff_unions/COLUMNS.h"
#include "Biff_unions/SCENARIOS.h"
#include "Biff_unions/SORTANDFILTER.h"
#include "Biff_unions/CELLTABLE.h"
#include "Biff_unions/OBJECTS.h"
#include "Biff_unions/PIVOTVIEW.h"
#include "Biff_unions/DCON.h"
#include "Biff_unions/WINDOW.h"
#include "Biff_unions/CUSTOMVIEW.h"
#include "Biff_unions/SORT.h"
#include "Biff_unions/QUERYTABLE.h"
#include "Biff_unions/PHONETICINFO.h"
#include "Biff_unions/CONDFMTS.h"
#include "Biff_unions/HLINK.h"
#include "Biff_unions/DVAL.h"
#include "Biff_unions/FEAT.h"
#include "Biff_unions/FEAT11.h"
#include "Biff_unions/RECORD12.h"
#include "Biff_unions/SHFMLA_SET.h"

#include "Biff_structures/ODRAW/OfficeArtDgContainer.h"

namespace XLS
{;


WorksheetSubstream::WorksheetSubstream(const size_t ws_index)
:	ws_index_(ws_index)
{
}


WorksheetSubstream::~WorksheetSubstream()
{
}


BaseObjectPtr WorksheetSubstream::clone()
{
	return BaseObjectPtr(new WorksheetSubstream(*this));
}


/*
WORKSHEETCONTENT = [Uncalced] Index GLOBALS PAGESETUP [HeaderFooter] [BACKGROUND] *BIGNAME [PROTECTION] 
					COLUMNS [SCENARIOS] SORTANDFILTER Dimensions [CELLTABLE] OBJECTS *HFPicture *Note 
					*PIVOTVIEW [DCON] 1*WINDOW *CUSTOMVIEW *2SORT [DxGCol] *MergeCells [LRng] *QUERYTABLE 
					[PHONETICINFO] CONDFMTS *HLINK [DVAL] [CodeName] *WebPub *CellWatch [SheetExt] *FEAT 
					*FEAT11 *RECORD12 EOF
WORKSHEET = BOF WORKSHEETCONTENT
*/
const bool WorksheetSubstream::loadContent(BinProcessor& proc)
{
	global_info_ = proc.getGlobalWorkbookInfo();
	
	GlobalWorkbookInfo::_sheet_size_info sheet_size_info;
	
	global_info_->sheet_size_info.push_back(sheet_size_info);
	global_info_->current_sheet = global_info_->sheet_size_info.size();

	global_info_->cmt_rules	= 0;

	int count = 0;
	std::vector<CellRangeRef>	shared_formulas_locations;
	
	if(!proc.mandatory<BOF>())
	{
		return false;
    }

	while (true)
	{
		CFRecordType::TypeId type = proc.getNextRecordType();
		
		if (type == rt_NONE || type == rt_BOF) //следующий пошел??
			break;
		if (type == rt_EOF) 
		{
			proc.mandatory<EOF_T>();
			break;
		}

		switch(type)
		{
			case rt_Uncalced:		proc.optional<Uncalced>();		break;
			case rt_Index:			proc.optional<Index>();			break;
			case rt_CalcRefMode://todooo сделать вариативно по всем проверку
			case rt_CalcMode:
			case rt_PrintRowCol:
			{
				GLOBALS globals(false);
				if (proc.mandatory(globals))
				{
					m_GLOBALS = elements_.back();
					elements_.pop_back();
				}
			}break;
			case rt_Dimensions:
			{
				if (proc.optional<Dimensions>())
				{
					m_Dimensions = elements_.back();
					elements_.pop_back();
				}		
			}break;
			case rt_Window2:
			{
				count = proc.repeated<WINDOW>(0, 0);
				while(count > 0)
				{
					m_arWINDOW.insert(m_arWINDOW.begin(), elements_.back());
					elements_.pop_back();
					count--;
				}
			}break;
			case rt_DefColWidth:
			case rt_ColInfo:
			{
				if (proc.optional<COLUMNS>())
				{
					if (!m_COLUMNS)//???
						m_COLUMNS = elements_.back();
					elements_.pop_back();
				}
			}break;
			case rt_DefaultRowHeight:
			{
				if (proc.optional<DefaultRowHeight>())
				{
					m_DefaultRowHeight = elements_.back();
					elements_.pop_back();
				}
			}break;
			case rt_Header:
			case rt_Footer:		
			case rt_BottomMargin:
			case rt_TopMargin:
			case rt_LeftMargin:
			case rt_RightMargin:
			{
				if (proc.mandatory<PAGESETUP>())
				{
					if (!m_PAGESETUP)
						m_PAGESETUP = elements_.back();
					elements_.pop_back();
				}
			}break;
			case rt_BkHim:
			{
				if (proc.optional<BACKGROUND>())
				{
					m_BACKGROUND = elements_.back();
					elements_.pop_back();
				}
			}break;
			case rt_BigName:		proc.repeated<BIGNAME>(0, 0);		break;
			case rt_Protect:		proc.optional<PROTECTION_COMMON>();	break;
			case rt_ScenMan:		proc.optional<SCENARIOS>();			break;	
			case rt_Sort:
			case rt_AutoFilterInfo:
			{
				if (proc.optional<SORTANDFILTER>())// Let it be optional
				{
					m_SORTANDFILTER = elements_.back();
					elements_.pop_back();
				}	
			}break;
			case rt_LabelSst://order_history.xls
			case rt_Label://file(6).xls
			case rt_Row:
			{
				CELLTABLE cell_table(shared_formulas_locations);
				if (proc.optional(cell_table))
				{
					m_CELLTABLE = elements_.back();
					elements_.pop_back();
				}
				if(0 != shared_formulas_locations.size())
				{
					SHFMLA_SET shfmla_set(shared_formulas_locations);
			       
					if (proc.optional(shfmla_set))
					{
						m_SHFMLA_SET = elements_.back();
						elements_.pop_back();
					}
				}
			}break;
			case rt_Obj:
			case rt_MsoDrawing:
			{
				OBJECTS objects(false);
				if (proc.optional(objects))
				{
					if (!m_OBJECTS) m_OBJECTS = elements_.back();
					else
					{
						Log::warning(L"Double set OBJECTS!!!");
					}
					elements_.pop_back();
				}
			}break;
			case rt_HFPicture:		
			{
				count = proc.repeated<HFPicture>(0, 0);		
				while(count > 0)
				{
					m_arHFPicture.insert(m_arHFPicture.begin(), elements_.back());
					elements_.pop_back();
					count--;
				}
			}break;
			case rt_CommentText:
			{
				count = proc.repeated<CommentText>(0, 0);
				while(count > 0)
				{
					m_arNote.insert(m_arNote.begin(), elements_.back());
					elements_.pop_back();
					count--;
				}
			}break;
			case rt_Note:
			{
				count = proc.repeated<Note>(0, 0);
				while(count > 0)
				{
					m_arNote.insert(m_arNote.begin(), elements_.back());
					elements_.pop_back();
					count--;
				}
			}break;
			case rt_SxView:			
			{
				count = proc.repeated<PIVOTVIEW>(0, 0);		
				while(count > 0)
				{
					m_arPIVOTVIEW.insert(m_arPIVOTVIEW.begin(), elements_.back());
					elements_.pop_back();
					count--;

					PIVOTVIEW *view = dynamic_cast<PIVOTVIEW*>(m_arPIVOTVIEW.back().get());
					mapPivotViews.insert(std::make_pair(view->name, m_arPIVOTVIEW.back()));
				}
			}break;
			case rt_DCon:
			{
				if (proc.optional<DCON>())
				{
					m_DCON = elements_.back();
					elements_.pop_back();
				}
			}break;
			case rt_UserSViewBegin:
			{
				count = proc.repeated<CUSTOMVIEW>(0, 0);
				while(count > 0)
				{
					m_arCUSTOMVIEW.insert(m_arCUSTOMVIEW.begin(), elements_.back());
					elements_.pop_back();
					count--;
				}
			}break;
			case rt_RRSort:
			{
				count = proc.repeated<SORT>(0, 2);
				while(count > 0)
				{
					m_arSORT.insert(m_arSORT.begin(), elements_.back());
					elements_.pop_back();
					count--;
				}
			}break;
			case rt_DxGCol:
			{				
				if (proc.optional<DxGCol>())
				{
					m_DxGCol = elements_.back();
					elements_.pop_back(); 
					
					DxGCol* dx = dynamic_cast<DxGCol*>(m_DxGCol.get());
					global_info_->sheet_size_info.back().defaultColumnWidth = dx->dxgCol / 256.;
				}
			}break;				
			case rt_MergeCells:
			{
				count = proc.repeated<MergeCells>(0, 0);
				while(count > 0)
				{
					MergeCells* m = dynamic_cast<MergeCells*>(elements_.back().get());
					if ((m) && (m->rgref.size() > 0))
					{
						m_arMergeCells.insert(m_arMergeCells.begin(), elements_.back());
					}
					elements_.pop_back();
					count--;
				}
			}break;
				
			case rt_LRng:
			{
				if (proc.optional<LRng>())
				{
					m_LRng = elements_.back();
					elements_.pop_back(); 
				}
			}break;
			case rt_Qsi:			
			{
				count = proc.repeated<QUERYTABLE>(0, 0);
				while(count > 0)
				{
					m_arQUERYTABLE.insert(m_arQUERYTABLE.begin(), elements_.back());
					elements_.pop_back();
					count--;
				}
			}break;
			case rt_PhoneticInfo:	proc.optional<PHONETICINFO>	();			break;			
			case rt_CondFmt:
			case rt_CondFmt12:
			{				
				if (proc.optional<CONDFMTS>())
				{
					m_CONDFMTS = elements_.back();
					elements_.pop_back();
				}
			}break;				
			case rt_HLink:
			{				
				count = proc.repeated<HLINK>(0, 0) ;
				while(count > 0)
				{
					m_arHLINK.insert(m_arHLINK.begin(), elements_.back());
					elements_.pop_back();
					count--;
				}
			}break;				
			case rt_DVal:	
			{
				if (proc.optional<DVAL>())
				{
					m_DVAL = elements_.back();
					elements_.pop_back();
				}
			}break;
			case rt_CodeName:
			{					
				if (proc.optional<CodeName>	())
				{
					m_CodeName = elements_.back();
					elements_.pop_back();
				}
			}break;
			case rt_WebPub:			proc.repeated<WebPub>	(0, 0);	break;
			case rt_CellWatch:		proc.repeated<CellWatch>(0, 0);	break;
			//case ExternCount:0x16
			//	{
			//	}break;
			case rt_SheetExt:
			{				
				if (proc.optional<SheetExt>())
				{
					m_SheetExt = elements_.back();
					elements_.pop_back();
				}
			}break;
			case rt_FeatHdr:
			{
				count = proc.repeated<FEAT>		(0, 0);
				while(count > 0)
				{
					m_arFEAT.insert(m_arFEAT.begin(), elements_.back());
					elements_.pop_back();
					count--;
				}
			}break;
			case rt_FeatHdr11:
			{
				count = proc.repeated<FEAT11>	(0, 0);
				while(count > 0)
				{
					m_arFEAT11.insert(m_arFEAT11.begin(), elements_.back());
					elements_.pop_back();
					count--;
				}
			}break;
			case rt_HeaderFooter:		
			{
				count = proc.repeated<RECORD12>	(0, 0);		
				while(count > 0)
				{
					m_arRECORD12.insert(m_arRECORD12.begin(), elements_.back());
					elements_.pop_back();
					count--;
				}
			}break;
			default://unknown .... skip					
			{
				proc.SkipRecord();	
			}break;
		}
	}	

	LoadHFPicture();

	return true;
}
void WorksheetSubstream::LoadHFPicture()
{
	if (m_arHFPicture.empty()) return;

	int current_size_hf = 0, j = 0;
	for ( size_t i = 0; i < m_arHFPicture.size(); i++)
	{
		HFPicture* hf = dynamic_cast<HFPicture*>(m_arHFPicture[i].get());
		if ((hf) && (hf->recordDrawingGroup))
		{
			if (!hf->fContinue && current_size_hf > 0)
			{
				XLS::CFRecord record(CFRecordType::ANY_TYPE, global_info_);
				for (; j < i; j++)
				{
					hf = dynamic_cast<HFPicture*>(m_arHFPicture[j].get());
					record.appendRawData(hf->recordDrawingGroup);
				}
				ODRAW::OfficeArtDgContainerPtr rgDrawing = ODRAW::OfficeArtDgContainerPtr(new ODRAW::OfficeArtDgContainer(ODRAW::OfficeArtRecord::CA_HF));
				rgDrawing->loadFields(record);
				m_arHFPictureDrawing.push_back(rgDrawing);
				current_size_hf = 0;

			}
			current_size_hf += hf->recordDrawingGroup->getDataSize();
		}
	}
	if (current_size_hf > 0)
	{
		XLS::CFRecord record(ODRAW::OfficeArtRecord::DggContainer, global_info_);
		for (; j < m_arHFPicture.size(); j++)
		{
			HFPicture* hf = dynamic_cast<HFPicture*>(m_arHFPicture[j].get());
			record.appendRawData(hf->recordDrawingGroup);
		}
		ODRAW::OfficeArtDgContainerPtr rgDrawing = ODRAW::OfficeArtDgContainerPtr(new ODRAW::OfficeArtDgContainer(ODRAW::OfficeArtRecord::CA_HF));
		rgDrawing->loadFields(record);
		m_arHFPictureDrawing.push_back(rgDrawing);
	}
}



} // namespace XLS

