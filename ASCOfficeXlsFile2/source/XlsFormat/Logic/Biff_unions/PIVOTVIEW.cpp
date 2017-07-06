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

#include "PIVOTVIEW.h"
#include "PIVOTCORE.h"
#include "PIVOTFRT.h"
#include "PIVOTVD.h"
#include "PIVOTIVD.h"
#include "PIVOTPI.h"
#include "PIVOTLI.h"
#include "PIVOTEX.h"

#include "../Biff_records/SXDI.h"
#include "../Biff_records/SxView.h"

namespace XLS
{

PIVOTVIEW::PIVOTVIEW()
{
	indexCache = -1;
}

PIVOTVIEW::~PIVOTVIEW()
{
}

BaseObjectPtr PIVOTVIEW::clone()
{
	return BaseObjectPtr(new PIVOTVIEW(*this));
}

// PIVOTVIEW = PIVOTCORE [PIVOTFRT]
const bool PIVOTVIEW::loadContent(BinProcessor& proc)
{
	if(!proc.mandatory<PIVOTCORE>())
	{
		return false;
	}
	m_PIVOTCORE = elements_.back();
	elements_.pop_back();
	
	if (proc.optional<PIVOTFRT>())
	{
		m_PIVOTFRT = elements_.back();
		elements_.pop_back();
	}

	return true;
}

int PIVOTVIEW::serialize(std::wostream & strm)
{
	PIVOTCORE* core = dynamic_cast<PIVOTCORE*>(m_PIVOTCORE.get());
	if (!core) return 0;

	SxView* view = dynamic_cast<SxView*>(core->m_SxView.get());
	if (!view) return 0;

	PIVOTFRT* frt = dynamic_cast<PIVOTFRT*>(m_PIVOTFRT.get());

	indexCache = view->iCache;

	CP_XML_WRITER(strm)
	{
		CP_XML_NODE(L"pivotTableDefinition")
		{ 
			CP_XML_ATTR(L"xmlns", L"http://schemas.openxmlformats.org/spreadsheetml/2006/main");
			
			CP_XML_ATTR(L"name",				view->stTable.value()); 
			CP_XML_ATTR(L"cacheId",				view->iCache); 
			CP_XML_ATTR(L"useAutoFormatting",	view->fAutoFormat); 
			CP_XML_ATTR(L"dataOnRows",			view->sxaxis4Data.bRw); 
			CP_XML_ATTR(L"autoFormatId",		view->itblAutoFmt);
			CP_XML_ATTR(L"applyNumberFormats",	view->fAtrNum);
			CP_XML_ATTR(L"applyBorderFormats",	view->fAtrBdr); 
			CP_XML_ATTR(L"applyFontFormats",	view->fAtrFnt);
			CP_XML_ATTR(L"applyPatternFormats",	view->fAtrPat);
			CP_XML_ATTR(L"applyAlignmentFormats",	view->fAtrAlc);
			CP_XML_ATTR(L"applyWidthHeightFormats",	view->fAtrProc);
			if (!view->stData.value().empty())
			{
				CP_XML_ATTR(L"dataCaption",			view->stData.value()); 
			}
			//updatedVersion="2" 
			//asteriskTotals="1" 
			//showMemberPropertyTips="0" 
			//itemPrintTitles="1" 
			//createdVersion="1" 
			//indent="0" 
			//compact="0" 
			//compactData="0" 
			//gridDropZones="1"		
			CP_XML_NODE(L"location")
			{
				CP_XML_ATTR(L"ref", view->ref.toString());
				CP_XML_ATTR(L"firstHeaderRow", view->rwFirstHead - view->ref.rowFirst );
				CP_XML_ATTR(L"firstDataRow", view->rwFirstData - view->ref.rowFirst);
				CP_XML_ATTR(L"firstDataCol", view->colFirstData - view->ref.columnFirst); 
				CP_XML_ATTR(L"rowPageCount", 1); 
				CP_XML_ATTR(L"colPageCount", 1);
			}
			CP_XML_NODE(L"pivotFields")
			{
				CP_XML_ATTR(L"count", view->cDim);//Sxvd 
				for (size_t i = 0; i <  core->m_arPIVOTVD.size(); i++)
				{
					core->m_arPIVOTVD[i]->serialize(CP_XML_STREAM());
				}
			}
			if (core->m_arPIVOTIVD.size() >= 1)
			{
				CP_XML_NODE(L"rowFields")
				{
					CP_XML_ATTR(L"count", view->cDimRw);

					PIVOTIVD* ivd = dynamic_cast<PIVOTIVD*>(core->m_arPIVOTIVD[0].get());
					ivd->serialize(CP_XML_STREAM());
				}
			}
			if (core->m_arPIVOTLI.size() >= 1)//0 or 2
			{
				CP_XML_NODE(L"rowItems")
				{
					CP_XML_ATTR(L"count", view->cRw);
					
					PIVOTLI* line = dynamic_cast<PIVOTLI*>(core->m_arPIVOTLI[0].get());
					line->serialize(CP_XML_STREAM());
				}
			}
			if (core->m_arPIVOTIVD.size() == 2)//0 or 2
			{
				CP_XML_NODE(L"colFields")
				{
					CP_XML_ATTR(L"count", view->cDimCol);
					
					PIVOTIVD* ivd = dynamic_cast<PIVOTIVD*>(core->m_arPIVOTIVD[1].get());
					ivd->serialize(CP_XML_STREAM());
				}
			}
			if (core->m_arPIVOTLI.size() == 2)//0 or 2
			{
				CP_XML_NODE(L"colItems")
				{
					CP_XML_ATTR(L"count", view->cCol);
					
					PIVOTLI* line = dynamic_cast<PIVOTLI*>(core->m_arPIVOTLI[1].get());
					line->serialize(CP_XML_STREAM());
				}
			}
			if (core->m_PIVOTPI)
			{
				CP_XML_NODE(L"pageFields")
				{
					CP_XML_ATTR(L"count", view->cDimPg);
					
					core->m_PIVOTPI->serialize(CP_XML_STREAM());
				}
			}
			CP_XML_NODE(L"dataFields")
			{
				CP_XML_ATTR(L"count", view->cDimData);
				for (size_t i = 0; i <  core->m_arSXDI.size(); i++)
				{
					core->m_arSXDI[i]->serialize(CP_XML_STREAM());
				}
			}
			//CP_XML_NODE(L"pivotTableStyleInfo")
			//{
			//	CP_XML_ATTR(L"showRowHeaders", 1); 
			//	CP_XML_ATTR(L"showColHeaders", 1);
			//	CP_XML_ATTR(L"showRowStripes", 0);
			//	CP_XML_ATTR(L"showColStripes", 0);
			//	CP_XML_ATTR(L"showLastColumn", 1);
			//}
		}
	}

	return 0;
}

} // namespace XLS

