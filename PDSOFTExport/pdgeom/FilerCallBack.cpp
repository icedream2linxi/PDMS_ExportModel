#include "StdAfx.h"
#include "FilerCallBack.h"

DwgFilerCallBack::DwgFilerCallBack( AcDbDwgFiler *f ) : m_filer( f )
{
}


void 
DwgFilerCallBack::saveBytes( const void* buffer, int requiredLength )
{
    m_filer->writeBytes( buffer, requiredLength );
}


void 
DwgFilerCallBack::restoreBytes( void* buffer, int requiredLength )
{
    m_filer->readBytes( buffer, requiredLength );
}
