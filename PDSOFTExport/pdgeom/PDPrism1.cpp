// PDPrism1.cpp: implementation of the PDPrism1 class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "language25.h"
#include "PDPrism1.h"
#include <dbproxy.h>
#include "acgi.h"
#include "dbmain.h"
#include "rxboiler.h"
#include "gevec3d.h"
#include "gemat3d.h"
#include "dbents.h"
#include "gelnsg3d.h"
#include "dbspline.h"
#include "geassign.h"
#include "dbidmap.h"
#include "dbcfilrs.h"
#include "dbsymtb.h"
#include <dbregion.h>
#include <dbsol3d.h>
#include "dbapserv.h"

#include "math.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define VERSION_PDPRISM1 1
#define PI 3.1415926535897932384626433832795L

#ifdef _OBJECTARX2000_
ACRX_DXF_DEFINE_MEMBERS(PDPrism1, PDPrimary3D,
                        AcDb::kDHL_CURRENT, AcDb::kMReleaseCurrent,
                        AcDbProxyEntity::kAllAllowedBits,
                        PDPRISM1, Gle);
#else
ACRX_DXF_DEFINE_MEMBERS(PDPrism1, PDPrimary3D, AcDbProxyEntity::kAllAllowedBits, PDPRISM1, Gle);
#endif

void PDPrism1::setDefault()
{
  PDPrimary3D::setDefault();
#if defined(NDEBUG) || !defined(_DEBUG)
  m_ptcenP1=AcGePoint3d(0,0,0);
  m_ptcenP2=AcGePoint3d(0,0,1);
  m_ptedgP3=AcGePoint3d(1,0,0);
  m_nedgeNum=4;
#else
  m_ptcenP1=AcGePoint3d(0,0,0);
  m_ptcenP2=AcGePoint3d(0,0,1000);
  m_ptedgP3=AcGePoint3d(1000,0,0);
  m_nedgeNum=4;
#endif
}
bool PDPrism1::isValidData(double &f)
{
  if(!PDPrimary3D::isValidData(f))
    return false;
  if(!isValidFloat(f=m_ptcenP1.x) || !isValidFloat(f=m_ptcenP1.y) || !isValidFloat(f=m_ptcenP1.z))
    return false;
  if(!isValidFloat(f=m_ptcenP2.x) || !isValidFloat(f=m_ptcenP2.y) || !isValidFloat(f=m_ptcenP2.z))
    return false;
  if(!isValidFloat(f=m_ptedgP3.x) || !isValidFloat(f=m_ptedgP3.y) || !isValidFloat(f=m_ptedgP3.z))
    return false;
  return true;
}

//zxb,20090113,增加audit，暂时只check自身成员数据
Acad::ErrorStatus PDPrism1::audit(AcDbAuditInfo* pAuditInfo)
{
  return auditPDPrimary3D(this,pAuditInfo,_T("PDPrism1"));
}

PDPrism1::PDPrism1(bool HasSnap/* = false*/) : PDPrimary3D(HasSnap)
{
	m_ptcenP1=AcGePoint3d(0,0,0);
	m_ptcenP2=AcGePoint3d(0,0,1);
	m_ptedgP3=AcGePoint3d(1,0,0);
	m_nedgeNum=4;
//	xVector=AcGeVector3d(1,0,0);
}

PDPrism1::PDPrism1(AcGePoint3d cP1,AcGePoint3d cP2,AcGePoint3d eP3,Adesk::UInt32 eNum, bool HasSnap/* = false*/) : PDPrimary3D(HasSnap)
{
	m_ptcenP1 = cP1 ;
	m_ptcenP2 = cP2 ;
	m_ptedgP3 = eP3 ;

    AcGeVector3d  p1p3, p1p2;
    p1p3 = eP3 - cP1;
    p1p2 = cP2 - cP1;
    if(p1p3.isZeroLength() && p1p2.isZeroLength())
    {
        m_ptcenP2 = m_ptcenP1 + AcGeVector3d(0, 0, 1);
        m_ptedgP3 = m_ptcenP1 + AcGeVector3d(1, 0, 0);
    }
    else if(p1p3.isZeroLength())
    {
        AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax, Ay;
        if (fabs(p1p2[X]) <1.0/64 && fabs(p1p2[Y])<1.0/64) 
            Ax = Wy.crossProduct(p1p2);
        else
            Ax = Wz.crossProduct(p1p2);
        Ax.normalize();

        p1p3 = Ax;
        m_ptedgP3 = m_ptcenP1 + p1p3;
    }
    else if(p1p2.isZeroLength())
    {
        AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax, Ay;
        if (fabs(p1p3[X]) <1.0/64 && fabs(p1p3[Y])<1.0/64) 
            Ax = Wy.crossProduct(p1p3);
        else
            Ax = Wz.crossProduct(p1p3);
        Ax.normalize();

        p1p2 = Ax;
        m_ptcenP2 = m_ptcenP1 + p1p2;
    }
    else if(p1p2.isParallelTo(p1p3))
    {
        AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax, Ay;
        if (fabs(p1p2[X]) <1.0/64 && fabs(p1p2[Y])<1.0/64) 
            Ax = Wy.crossProduct(p1p2);
        else
            Ax = Wz.crossProduct(p1p2);
        Ax.normalize();

        p1p3 = Ax * p1p3.length();
        m_ptedgP3 = m_ptcenP1 + p1p3;
    }

	if(eNum>=3)
		m_nedgeNum=eNum;
	else
		m_nedgeNum=3;

#ifdef _USEAMODELER_
	createBody();
#endif
}//added by  linlin 20050929

PDPrism1::~PDPrism1()
{
}

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDPrism1::subTransformBy(const AcGeMatrix3d& xform)
#else
Acad::ErrorStatus PDPrism1::transformBy(const AcGeMatrix3d& xform)
#endif
{
	assertWriteEnabled();
	// TODO: implement this function.
	m_ptcenP1.transformBy(xform);
	m_ptcenP2.transformBy(xform);
	m_ptedgP3.transformBy(xform);
#ifdef _OBJECTARX2010_
	return PDPrimary3D::subTransformBy(xform);
#else
	return PDPrimary3D::transformBy(xform);
#endif
}

Acad::ErrorStatus PDPrism1::dwgOutFields(AcDbDwgFiler* filer)const
{
    assertReadEnabled();
	Acad::ErrorStatus es=Acad::eOk;

	// Call dwgOutFields from father class: PDPrimary3D
	if((es=PDPrimary3D::dwgOutFields(filer))!=Acad::eOk)
	{
		return es;
	}

	// Write version number.
	filer->writeItem((Adesk::UInt16) VERSION_PDPRISM1);

	// Write the data members.
	filer->writeItem(m_ptcenP1);
	filer->writeItem(m_ptcenP2);
	filer->writeItem(m_ptedgP3);
	filer->writeItem(m_nedgeNum);

	return filer->filerStatus();
}

Acad::ErrorStatus PDPrism1::dwgInFields(AcDbDwgFiler* filer)
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
	if (version > VERSION_PDPRISM1)
		return Acad::eMakeMeProxy;

	// Read the data members.
	switch (version)
	{
	case (1):

		filer->readItem(&m_ptcenP1);
		filer->readItem(&m_ptcenP2);
		filer->readItem(&m_ptedgP3);
		filer->readItem(&m_nedgeNum);
		break;
	}

#ifdef _USEAMODELER_
	if(m_3dGeom.isNull())
		createBody();
#endif

	return filer->filerStatus();
}


Acad::ErrorStatus PDPrism1::dxfOutFields(AcDbDxfFiler* filer) const
{
    assertReadEnabled();
    Acad::ErrorStatus es;

    if ((es = PDPrimary3D::dxfOutFields(filer))
        != Acad::eOk)
    {
        return es;
    }

	// Write subclass marker.
    filer->writeItem(AcDb::kDxfSubclass, _T("PDPrism1"));

	// Write version number.
	filer->writeItem(AcDb::kDxfInt16, (Adesk::UInt16) VERSION_PDPRISM1);

	// Write data members.
    filer->writeItem(AcDb::kDxfXCoord, m_ptcenP1);
	filer->writeItem(AcDb::kDxfXCoord+1, m_ptcenP2);
	filer->writeItem(AcDb::kDxfXCoord+2, m_ptedgP3);
	filer->writeItem(AcDb::kDxfInt32,m_nedgeNum);

    return filer->filerStatus();
}

Acad::ErrorStatus PDPrism1::dxfInFields(AcDbDxfFiler* filer)
{
    assertWriteEnabled();
    Acad::ErrorStatus es = Acad::eOk;
    struct resbuf rb;

    if ((PDPrimary3D::dxfInFields(filer) != Acad::eOk)
        || !filer->atSubclassData(_T("PDPrism1")))
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
		if (version > VERSION_PDPRISM1)
			return Acad::eMakeMeProxy;
	}

	// Read the data members.
	switch (version){
	case (1):
	    while ((es == Acad::eOk) && ((es = filer->readResBuf(&rb)) == Acad::eOk)){

			switch (rb.restype){
			case AcDb::kDxfXCoord:
				m_ptcenP1 = asPnt3d(rb.resval.rpoint);
				break;

			case AcDb::kDxfXCoord+1:
				m_ptcenP2 = asPnt3d(rb.resval.rpoint);
				break;


			case AcDb::kDxfXCoord+2:
				m_ptedgP3 = asPnt3d(rb.resval.rpoint);
				break;

			case AcDb::kDxfInt32:
				m_nedgeNum=rb.resval.rlong;
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

    return filer->filerStatus();
}

Acad::ErrorStatus PDPrism1::getVertices(AcGePoint3dArray& vertexArray)const 
{
	assertReadEnabled();

	/*while(!vertexArray.isEmpty())
		vertexArray.removeLast();*/
	vertexArray.setLogicalLength(2 * m_nedgeNum);

	/*AcGePoint3dArray vertexArray1;*/
	AcGeVector3d vecZ = m_ptcenP2 - m_ptcenP1;
	AcGeVector3d VecX = (m_ptedgP3 - m_ptcenP1).normal();
	AcGeVector3d VecY = vecZ.crossProduct(VecX).normal();
	AcGeVector3d XYNormalZ = VecX.crossProduct(VecY);

	double angle = 2.0 * PI / m_nedgeNum;
	double r = (m_ptedgP3.distanceTo(m_ptcenP1)) / cos(angle / 2.0);

	AcGePoint3d verPoint = m_ptedgP3 - VecY * r * sin(angle / 2.0);
	AcGePoint3d verPoint1 = verPoint + vecZ;
	for(int i = 0; i < (int)m_nedgeNum; i++)
	{
		verPoint.rotateBy(angle, XYNormalZ, m_ptcenP1);
		/*vertexArray.append(verPoint);*/
		vertexArray[i] = verPoint;
		verPoint1.rotateBy(angle, XYNormalZ, m_ptcenP2);
		vertexArray[i + m_nedgeNum] = verPoint1;
		/*vertexArray1.append(verPoint1);*/
	}
	/*vertexArray.append(vertexArray1);*/

	return Acad::eOk;
}

Acad::ErrorStatus PDPrism1::getVerticesInP1Plane(AcGePoint3dArray& vertexArray)const
{
	assertReadEnabled();

	vertexArray.setLogicalLength(m_nedgeNum);

	AcGeVector3d vecZ = m_ptcenP2 - m_ptcenP1;
	AcGeVector3d VecX = (m_ptedgP3 - m_ptcenP1).normal();
	AcGeVector3d VecY = vecZ.crossProduct(VecX).normal();
	AcGeVector3d XYNormalZ = VecX.crossProduct(VecY);

	double angle = 2.0 * PI / m_nedgeNum;
	double r = (m_ptedgP3.distanceTo(m_ptcenP1)) / cos(angle / 2.0);

	AcGePoint3d verPoint = m_ptedgP3 - VecY * r * sin(angle / 2.0);
	for(int i = 0; i < (int)m_nedgeNum; i++)
	{
		verPoint.rotateBy(angle, XYNormalZ, m_ptcenP1);
		vertexArray[i] = verPoint;
	}

	return Acad::eOk;
}


Acad::ErrorStatus PDPrism1::getVerticesInP2Plane(AcGePoint3dArray& vertexArray)const
{
	assertReadEnabled();

	vertexArray.setLogicalLength(m_nedgeNum);

	AcGeVector3d vecZ = m_ptcenP2 - m_ptcenP1;
	AcGeVector3d VecX = (m_ptedgP3 - m_ptcenP1).normal();
	AcGeVector3d VecY = vecZ.crossProduct(VecX).normal();
	AcGeVector3d XYNormalZ = VecX.crossProduct(VecY);

	double angle = 2.0 * PI / m_nedgeNum;
	double r = (m_ptedgP3.distanceTo(m_ptcenP1)) / cos(angle / 2.0);

	AcGePoint3d verPoint = m_ptedgP3 - VecY * r * sin(angle / 2.0);
	verPoint += vecZ;
	for(int i = 0; i < (int)m_nedgeNum; i++)
	{
		verPoint.rotateBy(angle, XYNormalZ, m_ptcenP2);
		vertexArray[i] = verPoint;
	}

	return Acad::eOk;
}

#ifdef _OBJECTARX2010_
Adesk::Boolean PDPrism1::subWorldDraw(AcGiWorldDraw* mode)
#else
Adesk::Boolean PDPrism1::worldDraw(AcGiWorldDraw* mode)
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
	AcGePoint3dArray verArr;
	getVertices(verArr);
	if(getCalMaxPrecision())
	{
		//优化
		double dMax= m_ptedgP3.distanceTo(m_ptcenP1);
		dMax = dMax * tan(PI / m_nedgeNum);
		int Precision =  (int)(PI / acos(1.0 - mode->deviation(kAcGiMaxDevForCircle, m_ptedgP3) / dMax));
		if(Precision < PRIMARY3D_SIMPLIZE_SCALE)
		{
			AcGePoint3d pt[2];
			//下底面的边
			int i;
			for(i = 0; i < verArr.length() / 2 - 1; ++i)
			{
				pt[0] = verArr[i];
				pt[1] = verArr[i + 1];
				if(mode->geometry().polyline(2, pt))
					return Adesk::kTrue;
			}
			pt[0] = verArr[verArr.length() / 2 - 1];
			pt[1] = verArr[0];
			if(mode->geometry().polyline(2, pt))
				return Adesk::kTrue;
			//上底面的边
			for(i = verArr.length() / 2; i < verArr.length() - 1; ++i)
			{
				pt[0] = verArr[i];
				pt[1] = verArr[i + 1];
				if(mode->geometry().polyline(2, pt))
					return Adesk::kTrue;
			}
			pt[0] = verArr[verArr.length() - 1];
			pt[1] = verArr[verArr.length() / 2];
			if(mode->geometry().polyline(2, pt))
				return Adesk::kTrue;
			//侧面的边
			for(i = 0; i < verArr.length() / 2; ++i)
			{
				pt[0] = verArr[i];
				pt[1] = verArr[i + verArr.length() / 2];
				if(mode->geometry().polyline(2, pt))
					return Adesk::kTrue;
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
			int gsIndex = 0;
			mode->subEntityTraits().setFillType(kAcGiFillAlways);
			//下底面
			//添加实体模型各面的GS标记
			//added by szw 2009.11.18 : begin
			mode->subEntityTraits().setSelectionMarker(++gsIndex);
			//added by szw 2009.11.18 : end
			if(mode->geometry().polygon(verArr.length() / 2, verArr.asArrayPtr()))
				return Adesk::kTrue;
			//上底面
			//added by szw 2009.11.18 : begin
			mode->subEntityTraits().setSelectionMarker(++gsIndex);
			//added by szw 2009.11.18 : end
			if(mode->geometry().polygon(verArr.length() / 2, &(verArr.asArrayPtr()[verArr.length() / 2])))
				return Adesk::kTrue;
			//侧面
			AcGePoint3d pt[4];
			for(int i = 0; i < verArr.length() / 2 - 1; i++)
			{
				pt[0] = verArr[i];
				pt[1] = verArr[i + 1];
				pt[2] = verArr[i + 1 + verArr.length() / 2];
				pt[3] = verArr[i + verArr.length() / 2];
				mode->subEntityTraits().setSelectionMarker(++gsIndex);
				if(mode->geometry().polygon(4, pt))
					return Adesk::kTrue;
			}
			pt[0] = verArr[verArr.length() / 2 - 1];
			pt[1] = verArr[0];
			pt[2] = verArr[verArr.length() / 2];
			pt[3] = verArr[verArr.length() - 1];
			//added by szw 2009.11.18 : begin
			mode->subEntityTraits().setSelectionMarker(++gsIndex);
			//added by szw 2009.11.18 : end
			if(mode->geometry().polygon(4, pt))
				return Adesk::kTrue;
		}
#ifndef _ALWAYSSHOWWIRE_
		break;
	case kAcGiStandardDisplay:
	case kAcGiSaveWorldDrawForR12:
	case kAcGiSaveWorldDrawForProxy:
#endif
		{
			AcGePoint3d pt[2];
			int gsIndex = 0;
			//下底面的边
			int i;
			//统一线框模型和实体模型的GS标记
			//modified by szw 2009.11.18 : begin
			++gsIndex;
			for(i = 0; i < verArr.length() / 2 - 1; i++)
			{
				pt[0] = verArr[i];
				pt[1] = verArr[i + 1];
				mode->subEntityTraits().setSelectionMarker(gsIndex);
				if(mode->geometry().polyline(2, pt))
					return Adesk::kTrue;
			}
			pt[0] = verArr[verArr.length() / 2 - 1];
			pt[1] = verArr[0];
			mode->subEntityTraits().setSelectionMarker(gsIndex);
			if(mode->geometry().polyline(2, pt))
				return Adesk::kTrue;
			//上底面的边
			++gsIndex;
			for(i = verArr.length() / 2; i < verArr.length() - 1; i++)
			{
				pt[0] = verArr[i];
				pt[1] = verArr[i + 1];
				mode->subEntityTraits().setSelectionMarker(gsIndex);
				if(mode->geometry().polyline(2, pt))
					return Adesk::kTrue;
			}
			pt[0] = verArr[verArr.length() - 1];
			pt[1] = verArr[verArr.length() / 2];
			mode->subEntityTraits().setSelectionMarker(gsIndex);
			if(mode->geometry().polyline(2, pt))
				return Adesk::kTrue;
			//侧面的边
			for(i = 0; i < verArr.length() / 2; i++)
			{
				pt[0] = verArr[i];
				pt[1] = verArr[i + verArr.length() / 2];
				mode->subEntityTraits().setSelectionMarker(++gsIndex);
				if(mode->geometry().polyline(2, pt))
					return Adesk::kTrue;
			}
			//modified by szw 2009.11.18 : end
		}
#ifndef _ALWAYSSHOWWIRE_
		break;
	}
#endif
#endif
	return Adesk::kTrue;
}

/*
Acad::ErrorStatus 
PDPrism1::getCoordSys(AcGeMatrix3d& mat) 
{
   assertReadEnabled();
   AcGeVector3d yAxis=(midPt-cenPt1).normal();
   AcGeVector3d zAxis=(cenPt1-cenPt2).normal();
   AcGeVector3d xAxis=zAxis.crossProduct(yAxis);
   mat.getCoordSystem(cenPt1,xAxis,yAxis,zAxis);
   return Acad::eOk;
}*/


/*
Acad::ErrorStatus 
PDPrism1::transformBy(const AcGeMatrix3d &xfm)
{
   assertWriteEnabled();
   m_ptcenP1.transformBy(xfm);
   m_ptcenP2.transformBy(xfm);
   return Acad::eOk;
}

Acad::ErrorStatus 
PDPrism1::getOsnapPoints(
    AcDb::OsnapMode       osnapMode,
    int                   gsSelectionMark,
    const AcGePoint3d&    pickPoint,
    const AcGePoint3d&    lastPoint,
    const AcGeMatrix3d&   viewXform,
    AcGePoint3dArray&     snapPoints,
    AcDbIntArray&         geomIds) const
{
	assertReadEnabled();

    switch(osnapMode) {
    
	case AcDb::kOsModeEnd:
         return osnapEnd(pickPoint, snapPoints);
    case AcDb::kOsModeCen:
		 return osnapCen(pickPoint, snapPoints);
    case AcDb::kOsModeQuad:
    case AcDb::kOsModeNode:
    case AcDb::kOsModeIns:
    case AcDb::kOsModePerp:
    case AcDb::kOsModeTan:
    default:
        break;
    }
    return Acad::eInvalidInput;
}

Acad::ErrorStatus   
PDPrism1::osnapCen(const AcGePoint3d& pickPoint,
				AcGePoint3dArray& snapPoints) const
{
     AcGePoint3d tCenPt;
	 tCenPt=m_ptcenP1+AcGeVector3d(0,0,1)*height;
	 snapPoints.append(tCenPt);
	 snapPoints.append(m_ptcenP1);
	 return Acad::eOk;
}
Acad::ErrorStatus   
PDPrism1::osnapEnd(const AcGePoint3d& pickPoint,
				AcGePoint3dArray& snapPoints)const 
{
	 AcGePoint3dArray verArr;
	 this->getVertices(verArr);
	 for(int i=0;i<verArr.length();i++){
		 snapPoints.append(verArr[i]);
	 }
	 return Acad::eOk;
}
	
*/   


Acad::ErrorStatus PDPrism1::getpointCenP1(AcGePoint3d& ptcenP1)
{
	assertReadEnabled();
	ptcenP1 = m_ptcenP1;
	return Acad::eOk;
}

Acad::ErrorStatus PDPrism1::getpointCenP2(AcGePoint3d& ptcenP2)
{
	assertReadEnabled();
	ptcenP2 = m_ptcenP2;
	return Acad::eOk;
}

Acad::ErrorStatus PDPrism1::getpointEdgP3(AcGePoint3d& ptedgP3)
{
	assertReadEnabled();
	ptedgP3 = m_ptedgP3;
	return Acad::eOk;
}

Acad::ErrorStatus PDPrism1::getedgeNum(Adesk::UInt32& edgeNum)
{
	assertReadEnabled();
	edgeNum = m_nedgeNum;
	return Acad::eOk;
}


Acad::ErrorStatus PDPrism1:: setParameters(AcGePoint3d ptcenP1,AcGePoint3d ptcenP2,
		                            AcGePoint3d ptedgP3,Adesk::UInt32 edgeNum)
{
   assertWriteEnabled();


	m_ptcenP1 = ptcenP1 ;
	m_ptcenP2 = ptcenP2 ;
	m_ptedgP3 = ptedgP3 ;

    AcGeVector3d  p1p3, p1p2;
    p1p3 = ptedgP3 - ptcenP1;
    p1p2 = ptcenP2 - ptcenP1;
    if(p1p3.isZeroLength() && p1p2.isZeroLength())
    {
        m_ptcenP2 = m_ptcenP1 + AcGeVector3d(0, 0, 1);
        m_ptedgP3 = m_ptcenP1 + AcGeVector3d(1, 0, 0);
    }
    else if(p1p3.isZeroLength())
    {
        AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax, Ay;
        if (fabs(p1p2[X]) <1.0/64 && fabs(p1p2[Y])<1.0/64) 
            Ax = Wy.crossProduct(p1p2);
        else
            Ax = Wz.crossProduct(p1p2);
        Ax.normalize();

        p1p3 = Ax;
        m_ptedgP3 = m_ptcenP1 + p1p3;
    }
    else if(p1p2.isZeroLength())
    {
        AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax, Ay;
        if (fabs(p1p3[X]) <1.0/64 && fabs(p1p3[Y])<1.0/64) 
            Ax = Wy.crossProduct(p1p3);
        else
            Ax = Wz.crossProduct(p1p3);
        Ax.normalize();

        p1p2 = Ax;
        m_ptcenP2 = m_ptcenP1 + p1p2;
    }
    else if(p1p2.isParallelTo(p1p3))
    {
        AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax, Ay;
        if (fabs(p1p2[X]) <1.0/64 && fabs(p1p2[Y])<1.0/64) 
            Ax = Wy.crossProduct(p1p2);
        else
            Ax = Wz.crossProduct(p1p2);
        Ax.normalize();

        p1p3 = Ax * p1p3.length();
        m_ptedgP3 = m_ptcenP1 + p1p3;
    }

	if(edgeNum>= 3)
		m_nedgeNum=edgeNum;
	else
		m_nedgeNum=3;

#ifdef _USEAMODELER_
	createBody();
#endif

	return Acad::eOk;
}// added by linlin 20050929


#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDPrism1::subGetOsnapPoints(
	AcDb::OsnapMode     osnapMode,
	Adesk::GsMarker     gsSelectionMark,
	const AcGePoint3d&  pickPoint,
	const AcGePoint3d&  lastPoint,
	const AcGeMatrix3d& viewXform,
	AcGePoint3dArray&   snapPoints,
	AcDbIntArray &   geomIds) const
#else
Acad::ErrorStatus PDPrism1::getOsnapPoints(
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

	if(verArr.length() < 2)
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
			int arrLen = verArr.length();
			AcGePoint3d pt;
			//下底面边的中点
			int i;
			for(i = 0; i < arrLen / 2 - 1; i++)
			{
				pt = verArr[i] + (verArr[i + 1] - verArr[i]) / 2.0;
				snapPoints.append(pt);
			}
			pt = verArr[arrLen / 2 - 1] + (verArr[0] - verArr[arrLen / 2 - 1]) / 2.0;
			snapPoints.append(pt);

			//上底面边的中点
			for(i = arrLen / 2; i < arrLen - 1; i++)
			{
				pt = verArr[i] + (verArr[i + 1] - verArr[i]) / 2.0;
				snapPoints.append(pt);
			}
			pt = verArr[arrLen - 1] + (verArr[arrLen / 2] - verArr[arrLen - 1]) / 2.0;
			snapPoints.append(pt);

			//棱边的中点
			for(i = 0; i < arrLen / 2; i++)
			{
				pt = verArr[i] + (verArr[i + arrLen / 2] - verArr[i]) / 2.0;
				snapPoints.append(pt);
			}
		}
		break;
	case AcDb::kOsModeCen:
		{
			//重新定义对象中心点的捕捉方式，同时满足实体模型和线框模型的捕捉 
			//modified by szw 2009.11.18 : begin
			AcGePoint3d pt;
			if(gsSelectionMark_int == 1)
			{
				snapPoints.append(m_ptcenP1);
			}
			else if(gsSelectionMark_int == 2)
			{
				snapPoints.append(m_ptcenP2);
			}
			else if(gsSelectionMark_int < (int)m_nedgeNum + 2)
			{
				pt = verArr[gsSelectionMark_int - 3] +
					(verArr[gsSelectionMark_int - 3 + m_nedgeNum + 1] - 
					verArr[gsSelectionMark_int - 3]) / 2.0;
				snapPoints.append(pt);
			}
			else if(gsSelectionMark_int == (int)m_nedgeNum + 2)
			{
				pt = verArr[gsSelectionMark_int - 3] +
					(verArr[m_nedgeNum] - 
					verArr[gsSelectionMark_int - 3]) / 2.0;
				snapPoints.append(pt);
			}
			snapPoints.append(m_ptcenP1 + (m_ptcenP2 - m_ptcenP1) / 2.0);
			//modified by szw 2009.11.18 : end
			break;
		}
	case AcDb::kOsModeQuad:
		break;
	case AcDb::kOsModeNode:
		break;
	case AcDb::kOsModeIns:
		snapPoints.append(m_ptcenP1);
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
			if(gsSelectionMark_int == 1)
			{
				for(int i = 0; i < (int)m_nedgeNum - 1; ++i)
				{
					vec = verArr[i] - verArr[i+1];
					vec.normalize();
					line.set(verArr[i], vec);
					pt = line.closestPointTo(lastPoint);
					snapPoints.append(pt);
				}
				vec = verArr[m_nedgeNum - 1] - verArr[0];
				vec.normalize();
				line.set(verArr[0], vec);
				pt = line.closestPointTo(lastPoint);
				snapPoints.append(pt);
			}
			else if(gsSelectionMark_int == 2)
			{
				for(int i = 0; i < (int)m_nedgeNum - 1; ++i)
				{
					vec = verArr[m_nedgeNum + i] - verArr[m_nedgeNum + i + 1];
					vec.normalize();
					line.set(verArr[m_nedgeNum + i], vec);
					pt = line.closestPointTo(lastPoint);
					snapPoints.append(pt);
				}
				vec = verArr[m_nedgeNum * 2 - 1] - verArr[m_nedgeNum];
				vec.normalize();
				line.set(verArr[m_nedgeNum], vec);
				pt = line.closestPointTo(lastPoint);
				snapPoints.append(pt);
			}
			else
			{
				vec = verArr[gsSelectionMark_int - 3]
				- verArr[gsSelectionMark_int - 3 + m_nedgeNum];
				vec.normalize();
				line.set(verArr[gsSelectionMark_int - 3], vec);
				pt = line.closestPointTo(lastPoint);
				snapPoints.append(pt);
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
			int arrLen = verArr.length();
			//下底面的边
			int i;
			for(i = 0; i < arrLen / 2 - 1; i++)
			{
				lnsg.set(verArr[i], verArr[i + 1]);
				pt = lnsg.projClosestPointTo(pickPoint, viewDir);
				snapPoints.append(pt);
			}
			lnsg.set(verArr[arrLen / 2 - 1], verArr[0]);
			pt = lnsg.projClosestPointTo(pickPoint, viewDir);
			snapPoints.append(pt);

			//上底面的边
			for(i = arrLen / 2; i < arrLen - 1; i++)
			{
				lnsg.set(verArr[i], verArr[i + 1]);
				pt = lnsg.projClosestPointTo(pickPoint, viewDir);
				snapPoints.append(pt);
			}
			lnsg.set(verArr[arrLen - 1], verArr[arrLen / 2]);
			pt = lnsg.projClosestPointTo(pickPoint, viewDir);
			snapPoints.append(pt);

			//棱边
			for(i = 0; i < arrLen / 2; i++)
			{
				lnsg.set(verArr[i], verArr[i + arrLen / 2]);
				pt = lnsg.projClosestPointTo(pickPoint, viewDir);
				snapPoints.append(pt);
			}
		}
		break;
	default:
		break;
	}

	return Acad::eOk;
}


#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDPrism1::subGetGripPoints(
	AcGePoint3dArray& gripPoints,
	AcDbIntArray& osnapModes,
	AcDbIntArray& geomIds) const
#else
Acad::ErrorStatus PDPrism1::getGripPoints(
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
Acad::ErrorStatus PDPrism1::subExplode(AcDbVoidPtrArray& entitySet) const
#else
Acad::ErrorStatus PDPrism1::explode(AcDbVoidPtrArray& entitySet) const
#endif
{
  assertReadEnabled();

  AcGePoint3dArray vertexArray;
	AcGeVector3d vecZ = m_ptcenP2 - m_ptcenP1;
	AcGeVector3d VecX = (m_ptedgP3 - m_ptcenP1).normal();
	AcGeVector3d VecY = vecZ.crossProduct(VecX).normal();
  AcGeVector3d XYNormalZ = VecX.crossProduct(VecY);

	double angle = 2.0 * PI / m_nedgeNum;
	double r = (m_ptedgP3.distanceTo(m_ptcenP1)) / cos(angle / 2.0);
  AcGePoint3d verPoint = m_ptedgP3 - VecY * r * sin(angle / 2.0);
  vertexArray.append(verPoint);
  int i;
	for(i = 0; i<(int)m_nedgeNum; i++)
    {
		  verPoint.rotateBy(angle, XYNormalZ, m_ptcenP1);
		  vertexArray.append(verPoint);
	  }

  AcDbVoidPtrArray curveSegments;
  for(i = 0; i < (int)m_nedgeNum; i++)
    {
      AcDbLine *pLine;
      pLine = new AcDbLine(vertexArray[i], vertexArray[i + 1]);
      pLine->setNormal(XYNormalZ);
      curveSegments.append(pLine);
    }
  AcDbVoidPtrArray regions;
  Acad::ErrorStatus es = Acad::eOk;
  es = AcDbRegion::createFromCurves(curveSegments, regions);

  if(es == Acad::eOk && !regions.isEmpty())
  {
	  AcGeVector3d regionNor;
	  ((AcDbRegion*)(regions[0]))->getNormal(regionNor);
	  double extrudeHight = m_ptcenP1.distanceTo(m_ptcenP2);
	  if(!vecZ.isCodirectionalTo(regionNor))
		  extrudeHight = -extrudeHight;
      AcDb3dSolid *pBody;
      pBody = new AcDb3dSolid;
      es = pBody->extrude((AcDbRegion*)(regions[0]), extrudeHight, 0);
      if(es != Acad::eOk)
        {
          delete pBody;
          for(i = 0; i < curveSegments.length(); i++)
            delete (AcRxObject*)curveSegments[i];
          for(i = 0; i < regions.length(); i++)
            delete (AcRxObject*)regions[i];
          return Acad::eNotApplicable;
        }
      pBody->setPropertiesFrom(this);
      entitySet.append(pBody);
    }
  else
    {
      for(i = 0; i < curveSegments.length(); i++)
        delete (AcRxObject*)curveSegments[i];
      for(i = 0; i < regions.length(); i++)
        delete (AcRxObject*)regions[i];
      return Acad::eNotApplicable;
    }

  for(i = 0; i < curveSegments.length(); i++)
    delete (AcRxObject*)curveSegments[i];
  for(i = 0; i < regions.length(); i++)
    delete (AcRxObject*)regions[i];

	return Acad::eOk;
}

//  [9/5/2007 suzhiyong]
Acad::ErrorStatus PDPrism1::explodeTo3DSolid(AcDb3dSolid* &p3dSolid) const
{
	assertReadEnabled();

	AcGePoint3dArray vertexArray;
	AcGeVector3d vecZ = m_ptcenP2 - m_ptcenP1;
	AcGeVector3d VecX = (m_ptedgP3 - m_ptcenP1).normal();
	AcGeVector3d VecY = vecZ.crossProduct(VecX).normal();
	AcGeVector3d XYNormalZ = VecX.crossProduct(VecY);

	double angle = 2.0 * PI / m_nedgeNum;
	double r = (m_ptedgP3.distanceTo(m_ptcenP1)) / cos(angle / 2.0);
	AcGePoint3d verPoint = m_ptedgP3 - VecY * r * sin(angle / 2.0);
	vertexArray.append(verPoint);
	int i;
	for(i = 0; i<(int)m_nedgeNum; i++)
	{
		verPoint.rotateBy(angle, XYNormalZ, m_ptcenP1);
		vertexArray.append(verPoint);
	}

	AcDbVoidPtrArray curveSegments;
	for(i = 0; i < (int)m_nedgeNum; i++)
	{
		AcDbLine *pLine;
		pLine = new AcDbLine(vertexArray[i], vertexArray[i + 1]);
		pLine->setNormal(XYNormalZ);
		curveSegments.append(pLine);
	}
	AcDbVoidPtrArray regions;
	Acad::ErrorStatus es = Acad::eOk;
	es = AcDbRegion::createFromCurves(curveSegments, regions);

	if(es == Acad::eOk && !regions.isEmpty())
	{
		AcGeVector3d regionNor;
		((AcDbRegion*)(regions[0]))->getNormal(regionNor);
		double extrudeHight = m_ptcenP1.distanceTo(m_ptcenP2);
		if(!vecZ.isCodirectionalTo(regionNor))
			extrudeHight = -extrudeHight;
		AcDb3dSolid *pBody;
		pBody = new AcDb3dSolid;
		es = pBody->extrude((AcDbRegion*)(regions[0]), extrudeHight, 0);
		if(es != Acad::eOk)
		{
			delete pBody;
			for(i = 0; i < curveSegments.length(); i++)
				delete (AcRxObject*)curveSegments[i];
			for(i = 0; i < regions.length(); i++)
				delete (AcRxObject*)regions[i];
			return Acad::eNotApplicable;
		}
		pBody->setPropertiesFrom(this);
		p3dSolid = pBody;
	}
	else
	{
		for(i = 0; i < curveSegments.length(); i++)
			delete (AcRxObject*)curveSegments[i];
		for(i = 0; i < regions.length(); i++)
			delete (AcRxObject*)regions[i];
		return Acad::eNotApplicable;
	}

	for(i = 0; i < curveSegments.length(); i++)
		delete (AcRxObject*)curveSegments[i];
	for(i = 0; i < regions.length(); i++)
		delete (AcRxObject*)regions[i];

	return Acad::eOk;
}

void PDPrism1::createBody()
{
    assertWriteEnabled();
// #ifdef _OBJECTARX2004_
    if(m_nedgeNum < 3)  // 棱边数不能小于3
        return ;

    AcGePoint3d* vertices;
    PolygonVertexData** vertexData;
    vertices = new AcGePoint3d[m_nedgeNum];
    vertexData = new PolygonVertexData*[m_nedgeNum];

    AcGeVector3d vecZ = m_ptcenP2 - m_ptcenP1;
    AcGeVector3d VecX = (m_ptedgP3 - m_ptcenP1).normal();
    AcGeVector3d VecY = vecZ.crossProduct(VecX).normal();
    AcGeVector3d XYNormalZ = VecX.crossProduct(VecY).normal();

    double angle = 2.0 * PI / m_nedgeNum;
    double r = (m_ptedgP3.distanceTo(m_ptcenP1)) / cos(angle / 2.0);

    AcGePoint3d verPoint = m_ptedgP3 - VecY * r * sin(angle / 2.0);
    for(Adesk::UInt32 i = 0; i < m_nedgeNum; ++i)
    {
        verPoint.rotateBy(angle, XYNormalZ, m_ptcenP1);
        vertices[i] = verPoint;
        vertexData[i] = NULL;
    }

    m_3dGeom = Body::extrusion((Point3d*)vertices, vertexData, m_nedgeNum, *(Vector3d*)&XYNormalZ, *(Vector3d*)&vecZ);
    delete [] vertices;
    delete [] vertexData;
// #endif
}

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDPrism1::subGetGeomExtents(AcDbExtents& extents) const
#else
Acad::ErrorStatus PDPrism1::getGeomExtents(AcDbExtents& extents) const
#endif
{
    assertReadEnabled();
    extents.set(m_ptcenP1, m_ptcenP1);
    AcGePoint3dArray ptarr;
    getVertices(ptarr);
    int i, arrLen = ptarr.length();
    for(i = 0; i < arrLen; ++i)
        extents.addPoint(ptarr[i]);
    return Acad::eOk;
}

#ifdef _OBJECTARX2010_
void PDPrism1::subList() const
#else
void PDPrism1::list() const
#endif
{
	assertReadEnabled();
#ifdef _OBJECTARX2010_
	PDPrimary3D::subList();
#else
	PDPrimary3D::list();
#endif
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4738/*"\n 底面圆心(%f,%f,%f)\n"*/,m_ptcenP1.x,m_ptcenP1.y,m_ptcenP1.z);
    ads_printf(LANGUAGE_DEFINE_MACRO_V25_4739/*" 顶面圆心(%f,%f,%f)\n"*/,m_ptcenP2.x,m_ptcenP2.y,m_ptcenP2.z);
    ads_printf(LANGUAGE_DEFINE_MACRO_V25_4768/*" 棱柱上任一边的中点(%f,%f,%f)\n"*/,m_ptedgP3.x,m_ptedgP3.y,m_ptedgP3.z);
    ads_printf(LANGUAGE_DEFINE_MACRO_V25_4766/*" 棱边数=%d\n\n"*/,m_nedgeNum); 
	return ;
}// added by linlin 20050810

const AcGePoint3d & PDPrism1::getBottomCenter() const
{
	assertReadEnabled();
	return m_ptcenP1;
}

const AcGePoint3d & PDPrism1::getTopCenter() const
{
	assertReadEnabled();
	return m_ptcenP2;
}

const AcGePoint3d & PDPrism1::getBottomEdgeCenter() const
{
	assertReadEnabled();
	return m_ptedgP3;
}

Adesk::UInt32 PDPrism1::getEdgeNum() const
{
	assertReadEnabled();
	return m_nedgeNum;
}
