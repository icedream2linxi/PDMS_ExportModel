// PDWedge.cpp: implementation of the PDWedge class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "language25.h"
#include "PDWedge.h"
#include "dbproxy.h"
#include "acgi.h"
#include "geassign.h"
#include <dbsol3d.h>
#include "dbapserv.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define VERSION_PDWEDGE 2

#ifdef _OBJECTARX2000_
ACRX_DXF_DEFINE_MEMBERS(PDWedge, PDPrimary3D,
						AcDb::kDHL_CURRENT, AcDb::kMReleaseCurrent,
						AcDbProxyEntity::kAllAllowedBits,
						PDWEDGE, Gle);
#else
ACRX_DXF_DEFINE_MEMBERS(PDWedge, PDPrimary3D, AcDbProxyEntity::kAllAllowedBits, PDWEDGE, Gle);
#endif

void PDWedge::setDefault()
{
	PDPrimary3D::setDefault();
#if defined(NDEBUG) || !defined(_DEBUG)
	m_ptP1.set(0, 1, 0);
	m_ptP2.set(0, 0, 0);
	m_ptP3.set(1, 0, 0);
	m_ptP4.set(0, 0, 1);
#else
	m_ptP1.set(0, 1000, 0);
	m_ptP2.set(0, 0, 0);
	m_ptP3.set(1000, 0, 0);
	m_ptP4.set(0, 0, 1000);
#endif
}
bool PDWedge::isValidData(double &f)
{
	if(!PDPrimary3D::isValidData(f))
		return false;
	if(!isValidFloat(f=m_ptP1.x) || !isValidFloat(f=m_ptP1.y) || !isValidFloat(f=m_ptP1.z))
		return false;
	if(!isValidFloat(f=m_ptP2.x) || !isValidFloat(f=m_ptP2.y) || !isValidFloat(f=m_ptP2.z))
		return false;
	if(!isValidFloat(f=m_ptP3.x) || !isValidFloat(f=m_ptP3.y) || !isValidFloat(f=m_ptP3.z))
		return false;
	if(!isValidFloat(f=m_ptP4.x) || !isValidFloat(f=m_ptP4.y) || !isValidFloat(f=m_ptP4.z))
		return false;
	return true;
}

//zxb,20090113,增加audit，暂时只check自身成员数据
Acad::ErrorStatus PDWedge::audit(AcDbAuditInfo* pAuditInfo)
{
	return auditPDPrimary3D(this,pAuditInfo,_T("PDWedge"));
}

PDWedge::PDWedge(bool HasSnap/* = false*/) : PDPrimary3D(HasSnap)
{
	m_ptP1.set(0, 1, 0);
	m_ptP2.set(0, 0, 0);
	m_ptP3.set(1, 0, 0);
	m_ptP4.set(0, 0, 1);
#ifdef _USEAMODELER_
	createBody();
#endif
	GetMaxLength();
}

PDWedge::~PDWedge()
{

}

PDWedge::PDWedge(AcGePoint3d P1,AcGePoint3d P2,AcGePoint3d P3,double height, 
				 bool HasSnap/* = false*/,int CorrType/* = 1*/) : PDPrimary3D(HasSnap)
{
	m_ptP1 = P1;
	m_ptP2 = P2;
	m_ptP3 = P3;
	AcGeVector3d p2p4, p2p3, p2p1;
	p2p3 = P3 - P2;
	p2p1 = P1 - P2;
	if(p2p3.isZeroLength() && p2p1.isZeroLength())
	{
		m_ptP1 = m_ptP2 + AcGeVector3d(0, 1, 0);
		m_ptP3 = m_ptP2 + AcGeVector3d(1, 0, 0);
	}
	else if(p2p3.isZeroLength())
	{
		AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax, Ay;
		if (fabs(p2p1[X]) <1.0/64 && fabs(p2p1[Y])<1.0/64) 
			Ax = Wy.crossProduct(p2p1);
		else
			Ax = Wz.crossProduct(p2p1);
		Ax.normalize();

		p2p3 = Ax;
		m_ptP3 = m_ptP2 + p2p3;
	}
	else if(p2p1.isZeroLength())
	{
		AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax, Ay;
		if (fabs(p2p3[X]) <1.0/64 && fabs(p2p3[Y])<1.0/64) 
			Ax = Wy.crossProduct(p2p3);
		else
			Ax = Wz.crossProduct(p2p3);
		Ax.normalize();

		p2p1 = Ax;
		m_ptP1 = m_ptP2 + p2p1;
	}
	else if(p2p1.isParallelTo(p2p3))
	{
		AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax, Ay;
		if (fabs(p2p3[X]) <1.0/64 && fabs(p2p3[Y])<1.0/64) 
			Ax = Wy.crossProduct(p2p3);
		else
			Ax = Wz.crossProduct(p2p3);
		Ax.normalize();

		p2p1 = Ax * p2p1.length();
		m_ptP1 = m_ptP2 + p2p1;
	}
	if (CorrType) 
		p2p4 = p2p3.crossProduct(p2p1).normalize() * height;
	else
		p2p4=p2p1.crossProduct(p2p3).normalize()*height;  //added by linlin 20050908  
	m_ptP4 = m_ptP2 + p2p4;

#ifdef _USEAMODELER_
	createBody();
#endif
	GetMaxLength();
}

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDWedge::subTransformBy(const AcGeMatrix3d& xform)
#else
Acad::ErrorStatus PDWedge::transformBy(const AcGeMatrix3d& xform)
#endif
{
	assertWriteEnabled();
	// TODO: implement this function.
	m_ptP1.transformBy(xform);
	m_ptP2.transformBy(xform);
	m_ptP3.transformBy(xform);
	m_ptP4.transformBy(xform);

	GetMaxLength();

#ifdef _OBJECTARX2010_
	return PDPrimary3D::subTransformBy(xform);
#else
	return PDPrimary3D::transformBy(xform);
#endif
}

#ifdef _OBJECTARX2010_
Adesk::Boolean PDWedge::subWorldDraw(AcGiWorldDraw* mode)
#else
Adesk::Boolean PDWedge::worldDraw(AcGiWorldDraw* mode)
#endif
{
	assertReadEnabled();
	//if(acdbHostApplicationServices()->workingDatabase()->useri1() == 1)
	//	return Adesk::kTrue;

	if (mode->regenAbort())
	{
		return Adesk::kTrue;
	}
#ifdef _USEAMODELER_
#ifdef _OBJECTARX2010_
	PDPrimary3D::subWorldDraw(mode);
#else
	PDPrimary3D::worldDraw(mode);
#endif
#else  
	AcGePoint3dArray pArray;
	getVertices(pArray);
	int Precision = 0;
	if(getCalMaxPrecision())
	{
		//
		Precision = (int)(PI / acos(1.0 - mode->deviation(kAcGiMaxDevForCircle, m_ptP1) * 2.0 / m_dMaxLength));
		if(Precision < PRIMARY3D_SIMPLIZE_SCALE)
		{
			AcGePoint3d pts[4];
			AcGePoint3d pts1[4];
			int i;
			for(i = 0; i < 3; i++)
			{
				pts[i] = pArray[i];
			}
			pts[3] = pts[0];
			for (i = 3; i < 6; i++)
			{
				pts1[i - 3] = pArray[i];
			}
			pts1[3] = pts1[0];
			if(mode->geometry().polyline(4,pts))
				return Adesk::kTrue;
			if(mode->geometry().polyline(4,pts1))
				return Adesk::kTrue;
			AcGePoint3d pts2[2];
			for (i = 0 ;i < 3; i++)
			{
				pts2[0] = pts[i];
				pts2[1] = pts1[i];
				if(mode->geometry().polyline(2,pts2))
					return Adesk::kTrue;
			}
			return Adesk::kTrue;
		}
	}	
	int halfLen =  pArray.length() / 2;
#ifndef _ALWAYSSHOWWIRE_
	switch(mode->regenType())
	{
	case kAcGiHideOrShadeCommand:
	case kAcGiRenderCommand:
#endif
		{
			//添加实体模型各面的GS标记
			//added by szw 2009.11.18 : begin
			int gsIndex = 0;
			//added by szw 2009.11.18 : end
			mode->subEntityTraits().setFillType(kAcGiFillAlways);
			AcGePoint3d pt[4];
			//added by szw 2009.11.18 : begin
			mode->subEntityTraits().setSelectionMarker(++gsIndex);
			//added by szw 2009.11.18 : end
			if(mode->geometry().polygon(halfLen, pArray.asArrayPtr()))
				return Adesk::kTrue;
			//added by szw 2009.11.18 : begin
			mode->subEntityTraits().setSelectionMarker(++gsIndex);
			//added by szw 2009.11.18 : end
			if(mode->geometry().polygon(halfLen, &(pArray[halfLen])))
				return Adesk::kTrue;

			for(int i = 0; i < halfLen - 1; i++)
			{
				pt[0] = pArray[i];
				pt[1] = pArray[i + 1];
				pt[2] = pArray[i + 1 + halfLen];
				pt[3] = pArray[i + halfLen];
				//added by szw 2009.11.18 : begin
				mode->subEntityTraits().setSelectionMarker(++gsIndex);
				//added by szw 2009.11.18 : end
				if(mode->geometry().polygon(4, pt))
					return Adesk::kTrue;
			}
			pt[0] = pArray[halfLen - 1];
			pt[1] = pArray[0];
			pt[2] = pArray[halfLen];
			pt[3] = pArray[pArray.length() - 1];
			//added by szw 2009.11.18 : begin
			mode->subEntityTraits().setSelectionMarker(++gsIndex);
			//added by szw 2009.11.18 : end
			mode->geometry().polygon(4, pt);
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
			//统一线框模型和实体模型的GS标记
			//modified by szw 2009.11.18 : begin
			++gsIndex;
			for(i = 0; i < halfLen - 1; i++)
			{
				pt[0] = pArray[i];
				pt[1] = pArray[i + 1];
				mode->subEntityTraits().setSelectionMarker(gsIndex);
				if(mode->geometry().polyline(2, pt))
					return Adesk::kTrue;
			}
			pt[0] = pArray[halfLen - 1];
			pt[1] = pArray[0];
			mode->subEntityTraits().setSelectionMarker(gsIndex);
			if(mode->geometry().polyline(2, pt))
				return Adesk::kTrue;

			++gsIndex;
			for(i = halfLen; i < pArray.length() - 1; i++)
			{
				pt[0] = pArray[i];
				pt[1] = pArray[i + 1];
				mode->subEntityTraits().setSelectionMarker(gsIndex);
				if(mode->geometry().polyline(2, pt))
					return Adesk::kTrue;
			}
			pt[0] = pArray[pArray.length() - 1];
			pt[1] = pArray[halfLen];
			mode->subEntityTraits().setSelectionMarker(gsIndex);
			if(mode->geometry().polyline(2, pt))
				return Adesk::kTrue;
			for(i = 0; i < halfLen; i++)
			{
				pt[0] = pArray[i];
				pt[1] = pArray[i + halfLen];
				mode->subEntityTraits().setSelectionMarker(++gsIndex);
				if(mode->geometry().polyline(2, pt))
					return Adesk::kTrue;
			}
			//modified by szw 2009.11.18 : end
		}
#ifndef _ALWAYSSHOWWIRE_
		break;
	default:
		break;
	}
#endif
#endif
	return Adesk::kTrue;
}

/*Adesk::Boolean PDWedge::worldDraw(AcGiWorldDraw* mode)
{
assertReadEnabled();
if (mode->regenAbort()) {
return Adesk::kTrue;
}
#ifdef _USEAMODELER_
#ifdef _OBJECTARX2010_
PDPrimary3D::subWorldDraw(mode);
#else
PDPrimary3D::worldDraw(mode);
#endif
#else  
AcGePoint3dArray pArray;
getVertices(pArray);

switch(mode->regenType())
{
case kAcGiStandardDisplay:
case kAcGiSaveWorldDrawForR12:
case kAcGiSaveWorldDrawForProxy:
{
int gsIndex = 0;
AcGePoint3d pt[2];
for(int i = 0; i < pArray.length() / 2 - 1; i++)
{
pt[0] = pArray[i];
pt[1] = pArray[i + 1];
mode->subEntityTraits().setSelectionMarker(++gsIndex);
mode->geometry().polyline(2, pt);
}
pt[0] = pArray[pArray.length() / 2 - 1];
pt[1] = pArray[0];
mode->subEntityTraits().setSelectionMarker(++gsIndex);
mode->geometry().polyline(2, pt);

for(i = pArray.length() / 2; i < pArray.length() - 1; i++)
{
pt[0] = pArray[i];
pt[1] = pArray[i + 1];
mode->subEntityTraits().setSelectionMarker(++gsIndex);
mode->geometry().polyline(2, pt);
}
pt[0] = pArray[pArray.length() - 1];
pt[1] = pArray[pArray.length() / 2];
mode->subEntityTraits().setSelectionMarker(++gsIndex);
mode->geometry().polyline(2, pt);
for(i = 0; i < pArray.length() / 2; i++)
{
pt[0] = pArray[i];
pt[1] = pArray[i + pArray.length() / 2];
mode->subEntityTraits().setSelectionMarker(++gsIndex);
mode->geometry().polyline(2, pt);
}
}
break;
case kAcGiHideOrShadeCommand:
case kAcGiRenderCommand:
{
mode->subEntityTraits().setFillType(kAcGiFillAlways);
AcGePoint3d pt[4];
mode->geometry().polygon(pArray.length() / 2, pArray.asArrayPtr());
mode->geometry().polygon(pArray.length() / 2, &(pArray[pArray.length() / 2]));
for(int i = 0; i < pArray.length() / 2 - 1; i++)
{
pt[0] = pArray[i];
pt[1] = pArray[i + 1];
pt[2] = pArray[i + 1 + pArray.length() / 2];
pt[3] = pArray[i + pArray.length() / 2];
mode->geometry().polygon(4, pt);
}
pt[0] = pArray[pArray.length() / 2 - 1];
pt[1] = pArray[0];
pt[2] = pArray[pArray.length() / 2];
pt[3] = pArray[pArray.length() - 1];
mode->geometry().polygon(4, pt);
}
break;
default:
break;
}
#endif
return Adesk::kTrue;
}*/
//////////////////////////////////////////////////////////
///////		求出上下三角形的顶点，加入到引用数组中	//////
///////		共 2*3 个点								//////
//////////////////////////////////////////////////////////
Acad::ErrorStatus PDWedge::getVertices(AcGePoint3dArray& vertexArray) const
{
	assertReadEnabled();
	vertexArray.setLogicalLength(6);
	AcGeVector3d p2p4 = m_ptP4 - m_ptP2;
	vertexArray[0] = (m_ptP1);
	vertexArray[1] = (m_ptP2);
	vertexArray[2] = (m_ptP3);
	vertexArray[3] = (m_ptP1 + p2p4);
	vertexArray[4] = (m_ptP4);
	vertexArray[5] = (m_ptP3 + p2p4);

	return Acad::eOk;
}


Acad::ErrorStatus PDWedge::dwgOutFields(AcDbDwgFiler* filer)const
{
	assertReadEnabled();
	Acad::ErrorStatus es=Acad::eOk;

	// Call dwgOutFields from father class: PDPrimary3D
	if((es=PDPrimary3D::dwgOutFields(filer))!=Acad::eOk)
	{
		return es;
	}

	// Write version number.
	filer->writeItem((Adesk::UInt16) VERSION_PDWEDGE);

	// version 2
	filer->writeItem(m_ptP4);

	// Write the data members.
	filer->writeItem(m_ptP1);
	filer->writeItem(m_ptP2);
	filer->writeItem(m_ptP3);
	//filer->writeItem(m_dHeight);  // version 2 commented

	return filer->filerStatus();
}

Acad::ErrorStatus PDWedge::dwgInFields(AcDbDwgFiler* filer)
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
	if (version > VERSION_PDWEDGE)
		return Acad::eMakeMeProxy;

	if(version >= 2)
		filer->readItem(&m_ptP4);

	// Read the data members.
	if(version >= 1)
	{
		filer->readItem(&m_ptP1);
		filer->readItem(&m_ptP2);
		filer->readItem(&m_ptP3);
		if(version == 1)
		{
			double height;
			filer->readItem(&height);

			AcGeVector3d p2p4, p2p3, p2p1;
			p2p3 = m_ptP3 - m_ptP2;
			p2p1 = m_ptP1 - m_ptP2;
			p2p4 = p2p3.crossProduct(p2p1).normalize() * height;
			m_ptP4 = m_ptP2 + p2p4;
		}
	}

#ifdef _USEAMODELER_
	if(m_3dGeom.isNull())
		createBody();
#endif
	GetMaxLength();
	return filer->filerStatus();
}


Acad::ErrorStatus PDWedge::dxfOutFields(AcDbDxfFiler* filer) const
{
	assertReadEnabled();
	Acad::ErrorStatus es;

	if ((es = PDPrimary3D::dxfOutFields(filer))
		!= Acad::eOk)
	{
		return es;
	}

	// Write subclass marker.
	filer->writeItem(AcDb::kDxfSubclass, _T("PDWedge"));

	// Write version number.
	filer->writeItem(AcDb::kDxfInt16, (Adesk::UInt16) VERSION_PDWEDGE);

	// version 2
	filer->writeItem(AcDb::kDxfXCoord+3, m_ptP4);

	// Write data members.
	filer->writeItem(AcDb::kDxfXCoord, m_ptP1);
	filer->writeItem(AcDb::kDxfXCoord+1, m_ptP2);
	filer->writeItem(AcDb::kDxfXCoord+2, m_ptP3);
	//filer->writeItem(AcDb::kDxfReal,m_dHeight);   // version 2 commented

	return filer->filerStatus();
}

Acad::ErrorStatus PDWedge::dxfInFields(AcDbDxfFiler* filer)
{
	assertWriteEnabled();
	Acad::ErrorStatus es = Acad::eOk;
	struct resbuf rb;

	if ((PDPrimary3D::dxfInFields(filer) != Acad::eOk)
		|| !filer->atSubclassData(_T("PDWedge")))
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
		if (version > VERSION_PDWEDGE)
			return Acad::eMakeMeProxy;
	}

	if(version >= 2)
	{
		filer->readItem(&rb);
		if (rb.restype != AcDb::kDxfXCoord+3) {
			filer->pushBackItem();
			filer->setError(Acad::eInvalidDxfCode,
				_T("nError: expected object version group code %d"),
				AcDb::kDxfXCoord+3);
			return filer->filerStatus();
		} 
		else {
			m_ptP4 = asPnt3d(rb.resval.rpoint);
		}
	}

	// Read the data members.
	if(version >= 1)
	{
		while ((es == Acad::eOk) && ((es = filer->readResBuf(&rb)) == Acad::eOk)){
			switch (rb.restype){
	case AcDb::kDxfXCoord:
		m_ptP1 = asPnt3d(rb.resval.rpoint);
		break;

	case AcDb::kDxfXCoord+1:
		m_ptP2 = asPnt3d(rb.resval.rpoint);
		break;

	case AcDb::kDxfXCoord+2:
		m_ptP3 = asPnt3d(rb.resval.rpoint);
		break;

	case AcDb::kDxfReal:
		if(version == 1)
		{
			double height;
			height = rb.resval.rreal;

			AcGeVector3d p2p4, p2p3, p2p1;
			p2p3 = m_ptP3 - m_ptP2;
			p2p1 = m_ptP1 - m_ptP2;
			p2p4 = p2p3.crossProduct(p2p1).normalize() * height;
			m_ptP4 = m_ptP2 + p2p4;
		}
		break;

	default:
		// An unrecognized group. Push it back so that
		// the subclass can read it again.
		filer->pushBackItem();
		es = Acad::eEndOfFile;
		break;
			}
		}
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
	GetMaxLength();
	return filer->filerStatus();
}

AcGePoint3d PDWedge::getpointP1()
{
	assertReadEnabled();
	return m_ptP1;
}


AcGePoint3d PDWedge::getpointP2()
{
	assertReadEnabled();
	return m_ptP2;
}

AcGePoint3d PDWedge::getpointP3()
{
	assertReadEnabled();
	return m_ptP3;
}

AcGePoint3d PDWedge::getpointP4()
{
	assertReadEnabled();
	return m_ptP4;
}

Acad::ErrorStatus PDWedge::setParameters(AcGePoint3d P1,AcGePoint3d P2,
										 AcGePoint3d P3,double height, int CorrType/* = 1*/)
{
	assertWriteEnabled();
	m_ptP1 = P1;
	m_ptP2 = P2;
	m_ptP3 = P3;
	AcGeVector3d p2p4, p2p3, p2p1;
	p2p3 = P3 - P2;
	p2p1 = P1 - P2;
	if(p2p3.isZeroLength() && p2p1.isZeroLength())
	{
		m_ptP1 = m_ptP2 + AcGeVector3d(0, 1, 0);
		m_ptP3 = m_ptP2 + AcGeVector3d(1, 0, 0);
	}
	else if(p2p3.isZeroLength())
	{
		AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax, Ay;
		if (fabs(p2p1[X]) <1.0/64 && fabs(p2p1[Y])<1.0/64) 
			Ax = Wy.crossProduct(p2p1);
		else
			Ax = Wz.crossProduct(p2p1);
		Ax.normalize();

		p2p3 = Ax;
		m_ptP3 = m_ptP2 + p2p3;
	}
	else if(p2p1.isZeroLength())
	{
		AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax, Ay;
		if (fabs(p2p3[X]) <1.0/64 && fabs(p2p3[Y])<1.0/64) 
			Ax = Wy.crossProduct(p2p3);
		else
			Ax = Wz.crossProduct(p2p3);
		Ax.normalize();

		p2p1 = Ax;
		m_ptP1 = m_ptP2 + p2p1;
	}
	else if(p2p1.isParallelTo(p2p3))
	{
		AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax, Ay;
		if (fabs(p2p3[X]) <1.0/64 && fabs(p2p3[Y])<1.0/64) 
			Ax = Wy.crossProduct(p2p3);
		else
			Ax = Wz.crossProduct(p2p3);
		Ax.normalize();

		p2p1 = Ax * p2p1.length();
		m_ptP1 = m_ptP2 + p2p1;
	}

	if (CorrType) 
		p2p4 = p2p3.crossProduct(p2p1).normalize() * height;
	else
		p2p4=p2p1.crossProduct(p2p3).normalize()*height;  //added by linlin 20050908  

	m_ptP4 = m_ptP2 + p2p4;

#ifdef _USEAMODELER_
	createBody();
#endif
	GetMaxLength();
	return Acad::eOk;
}

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDWedge::subGetOsnapPoints(
	AcDb::OsnapMode     osnapMode,
	Adesk::GsMarker     gsSelectionMark,
	const AcGePoint3d&  pickPoint,
	const AcGePoint3d&  lastPoint,
	const AcGeMatrix3d& viewXform,
	AcGePoint3dArray&   snapPoints,
	AcDbIntArray &   geomIds) const
#else
Acad::ErrorStatus PDWedge::getOsnapPoints(
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

	if(verArr.length() != 6)
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
			int i;
			for(i = 0; i < verArr.length() / 2 - 1; i++)
			{
				pt = verArr[i] + (verArr[i + 1] - verArr[i]) / 2.0;
				snapPoints.append(pt);
			}
			pt = verArr[verArr.length() / 2 - 1] + (verArr[0] - verArr[verArr.length() / 2 - 1]) / 2.0;
			snapPoints.append(pt);
			for(i = verArr.length() / 2; i < verArr.length() - 1; i++)
			{
				pt = verArr[i] + (verArr[i + 1] - verArr[i]) / 2.0;
				snapPoints.append(pt);
			}
			pt = verArr[verArr.length() - 1] + (verArr[verArr.length() / 2] - verArr[verArr.length() - 1]) / 2.0;
			snapPoints.append(pt);
			for(i = 0; i < verArr.length() / 2; i++)
			{
				pt = verArr[i] + (verArr[i + verArr.length() / 2] - verArr[i]) / 2.0;
				snapPoints.append(pt);
			}
		}
		break;
	case AcDb::kOsModeCen:
		{
			AcGePoint3d cenpt1, cenpt2;
			AcGePoint3d pt23MidPt = verArr[1] + (verArr[2] - verArr[1]) / 2.0;
			cenpt1 = verArr[0] + (pt23MidPt - verArr[0]) / 3.0 * 2.0;
			AcGePoint3d pt56MidPt = verArr[4] + (verArr[5] - verArr[4]) / 2.0;
			cenpt2 = verArr[3] + (pt56MidPt - verArr[3]) / 3.0 * 2.0;
			//重新定义对象中心点的捕捉方式，同时满足实体模型和线框模型的捕捉 
			//modified by szw 2009.11.18 : begin
			if(gsSelectionMark_int == 1)
				snapPoints.append(cenpt1);
			else if(gsSelectionMark_int == 2)
				snapPoints.append(cenpt2);
			else
			{
				AcGePoint3d pt;
				for(int i = 0; i < verArr.length() / 2 - 1; ++i)
				{
					pt = verArr[i] + (verArr[i + verArr.length() / 2 + 1] - verArr[i]) / 2.0;
					snapPoints.append(pt);
				}
				pt = verArr[0] + (verArr[verArr.length() - 1] - verArr[0]) / 2.0;
				snapPoints.append(pt);

			}
			snapPoints.append(cenpt1 + (cenpt2 - cenpt1) / 2.0);
			//modified by szw 2009.11.18 : end
		}
		break;
	case AcDb::kOsModeQuad:
		break;
	case AcDb::kOsModeNode:
		break;
	case AcDb::kOsModeIns:
		snapPoints.append(m_ptP1);
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
				for(int i = 0; i < verArr.length() / 2 - 1; ++i)
				{
					vec = verArr[i + 1] - verArr[i];
					vec.normalize();
					line.set(verArr[i], vec);
					pt = line.closestPointTo(lastPoint);
					snapPoints.append(pt);
				}
				vec = verArr[0] - verArr[verArr.length() / 2 - 1];
				vec.normalize();
				line.set(verArr[0], vec);
				pt = line.closestPointTo(lastPoint);
				snapPoints.append(pt);
			}
			else if(gsSelectionMark_int == 2)
			{
				for(int i = verArr.length() / 2; i < verArr.length() - 1; ++i)
				{
					vec = verArr[i + 1] - verArr[i];
					vec.normalize();
					line.set(verArr[i], vec);
					pt = line.closestPointTo(lastPoint);
					snapPoints.append(pt);
				}
				vec = verArr[verArr.length() / 2] - verArr[verArr.length() - 1];
				vec.normalize();
				line.set(verArr[verArr.length() / 2], vec);
				pt = line.closestPointTo(lastPoint);
				snapPoints.append(pt);
			}
			else
			{
				for(int i = 0; i < verArr.length() / 2; ++i)
				{
					vec = verArr[i + verArr.length() / 2] - verArr[i];
					vec.normalize();
					line.set(verArr[i], vec);
					pt = line.closestPointTo(lastPoint);
					snapPoints.append(pt);
				}
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
Acad::ErrorStatus PDWedge::subGetGripPoints(
	AcGePoint3dArray& gripPoints,
	AcDbIntArray& osnapModes,
	AcDbIntArray& geomIds) const
#else
Acad::ErrorStatus PDWedge::getGripPoints(
	AcGePoint3dArray& gripPoints,
	AcDbIntArray& osnapModes,
	AcDbIntArray& geomIds) const
#endif
{
	assertReadEnabled();
	//	getVertices(gripPoints);
	return Acad::eOk; 
}

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDWedge::subExplode(AcDbVoidPtrArray& entitySet) const
#else
Acad::ErrorStatus PDWedge::explode(AcDbVoidPtrArray& entitySet) const
#endif
{
	assertReadEnabled();

	AcDb3dSolid *pBody;
	pBody = new AcDb3dSolid;
	if(pBody->createWedge(m_ptP2.distanceTo(m_ptP3), 
		m_ptP2.distanceTo(m_ptP4), 
		m_ptP2.distanceTo(m_ptP1)) != Acad::eOk)
	{
		delete pBody;
		return Acad::eNotApplicable;
	}

	AcGeMatrix3d mat;
	AcGeVector3d x, y, z;
	x = (m_ptP3 - m_ptP2).normal();
	z = (m_ptP1 - m_ptP2).normal();
	y = z.crossProduct(x);
	mat.setCoordSystem(m_ptP1 + (m_ptP3 - m_ptP1) / 2.0 + (m_ptP4 - m_ptP2) / 2.0, 
		x, y, z);

	pBody->transformBy(mat);
	pBody->setPropertiesFrom(this);
	entitySet.append(pBody);

	return Acad::eOk;
}

//  [9/5/2007 suzhiyong]
Acad::ErrorStatus PDWedge::explodeTo3DSolid(AcDb3dSolid* &p3dSolid) const
{
	assertReadEnabled();

	AcDb3dSolid *pBody;
	pBody = new AcDb3dSolid;
	if(pBody->createWedge(m_ptP2.distanceTo(m_ptP3), 
		m_ptP2.distanceTo(m_ptP4), 
		m_ptP2.distanceTo(m_ptP1)) != Acad::eOk)
	{
		delete pBody;
		return Acad::eNotApplicable;
	}

	AcGeMatrix3d mat;
	AcGeVector3d x, y, z;
	x = (m_ptP3 - m_ptP2).normal();
	z = (m_ptP1 - m_ptP2).normal();
	y = z.crossProduct(x);
	mat.setCoordSystem(m_ptP1 + (m_ptP3 - m_ptP1) / 2.0 + (m_ptP4 - m_ptP2) / 2.0, 
		x, y, z);

	pBody->transformBy(mat);
	pBody->setPropertiesFrom(this);
	p3dSolid = pBody;

	return Acad::eOk;
}

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDWedge::subGetGeomExtents(AcDbExtents& extents) const
#else
Acad::ErrorStatus PDWedge::getGeomExtents(AcDbExtents& extents) const
#endif
{
	assertReadEnabled();
	extents.set(m_ptP1, m_ptP1);
	AcGePoint3dArray pArray;
	getVertices(pArray);
	int i, arrlen = pArray.length();
	for(i = 0; i < arrlen; ++i)
		extents.addPoint(pArray[i]);
	return Acad::eOk;
}

#ifdef _OBJECTARX2010_
void PDWedge::subList() const
#else
void PDWedge::list() const
#endif
{
	assertReadEnabled();
#ifdef _OBJECTARX2010_
	PDPrimary3D::subList();
#else
	PDPrimary3D::list();
#endif
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4832/*"\n 底面三角形的顶点P1(%f,%f,%f)\n"*/,m_ptP1.x,m_ptP1.y,m_ptP1.z);
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4833/*" 底面三角形的直角点P2(%f,%f,%f)\n"*/,m_ptP2.x,m_ptP2.y,m_ptP2.z);
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4834/*" 底面三角形的顶点P3(%f,%f,%f)\n"*/,m_ptP3.x,m_ptP3.y,m_ptP3.z);
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4835/*" 顶面三角形的直角点P4(%f,%f,%f)\n\n"*/,m_ptP4.x,m_ptP4.y,m_ptP4.z);   
	return ;
}// added by linlin 20050811


void PDWedge::createBody()
{
	assertWriteEnabled();
	// #ifdef _OBJECTARX2004_

	AcGePoint3dArray pArray;
	getVertices( pArray);
	int n =pArray.length();
	AcGePoint3d* vertices;
	vertices = new AcGePoint3d[n];
	for( int i =0; i<n; ++i){
		vertices[i] = pArray[i];
	}

	m_3dGeom = Body::convexHull((Point3d*)vertices, n);

	delete [] vertices;

	// #endif
}
double PDWedge::GetMaxLength()
{
	m_dMaxLength = 0;
	double dDist = 0;
	dDist = m_ptP1.distanceTo(m_ptP2);
	if(dDist > m_dMaxLength)
		m_dMaxLength = dDist;
	dDist = m_ptP1.distanceTo(m_ptP3);
	if(dDist > m_dMaxLength)
		m_dMaxLength = dDist;
	dDist = m_ptP3.distanceTo(m_ptP2);
	if(dDist > m_dMaxLength)
		m_dMaxLength = dDist;
	dDist = m_ptP4.distanceTo(m_ptP2);
	if(dDist > m_dMaxLength)
		m_dMaxLength = dDist;
	return m_dMaxLength;
}