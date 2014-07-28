// PDBox.cpp: implementation of the PDBox class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "language25.h"

#include <float.h>

#include "PDBox.h"
#include <dbproxy.h>
#include <acgi.h>
#include <dbsol3d.h>
#include <geassign.h>
#include <gebndpln.h>
//#include <dbregion.h>
#include <dbapserv.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define VERSION_PDBOX 2

//Adesk::Boolean append(AcDbEntity* pEntity);

/*
static enum  colors{kCyan,kGreen};
static Acad::ErrorStatus intLine   (const PDBox*          poly, 
                                    const AcGeLine3d        line,
                                          AcGePoint3dArray& points);
*/
static Acad::ErrorStatus intLine   (const PDBox*         poly, 
                                    const AcDbLine*         line,
                                          AcDb::Intersect   intType,
                                    const AcGePlane*        projPlane,
                                          AcGePoint3dArray& points);

static Acad::ErrorStatus intLine   (const PDBox*         poly, 
                                    const AcGeLineSeg3d     line,
                                          AcDb::Intersect   intType,
                                    const AcGePlane*        projPlane,
                                          AcGePoint3dArray& points);

static Acad::ErrorStatus intArc    (const PDBox*         poly, 
                                    const AcDbArc*          arc,
                                          AcDb::Intersect   intType,
                                    const AcGePlane*        projPlane,
                                          AcGePoint3dArray& points);

static Acad::ErrorStatus intArc    (const PDBox*         poly, 
                                    const AcGeCircArc3d     arc,
                                          AcDb::Intersect   intType,
                                    const AcGePlane*        projPlane,
                                          AcGePoint3dArray& points);
/*
static Acad::ErrorStatus intCircle (const PDBox*         poly, 
                                    const AcDbCircle*       circle,
                                          AcDb::Intersect   intType,
                                    const AcGePlane*        projPlane,
                                          AcGePoint3dArray& points);

static Acad::ErrorStatus intCircle (const PDBox*         poly, 
                                    const AcGeCircArc3d     circle,
                                          AcDb::Intersect   intType,
                                    const AcGePlane*        projPlane,
                                          AcGePoint3dArray& points);

static Acad::ErrorStatus intPline  (const PDBox*         poly, 
                                          AcDb2dPolyline*   pline,
                                          AcDb::Intersect   intType,
                                    const AcGePlane*        projPlane,
                                          AcGePoint3dArray& points);

static Acad::ErrorStatus intPline  (const PDBox*         poly, 
                                          AcDb3dPolyline*   pline,
                                          AcDb::Intersect   intType,
                                    const AcGePlane*        projPlane,
                                          AcGePoint3dArray& points);

*/
bool isInBoundPlane(const AcGePlane &plane, int faceIdx, const PDBox *&poly, const AcGePoint3d &pt);

#ifdef _OBJECTARX2000_
ACRX_DXF_DEFINE_MEMBERS(PDBox, PDPrimary3D,
                        AcDb::kDHL_CURRENT, AcDb::kMReleaseCurrent,
                        AcDbProxyEntity::kAllAllowedBits,
                        PDBOX, Gle);
#else
ACRX_DXF_DEFINE_MEMBERS(PDBox, PDPrimary3D, AcDbProxyEntity::kAllAllowedBits, PDBOX, Gle);
#endif

PDBox::PDBox(bool HasSnap/* = false*/) : PDPrimary3D(HasSnap)
{
	m_porign = AcGePoint3d::kOrigin;
	m_dlength = 1;
	m_dwidth = 1;
	m_dheight = 1;
	m_Vectx.set(1, 0, 0);
	m_Vecty.set(0, 1, 0);
    m_Vectz.set(0, 0, 1);
#ifdef _USEAMODELER_
    createBody();
#endif
}

PDBox::PDBox(const AcGePoint3d &orn, double L, double W, double H,
		     const AcGeVector3d &xVec, const AcGeVector3d &yVec, 
             bool HasSnap/* = false*/, int CorrType/* = 1*/) : PDPrimary3D(HasSnap)
{
	m_porign = orn;

	m_dlength = fabs(L);
    if(m_dlength == 0)
        m_dlength = 1;

	m_dwidth = fabs(W);
    if(m_dwidth == 0)
        m_dwidth = 1;

	m_dheight = fabs(H);
    if(m_dheight == 0)
        m_dheight = 1;

	m_Vectx = xVec.normal();
    m_Vecty = yVec.normal();

    if(m_Vectx.isZeroLength())
    {
        if(m_Vecty.isParallelTo(AcGeVector3d(1, 0, 0)))
            m_Vectx.set(0, -1, 0);
        else
            m_Vectx.set(1, 0, 0);
    }

    if(m_Vecty.isZeroLength())
    {
        if(m_Vectx.isParallelTo(AcGeVector3d(0, 1, 0)))
            m_Vecty.set(-1, 0, 0);
        else
            m_Vecty.set(0, 1, 0);
    }
    if(CorrType)
        m_Vectz = m_Vectx.crossProduct(m_Vecty).normalize();
    else
        m_Vectz = m_Vecty.crossProduct(m_Vectx).normalize();
#ifdef _USEAMODELER_
    createBody();
#endif
}

PDBox::~PDBox()
{

}

/////worldDraw分为线框模式和渲染模式分别设计。返回false
///// 转入执行viewportDraw 函数中的代码
#ifdef _OBJECTARX2010_
Adesk::Boolean PDBox::subWorldDraw(AcGiWorldDraw* mode)
#else
Adesk::Boolean PDBox::worldDraw(AcGiWorldDraw* mode)
#endif
{
	assertReadEnabled();

	//if(acdbHostApplicationServices()->workingDatabase()->useri1() == 1)
		//return Adesk::kTrue;

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
		a[0] = m_dheight;
		a[1] = m_dwidth;
		a[2] = m_dlength;
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
		Precision = (int)(PI / acos(1.0 - mode->deviation(kAcGiMaxDevForCircle, m_porign)
			* 2.0 / mMax));
		if(Precision < PRIMARY3D_SIMPLIZE_SCALE)
		{	
			AcGePoint3d pt[5];
			//////////////////////////////////////////////////////////////////////////
			//draw lines
			int i;
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
			//////////////////////////////////////////////////////////////////////////
		/*	pt[0] =  (pArray[0] + pArray[2].asVector() + m_dheight * m_Vectz) / 2.0;
			pt[1] = pt[0];
			mode->geometry().polyline(2,pt);*/
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
			if(mode->geometry().polygon(4, pArray.asArrayPtr()) )
			{
				return Adesk::kTrue;
			}
			////	上底面	///
			//added by szw 2009.11.18 : begin
			mode->subEntityTraits().setSelectionMarker(TOP_PLANE);
			//added by szw 2009.11.18 : end
			if(mode->geometry().polygon(4, &(pArray.asArrayPtr()[5])))
			{
				
				return Adesk::kTrue;
			}
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
			{
				return Adesk::kTrue;
			}
			////	左侧面	///
			verts[1] = pArray[5];
			verts[2] = pArray[8];
			verts[3] = pArray[3];
			//added by szw 2009.11.18 : begin
			mode->subEntityTraits().setSelectionMarker(LEFT_PLANE);
			//added by szw 2009.11.18 : end
			if(mode->geometry().polygon(4, verts))
			{
				return Adesk::kTrue;
			}
			////	右侧面	///
			verts[0] = pArray[7];
			verts[1] = pArray[6];
			verts[2] = pArray[1];
			verts[3] = pArray[2];
			//added by szw 2009.11.18 : begin
			mode->subEntityTraits().setSelectionMarker(RIGHT_PLANE);
			//added by szw 2009.11.18 : end
			if(mode->geometry().polygon(4, verts))
			{
				return Adesk::kTrue;
			}
			////	后侧面	///
			verts[1] = pArray[2];
			verts[2] = pArray[3];
			verts[3] = pArray[8];
			//added by szw 2009.11.18 : begin
			mode->subEntityTraits().setSelectionMarker(BACK_PLANE);
			//added by szw 2009.11.18 : end
			if(mode->geometry().polygon(4, verts))
			{
				return Adesk::kTrue;
			}
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
                if(!(_isnan(pt[0].x) || _isnan(pt[0].y) || _isnan(pt[0].z)
                    || _isnan(pt[1].x) || _isnan(pt[1].y) || _isnan(pt[1].z)))
                {
					//统一线框模型和实体模型的GS标记
					//modified by szw 2009.11.18 : begin
					mode->subEntityTraits().setSelectionMarker(BOTTOM_PLANE);
					//modified by szw 2009.11.18 : end
                    if (mode->geometry().polyline(2, pt))
					    break;
                }
			}
			for(i = 5; i < 9; ++i)
			{
				pt[0] = pArray[i];
				pt[1] = pArray[i + 1];
                if(!(_isnan(pt[0].x) || _isnan(pt[0].y) || _isnan(pt[0].z)
                    || _isnan(pt[1].x) || _isnan(pt[1].y) || _isnan(pt[1].z)))
                {
					//modified by szw 2009.11.18 : begin
					mode->subEntityTraits().setSelectionMarker(TOP_PLANE);
					//modified by szw 2009.11.18 : end
					if (mode->geometry().polyline(2, pt))
					    break;
                }
			}
			for(i = 0; i < 4; ++i)
			{
				pt[0] = pArray[i];
				pt[1] = pArray[i + 5];
                if(!(_isnan(pt[0].x) || _isnan(pt[0].y) || _isnan(pt[0].z)
                    || _isnan(pt[1].x) || _isnan(pt[1].y) || _isnan(pt[1].z)))
                {
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
		}
#ifndef _ALWAYSSHOWWIRE_
		break;
	}
#endif
#endif
	return Adesk::kTrue;
}
/*Adesk::Boolean PDBox::worldDraw(AcGiWorldDraw *mode)
{
	assertReadEnabled();

#ifdef _USEAMODELER_
    PDPrimary3D::worldDraw(mode);
#endif
    if (mode->regenAbort()) {
        return Adesk::kTrue;
    }
#ifndef _USEAMODELER_

    AcGePoint3dArray pArray;
    getVertices(pArray);
    
    switch (mode->regenType())
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
                if (mode->geometry().polyline(2, pt))
					break;
            }
            for(i = 5; i < 9; ++i)
            {
                pt[0] = pArray[i];
                pt[1] = pArray[i + 1];
                mode->subEntityTraits().setSelectionMarker(++gsIndex);
                if (mode->geometry().polyline(2, pt))
					break;
            }
            for(i = 0; i < 4; ++i)
            {
                pt[0] = pArray[i];
                pt[1] = pArray[i + 5];
                mode->subEntityTraits().setSelectionMarker(++gsIndex);
                if (mode->geometry().polyline(2, pt))
					break;
            }
        }
        break;
    }
#endif
  return Adesk::kTrue;
}*/

///////////////////////////////////////////////////////////////
//////	用于对物体进行移动，旋转等矩阵操作。			///////
//////	对顶点坐标，方向矢量分别操作					///////
///////////////////////////////////////////////////////////////
#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDBox::subTransformBy(const AcGeMatrix3d &xform)
#else
Acad::ErrorStatus PDBox::transformBy(const AcGeMatrix3d &xform)
#endif
{
    assertWriteEnabled();
	m_porign.transformBy(xform);
	m_Vectx.transformBy(xform);
    m_Vectx.normalize();
	m_Vecty.transformBy(xform);
    m_Vecty.normalize();
    m_Vectz.transformBy(xform);
    m_Vectz.normalize();
	m_dheight *= xform.scale();
	m_dlength *= xform.scale();
	m_dwidth *= xform.scale();

#ifdef _OBJECTARX2010_
	return PDPrimary3D::subTransformBy(xform);
#else
	return PDPrimary3D::transformBy(xform);
#endif
}

Acad::ErrorStatus PDBox::setParameters(const AcGePoint3d &orn, 
                                       double L, double W, double H, 
                                       const AcGeVector3d &xVec, 
                                       const AcGeVector3d &yVec, 
                                       int CorrType/* = 1*/)
{
    assertWriteEnabled();

    m_porign = orn;
    
    m_dlength = fabs(L);
    if(m_dlength == 0)
        m_dlength = 1;
    
    m_dwidth = fabs(W);
    if(m_dwidth == 0)
        m_dwidth = 1;
    
    m_dheight = fabs(H);
    if(m_dheight == 0)
        m_dheight = 1;
    
    m_Vectx = xVec.normal();
    m_Vecty = yVec.normal();

    if(m_Vectx.isZeroLength())
    {
        if(m_Vecty.isParallelTo(AcGeVector3d(1, 0, 0)))
            m_Vectx.set(0, -1, 0);
        else
            m_Vectx.set(1, 0, 0);
    }
    
    if(m_Vecty.isZeroLength())
    {
        if(m_Vectx.isParallelTo(AcGeVector3d(0, 1, 0)))
            m_Vecty.set(-1, 0, 0);
        else
            m_Vecty.set(0, 1, 0);
    }
    if(CorrType)
        m_Vectz = m_Vectx.crossProduct(m_Vecty).normalize();
    else
        m_Vectz = m_Vecty.crossProduct(m_Vectx).normalize();
#ifdef _USEAMODELER_
    createBody();
#endif
    return Acad::eOk;
}

double PDBox::getheight() const
{
	assertReadEnabled();
	return m_dheight;
}

double PDBox::getlength() const
{
	assertReadEnabled();
	return m_dlength;
}

double PDBox::getwidth() const
{
	assertReadEnabled();
	return m_dwidth;
}

AcGePoint3d PDBox::getOrign() const
{
	assertReadEnabled();
	return m_porign;
}

AcGePoint3d PDBox::getCenter()const
{
	assertReadEnabled();
	AcGePoint3d result = m_porign;
	result += m_Vectx*(m_dlength/2);
	result += m_Vecty*(m_dwidth/2);
	result += m_Vectz*(m_dheight/2);
	return result;
}

AcGeVector3d PDBox::getXvec() const
{
	assertReadEnabled();
	return m_Vectx;
}

AcGeVector3d PDBox::getYvec() const
{
	assertReadEnabled();
	return m_Vecty;
}

AcGeVector3d PDBox::getZvec() const
{
    assertReadEnabled();
    return m_Vectz;
}

Acad::ErrorStatus PDBox::dwgOutFields(AcDbDwgFiler* filer)const
{
    assertReadEnabled();
	Acad::ErrorStatus es=Acad::eOk;

	// Call dwgOutFields from father class: PDPrimary3D
	if((es=PDPrimary3D::dwgOutFields(filer))!=Acad::eOk)
	{
		return es;
	}

	// Write version number.
	filer->writeItem((Adesk::UInt16) VERSION_PDBOX);

    // version 2
    filer->writeItem(m_Vectz);

	// Write the data members.
	filer->writeItem(m_dheight);
	filer->writeItem(m_dlength);
	filer->writeItem(m_dwidth);
	filer->writeItem(m_porign);
	filer->writeItem(m_Vectx);
	filer->writeItem(m_Vecty);

	return filer->filerStatus();
}

Acad::ErrorStatus PDBox::dwgInFields(AcDbDwgFiler* filer)
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
	if (version > VERSION_PDBOX)
		return Acad::eMakeMeProxy;

	// Read the data members.

    if(version >= 2)
        filer->readItem(&m_Vectz);
    if(version >= 1)
    {
	    filer->readItem(&m_dheight);
	    filer->readItem(&m_dlength);
	    filer->readItem(&m_dwidth);
	    filer->readItem(&m_porign);
	    filer->readItem(&m_Vectx);
	    filer->readItem(&m_Vecty);
        if(version == 1)
            m_Vectz = m_Vectx.crossProduct(m_Vecty).normalize();
    }

#ifdef _USEAMODELER_
    if(m_3dGeom.isNull())
        createBody();
#endif

	return filer->filerStatus();
}


Acad::ErrorStatus PDBox::dxfOutFields(AcDbDxfFiler* filer) const
{
    assertReadEnabled();
    Acad::ErrorStatus es;

    if ((es = PDPrimary3D::dxfOutFields(filer))
        != Acad::eOk)
    {
        return es;
    }

	// Write subclass marker.
    filer->writeItem(AcDb::kDxfSubclass, _T("PDBox"));

	// Write version number.
	filer->writeItem(AcDb::kDxfInt16, (Adesk::UInt16) VERSION_PDBOX);

    // version 2
    filer->writeItem(AcDb::kDxfNormalX+2,m_Vectz);

	// Write data members.
    filer->writeItem(AcDb::kDxfXCoord, m_porign);
    filer->writeItem(AcDb::kDxfNormalX, m_Vectx);
	filer->writeItem(AcDb::kDxfNormalX+1,m_Vecty);
    filer->writeItem(AcDb::kDxfReal,m_dlength);
	filer->writeItem(AcDb::kDxfReal+1,m_dwidth);
	filer->writeItem(AcDb::kDxfReal+2,m_dheight);

    return filer->filerStatus();
}

Acad::ErrorStatus PDBox::dxfInFields(AcDbDxfFiler* filer)
{
    assertWriteEnabled();
    Acad::ErrorStatus es = Acad::eOk;
    struct resbuf rb;

    if ((PDPrimary3D::dxfInFields(filer) != Acad::eOk)
        || !filer->atSubclassData(_T("PDBox")))
    {
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
	else {
		version = rb.resval.rint;
		if (version > VERSION_PDBOX)
			return Acad::eMakeMeProxy;
	}

    if(version >= 2)
    {
        filer->readItem(&rb);
        if (rb.restype != AcDb::kDxfNormalX+2) {
            filer->pushBackItem();
            filer->setError(Acad::eInvalidDxfCode,
				_T("nError: expected object version group code %d"),
                AcDb::kDxfNormalX+2);
            return filer->filerStatus();
        } 
        else {
            m_Vectz = asVec3d(rb.resval.rpoint);
        }
    }
	// Read the data members.
	if(version >= 1)
    {
	    while ((es == Acad::eOk) && ((es = filer->readResBuf(&rb)) == Acad::eOk)){

			switch (rb.restype){
			case AcDb::kDxfXCoord:
				m_porign = asPnt3d(rb.resval.rpoint);
				break;

			 case AcDb::kDxfNormalX:
			    m_Vectx = asVec3d(rb.resval.rpoint);
			    break;
				
			 case AcDb::kDxfNormalX+1:
				m_Vecty = asVec3d(rb.resval.rpoint);
				break;

			 case AcDb::kDxfReal:
				m_dlength=rb.resval.rreal;
				break;

			 case AcDb::kDxfReal+1:
				m_dwidth=rb.resval.rreal;
				break;

			 case AcDb::kDxfReal+2:
				m_dheight=rb.resval.rreal;
				break;

			 default:
			   // An unrecognized group. Push it back so that
			   // the subclass can read it again.
				filer->pushBackItem();
				es = Acad::eEndOfFile;
				break;
			}
		}
        if(version == 1)
            m_Vectz = m_Vectx.crossProduct(m_Vecty).normalize();
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

    return filer->filerStatus();
}

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDBox::subIntersectWith(
	const AcDbEntity* ent,
	AcDb::Intersect intType, 
	AcGePoint3dArray& points,
	Adesk::GsMarker thisGsMarker, 
	Adesk::GsMarker otherGsMarker) const
#else
Acad::ErrorStatus PDBox::intersectWith(
	const AcDbEntity* ent,
	AcDb::Intersect intType,
	AcGePoint3dArray& points,
	int thisGsMarker,
	int otherGsMarker) const
#endif
{
    assertReadEnabled();
    Acad::ErrorStatus es = Acad::eOk;
    AcGePoint3dArray pts;
	if (ent == NULL)
        return Acad::eNullEntityPointer;

   if (ent->isKindOf(AcDbLine::desc())) {
        if ((es = intLine(this, AcDbLine::cast(ent),
            intType, NULL, points)) != Acad::eOk)
        {
            return es;
        }
    } else if (ent->isKindOf(AcDbArc::desc())) {
        if ((es = intArc(this, AcDbArc::cast(ent), intType,
            NULL, points)) != Acad::eOk)
        {
            return es;
        }
    }/* else if (ent->isKindOf(AcDbCircle::desc())) {
        if ((es = intCircle(this, AcDbCircle::cast(ent),
            intType, NULL, points)) != Acad::eOk)
        {
            return es;
        }
    } else if (ent->isKindOf(AcDb2dPolyline::desc())) {
        if ((es = intPline(this, AcDb2dPolyline::cast(ent),
            intType, NULL, points)) != Acad::eOk)
        {
            return es;
        }
    } else if (ent->isKindOf(AcDb3dPolyline::desc())) {
        if ((es = intPline(this, AcDb3dPolyline::cast(ent),
            intType, NULL, points)) != Acad::eOk)
        {
            return es;
        }
    } else {
        AcGePoint3dArray vertexArray;
        if ((es = this->getVertices(vertexArray))
            != Acad::eOk)
        {
            return es;
        }
        if (intType == AcDb::kExtendArg
            || intType == AcDb::kExtendBoth)
        {
            intType = AcDb::kExtendThis;
        }

        int startLength = points.length();
        AcDbLine *pAcam_dlengthine;
        for (int i = 0; i < vertexArray.length() - 1; i++) {
            pAcam_dlengthine = new AcDbLine();
            pAcam_dlengthine->setStartPoint(vertexArray[i]);
            pAcam_dlengthine->setEndPoint(vertexArray[i + 1]);
        //    pAcam_dlengthine->setNormal(this->normal());

            if ((es = ent->intersectWith(pAcam_dlengthine, intType,
                points)) != Acad::eOk)
            {
                for (int j = points.length(); j > startLength; j--)
                    points.removeLast();
                delete pAcam_dlengthine;
                return es;
            }
            delete pAcam_dlengthine;
        }
    }*/
    return es;
}


/*
static Acad::ErrorStatus intLine(const PDBox*          poly, 
                                 const AcGeLine3d        line,
                                       AcGePoint3dArray& points)
{
    Acad::ErrorStatus es = Acad::eOk;

    AcGePoint3dArray vertexArray;
    if ((es = poly->getVertices(vertexArray)) != Acad::eOk) {
        return es;
    }

    AcGeLineSeg3d tlnsg;
    AcGePoint3d   pt;

    for (int i = 0; i < vertexArray.length() - 1; i++) {

        tlnsg.set(vertexArray[i], vertexArray[i + 1]);

        if (!tlnsg.intersectWith(line, pt)) {
            continue;
        } else {
            points.append(pt);
        }
    }

    return es;
}
*/

static Acad::ErrorStatus intLine(const PDBox*         poly, 
                                 const AcDbLine*         line,
                                       AcDb::Intersect   intType,
                                 const AcGePlane*        projPlane,
                                       AcGePoint3dArray& points)
{
    Acad::ErrorStatus es = Acad::eOk;

    AcGeLineSeg3d lnsg(line->startPoint(), line->endPoint());
    es = intLine(poly, lnsg, intType, projPlane, points);

    return es;
}


static Acad::ErrorStatus intLine(const PDBox*         poly, 
                                 const AcGeLineSeg3d     lnsg,
                                       AcDb::Intersect   intType,
                                 const AcGePlane*        projPlane,
                                       AcGePoint3dArray& points)
{
    Acad::ErrorStatus es = Acad::eOk;

    int faceIdx = 0;
    if(poly->getheight() < poly->getwidth() && poly->getheight() < poly->getlength())
        faceIdx = 0;
    else if(poly->getwidth() < poly->getheight() && poly->getwidth() < poly->getlength())
        faceIdx = 2;
    else if(poly->getlength() < poly->getheight() && poly->getlength() < poly->getwidth())
        faceIdx = 4;

    AcGePoint3dArray pArray; 
    poly->getVertices(pArray);
    //std::vector<AcGeBoundedPlane> BndPlanes;
    //AcGeBoundedPlane BndPlaneTemp;
    std::vector<AcGePlane> BndPlanes;
    AcGePlane BndPlaneTemp;
    // 下底面
    if(faceIdx == 0)
    {
        BndPlaneTemp.set(pArray[0], pArray[1], pArray[3]);
        BndPlanes.push_back(BndPlaneTemp);
    }
    // 上底面
    //BndPlaneTemp.set(pArray[5], pArray[6], pArray[8]);
    //BndPlanes.push_back(BndPlaneTemp);
    // 前侧面
    else if(faceIdx == 2)
    {
        BndPlaneTemp.set(pArray[0], pArray[1], pArray[5]);
        BndPlanes.push_back(BndPlaneTemp);
    }
    // 后侧面
    //BndPlaneTemp.set(pArray[3], pArray[2], pArray[8]);
    //BndPlanes.push_back(BndPlaneTemp);
    // 左侧面
    else if(faceIdx == 4)
    {
        BndPlaneTemp.set(pArray[0], pArray[3], pArray[5]);
        BndPlanes.push_back(BndPlaneTemp);
    }
    // 右侧面
    //BndPlaneTemp.set(pArray[1], pArray[2], pArray[6]);
    //BndPlanes.push_back(BndPlaneTemp);

    AcGeLine3d aline(lnsg.startPoint(), lnsg.endPoint());
    AcGePoint3d   pt;
    AcGePoint3d   dummy;

    for (size_t i = 0; i < BndPlanes.size(); i++) {
        if (intType == AcDb::kExtendArg || intType == AcDb::kExtendBoth) {
            if (!BndPlanes[i].intersectWith(aline, pt)) {
                continue;
            } else {
                if(isInBoundPlane(BndPlanes[i], faceIdx, poly, pt))
                    points.append(pt);
            }
        } else {
            if (!BndPlanes[i].intersectWith(lnsg, pt)) {
                continue;
            } else {
                if(isInBoundPlane(BndPlanes[i], faceIdx, poly, pt))
                    points.append(pt);
            }
        }
    }

    return es;
}

static Acad::ErrorStatus intArc(const PDBox*         poly, 
                                const AcDbArc*          arc,
                                      AcDb::Intersect   intType,
                                const AcGePlane*        projPlane,
                                      AcGePoint3dArray& points)
{
    Acad::ErrorStatus es = Acad::eOk;

    AcGeCircArc3d aarc(arc->center(), arc->normal(), 
                       arc->normal().perpVector(), arc->radius(), 
                       arc->startAngle(), arc->endAngle());
    es = intArc(poly, aarc, intType, projPlane, points);

    return es;
}


static Acad::ErrorStatus intArc(const PDBox*         poly, 
                                const AcGeCircArc3d     arc,
                                      AcDb::Intersect   intType,
                                const AcGePlane*        projPlane,
                                      AcGePoint3dArray& points)
{
    Acad::ErrorStatus es = Acad::eOk;

    int faceIdx = 0;
    if(poly->getheight() < poly->getwidth() && poly->getheight() < poly->getlength())
        faceIdx = 0;
    else if(poly->getwidth() < poly->getheight() && poly->getwidth() < poly->getlength())
        faceIdx = 2;
    else if(poly->getlength() < poly->getheight() && poly->getlength() < poly->getwidth())
        faceIdx = 4;

    AcGePoint3dArray pArray; 
    poly->getVertices(pArray);
    //std::vector<AcGeBoundedPlane> BndPlanes;
    //AcGeBoundedPlane BndPlaneTemp;
    std::vector<AcGePlane> BndPlanes;
    AcGePlane BndPlaneTemp;
    // 下底面
    if(faceIdx == 0)
    {
        BndPlaneTemp.set(pArray[0], pArray[1], pArray[3]);
        BndPlanes.push_back(BndPlaneTemp);
    }
    // 上底面
    //BndPlaneTemp.set(pArray[5], pArray[6], pArray[8]);
    //BndPlanes.push_back(BndPlaneTemp);
    // 前侧面
    else if(faceIdx == 2)
    {
        BndPlaneTemp.set(pArray[0], pArray[1], pArray[5]);
        BndPlanes.push_back(BndPlaneTemp);
    }
    // 后侧面
    //BndPlaneTemp.set(pArray[3], pArray[2], pArray[8]);
    //BndPlanes.push_back(BndPlaneTemp);
    // 左侧面
    else if(faceIdx == 4)
    {
        BndPlaneTemp.set(pArray[0], pArray[3], pArray[5]);
        BndPlanes.push_back(BndPlaneTemp);
    }
    // 右侧面
    //BndPlaneTemp.set(pArray[1], pArray[2], pArray[6]);
    //BndPlanes.push_back(BndPlaneTemp);

    AcGeCircArc3d  acircle(arc.center(), arc.normal(), arc.radius());
    AcGeLineSeg3d lnsg;
    AcGePoint3d   pt1, pt2;
    AcGePoint3d   dummy1, dummy2;
    int           howMany;

    for (size_t i = 0; i < BndPlanes.size(); i++) {
        if (intType == AcDb::kExtendArg || intType == AcDb::kExtendBoth) {
            if (!acircle.intersectWith(BndPlanes[i], howMany, pt1, pt2)) {
                continue;
            } else {
                if (howMany > 1) {
                    if(isInBoundPlane(BndPlanes[i], faceIdx, poly, pt1))
                        points.append(pt1);
                    if(isInBoundPlane(BndPlanes[i], faceIdx, poly, pt2))
                        points.append(pt2);
                } else {
                    if(isInBoundPlane(BndPlanes[i], faceIdx, poly, pt1))
                        points.append(pt1);
                }
            }
        } else {
            if (!arc.intersectWith(BndPlanes[i], howMany, pt1, pt2)) {
                continue;
            } else {
                if (howMany > 1) {
                    if(isInBoundPlane(BndPlanes[i], faceIdx, poly, pt1))
                        points.append(pt1);
                    if(isInBoundPlane(BndPlanes[i], faceIdx, poly, pt2))
                        points.append(pt2);
                } else {
                    if(isInBoundPlane(BndPlanes[i], faceIdx, poly, pt1))
                        points.append(pt1);
                }
            }
        }
    }

    return es;
}

/*
static Acad::ErrorStatus intCircle(const PDBox*          poly, 
                                   const AcDbCircle*       circle,
                                         AcDb::Intersect   intType,
                                   const AcGePlane*        projPlane,
                                         AcGePoint3dArray& points)
{
    Acad::ErrorStatus es = Acad::eOk;

    AcGeCircArc3d  acircle(circle->center(), circle->normal(),
                          circle->radius());
    es = intCircle(poly, acircle, intType, projPlane, points);

    return es;
}


static Acad::ErrorStatus intCircle(const PDBox*         poly, 
                                   const AcGeCircArc3d     circle,
                                         AcDb::Intersect   intType,
                                   const AcGePlane*        projPlane,
                                         AcGePoint3dArray& points)
{
    Acad::ErrorStatus es = Acad::eOk;

    AcGePoint3dArray vertexArray;
    if ((es = poly->getVertices(vertexArray)) != Acad::eOk) {
        return es;
    }

    AcGeLineSeg3d lnsg;
    AcGePoint3d   pt1, pt2;
    AcGePoint3d   dummy1, dummy2;
    int           howMany;

    for (int i = 0; i < vertexArray.length() - 1; i++) {

        lnsg.set(vertexArray[i], vertexArray[i + 1]);

        if (projPlane == NULL) {
            if (!circle.intersectWith(lnsg, howMany, pt1, pt2)) {
                continue;
            } else {
                if (howMany > 1) {
                    points.append(pt1);
                    points.append(pt2);
                } else {
                    points.append(pt1);
                }
            }
        } else {
            if (!circle.projIntersectWith(lnsg, projPlane->normal(), 
               howMany, pt1, pt2, dummy1, dummy2))
            {
                continue;
            } else {
                if (howMany > 1) {
                    points.append(pt1);
                    points.append(pt2);
                } else {
                    points.append(pt1);
                }
            }
        }
    }

    return es;
}


static Acad::ErrorStatus intPline(const PDBox*         poly, 
                                        AcDb2dPolyline*   pline,
                                        AcDb::Intersect   intType,
                                  const AcGePlane*        projPlane,
                                        AcGePoint3dArray& points)
{
    Acad::ErrorStatus es = Acad::eOk;

    AcGePoint3dArray vertexArray;
    if ((es = poly->getVertices(vertexArray)) != Acad::eOk) {
        return es;
    }

    AcDbSpline*      spline = NULL;
    AcDbLine*        acam_dlengthine = NULL;

    AcGePoint3dArray    pts;
    AcGeDoubleArray  bulges;
    int               i;
    AcGePoint3d      pt1, pt2;

    AcGeCircArc3d    arc;
    AcGeLineSeg3d    lnsg;

    AcGeVector3d entNorm = pline->normal();
    AcDb::Intersect type = AcDb::kOnBothOperands;

    switch (pline->polyType()) {

    case AcDb::k2dSimplePoly:
/*    case AcDb::k2dFitCurvePoly:

        // Intersect with each line or arc segment of the polyline. 
        // Depending on the intType, extend the last segment.

        if ((es = rx_scanPline(pline, pts, bulges)) != Acad::eOk) {
            return es;
        }
        numPoints = pts.length();

        for (i = 0; i < numPoints - 1; i++) {

            pt1 = pts[i]; pt2 = pts[i + 1];
            if (i == numPoints - 2)
                type = intType;

            if (bulges[i] > 1.0e-10) {     // create an arc

//                rx_wc2ec(asDblArray(pt1), asDblArray(pt1), asDblArray(entNorm), 
                                                              Adesk::kFalse);
//                rx_wc2ec(asDblArray(pt2), asDblArray(pt2), asDblArray(entNorm), 
                                                              Adesk::kFalse);

////                AOK(rx_makeArc(pt1, pt2, bulges[i], entNorm, arc));
//                AOK(intArc(poly, arc, type, projPlane, points));

            } else {                       // create a line

                lnsg.set(pt1, pt2);
//                AOK(intLine(poly, lnsg, type, projPlane, points));
            }
        }
        break;

    case AcDb::k2dQuadSplinePoly:
    case AcDb::k2dCubicSplinePoly:

        if ((es = pline->getSpline(spline)) != Acad::eOk) {
            return es;
        }

        if (intType == AcDb::kExtendArg || intType == AcDb::kExtendBoth) {
            intType = AcDb::kExtendThis;
        }

        for (i = 0; i < vertexArray.length() - 1; i++) {

            acam_dlengthine = new AcDbLine();
            acam_dlengthine->setStartPoint(vertexArray[i]);
            acam_dlengthine->setEndPoint(vertexArray[i + 1]);
            acam_dlengthine->setNormal(AcGeVector3d(0,0,1));

            if (projPlane == NULL) {
//                AOK(spline->intersectWith(acam_dlengthine, intType, points));
            } else {
//                AOK(spline->intersectWith(acam_dlengthine, intType, 
 //                                                         *projPlane, points));
            }

            delete acam_dlengthine;
        }

        delete spline;
        break;

    default:
       return Acad::eInvalidInput;
    }

    return es;
}


static Acad::ErrorStatus intPline(const PDBox*         poly, 
                                        AcDb3dPolyline*   pline,
                                        AcDb::Intersect   intType,
                                  const AcGePlane*        projPlane,
                                        AcGePoint3dArray& points)
{
    Acad::ErrorStatus es = Acad::eOk;

    AcGePoint3dArray vertexArray;
    if ((es = poly->getVertices(vertexArray)) != Acad::eOk) {
        return es;
    }

    AcDbSpline*      spline = NULL;
    AcDbLine*        acam_dlengthine = NULL;

    AcGePoint3dArray    pts;
    int              numPoints, i;

    AcGeLineSeg3d    lnsg;
    AcDb::Intersect  type = AcDb::kOnBothOperands;

    switch (pline->polyType()) {

    case AcDb::k3dSimplePoly:

//        if ((es = rx_scanPline(pline, pts)) != Acad::eOk) {
            return es;
  //      }
        numPoints = pts.length();

        // Intersect with each line segment of the polyline. 
        // Depending on the intType, extend the last segment.

        for (i = 0; i < numPoints - 1; i++) {

            if (i == numPoints - 2)
                type = intType;

            lnsg.set(pts[i], pts[i + 1]);
            if ((es = intLine(poly, lnsg, type, projPlane, points)) 
                   != Acad::eOk) {
                return es;
            }
        }

    case AcDb::k3dQuadSplinePoly:
 /*   case AcDb::k3dCubicSplinePoly:

        if ((es = pline->getSpline(spline)) != Acad::eOk) {
            delete spline;
            return es;
        }

        if (intType == AcDb::kExtendArg || intType == AcDb::kExtendBoth) {
            intType = AcDb::kExtendThis;
        }

        for (i = 0; i < vertexArray.length() - 1; i++) {

            acam_dlengthine = new AcDbLine();
            acam_dlengthine->setStartPoint(vertexArray[i]);
            acam_dlengthine->setEndPoint(vertexArray[i + 1]);
  //          acam_dlengthine->setNormal(poly->normal());

            if (projPlane == NULL) {
                AOK(spline->intersectWith(acam_dlengthine, intType, points));
            } else {
                AOK(spline->intersectWith(acam_dlengthine, intType, 
                                                          *projPlane, points));
            }

            delete acam_dlengthine;
        }

        delete spline;
        break;

    default:
       return Acad::eInvalidInput;
    }

    return es;
}
*/

bool isInBoundPlane(const AcGePlane &plane, int faceIdx, const PDBox *&poly, const AcGePoint3d &pt)
{
    AcGePoint3d projPt = pt;
    AcGeMatrix3d mat;
    AcGePoint3d ori;
    AcGeVector3d xVec, yVec, zVec;
    plane.getCoordSystem(ori, xVec, yVec);
    zVec = xVec.crossProduct(yVec).normal();
    mat.setCoordSystem(ori, xVec, yVec, zVec);
    mat.invert();
    projPt = projPt.transformBy(mat);
    bool isIn = false;
    switch(faceIdx)
    {
    case 0:
    case 1:
        if(projPt.x >= 0 && projPt.x <= poly->getlength() && projPt.y >= 0 && projPt.y <= poly->getwidth())
            isIn = true;
        break;
    case 2:
    case 3:
        if(projPt.x >= 0 && projPt.x <= poly->getlength() && projPt.y >= 0 && projPt.y <= poly->getheight())
            isIn = true;
        break;
    case 4:
    case 5:
        if(projPt.x >= 0 && projPt.x <= poly->getwidth() && projPt.y >= 0 && projPt.y <= poly->getheight())
            isIn = true;
        break;
    }
    return isIn;
}

////	通过调用此函数，在引用参数vertexArray 中返回Box 的各个顶点坐标，
////	以便在 viewportDraw 中利用。
////	底面返回5个顶点，顶面返回5个顶点。
Acad::ErrorStatus PDBox::getVertices(AcGePoint3dArray& vertexArray) const
{
    assertReadEnabled();

    //清空vertexArray数组
 /*   while(!vertexArray.isEmpty())
        vertexArray.removeLast();*/
	vertexArray.setLogicalLength(10);//shinf 2007.01.04 to speed up

    AcGePoint3d  p1,p2,p3,orign1,p11,p22,p33;
	AcGeVector3d widthVecAlongy,heightVecAlongZ;
	p1 = m_porign + m_dlength * m_Vectx;
	widthVecAlongy = m_dwidth * m_Vecty;
	p3 = m_porign + /*m_dwidth * m_Vecty*/widthVecAlongy;
    p2 = p1 + /*m_dwidth * m_Vecty*/widthVecAlongy;
	heightVecAlongZ = m_dheight * m_Vectz;
	orign1 = m_porign + /*m_dheight * m_Vectz*/heightVecAlongZ;
	p11 = p1 + /*m_dheight * m_Vectz*/heightVecAlongZ;
	p22 = p2 + /*m_dheight * m_Vectz*/heightVecAlongZ;
	p33 = p3 + /*m_dheight * m_Vectz*/heightVecAlongZ;

    /*vertexArray.append(m_porign);
    vertexArray.append(p1);
	vertexArray.append(p2);
	vertexArray.append(p3);
    vertexArray.append(m_porign);
	vertexArray.append(orign1);
	vertexArray.append(p11);
	vertexArray.append(p22);
	vertexArray.append(p33);
    vertexArray.append(orign1);*/
	vertexArray[0] = m_porign;
	vertexArray[1] = (p1);
	vertexArray[2] = (p2);
	vertexArray[3] = (p3);
	vertexArray[4] = (m_porign);
	vertexArray[5] = (orign1);
	vertexArray[6] = (p11);
	vertexArray[7] = (p22);
	vertexArray[8] = (p33);
	vertexArray[9] = (orign1);

    return Acad::eOk;
}
#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDBox::subGetOsnapPoints(
	AcDb::OsnapMode     osnapMode,
	Adesk::GsMarker     gsSelectionMark,
	const AcGePoint3d&  pickPoint,
	const AcGePoint3d&  lastPoint,
	const AcGeMatrix3d& viewXform,
	AcGePoint3dArray&   snapPoints,
	AcDbIntArray &   geomIds) const
#else
Acad::ErrorStatus PDBox::getOsnapPoints(
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

    switch(osnapMode) {
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
				for(i = 0; i < 4; ++i)
				{
					pt = verArr[i] + (verArr[i+1] - verArr[i]) / 2.0;
					snapPoints.append(pt);
				}
				break;
			case TOP_PLANE:
				for(i = 5; i < 9; ++i)
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
			//重新定义对象中心点的捕捉方式，同时满足实体模型和线框模型的捕捉 
			//modified by szw 2009.11.18 : begin
            //下底面中心
		    cenPt[0] = m_porign + m_Vectx.normal() * m_dlength / 2.0
                                + m_Vecty.normal() * m_dwidth / 2.0;
            //上底面中心
            cenPt[1] = cenPt[0] + m_Vectz.normal() * m_dheight;
            //前侧面中心
            cenPt[2] = m_porign + m_Vectx.normal() * m_dlength / 2.0 
                                + m_Vectz.normal() * m_dheight / 2.0;
            //左侧面中心(变换了中心定义的顺序，使其与worldDraw中定义顺序相同)
            cenPt[3] = m_porign + m_Vecty.normal() * m_dwidth / 2.0
                                + m_Vectz.normal() * m_dheight / 2.0;
            //右侧面中心
            cenPt[4] = m_porign + m_Vectx.normal() * m_dlength
                                + m_Vecty.normal() * m_dwidth / 2.0
                                + m_Vectz.normal() * m_dheight / 2.0;
            //后侧面中心
            cenPt[5] = cenPt[2] + m_Vecty.normal() * m_dwidth;
            
		        //体的中心
            cenPt[6] = m_porign + m_Vectx.normal() * m_dlength / 2.0
                                + m_Vecty.normal() * m_dwidth / 2.0
                                + m_Vectz.normal() * m_dheight / 2.0;
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
    case AcDb::kOsModeNode:
		break;
	case AcDb::kOsModeQuad:
		break;
    case AcDb::kOsModeIns:
		snapPoints.append(m_porign);
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
				for(i = 0; i < 4; ++i)
				{
					vec = verArr[i] - verArr[i+1];
					vec.normalize();
					line.set(verArr[i], vec);
					pt = line.closestPointTo(lastPoint);
					snapPoints.append(pt);
				}
				break;
			case TOP_PLANE:
				for(i = 5; i < 9; ++i)
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

/*
Acad::ErrorStatus PDBox::osnapEnd(const AcGePoint3d& pickPoint,
    AcGePoint3dArray& snapPoints) const
{
    AcGePoint3dArray verArr;
	this->getVertices(verArr);
	for(int i=0;i<verArr.length();i++){
		snapPoints.append(verArr[i]);
    }
    return Acad::eOk;
}
Acad::ErrorStatus
PDBox::osnapIns(const AcGePoint3d& pickPoint,
    AcGePoint3dArray& snapPoints) const
{
    assertReadEnabled();
	AcGePoint3d cenPt,tCenPt,bCenPt;
	AcGeVector3d zVector;
	zVector=(m_Vectx.crossProduct(m_Vecty)).normal();
	bCenPt=m_porign+m_Vectx.normal()*m_dlength/2.0+m_Vecty.normal()*m_dwidth/2.0;
	cenPt=bCenPt+zVector*m_dheight/2.0;
	tCenPt=bCenPt+zVector*m_dheight;
    snapPoints.append(cenPt);
	snapPoints.append(bCenPt);
	snapPoints.append(tCenPt);
    return Acad::eOk;
}
*/

///////////////////////////////////////
///		重载夹点函数			///////
///////////////////////////////////////
#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDBox::subGetGripPoints(
          AcGePoint3dArray& gripPoints,
         AcDbIntArray& osnapModes,
        AcDbIntArray& geomIds) const
#else
Acad::ErrorStatus PDBox::getGripPoints(
		AcGePoint3dArray& gripPoints,
		AcDbIntArray& osnapModes,
		AcDbIntArray& geomIds) const
#endif
{
    assertReadEnabled();
/*	getVertices(gripPoints);*/
	return Acad::eOk; 
}



/*
Acad::ErrorStatus 
PDBox :: moveGripPointsAt(
    const AcDbIntArray& indices,
    const AcGeVector3d& offset)
{
	AcGePoint3dArray oldVertexArray;
	getVertices(oldVertexArray);
	for(int i=0;i<oldVertexArray.length();i++)
	{
		oldVertexArray[i] += offset;
	}
	m_porign=m_porign+offset;
//	PDBox* pNewBox=new PDBox(orign,m_dlength,m_dwidth,m_dheight,m_Vectx,m_Vecty);
//	AcGePoint3dArray newVertexArray;
//	this->getVertices(newVertexArray);
//	delete pNewBox;
	
	return Acad::eOk;
}


Acad::ErrorStatus 
PDBox::getStretchPoints(
AcGePoint3dArray& stretchPoints) const
{
    assertReadEnabled();
	AcGePoint3dArray verArr;
	getVertices(verArr);
	for(int i=0;i<verArr.length();i++){
		stretchPoints.append(verArr[i]);
	}
	return Acad::eOk;
}

Acad::ErrorStatus 
PDBox::moveStretchPointsAt(
    const AcDbIntArray& indices,
      const AcGeVector3d& offset)
{
     assertReadEnabled();
	 AcGePoint3dArray oldVertexArray;
	this->getVertices(oldVertexArray);
	for(int i=0;i<oldVertexArray.length();i++)
	{
		oldVertexArray[i]=oldVertexArray[i]+offset;
	}
	m_porign=m_porign+offset;
	return Acad::eOk;
}
*/     
#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDBox::subExplode(AcDbVoidPtrArray& entitySet) const
#else
Acad::ErrorStatus PDBox::explode(AcDbVoidPtrArray& entitySet) const
#endif
{
    assertReadEnabled();

    // AcDbPolyFaceMesh 方式
    //AcDbPolyFaceMesh *pMesh = NULL;
    //pMesh = new AcDbPolyFaceMesh();
    //pMesh->setPropertiesFrom(this);
    //entitySet.append(pMesh);
    //AcDbPolyFaceMeshVertex *pVertex = NULL;
    //AcDbFaceRecord *pFace = NULL;

    //AcGePoint3dArray ptarr;
    //getVertices(ptarr);

    //int i;
    //for(i = 1; i <= ptarr.length(); ++i)
    //{
    //    if(i % 5 == 0)
    //        continue;
    //    pVertex = new AcDbPolyFaceMeshVertex(ptarr[i - 1]);
    //    entitySet.append(pVertex);
    //    pMesh->appendVertex(pVertex);
    //}
    //// 上下面
    //for(i = 1; i <= 8; i += 4)
    //{
    //    pFace = new AcDbFaceRecord(i, i + 1, i + 2, i + 3);
    //    entitySet.append(pFace);
    //    pMesh->appendFaceRecord(pFace);
    //}
    //// 周围的四个面
    //for(i = 1; i < 4; ++i)
    //{
    //    pFace = new AcDbFaceRecord(i, i + 1, i + 5, i + 4);
    //    entitySet.append(pFace);
    //    pMesh->appendFaceRecord(pFace);
    //}
    //pFace = new AcDbFaceRecord(4, 1, 5, 8);
    //entitySet.append(pFace);
    //pMesh->appendFaceRecord(pFace);

    // 实体方式
    AcGePoint3d ori;
    AcGeVector3d Vectx, Vecty, Vectz;
    double length, width, height;
    getRightHandCoord(ori, Vectx, Vecty, Vectz, length, width, height);

    AcDb3dSolid *pBody;
    pBody = new AcDb3dSolid;
    if(pBody->createBox(length, width, height) != Acad::eOk)
    {
        delete pBody;
        return Acad::eNotApplicable;
    }
    AcGeMatrix3d mat;
    mat.setCoordSystem(ori + Vectx * length / 2.0 + Vecty * width / 2.0 + Vectz * height / 2.0, 
                       Vectx, Vecty, Vectz);
    pBody->transformBy(mat);
    pBody->setPropertiesFrom(this);
    entitySet.append(pBody);
	
	return Acad::eOk;
}

Acad::ErrorStatus PDBox::explodeTo3DSolid(AcDb3dSolid* &p3dSolid) const
{
	AcGePoint3d ori;
	AcGeVector3d Vectx, Vecty, Vectz;
	double length, width, height;
	getRightHandCoord(ori, Vectx, Vecty, Vectz, length, width, height);

	AcDb3dSolid *pBody;
	pBody = new AcDb3dSolid;

	if(pBody->createBox(length, width, height) != Acad::eOk)
	{
		delete pBody;
		return Acad::eNotApplicable;
	}

	AcGeMatrix3d mat;
	mat.setCoordSystem(ori + Vectx * length / 2.0 + Vecty * width / 2.0 + Vectz * height / 2.0, 
		Vectx, Vecty, Vectz);
	pBody->transformBy(mat);
	pBody->setPropertiesFrom(this);

	p3dSolid = pBody;

	return Acad::eOk;
}

/*Acad::ErrorStatus 
PDBox::getCoordSys(AcGeMatrix3d& mat) const
{
    assertReadEnabled();
    AcGeVector3d zVector;
	zVector=m_Vectx.crossProduct(m_Vecty);
	zVector.normal();
	mat.setCoordSystem(m_porign, m_Vectx, m_Vecty,zVector);

    return Acad::eOk;
}
*/
#ifdef _OBJECTARX2010_
void PDBox::subList() const
#else
void PDBox::list() const
#endif
{
	assertReadEnabled();
#ifdef _OBJECTARX2010_
	PDPrimary3D::subList();
#else
	PDPrimary3D::list();
#endif
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4732/*"\n 插入点(%f,%f,%f)\n"*/,m_porign.x,m_porign.y,m_porign.z);
    ads_printf(LANGUAGE_DEFINE_MACRO_V25_4733/*" 插入X轴(%f,%f,%f)\n"*/,m_Vectx.x,m_Vectx.y,m_Vectx.z);
    ads_printf(LANGUAGE_DEFINE_MACRO_V25_4734/*" 插入Y轴(%f,%f,%f)\n"*/,m_Vecty.x,m_Vecty.y,m_Vecty.z);
    ads_printf(LANGUAGE_DEFINE_MACRO_V25_4735/*" 插入Z轴(%f,%f,%f)\n"*/,m_Vectz.x,m_Vectz.y,m_Vectz.z);
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4736/*" 长度=%.3f 宽度=%.3f 高度=%.3f\n\n"*/,m_dlength,m_dwidth,m_dheight);
	return ;
}
/*
Acad::ErrorStatus
PDBox::deepClone(AcDbObject*    pOwner,
                    AcDbObject*&   pClonedObject,
                    AcDbIdMapping& idMap,
                    Adesk::Boolean isPrimary) const
{
    // You should always pass back pClonedObject == NULL
    // if, for any reason, you do not actually clone it
    // during this call.  The caller should pass it in
    // as NULL, but to be safe, we set it here as well.
    //
    pClonedObject = NULL;

    // If this object is in the idMap and is already
    // cloned, then return.
    //
    AcDbIdPair idPair(objectId(), (AcDbObjectId)NULL,
                      Adesk::kFalse, isPrimary);
    if (idMap.compute(idPair) && (idPair.value() != NULL))
        return Acad::eOk;    

    // STEP 1:
    // Create the clone
    //
   PDBox *pClone = (PDBox*)isA()->create();
    if (pClone != NULL)
        pClonedObject = pClone;    // set the return value
    else
        return Acad::eOutOfMemory;

    // STEP 2:
    // Append the clone to its new owner.  In this example,
    // we know that we are derived from AcDbEntity, so we
    // can expect our owner to be an AcDbBlockTableRecord.
    //
    // If an INSERT* operation is in progress pOwner will
    // point to an AcDbBlockTable rather than the actual
    // AcDbBlockTableRecord that is to be the owner.  In
    // this case we just append the clone to the database
    // and don't deal with establising ownership (ownership
    // will be established later on in the INSERT* process).
    //
    // If we have set up an ownership relationship with
    // another of our objects rather than an AcDbBlockTableRecord,
    // then we need to establish how we connect to that owner in
    // our own way.  This sample shows a generic method using
    // setOwnerId().
    // 
    AcDbBlockTableRecord *pBTR =
        AcDbBlockTableRecord::cast(pOwner);
    if (pBTR != NULL) {
        pBTR->appendAcDbEntity(pClone);

    } else if (pOwner->isKindOf(AcDbBlockTable::desc())) {
        pOwner->database()->addAcDbObject(pClone);

    } else {
        if (isPrimary)
            return Acad::eInvalidOwnerObject;

        // Some form of this code is only necessary if
        // anyone has set up an ownership for our object
        // other than with an AcDbBlockTableRecord.
        //
        pOwner->database()->addAcDbObject(pClone);
        pClone->setOwnerId(pOwner->objectId());
    }
    
    // STEP 3:
    // Now we copy our contents to the clone.  This is done
    // using an AcDbDeepCloneFiler.  This filer keeps a
    // list of all AcDbHardOwnershipIds and
    // AcDbSoftOwnershipIds we, and any classes we derive
    // from,  have.  This list is then used to know what
    // additional, "owned" objects need to be cloned below.
    //
    AcDbDeepCloneFiler filer;
    dwgOut(&filer);

    // STEP 4:
    // Rewind the filer and read the data into the clone.
    //
    filer.seek(0L, AcDb::kSeekFromStart);
    pClone->dwgIn(&filer);

    // STEP 5:
    // This must be called for all newly created objects
    // in deepClone.  It is turned off by endDeepClone()
    // after it has translated the references to their
    // new values.
    //
    pClone->setAcDbObjectIdsInFlux();

    // STEP 6:
    // Add the new information to the idMap.  We can use
    // the idPair started above.
    //
    idPair.setValue(pClonedObject->objectId());
    idPair.setIsCloned(Adesk::kTrue);
    idMap.assign(idPair);

    // STEP 7:
    // Using the filer list created above, find and clone
    // any owned objects.
    //
    AcDbObjectId id;
    while (filer.getNextOwnedObject(id)) {

        AcDbObject *pSubObject;
        AcDbObject *pClonedSubObject;

        // Some object's references may be set to NULL, 
        // so don't try to clone them.
        //
        if (id == NULL)
            continue;

        // Open the object and clone it.  Note that we now
        // set "isPrimary" to kFalse here because the object
        // is being cloned, not as part of the primary set,
        // but because it is owned by something in the
        // primary set.
        //
        acdbOpenAcDbObject(pSubObject, id, AcDb::kForRead);
        pClonedSubObject = NULL;
        pSubObject->deepClone(pClonedObject,
                              pClonedSubObject,
                              idMap, Adesk::kFalse);

        // If this is a kDcInsert context, the objects
        // may be "cheapCloned".  In this case, they are
        // "moved" instead of cloned.  The result is that
        // pSubObject and pClonedSubObject will point to
        // the same object.  So, we only want to close
        // pSubObject if it really is a different object
        // than its clone.
        //
        if (pSubObject != pClonedSubObject)
            pSubObject->close();
        
        // The pSubObject may either already have been
        // cloned, or for some reason has chosen not to be
        // cloned.  In that case, the returned pointer will
        // be NULL.  Otherwise, since we have no immediate
        // use for it now, we can close the clone.
        //
        if (pClonedSubObject != NULL)
            pClonedSubObject->close();
    }

    // Leave pClonedObject open for the caller
    //
    return Acad::eOk;
}
*/
void PDBox::createBody()
{
    assertWriteEnabled();
//#ifdef _OBJECTARX2004_
    AcGePoint3d ori;
    AcGeVector3d Vectx, Vecty, Vectz;
    double length, width, height;
    getRightHandCoord(ori, Vectx, Vecty, Vectz, length, width, height);

    AcGeMatrix3d mat;
    mat.setCoordSystem(ori, Vectx, Vecty, Vectz);

    Vector3d GeomVec(length, width, height);
    m_3dGeom = Body::box(Point3d::kNull, GeomVec);
    m_3dGeom.transform(*(Transf3d*)&mat);
//#endif
}

Acad::ErrorStatus PDBox::getRightHandCoord(AcGePoint3d &ori, 
                                           AcGeVector3d &vecx, 
                                           AcGeVector3d &vecy, 
                                           AcGeVector3d &vecz, 
                                           double &length, 
                                           double &width, 
                                           double &height) const
{
    assertReadEnabled();
    ori = m_porign;
    vecx = m_Vectx;
    vecy = m_Vecty;
    vecz = m_Vectz;
    length = m_dlength;
    width = m_dwidth;
    height = m_dheight;
    AcGeVector3d z;
    double ang;
    z = m_Vectx.crossProduct(m_Vecty).normal();
    ang = z.angleTo(m_Vectz, m_Vectx);
    if(ang > PI / 2.0 && ang < PI * 3.0 / 2.0)
    {
        length = m_dwidth;
        width = m_dlength;
        vecx = m_Vecty;
        vecy = m_Vectx;
    }
    return Acad::eOk;
}

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDBox::subGetGeomExtents(AcDbExtents& extents) const
#else
Acad::ErrorStatus PDBox::getGeomExtents(AcDbExtents& extents) const
#endif
{
    assertReadEnabled();
    extents.set(m_porign, m_porign);
    AcGePoint3dArray ptarr;
    getVertices(ptarr);
    int i, arrLen = ptarr.length();
    for(i = 0; i < arrLen; ++i)
        extents.addPoint(ptarr[i]);
    return Acad::eOk;
}

// append - append an opened entity to paper space in the database:
//Adesk::Boolean
//append(AcDbEntity* pEntity)
//{
//    AcDbBlockTable *pBlockTable;
//    Acad::ErrorStatus es = acdbHostApplicationServices()->workingDatabase()->getBlockTable(pBlockTable,AcDb::kForRead);
//    if (es != Acad::eOk) {
//        ads_alert(_T("Failed to get block table!"));
//        return Adesk::kFalse;
//    }
//
//    AcDbBlockTableRecord *pBlockRec;
//    es = pBlockTable->getAt(ACDB_MODEL_SPACE, pBlockRec, AcDb::kForWrite);
//    if (es != Acad::eOk) {
//        ads_alert(_T("Failed to get block table record!"));
//        pBlockTable->close();
//        return Adesk::kFalse;
//    }
//
//    es = pBlockRec->appendAcDbEntity(pEntity);
//    if (es != Acad::eOk) {
//        ads_alert(_T("Failed to append entity!"));
//        pBlockTable->close();
//        pBlockRec->close();
//        delete pEntity;
//        return Adesk::kFalse;
//    }
//    pBlockRec->close();
//    pBlockTable->close();
//    return Adesk::kTrue;
//}

///< 取得世界坐标系中最底点的Z值
double PDBox::getLowestZ() const
{
    assertReadEnabled();
    AcGePoint3dArray pts;
    getVertices(pts);
    if(pts.length() <= 0)
        return 0;
    double ret = 0;
    ret = pts[0].z;
    for(int i = 1; i < pts.length(); ++i)
    {
        if(pts[i].z < ret)
            ret = pts[i].z;
    }
    return ret;
}

// Added by SongNan 2007/6/22 : begin
///< 取得BOX体元的六个有界平面，考虑方向
const std::vector<AcGeBoundedPlane> &PDBox::
					GetAllBndPlns(std::vector<AcGeBoundedPlane> &plnVec)const
{
	assertReadEnabled();
	// 构造六个有界平面
	AcGeVector3d vecX = m_Vectx.normal()*m_dlength;
	AcGeVector3d vecY = m_Vecty.normal()*m_dwidth;
	AcGeVector3d vecZ = m_Vectz.normal()*m_dheight;

	bool corrType = 
			(Adesk::kTrue == vecZ.isCodirectionalTo(vecX.crossProduct(vecY)));


	AcGePoint3d ptOpposite = m_porign + vecX + vecY +vecZ;
	plnVec.clear();

	if (corrType)
	{
		plnVec.push_back(AcGeBoundedPlane(m_porign, vecY, vecX));	
		plnVec.push_back(AcGeBoundedPlane(m_porign, vecX, vecZ));
		plnVec.push_back(AcGeBoundedPlane(m_porign, vecZ, vecY));


		vecX.negate();
		vecY.negate();
		vecZ.negate();

		plnVec.push_back(AcGeBoundedPlane(ptOpposite, vecX, vecY));
		plnVec.push_back(AcGeBoundedPlane(ptOpposite, vecY, vecZ));
		plnVec.push_back(AcGeBoundedPlane(ptOpposite, vecZ, vecX));

	}
	else
	{		
		plnVec.push_back(AcGeBoundedPlane(m_porign, vecX, vecY));
		plnVec.push_back(AcGeBoundedPlane(m_porign, vecZ, vecX));
		plnVec.push_back(AcGeBoundedPlane(m_porign, vecY, vecZ));

		vecX.negate();
		vecY.negate();
		vecZ.negate();

		plnVec.push_back(AcGeBoundedPlane(ptOpposite, vecY, vecX));
		plnVec.push_back(AcGeBoundedPlane(ptOpposite, vecZ, vecY));
		plnVec.push_back(AcGeBoundedPlane(ptOpposite, vecX, vecZ));

	}

	return plnVec;
}
// Added by SongNan 2007/6/22 : end

void PDBox::setDefault()
{
	PDPrimary3D::setDefault();
#if defined(NDEBUG) || !defined(_DEBUG)
	m_porign = AcGePoint3d::kOrigin;
	m_dlength = 1;
	m_dwidth = 1;
	m_dheight = 1;
	m_Vectx.set(1, 0, 0);
	m_Vecty.set(0, 1, 0);
	m_Vectz.set(0, 0, 1);
#else
	m_porign.set(0,0,0);
	m_dlength = 1000;
	m_dwidth = 1000;
	m_dheight = 1000;
	m_Vectx.set(1, 0, 0);
	m_Vecty.set(0, 1, 0);
	m_Vectz.set(0, 0, 1);
#endif
}
bool PDBox::isValidData(double &f)
{
	if(!PDPrimary3D::isValidData(f))
		return false;
	if(!isValidFloat(f=m_dlength))
		return false;
	if(!isValidFloat(f=m_dwidth))
		return false;
	if(!isValidFloat(f=m_dheight))
		return false;
	if(!isValidFloat(f=m_porign.x) || !isValidFloat(f=m_porign.y) || !isValidFloat(f=m_porign.z))
		return false;
	if(!isValidFloat(f=m_Vectx.x) || !isValidFloat(f=m_Vectx.y) || !isValidFloat(f=m_Vectx.z))
		return false;
	if(!isValidFloat(f=m_Vecty.x) || !isValidFloat(f=m_Vecty.y) || !isValidFloat(f=m_Vecty.z))
		return false;
	if(!isValidFloat(f=m_Vectz.x) || !isValidFloat(f=m_Vectz.y) || !isValidFloat(f=m_Vectz.z))
		return false;
	return true;
}

//zxb,20090113,增加audit，暂时只check自身成员数据
Acad::ErrorStatus PDBox::audit(AcDbAuditInfo* pAuditInfo)
{
	return auditPDPrimary3D(this,pAuditInfo,_T("PDBox"));
}
