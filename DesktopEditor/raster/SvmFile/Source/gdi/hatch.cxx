/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: hatch.cxx,v $
 * $Revision: 1.7 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifdef AVS
// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"
#include <tools/stream.hxx>
#include <tools/vcompat.hxx>
#include <tools/debug.hxx>
#ifndef _SV_HATCX_HXX
#include <vcl/hatch.hxx>
#endif

DBG_NAME( Hatch )
#endif
#include "../vcl/hatch.hxx"
#include "../tools/vcompat.hxx"
#include "../ASC/ASCStreamReader.h"

using namespace SVMCore;
// --------------
// - ImplHatch -
// --------------

ImplHatch::ImplHatch() :
    mnRefCount	( 1 ),
    maColor		( COL_BLACK ),
    meStyle		( HATCH_SINGLE ),
	mnDistance	( 1 ),
    mnAngle		( 0 )
{
}

// -----------------------------------------------------------------------

ImplHatch::ImplHatch( const ImplHatch& rImplHatch ) :
	mnRefCount	( 1 ),
    maColor		( rImplHatch.maColor ),
    meStyle		( rImplHatch.meStyle ),
	mnDistance	( rImplHatch.mnDistance ),
    mnAngle		( rImplHatch.mnAngle )
{
}

// ---------
// - Hatch -
// ---------

Hatch::Hatch()
{
    ////////DBG_CTOR( Hatch, NULL );
    mpImplHatch = new ImplHatch;
}

// -----------------------------------------------------------------------

Hatch::Hatch( const Hatch& rHatch )
{
    ////////DBG_CTOR( Hatch, NULL );
    ////////DBG_CHKOBJ( &rHatch, Hatch, NULL );
    mpImplHatch = rHatch.mpImplHatch;
    mpImplHatch->mnRefCount++;
}

// -----------------------------------------------------------------------

Hatch::Hatch( HatchStyle eStyle, const Color& rColor,
			  long nDistance, USHORT nAngle10 )
{
    ////////DBG_CTOR( Hatch, NULL );
    mpImplHatch = new ImplHatch;
    mpImplHatch->maColor = rColor;
    mpImplHatch->meStyle = eStyle;
    mpImplHatch->mnDistance = nDistance;
    mpImplHatch->mnAngle = nAngle10;
}

// -----------------------------------------------------------------------

Hatch::~Hatch()
{
    ////////DBG_DTOR( Hatch, NULL );
    if( !( --mpImplHatch->mnRefCount ) )
        delete mpImplHatch;
}

// -----------------------------------------------------------------------

Hatch& Hatch::operator=( const Hatch& rHatch )
{
    //////DBG_CHKTHIS( Hatch, NULL );
    ////////DBG_CHKOBJ( &rHatch, Hatch, NULL );

    rHatch.mpImplHatch->mnRefCount++;

    if( !( --mpImplHatch->mnRefCount ) )
        delete mpImplHatch;
    
	mpImplHatch = rHatch.mpImplHatch;
    return *this;
}
#ifdef AVS
// -----------------------------------------------------------------------

BOOL Hatch::operator==( const Hatch& rHatch ) const
{
    //////DBG_CHKTHIS( Hatch, NULL );
    ////////DBG_CHKOBJ( &rHatch, Hatch, NULL );

    return( mpImplHatch == rHatch.mpImplHatch ||
			( mpImplHatch->maColor == rHatch.mpImplHatch->maColor &&
			  mpImplHatch->meStyle == rHatch.mpImplHatch->meStyle &&
			  mpImplHatch->mnDistance == rHatch.mpImplHatch->mnDistance &&
			  mpImplHatch->mnAngle == rHatch.mpImplHatch->mnAngle ) );
}
#endif
// -----------------------------------------------------------------------

void Hatch::ImplMakeUnique()
{
    if( mpImplHatch->mnRefCount != 1 )
	{
		if( mpImplHatch->mnRefCount )
			mpImplHatch->mnRefCount--;

        mpImplHatch = new ImplHatch( *mpImplHatch );
	}
}

// -----------------------------------------------------------------------

void Hatch::SetStyle( HatchStyle eStyle )
{
    //////DBG_CHKTHIS( Hatch, NULL );
    ImplMakeUnique();
    mpImplHatch->meStyle = eStyle;
}

// -----------------------------------------------------------------------

void Hatch::SetColor( const Color& rColor )
{
    //////DBG_CHKTHIS( Hatch, NULL );
    ImplMakeUnique();
    mpImplHatch->maColor = rColor;
}

// -----------------------------------------------------------------------

void Hatch::SetDistance( long nDistance )
{
    //////DBG_CHKTHIS( Hatch, NULL );
    ImplMakeUnique();
    mpImplHatch->mnDistance = nDistance;
}

// -----------------------------------------------------------------------

void Hatch::SetAngle( USHORT nAngle10 )
{
    //////DBG_CHKTHIS( Hatch, NULL );
    ImplMakeUnique();
    mpImplHatch->mnAngle = nAngle10;
}
namespace SVMCore{
// -----------------------------------------------------------------------

SvStream& operator>>( SvStream& rIStm, ImplHatch& rImplHatch )
{
    VersionCompat	aCompat( rIStm, STREAM_READ );
    UINT16			nTmp16;

    rIStm >> nTmp16; rImplHatch.meStyle = (HatchStyle) nTmp16;
    rIStm >> rImplHatch.maColor >> rImplHatch.mnDistance >> rImplHatch.mnAngle;

    return rIStm;
}
}//SVMCore
#ifdef AVS
// -----------------------------------------------------------------------

SvStream& operator<<( SvStream& rOStm, const ImplHatch& rImplHatch )
{
    VersionCompat aCompat( rOStm, STREAM_WRITE, 1 );

    rOStm << (UINT16) rImplHatch.meStyle << rImplHatch.maColor;
	rOStm << rImplHatch.mnDistance << rImplHatch.mnAngle;

    return rOStm;
}
#endif
namespace SVMCore{
// -----------------------------------------------------------------------

SvStream& operator>>( SvStream& rIStm, Hatch& rHatch )
{
    rHatch.ImplMakeUnique();
    return( rIStm >> *rHatch.mpImplHatch );
}
}//SVMCore
#ifdef AVS
// -----------------------------------------------------------------------

SvStream& operator<<( SvStream& rOStm, const Hatch& rHatch )
{
    return( rOStm << *rHatch.mpImplHatch );
}
#endif