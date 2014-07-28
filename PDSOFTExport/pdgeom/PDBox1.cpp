// PDBox1.cpp: implementation of the PDBox1 class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "language25.h"
#include "PDBox1.h"
#include "dbproxy.h"
#include "acgi.h"
#include "gecspl3d.h"
#include "geassign.h"
#include <dbsol3d.h>
#include <dbapserv.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define VERSION_PDBOX1 1

#ifdef _OBJECTARX2000_
ACRX_DXF_DEFINE_MEMBERS(PDBox1, PDPrimary3D,
                        AcDb::kDHL_CURRENT, AcDb::kMReleaseCurrent,
                        AcDbProxyEntity::kAllAllowedBits,
                        PDBOX1, Gle);
#else
ACRX_DXF_DEFINE_MEMBERS(PDBox1, PDPrimary3D, AcDbProxyEntity::kAllAllowedBits, PDBOX1, Gle);
#endif

void PDBox1::setDefault()
{
  PDPrimary3D::setDefault();
#if defined(NDEBUG) || !defined(_DEBUG)
  m_ptP = AcGePoint3d::kOrigin;
  m_VectLength.set(1, 0, 0);
  m_VectWidth.set(0, 1, 0);
  m_dLength =	m_dWidth = m_dHeight = 1;
#else
  m_ptP.set(0,0,0);
  m_VectLength.set(1, 0, 0);
  m_VectWidth.set(0, 1, 0);
  m_dLength =	m_dWidth = m_dHeight = 1000;
#endif
}
bool PDBox1::isValidData(double &f)
{
  if(!PDPrimary3D::isValidData(f))
    return false;
  if(!isValidFloat(f=m_dLength))
    return false;
  if(!isValidFloat(f=m_dWidth))
    return false;
  if(!isValidFloat(f=m_dHeight))
    return false;
  if(!isValidFloat(f=m_ptP.x) || !isValidFloat(f=m_ptP.y) || !isValidFloat(f=m_ptP.z))
    return false;
  if(!isValidFloat(f=m_VectLength.x) || !isValidFloat(f=m_VectLength.y) || !isValidFloat(f=m_VectLength.z))
    return false;
  if(!isValidFloat(f=m_VectWidth.x) || !isValidFloat(f=m_VectWidth.y) || !isValidFloat(f=m_VectWidth.z))
    return false;
  return true;
}

//zxb,20090113,增加audit，暂时只check自身成员数据
Acad::ErrorStatus PDBox1::audit(AcDbAuditInfo* pAuditInfo)
{
  return auditPDPrimary3D(this,pAuditInfo,_T("PDBox1"));
}
PDBox1::PDBox1(bool HasSnap/* = false*/) : PDPrimary3D(HasSnap)
{
    m_ptP = AcGePoint3d::kOrigin;
	m_VectLength.set(1, 0, 0);
    m_VectWidth.set(0, 1, 0);
	m_dLength =	m_dWidth = m_dHeight = 1;
#ifdef _USEAMODELER_
    createBody();
#endif
}

PDBox1::~PDBox1()
{

}

PDBox1::PDBox1(const AcGePoint3d &ptP, const AcGeVector3d &vectL, 
               const AcGeVector3d &vectW, double l,double w,double h, 
               bool HasSnap/* = false*/) : PDPrimary3D(HasSnap)
{
	m_ptP = ptP;
	m_VectLength = vectL.normal();
	m_VectWidth = vectW.normal();
	m_dLength = fabs(l);
    if(m_dLength == 0)
        m_dLength = 1;
    m_dWidth = fabs(w);
    if(m_dWidth == 0)
        m_dWidth = 1;
	m_dHeight = fabs(h);
    if(m_dHeight == 0)
        m_dHeight = 1;

    if(m_VectLength.isZeroLength())
    {
        if(m_VectWidth.isParallelTo(AcGeVector3d(1, 0, 0)))
            m_VectLength.set(0, -1, 0);
        else
            m_VectLength.set(1, 0, 0);
    }

    if(m_VectWidth.isZeroLength())
    {
        if(m_VectLength.isParallelTo(AcGeVector3d(0, 1, 0)))
            m_VectWidth.set(-1, 0, 0);
        else
            m_VectWidth.set(0, 1, 0);
    }

#ifdef _USEAMODELER_
    createBody();
#endif
}

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDBox1::subTransformBy(const AcGeMatrix3d &xform)
#else
Acad::ErrorStatus PDBox1::transformBy(const AcGeMatrix3d &xform)
#endif
{
    assertWriteEnabled();
	m_ptP.transformBy(xform);
	m_VectLength.transformBy(xform);
    m_VectLength.normalize();
	m_VectWidth.transformBy(xform);
    m_VectWidth.normalize();
	m_dHeight *= xform.scale();
	m_dLength *= xform.scale();
	m_dWidth *= xform.scale();

#ifdef _OBJECTARX2010_
	return PDPrimary3D::subTransformBy(xform);
#else
	return PDPrimary3D::transformBy(xform);
#endif
}

#ifdef _OBJECTARX2010_
Adesk::Boolean PDBox1::subWorldDraw(AcGiWorldDraw* mode)
#else
Adesk::Boolean PDBox1::worldDraw(AcGiWorldDraw* mode)
#endif
{
	assertReadEnabled();
	//if(acdbHostApplicationServices()->workingDatabase()->useri1() == 1)
	//	return Adesk::kTrue;

#ifdef _USEAMODELER_
#ifdef _OBJECTARX2010_
	PDPrimary3D::subWorldDraw(mode);
#else
	PDPrimary3D::worldDraw(mode);
#endif
#endif
    if (mode->regenAbort()) {
        return Adesk::kTrue;
    }
#ifndef _USEAMODELER_
    AcGePoint3dArray pArray;
	getVertices(pArray);
	if (getCalMaxPrecision())
	{
		//优化
		double a [3];
		a[0] = m_dHeight;
		a[1] = m_dWidth;
		a[2] = m_dLength;
		double mMax = a[0];
		double tmpD = 0;
		int nIndex = -1;
		for(int m = 0 ; m < 3; m++)
		{
			mMax = a[m];
			nIndex = m;
			for(int  j = m + 1; j < 3; j++)
			{
				if(mMax < a[j])
				{
					mMax = a[j];
					nIndex = j;
				}
			}
			if(m != nIndex)
			{
				tmpD = a[m];
				a[m] = a[nIndex];
				a[nIndex] = tmpD;
			}
		}
		mMax = a[1];
		int Precision = m_dDividPrecision;
		Precision = (int)(PI / acos(1.0 - mode->deviation(kAcGiMaxDevForCircle, pArray[0])
			* 2.0 / mMax));
		if(Precision < PRIMARY3D_SIMPLIZE_SCALE)
		{
			AcGePoint3d pt[5];
			int i = 0;
			//////////////////////////////////////////////////////////////////////////
			//draw lines
			for(i = 0; i < 4; ++i)
			{
				pt[i] = pArray[i];				
			}
			pt[i] = pArray[i];
			if (mode->geometry().polyline(5, pt))
			{
				return Adesk::kTrue;
			}
			for(i = 5; i < 9; ++i)
			{
				pt[i - 5] = pArray[i];
			}
			pt[4] = pArray[i];
			if (mode->geometry().polyline(5, pt))
			{
				return Adesk::kTrue;
			}
			for(i = 0; i < 4; ++i)
			{
				pt[0] = pArray[i];
				pt[1] = pArray[i + 5];
				if (mode->geometry().polyline(2, pt))
				{
					return Adesk::kTrue;
				}
			}
			return Adesk::kTrue;
		}
	}
#ifndef _ALWAYSSHOWWIRE_
	switch (mode->regenType())
	{
	case kAcGiHideOrShadeCommand:
	case kAcGiRenderCommand:
#endif
		{
			mode->subEntityTraits().setFillType(kAcGiFillAlways);
			////	下底面	///
			//添加实体模型各面的GS标记
			//added by szw 2009.11.18 : begin
			mode->subEntityTraits().setSelectionMarker(BOTTOM_PLANE);
			//added by szw 2009.11.18 : end
			if(mode->geometry().polygon(4, pArray.asArrayPtr()))
				return Adesk::kTrue;
			////	上底面	///
			//added by szw 2009.11.18 : begin
			mode->subEntityTraits().setSelectionMarker(TOP_PLANE);
			//added by szw 2009.11.18 : end
			if(mode->geometry().polygon(4, &(pArray.asArrayPtr()[5])))
				return Adesk::kTrue;
			AcGePoint3d verts[4];
			////	前侧面	///
			verts[0] = pArray[0];
			verts[1] = pArray[1];
			verts[2] = pArray[6];
			verts[3] = pArray[5];
			//added by szw 2009.11.18 : begin
			mode->subEntityTraits().setSelectionMarker(FRONT_PLANE);
			//added by szw 2009.11.18 : end
			if(mode->geometry().polygon(4, verts))
				return Adesk::kTrue;
			////	左侧面	///
			verts[1] = pArray[5];
			verts[2] = pArray[8];
			verts[3] = pArray[3];
			//added by szw 2009.11.18 : begin
			mode->subEntityTraits().setSelectionMarker(LEFT_PLANE);
			//added by szw 2009.11.18 : end
			if(mode->geometry().polygon(4, verts))
				return Adesk::kTrue;
			////	右侧面	///
			verts[0] = pArray[7];
			verts[1] = pArray[6];
			verts[2] = pArray[1];
			verts[3] = pArray[2];
			//added by szw 2009.11.18 : begin
			mode->subEntityTraits().setSelectionMarker(RIGHT_PLANE);
			//added by szw 2009.11.18 : end
			if(mode->geometry().polygon(4, verts))
				return Adesk::kTrue;
			////	后侧面	///
			verts[1] = pArray[2];
			verts[2] = pArray[3];
			verts[3] = pArray[8];
			//added by szw 2009.11.18 : begin
			mode->subEntityTraits().setSelectionMarker(BACK_PLANE);
			//added by szw 2009.11.18 : end
			mode->geometry().polygon(4, verts);
		}
#ifndef _ALWAYSSHOWWIRE_
		break;
	case kAcGiStandardDisplay:
	case kAcGiSaveWorldDrawForR12:
	case kAcGiSaveWorldDrawForProxy:
#endif
		{
			int gsIndex = 0;
			AcGePoint3d pt[2];
			int i;
			for(i = 0; i < 4; ++i)
			{
				pt[0] = pArray[i];
				pt[1] = pArray[i + 1];
				//统一线框模型和实体模型的GS标记
				//modified by szw 2009.11.18 : begin
				mode->subEntityTraits().setSelectionMarker(BOTTOM_PLANE);
				//modified by szw 2009.11.18 : end
				if (mode->geometry().polyline(2, pt))
					break;
			}
			for(i = 5; i < 9; ++i)
			{
				pt[0] = pArray[i];
				pt[1] = pArray[i + 1];
				//modified by szw 2009.11.18 : begin
				mode->subEntityTraits().setSelectionMarker(TOP_PLANE);
				//modified by szw 2009.11.18 : end
				if (mode->geometry().polyline(2, pt))
					break;
			}
			for(i = 0; i < 4; ++i)
			{
				pt[0] = pArray[i];
				pt[1] = pArray[i + 5];
				//modified by szw 2009.11.18 : begin
				if(i == 0)
					mode->subEntityTraits().setSelectionMarker(FRONT_PLANE);
				else if(i == 1)
					mode->subEntityTraits().setSelectionMarker(RIGHT_PLANE);
				else if(i == 2)
					mode->subEntityTraits().setSelectionMarker(BACK_PLANE);
				else if(i == 3)
					mode->subEntityTraits().setSelectionMarker(LEFT_PLANE);
				//modified by szw 2009.11.18 : end
				if (mode->geometry().polyline(2, pt))
					break;
			}
		}
#ifndef _ALWAYSSHOWWIRE_
		break;
	}
#endif
    /*switch (mode->regenType())
    {
    case kAcGiHideOrShadeCommand:
    case kAcGiRenderCommand:
        {
		    mode->subEntityTraits().setFillType(kAcGiFillAlways);
            ////	下底面	///
            mode->geometry().polygon(4, pArray.asArrayPtr());
            ////	上底面	///
            mode->geometry().polygon(4, &(pArray.asArrayPtr()[5]));
            AcGePoint3d verts[4];
            ////	前侧面	///
            verts[0] = pArray[0];
            verts[1] = pArray[1];
            verts[2] = pArray[6];
            verts[3] = pArray[5];
            mode->geometry().polygon(4, verts);
            ////	左侧面	///
            verts[1] = pArray[5];
            verts[2] = pArray[8];
            verts[3] = pArray[3];
            mode->geometry().polygon(4, verts);
            ////	右侧面	///
            verts[0] = pArray[7];
            verts[1] = pArray[6];
            verts[2] = pArray[1];
            verts[3] = pArray[2];
            mode->geometry().polygon(4, verts);
            ////	后侧面	///
            verts[1] = pArray[2];
            verts[2] = pArray[3];
            verts[3] = pArray[8];
            mode->geometry().polygon(4, verts);
        }
        break;
    case kAcGiStandardDisplay:
    case kAcGiSaveWorldDrawForR12:
    case kAcGiSaveWorldDrawForProxy:
        {
            int gsIndex = 0;
            AcGePoint3d pt[2];
            for(int i = 0; i < 4; ++i)
            {
                pt[0] = pArray[i];
                pt[1] = pArray[i + 1];
                mode->subEntityTraits().setSelectionMarker(++gsIndex);
                mode->geometry().polyline(2, pt);
            }
            for(i = 5; i < 9; ++i)
            {
                pt[0] = pArray[i];
                pt[1] = pArray[i + 1];
                mode->subEntityTraits().setSelectionMarker(++gsIndex);
                mode->geometry().polyline(2, pt);
            }
            for(i = 0; i < 4; ++i)
            {
                pt[0] = pArray[i];
                pt[1] = pArray[i + 5];
                mode->subEntityTraits().setSelectionMarker(++gsIndex);
                mode->geometry().polyline(2, pt);
            }
        }
        break;
    }*/
#endif
	return Adesk::kTrue;
}

////	通过调用此函数，在引用参数vertexArray 中返回Box 的各个顶点坐标，
////	以便在 viewportDraw 中利用。
////	底面返回5个顶点，顶面返回5个顶点。
Acad::ErrorStatus PDBox1::getVertices(AcGePoint3dArray& vertexArray) const
{
    assertReadEnabled();

    //清空vertexArray数组
    /*while(!vertexArray.isEmpty())
        vertexArray.removeLast();*/
	vertexArray.setLogicalLength(10);

    AcGeVector3d vectH = m_VectLength.crossProduct(m_VectWidth).normal();
    AcGePoint3d  p0,p1,p2,p3,p00,p11,p22,p33;

	////////////////////////////////////////////////////////
	////////	resolve the 8 vertext				////////
	AcGeVector3d vecAlongWidth,vecAlongLength;
	vectH = vectH * m_dHeight;//shinf 070105
	vecAlongLength = m_dLength* m_VectLength;
	vecAlongWidth = m_VectWidth * m_dWidth;

   /* p0 = m_ptP - m_VectLength * (m_dLength / 2.0) 
               - m_VectWidth * (m_dWidth / 2.0) 
               - vectH * (m_dHeight / 2.0);
	p00 = p0 + vectH * m_dHeight;
	p1 = p0 + m_VectWidth * m_dWidth;
	p11 = p1 + vectH * m_dHeight;
	p2 = p1 + m_VectLength * m_dLength;
	p22 = p2 + vectH * m_dHeight;
	p3 = p0 + m_VectLength * m_dLength;
	p33 = p3 + vectH * m_dHeight;*/
// 	p0 = m_ptP - (vecAlongLength + vecAlongWidth + vectH ) / 2.0;
// 	p00 = p0 + vectH;
// 	p1 = p0 + vecAlongWidth;
// 	p11 = p1 + vectH;
// 	p2 = p1 + vecAlongLength;
// 	p22 = p2 + vectH;
// 	p3 = p0 + vecAlongLength;
// 	p33 = p3 + vectH; 
	p0 = m_ptP - (vecAlongLength + vecAlongWidth + vectH ) / 2.0;
	p00 = p0 + vectH;
	p1 = p0 + vecAlongLength;
	p11 = p1 + vectH;
	p2 = p1 + vecAlongWidth;
	p22 = p2 + vectH;
	p3 = p0 + vecAlongWidth;
	p33 = p3 + vectH;

	vertexArray[0] = (p0);
    vertexArray[1] =(p1);
	vertexArray[2] =(p2);
	vertexArray[3] =(p3);
    vertexArray[4] =(p0);
	vertexArray[5] =(p00);
	vertexArray[6] =(p11);
	vertexArray[7] =(p22);
	vertexArray[8] =(p33);
    vertexArray[9] =(p00);
	
	return Acad::eOk;
}

Acad::ErrorStatus PDBox1::dwgOutFields(AcDbDwgFiler* filer)const
{
  assertReadEnabled();
	Acad::ErrorStatus es=Acad::eOk;

	// Call dwgOutFields from father class: PDPrimary3D
	if((es=PDPrimary3D::dwgOutFields(filer))!=Acad::eOk)
	{
		return es;
	}

	// Write version number.
	filer->writeItem((Adesk::UInt16) VERSION_PDBOX1);

	// Write the data members.
	filer->writeItem(m_dHeight);
	filer->writeItem(m_dLength);
	filer->writeItem(m_dWidth);
	filer->writeItem(m_ptP);
	filer->writeItem(m_VectLength);
	filer->writeItem(m_VectWidth);

	return filer->filerStatus();
}

Acad::ErrorStatus PDBox1::dwgInFields(AcDbDwgFiler* filer)
{
	assertWriteEnabled();
	Acad::ErrorStatus es=Acad::eOk;

	// Call dwgInFields from father class: PDPrimary3D
	if((es=PDPrimary3D::dwgInFields(filer))!=Acad::eOk)
	{
		return es;
	}

	// Read version number.
	Adesk::UInt16 version;
	filer->readItem(&version);
	if (version > VERSION_PDBOX1)
		return Acad::eMakeMeProxy;

	// Read the data members.
	switch (version)
	{
	case (1):
		filer->readItem(&m_dHeight);
		filer->readItem(&m_dLength);
		filer->readItem(&m_dWidth);
		filer->readItem(&m_ptP);
		filer->readItem(&m_VectLength);
    m_VectLength.normalize();
		filer->readItem(&m_VectWidth);
    m_VectWidth.normalize();
		break;
	}

#ifdef _USEAMODELER_
    if(m_3dGeom.isNull())
        createBody();
#endif

	return filer->filerStatus();
}

Acad::ErrorStatus PDBox1::dxfOutFields(AcDbDxfFiler* filer) const
{
    assertReadEnabled();
    Acad::ErrorStatus es;

	// Call dxfOutFields from father class: PDPrimary3D
    if ((es = PDPrimary3D::dxfOutFields(filer)) != Acad::eOk){
        return es;
	}

	// Write subclass marker.
    filer->writeItem(AcDb::kDxfSubclass, _T("PDBox1"));

	// Write version number.
	filer->writeItem(AcDb::kDxfInt16, (Adesk::UInt16) VERSION_PDBOX1);

	// Write data members.
    filer->writeItem(AcDb::kDxfXCoord, m_ptP);
    filer->writeItem(AcDb::kDxfNormalX, m_VectLength);
	filer->writeItem(AcDb::kDxfNormalX+1,m_VectWidth);
    filer->writeItem(AcDb::kDxfReal,m_dLength);
	filer->writeItem(AcDb::kDxfReal+1,m_dWidth);
	filer->writeItem(AcDb::kDxfReal+2,m_dHeight);

    return filer->filerStatus();
}

Acad::ErrorStatus PDBox1::dxfInFields(AcDbDxfFiler* filer)
{
    assertWriteEnabled();
    Acad::ErrorStatus es = Acad::eOk;
    struct resbuf rb;

	// Call dxfInFields from father class: PDPrimary3D
    if ((PDPrimary3D::dxfInFields(filer) != Acad::eOk)
        || !filer->atSubclassData(_T("PDBox1"))){
        return filer->filerStatus();
    }

	// Read version number.
	Adesk::UInt16 version;
	filer->readItem(&rb);
	if (rb.restype != AcDb::kDxfInt16) {
		filer->pushBackItem();
		filer->setError(Acad::eInvalidDxfCode,
			_T("nError: expected object version group code %d"),
		                 AcDb::kDxfInt16);
		return filer->filerStatus();
	} 
	else{
		version = rb.resval.rint;
		if (version > VERSION_PDBOX1)
			return Acad::eMakeMeProxy;
	}

	// Read the data members.
	switch (version){
	case (1):
		while ((es == Acad::eOk) && ((es = filer->readResBuf(&rb)) == Acad::eOk)){
	        switch (rb.restype) {
			case AcDb::kDxfXCoord:
			    m_ptP = asPnt3d(rb.resval.rpoint);
				break;

			case AcDb::kDxfNormalX:
				m_VectLength = asVec3d(rb.resval.rpoint);
        m_VectLength.normalize();
				break;

			case AcDb::kDxfNormalX+1:
				m_VectWidth = asVec3d(rb.resval.rpoint);
        m_VectWidth.normalize();
				break;

			case AcDb::kDxfReal:
				m_dLength=rb.resval.rreal;
				break;

			case AcDb::kDxfReal+1:
				m_dWidth=rb.resval.rreal;
				break;

			case AcDb::kDxfReal+2:
				m_dHeight=rb.resval.rreal;
				break;

			default:
            // An unrecognized group. Push it back so that
            // the subclass can read it again.
				filer->pushBackItem();
				es = Acad::eEndOfFile;
				break;
			}
		}
		break;
	}

#ifdef _USEAMODELER_
    if(m_3dGeom.isNull())
        createBody();
#endif

    // At this point the es variable must contain eEndOfFile
    // - either from readResBuf() or from pushback. If not,
    // it indicates that an error happened and we should
    // return immediately.
    //
    if (es != Acad::eEndOfFile)
        return Acad::eInvalidResBuf;

    return es;
}

Acad::ErrorStatus PDBox1::setParameters(const AcGePoint3d &ptP, const AcGeVector3d &vectL, 
                                const AcGeVector3d &vectW, double l,double w,double h)
{
    assertWriteEnabled();
    m_ptP = ptP;
    m_VectLength = vectL.normal();
    m_VectWidth = vectW.normal();
    m_dLength = fabs(l);
    if(m_dLength == 0)
        m_dLength = 1;
    m_dWidth = fabs(w);
    if(m_dWidth == 0)
        m_dWidth = 1;
    m_dHeight = fabs(h);
    if(m_dHeight == 0)
        m_dHeight = 1;

    if(m_VectLength.isZeroLength())
    {
        if(m_VectWidth.isParallelTo(AcGeVector3d(1, 0, 0)))
            m_VectLength.set(0, -1, 0);
        else
            m_VectLength.set(1, 0, 0);
    }

    if(m_VectWidth.isZeroLength())
    {
        if(m_VectLength.isParallelTo(AcGeVector3d(0, 1, 0)))
            m_VectWidth.set(-1, 0, 0);
        else
            m_VectWidth.set(0, 1, 0);
    }

#ifdef _USEAMODELER_
    createBody();
#endif
    return Acad::eOk;
}

double PDBox1::getHeight() const
{
	assertReadEnabled();
	return m_dHeight;
}

double PDBox1::getLength() const
{
	assertReadEnabled();
	return m_dLength;
}

double PDBox1::getWidth() const
{
	assertReadEnabled();
	return m_dWidth;
}

AcGePoint3d PDBox1::getpointP() const
{
	assertReadEnabled();
	return m_ptP;
}

AcGeVector3d PDBox1::getVectLength() const
{
	assertReadEnabled();
	return m_VectLength;
}

AcGeVector3d PDBox1::getVectWidth() const
{
	assertReadEnabled();
	return m_VectWidth;
}


#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDBox1::subGetOsnapPoints(
	AcDb::OsnapMode     osnapMode,
	Adesk::GsMarker     gsSelectionMark,
	const AcGePoint3d&  pickPoint,
	const AcGePoint3d&  lastPoint,
	const AcGeMatrix3d& viewXform,
	AcGePoint3dArray&   snapPoints,
	AcDbIntArray &   geomIds) const
#else
Acad::ErrorStatus PDBox1::getOsnapPoints(
	AcDb::OsnapMode       osnapMode,
	int                   gsSelectionMark,
	const AcGePoint3d&    pickPoint,
	const AcGePoint3d&    lastPoint,
	const AcGeMatrix3d&   viewXform,
	AcGePoint3dArray&     snapPoints,
	AcDbIntArray&         geomIds) const
#endif
{
    assertReadEnabled();

    if(!hasSnap())
        return Acad::eOk;

	int gsSelectionMark_int = (int)gsSelectionMark;
    if(gsSelectionMark_int == 0)
        return Acad::eOk;

    AcGePoint3dArray verArr;
    getVertices(verArr);

    if(verArr.length() <= 0)
        return Acad::eOk;

    AcGeVector3d viewDir(viewXform(Z, 0), viewXform(Z, 1),
                viewXform(Z, 2));

    switch(osnapMode)
    {
    case AcDb::kOsModeEnd:
        snapPoints.append(verArr);
	    break;
    case AcDb::kOsModeMid:
        {
            AcGePoint3d pt;
			//重新定义对象中点的捕捉方式，同时满足实体模型和线框模型的捕捉 
			//modified by szw 2009.11.18 : begin
			int i = 0;
			switch(gsSelectionMark_int)
			{
			case BOTTOM_PLANE:
				for(i = 0; i < 4; i++)
				{
					pt = verArr[i] + (verArr[i+1] - verArr[i]) / 2.0;
					snapPoints.append(pt);
				}
				break;
			case TOP_PLANE:
				for(i = 5; i < 9; i++)
				{
					pt = verArr[i] + (verArr[i+1] - verArr[i]) / 2.0;
					snapPoints.append(pt);
				}
				break;
			case FRONT_PLANE://[0,1,6,5]
				pt = verArr[0] + (verArr[1] - verArr[0]) / 2.0;
				snapPoints.append(pt);
				pt = verArr[1] + (verArr[6] - verArr[1]) / 2.0;
				snapPoints.append(pt);
				pt = verArr[6] + (verArr[5] - verArr[6]) / 2.0;
				snapPoints.append(pt);
				pt = verArr[0] + (verArr[5] - verArr[0]) / 2.0;
				snapPoints.append(pt);
				break;
			case LEFT_PLANE://[0,3,8,5]
				pt = verArr[0] + (verArr[3] - verArr[0]) / 2.0;
				snapPoints.append(pt);
				pt = verArr[3] + (verArr[8] - verArr[3]) / 2.0;
				snapPoints.append(pt);
				pt = verArr[8] + (verArr[5] - verArr[8]) / 2.0;
				snapPoints.append(pt);
				pt = verArr[0] + (verArr[5] - verArr[0]) / 2.0;
				snapPoints.append(pt);
				break;
			case RIGHT_PLANE://[1,2,7,6]
				pt = verArr[1] + (verArr[2] - verArr[1]) / 2.0;
				snapPoints.append(pt);
				pt = verArr[2] + (verArr[7] - verArr[2]) / 2.0;
				snapPoints.append(pt);
				pt = verArr[7] + (verArr[6] - verArr[7]) / 2.0;
				snapPoints.append(pt);
				pt = verArr[1] + (verArr[6] - verArr[1]) / 2.0;
				snapPoints.append(pt);
				break;
			case BACK_PLANE://[2,3,8,7]
				pt = verArr[2] + (verArr[3] - verArr[2]) / 2.0;
				snapPoints.append(pt);
				pt = verArr[3] + (verArr[8] - verArr[3]) / 2.0;
				snapPoints.append(pt);
				pt = verArr[8] + (verArr[7] - verArr[8]) / 2.0;
				snapPoints.append(pt);
				pt = verArr[2] + (verArr[7] - verArr[2]) / 2.0;
				snapPoints.append(pt);
				break;
			default:
				break;
			}
			//modified by szw 2009.11.18 : end
        }
        break;
    case AcDb::kOsModeCen:
	    {
		    AcGePoint3d cenPt[7];
		    AcGeVector3d zVector;
		    zVector=(m_VectLength.crossProduct(m_VectWidth)).normal();
			
			//重新定义对象中心点的捕捉方式，同时满足实体模型和线框模型的捕捉 
			//modified by szw 2009.11.18 : begin
            //下底面中心
		    cenPt[0] = verArr[0] + m_VectLength * m_dLength / 2.0 
                                 + m_VectWidth * m_dWidth / 2.0;
            //上底面中心
            cenPt[1] = cenPt[0] + zVector * m_dHeight;
            //前侧面中心
            cenPt[2] = verArr[0] + m_VectLength * m_dLength / 2.0 
                                 + zVector * m_dHeight / 2.0;
            //左侧面中心
            cenPt[3] = verArr[0] + m_VectWidth * m_dWidth / 2.0 
                                 + zVector * m_dHeight / 2.0;            
			//右侧面中心
            cenPt[4] = verArr[0] + m_VectLength * m_dLength
                                 + m_VectWidth * m_dWidth / 2.0
                                 + zVector * m_dHeight / 2.0;
            //后侧面中心
            cenPt[5] = cenPt[2] + m_VectWidth * m_dWidth;

		    //体的中心
            cenPt[6] = m_ptP;

			switch(gsSelectionMark_int)
			{
			case BOTTOM_PLANE:
				snapPoints.append(cenPt[0]);
				break;
			case TOP_PLANE:
				snapPoints.append(cenPt[1]);
				break;
			case FRONT_PLANE:
				snapPoints.append(cenPt[2]);
				break;
			case LEFT_PLANE:
				snapPoints.append(cenPt[3]);
				break;
			case RIGHT_PLANE:
				snapPoints.append(cenPt[4]);
				break;
			case BACK_PLANE:
				snapPoints.append(cenPt[5]);
				break;
			default:
				break;
			}
			//modified by szw 2009.11.18 : end
			snapPoints.append(cenPt[6]);
	    }
        break;
	case AcDb::kOsModeQuad:
		break;
    case AcDb::kOsModeNode:
		break;
    case AcDb::kOsModeIns:
        snapPoints.append(m_ptP);
        break;
    case AcDb::kOsModePerp:
		{
            // Create a semi-infinite line and find a point on it.
            //
            AcGeLine3d line;
            AcGeVector3d vec;
            AcGePoint3d pt;
			//重新定义对象垂直正交点的捕捉方式，同时满足实体模型和线框模型的捕捉 
			//modified by szw 2009.11.18 : begin
			int i = 0;
			switch(gsSelectionMark_int)
			{
			case BOTTOM_PLANE:
				for(i = 0; i < 4; i++)
				{
					vec = verArr[i] - verArr[i+1];
					vec.normalize();
					line.set(verArr[i], vec);
					pt = line.closestPointTo(lastPoint);
					snapPoints.append(pt);
				}
				break;
			case TOP_PLANE:
				for(i = 5; i < 9; i++)
				{
					vec = verArr[i] - verArr[i+1];
					vec.normalize();
					line.set(verArr[i], vec);
					pt = line.closestPointTo(lastPoint);
					snapPoints.append(pt);
				}
				break;
			case FRONT_PLANE://[0,1,6,5]
				vec = verArr[0] - verArr[1];
				vec.normalize();
				line.set(verArr[0], vec);
				pt = line.closestPointTo(lastPoint);
				snapPoints.append(pt);

				vec = verArr[1] - verArr[6];
				vec.normalize();
				line.set(verArr[1], vec);
				pt = line.closestPointTo(lastPoint);
				snapPoints.append(pt);

				vec = verArr[6] - verArr[5];
				vec.normalize();
				line.set(verArr[6], vec);
				pt = line.closestPointTo(lastPoint);
				snapPoints.append(pt);

				vec = verArr[0] - verArr[5];
				vec.normalize();
				line.set(verArr[0], vec);
				pt = line.closestPointTo(lastPoint);
				snapPoints.append(pt);
				break;
			case LEFT_PLANE://[0,3,8,5]
				vec = verArr[0] - verArr[3];
				vec.normalize();
				line.set(verArr[0], vec);
				pt = line.closestPointTo(lastPoint);
				snapPoints.append(pt);

				vec = verArr[3] - verArr[8];
				vec.normalize();
				line.set(verArr[3], vec);
				pt = line.closestPointTo(lastPoint);
				snapPoints.append(pt);

				vec = verArr[8] - verArr[5];
				vec.normalize();
				line.set(verArr[8], vec);
				pt = line.closestPointTo(lastPoint);
				snapPoints.append(pt);

				vec = verArr[0] - verArr[5];
				vec.normalize();
				line.set(verArr[0], vec);
				pt = line.closestPointTo(lastPoint);
				snapPoints.append(pt);
				break;
			case RIGHT_PLANE://[1,2,7,6]
				vec = verArr[1] - verArr[2];
				vec.normalize();
				line.set(verArr[1], vec);
				pt = line.closestPointTo(lastPoint);
				snapPoints.append(pt);

				vec = verArr[2] - verArr[7];
				vec.normalize();
				line.set(verArr[2], vec);
				pt = line.closestPointTo(lastPoint);
				snapPoints.append(pt);

				vec = verArr[7] - verArr[6];
				vec.normalize();
				line.set(verArr[7], vec);
				pt = line.closestPointTo(lastPoint);
				snapPoints.append(pt);

				vec = verArr[1] - verArr[6];
				vec.normalize();
				line.set(verArr[1], vec);
				pt = line.closestPointTo(lastPoint);
				snapPoints.append(pt);				
				break;
			case BACK_PLANE://[2,3,8,7]
				vec = verArr[2] - verArr[3];
				vec.normalize();
				line.set(verArr[2], vec);
				pt = line.closestPointTo(lastPoint);
				snapPoints.append(pt);

				vec = verArr[3] - verArr[8];
				vec.normalize();
				line.set(verArr[3], vec);
				pt = line.closestPointTo(lastPoint);
				snapPoints.append(pt);

				vec = verArr[8] - verArr[7];
				vec.normalize();
				line.set(verArr[8], vec);
				pt = line.closestPointTo(lastPoint);
				snapPoints.append(pt);

				vec = verArr[2] - verArr[7];
				vec.normalize();
				line.set(verArr[2], vec);
				pt = line.closestPointTo(lastPoint);
				snapPoints.append(pt);
				break;
			default:
				break;
			}
			//modified by szw 2009.11.18 : end
        }
	    break;
    case AcDb::kOsModeTan:
		break;
    case AcDb::kOsModeNear:
        {
            AcGeLineSeg3d lnsg;
            AcGePoint3d pt;
			//重新定义对象最近点的捕捉方式，同时满足实体模型和线框模型的捕捉 
			//modified by szw 2009.11.18 : begin
			int i = 0;
			switch(gsSelectionMark_int)
			{
			case BOTTOM_PLANE://[0,1,2,3]
				for(i = 0; i < 4; i++)
				{
					lnsg.set(verArr[i], verArr[i+1]);
					pt = lnsg.projClosestPointTo(pickPoint, viewDir);
					snapPoints.append(pt);
				}
				break;
			case TOP_PLANE://[5,6,7,8]
				for(i = 5; i < 9; i++)
				{
					lnsg.set(verArr[i], verArr[i+1]);
					pt = lnsg.projClosestPointTo(pickPoint, viewDir);
					snapPoints.append(pt);
				}
				break;
			case FRONT_PLANE://[0,1,6,5]
				lnsg.set(verArr[0], verArr[1]);
				pt = lnsg.projClosestPointTo(pickPoint, viewDir);
				snapPoints.append(pt);

				lnsg.set(verArr[1], verArr[6]);
				pt = lnsg.projClosestPointTo(pickPoint, viewDir);
				snapPoints.append(pt);

				lnsg.set(verArr[6], verArr[5]);
				pt = lnsg.projClosestPointTo(pickPoint, viewDir);
				snapPoints.append(pt);

				lnsg.set(verArr[0], verArr[5]);
				pt = lnsg.projClosestPointTo(pickPoint, viewDir);
				snapPoints.append(pt);
				break;
			case LEFT_PLANE://[0,3,8,5]
				lnsg.set(verArr[0], verArr[3]);
				pt = lnsg.projClosestPointTo(pickPoint, viewDir);
				snapPoints.append(pt);

				lnsg.set(verArr[3], verArr[8]);
				pt = lnsg.projClosestPointTo(pickPoint, viewDir);
				snapPoints.append(pt);

				lnsg.set(verArr[8], verArr[5]);
				pt = lnsg.projClosestPointTo(pickPoint, viewDir);
				snapPoints.append(pt);

				lnsg.set(verArr[0], verArr[5]);
				pt = lnsg.projClosestPointTo(pickPoint, viewDir);
				snapPoints.append(pt);
				break;
			case RIGHT_PLANE://[1,2,7,6]
				lnsg.set(verArr[1], verArr[2]);
				pt = lnsg.projClosestPointTo(pickPoint, viewDir);
				snapPoints.append(pt);

				lnsg.set(verArr[2], verArr[7]);
				pt = lnsg.projClosestPointTo(pickPoint, viewDir);
				snapPoints.append(pt);

				lnsg.set(verArr[7], verArr[6]);
				pt = lnsg.projClosestPointTo(pickPoint, viewDir);
				snapPoints.append(pt);

				lnsg.set(verArr[1], verArr[6]);
				pt = lnsg.projClosestPointTo(pickPoint, viewDir);
				snapPoints.append(pt);				
				break;
			case BACK_PLANE://[2,3,8,7]
				lnsg.set(verArr[2], verArr[3]);
				pt = lnsg.projClosestPointTo(pickPoint, viewDir);
				snapPoints.append(pt);

				lnsg.set(verArr[3], verArr[8]);
				pt = lnsg.projClosestPointTo(pickPoint, viewDir);
				snapPoints.append(pt);

				lnsg.set(verArr[8], verArr[7]);
				pt = lnsg.projClosestPointTo(pickPoint, viewDir);
				snapPoints.append(pt);

				lnsg.set(verArr[2], verArr[7]);
				pt = lnsg.projClosestPointTo(pickPoint, viewDir);
				snapPoints.append(pt);
				break;
			default:
				break;
			}
			//modified by szw 2009.11.18 : end
        }
        break;
    default:
        break;
    }

  return Acad::eOk;
}

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDBox1::subGetGripPoints(
	AcGePoint3dArray& gripPoints,
	AcDbIntArray& osnapModes,
	AcDbIntArray& geomIds) const
#else
Acad::ErrorStatus PDBox1::getGripPoints(
									   AcGePoint3dArray& gripPoints,
									   AcDbIntArray& osnapModes,
									   AcDbIntArray& geomIds) const
#endif
{
    assertReadEnabled();
/*	getVertices(gripPoints);*/
	return Acad::eOk; 
}

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDBox1::subExplode(AcDbVoidPtrArray& entitySet) const
#else
Acad::ErrorStatus PDBox1::explode(AcDbVoidPtrArray& entitySet) const
#endif
{
    assertReadEnabled();

    AcDb3dSolid *pBody;
    pBody = new AcDb3dSolid;
    if(pBody->createBox(m_dLength, m_dWidth, m_dHeight) != Acad::eOk)
    {
        delete pBody;
        return Acad::eNotApplicable;
    }

    AcGeMatrix3d mat;
    AcGeVector3d z;
    z = m_VectLength.crossProduct(m_VectWidth).normal();
    mat.setCoordSystem(m_ptP, m_VectLength, m_VectWidth, z);

    pBody->transformBy(mat);
    pBody->setPropertiesFrom(this);
    entitySet.append(pBody);
    	
	return Acad::eOk;
}

Acad::ErrorStatus PDBox1::explodeTo3DSolid(AcDb3dSolid* &p3dSolid) const
{
	assertReadEnabled();

	AcDb3dSolid *pBody;
	pBody = new AcDb3dSolid;
	if(pBody->createBox(m_dLength, m_dWidth, m_dHeight) != Acad::eOk)
	{
		delete pBody;
		return Acad::eNotApplicable;
	}

	AcGeMatrix3d mat;
	AcGeVector3d z;
	z = m_VectLength.crossProduct(m_VectWidth).normal();
	mat.setCoordSystem(m_ptP, m_VectLength, m_VectWidth, z);

	pBody->transformBy(mat);
	pBody->setPropertiesFrom(this);

	p3dSolid = pBody;

	return Acad::eOk;
}

void PDBox1::createBody()
{
    assertWriteEnabled();
//#ifdef _OBJECTARX2004_
    AcGePoint3d  p0;
    AcGeVector3d zTemp;
    AcGeMatrix3d mat;
    zTemp = m_VectLength.crossProduct(m_VectWidth).normal();
    p0 = m_ptP - m_VectLength * (m_dLength / 2.0) - m_VectWidth * (m_dWidth / 2.0) - zTemp * (m_dHeight / 2.0);
    mat.setCoordSystem(p0, m_VectLength, m_VectWidth, zTemp);

    Vector3d GeomVec(m_dLength, m_dWidth, m_dHeight);
    m_3dGeom = Body::box(Point3d::kNull, GeomVec);
    m_3dGeom.transform(*(Transf3d*)&mat);
//#endif
}

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDBox1::subGetGeomExtents(AcDbExtents& extents) const
#else
Acad::ErrorStatus PDBox1::getGeomExtents(AcDbExtents& extents) const
#endif
{
    assertReadEnabled();
    extents.set(m_ptP, m_ptP);
    AcGePoint3dArray ptarr;
    getVertices(ptarr);
    int i, arrLen = ptarr.length();
    for(i = 0; i < arrLen; ++i)
        extents.addPoint(ptarr[i]);
    return Acad::eOk;
}

#ifdef _OBJECTARX2010_
void PDBox1::subList() const
#else
void PDBox1::list() const
#endif
{
    assertReadEnabled();
#ifdef _OBJECTARX2010_
	PDPrimary3D::subList();
#else
	PDPrimary3D::list();
#endif
    ads_printf(LANGUAGE_DEFINE_MACRO_V25_4732/*"\n 插入点(%f,%f,%f)\n"*/,m_ptP.x,m_ptP.y,m_ptP.z);
    ads_printf(LANGUAGE_DEFINE_MACRO_V25_4733/*" 插入X轴(%f,%f,%f)\n"*/,m_VectLength.x,m_VectLength.y,m_VectLength.z);
    ads_printf(LANGUAGE_DEFINE_MACRO_V25_4734/*" 插入Y轴(%f,%f,%f)\n"*/,m_VectWidth.x,m_VectWidth.y,m_VectWidth.z);
    ads_printf(LANGUAGE_DEFINE_MACRO_V25_4736/*" 长度=%.3f 宽度=%.3f 高度=%.3f\n\n"*/,m_dLength,m_dWidth,m_dHeight);
    return ;
}
