// PDSphere.cpp: implementation of the PDSphere class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "language25.h"
#include "PDSphere.h"
#include "dbproxy.h"
#include "acgi.h"
#include "math.h"
#include "gecspl3d.h"
#include "geassign.h "
#include <dbsol3d.h>
#include < geintarr.h >
#include <adscodes.h>
#include "dbapserv.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define VERSION_PDSPHERE 1
#define PI 3.1415926535897932384626433832795L

#ifdef _OBJECTARX2000_
ACRX_DXF_DEFINE_MEMBERS(PDSphere, PDPrimary3D,
						AcDb::kDHL_CURRENT, AcDb::kMReleaseCurrent,
						AcDbProxyEntity::kAllAllowedBits,
						PDSPHERE, Gle);
#else
ACRX_DXF_DEFINE_MEMBERS(PDSphere, PDPrimary3D, AcDbProxyEntity::kAllAllowedBits, PDSPHERE, Gle);
#endif

void PDSphere::setDefault()
{
	PDPrimary3D::setDefault();
#if defined(NDEBUG) || !defined(_DEBUG)
	m_dRadius =1.0;
	m_ptCenter = AcGePoint3d();
#else
	m_dRadius =1000;
	m_ptCenter.set(0,0,0);
#endif
}
bool PDSphere::isValidData(double &f)
{
	if(!PDPrimary3D::isValidData(f))
		return false;
	if(!isValidFloat(f=m_dRadius))
		return false;
	if(!isValidFloat(f=m_ptCenter.x) || !isValidFloat(f=m_ptCenter.y) || !isValidFloat(f=m_ptCenter.z))
		return false;
	return true;
}

//zxb,20090113,增加audit，暂时只check自身成员数据
Acad::ErrorStatus PDSphere::audit(AcDbAuditInfo* pAuditInfo)
{
	return auditPDPrimary3D(this,pAuditInfo,_T("PDSphere"));
}

PDSphere::PDSphere(bool HasSnap/* = false*/) : PDPrimary3D(HasSnap)
{
	m_dRadius =1.0;
	m_ptCenter = AcGePoint3d();

	m_dDividPrecision = PRIMARY3D_DEFAULT_PRECISION;
}

PDSphere::~PDSphere()
{

}

PDSphere::PDSphere( AcGePoint3d ptcenter, double r, int precision/* = 12*/, bool HasSnap/* = false*/) : PDPrimary3D(HasSnap)
{
	m_dRadius =fabs(r) ;
	if(m_dRadius==0)
		m_dRadius=1;
	m_ptCenter = ptcenter;
	if(precision > 2)
		m_dDividPrecision = precision;
	else
		m_dDividPrecision = PRIMARY3D_DEFAULT_PRECISION;

#ifdef _USEAMODELER_
	createBody();
#endif
}//added by linlin 20050929

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDSphere::subTransformBy(const AcGeMatrix3d& xform)
#else
Acad::ErrorStatus PDSphere::transformBy(const AcGeMatrix3d& xform)
#endif
{
	assertWriteEnabled();
	// TODO: implement this function.
	m_ptCenter.transformBy(xform);
	m_dRadius *=  xform.scale();

#ifdef _OBJECTARX2010_
	return PDPrimary3D::subTransformBy(xform);
#else
	return PDPrimary3D::transformBy(xform);
#endif
}

#ifdef _OBJECTARX2010_
Adesk::Boolean PDSphere::subWorldDraw(AcGiWorldDraw* mode)
#else
Adesk::Boolean PDSphere::worldDraw(AcGiWorldDraw* mode)
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
	int Precision = m_dDividPrecision;
	Adesk::Int16 nRating = getCalMaxPrecision();

	if(nRating == PRIMARY3D_SIMPLIZE_RATE1 ||
		nRating == PRIMARY3D_SIMPLIZE_RATE2)
	{
		Precision = (int)	(PI / acos(1.0 - mode->deviation(kAcGiMaxDevForCircle, m_ptCenter) / m_dRadius));
		if(Precision < PRIMARY3D_SIMPLIZE_SCALE)
		{
			if (nRating == PRIMARY3D_SIMPLIZE_RATE2)
			{
				AcGePoint3d pts[2];
				pts[0] = pts[1] = m_ptCenter;
				mode->geometry().polyline(2,pts);
			}
			else if (nRating == PRIMARY3D_SIMPLIZE_RATE1)
			{
				//Precision = PRIMARY3D_WIREFRAME_LINENUM4;
				//getVertices(Precision, pArray, vertexNors);
				//actPrecision__1 = Precision + 1;
				if(mode -> geometry().circle( m_ptCenter,m_dRadius,AcGeVector3d(1,0,0)))
					return Adesk::kTrue;
				if(mode -> geometry().circle( m_ptCenter,m_dRadius,AcGeVector3d(1,1,0)))
					return Adesk::kTrue;
				if(mode -> geometry().circle( m_ptCenter,m_dRadius,AcGeVector3d(0,1,0)))
					return Adesk::kTrue;
				if(mode -> geometry().circle( m_ptCenter,m_dRadius,AcGeVector3d(-1,1,0)))
				{
					return Adesk::kTrue;
				}
				if(mode -> geometry().circle( m_ptCenter,m_dRadius,AcGeVector3d(0,0,1)))
				{
					return Adesk::kTrue;
				}
				if(mode -> geometry().circle( m_ptCenter - AcGeVector3d(0, 0, 1) * m_dRadius * cos(PI / 4.0),m_dRadius * sin(PI / 4.0),AcGeVector3d(0,0,1)))
				{
					return Adesk::kTrue;
				}
				mode -> geometry().circle( m_ptCenter + AcGeVector3d(0, 0, 1) * m_dRadius *  cos(PI / 4.0),m_dRadius * sin(PI / 4.0),AcGeVector3d(0,0,1));
			}
			Adesk::kTrue;
		}
	}
#ifndef _ALWAYSSHOWWIRE_	
	switch(mode->regenType())
	{
	case kAcGiHideOrShadeCommand:
	case kAcGiRenderCommand:
#endif
		{
			int PreStep = 1;	
			AcGePoint3dArray pArray;
			AcGeVector3dArray vertexNors;
			int actPrecision__1 = Precision + 1;
			if(Precision < PRIMARY3D_WIREFRAME_LINENUM * 2)
				Precision = PRIMARY3D_WIREFRAME_LINENUM;
			else
			{
				if(Precision > m_dDividPrecision)
					Precision = m_dDividPrecision;
				PreStep = Precision / PRIMARY3D_WIREFRAME_LINENUM;
				Precision = PreStep * PRIMARY3D_WIREFRAME_LINENUM;
			}
			getVertices(Precision, pArray, vertexNors);
			actPrecision__1 = Precision + 1;
			AcGiEdgeData edgeData;
			int edgeDataLen = Precision * 2 * (actPrecision__1)* 2;
			Adesk::UInt8* edgeVisibility = new Adesk::UInt8[edgeDataLen];
			int i, j;
			for(i = 0; i < edgeDataLen; ++i)
				edgeVisibility[i] = kAcGiInvisible;//kAcGiSilhouette;

// 			int actPrecision___2 = Precision * 2;
// 			for (i =0; i < actPrecision__1; i += PreStep )
// 			{
// 				for(j = 0; j < actPrecision___2; j++)
// 				{
// 					edgeVisibility[i * actPrecision___2 + j] = kAcGiVisible;
// 				}
// 			}
// 			int PrecisionArc___Cir = actPrecision__1 * Precision * 2;
// 			for (i = 0; i < actPrecision___2; i += PreStep)
// 			{	
// 				for(j = 0; j < Precision;  j ++)
// 				{
// 					edgeVisibility[PrecisionArc___Cir + i * Precision + j] = kAcGiVisible;
// 				}
// 			}
			int actPrecision___2 = Precision * 2;
 			for (i = Precision / 4; i < Precision; i += Precision / 4 )
 			{
				for(j = 0; j < actPrecision___2; j++)
				{
					edgeVisibility[i * actPrecision___2 + j] = kAcGiVisible;
				}
 			}
			int PrecisionArc___Cir = actPrecision__1 * Precision * 2;
			for (i = 0; i < actPrecision___2; i += actPrecision___2 / 8)
			{	
				for(j = 0; j < Precision;  j ++)
				{
					edgeVisibility[PrecisionArc___Cir + i * Precision + j] = kAcGiVisible;
				}
			}

			edgeData.setVisibility(edgeVisibility);

			// 因为下面代码会导致球体的render效果图不正确，所以将其注释。
			//AcGiVertexData vertexData;
			//vertexData.setNormals(vertexNors.asArrayPtr());
			//vertexData.setOrientationFlag(kAcGiCounterClockwise);
			
			//添加球面的GS标记，与线框模型的GS标记分开处理
			//added by szw 2009.11.18 : begin
			int gsIndex = 8;
			mode->subEntityTraits().setSelectionMarker(gsIndex);
			//added by szw 2009.11.18 : end

			mode->geometry().mesh(actPrecision__1,actPrecision___2 + 1,pArray.asArrayPtr(),&edgeData/*, NULL, &vertexData*/);
			delete [] edgeVisibility;
		}
#ifndef _ALWAYSSHOWWIRE_
		break;
	case kAcGiStandardDisplay:
	case kAcGiSaveWorldDrawForR12:
	case kAcGiSaveWorldDrawForProxy:
#endif
		{
			//Precision = PRIMARY3D_WIREFRAME_LINENUM;
			//getVertices(Precision, pArray, vertexNors);
			//actPrecision__1 = Precision + 1;
			int gsIndex = 0;
			mode->subEntityTraits().setSelectionMarker(++gsIndex);
			if(mode -> geometry().circle( m_ptCenter,m_dRadius,AcGeVector3d(1,0,0)))
				return Adesk::kTrue;
			mode->subEntityTraits().setSelectionMarker(++gsIndex);
			if(mode -> geometry().circle( m_ptCenter,m_dRadius,AcGeVector3d(1,1,0)))
				return Adesk::kTrue;
			mode->subEntityTraits().setSelectionMarker(++gsIndex);
			if(mode -> geometry().circle( m_ptCenter,m_dRadius,AcGeVector3d(0,1,0)))
				return Adesk::kTrue;
			mode->subEntityTraits().setSelectionMarker(++gsIndex);
			if(mode -> geometry().circle( m_ptCenter,m_dRadius,AcGeVector3d(-1,1,0)))
			{
				return Adesk::kTrue;
			}
			mode->subEntityTraits().setSelectionMarker(++gsIndex);
			if(mode -> geometry().circle( m_ptCenter,m_dRadius,AcGeVector3d(0,0,1)))
			{
				return Adesk::kTrue;
			}
			mode->subEntityTraits().setSelectionMarker(++gsIndex);
			if(mode -> geometry().circle( m_ptCenter - AcGeVector3d(0, 0, 1) * m_dRadius * cos(PI / 4.0),m_dRadius * sin(PI / 4.0),AcGeVector3d(0,0,1)))
			{
				return Adesk::kTrue;
			}
			mode->subEntityTraits().setSelectionMarker(++gsIndex);
			mode -> geometry().circle( m_ptCenter + AcGeVector3d(0, 0, 1) * m_dRadius *  cos(PI / 4.0),m_dRadius * sin(PI / 4.0),AcGeVector3d(0,0,1));
		}
#ifndef _ALWAYSSHOWWIRE_
		break;
	default:
		break;
	}
#endif
#endif
	return Adesk::kTrue;
}//added by linlin 20050819

/*Adesk::Boolean PDSphere::worldDraw(AcGiWorldDraw* mode)
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
int maxPrecision = getMaxPrecision(mode);
getVertices(getMaxPrecision(mode), pArray, stdIdx, actPrecision);
int actPrecision__1 = actPrecision + 1;
int stdIdxLen = stdIdx.length();
int stdIdxLen_1 = stdIdxLen - 1;

switch(mode->regenType())
{
case kAcGiStandardDisplay:
case kAcGiSaveWorldDrawForR12:
case kAcGiSaveWorldDrawForProxy:
{
int gsIndex = 0;
mode->subEntityTraits().setSelectionMarker(++gsIndex);
mode -> geometry().circle( m_ptCenter,m_dRadius,AcGeVector3d(1,0,0));
mode->subEntityTraits().setSelectionMarker(++gsIndex);
mode -> geometry().circle( m_ptCenter,m_dRadius,AcGeVector3d(1,1,0));
mode->subEntityTraits().setSelectionMarker(++gsIndex);
mode -> geometry().circle( m_ptCenter,m_dRadius,AcGeVector3d(0,1,0));
mode->subEntityTraits().setSelectionMarker(++gsIndex);
mode -> geometry().circle( m_ptCenter,m_dRadius,AcGeVector3d(-1,1,0));
mode->subEntityTraits().setSelectionMarker(++gsIndex);
mode -> geometry().circle( m_ptCenter,m_dRadius,AcGeVector3d(0,0,1));
mode->subEntityTraits().setSelectionMarker(++gsIndex);
mode -> geometry().circle( m_ptCenter - AcGeVector3d(0, 0, 1) * m_dRadius * cos(PI / 4.0),m_dRadius * sin(PI / 4.0),AcGeVector3d(0,0,1));
mode->subEntityTraits().setSelectionMarker(++gsIndex);
mode -> geometry().circle( m_ptCenter + AcGeVector3d(0, 0, 1) * m_dRadius *  cos(PI / 4.0),m_dRadius * sin(PI / 4.0),AcGeVector3d(0,0,1));
}
break;
case kAcGiHideOrShadeCommand:
case kAcGiRenderCommand:
{
AcGiEdgeData edgeData;
int edgeDataLen = actPrecision * 2 * (actPrecision__1) * 2;
Adesk::UInt8* edgeVisibility = new Adesk::UInt8[edgeDataLen];
int i, j;
for(i = 0; i < edgeDataLen; ++i)
edgeVisibility[i] = kAcGiInvisible;//kAcGiSilhouette;
int actPrecision___2 = actPrecision * 2;
for(i = 1; i < stdIdxLen_1/2; ++i)
{
for(j = 0; j < actPrecision___2; ++j)
edgeVisibility[stdIdx[i] * actPrecision___2 + j] = kAcGiVisible;
}
int PrecisionArc___Cir = actPrecision__1 * actPrecision * 2;
for(i = 0; i < stdIdxLen; ++i)
{
for(j = 0; j < actPrecision; ++j)
edgeVisibility[PrecisionArc___Cir + stdIdx[i] * actPrecision + j] = kAcGiVisible;
}	
for(i = 1; i < stdIdxLen; ++i)
{
for(j = 0; j < actPrecision; ++j)
edgeVisibility[PrecisionArc___Cir + stdIdx[stdIdxLen - 1] * actPrecision + stdIdx[i] * actPrecision + j] = kAcGiVisible;
}	
edgeData.setVisibility(edgeVisibility);

mode->geometry().mesh(actPrecision__1,actPrecision___2 + 1,pArray.asArrayPtr(),&edgeData);
delete [] edgeVisibility;
}
break;
default:
break;
}
#endif
return Adesk::kTrue;
}//added by linlin 20050819

*/
////////////////////////////////////////////////////////////////////////////////
///////		将球在圆上分成m_nPrecision段，然后求相应处圆的半径	///////
///////		再将此圆分成m_nPrecision份。								////////
///////		所以共 (m_nPrecision+1)*(m_nPrecision+1)个点				////
////////////////////////////////////////////////////////////////////////////////
Acad::ErrorStatus  PDSphere::getVertices(int Precision, AcGePoint3dArray& vertexArray, 
										 AcGeIntArray &stdIdx, int &actPrecision) const

{  assertReadEnabled();

while(!vertexArray.isEmpty())
vertexArray.removeLast();

AcGeDoubleArray dividDbl;
getActDivid(Precision, dividDbl, stdIdx);

int actPrecision__1 = dividDbl.length();
int actPrecision___2__1 = (dividDbl.length() - 1) * 2 + 1;
actPrecision = actPrecision__1 - 1;
vertexArray.setLogicalLength(actPrecision__1*actPrecision___2__1);


AcGePoint3d varPt = m_ptCenter;        //变截面的中心点
double varR = 0;
double deltaAngle = PI / Precision;

/////////////////////////////////////////////////////////////
///////		begin the division						/////////
AcGeCubicSplineCurve3d varcur;
AcGePoint3d point;

for(int i = 0; i < actPrecision__1; ++i)
{
	varPt.z=m_ptCenter.z- m_dRadius*cos(deltaAngle*dividDbl[i]);

	double tt = m_dRadius * m_dRadius - (m_ptCenter.z - varPt.z) * 
		(m_ptCenter.z - varPt.z);
	if(tt < 0)
		tt = 0;
	varR = sqrt(tt);

	int j;
	for(j = 0; j <actPrecision__1; ++j)
	{		  
		if(i == 0)
		{
			point = m_ptCenter - AcGeVector3d(0, 0, 1) * m_dRadius;
			vertexArray[i*actPrecision___2__1+j]=point;
		}
		else if(i == actPrecision__1)
		{ 
			point = m_ptCenter + AcGeVector3d(0, 0, 1) * m_dRadius;
			vertexArray[i*actPrecision___2__1+j]=point;
		}         
		else
		{ 
			point=varPt + AcGeVector3d(1, 0, 0) * varR;
			point.rotateBy(deltaAngle * dividDbl[j], AcGeVector3d(0, 0, 1), varPt);
			vertexArray[i*actPrecision__1+j]=point;
		}  	
	}
	for(j = 1; j <actPrecision__1; ++j)
	{		  
		if(i == 0)
		{
			point = m_ptCenter - AcGeVector3d(0, 0, 1) * m_dRadius;
			vertexArray[i*actPrecision__1 + j + actPrecision]=point;
		}
		else if(i == actPrecision__1)
		{ 
			point = m_ptCenter + AcGeVector3d(0, 0, 1) * m_dRadius;
			vertexArray[i*actPrecision__1 + j + actPrecision]=point;
		}         
		else
		{ 
			point=varPt + AcGeVector3d(1, 0, 0) * varR;
			point.rotateBy(deltaAngle * dividDbl[j] + PI, AcGeVector3d(0, 0, 1), varPt);
			vertexArray[i*actPrecision__1 + j + actPrecision]=point;
		}  	
	}
}

return Acad::eOk;
}
//addded by linlin 20050819
Acad::ErrorStatus  PDSphere::getVertices(int Precision, AcGePoint3dArray& vertexArray, AcGeVector3dArray &vertexNors) const
{  
	assertReadEnabled();

	int actPrecision__1 = Precision + 1;   
	int actPrecision___2__1 = Precision * 2 + 1;   
	vertexArray.setLogicalLength(actPrecision__1*actPrecision___2__1);
	vertexNors.setLogicalLength(actPrecision__1*actPrecision___2__1);


	AcGePoint3d varPt = m_ptCenter;        //变截面的中心点
	double varR = 0;
	double deltaAngle = PI / Precision;

	/////////////////////////////////////////////////////////////
	///////		begin the division						/////////
	AcGePoint3d point;

	for(int i = 0; i < actPrecision__1; ++i)
	{
		varPt.z=m_ptCenter.z- m_dRadius*cos(deltaAngle*i);

		varR = m_dRadius * sin(deltaAngle * i);
		int j;
		for(j = 0; j <actPrecision___2__1; ++j)
		{		  
			if(i == 0)
			{
				point = m_ptCenter;
				point.z -= m_dRadius;
			}
			else	 if(i == actPrecision__1)
			{ 
				point = m_ptCenter;
				point.z += m_dRadius;
			}         
			else
			{ 
				point=varPt;
				point.x += varR;
				point.rotateBy(deltaAngle * j, AcGeVector3d(0, 0, 1), varPt);
			}  	
			vertexArray[i*actPrecision___2__1+j]=point;
			vertexNors[i*actPrecision___2__1+j] = point - m_ptCenter;
		}
	}
	return Acad::eOk;
}//addded by linlin 20050819

Acad::ErrorStatus PDSphere::dwgOutFields(AcDbDwgFiler* filer)const
{
	assertReadEnabled();
	Acad::ErrorStatus es=Acad::eOk;

	// Call dwgOutFields from father class: PDPrimary3D
	if((es=PDPrimary3D::dwgOutFields(filer))!=Acad::eOk)
	{
		return es;
	}

	// Write version number.
	filer->writeItem((Adesk::UInt16) VERSION_PDSPHERE);

	// Write the data members.
	filer->writeItem(m_ptCenter);
	filer->writeItem(m_dRadius);
	filer->writeItem(m_nprecision);
	return filer->filerStatus();
}

Acad::ErrorStatus PDSphere::dwgInFields(AcDbDwgFiler* filer)
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
	if (version > VERSION_PDSPHERE)
		return Acad::eMakeMeProxy;

	// Read the data members.
	switch (version)
	{
	case (1):

		filer->readItem(&m_ptCenter);
		filer->readItem(&m_dRadius);
		filer->readItem(&m_nprecision);
		break;
	}

#ifdef _USEAMODELER_
	if(m_3dGeom.isNull())
		createBody();
#endif

	return filer->filerStatus();
}


Acad::ErrorStatus PDSphere::dxfOutFields(AcDbDxfFiler* filer) const
{
	assertReadEnabled();
	Acad::ErrorStatus es;

	if ((es = PDPrimary3D::dxfOutFields(filer))
		!= Acad::eOk)
	{
		return es;
	}

	// Write subclass marker.
	filer->writeItem(AcDb::kDxfSubclass, _T("PDSphere"));

	// Write version number.
	filer->writeItem(AcDb::kDxfInt16, (Adesk::UInt16) VERSION_PDSPHERE);

	// Write data members.
	filer->writeItem(AcDb::kDxfXCoord, m_ptCenter);
	filer->writeItem(AcDb::kDxfReal,m_dRadius);
	filer->writeItem(AcDb::kDxfInt32,m_nprecision);

	return filer->filerStatus();
}

Acad::ErrorStatus PDSphere::dxfInFields(AcDbDxfFiler* filer)
{
	assertWriteEnabled();
	Acad::ErrorStatus es = Acad::eOk;
	struct resbuf rb;

	if ((PDPrimary3D::dxfInFields(filer) != Acad::eOk)
		|| !filer->atSubclassData(_T("PDSphere")))
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
		if (version > VERSION_PDSPHERE)
			return Acad::eMakeMeProxy;
	}

	// Read the data members.
	switch (version){
	case (1):
		while ((es == Acad::eOk) && ((es = filer->readResBuf(&rb)) == Acad::eOk)){

			switch (rb.restype){
	case AcDb::kDxfXCoord:
		m_ptCenter = asPnt3d(rb.resval.rpoint);
		break;

	case AcDb::kDxfReal:
		m_dRadius=rb.resval.rreal;
		break;

	case AcDb::kDxfInt32:
		m_nprecision=rb.resval.rlong;
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

Acad::ErrorStatus PDSphere::getpointCenter(AcGePoint3d& ptcenter)
{
	assertReadEnabled();
	ptcenter = m_ptCenter;
	return Acad::eOk;
}

Acad::ErrorStatus PDSphere::getRadius(double& radius)
{
	assertReadEnabled();
	radius = m_dRadius;
	return Acad::eOk;
}

Acad::ErrorStatus PDSphere::getprecision(Adesk::UInt32& precision)
{
	assertReadEnabled();
	precision = m_dDividPrecision;
	return Acad::eOk;
}


Acad::ErrorStatus PDSphere::setParameters(AcGePoint3d ptcenter,double radius, Adesk::UInt32 precision)
{

	assertWriteEnabled();
	m_ptCenter = ptcenter ;
	m_dRadius =fabs(radius)  ;
	if(m_dRadius==0)
		m_dRadius=1;
	if(precision > 2)
		m_dDividPrecision = precision;
	else
		m_dDividPrecision = PRIMARY3D_DEFAULT_PRECISION;

#ifdef _USEAMODELER_
	createBody();
#endif

	return Acad::eOk;
}// added by linlin 20050929

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDSphere::subGetOsnapPoints(
	AcDb::OsnapMode     osnapMode,
	Adesk::GsMarker     gsSelectionMark,
	const AcGePoint3d&  pickPoint,
	const AcGePoint3d&  lastPoint,
	const AcGeMatrix3d& viewXform,
	AcGePoint3dArray&   snapPoints,
	AcDbIntArray &   geomIds) const
#else
Acad::ErrorStatus PDSphere::getOsnapPoints(
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

	if(gsSelectionMark == 0)
		return Acad::eOk;

	AcGeVector3d viewDir(viewXform(Z, 0), viewXform(Z, 1),
		viewXform(Z, 2));

	switch(osnapMode)
	{
	case AcDb::kOsModeEnd:
		break;
	case AcDb::kOsModeMid:
		break;
	case AcDb::kOsModeCen:
		snapPoints.append(m_ptCenter);
		break;
	case AcDb::kOsModeQuad:
		{
			AcGeCubicSplineCurve3d varcur;
			AcGeCubicSplineCurve3d varcurs[7];
			switch(gsSelectionMark)
			{
			case 1:
				varcur = AcGeCubicSplineCurve3d(AcGeCircArc3d(m_ptCenter, AcGeVector3d(1, 0, 0), m_dRadius));
				break;
			case 2:
				varcur = AcGeCubicSplineCurve3d(AcGeCircArc3d(m_ptCenter, AcGeVector3d(1, 1, 0), m_dRadius));
				break;
			case 3:
				varcur = AcGeCubicSplineCurve3d(AcGeCircArc3d(m_ptCenter, AcGeVector3d(0, 1, 0), m_dRadius));
				break;
			case 4:
				varcur = AcGeCubicSplineCurve3d(AcGeCircArc3d(m_ptCenter, AcGeVector3d(-1, 1, 0), m_dRadius));
				break;
			case 5:
				varcur = AcGeCubicSplineCurve3d(AcGeCircArc3d(m_ptCenter, AcGeVector3d(0, 0, 1), m_dRadius));
				break;
			case 6:
				varcur = AcGeCubicSplineCurve3d(AcGeCircArc3d(m_ptCenter - AcGeVector3d(0, 0, 1) * m_dRadius *  cos(PI / 4.0), AcGeVector3d(0, 0, 1), m_dRadius * sin(PI / 4.0)));
				break;
			case 7:
				varcur = AcGeCubicSplineCurve3d(AcGeCircArc3d(m_ptCenter + AcGeVector3d(0, 0, 1) * m_dRadius * cos(PI / 4.0), AcGeVector3d(0, 0, 1), m_dRadius * sin(PI / 4.0)));
				break;
			//定义球面象限点的捕捉方式
			//added by szw 2009.11.18 : begin
			case 8:
				varcurs[0] = AcGeCubicSplineCurve3d(AcGeCircArc3d(m_ptCenter, AcGeVector3d(1, 0, 0), m_dRadius));
				varcurs[1] = AcGeCubicSplineCurve3d(AcGeCircArc3d(m_ptCenter, AcGeVector3d(1, 1, 0), m_dRadius));
				varcurs[2] = AcGeCubicSplineCurve3d(AcGeCircArc3d(m_ptCenter, AcGeVector3d(0, 1, 0), m_dRadius));
				varcurs[3] = AcGeCubicSplineCurve3d(AcGeCircArc3d(m_ptCenter, AcGeVector3d(-1, 1, 0), m_dRadius));
				varcurs[4] = AcGeCubicSplineCurve3d(AcGeCircArc3d(m_ptCenter, AcGeVector3d(0, 0, 1), m_dRadius));
				varcurs[5] = AcGeCubicSplineCurve3d(AcGeCircArc3d(m_ptCenter - AcGeVector3d(0, 0, 1) * m_dRadius *  cos(PI / 4.0), AcGeVector3d(0, 0, 1), m_dRadius * sin(PI / 4.0)));
				varcurs[6] = AcGeCubicSplineCurve3d(AcGeCircArc3d(m_ptCenter + AcGeVector3d(0, 0, 1) * m_dRadius * cos(PI / 4.0), AcGeVector3d(0, 0, 1), m_dRadius * sin(PI / 4.0)));
				break;
			default:
				break;
			//added by szw 2009.11.18 : end
			}
			if(gsSelectionMark < 8)
			{
				double fromParam = varcur.startParam();
				double toParam = varcur.endParam();
				double delta = (toParam - fromParam) / 4.0;
				for(int i = 0; i < 4; i++)
					snapPoints.append(varcur.evalPoint(fromParam + i * delta));
			}
			//定义球面象限点的捕捉方式
			//added by szw 2009.11.18 : begin
			else if(gsSelectionMark == 8)
			{
				for(int j = 0; j < 7; ++j)
				{
					double fromParam = varcurs[j].startParam();
					double toParam = varcurs[j].endParam();
					double delta = (toParam - fromParam) / 4.0;
					for(int i = 0; i < 4; i++)
						snapPoints.append(varcurs[j].evalPoint(fromParam + i * delta));
				}
			}
			//added by szw 2009.11.18 : end
		}
		break;
	case AcDb::kOsModeNode:
		break;
	case AcDb::kOsModeIns:
		snapPoints.append(m_ptCenter);
		break;
	case AcDb::kOsModePerp:
		{
			AcGeCircArc3d cir;
			AcGeCircArc3d cirs[7];
			switch(gsSelectionMark)
			{
			case 1:
				cir.set(m_ptCenter, AcGeVector3d(1, 0, 0), m_dRadius);
				break;
			case 2:
				cir.set(m_ptCenter, AcGeVector3d(1, 1, 0), m_dRadius);
				break;
			case 3:
				cir.set(m_ptCenter, AcGeVector3d(0, 1, 0), m_dRadius);
				break;
			case 4:
				cir.set(m_ptCenter, AcGeVector3d(-1, 1, 0), m_dRadius);
				break;
			case 5:
				cir.set(m_ptCenter, AcGeVector3d(0, 0, 1), m_dRadius);
				break;
			case 6:
				cir.set(m_ptCenter - AcGeVector3d(0, 0, 1) * m_dRadius * cos(PI / 4.0), AcGeVector3d(0, 0, 1), m_dRadius *sin(PI / 4.0));
				break;
			case 7:
				cir.set(m_ptCenter + AcGeVector3d(0, 0, 1) * m_dRadius * cos(PI / 4.0), AcGeVector3d(0, 0, 1), m_dRadius * sin(PI / 4.0));
				break;
			//定义球面垂直正交点的捕捉方式
			//added by szw 2009.11.18 : begin
			case 8:
				cirs[0].set(m_ptCenter, AcGeVector3d(1, 0, 0), m_dRadius);
				cirs[1].set(m_ptCenter, AcGeVector3d(1, 1, 0), m_dRadius);
				cirs[2].set(m_ptCenter, AcGeVector3d(0, 1, 0), m_dRadius);
				cirs[3].set(m_ptCenter, AcGeVector3d(-1, 1, 0), m_dRadius);
				cirs[4].set(m_ptCenter, AcGeVector3d(0, 0, 1), m_dRadius);
				cirs[5].set(m_ptCenter - AcGeVector3d(0, 0, 1) * m_dRadius * cos(PI / 4.0), AcGeVector3d(0, 0, 1), m_dRadius *sin(PI / 4.0));
				cirs[6].set(m_ptCenter + AcGeVector3d(0, 0, 1) * m_dRadius * cos(PI / 4.0), AcGeVector3d(0, 0, 1), m_dRadius * sin(PI / 4.0));
				break;
			default:
				break;
			//added by szw 2009.11.18 : end
			}
			if(gsSelectionMark < 8)
			{
				AcGePoint3d pt;
				pt = cir.closestPointTo(lastPoint);
				snapPoints.append(pt);
			}
			//定义球面垂直正交点的捕捉方式
			//added by szw 2009.11.18 : begin
			else if(gsSelectionMark == 8)
			{
				for(int i = 0; i < 7; ++i)
				{
					AcGePoint3d pt;
					pt = cirs[i].closestPointTo(lastPoint);
					snapPoints.append(pt);
				}
			}
			//added by szw 2009.11.18 : end
		}
		break;
	case AcDb::kOsModeTan:
		break;
	case AcDb::kOsModeNear:
		{
			AcGePoint3d pt;
			AcGeCircArc3d cir;
			AcGeCircArc3d cirs[7];
			switch(gsSelectionMark)
			{
			case 1:
				cir.set(m_ptCenter, AcGeVector3d(1, 0, 0), m_dRadius);
				break;
			case 2:
				cir.set(m_ptCenter, AcGeVector3d(1, 1, 0), m_dRadius);
				break;
			case 3:
				cir.set(m_ptCenter, AcGeVector3d(0, 1, 0), m_dRadius);
				break;
			case 4:
				cir.set(m_ptCenter, AcGeVector3d(-1, 1, 0), m_dRadius);
				break;
			case 5:
				cir.set(m_ptCenter, AcGeVector3d(0, 0, 1), m_dRadius);
				break;
			case 6:
				cir.set(m_ptCenter - AcGeVector3d(0, 0, 1) * m_dRadius * cos(PI / 4.0), AcGeVector3d(0, 0, 1), m_dRadius *sin(PI / 4.0));
				break;
			case 7:
				cir.set(m_ptCenter + AcGeVector3d(0, 0, 1) * m_dRadius * cos(PI / 4.0), AcGeVector3d(0, 0, 1), m_dRadius * sin(PI / 4.0));
				break;
			//定义球面垂直正交点的捕捉方式
			//added by szw 2009.11.18 : begin
			case 8:
				cirs[0].set(m_ptCenter, AcGeVector3d(1, 0, 0), m_dRadius);
				cirs[1].set(m_ptCenter, AcGeVector3d(1, 1, 0), m_dRadius);
				cirs[2].set(m_ptCenter, AcGeVector3d(0, 1, 0), m_dRadius);
				cirs[3].set(m_ptCenter, AcGeVector3d(-1, 1, 0), m_dRadius);
				cirs[4].set(m_ptCenter, AcGeVector3d(0, 0, 1), m_dRadius);
				cirs[5].set(m_ptCenter - AcGeVector3d(0, 0, 1) * m_dRadius * cos(PI / 4.0), AcGeVector3d(0, 0, 1), m_dRadius *sin(PI / 4.0));
				cirs[6].set(m_ptCenter + AcGeVector3d(0, 0, 1) * m_dRadius * cos(PI / 4.0), AcGeVector3d(0, 0, 1), m_dRadius * sin(PI / 4.0));
				break;
			default:
				break;
			//added by szw 2009.11.18 : end
			}
			if(gsSelectionMark < 8)
			{
				pt = cir.projClosestPointTo(pickPoint, viewDir);
				snapPoints.append(pt);
			}
			//定义球面垂直正交点的捕捉方式
			//added by szw 2009.11.18 : begin
			else if(gsSelectionMark == 8)
			{
				for(int i = 0; i < 7; ++i)
				{
					pt = cirs[i].projClosestPointTo(pickPoint, viewDir);
					snapPoints.append(pt);
				}
			}
			//added by szw 2009.11.18 : end
		}
		break;
	default:
		break;
	}

	return Acad::eOk;
}


#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDSphere::subGetGripPoints(
	AcGePoint3dArray& gripPoints,
	AcDbIntArray& osnapModes,
	AcDbIntArray& geomIds) const
#else
Acad::ErrorStatus PDSphere::getGripPoints(
	AcGePoint3dArray& gripPoints,
	AcDbIntArray& osnapModes,
	AcDbIntArray& geomIds) const
#endif
{
	assertReadEnabled();
	/*	getgrippoints(gripPoints);*/
	return Acad::eOk; 
}

Acad::ErrorStatus PDSphere::getgrippoints(AcGePoint3dArray& gripArray) const
{
	assertReadEnabled();
	gripArray.append(m_ptCenter);

	gripArray.append(m_ptCenter + AcGeVector3d(1,0,0)*m_dRadius);
	gripArray.append(m_ptCenter + AcGeVector3d(0,1,0)*m_dRadius);
	gripArray.append(m_ptCenter + AcGeVector3d(0,0,1)*m_dRadius);

	gripArray.append(m_ptCenter + AcGeVector3d(-1,0,0)*m_dRadius);
	gripArray.append(m_ptCenter + AcGeVector3d(0,-1,0)*m_dRadius);
	gripArray.append(m_ptCenter + AcGeVector3d(0,0,-1)*m_dRadius);

	return Acad::eOk; 
}

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDSphere::subMoveGripPointsAt(
	const AcDbIntArray& indices,
	const AcGeVector3d& offset)
#else
Acad::ErrorStatus PDSphere::moveGripPointsAt(
	const AcDbIntArray& indices,
	const AcGeVector3d& offset)
#endif
{
	assertWriteEnabled();
	/*	if(indices[0] !=0){
	AcGePoint3dArray grippoint;
	getgrippoints(grippoint);
	grippoint[indices[0]].transformBy(offset);
	double r = grippoint[indices[0]].distanceTo(m_ptCenter);
	m_dRadius =r;
	}
	else
	m_ptCenter.transformBy(offset);
	*/

	return Acad::eOk;
}

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDSphere::subExplode(AcDbVoidPtrArray& entitySet) const
#else
Acad::ErrorStatus PDSphere::explode(AcDbVoidPtrArray& entitySet) const
#endif
{
	assertReadEnabled();

	AcDb3dSolid *pBody;
	pBody = new AcDb3dSolid;
	if(pBody->createSphere(m_dRadius) != Acad::eOk)
	{
		delete pBody;
		return Acad::eNotApplicable;
	}

	AcGeMatrix3d mat;
	mat.setTranslation(m_ptCenter.asVector());

	pBody->transformBy(mat);

	pBody->setPropertiesFrom(this);
	entitySet.append(pBody);

	return Acad::eOk;
}

//  [9/5/2007 suzhiyong]
Acad::ErrorStatus PDSphere::explodeTo3DSolid(AcDb3dSolid* &p3dSolid) const
{
	assertReadEnabled();

	AcDb3dSolid *pBody;
	pBody = new AcDb3dSolid;
	if(pBody->createSphere(m_dRadius) != Acad::eOk)
	{
		delete pBody;
		return Acad::eNotApplicable;
	}

	AcGeMatrix3d mat;
	mat.setTranslation(m_ptCenter.asVector());

	pBody->transformBy(mat);

	pBody->setPropertiesFrom(this);
	p3dSolid = pBody;

	return Acad::eOk;
}

void PDSphere::createBody()
{
	assertWriteEnabled();
	//#ifdef _OBJECTARX2004_
	m_3dGeom = Body::sphere(*(Point3d*)&m_ptCenter, m_dRadius, m_dDividPrecision);
	//#endif
}

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDSphere::subGetGeomExtents(AcDbExtents& extents) const
#else
Acad::ErrorStatus PDSphere::getGeomExtents(AcDbExtents& extents) const
#endif
{
	assertReadEnabled();
	extents.set(m_ptCenter - AcGeVector3d(1, 1, 1) * m_dRadius, 
		m_ptCenter + AcGeVector3d(1, 1, 1) * m_dRadius);
	return Acad::eOk;
}

#ifdef _OBJECTARX2010_
void PDSphere::subList() const
#else
void PDSphere::list() const
#endif
{
	assertReadEnabled();
#ifdef _OBJECTARX2010_
	PDPrimary3D::subList();
#else
	PDPrimary3D::list();
#endif
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4776/*"\n 球心(%f,%f,%f)\n"*/,m_ptCenter.x,m_ptCenter.y,m_ptCenter.z);   
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4777/*" 球半径=%.3f\n"*/,m_dRadius);
	return ;
}// added by linlin 20050810



/*---------------------------------------------------------------------------
* 名称: _make_eqovalpt
* 功能: 完成 "在设备椭球封头上画出点" 的功能
* 注意: 无
*/
int  PDSphere::_make_eqovalpt(double lengthR,AcGeVector3d vec,ads_point pt,ads_point normal,AcGeVector3d v) 
{

	if(lengthR >m_dRadius || lengthR<=0)
		return RTERROR;


	AcGePoint3d  point,center1;
	AcGeVector3d Lnormal;
	double z;
	z=sqrt(m_dRadius*m_dRadius-lengthR*lengthR);

	center1=m_ptCenter+v*z;

	point =center1+vec*lengthR;
	Lnormal=(point-m_ptCenter).normalize();

	pt[0]=point.x;
	pt[1]=point.y;
	pt[2]=point.z;

	normal[0]=Lnormal.x;
	normal[1]=Lnormal.y;
	normal[2]=Lnormal.z;


	return RTNORM;

}//added by linlin 20051013
