// PDSqucone.cpp: implementation of the PDSqucone class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "language25.h"
#include "PDSqucone.h"

#include <math.h>

#include <dbproxy.h>
#include <acgi.h>
#include <geassign.h>
#include <dbents.h>
#include <dbregion.h>
#include <dbsol3d.h>
#include "dbapserv.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define VERSION_PDSQUCONE 1

#ifdef _OBJECTARX2000_
ACRX_DXF_DEFINE_MEMBERS(PDSqucone, PDPrimary3D,
						AcDb::kDHL_CURRENT, AcDb::kMReleaseCurrent,
						AcDbProxyEntity::kAllAllowedBits,
						PDSQUCONE, Gle);
#else
ACRX_DXF_DEFINE_MEMBERS(PDSqucone, PDPrimary3D, AcDbProxyEntity::kAllAllowedBits, PDSQUCONE, Gle);
#endif

void PDSqucone::setDefault()
{
	PDPrimary3D::setDefault();
#if defined(NDEBUG) || !defined(_DEBUG)
	m_dheight = m_dlength1 = m_dlength2 = m_dwidth1 = m_dwidth2 = m_dlean = 1.0;
	m_VectH .set(0,0,1);
	m_VectL .set(1,0,0);
	m_VectV .set(0,0,0);
	m_ptorign = AcGePoint3d();
#else
	m_dheight = m_dlength1 = m_dlength2 = m_dwidth1 = m_dwidth2 = m_dlean = 1000;
	m_VectH .set(0,0,1);
	m_VectL .set(1,0,0);
	m_VectV .set(0,0,0);
	m_ptorign.set(0,0,0);
#endif
}
bool PDSqucone::isValidData(double &f)
{
	if(!PDPrimary3D::isValidData(f))
		return false;
	if(!isValidFloat(f=m_dheight))
		return false;
	if(!isValidFloat(f=m_dlength1))
		return false;
	if(!isValidFloat(f=m_dlength2))
		return false;
	if(!isValidFloat(f=m_dwidth1))
		return false;
	if(!isValidFloat(f=m_dwidth2))
		return false;
	if(!isValidFloat(f=m_dlean))
		return false;
	if(!isValidFloat(f=m_VectH.x) || !isValidFloat(f=m_VectH.y) || !isValidFloat(f=m_VectH.z))
		return false;
	if(!isValidFloat(f=m_VectL.x) || !isValidFloat(f=m_VectL.y) || !isValidFloat(f=m_VectL.z))
		return false;
	if(!isValidFloat(f=m_VectV.x) || !isValidFloat(f=m_VectV.y) || !isValidFloat(f=m_VectV.z))
		return false;
	if(!isValidFloat(f=m_ptorign.x) || !isValidFloat(f=m_ptorign.y) || !isValidFloat(f=m_ptorign.z))
		return false;
	return true;
}

//zxb,20090113,增加audit，暂时只check自身成员数据
Acad::ErrorStatus PDSqucone::audit(AcDbAuditInfo* pAuditInfo)
{
	return auditPDPrimary3D(this,pAuditInfo,_T("PDSqucone"));
}

PDSqucone::PDSqucone(bool HasSnap/* = false*/) : PDPrimary3D(HasSnap)
{
	m_dheight = m_dlength1 = m_dlength2 = m_dwidth1 = m_dwidth2 = m_dlean = 1.0;
	m_VectH .set(0,0,1);
	m_VectL .set(1,0,0);
	m_VectV .set(0,0,0);
	m_ptorign = AcGePoint3d();
#ifdef _USEAMODELER_
	createBody();
#endif
	GetMaxLength();
}

PDSqucone::~PDSqucone()
{

}

PDSqucone::PDSqucone( double l1,double w1,double l2,double w2,
					 double h,AcGePoint3d p,
					 AcGeVector3d vh,AcGeVector3d vl,
					 AcGeVector3d v,double m, bool HasSnap/* = false*/) : PDPrimary3D(HasSnap)
{
	m_dheight =fabs(h) ;
	if(m_dheight==0)
		m_dheight=1;
	m_dlean =fabs(m) ;
	m_dlength1 =fabs(l1) ;
	if(m_dlength1==0)
		m_dlength1=1;
	m_dlength2 =fabs(l2) ;
	if(m_dlength2==0)
		m_dlength2=1;
	m_dwidth1 =fabs(w1) ;
	if(m_dwidth1==0)
		m_dwidth1=1;	
	m_dwidth2 =fabs(w2) ;
	if(m_dwidth2==0)
		m_dwidth2=1;

	m_ptorign = p;

	m_VectH = vh.normal();
	m_VectL = vl.normal();
	m_VectV = v.normal();

	if (vh.isZeroLength()&& vl.isZeroLength())
	{
		m_VectH.set(0,0,1);
		m_VectL.set(1,0,0);
	}
	else
		if(vh.isZeroLength())
		{
			AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax, Ay;
			if (fabs(vl[X]) <1.0/64 && fabs(vl[Y])<1.0/64) 
				Ax = Wy.crossProduct(vl);
			else
				Ax = Wz.crossProduct(vl);
			Ax.normalize();
			m_VectH=Ax;
		}
		else
			if(vl.isZeroLength())
			{
				AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax, Ay;
				if (fabs(vh[X]) <1.0/64 && fabs(vh[Y])<1.0/64) 
					Ax = Wy.crossProduct(vh);
				else
					Ax = Wz.crossProduct(vh);
				Ax.normalize();
				m_VectL=Ax;
			} 
			else
				if(vh.isParallelTo(vl))
				{
					AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax, Ay;
					if (fabs(vh[X]) <1.0/64 && fabs(vh[Y])<1.0/64) 
						Ax = Wy.crossProduct(vh);
					else
						Ax = Wz.crossProduct(vh);
					Ax.normalize();
					m_VectL=Ax;
				}

				m_VectV = m_VectV.orthoProject(m_VectH).normal();
#ifdef _USEAMODELER_
				createBody();
#endif
				GetMaxLength();
}

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDSqucone::subTransformBy(const AcGeMatrix3d& xform)
#else
Acad::ErrorStatus PDSqucone::transformBy(const AcGeMatrix3d& xform)
#endif
{
	assertWriteEnabled();
	// TODO: implement this function.
	m_ptorign.transformBy(xform);
	m_VectH.transformBy(xform).normalize();
	m_VectL.transformBy(xform).normalize();
	m_VectV.transformBy(xform).normalize();

	m_dheight *= xform.scale();
	m_dlean *= xform.scale();
	m_dlength1 *= xform.scale();
	m_dlength2 *= xform.scale();
	m_dwidth1 *= xform.scale();
	m_dwidth2 *= xform.scale();
	GetMaxLength();

#ifdef _OBJECTARX2010_
	return PDPrimary3D::subTransformBy(xform);
#else
	return PDPrimary3D::transformBy(xform);
#endif
}

#ifdef _OBJECTARX2010_
Adesk::Boolean PDSqucone::subWorldDraw(AcGiWorldDraw* mode)
#else
Adesk::Boolean PDSqucone::worldDraw(AcGiWorldDraw* mode)
#endif
{
	assertReadEnabled();
	//if(acdbHostApplicationServices()->workingDatabase()->useri1() == 1)
	// return Adesk::kTrue;

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
	int Precision = m_dDividPrecision;
	AcGePoint3dArray pArray;
	int halfLen = 0;
	Adesk::Int16 nRating = getCalMaxPrecision();
	if(nRating == PRIMARY3D_SIMPLIZE_RATE1 ||
		nRating == PRIMARY3D_SIMPLIZE_RATE2)
	{
		Precision = (int)(PI / acos(1.0 - mode->deviation(kAcGiMaxDevForCircle, m_ptorign)
			* 2.0 / m_dMaxLength));
		if(Precision < PRIMARY3D_SIMPLIZE_SCALE)
		{	
			if (nRating == PRIMARY3D_SIMPLIZE_RATE2)
			{
				AcGePoint3d pts[2];
				pts[0] = m_ptorign;
				pts[1] =pts[0] + m_VectH * m_dheight + m_VectV * m_dlean;
				mode->geometry().polyline(2,pts);	  
			}
			else if (nRating == PRIMARY3D_SIMPLIZE_RATE1)
			{
				getVertices(pArray);
				halfLen = pArray.length() / 2; 
				AcGePoint3d pt[2];
				int i;
				for(i = 0; i < halfLen - 1; i++)
				{
					pt[0] = pArray[i];
					pt[1] = pArray[i + 1];
					if(mode->geometry().polyline(2, pt))
						return Adesk::kTrue;
				}
				pt[0] = pArray[halfLen - 1];
				pt[1] = pArray[0];
				if(mode->geometry().polyline(2, pt))
					return Adesk::kTrue;
				for(i = halfLen; i < pArray.length() - 1; i++)
				{
					pt[0] = pArray[i];
					pt[1] = pArray[i + 1];
					if(mode->geometry().polyline(2, pt))
						return Adesk::kTrue;
				}
				pt[0] = pArray[halfLen *2- 1];
				pt[1] = pArray[halfLen];
				if(mode->geometry().polyline(2, pt))
					return Adesk::kTrue;
				for(i = 0; i < halfLen; i++)
				{
					pt[0] = pArray[i];
					pt[1] = pArray[i + halfLen];
					if(mode->geometry().polyline(2, pt))
						return Adesk::kTrue;
				}
			}
			return Adesk::kTrue;
		}
	}
	getVertices(pArray);
	halfLen = pArray.length() / 2; 
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
			pt[0] = pArray[halfLen *2- 1];
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

/*Adesk::Boolean PDSqucone::worldDraw(AcGiWorldDraw* mode)
{
assertReadEnabled();
if (mode->regenAbort()) {
return Adesk::kTrue;
}
#ifdef _USEAMODELER_
PDPrimary3D::worldDraw(mode);
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
/////////////////////////////////////////////////////////////
/////		求出上下矩形的8个顶点，加到输出数组中	/////////
/////////////////////////////////////////////////////////////
Acad::ErrorStatus PDSqucone::getVertices(AcGePoint3dArray& vertexArray) const
{
	assertReadEnabled();

	/*while(!vertexArray.isEmpty())
	vertexArray.removeLast();*/
	vertexArray.setLogicalLength(8);

	AcGePoint3d point ;
	AcGeVector3d vectw = m_VectH.crossProduct(m_VectL).normal();
	AcGeVector3d vectl = vectw.crossProduct(m_VectH).normal();

	/////////////////////////////////////////////////////////
	/////	resolve the first point						/////
	AcGeVector3d vectAlongW = vectw * (m_dwidth1);
	AcGeVector3d vectAlongL = vectl * (m_dlength1);
	point = m_ptorign - vectAlongW / 2.0 - vectAlongL / 2.0;
	vertexArray[0] = (point);
	/////////////////////////////////////////////////////////
	/////	resolve the other point						/////
	point += vectAlongL;
	vertexArray[1] = (point);
	point += vectAlongW;
	vertexArray[2] = (point);
	point -= vectAlongL;
	vertexArray[3] = (point);

	//////////////////////////////////////////////////////////
	//////	resolve the top face						//////
	AcGePoint3d p2 = m_ptorign + m_VectH * m_dheight + m_VectV * m_dlean;

	/////////////////////////////////////////////////////////
	/////	resolve the first point						/////
	vectAlongL = vectl * (m_dlength2);
	vectAlongW = vectw * (m_dwidth2);
	point = p2 - vectAlongL/2.0 - vectAlongW / 2.0;
	vertexArray[4] = (point);
	/////////////////////////////////////////////////////////
	/////	resolve the other point						/////
	point += vectAlongL;
	vertexArray[5] = (point);
	point += vectAlongW;
	vertexArray[6]= (point);
	point -= vectAlongL;
	vertexArray[7] = (point);

	return Acad::eOk;
}

//当矩形断截面底部位于XY平面，方向朝Z正向时各点的坐标求
//P为原点，L方向为X正向，W方向为Y正向，H方向为Z正向
Acad::ErrorStatus PDSqucone::getVertices_XY(AcGePoint3dArray& vertexArray) const
{
	assertReadEnabled();

	vertexArray.setLogicalLength(8);

	AcGePoint3d ptorign(0,0,0);
	AcGePoint3d point(0,0,0);
	AcGeVector3d vectY(0,1,0);
	AcGeVector3d vectX(1,0,0);
	AcGeVector3d vectZ(0,0,1);

	AcGeVector3d vectAlongW = vectY * (m_dwidth1);
	AcGeVector3d vectAlongL = vectX * (m_dlength1);
	point = ptorign - vectAlongW / 2.0 - vectAlongL / 2.0;

	//bottom face
	vertexArray[0] = point;
	point += vectAlongL;
	vertexArray[1] = (point);
	point += vectAlongW;
	vertexArray[2] = (point);
	point -= vectAlongL;
	vertexArray[3] = (point);

	//top face
	//顶面的中心点
	AcGeVector3d vectw = m_VectH.crossProduct(m_VectL).normal();
	AcGeVector3d vectl = vectw.crossProduct(m_VectH).normal();
	AcGeMatrix3d mat;
	mat.setToAlignCoordSys(m_ptorign, vectl, vectw, m_VectH,
		ptorign, vectX, vectY, vectZ);

	AcGeVector3d VectV = m_VectV;
	VectV.transformBy(mat);
	AcGePoint3d p2 = ptorign + vectZ * m_dheight + VectV * m_dlean;//变换后顶面的点

	vectAlongL = vectX * (m_dlength2);
	vectAlongW = vectY * (m_dwidth2);
	point = p2 - vectAlongL/2.0 - vectAlongW / 2.0;
	vertexArray[4] = point;
	point += vectAlongL;
	vertexArray[5] = (point);
	point += vectAlongW;
	vertexArray[6]= (point);
	point -= vectAlongL;
	vertexArray[7] = (point);

	return Acad::eOk;
}

Acad::ErrorStatus PDSqucone::dwgOutFields(AcDbDwgFiler* filer)const
{
	assertReadEnabled();
	Acad::ErrorStatus es=Acad::eOk;

	// Call dwgOutFields from father class: PDPrimary3D
	if((es=PDPrimary3D::dwgOutFields(filer))!=Acad::eOk)
	{
		return es;
	}

	// Write version number.
	filer->writeItem((Adesk::UInt16) VERSION_PDSQUCONE);

	// Write the data members.
	filer->writeItem(m_ptorign);
	filer->writeItem(m_dlength1);
	filer->writeItem(m_dlength2);
	filer->writeItem(m_dwidth1);
	filer->writeItem(m_dwidth2);
	filer->writeItem(m_dheight);
	filer->writeItem(m_dlean);
	filer->writeItem(m_VectH);
	filer->writeItem(m_VectL);
	filer->writeItem(m_VectV);

	return filer->filerStatus();
}

Acad::ErrorStatus PDSqucone::dwgInFields(AcDbDwgFiler* filer)
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
	if (version > VERSION_PDSQUCONE)
		return Acad::eMakeMeProxy;

	// Read the data members.
	switch (version)
	{
	case (1):

		filer->readItem(&m_ptorign);
		filer->readItem(&m_dlength1);
		filer->readItem(&m_dlength2);
		filer->readItem(&m_dwidth1);
		filer->readItem(&m_dwidth2);
		filer->readItem(&m_dheight);
		filer->readItem(&m_dlean);
		filer->readItem(&m_VectH);
		m_VectH.normalize();
		filer->readItem(&m_VectL);
		m_VectL.normalize();
		filer->readItem(&m_VectV);
		m_VectV.normalize();
		break;
	}

#ifdef _USEAMODELER_
	if(m_3dGeom.isNull())
		createBody();
#endif
	GetMaxLength();
	return filer->filerStatus();
}

Acad::ErrorStatus PDSqucone::dxfOutFields(AcDbDxfFiler* filer) const
{
	assertReadEnabled();
	Acad::ErrorStatus es;

	if ((es = PDPrimary3D::dxfOutFields(filer))
		!= Acad::eOk)
	{
		return es;
	}

	// Write subclass marker.
	filer->writeItem(AcDb::kDxfSubclass, _T("PDSqucone"));

	// Write version number.
	filer->writeItem(AcDb::kDxfInt16, (Adesk::UInt16) VERSION_PDSQUCONE);

	// Write data members.
	filer->writeItem(AcDb::kDxfXCoord, m_ptorign);
	filer->writeItem(AcDb::kDxfReal,m_dlength1);
	filer->writeItem(AcDb::kDxfReal+1,m_dlength2);
	filer->writeItem(AcDb::kDxfReal+2,m_dwidth1);
	filer->writeItem(AcDb::kDxfReal+3,m_dwidth2);
	filer->writeItem(AcDb::kDxfReal+4,m_dheight);
	filer->writeItem(AcDb::kDxfReal+5,m_dlean);
	filer->writeItem(AcDb::kDxfNormalX,m_VectH);
	filer->writeItem(AcDb::kDxfNormalX+1,m_VectL);
	filer->writeItem(AcDb::kDxfNormalX+2,m_VectV);

	return filer->filerStatus();
}

Acad::ErrorStatus PDSqucone::dxfInFields(AcDbDxfFiler* filer)
{
	assertWriteEnabled();
	Acad::ErrorStatus es = Acad::eOk;
	struct resbuf rb;

	if ((PDPrimary3D::dxfInFields(filer) != Acad::eOk)
		|| !filer->atSubclassData(_T("PDSqucone")))
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
		if (version > VERSION_PDSQUCONE)
			return Acad::eMakeMeProxy;
	}

	// Read the data members.
	switch (version){
	case (1):
		while ((es == Acad::eOk) && ((es = filer->readResBuf(&rb)) == Acad::eOk)){
			switch (rb.restype){
	case AcDb::kDxfXCoord:
		m_ptorign = asPnt3d(rb.resval.rpoint);
		break;

	case AcDb::kDxfReal:
		m_dlength1 = rb.resval.rreal;
		break;

	case AcDb::kDxfReal+1:
		m_dlength2 = rb.resval.rreal;
		break;

	case AcDb::kDxfReal+2:
		m_dwidth1 = rb.resval.rreal;
		break;

	case AcDb::kDxfReal+3:
		m_dwidth2 = rb.resval.rreal;
		break;

	case AcDb::kDxfReal+4:
		m_dheight = rb.resval.rreal;
		break;

	case AcDb::kDxfReal+5:
		m_dlean = rb.resval.rreal;
		break;

	case AcDb::kDxfNormalX:
		m_VectH = asVec3d(rb.resval.rpoint);
		m_VectH.normalize();
		break;

	case AcDb::kDxfNormalX+1:
		m_VectL = asVec3d(rb.resval.rpoint);
		m_VectL.normalize();
		break;

	case AcDb::kDxfNormalX+2:
		m_VectV = asVec3d(rb.resval.rpoint);
		m_VectV.normalize();
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
	GetMaxLength();
	return filer->filerStatus();
}

Acad::ErrorStatus PDSqucone::getpointOrign(AcGePoint3d& ptorign)
{
	assertReadEnabled();
	ptorign = m_ptorign;
	return Acad::eOk;
}

AcGePoint3d PDSqucone::getUpOrignPoint()const
{
	assertReadEnabled();
	return m_ptorign + m_VectH * m_dheight + m_VectV * m_dlean;
}

double PDSqucone::getOffsetAlongLength()const
{
	assertReadEnabled();
	return m_dlean * fabs(cos(m_VectV.angleTo(m_VectL))); //!< cos 在第二象限为负
}

double PDSqucone::getOffsetAlongWidth() const
{
	assertReadEnabled();
	return m_dlean * sin(m_VectV.angleTo(m_VectL)); //!< sin在第一、二象限都为正 
}

Acad::ErrorStatus PDSqucone::getLength1(double& length1)
{
	assertReadEnabled();
	length1 = m_dlength1;
	return Acad::eOk;
}

Acad::ErrorStatus PDSqucone::getLength2(double& length2)
{
	assertReadEnabled();
	length2 = m_dlength2;
	return Acad::eOk;
}

Acad::ErrorStatus PDSqucone::getWidth1(double& width1)
{
	assertReadEnabled();
	width1 = m_dwidth1;
	return Acad::eOk;
}

Acad::ErrorStatus PDSqucone::getWidth2(double& width2)
{
	assertReadEnabled();
	width2 = m_dwidth2;
	return Acad::eOk;
}

Acad::ErrorStatus PDSqucone::getHeight(double& height)
{
	assertReadEnabled();
	height = m_dheight;
	return Acad::eOk;
}

Acad::ErrorStatus PDSqucone::getLean(double& lean)
{
	assertReadEnabled();
	lean = m_dlean;
	return Acad::eOk;
}

Acad::ErrorStatus PDSqucone::getVectH(AcGeVector3d& vectH)
{
	assertReadEnabled();
	vectH = m_VectH;
	return Acad::eOk;
}

Acad::ErrorStatus PDSqucone::getVectL(AcGeVector3d& vectL)
{
	assertReadEnabled();
	vectL = m_VectL;
	return Acad::eOk;
}

Acad::ErrorStatus PDSqucone::getVectV(AcGeVector3d& vectV)
{
	assertReadEnabled();
	vectV = m_VectV;
	return Acad::eOk;
}

Acad::ErrorStatus PDSqucone:: setParameters(AcGePoint3d ptorign,double length1,double length2,
											double width1,double width2,double height,
											double lean,AcGeVector3d vectH,AcGeVector3d vectL,
											AcGeVector3d vectV)
{	
	assertWriteEnabled();
	m_ptorign = ptorign ;
	m_dlength1 =fabs(length1)  ;
	if(m_dlength1==0)
		m_dlength1=1;
	m_dlength2 =fabs(length2) ;
	if(m_dlength2==0)
		m_dlength2=1;
	m_dwidth1 =fabs(width1)  ;
	if(m_dwidth1==0)
		m_dwidth1=1;
	m_dwidth2 =fabs(width2)  ;
	if(m_dwidth2==0)
		m_dwidth2=1;
	m_dheight =fabs(height)  ;
	if(m_dheight==0)
		m_dheight=1;
	m_dlean =fabs(lean)  ;

	m_VectH = vectH.normal();
	m_VectL = vectL.normal();
	m_VectV = vectV.normal();

	if (vectH.isZeroLength()&& vectL.isZeroLength())
	{
		m_VectH.set(0,0,1);
		m_VectL.set(1,0,0);
	}
	else
		if(vectH.isZeroLength())
		{
			AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax, Ay;
			if (fabs(vectL[X]) <1.0/64 && fabs(vectL[Y])<1.0/64) 
				Ax = Wy.crossProduct(vectL);
			else
				Ax = Wz.crossProduct(vectL);
			Ax.normalize();
			m_VectH=Ax;
		}
		else
			if(vectL.isZeroLength())
			{
				AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax, Ay;
				if (fabs(vectH[X]) <1.0/64 && fabs(vectH[Y])<1.0/64) 
					Ax = Wy.crossProduct(vectH);
				else
					Ax = Wz.crossProduct(vectH);
				Ax.normalize();
				m_VectL=Ax;
			} 
			else
				if(vectH.isParallelTo(vectL))
				{
					AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax, Ay;
					if (fabs(vectH[X]) <1.0/64 && fabs(vectH[Y])<1.0/64) 
						Ax = Wy.crossProduct(vectH);
					else
						Ax = Wz.crossProduct(vectH);
					Ax.normalize();
					m_VectL=Ax;
				}

#ifdef _USEAMODELER_
				createBody();
#endif
				GetMaxLength();		
				return Acad::eOk;
}//added by linlin 20052909

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDSqucone::subGetOsnapPoints(
	AcDb::OsnapMode     osnapMode,
	Adesk::GsMarker     gsSelectionMark,
	const AcGePoint3d&  pickPoint,
	const AcGePoint3d&  lastPoint,
	const AcGeMatrix3d& viewXform,
	AcGePoint3dArray&   snapPoints,
	AcDbIntArray &   geomIds) const
#else
Acad::ErrorStatus PDSqucone::getOsnapPoints(
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
			//重新定义对象中心点的捕捉方式，同时满足实体模型和线框模型的捕捉 
			//modified by szw 2009.11.18 : begin
			AcGePoint3d pt1, pt2;
			if(gsSelectionMark_int == 1)//底面
				snapPoints.append(m_ptorign);
			else if(gsSelectionMark_int == 2)//顶面
				snapPoints.append(m_ptorign + m_VectV * m_dlean + m_VectH * m_dheight);
			else if(gsSelectionMark_int < verArr.length() / 2 + 2)//3个侧面
			{
				pt1 = verArr[gsSelectionMark_int - 3] + 
					(verArr[gsSelectionMark_int - 2] - 
					verArr[gsSelectionMark_int - 3]) / 2.0;
				pt2 = verArr[gsSelectionMark_int - 3 + verArr.length() / 2] + 
					(verArr[gsSelectionMark_int - 2 + verArr.length() / 2] - 
					verArr[gsSelectionMark_int - 3 + verArr.length() / 2]) / 2.0;

				snapPoints.append(pt1 + (pt2 - pt1) / 2.0);
			}
			else if(gsSelectionMark_int == verArr.length() / 2 + 2)//最后1个侧面
			{
				pt1 = verArr[0] + (verArr[verArr.length() / 2 - 1] - verArr[0]) / 2.0;
				pt2 = verArr[verArr.length() / 2] + 
					(verArr[verArr.length() - 1] - verArr[verArr.length() / 2]) / 2.0;
				snapPoints.append(pt1 + (pt2 - pt1) / 2.0);
			}
			snapPoints.append(m_ptorign + m_VectV * m_dlean / 2.0 + 
				m_VectH * m_dheight / 2.0);
			//modified by szw 2009.11.18 : end
		}
		break;
	case AcDb::kOsModeQuad:
		break;
	case AcDb::kOsModeNode:
		break;
	case AcDb::kOsModeIns:
		snapPoints.append(m_ptorign);
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
			if(gsSelectionMark_int == 1)//底面
			{
				for(int i = 0; i < 3; ++i)
				{
					vec = verArr[i + 1] - verArr[i];
					vec.normalize();
					line.set(verArr[i], vec);
					pt = line.closestPointTo(lastPoint);
					snapPoints.append(pt);
				}
				vec = verArr[0] - verArr[3];
				vec.normalize();
				line.set(verArr[3], vec);
				pt = line.closestPointTo(lastPoint);
				snapPoints.append(pt);
			}
			else if(gsSelectionMark_int == 2)//顶面
			{
				for(int i = 0; i < 3; ++i)
				{
					vec = verArr[i + 1 + verArr.length() / 2] - verArr[i + verArr.length() / 2];
					vec.normalize();
					line.set(verArr[i + verArr.length() / 2], vec);
					pt = line.closestPointTo(lastPoint);
					snapPoints.append(pt);
				}
				vec = verArr[verArr.length() / 2] - verArr[3 + verArr.length() / 2];
				vec.normalize();
				line.set(verArr[3 + verArr.length() / 2], vec);
				pt = line.closestPointTo(lastPoint);
				snapPoints.append(pt);
			}
			else//侧面
			{
				vec = verArr[gsSelectionMark_int - 3 + verArr.length() / 2]
				- verArr[gsSelectionMark_int - 3];
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
			int i;
			for(i = 0; i < verArr.length() / 2 - 1; i++)
			{
				lnsg.set(verArr[i], verArr[i + 1]);
				pt = lnsg.projClosestPointTo(pickPoint, viewDir);
				snapPoints.append(pt);
			}
			lnsg.set(verArr[verArr.length() / 2 - 1], verArr[0]);
			pt = lnsg.projClosestPointTo(pickPoint, viewDir);
			snapPoints.append(pt);
			for(i = verArr.length() / 2; i < verArr.length() - 1; i++)
			{
				lnsg.set(verArr[i], verArr[i + 1]);
				pt = lnsg.projClosestPointTo(pickPoint, viewDir);
				snapPoints.append(pt);
			}
			lnsg.set(verArr[verArr.length() - 1], verArr[verArr.length() / 2]);
			pt = lnsg.projClosestPointTo(pickPoint, viewDir);
			snapPoints.append(pt);
			for(i = 0; i < verArr.length() / 2; i++)
			{
				lnsg.set(verArr[i], verArr[i + verArr.length() / 2]);
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
Acad::ErrorStatus PDSqucone::subGetGripPoints(
	AcGePoint3dArray& gripPoints,
	AcDbIntArray& osnapModes,
	AcDbIntArray& geomIds) const
#else
Acad::ErrorStatus PDSqucone::getGripPoints(
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
Acad::ErrorStatus PDSqucone::subExplode(AcDbVoidPtrArray& entitySet) const
#else
Acad::ErrorStatus PDSqucone::explode(AcDbVoidPtrArray& entitySet) const
#endif
{
	assertReadEnabled();

	AcGePoint3dArray pts;
	//为了避免多次坐标转换的误差，重写了getVertices函数
	getVertices_XY(pts);
	if(pts.length() < 8)
		return Acad::eNotApplicable;

	//  [9/6/2007 suzhiyong]
	//画出矩形断台面的包围盒，对其包围盒进行裁剪
	//包围盒由其一个底面所在平面拉伸而成

	//createFromCurves的输入参数要求curves位于xy平面，所以先在xy平面创建solid，
	//然后用setCoordSystem做坐标转换

	//求此8个点的包围盒（轴平行）
	AcGePoint3d p1(0,0,0), p2(0,0,0), p3(0,0,0), p4(0,0,0);

	{
		double minX = pts[0].x, maxX = pts[0].x, minY = pts[0].y, maxY = pts[0].y;
		for (int i=1; i<8; i++)
		{
			minX = min(minX, pts[i].x);
			maxX = max(maxX, pts[i].x);
			minY = min(minY, pts[i].y);
			maxY = max(maxY, pts[i].y);
		}

		p1.set(minX, minY, 0);
		p2.set(maxX, minY, 0);
		p3.set(maxX, maxY, 0);
		p4.set(minX, maxY, 0);
	}

	AcGeVector3d vec(0,0,1);

	AcDbLine line11(p1, p2);
	line11.setNormal(vec);
	AcDbLine line12(p2, p3);
	line12.setNormal(vec);
	AcDbLine line13(p3, p4);
	line13.setNormal(vec);
	AcDbLine line14(p4, p1);
	line14.setNormal(vec);

	AcDbVoidPtrArray curveSegments;
	curveSegments.append(&line11);
	curveSegments.append(&line12);
	curveSegments.append(&line13);
	curveSegments.append(&line14);

	AcDbVoidPtrArray regions;
	Acad::ErrorStatus es = Acad::eOk;
	es = AcDbRegion::createFromCurves(curveSegments, regions);

	if(es == Acad::eOk && !regions.isEmpty())
	{
		AcDb3dSolid *pBody;
		pBody = new AcDb3dSolid;
		es = pBody->extrude((AcDbRegion*)(regions[0]), m_dheight, 0.0);
		if(es != Acad::eOk)
		{
			delete pBody;
			for(int i = 0; i < regions.length(); i++)
				delete (AcRxObject*)regions[i];
			return Acad::eNotApplicable;
		}

		AcGePlane slicePlane;
		AcDb3dSolid *pHalfSolid = NULL;
		slicePlane.set(pts[0], -((pts[1] - pts[0]).crossProduct(pts[4] - pts[0])));
		pBody->getSlice(slicePlane, Adesk::kFalse, pHalfSolid);
		slicePlane.set(pts[1], -((pts[2] - pts[1]).crossProduct(pts[5] - pts[1])));
		pBody->getSlice(slicePlane, Adesk::kFalse, pHalfSolid);
		slicePlane.set(pts[2], -((pts[3] - pts[2]).crossProduct(pts[6] - pts[2])));
		pBody->getSlice(slicePlane, Adesk::kFalse, pHalfSolid);
		slicePlane.set(pts[3], -((pts[0] - pts[3]).crossProduct(pts[7] - pts[3])));
		pBody->getSlice(slicePlane, Adesk::kFalse, pHalfSolid);

		//变换到原坐标系
		AcGeVector3d vectw = m_VectH.crossProduct(m_VectL).normal();
		AcGeVector3d vectl = vectw.crossProduct(m_VectH).normal();

		AcGeMatrix3d mat;
		mat.setCoordSystem(m_ptorign, vectl, vectw, m_VectH);
		pBody->transformBy(mat);

		pBody->setPropertiesFrom(this);
		entitySet.append(pBody);
	}
	else
	{
		for(int i = 0; i < regions.length(); i++)
			delete (AcRxObject*)regions[i];
		return Acad::eNotApplicable;
	}

	for(int i = 0; i < regions.length(); i++)
		delete (AcRxObject*)regions[i];

	return Acad::eOk;
}

//  [9/5/2007 suzhiyong]
Acad::ErrorStatus PDSqucone::explodeTo3DSolid(AcDb3dSolid* &p3dSolid) const
{
	assertReadEnabled();

	AcGePoint3dArray pts;
	//为了避免多次坐标转换的误差，重写了getVertices函数
	getVertices_XY(pts);
	if(pts.length() < 8)
		return Acad::eNotApplicable;

	//  [9/6/2007 suzhiyong]
	//画出矩形断台面的包围盒，对其包围盒进行裁剪
	//包围盒由其一个底面所在平面拉伸而成

	//createFromCurves的输入参数要求curves位于xy平面，所以先在xy平面创建solid，
	//然后用setCoordSystem做坐标转换

	//求此8个点的包围盒（轴平行）
	AcGePoint3d p1(0,0,0), p2(0,0,0), p3(0,0,0), p4(0,0,0);

	{
		double minX = pts[0].x, maxX = pts[0].x, minY = pts[0].y, maxY = pts[0].y;
		for (int i=1; i<8; i++)
		{
			minX = min(minX, pts[i].x);
			maxX = max(maxX, pts[i].x);
			minY = min(minY, pts[i].y);
			maxY = max(maxY, pts[i].y);
		}

		p1.set(minX, minY, 0);
		p2.set(maxX, minY, 0);
		p3.set(maxX, maxY, 0);
		p4.set(minX, maxY, 0);
	}

	AcGeVector3d vec(0,0,1);

	AcDbLine line11(p1, p2);
	line11.setNormal(vec);
	AcDbLine line12(p2, p3);
	line12.setNormal(vec);
	AcDbLine line13(p3, p4);
	line13.setNormal(vec);
	AcDbLine line14(p4, p1);
	line14.setNormal(vec);

	AcDbVoidPtrArray curveSegments;
	curveSegments.append(&line11);
	curveSegments.append(&line12);
	curveSegments.append(&line13);
	curveSegments.append(&line14);

	AcDbVoidPtrArray regions;
	Acad::ErrorStatus es = Acad::eOk;
	es = AcDbRegion::createFromCurves(curveSegments, regions);

	if(es == Acad::eOk && !regions.isEmpty())
	{
		AcDb3dSolid *pBody;
		pBody = new AcDb3dSolid;
		es = pBody->extrude((AcDbRegion*)(regions[0]), m_dheight, 0.0);
		if(es != Acad::eOk)
		{
			delete pBody;
			for(int i = 0; i < regions.length(); i++)
				delete (AcRxObject*)regions[i];
			return Acad::eNotApplicable;
		}

		AcGePlane slicePlane;
		AcDb3dSolid *pHalfSolid = NULL;
		slicePlane.set(pts[0], -((pts[1] - pts[0]).crossProduct(pts[4] - pts[0])));
		pBody->getSlice(slicePlane, Adesk::kFalse, pHalfSolid);
		slicePlane.set(pts[1], -((pts[2] - pts[1]).crossProduct(pts[5] - pts[1])));
		pBody->getSlice(slicePlane, Adesk::kFalse, pHalfSolid);
		slicePlane.set(pts[2], -((pts[3] - pts[2]).crossProduct(pts[6] - pts[2])));
		pBody->getSlice(slicePlane, Adesk::kFalse, pHalfSolid);
		slicePlane.set(pts[3], -((pts[0] - pts[3]).crossProduct(pts[7] - pts[3])));
		pBody->getSlice(slicePlane, Adesk::kFalse, pHalfSolid);

		//变换到原坐标系
		AcGeVector3d vectw = m_VectH.crossProduct(m_VectL).normal();
		AcGeVector3d vectl = vectw.crossProduct(m_VectH).normal();

		AcGeMatrix3d mat;
		mat.setCoordSystem(m_ptorign, vectl, vectw, m_VectH);
		pBody->transformBy(mat);

		pBody->setPropertiesFrom(this);

		p3dSolid = pBody;
	}
	else
	{
		for(int i = 0; i < regions.length(); i++)
			delete (AcRxObject*)regions[i];
		return Acad::eNotApplicable;
	}

	for(int i = 0; i < regions.length(); i++)
		delete (AcRxObject*)regions[i];

	return Acad::eOk;
}

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDSqucone::subGetGeomExtents(AcDbExtents& extents) const
#else
Acad::ErrorStatus PDSqucone::getGeomExtents(AcDbExtents& extents) const
#endif
{
	assertReadEnabled();
	extents.set(m_ptorign, m_ptorign);
	AcGePoint3dArray pArray;
	getVertices(pArray);
	int i, arrlen = pArray.length();
	for(i = 0; i < arrlen; ++i)
		extents.addPoint(pArray[i]);
	return Acad::eOk;
}

#ifdef _OBJECTARX2010_
void PDSqucone::subList() const
#else
void PDSqucone::list() const
#endif
{
	assertReadEnabled();
#ifdef _OBJECTARX2010_
	PDPrimary3D::subList();
#else
	PDPrimary3D::list();
#endif
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4732/*"\n 插入点(%f,%f,%f)\n"*/,m_ptorign.x,m_ptorign.y,m_ptorign.z);
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4802/*" 底面的法向量(%f,%f,%f)\n"*/,m_VectH.x,m_VectH.y,m_VectH.z);
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4803/*" 底面矩形的长度=%.3f  底面矩形的宽度=%.3f\n"*/,m_dlength1,m_dwidth1);
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4804/*" 底面长轴方向(%f,%f,%f)\n"*/,m_VectL.x,m_VectL.y,m_VectL.z);
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4805/*" 顶面矩形的长度=%.3f  顶面矩形的宽度=%.3f\n"*/,m_dlength2,m_dwidth2);
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4806/*" 断面台的高=%.3f\n"*/,m_dheight);
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4748/*" 偏心方向(%f,%f,%f)\n\n"*/,m_VectV.x,m_VectV.y,m_VectV.z);
	return ;
}//added by linin 20050810

void PDSqucone::createBody()
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
double PDSqucone::GetMaxLength()
{
	double dMax  = (m_VectH * m_dheight  + m_VectV * m_dlean).length();
	if(dMax < m_dlength1)
		dMax = m_dlength1;
	if(dMax < m_dlength2)
		dMax = m_dlength2;
	if(dMax < m_dwidth1)
		dMax = m_dwidth1;
	if(dMax < m_dwidth2)
		dMax = m_dwidth2;

	m_dMaxLength = dMax;
	return m_dMaxLength;
}