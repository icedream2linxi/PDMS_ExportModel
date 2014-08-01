// PDScylinder.cpp: implementation of the PDScylinder class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "language25.h"
#include "PDScylinder.h"
#include "dbmain.h"
#include "dbproxy.h"
#include "math.h"
#include "gecspl3d.h"
#include "geell3d.h"
#include "gearc3d.h"
#include "gept3dar.h"
#include "geassign.h"
#include "acgi.h"
#include <dbsol3d.h>
#include "dbapserv.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define VERSION_PDSCYLINDER 1
#define PI 3.1415926535897932384626433832795L

#ifdef _OBJECTARX2000_
ACRX_DXF_DEFINE_MEMBERS(PDScylinder, PDCylinder,
						AcDb::kDHL_CURRENT, AcDb::kMReleaseCurrent,
						AcDbProxyEntity::kAllAllowedBits,
						PDSCYLINDER, Gle);
#else
ACRX_DXF_DEFINE_MEMBERS(PDScylinder, PDCylinder, AcDbProxyEntity::kAllAllowedBits, PDSCYLINDER, Gle);
#endif

void PDScylinder::setDefault()
{
	PDPrimary3D::setDefault();
#if defined(NDEBUG) || !defined(_DEBUG)
	m_vect = AcGeVector3d( 0,0,-1);
#else
	m_vect = AcGeVector3d( 0,0,-1);
#endif
}
bool PDScylinder::isValidData(double &f)
{
	if(!PDPrimary3D::isValidData(f))
		return false;
	if(!isValidFloat(f=m_vect.x) || !isValidFloat(f=m_vect.y) || !isValidFloat(f=m_vect.z))
		return false;
	return true;
}

//zxb,20090113,增加audit，暂时只check自身成员数据
Acad::ErrorStatus PDScylinder::audit(AcDbAuditInfo* pAuditInfo)
{
	return auditPDPrimary3D(this,pAuditInfo,_T("PDScylinder"));
}

PDScylinder::PDScylinder(bool HasSnap/* = false*/) : PDCylinder(HasSnap)
{
	m_vect = AcGeVector3d( 0,0,-1);
}

PDScylinder::PDScylinder(AcGePoint3d ptStart,AcGePoint3d ptEnd,double Diameter,AcGeVector3d vect,int Precision/* = 12*/, bool HasSnap/* = false*/)
: PDCylinder(ptStart, ptEnd, Diameter, Precision, HasSnap)
{
	m_vect = vect;

#ifdef _USEAMODELER_
	createBody();
#endif

	//合法控制
	/*  double leng=m_ptStart.distanceTo(m_ptEnd);
	AcGeVector3d tmpvect=m_ptEnd-m_ptStart;
	double ang=m_vect.angleTo(tmpvect) ;
	if( ang>PI/2 ){ ang=PI-ang; }
	if( m_dDiameter*cos(ang)/sin(ang) >= leng ){
	m_vect=tmpvect.normal();
	}
	*/
}

PDScylinder::~PDScylinder()
{
}

Acad::ErrorStatus PDScylinder::setParameters(const AcGePoint3d &ptStart, 
											 const AcGePoint3d &ptEnd, 
											 double Diameter, 
											 const AcGeVector3d &vec,
											 int Precision/* = PRIMARY3D_DEFAULT_PRECISION*/)
{
	assertWriteEnabled();

	m_vect = vec;

	return PDCylinder::setParameters(ptStart,ptEnd,Diameter,Precision);
}

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDScylinder::subTransformBy(const AcGeMatrix3d& xform)
#else
Acad::ErrorStatus PDScylinder::transformBy(const AcGeMatrix3d& xform)
#endif
{
	assertWriteEnabled();
	// TODO: implement this function.
	m_vect.transformBy(xform);
#ifdef _OBJECTARX2010_
	return PDCylinder::subTransformBy(xform);
#else
	return PDCylinder::transformBy(xform);
#endif
}

#ifdef _OBJECTARX2010_
Adesk::Boolean PDScylinder::subWorldDraw(AcGiWorldDraw* mode)
#else
Adesk::Boolean PDScylinder::worldDraw(AcGiWorldDraw* mode)
#endif
{
	assertReadEnabled();
	//if(acdbHostApplicationServices()->workingDatabase()->useri1() == 1)
	// return Adesk::kTrue;

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
	int Precision = m_dDividPrecision;
	int PreStep = 1;
	AcGePoint3dArray pArray;
	int actPrecision__1 = Precision + 1;	
	int i;
	double radius = m_dDiameter / 2.0;

	Adesk::Int16 nRating = getCalMaxPrecision();
	if (nRating == PRIMARY3D_SIMPLIZE_RATE1 ||
		nRating == PRIMARY3D_SIMPLIZE_RATE2)
	{
		Precision = (int)	(PI / acos(1.0 - mode->deviation(kAcGiMaxDevForCircle, m_ptStart)* 2.0 / m_dDiameter));
		if(Precision < PRIMARY3D_SIMPLIZE_SCALE)
		{
			if (nRating == PRIMARY3D_SIMPLIZE_RATE2)
			{	
				AcGePoint3d pts[2];
				pts[0] = m_ptStart;
				pts[1] = m_ptEnd;
				mode->geometry().polyline(2,pts);
			}
			else if (nRating == PRIMARY3D_SIMPLIZE_RATE1)
			{
				Precision = PRIMARY3D_WIREFRAME_LINENUM4;
				getVertices(Precision, pArray);
				actPrecision__1 = Precision + 1;

				AcGeVector3d vect = (m_ptEnd - m_ptStart).normal();
				if(mode -> geometry().polyline( actPrecision__1,pArray.asArrayPtr()))
					return Adesk::kTrue;
				if(mode -> geometry().circle( m_ptEnd, radius, vect))
					return Adesk::kTrue;

				AcGePoint3d pt[2];
				for(i = 0; i < Precision; i += PreStep)
				{
					pt[0] = pArray[i];
					pt[1] = pArray[i + actPrecision__1];
					if(mode->geometry().polyline(2, pt))
						return Adesk::kTrue;
				}
			}
			return Adesk::kTrue;
		}
	} 
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
			if(Precision < PRIMARY3D_WIREFRAME_LINENUM * 2)
				Precision = PRIMARY3D_WIREFRAME_LINENUM;
			else
			{
				if(Precision > m_dDividPrecision)
					Precision = m_dDividPrecision;
				PreStep = Precision / PRIMARY3D_WIREFRAME_LINENUM;
				Precision = PreStep * PRIMARY3D_WIREFRAME_LINENUM;
			}
			getVertices(Precision, pArray);
			actPrecision__1 = Precision + 1;
			//added by szw 2009.11.18 : begin
			mode->subEntityTraits().setSelectionMarker(++gsIndex);
			//added by szw 2009.11.18 : end
			if(mode->geometry().polygon(Precision, &pArray[0]))
				return Adesk::kTrue;
			//added by szw 2009.11.18 : begin
			mode->subEntityTraits().setSelectionMarker(++gsIndex);
			//added by szw 2009.11.18 : end
			if(mode->geometry().polygon(Precision, &pArray[actPrecision__1]))
				return Adesk::kTrue;


			AcGiEdgeData edgeData;
			int edgeVisibilityLen = actPrecision__1 * 3 - 2;
			Adesk::UInt8* edgeVisibility = new Adesk::UInt8[edgeVisibilityLen];
			int Precision___2 = Precision * 2;
			for(i = 0; i < Precision___2; ++i)
				edgeVisibility[i] = kAcGiVisible;
			for(i = Precision___2; i < edgeVisibilityLen; i += PreStep)
			{
				edgeVisibility[i] = kAcGiVisible;
				if(PreStep > 1 && i != edgeVisibilityLen - 1)
				{
					for(int j = 1; j < PreStep; ++j)
						edgeVisibility[i + j] = kAcGiInvisible;
				}
			}
			//added by szw 2009.11.18 : begin
			mode->subEntityTraits().setSelectionMarker(++gsIndex);
			//added by szw 2009.11.18 : end
			mode->geometry().mesh(2, actPrecision__1, pArray.asArrayPtr(), &edgeData);//, NULL&faceData);
			delete [] edgeVisibility;
		}
#ifndef _ALWAYSSHOWWIRE_
		break;
	case kAcGiStandardDisplay:
	case kAcGiSaveWorldDrawForR12:
	case kAcGiSaveWorldDrawForProxy:
#endif
		{
			Precision = PRIMARY3D_WIREFRAME_LINENUM;
			getVertices(Precision, pArray);
			actPrecision__1 = Precision + 1;
			//统一线框模型和实体模型的GS标记
			//modified by szw 2009.11.18 : begin
			int gsIndex = 0;
			AcGeVector3d vect = (m_ptEnd - m_ptStart).normal();
			mode->subEntityTraits().setSelectionMarker(++gsIndex);
			if(mode -> geometry().polyline( actPrecision__1,pArray.asArrayPtr()))
				return Adesk::kTrue;
			mode->subEntityTraits().setSelectionMarker(++gsIndex);
			if(mode -> geometry().circle( m_ptEnd, radius, vect))
				return Adesk::kTrue;

			AcGePoint3d pt[2];
			gsIndex++;
			for(i = 0; i < Precision; i += PreStep)
			{
				pt[0] = pArray[i];
				pt[1] = pArray[i + actPrecision__1];
				mode->subEntityTraits().setSelectionMarker(gsIndex);
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
}// added by linlin 20050815

/*Adesk::Boolean PDScylinder::worldDraw(AcGiWorldDraw* mode)
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
AcGeIntArray stdIdx;
int actPrecision;
getVertices(getMaxPrecision(mode), pArray, stdIdx, actPrecision);
int actPrecision__1 = actPrecision + 1;
int stdIdxLen = stdIdx.length();
int stdIdxLen_1 = stdIdxLen - 1;
int i;

double radius = m_dDiameter / 2.0;
int maxPrecision = getMaxPrecision(mode);


switch(mode->regenType())
{
case kAcGiStandardDisplay:
case kAcGiSaveWorldDrawForR12:
case kAcGiSaveWorldDrawForProxy:
{
int gsIndex = 0;
AcGeVector3d vect = (m_ptEnd - m_ptStart).normal();
mode->subEntityTraits().setSelectionMarker(++gsIndex);
mode -> geometry().polyline( actPrecision__1,pArray.asArrayPtr());
mode->subEntityTraits().setSelectionMarker(++gsIndex);
mode -> geometry().circle( m_ptEnd, radius, vect);

AcGePoint3d pt[2];
for(i = 0; i < stdIdxLen_1; i++)
{
pt[0] = pArray[stdIdx[i]];
pt[1] = pArray[stdIdx[i] + actPrecision__1];
mode->subEntityTraits().setSelectionMarker(++gsIndex);
mode->geometry().polyline(2, pt);
}
}
break;
case kAcGiHideOrShadeCommand:
case kAcGiRenderCommand:
{
mode->subEntityTraits().setFillType(kAcGiFillAlways);
mode->geometry().polygon(actPrecision, &pArray[0]);
mode->geometry().polygon(actPrecision, &pArray[actPrecision__1]);


AcGiEdgeData edgeData;
int edgeVisibilityLen = actPrecision__1 * 3 - 2;
Adesk::UInt8* edgeVisibility = new Adesk::UInt8[edgeVisibilityLen];
for(i = 0; i < edgeVisibilityLen; ++i)
edgeVisibility[i] = kAcGiInvisible;//kAcGiSilhouette;
int actPrecision___2 = actPrecision * 2;
for(i = 0; i < stdIdxLen; ++i)
edgeVisibility[stdIdx[i] + actPrecision___2] = kAcGiVisible;
edgeData.setVisibility(edgeVisibility); 

mode->geometry().mesh(2, actPrecision__1, pArray.asArrayPtr(), &edgeData);//, NULL&faceData);
delete [] edgeVisibility;
}
break;
default:
break;
}
#endif
return Adesk::kTrue;
}// added by linlin 20050815*/

/////////////////////////////////////////////////////////////////
//////	求两端的椭圆并进行剖分，取得离散点，放到引用数组中	//////
//////////////////////////////////////////////////////////////////
Acad::ErrorStatus PDScylinder::getVertices(int Precision, AcGePoint3dArray& vertexArray, 
										   AcGeIntArray &stdIdx, int &actPrecision) const
{
	assertReadEnabled();

	AcGeDoubleArray dividDbl;
	getActDivid(Precision, dividDbl, stdIdx);

	int actPrecision__1 = dividDbl.length();
	actPrecision = actPrecision__1 - 1;
	vertexArray.setLogicalLength(actPrecision__1 * 2);

	AcGeVector3d vectCenter = (m_ptStart - m_ptEnd).normal();
	double angle = m_vect.angleTo(vectCenter);
	AcGeVector3d vect= m_vect;
	if(angle > PI / 2.0)
	{
		angle = PI - angle;
		vect.negate();
	}
	double dist;
	dist = m_dDiameter / 2.0 * tan(angle);
	AcGePlane plane;
	plane.set(m_ptStart, vect);


	AcGePoint3d spPt;
	spPt = m_ptStart + vectCenter * dist;

	AcGeCircArc3d sCir, eCir;
	sCir.set(spPt, vectCenter, m_dDiameter / 2.0);
	eCir.set(m_ptEnd, vectCenter, m_dDiameter / 2.0);
	AcGePoint3dArray spArr, epArr, interSpArr;
	if(actPrecision> 0)
	{
		spArr.append(sCir.startPoint());
		epArr.append(eCir.startPoint());
		AcGeVector3d SCirSpVec(sCir.startPoint() - spPt);
		AcGeVector3d ECirSpVec(eCir.startPoint() - m_ptEnd);
		for(int i = 1; i < actPrecision; i++)
		{
			AcGeVector3d vecTemp = SCirSpVec;
			spArr.append(spPt + vecTemp.rotateBy(PI * 2.0 / Precision * dividDbl[i], vectCenter).normal() * m_dDiameter / 2.0);
			vecTemp = ECirSpVec;
			epArr.append(m_ptEnd + vecTemp.rotateBy(PI * 2.0 / Precision * dividDbl[i], vectCenter).normal() * m_dDiameter / 2.0);
		}
		spArr.append(sCir.startPoint());
		epArr.append(eCir.startPoint());
	}

	for(int i = 0; i < spArr.length(); i++)
	{
		AcGeLineSeg3d lineTemp(epArr[i], spArr[i] + vectCenter * 1);
		AcGePoint3d interPt;
		if(plane.intersectWith(lineTemp, interPt))
			interSpArr.append(interPt);
		else
			interSpArr.append(spArr[i]);
	}
	while(!vertexArray.isEmpty())
		vertexArray.removeLast();
	vertexArray.append(interSpArr);
	vertexArray.append(epArr);



	return Acad::eOk;
}//added by linlin 20050815


Acad::ErrorStatus PDScylinder::getVertices(int Precision, AcGePoint3dArray& vertexArray) const
{
	assertReadEnabled();

	int actPrecision__1 = Precision + 1;

	vertexArray.setLogicalLength(0);

	AcGeVector3d vectCenter = (m_ptStart - m_ptEnd).normal();
	double angle = m_vect.angleTo(vectCenter);
	AcGeVector3d vect= m_vect;
	if(angle > PI / 2.0)
	{
		angle = PI - angle;
		vect.negate();
	}
	double dist;
	dist = m_dDiameter / 2.0 * tan(angle);
	AcGePlane plane;
	plane.set(m_ptStart, vect);

	AcGePoint3d spPt;
	spPt = m_ptStart + vectCenter * dist;

	AcGeCircArc3d sCir, eCir;
	sCir.set(spPt, vectCenter, m_dDiameter / 2.0);
	eCir.set(m_ptEnd, vectCenter, m_dDiameter / 2.0);
	AcGePoint3dArray spArr, epArr, interSpArr;
	spArr.append(sCir.startPoint());
	epArr.append(eCir.startPoint());
	AcGeVector3d SCirSpVec(sCir.startPoint() - spPt);
	AcGeVector3d ECirSpVec(eCir.startPoint() - m_ptEnd);
	int i ;
	AcGeVector3d vecTemp;
	double anglePre = PI * 2.0 / Precision;
	for(i = 1; i < Precision; i++)
	{
		SCirSpVec.rotateBy(anglePre,vectCenter);
		spArr.append(spPt + SCirSpVec.normal() * m_dDiameter / 2.0);
		ECirSpVec.rotateBy(anglePre,vectCenter);
		epArr.append(m_ptEnd + ECirSpVec.normal() * m_dDiameter / 2.0);
	}
	spArr.append(sCir.startPoint());
	epArr.append(eCir.startPoint());

	AcGePoint3d interPt;
	AcGeLineSeg3d lineTemp;
	for(i = 0; i < spArr.length(); i++)
	{
		lineTemp.set(epArr[i], spArr[i] + vectCenter);
		if(plane.intersectWith(lineTemp, interPt))
			interSpArr.append(interPt);
		else
			interSpArr.append(spArr[i]);
	}
	vertexArray.append(interSpArr);
	vertexArray.append(epArr);

	return Acad::eOk;
}//added by linlin 20050815

Acad::ErrorStatus PDScylinder::dwgOutFields(AcDbDwgFiler* filer) const
{
	assertReadEnabled();
	Acad::ErrorStatus es;

	// Call dwgOutFields from PDPrimary3D
	if ((es = PDCylinder::dwgOutFields(filer)) != Acad::eOk) {
		return es;
	}

	// Write version number.
	filer->writeItem((Adesk::UInt16) VERSION_PDSCYLINDER);

	// Write the data members.
	// TODO: here you can file datamembers not
	//       created by the ObjectARX Add-In.
	filer->writeItem(m_vect);

	return filer->filerStatus();
}


Acad::ErrorStatus PDScylinder::dwgInFields(AcDbDwgFiler* filer)
{
	assertWriteEnabled();
	Acad::ErrorStatus es;

	// Call dwgInFields from PDPrimary3D
	if ((es = PDCylinder::dwgInFields(filer)) != Acad::eOk) {
		return es;
	}

	// Read version number.
	Adesk::UInt16 version;
	filer->readItem(&version);
	if (version > VERSION_PDSCYLINDER)
		return Acad::eMakeMeProxy;

	// Read the data members.
	switch (version)
	{
	case (1):
		// TODO: here you can file datamembers not
		//       created by the ObjectARX Add-In.
		filer->readItem(&m_vect);
		break;
	}

#ifdef _USEAMODELER_
	if(m_3dGeom.isNull())
		createBody();
#endif

	return filer->filerStatus();
}


Acad::ErrorStatus PDScylinder::dxfOutFields(AcDbDxfFiler* filer) const
{
	assertReadEnabled();
	Acad::ErrorStatus es;

	if ((es = PDCylinder::dxfOutFields(filer)) != Acad::eOk)
		return es;

	// Write subclass marker.
	filer->writeItem(AcDb::kDxfSubclass, _T("PDSCylinder"));

	// Write version number.
	filer->writeItem(AcDb::kDxfInt16, (Adesk::UInt16) VERSION_PDSCYLINDER);

	// TODO: here you can file datamembers not
	//       created by the ObjectARX Add-In.
	filer->writeItem(AcDb::kDxfNormalX, m_vect );

	return filer->filerStatus();
}

Acad::ErrorStatus PDScylinder::dxfInFields(AcDbDxfFiler* filer)
{
	assertWriteEnabled();
	Acad::ErrorStatus es = Acad::eOk;
	struct resbuf rb;

	if ((PDCylinder::dxfInFields(filer) != Acad::eOk) ||
		!filer->atSubclassData(_T("PDSCylinder")))
		return filer->filerStatus();

	// Read version number.
	Adesk::UInt16 version;
	filer->readItem(&rb);
	if (rb.restype != AcDb::kDxfInt16) {
		filer->pushBackItem();
		filer->setError(Acad::eInvalidDxfCode,
			_T("nError: expected object version group code %d"),
			AcDb::kDxfInt16);
		return filer->filerStatus();
	} else {
		version = rb.resval.rint;
		if (version > VERSION_PDSCYLINDER)
			return Acad::eMakeMeProxy;
	}

	// TODO: here you can file datamembers not
	//       created by the ObjectARX Add-In.

	// Read the data members.
	switch (version){
	case (1):
		while ((es == Acad::eOk) && ((es = filer->readResBuf(&rb)) == Acad::eOk)){
			switch (rb.restype){
	case AcDb::kDxfNormalX:
		m_vect = asVec3d(rb.resval.rpoint);
		break;
			}
		}
		break;
	}

#ifdef _USEAMODELER_
	if(m_3dGeom.isNull())
		createBody();
#endif

	return filer->filerStatus();
}

Acad::ErrorStatus PDScylinder::vect(AcGeVector3d& vect)
{
	assertReadEnabled();
	vect = m_vect;
	return Acad::eOk;
}

Acad::ErrorStatus PDScylinder::setVect(AcGeVector3d vect)
{
	assertWriteEnabled();
	m_vect = vect;
	return Acad::eOk;
}

double PDScylinder::getLeanAngle()const
{
	return m_vect.angleTo(m_ptStart-m_ptEnd);
}

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDScylinder::subGetOsnapPoints(
	AcDb::OsnapMode     osnapMode,
	Adesk::GsMarker     gsSelectionMark,
	const AcGePoint3d&  pickPoint,
	const AcGePoint3d&  lastPoint,
	const AcGeMatrix3d& viewXform,
	AcGePoint3dArray&   snapPoints,
	AcDbIntArray &   geomIds) const
#else
Acad::ErrorStatus PDScylinder::getOsnapPoints(
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


	AcGePoint3dArray pArray;
	AcGeIntArray stdIdx;
	int actPrecision;
	getVertices(m_dDividPrecision, pArray, stdIdx, actPrecision);
	int actPrecision__1 = actPrecision + 1;
	int stdIdxLen = stdIdx.length();
	int stdIdxLen_1 = stdIdxLen - 1;
	//	
	//  if(verArr.length() < 2)
	//    return Acad::eOk;

	AcGeVector3d viewDir(viewXform(Z, 0), viewXform(Z, 1),
		viewXform(Z, 2));
	AcGeVector3d vect = (m_ptStart - m_ptEnd).normal();

	double angle = m_vect.angleTo(vect);
	AcGeVector3d PlaneVect = m_vect;
	if(angle > PI / 2.0)
	{
		angle = PI - angle;
		PlaneVect.negate();
	}
	double dist;
	dist = m_dDiameter / 2.0 * tan(angle);
	AcGePlane plane;
	plane.set(m_ptStart, PlaneVect);
	AcGePoint3d spPt;
	spPt = m_ptStart + vect * dist;
	AcGeVector3d PlaneX, PlaneY;
	PlaneX = PlaneVect.crossProduct(vect).normal();
	PlaneY = PlaneVect.crossProduct(PlaneX).normal();

	int i;
	switch(osnapMode)
	{
	case AcDb::kOsModeEnd:
		snapPoints.append(m_ptStart);
		snapPoints.append(m_ptEnd);
		for( i = 0; i < stdIdxLen_1; ++i)
		{
			snapPoints.append(pArray[stdIdx[i]]);
			snapPoints.append(pArray[stdIdx[i] + actPrecision__1]);
		}
		break;
	case AcDb::kOsModeMid:
		{
			snapPoints.append(m_ptStart + (m_ptEnd - m_ptStart) / 2.0);
			//棱边的中点

			for(i = 0; i < stdIdxLen_1; ++i)
				snapPoints.append(pArray[stdIdx[i]] + 
				(pArray[stdIdx[i] + actPrecision__1] - pArray[stdIdx[i]]) / 2.0);
		}
		break;
	case AcDb::kOsModeCen:
		{
			if(gsSelectionMark_int == 1)
				snapPoints.append(m_ptStart);
			else if(gsSelectionMark_int == 2)
				snapPoints.append(m_ptEnd);
			else
			{
				AcGeVector3d cenVect = m_ptEnd - m_ptStart;
				snapPoints.append(m_ptStart + cenVect / 2.0);
			}
			break;
		}
	case AcDb::kOsModeQuad:
		{
			AcGePoint3d stdPtArr[8];
			AcGeCircArc3d sCir(spPt, vect, m_dDiameter / 2.0);
			AcGeCircArc3d eCir(m_ptEnd, vect, m_dDiameter/ 2.0);
			AcGeVector3d SCirSpVec(sCir.startPoint() - spPt);
			AcGeVector3d ECirSpVec(eCir.startPoint() - m_ptEnd);
			for(i = 0; i < 4; i++)
			{
				AcGeVector3d vecTemp = SCirSpVec;
				stdPtArr[i] = spPt + vecTemp.rotateBy(PI / 2.0 * i, vect).normal() * m_dDiameter / 2.0;
				vecTemp = ECirSpVec;
				stdPtArr[i + 4] = m_ptEnd + vecTemp.rotateBy(PI / 2.0 * i, vect).normal() * m_dDiameter / 2.0;
			}
			for(i = 0; i < 4; i++)
			{
				AcGeLineSeg3d lineTemp(stdPtArr[i + 4], stdPtArr[i] + vect * 1);
				AcGePoint3d interPt;
				if(plane.intersectWith(lineTemp, interPt))
					stdPtArr[i] = interPt;
			}
			for(i = 0; i < 8; i++)
				snapPoints.append(stdPtArr[i]);
			break;
		}
	case AcDb::kOsModeNode:
		break;
	case AcDb::kOsModeIns:
		snapPoints.append(m_ptStart);
		break;
	case AcDb::kOsModePerp:
		{
			AcGeLine3d line;
			AcGeVector3d vec;
			AcGePoint3d pt;
			//重新定义对象垂直正交点的捕捉方式，同时满足实体模型和线框模型的捕捉 
			//modified by szw 2009.11.18 : begin
			if(gsSelectionMark_int == 1)
			{
				AcGeEllipArc3d cir(m_ptStart, PlaneX, PlaneY, m_dDiameter / 2.0, m_dDiameter / 2.0 / cos(angle));
				pt = cir.closestPointTo(lastPoint);
				snapPoints.append(pt);
			}
			else if(gsSelectionMark_int == 2)
			{
				AcGeCircArc3d cir(m_ptEnd, vect, m_dDiameter / 2.0);
				pt = cir.closestPointTo(lastPoint);
				snapPoints.append(pt);
			}
			else if(gsSelectionMark_int <= stdIdxLen + 1)
			{
				for(int i = 0; i < stdIdxLen - 1; ++i)
				{
					vec = pArray[stdIdx[i]] - pArray[stdIdx[i] + actPrecision__1];
					line.set(pArray[stdIdx[i]], vec);
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
			//重新定义对象最近点的捕捉方式，同时满足实体模型和线框模型的捕捉 
			//modified by szw 2009.11.18 : begin
			//下底面
			AcGeEllipArc3d elipCir(m_ptStart, PlaneX, PlaneY, m_dDiameter / 2.0, m_dDiameter / 2.0 / cos(angle));
			pt = elipCir.projClosestPointTo(pickPoint, viewDir);
			snapPoints.append(pt);

			//上底面
			AcGeCircArc3d cir(m_ptEnd, vect, m_dDiameter / 2.0);
			pt = cir.projClosestPointTo(pickPoint, viewDir);
			snapPoints.append(pt);

			//棱边
			for(int i = 0; i <  stdIdxLen_1 ; i++)
			{ 
				lnsg.set(pArray[stdIdx[i]], pArray[stdIdx[i] + actPrecision__1]);
				pt = lnsg.projClosestPointTo(pickPoint, viewDir);
				snapPoints.append(pt);
			}
			//modified by szw 2009.11.18 : end
		}
		break;
	default:
		break;
	}

	return Acad::eOk;
}//added by linlin 20050815

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDScylinder::subGetGripPoints(
	AcGePoint3dArray& gripPoints,
	AcDbIntArray& osnapModes,
	AcDbIntArray& geomIds) const
#else
Acad::ErrorStatus PDScylinder::getGripPoints(
	AcGePoint3dArray& gripPoints,
	AcDbIntArray& osnapModes,
	AcDbIntArray& geomIds) const
#endif
{
	assertReadEnabled();
	/*
	gripPoints.append(m_ptStart);
	gripPoints.append(m_ptEnd);

	AcGeVector3d vectCenter = (m_ptStart - m_ptEnd).normal();
	//zxy 20020917 del		AcGeVector3d vectShort = m_vect.crossProduct(vectCenter).normal();
	AcGeVector3d vectShort = vectCenter.crossProduct(m_vect).normal();//zxy 20020917 add:保证椭圆主轴（这里是短轴）和另一轴（长轴）的叉积方向同m_vect
	AcGeVector3d vectLong = vectCenter.orthoProject(m_vect).normal();// * (m_dDiameter/2/cos(ang));
	*/
	/*zxy 20020917 del:斜截圆柱一端是椭圆，一端是圆
	AcGeVector3d mirrorvect = m_vect;
	mirrorvect.mirror(vectCenter);
	AcGeVector3d vectShortM = mirrorvect.crossProduct(vectCenter).normal();// * (m_dDiameter/2);
	AcGeVector3d vectLongM = vectCenter.orthoProject(mirrorvect).normal();// * (m_dDiameter/2/cos(ang));
	*/
	/*
	double ang = m_vect.angleTo(vectCenter);
	double minorRadius = m_dDiameter/2;
	double majorRadius = fabs(minorRadius/cos(ang));

	AcGeEllipArc3d ellipse = AcGeEllipArc3d(m_ptStart,vectShort, vectLong, minorRadius,majorRadius );//zxy 20020917 add：创建椭圆
	AcGeCircArc3d  circle = AcGeCircArc3d(m_ptEnd,-vectCenter,minorRadius);//zxy 20020917 add：创建圆
	*/
	/* zxy 20020917 del 
	AcGeCubicSplineCurve3d ellipseStart = AcGeCubicSplineCurve3d(AcGeEllipArc3d( 
	m_ptStart,vectShort, vectLong, minorRadius,majorRadius ));
	AcGeCubicSplineCurve3d ellipseEnd = AcGeCubicSplineCurve3d(AcGeEllipArc3d( 
	m_ptEnd,vectShortM,-vectLongM,minorRadius,majorRadius));//AcGeVector3d(vectLong[0],vectLong[1],0),minorRadius,minorRadius));
	*/
	//zxy 20020917 add: 
	/*        AcGePoint3dArray tmp;
	ellipse.getSamplePoints(4,tmp);
	gripPoints.append(tmp);
	AcGePoint3dArray tmpCirPtArr;
	circle.getSamplePoints(4, tmpCirPtArr);
	gripPoints.append(tmpCirPtArr);*/
	//zxy add end
	/*zxy 20020917 del
	double parameter;
	AcGePoint3d pt1,pt2;

	for(int i =0;i<4;i++){
	parameter = PI/2*i;
	pt1 = ellipseStart.evalPoint(parameter);
	gripPoints.append(pt1);
	pt2 = ellipseEnd.evalPoint(parameter);
	tmp.append(pt2);
	}
	gripPoints.append(tmp);
	*/
	return Acad::eOk; 
}

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDScylinder::subExplode(AcDbVoidPtrArray& entitySet) const
#else
Acad::ErrorStatus PDScylinder::explode(AcDbVoidPtrArray& entitySet) const
#endif
{
	assertReadEnabled();

	AcGeVector3d vectCenter = (m_ptStart - m_ptEnd).normal();
	double angle = m_vect.angleTo(vectCenter);
	AcGeVector3d vect = m_vect;
	if(angle > PI / 2.0)
	{
		angle = PI - angle;
		vect.negate();
	}
	double dist;
	dist = m_dDiameter / 2.0 * tan(angle);

	AcGePoint3d spPt;
	spPt = m_ptStart + vectCenter * dist;

	AcDb3dSolid *pBody;
	pBody = new AcDb3dSolid;
	if(pBody->createFrustum(spPt.distanceTo(m_ptEnd), m_dDiameter / 2.0,
		m_dDiameter / 2.0, m_dDiameter / 2.0) != Acad::eOk)
	{
		delete pBody;
		return Acad::eNotApplicable;
	}

	AcGeVector3d nor;
	nor = (m_ptEnd - spPt).normal();
	AcGeCircArc3d sCir;
	sCir.set(spPt, nor, m_dDiameter / 2.0);

	AcGeMatrix3d mat;
	AcGeVector3d x, y, z;
	z = nor;
	x = (sCir.startPoint() - spPt).normal();
	y = z.crossProduct(x).normal();
	mat.setCoordSystem(spPt + z * spPt.distanceTo(m_ptEnd) / 2.0, 
		x, y, z);
	pBody->transformBy(mat);

	AcGePlane slicePlane;
	slicePlane.set(m_ptStart, -m_vect);
	AcDb3dSolid *pHalfSolid = NULL;
	pBody->getSlice(slicePlane, Adesk::kFalse, pHalfSolid);

	pBody->setPropertiesFrom(this);
	entitySet.append(pBody);

	return Acad::eOk;
}

//  [9/5/2007 suzhiyong]
Acad::ErrorStatus PDScylinder::explodeTo3DSolid(AcDb3dSolid* &p3dSolid) const
{
	assertReadEnabled();

	AcGeVector3d vectCenter = (m_ptStart - m_ptEnd).normal();
	double angle = m_vect.angleTo(vectCenter);
	AcGeVector3d vect = m_vect;
	if(angle > PI / 2.0)
	{
		angle = PI - angle;
		vect.negate();
	}
	double dist;
	dist = m_dDiameter / 2.0 * tan(angle);

	AcGePoint3d spPt;
	spPt = m_ptStart + vectCenter * dist;

	AcDb3dSolid *pBody;
	pBody = new AcDb3dSolid;
	if(pBody->createFrustum(spPt.distanceTo(m_ptEnd), m_dDiameter / 2.0,
		m_dDiameter / 2.0, m_dDiameter / 2.0) != Acad::eOk)
	{
		delete pBody;
		return Acad::eNotApplicable;
	}

	AcGeVector3d nor;
	nor = (m_ptEnd - spPt).normal();
	AcGeCircArc3d sCir;
	sCir.set(spPt, nor, m_dDiameter / 2.0);

	AcGeMatrix3d mat;
	AcGeVector3d x, y, z;
	z = nor;
	x = (sCir.startPoint() - spPt).normal();
	y = z.crossProduct(x).normal();
	mat.setCoordSystem(spPt + z * spPt.distanceTo(m_ptEnd) / 2.0, 
		x, y, z);
	pBody->transformBy(mat);

	AcGePlane slicePlane;
	slicePlane.set(m_ptStart, -m_vect);
	AcDb3dSolid *pHalfSolid = NULL;
	pBody->getSlice(slicePlane, Adesk::kFalse, pHalfSolid);

	pBody->setPropertiesFrom(this);
	p3dSolid = pBody;

	return Acad::eOk;
}

void PDScylinder::createBody()
{
	assertWriteEnabled();
	//#ifdef _OBJECTARX2004_
	AcGeVector3d vectCenter = (m_ptStart - m_ptEnd).normal();
	double angle = m_vect.angleTo(vectCenter);
	AcGeVector3d vect = m_vect;
	if(angle > PI / 2.0)
	{
		angle = PI - angle;
		vect.negate();
	}
	double dist;
	dist = m_dDiameter / 2.0 * tan(angle);

	AcGePoint3d spPt;
	spPt = m_ptStart + vectCenter * dist;

	m_3dGeom = Body::cylinder(Line3d(*(Point3d*)&spPt, *(Point3d*)&m_ptEnd), m_dDiameter / 2.0, m_dDividPrecision);

	Plane secPlane(*(Point3d*)&m_ptStart, *(Vector3d*)&vect);
	m_3dGeom.section(secPlane);
	// #endif
}

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDScylinder::subGetGeomExtents(AcDbExtents& extents) const
#else
Acad::ErrorStatus PDScylinder::getGeomExtents(AcDbExtents& extents) const
#endif
{
	assertReadEnabled();
	extents.set(m_ptStart, m_ptStart);
	AcGePoint3dArray pArray;
	AcGeIntArray stdIdx;
	int actPrecision;
	getVertices(m_dDividPrecision, pArray, stdIdx, actPrecision);
	int i, arrLen = pArray.length();
	for(i = 0; i < arrLen; ++i)
		extents.addPoint(pArray[i]);
	return Acad::eOk;
}

#ifdef _OBJECTARX2010_
void PDScylinder::subList() const
#else
void PDScylinder::list() const
#endif
{
	assertReadEnabled();
#ifdef _OBJECTARX2010_
	PDCylinder::subList();
#else
	PDCylinder::list();
#endif
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4774/*" 斜截面的法矢(%f,%f,%f)\n\n"*/,m_vect.x,m_vect.y,m_vect.z);
	return ;
}// added by linlin 20050810

const AcGeVector3d & PDScylinder::getBottomNormal() const
{
	assertReadEnabled();
	return m_vect;
}

