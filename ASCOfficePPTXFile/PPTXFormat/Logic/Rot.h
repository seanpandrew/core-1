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
#pragma once
#ifndef PPTX_LOGIC_ROT_INCLUDE_H_
#define PPTX_LOGIC_ROT_INCLUDE_H_

#include "./../WrapperWritingElement.h"

namespace PPTX
{
	namespace Logic
	{

		class Rot : public WrapperWritingElement
		{
		public:
			WritingElement_AdditionConstructors(Rot)
			PPTX_LOGIC_BASE2(Rot)

			virtual OOX::EElementType getType() const
			{
				return OOX::et_a_rot;
			}	
			void fromXML(XmlUtils::CXmlLiteReader& oReader)
			{
				ReadAttributes( oReader );
			}
			void ReadAttributes(XmlUtils::CXmlLiteReader& oReader)
			{
				nullable_int lat_, lon_, rev_;

				WritingElement_ReadAttributes_Start_No_NS( oReader )
					WritingElement_ReadAttributes_Read_if		( oReader, _T("lat"), lat_)
					WritingElement_ReadAttributes_Read_else_if	( oReader, _T("lon"), lon_)
					WritingElement_ReadAttributes_Read_else_if	( oReader, _T("rev"), rev_)
				WritingElement_ReadAttributes_End_No_NS( oReader )
			
				lat = lat_.get_value_or(0);
				lon = lon_.get_value_or(0);
				rev = rev_.get_value_or(0);
				Normalize();
			}
			virtual void fromXML(XmlUtils::CXmlNode& node)
			{
				lat = node.ReadAttributeInt(L"lat");
				lon = node.ReadAttributeInt(L"lon");
				rev = node.ReadAttributeInt(L"rev");

				Normalize();
			}
			virtual std::wstring toXML() const
			{
				XmlUtils::CAttribute oAttr;
				oAttr.Write(_T("lat"), lat);
				oAttr.Write(_T("lon"), lon);
				oAttr.Write(_T("rev"), rev);

				return XmlUtils::CreateNode(_T("a:rot"), oAttr);
			}
		public:
			int lat;
			int lon;
			int rev;
		protected:
			virtual void FillParentPointersForChilds(){};

			AVSINLINE void Normalize()
			{
				normalize_value(lat, 0, 21600000);
				normalize_value(lon, 0, 21600000);
				normalize_value(rev, 0, 21600000);
			}
		};
	} // namespace Logic
} // namespace PPTX

#endif // PPTX_LOGIC_ROT_INCLUDE_H_