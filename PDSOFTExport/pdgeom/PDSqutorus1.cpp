// PDSqutorus1.cpp: implementation of the PDSqutorus1 class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "language25.h"
#include "PDSqutorus1.h"
#include "dbproxy.h"
#include "acgi.h"
#include "math.h"
#include "gecspl3d.h"
#include "geassign.h"
#include <GEINTARR.H>
#include <dbents.h>
#include <dbregion.h>
#include <dbsol3d.h>
#include <aced.h>


#ifdef _OBJECTARX2000_
#include <migrtion.h>
#include <dbapserv.h>
#include <dbxutil.h>
#else
#include <dbautil.h>
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define VERSION_PDSQUTORUS1 3
#define PI 3.1415926535897932384626433832795L
const double SIGMA = 0.01;
#ifdef _OBJECTARX2000_
ACRX_DXF_DEFINE_MEMBERS(PDSqutorus1, PDPrimary3D,
						AcDb::kDHL_CURRENT, AcDb::kMReleaseCurrent,
						AcDbProxyEntity::kAllAllowedBits,
						PDSQUTORUS1, Gle);
#else
ACRX_DXF_DEFINE_MEMBERS(PDSqutorus1, PDPrimary3D, AcDbProxyEntity::kAllAllowedBits, PDSQUTORUS1, Gle);
#endif

//只要有一个数据不对，均返回false
bool PDSqutorus1::isValidData(double &f)
{
	if(!PDPrimary3D::isValidData(f))
		return false;
	if(m_dDividPrecision >= 10000) //剖分精度需要小于10000
		return false;
	if(!isValidFloat(f=m_dLength1))
		return false;
	if(!isValidFloat(f=m_dLength2))
		return false;
	if(!isValidFloat(f=m_dWidth1))
		return false;
	if(!isValidFloat(f=m_dWidth2))
		return false;
	if(!isValidFloat(f=m_dDiameter))
		return false;
	if(!isValidFloat(f=m_dAngle))
		return false;
	if(!isValidFloat(f=m_ptP.x) || !isValidFloat(f=m_ptP.y) || !isValidFloat(f=m_ptP.z))
		return false;
	if(!isValidFloat(f=m_ptP1.x) || !isValidFloat(f=m_ptP1.y) || !isValidFloat(f=m_ptP1.z))
		return false;
	if(!isValidFloat(f=m_normalP2.x) || !isValidFloat(f=m_normalP2.y) || !isValidFloat(f=m_normalP2.z))
		return false;
	if(!isValidFloat(f=m_v3.x) || !isValidFloat(f=m_v3.y) || !isValidFloat(f=m_v3.z))
		return false;
	return true;
}
void PDSqutorus1::setDefault()
{
	PDPrimary3D::setDefault();
	m_dDividPrecision = PRIMARY3D_DEFAULT_PRECISION;
#if defined(NDEBUG) || !defined(_DEBUG)
	m_dLength1 = m_dLength2 = m_dWidth1 = m_dWidth2 = m_dDiameter = 1.0;
	m_ptP= m_ptP1 = AcGePoint3d();
	m_normalP2.set(0, 0, 1);
	m_v3.set(0,0,0);
	m_dAngle = 0;
#else
	m_dLength1 = m_dLength2 = m_dWidth1 = m_dWidth2 = m_dDiameter = 1000;
	m_ptP.set(0,0,0);
	m_ptP1.set(1000,0,0);
	m_normalP2.set(0, 0, 1);
	m_v3.set(0,0,0);
	m_dAngle = 60;
#endif
}

//zxb,20090113,增加audit，暂时只check自身成员数据
Acad::ErrorStatus PDSqutorus1::audit(AcDbAuditInfo* pAuditInfo)
{
	return auditPDPrimary3D(this,pAuditInfo,_T("PDSqutorus1"));
}

PDSqutorus1::PDSqutorus1(bool HasSnap/* = false*/) : PDPrimary3D(HasSnap)
{
	m_dDividPrecision = PRIMARY3D_DEFAULT_PRECISION;
	m_dLength1 = m_dLength2 = m_dWidth1 = m_dWidth2 = m_dDiameter = 1.0;
	m_ptP= m_ptP1 = AcGePoint3d();
	m_normalP2.set(0, 0, 1);
	m_v3.set(0,0,0);
	m_dAngle = 0;
#ifdef _USEAMODELER_
	createBody();
#endif
}

PDSqutorus1::~PDSqutorus1()
{

}

PDSqutorus1::PDSqutorus1( AcGePoint3d p, AcGePoint3d p1, AcGeVector3d normalP2,
						 double l1,double w1,double l2,double w2,
						 double angle, int n/* = 20*/, bool HasSnap/* = false*/,int CorrType/*0-左手系，1-右手系*/) : PDPrimary3D(HasSnap)
{
	if(angle < 0)
		angle = -angle;
	while(angle > 360.0)
		angle -= 360.0;
	if(angle==0)
		angle=1;
	m_dAngle = angle * PI / 180.0;

	m_dLength1 =fabs(l1);
	if(m_dLength1==0)
		m_dLength1=1;

	m_dWidth1 =fabs(w1) ;
	if(m_dWidth1==0)
		m_dWidth1=1;

	//	if( fabs(angle) <360 ){
	m_dLength2 =fabs(l2) ;
	if(m_dLength2==0)
		m_dLength2=1;

	m_dWidth2 =fabs(w2) ;
	if(m_dWidth2==0)
		m_dWidth2=1;
	//	}
	//	else{
	//		m_dAngle = PI*2;
	//		m_dLength2 = m_dLength1;
	//		m_dWidth2 = m_dWidth1;
	//	}

	m_ptP = p;
	m_ptP1 = p1;
	m_normalP2 = normalP2;


	AcGeVector3d p0p1,facevect;
	p0p1 = p1-p;
	facevect=normalP2;
	if(p0p1.isZeroLength() && facevect.isZeroLength())
	{
		m_ptP1 = p + AcGeVector3d(1, 0, 0);
		m_normalP2.set(0,0,1);
		facevect=m_normalP2;
	}
	else if(facevect.isZeroLength())
	{
		AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax, Ay;
		if (fabs(p0p1[X]) <1.0/64 && fabs(p0p1[Y])<1.0/64) 
			Ax = Wy.crossProduct(p0p1);
		else
			Ax = Wz.crossProduct(p0p1);
		Ax.normalize();

		facevect = Ax;
		m_normalP2=facevect;
	}
	else if(p0p1.isZeroLength())
	{
		AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax, Ay;
		if (fabs(facevect[X]) <1.0/64 && fabs(facevect[Y])<1.0/64) 
			Ax = Wy.crossProduct(facevect);
		else
			Ax = Wz.crossProduct(facevect);
		Ax.normalize();

		p0p1 = Ax;
		m_ptP1 = p+ p0p1;
	}
	else if(p0p1.isParallelTo(facevect))
	{
		AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax, Ay;
		if (fabs(facevect[X]) <1.0/64 && fabs(facevect[Y])<1.0/64) 
			Ax = Wy.crossProduct(facevect);
		else
			Ax = Wz.crossProduct(facevect);
		Ax.normalize();

		p0p1 = Ax *p0p1.length();
		m_ptP1 = p + p0p1;
	}


	double startDist, endDist;
	startDist = m_ptP1.distanceTo(p);
	AcGeVector3d startVect = (m_ptP1 -  p).normal();
	double r1 = l1/ 2.0;
	double r2 = l2/2.0;

	if (startDist<r1)
		m_ptP1=p+startVect*r1;

	AcGePoint3d P2=m_ptP1;
	P2= P2.rotateBy(angle,facevect,p);	
	AcGeVector3d endVect = (P2 - p).normal();
	endDist=P2.distanceTo(p);

	if (endDist<r2)
		m_dLength2=endDist*2.0;

	m_dDiameter = m_ptP1.distanceTo(m_ptP) * 2.0;

	if(n > 2)
		m_dDividPrecision = n;
	else
		m_dDividPrecision = PRIMARY3D_DEFAULT_PRECISION;

	AcGeVector3d v1,v2;
	v1 = (m_ptP1-m_ptP).normal();
	v2 = getNormalP2();
	if(CorrType)
		m_v3 = v1.crossProduct(v2).normal();
	else
		m_v3 = v2.crossProduct(v1).normal();

#ifdef _USEAMODELER_
	createBody();
#endif
}

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDSqutorus1::subTransformBy(const AcGeMatrix3d& xform)
#else
Acad::ErrorStatus PDSqutorus1::transformBy(const AcGeMatrix3d& xform)
#endif
{
	assertWriteEnabled();
	// TODO: implement this function.
	m_ptP.transformBy(xform);
	m_ptP1.transformBy(xform);
	m_normalP2.transformBy(xform);
	m_v3.transformBy(xform);

	m_dLength1 *= xform.scale();
	m_dLength2 *= xform.scale();
	m_dWidth1 *= xform.scale();
	m_dWidth2 *= xform.scale();
	m_dDiameter *= xform.scale();

#ifdef _OBJECTARX2010_
	return PDPrimary3D::subTransformBy(xform);
#else
	return PDPrimary3D::transformBy(xform);
#endif
}

#ifdef _OBJECTARX2010_
Adesk::Boolean PDSqutorus1::subWorldDraw(AcGiWorldDraw* mode)
#else
Adesk::Boolean PDSqutorus1::worldDraw(AcGiWorldDraw* mode)
#endif
{
	assertReadEnabled();
	//if(acdbHostApplicationServices()->workingDatabase()->useri1() == 1)
	//	return Adesk::kTrue;

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
	int PreStep = 1;	
	AcGePoint3dArray pArray;
	int actPrecision__1 = Precision + 1;

	Adesk::Int16 nRating = getCalMaxPrecision();
	if(nRating == PRIMARY3D_SIMPLIZE_RATE1 ||
		nRating == PRIMARY3D_SIMPLIZE_RATE2)
	{
		Precision = (int)(PI / acos(1.0 - mode->deviation(kAcGiMaxDevForCircle, m_ptP1) * 2/ m_dDiameter));
		if(Precision < PRIMARY3D_SIMPLIZE_SCALE)
		{
			if (nRating == PRIMARY3D_SIMPLIZE_RATE2)
			{
				//二级简化
				if(fabs(m_dAngle - 2 *PI) < 0.0001)
				{
					mode->geometry().circle(m_ptP,m_dDiameter/2.0,m_normalP2);
				}
				else
				{
					mode->geometry().circularArc(m_ptP,m_dDiameter/2.0,m_normalP2,m_ptP1 - m_ptP,m_dAngle);
				}
			}
			else if (nRating == PRIMARY3D_SIMPLIZE_RATE1)
			{
				//一级简化
				Precision = PRIMARY3D_WIREFRAME_LINENUM4;
				getVertices(Precision, pArray);
				actPrecision__1 = Precision + 1;

				AcGeVector3d faceVect;
				faceVect.set(m_normalP2.x, m_normalP2.y, m_normalP2.z).normalize();
				AcGePoint3d pts[2];
				int i = 0;
				for(i = 0; i < actPrecision__1; i += PreStep)
				{
					for(int j = 0; j < 4; j++)
					{       
						pts[0] = pArray[i + actPrecision__1  * j];
						if (j==3)
							pts[1] = pArray[i];
						else
							pts[1] = pArray[i + actPrecision__1  * (j + 1)];
						if(mode->geometry().polyline(2, pts))
							return Adesk::kTrue;
					}
				}
				int nIndex = 0;
				if(m_dAngle <= PI * 2.0 + 0.0001 && m_dAngle >= PI * 2.0 - 0.0001 && m_dLength1 == m_dLength2 && m_dWidth1 == m_dWidth2)
				{
					for(i = 0; i < 4; i++)
					{
						nIndex = i*actPrecision__1;
						if(mode->geometry().circle(pArray[nIndex], pArray[1 +nIndex], pArray[2 +nIndex]))
							return Adesk::kTrue;
					}
				}
				else
				{
					if(m_dLength1 == m_dLength2 && m_dWidth1 == m_dWidth2)
					{
						for( i = 0; i < 4; i++)
						{
							nIndex = i * actPrecision__1;
							if(mode->geometry().circularArc(pArray[nIndex], pArray[nIndex + 1], pArray[nIndex + Precision]))
								return Adesk::kTrue;
						}
					}
					else
					{	
						AcGePoint3dArray pts;
						for(i= 0; i < 4;i++)
						{
							pts.setLogicalLength(0);
							for( int j = 0; j <= Precision; j++)
								pts.append(pArray[i * (actPrecision__1) + j]);
							if(mode->geometry().polyline(actPrecision__1, pts.asArrayPtr()))
								return Adesk::kTrue;
						}
					}
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

			AcGePoint3d pts[4];

			///////////////////////////////////////////////////////////////
			////////	draw the start and end faces	///////////////////
			//添加实体模型各面的GS标记
			//added by szw 2009.11.18 : begin
			int gsIndex = 0;
			//added by szw 2009.11.18 : end
			if(m_dAngle <= PI * 2.0 + 0.0001 && m_dAngle >= PI * 2.0 - 0.0001 && m_dLength1 == m_dLength2 && m_dWidth1 == m_dWidth2)
				//圆环的情况，使gsIndex保持一致
				//added by szw 2009.11.18 : begin
				gsIndex += PRIMARY3D_WIREFRAME_LINENUM + 1;
				//added by szw 2009.11.18 : end
			else
			{
				pts[0] = pArray[0];
				pts[1] = pArray[actPrecision__1];
				pts[2] = pArray[actPrecision__1*2];
				pts[3] = pArray[actPrecision__1*3];
				//added by szw 2009.11.18 : begin
				mode->subEntityTraits().setSelectionMarker(++gsIndex);
				//added by szw 2009.11.18 : end
				if(mode->geometry().polygon(4, pts))
					return Adesk::kTrue;
				pts[0] = pArray[Precision];
				pts[1] = pArray[Precision + actPrecision__1];
				pts[2] = pArray[Precision + actPrecision__1*2];
				pts[3] = pArray[Precision + actPrecision__1*3];
				gsIndex += PRIMARY3D_WIREFRAME_LINENUM;
				//added by szw 2009.11.18 : begin
				mode->subEntityTraits().setSelectionMarker(gsIndex);
				//added by szw 2009.11.18 : end
				if(mode->geometry().polygon(4, pts))
					return Adesk::kTrue;
			}

			///////////////////////////////////////////////////////////////////////////
			///////		use the mesh function to show the four surround faces	///////
			//////		have to use mesh individal for the each face		///////////

			for(int j= 0; j < 4; j++)
			{
				AcGiEdgeData edgeData;
				int edgeDataLen = actPrecision__1 + 2 * Precision;
				Adesk::UInt8* edgeVisibility = new Adesk::UInt8[edgeDataLen];

				for(int k = 0; k< edgeDataLen; ++k)                          
				{	 
					if(k < 2 * Precision)
						edgeVisibility[k] = kAcGiVisible;//kAcGiSilhouette;
					else
						edgeVisibility[k] = kAcGiInvisible;

					int actPrecision___2 = Precision * 2;			  
					for(int i = 0; i < Precision; i += PreStep)
						edgeVisibility[i +actPrecision___2] = kAcGiVisible;

				}
				edgeData.setVisibility(edgeVisibility);
				//added by szw 2009.11.18 : begin
				mode->subEntityTraits().setSelectionMarker(++gsIndex);
				//added by szw 2009.11.18 : end
				if(mode->geometry().mesh(2, actPrecision__1 , &pArray[j * (actPrecision__1 )], &edgeData))
				{
					delete [] edgeVisibility;
					return Adesk::kTrue;
				}
				delete [] edgeVisibility;
			}

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

			AcGeVector3d faceVect;
			faceVect.set(m_normalP2.x, m_normalP2.y, m_normalP2.z).normalize();

			int gsIndex = 0;
			AcGePoint3d pts[2];
			int i = 0;
			//统一线框模型和实体模型的GS标记
			//modified by szw 2009.11.18 : begin
			for(i = 0; i < actPrecision__1; i += PreStep)
			{
				for(int j = 0; j < 4; j++)
				{       
					pts[0] = pArray[i + actPrecision__1  * j];
					if (j==3)
						pts[1] = pArray[i];
					else
						pts[1] = pArray[i + actPrecision__1  * (j + 1)];
					mode->subEntityTraits().setSelectionMarker(++gsIndex);
					if(mode->geometry().polyline(2, pts))
						return Adesk::kTrue;
				}
			}
			int nIndex = 0;
			if(m_dAngle <= PI * 2.0 + 0.0001 && m_dAngle >= PI * 2.0 - 0.0001 && m_dLength1 == m_dLength2 && m_dWidth1 == m_dWidth2)
			{
				for(i = 0; i < 4; i++)
				{
					mode->subEntityTraits().setSelectionMarker(++gsIndex);
					nIndex = i*actPrecision__1;
					if(mode->geometry().circle(pArray[nIndex], pArray[3 +nIndex], pArray[6 +nIndex]))
						return Adesk::kTrue;
				}
			}
			else
			{
				if(m_dLength1 == m_dLength2 && m_dWidth1 == m_dWidth2)
				{
					for( i = 0; i < 4; i++)
					{
						nIndex = i * actPrecision__1;
						mode->subEntityTraits().setSelectionMarker(++gsIndex);
						if(mode->geometry().circularArc(pArray[nIndex], pArray[nIndex + 4], pArray[nIndex + Precision]))
							return Adesk::kTrue;
					}
				}
				else
				{	
					AcGePoint3dArray pts;
					for(i= 0; i < 4;i++)
					{
						pts.setLogicalLength(0);
						for( int j = 0; j <= Precision; j++)
							pts.append(pArray[i * (actPrecision__1) + j]);
						mode->subEntityTraits().setSelectionMarker(++gsIndex);
						if(mode->geometry().polyline(actPrecision__1, pts.asArrayPtr()))
							return Adesk::kTrue;
					}
				}
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

/*Adesk::Boolean PDSqutorus1::worldDraw(AcGiWorldDraw* mode)
{
assertReadEnabled();
if (mode->regenAbort()) {
return Adesk::kTrue;
}
#ifdef _USEAMODELER_
PDPrimary3D::worldDraw(mode);
#else
AcGePoint3dArray pArray;
AcGeIntArray stdIdx;
int actPrecision;
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
AcGeVector3d faceVect;
faceVect.set(m_normalP2.x, m_normalP2.y, m_normalP2.z).normalize();

int gsIndex = 0;
for(int i = 0; i < stdIdxLen; i++)
{
AcGePoint3d pts[2];
for(int j = 0; j < 4; j++)
{       
pts[0] = pArray[stdIdx[i]+ actPrecision__1  * j];
if (j==3)
pts[1] = pArray[stdIdx[i]];
else
pts[1] = pArray[stdIdx[i]+ actPrecision__1  * (j + 1)];
mode->subEntityTraits().setSelectionMarker(++gsIndex);
mode->geometry().polyline(2, pts);
}
}

if(m_dAngle <= PI * 2.0 + 0.0001 && m_dAngle >= PI * 2.0 - 0.0001 && m_dLength1 == m_dLength2 && m_dWidth1 == m_dWidth2)
{
for(i = 0; i < 4; i++)
{
mode->subEntityTraits().setSelectionMarker(++gsIndex);
mode->geometry().circle(pArray[stdIdx[0]+i*actPrecision__1], pArray[stdIdx[3]+i*actPrecision__1], pArray[stdIdx[6]+i*actPrecision__1]);
}
}
else
{
if(m_dLength1 == m_dLength2 && m_dWidth1 == m_dWidth2)
{
for( i = 0; i < 4; i++)
{
mode->subEntityTraits().setSelectionMarker(++gsIndex);
mode->geometry().circularArc(pArray[stdIdx[0]+i*actPrecision__1], pArray[stdIdx[4]+i*actPrecision__1], pArray[stdIdx[stdIdxLen_1]+i*actPrecision__1]);
}
}
else
{
for(int i= 0; i < 4;i++)
{
AcGePoint3dArray pts;
for( int j = 0; j <= actPrecision; j++)
pts.append(pArray[i * (actPrecision__1) + j]);
mode->subEntityTraits().setSelectionMarker(++gsIndex);
mode->geometry().polyline(actPrecision__1, pts.asArrayPtr());
}
}
}
}
break;
case kAcGiHideOrShadeCommand:
case kAcGiRenderCommand:
{		  
mode->subEntityTraits().setFillType(kAcGiFillAlways);
AcGePoint3d pts[4];

///////////////////////////////////////////////////////////////
////////	draw the start and end faces	///////////////////
if(m_dAngle <= PI * 2.0 + 0.0001 && m_dAngle >= PI * 2.0 - 0.0001 && m_dLength1 == m_dLength2 && m_dWidth1 == m_dWidth2)
;
else
{

pts[0] = pArray[stdIdx[0]];
pts[1] = pArray[stdIdx[0]+actPrecision__1];
pts[2] = pArray[stdIdx[0]+actPrecision__1*2];
pts[3] = pArray[stdIdx[0]+actPrecision__1*3];
mode->geometry().polygon(4, pts);
pts[0] = pArray[stdIdx[stdIdxLen_1]];
pts[1] = pArray[stdIdx[stdIdxLen_1]+actPrecision__1];
pts[2] = pArray[stdIdx[stdIdxLen_1]+actPrecision__1*2];
pts[3] = pArray[stdIdx[stdIdxLen_1]+actPrecision__1*3];
mode->geometry().polygon(4, pts);
}

///////////////////////////////////////////////////////////////////////////
///////		use the mesh function to show the four surround faces	///////
//////		have to use mesh individal for the each face		///////////

for(int j= 0; j < 4; j++)
{
AcGiEdgeData edgeData;
int edgeDataLen = actPrecision+ 1 + 2 * actPrecision;
Adesk::UInt8* edgeVisibility = new Adesk::UInt8[edgeDataLen];

for(int k = 0; k< edgeDataLen; ++k)                          
{	 
if(k < 2 * actPrecision)
edgeVisibility[k] = kAcGiVisible;//kAcGiSilhouette;
else
edgeVisibility[k] = kAcGiInvisible;

int actPrecision___2 = actPrecision * 2;			  
for(int i = 0; i < stdIdxLen; ++i)
edgeVisibility[stdIdx[i] +actPrecision___2] = kAcGiVisible;

}
edgeData.setVisibility(edgeVisibility);

mode->geometry().mesh(2, actPrecision__1 , &pArray[j * (actPrecision__1 )], &edgeData);
delete [] edgeVisibility;
}

}
break;
default:
break;
}
#endif
return Adesk::kTrue;
}*/
//added by linlin 20050816


///////////////////////////////////////////////////////////////////////
//////	先将圆弧在角度方向上分成m_nN段							///////
//////	矩形的长度和宽度随角度递增线性变化，求出每段矩形长宽	///////
//////	再求出矩形的5个顶点，加入到输出数组中					///////
/////	得到5个棱线上的m_nN个离散点，共5*m_nN个点				///////
////////////////////////////////////////////////////////////////////////
Acad::ErrorStatus PDSqutorus1::getVertices(int Precision, AcGePoint3dArray& vertexArray, 
										   AcGeIntArray &stdIdx, int &actPrecision) const
{
	assertReadEnabled();

	while(!vertexArray.isEmpty())
		vertexArray.removeLast();

	AcGeDoubleArray dividDbl;
	getActDivid(Precision, dividDbl, stdIdx);

	int actPrecision__1 = dividDbl.length();
	actPrecision = actPrecision__1 - 1;

	AcGeVector3d Facevect;
	Facevect.set(m_normalP2.x, m_normalP2.y, m_normalP2.z).normalize();

	double deltaL = (m_dLength2 - m_dLength1) /Precision;
	double deltaW = (m_dWidth2 - m_dWidth1) / Precision;

	AcGeVector3d v1,v2;
	v1 = (m_ptP1 - m_ptP).normal();
	v2 = getNormalP2();
	if(isLeftCoord(v1,v2,m_v3))
		Facevect *= -1;

	AcGePoint3dArray vertexArray1,vertexArray2,vertexArray3,vertexArray4;
	for(int i = 0; i <= actPrecision; i++)
	{
		AcGePoint3d varCenter = m_ptP1;
		double varL,varW;
		AcGePoint3d point, point1;

		varCenter.rotateBy(m_dAngle / Precision*dividDbl[i], Facevect, m_ptP);
		AcGeVector3d curvect = AcGeVector3d(varCenter-m_ptP).crossProduct(Facevect).normal();
		AcGeVector3d Centervect= (varCenter-m_ptP).normal();
		varL =m_dLength1 + ( dividDbl[i]*deltaL);
		varW =m_dWidth1 + ( dividDbl[i]*deltaW);

		point = varCenter - (Centervect * varL / 2.0) + (Facevect * varW / 2.0);
		point1 = point;
		vertexArray.append(point);

		point -= Facevect * varW;
		vertexArray1.append(point);

		point += Centervect * varL;
		vertexArray2.append(point);

		point += Facevect * varW;
		vertexArray3.append(point);

		vertexArray4.append(point1);
	}

	//////////////////////////////////////////////////////
	//////	将5个棱线离散点合并到输出数组中

	vertexArray.append(vertexArray1);
	vertexArray.append(vertexArray2);
	vertexArray.append(vertexArray3);
	vertexArray.append(vertexArray4);

	return Acad::eOk;
}
//added  by linlin 20050816
Acad::ErrorStatus PDSqutorus1::getVertices(int Precision, AcGePoint3dArray& vertexArray) const
{
	assertReadEnabled();
	vertexArray.setLogicalLength(0);

	AcGeVector3d Facevect;
	Facevect.set(m_normalP2.x, m_normalP2.y, m_normalP2.z).normalize();

	double deltaL = (m_dLength2 - m_dLength1) /Precision;
	double deltaW = (m_dWidth2 - m_dWidth1) / Precision;

	AcGeVector3d v1,v2;
	v1 = (m_ptP1 - m_ptP).normal();
	v2 = getNormalP2();
	if(isLeftCoord(v1,v2,m_v3))
		Facevect *= -1;

	AcGePoint3dArray vertexArray1,vertexArray2,vertexArray3,vertexArray4;
	AcGePoint3d varCenter = m_ptP1;
	double varL,varW;
	AcGePoint3d point, point1;
	double tmpAngle = m_dAngle / Precision;
	AcGeVector3d Centervect,curvect;
	varL = m_dLength1;
	varW = m_dWidth1;
	for(int i = 0; i <= Precision; i++)
	{
		Centervect= (varCenter-m_ptP).normal();
		curvect = Centervect.crossProduct(Facevect).normal();

		point = varCenter - (Centervect * varL / 2.0) + (Facevect * varW / 2.0);
		point1 = point;
		vertexArray.append(point);

		point -= Facevect * varW;
		vertexArray1.append(point);

		point += Centervect * varL;
		vertexArray2.append(point);

		point += Facevect * varW;
		vertexArray3.append(point);

		vertexArray4.append(point1);

		varCenter.rotateBy(tmpAngle, Facevect, m_ptP);
		varL +=deltaL;
		varW +=deltaW;
	}

	//////////////////////////////////////////////////////
	//////	将5个棱线离散点合并到输出数组中

	vertexArray.append(vertexArray1);
	vertexArray.append(vertexArray2);
	vertexArray.append(vertexArray3);
	vertexArray.append(vertexArray4);

	return Acad::eOk;
}

Acad::ErrorStatus PDSqutorus1::dwgOutFields(AcDbDwgFiler* filer)const
{
	assertReadEnabled();
	Acad::ErrorStatus es=Acad::eOk;

	// Call dwgOutFields from father class: PDPrimary3D
	if((es=PDPrimary3D::dwgOutFields(filer))!=Acad::eOk)
	{
		return es;
	}

	// Write version number.
	filer->writeItem((Adesk::UInt16) VERSION_PDSQUTORUS1);

	// version 3
	filer->writeItem(m_v3);

	// Write the data members.
	filer->writeItem(m_ptP);
	filer->writeItem(m_ptP1);
	//version 2
	filer->writeItem(m_normalP2);
	//version 1
	//filer->writeItem(m_ptP2);
	filer->writeItem(m_dLength1);
	filer->writeItem(m_dLength2);
	filer->writeItem(m_dWidth1);
	filer->writeItem(m_dWidth2);
	filer->writeItem(m_dAngle);
	filer->writeItem(m_dDiameter);
	filer->writeItem(m_nN);

	return filer->filerStatus();
}

Acad::ErrorStatus PDSqutorus1::dwgInFields(AcDbDwgFiler* filer)
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
	if (version > VERSION_PDSQUTORUS1)
		return Acad::eMakeMeProxy;

	if(version >= 3)
		filer->readItem(&m_v3);

	// Read the data members.
	filer->readItem(&m_ptP);
	filer->readItem(&m_ptP1);
	if(version >= 2)
		filer->readItem(&m_normalP2);
	else if(version == 1)
	{
		AcGePoint3d ptTemp;
		filer->readItem(&ptTemp);
		m_normalP2.set(ptTemp.x, ptTemp.y, ptTemp.z);
	}
	filer->readItem(&m_dLength1);
	filer->readItem(&m_dLength2);
	filer->readItem(&m_dWidth1);
	filer->readItem(&m_dWidth2);
	filer->readItem(&m_dAngle);
	filer->readItem(&m_dDiameter);
	filer->readItem(&m_nN);

#ifdef _USEAMODELER_
	if(m_3dGeom.isNull())
		createBody();
#endif

	return filer->filerStatus();
}

Acad::ErrorStatus PDSqutorus1::dxfOutFields(AcDbDxfFiler* filer) const
{
	assertReadEnabled();
	Acad::ErrorStatus es;

	if ((es = PDPrimary3D::dxfOutFields(filer))
		!= Acad::eOk)
	{
		return es;
	}

	// Write subclass marker.
	filer->writeItem(AcDb::kDxfSubclass, _T("PDSqutorus1"));

	// Write version number.
	filer->writeItem(AcDb::kDxfInt16, (Adesk::UInt16) VERSION_PDSQUTORUS1);

	// version 3
	filer->writeItem(AcDb::kDxfXCoord+3, m_v3);

	// Write data members.
	filer->writeItem(AcDb::kDxfXCoord, m_ptP);
	filer->writeItem(AcDb::kDxfXCoord+1, m_ptP1);
	filer->writeItem(AcDb::kDxfXCoord+2, m_normalP2);
	filer->writeItem(AcDb::kDxfReal,m_dLength1);
	filer->writeItem(AcDb::kDxfReal+1,m_dLength2);
	filer->writeItem(AcDb::kDxfReal+2,m_dWidth1);
	filer->writeItem(AcDb::kDxfReal+3,m_dWidth2);
	filer->writeItem(AcDb::kDxfReal+4,m_dAngle);
	filer->writeItem(AcDb::kDxfReal+5,m_dDiameter);
	filer->writeItem(AcDb::kDxfInt32,m_nN);


	return filer->filerStatus();
}

Acad::ErrorStatus PDSqutorus1::dxfInFields(AcDbDxfFiler* filer)
{
	assertWriteEnabled();
	Acad::ErrorStatus es = Acad::eOk;
	struct resbuf rb;

	if ((PDPrimary3D::dxfInFields(filer) != Acad::eOk)
		|| !filer->atSubclassData(_T("PDSqutorus1")))
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
		if (version > VERSION_PDSQUTORUS1)
			return Acad::eMakeMeProxy;
	}

	if(version >= 3)
	{
		filer->readItem(&rb);
		if (rb.restype == AcDb::kDxfXCoord + 3)
		{
			m_v3.set(rb.resval.rpoint[0], rb.resval.rpoint[1], rb.resval.rpoint[2]);
		}
		else
		{
			filer->pushBackItem();
			filer->setError(Acad::eInvalidDxfCode,
				_T("\nError: expected group code %d"),
				AcDb::kDxfXCoord + 3);
			return filer->filerStatus();
		}
	}

	filer->readItem(&rb);
	if (rb.restype == AcDb::kDxfXCoord)
	{
		m_ptP.set(rb.resval.rpoint[0], rb.resval.rpoint[1], rb.resval.rpoint[2]);
	}
	else
	{
		filer->pushBackItem();
		filer->setError(Acad::eInvalidDxfCode,
			_T("\nError: expected group code %d"),
			AcDb::kDxfXCoord);
		return filer->filerStatus();
	}

	filer->readItem(&rb);
	if (rb.restype == AcDb::kDxfXCoord + 1)
	{
		m_ptP1.set(rb.resval.rpoint[0], rb.resval.rpoint[1], rb.resval.rpoint[2]);
	}
	else
	{
		filer->pushBackItem();
		filer->setError(Acad::eInvalidDxfCode,
			_T("\nError: expected group code %d"),
			AcDb::kDxfXCoord + 1);
		return filer->filerStatus();
	}

	filer->readItem(&rb);
	if (rb.restype == AcDb::kDxfXCoord + 2)
	{
		m_normalP2.set(rb.resval.rpoint[0], rb.resval.rpoint[1], rb.resval.rpoint[2]);
	}
	else
	{
		filer->pushBackItem();
		filer->setError(Acad::eInvalidDxfCode,
			_T("\nError: expected group code %d"),
			AcDb::kDxfXCoord + 2);
		return filer->filerStatus();
	}
	filer->readItem(&rb);
	if (rb.restype == AcDb::kDxfReal)
	{
		m_dLength1 = rb.resval.rreal;
	}
	else
	{
		filer->pushBackItem();
		filer->setError(Acad::eInvalidDxfCode,
			_T("\nError: expected group code %d"),
			AcDb::kDxfReal);
		return filer->filerStatus();
	}

	filer->readItem(&rb);
	if (rb.restype == AcDb::kDxfReal + 1)
	{
		m_dLength2 = rb.resval.rreal;
	}
	else
	{
		filer->pushBackItem();
		filer->setError(Acad::eInvalidDxfCode,
			_T("\nError: expected group code %d"),
			AcDb::kDxfReal + 1);
		return filer->filerStatus();
	}

	filer->readItem(&rb);
	if (rb.restype == AcDb::kDxfReal + 2)
	{
		m_dWidth1 = rb.resval.rreal;
	}
	else
	{
		filer->pushBackItem();
		filer->setError(Acad::eInvalidDxfCode,
			_T("\nError: expected group code %d"),
			AcDb::kDxfReal + 2);
		return filer->filerStatus();
	}

	filer->readItem(&rb);
	if (rb.restype == AcDb::kDxfReal + 3)
	{
		m_dWidth2 = rb.resval.rreal;
	}
	else
	{
		filer->pushBackItem();
		filer->setError(Acad::eInvalidDxfCode,
			_T("\nError: expected group code %d"),
			AcDb::kDxfReal + 3);
		return filer->filerStatus();
	}

	filer->readItem(&rb);
	if (rb.restype == AcDb::kDxfReal + 4)
	{
		m_dAngle = rb.resval.rreal;
	}
	else
	{
		filer->pushBackItem();
		filer->setError(Acad::eInvalidDxfCode,
			_T("\nError: expected group code %d"),
			AcDb::kDxfReal + 4);
		return filer->filerStatus();
	}

	filer->readItem(&rb);
	if (rb.restype == AcDb::kDxfReal + 5)
	{
		m_dDiameter = rb.resval.rreal;
	}
	else
	{
		filer->pushBackItem();
		filer->setError(Acad::eInvalidDxfCode,
			_T("\nError: expected group code %d"),
			AcDb::kDxfReal + 5);
		return filer->filerStatus();
	}

	filer->readItem(&rb);
	if (rb.restype == AcDb::kDxfInt32)
	{
		m_nN = rb.resval.rlong;
	}
	else
	{
		filer->pushBackItem();
		filer->setError(Acad::eInvalidDxfCode,
			_T("\nError: expected group code %d"),
			AcDb::kDxfInt32);
		return filer->filerStatus();
	}

#ifdef _USEAMODELER_
	if(m_3dGeom.isNull())
		createBody();
#endif

	return filer->filerStatus();
}


const AcGePoint3d & PDSqutorus1::getCenter() const
{
	assertReadEnabled();
	return m_ptP;
}

const AcGePoint3d & PDSqutorus1::getP1() const
{
	assertReadEnabled();
	return m_ptP1;
}

const AcGeVector3d & PDSqutorus1::getNormalP2() const
{
	assertReadEnabled();
	return m_normalP2;
}

double PDSqutorus1::getLength1() const
{
	assertReadEnabled();
	return m_dLength1;
}

double PDSqutorus1::getLength2() const
{
	assertReadEnabled();
	return m_dLength2;
}

double PDSqutorus1::getWidth1() const
{
	assertReadEnabled();
	return m_dWidth1;
}

double PDSqutorus1::getWidth2() const
{
	assertReadEnabled();
	return m_dWidth2;
}

double PDSqutorus1::getAngle() const
{
	assertReadEnabled();
	return m_dAngle / PI * 180;
}

double PDSqutorus1::getDiameter() const
{
	assertReadEnabled();
	return m_dDiameter;
}

Acad::ErrorStatus PDSqutorus1::getN(Adesk::UInt32& n)
{
	assertReadEnabled();
	n = m_dDividPrecision;
	return Acad::eOk;
}

AcGeVector3d PDSqutorus1::getStartPlaneNormal()const
{
	assertReadEnabled();

	AcGeVector3d startVect = (m_ptP1 - m_ptP).normal();
	AcGeVector3d faceVect = m_normalP2.normal();

	AcGeVector3d result = startVect.crossProduct(faceVect).normal();
	if (isLeftCoord(startVect, faceVect, m_v3))
		return -result;
	else 
		return result;
}

AcGeVector3d PDSqutorus1::getEndPlaneNormal() const
{
	assertReadEnabled();
	if (m_dAngle == PI * 2)
		return -getStartPlaneNormal();
	else
	{
		AcGeVector3d endVect = getEndCenVect();
		AcGeVector3d faceVect = m_normalP2.normal();

		AcGeVector3d result = faceVect.crossProduct(endVect).normal();
		AcGeVector3d startVect = (m_ptP1 - m_ptP).normal();
		if (isLeftCoord(startVect, faceVect, m_v3))
			return -result;
		else
			return result;
	}
}

Acad::ErrorStatus PDSqutorus1:: setParameters(AcGePoint3d ptP,AcGePoint3d ptP1,AcGeVector3d normalP2,
											  double length1, double length2, double width1,
											  double width2, double angle, double diameter,
											  Adesk::UInt32 n,int CorrType  /*0-左手系，1-右手系*/)
{	assertWriteEnabled();
m_ptP = ptP ;
m_ptP1 = ptP1 ;
m_normalP2 = normalP2;

m_dLength1 =fabs(length1) ;
if(m_dLength1==0)
m_dLength1=1;

m_dLength2 =fabs(length2) ;
if(m_dLength2==0)
m_dLength2=1;

m_dWidth1 =fabs(width1) ;
if(m_dWidth1==0)
m_dWidth1=1;

m_dWidth2 =fabs(width2) ;
if(m_dWidth2==0)
m_dWidth2=1;

if(angle < 0)
angle = -angle;
while(angle > 360.0)
angle -= 360.0;
if(angle==0)
angle=1;
m_dAngle = angle ;
m_dDiameter = diameter ;


AcGeVector3d p0p1,facevect;
p0p1 = ptP1-ptP;
facevect=normalP2;
if(p0p1.isZeroLength() && facevect.isZeroLength())
{
	m_ptP1 = ptP + AcGeVector3d(1, 0, 0);
	m_normalP2.set(0,0,1);
	facevect=m_normalP2;
}
else if(facevect.isZeroLength())
{
	AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax, Ay;
	if (fabs(p0p1[X]) <1.0/64 && fabs(p0p1[Y])<1.0/64) 
		Ax = Wy.crossProduct(p0p1);
	else
		Ax = Wz.crossProduct(p0p1);
	Ax.normalize();

	facevect = Ax;
	m_normalP2=facevect;
}
else if(p0p1.isZeroLength())
{
	AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax, Ay;
	if (fabs(facevect[X]) <1.0/64 && fabs(facevect[Y])<1.0/64) 
		Ax = Wy.crossProduct(facevect);
	else
		Ax = Wz.crossProduct(facevect);
	Ax.normalize();

	p0p1 = Ax;
	m_ptP1 = ptP+ p0p1;
}
else if(p0p1.isParallelTo(facevect))
{
	AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax, Ay;
	if (fabs(facevect[X]) <1.0/64 && fabs(facevect[Y])<1.0/64) 
		Ax = Wy.crossProduct(facevect);
	else
		Ax = Wz.crossProduct(facevect);
	Ax.normalize();

	p0p1 = Ax *p0p1.length();
	m_ptP1 = ptP + p0p1;
}


double startDist, endDist;
startDist = m_ptP1.distanceTo(ptP);
AcGeVector3d startVect = (m_ptP1 -  ptP).normal();
double r1 = length1/ 2.0;
double r2 = length2/2.0;

if (startDist<r1)
m_ptP1=ptP+startVect*r1;

AcGePoint3d P2=m_ptP1;
P2= P2.rotateBy(angle,facevect,ptP);	
AcGeVector3d endVect = (P2 - ptP).normal();
endDist=P2.distanceTo(ptP);

if (endDist<r2)
m_dLength2=endDist*2.0;

if(n > 2)
m_dDividPrecision = n;
else
m_dDividPrecision = PRIMARY3D_DEFAULT_PRECISION;

AcGeVector3d v1,v2;
v1 = (m_ptP1-m_ptP).normal();
v2 = getNormalP2();
if(CorrType)
m_v3 = v1.crossProduct(v2).normal();
else
m_v3 = v2.crossProduct(v1).normal();


#ifdef _USEAMODELER_
createBody();
#endif

return Acad::eOk;
}//added by linlin 20050905

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDSqutorus1::subGetOsnapPoints(
	AcDb::OsnapMode     osnapMode,
	Adesk::GsMarker     gsSelectionMark,
	const AcGePoint3d&  pickPoint,
	const AcGePoint3d&  lastPoint,
	const AcGeMatrix3d& viewXform,
	AcGePoint3dArray&   snapPoints,
	AcDbIntArray &   geomIds) const
#else
Acad::ErrorStatus PDSqutorus1::getOsnapPoints(
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

	AcGeVector3d viewDir(viewXform(Z, 0), viewXform(Z, 1),
		viewXform(Z, 2));

	int  dividSegNum = m_dDividPrecision;
	if(dividSegNum < 2)
		dividSegNum = 20;

	AcGePoint3dArray pArray;
	AcGeIntArray stdIdx;
	int actPrecision;
	getVertices(dividSegNum, pArray, stdIdx, actPrecision);
	int actPrecision__1 = actPrecision + 1;
	int stdIdxLen = stdIdx.length();

	AcGeVector3d faceVect;
	faceVect.set(m_normalP2.x, m_normalP2.y, m_normalP2.z).normalize();

	int arcN, lineN;
	arcN = (gsSelectionMark_int - 1) / 4;
	lineN = (gsSelectionMark_int - 1) % 4;
	AcGePoint3d p1,p2;

	switch(osnapMode)
	{
	case AcDb::kOsModeEnd:
		{
			if(m_dAngle <= PI * 2.0 + 0.0001 && m_dAngle >= PI * 2.0 - 0.0001 && m_dLength1 == m_dLength2 && m_dWidth1 == m_dWidth2)
				;
			else
			{
				AcGePoint3d pt = m_ptP1;
				snapPoints.append(pt);
				pt.rotateBy(m_dAngle, faceVect, m_ptP);
				snapPoints.append(pt);
			}
			for (int j=0;j<4;j++)
			{
				for(int i = 0; i <  stdIdxLen; i++)
					snapPoints.append(pArray[stdIdx[i]+actPrecision__1*j]);
			}
		}
		break;
	case AcDb::kOsModeMid:
		//重新定义对象中点的捕捉方式，同时满足实体模型和线框模型的捕捉 
		//modified by szw 2009.11.18 : begin
		if(gsSelectionMark_int <= stdIdxLen)
		{
			for(int i = 0; i < 4; ++i)
			{
				p1 = pArray[i * actPrecision__1 + stdIdx[gsSelectionMark_int - 1]];
				p2 = pArray[(i + 1) * actPrecision__1 + stdIdx[gsSelectionMark_int - 1]];
				snapPoints.append(p1 + (p2 - p1) / 2.0);
			}
		}
		//modified by szw 2009.11.18 : end
		break;
	case AcDb::kOsModeCen:
		//重新定义对象中心点的捕捉方式，同时满足实体模型和线框模型的捕捉 
		//modified by szw 2009.11.18 : begin
		if(gsSelectionMark_int <= stdIdxLen)
		{
			p1 = pArray[stdIdx[gsSelectionMark_int - 1]];
			p2 = pArray[2 * actPrecision__1 + stdIdx[gsSelectionMark_int - 1]];
			snapPoints.append(p1 + (p2 - p1) / 2.0);
		}
		else
			snapPoints.append(m_ptP);
		//modified by szw 2009.11.18 : end
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
			AcGePoint3d pt;
			//重新定义对象垂直正交点的捕捉方式，同时满足实体模型和线框模型的捕捉 
			//modified by szw 2009.11.18 : begin
			if(gsSelectionMark_int <= stdIdxLen)
			{
				for(int i = 0; i < 4; ++i)
				{
					p1 = pArray[i * actPrecision__1 + stdIdx[gsSelectionMark_int - 1]];
					p2 = pArray[(i + 1) * actPrecision__1 + stdIdx[gsSelectionMark_int - 1]];
					AcGeLine3d line(p1, p2);
					pt = line.closestPointTo(lastPoint);
					snapPoints.append(pt);
				}
			}
			else
			{
				if(m_dAngle <= PI * 2.0 + 0.0001 && m_dAngle >= PI * 2.0 - 0.0001 
					&& m_dLength1 == m_dLength2 && m_dWidth1 == m_dWidth2)
				{
					for(int i = 0; i < 4; ++i)
					{
						AcGeCircArc3d cirArc(pArray[i * actPrecision__1],
							pArray[i * actPrecision__1 + actPrecision__1 / 4],
							pArray[i * actPrecision__1 + actPrecision__1 / 2]);
						AcGeCircArc3d cir(cirArc.center(), cirArc.normal(), cirArc.radius());
						pt = cir.closestPointTo(lastPoint);
						snapPoints.append(pt);
					}
				}
				else
				{
					if(m_dLength1 == m_dLength2 && m_dWidth1 == m_dWidth2)
					{
						for(int i = 0; i < 4; ++i)
						{
							AcGeCircArc3d cirArc(pArray[i * actPrecision__1],
								pArray[i * actPrecision__1 + actPrecision__1 / 2],
								pArray[i * actPrecision__1 + actPrecision__1 - 1]);
							AcGeCircArc3d cir(cirArc.center(), cirArc.normal(), cirArc.radius());
							pt = cir.closestPointTo(lastPoint);
							snapPoints.append(pt);
						}
					}
					else
					{
						for(int i = 0; i < 4; ++i)
						{
							AcGePoint3dArray pts;
							for(int j = 0; j <= actPrecision; ++j)
								pts.append(pArray[i * actPrecision__1 + j]);
							AcGeCubicSplineCurve3d curve;
							curve = AcGeCubicSplineCurve3d(pts);
							pt = curve.closestPointTo(lastPoint);
							snapPoints.append(pt);						
						}
					}
				}
			}
			//modified by szw 2009.11.18 : end
		}
		break;
	case AcDb::kOsModeTan:
		break;
	case AcDb::kOsModeNear:
		{
			AcGePoint3d pt;
			//重新定义对象最近点的捕捉方式，同时满足实体模型和线框模型的捕捉 
			//modified by szw 2009.11.18 : begin
			if(gsSelectionMark_int <= stdIdxLen)
			{
				for(int i = 0; i < 4; ++i)
				{
					p1 = pArray[i * actPrecision__1 + stdIdx[gsSelectionMark_int - 1]];
					p2 = pArray[(i + 1) * actPrecision__1 + stdIdx[gsSelectionMark_int - 1]];
					AcGeLine3d line(p1, p2);
					pt = line.projClosestPointTo(pickPoint, viewDir);
					snapPoints.append(pt);
				}
			}
			else
			{
				if(m_dAngle <= PI * 2.0 + 0.0001 && m_dAngle >= PI * 2.0 - 0.0001 
					&& m_dLength1 == m_dLength2 && m_dWidth1 == m_dWidth2)
				{
					for(int i = 0; i < 4; ++i)
					{
						AcGeCircArc3d cirArc(pArray[i * actPrecision__1],
							pArray[i * actPrecision__1 + actPrecision__1 / 4],
							pArray[i * actPrecision__1 + actPrecision__1 / 2]);
						AcGeCircArc3d cir(cirArc.center(), cirArc.normal(), cirArc.radius());
						pt = cir.projClosestPointTo(pickPoint, viewDir);
						snapPoints.append(pt);
					}
				}
				else
				{
					if(m_dLength1 == m_dLength2 && m_dWidth1 == m_dWidth2)
					{
						for(int i = 0; i < 4; ++i)
						{
							AcGeCircArc3d cirArc(pArray[i * actPrecision__1],
								pArray[i * actPrecision__1 + actPrecision__1 / 2],
								pArray[i * actPrecision__1 + actPrecision__1 - 1]);
							AcGeCircArc3d cir(cirArc.center(), cirArc.normal(), cirArc.radius());
							pt = cir.projClosestPointTo(pickPoint, viewDir);
							snapPoints.append(pt);
						}
					}
					else
					{
						for(int i = 0; i < 4; ++i)
						{
							AcGePoint3dArray pts;
							for(int j = 0; j <= actPrecision; ++j)
								pts.append(pArray[i * actPrecision__1 + j]);
							AcGeCubicSplineCurve3d curve;
							curve = AcGeCubicSplineCurve3d(pts);
							pt = curve.projClosestPointTo(pickPoint, viewDir);
							snapPoints.append(pt);						
						}
					}
				}
			}
			//modified by szw 2009.11.18 : end
		}
		break;
	default:
		break;
	}

	return Acad::eOk;
}
//added by linlin 20050816

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDSqutorus1::subGetGripPoints(
	AcGePoint3dArray& gripPoints,
	AcDbIntArray& osnapModes,
	AcDbIntArray& geomIds) const
#else
Acad::ErrorStatus PDSqutorus1::getGripPoints(
	AcGePoint3dArray& gripPoints,
	AcDbIntArray& osnapModes,
	AcDbIntArray& geomIds) const
#endif
{
	assertReadEnabled();
	/*	AcGeVector3d Startvect=(m_ptP1-m_ptP).normal();
	AcGeVector3d Facevect = m_normalP2.normal();
	AcGeVector3d Centervect;
	AcGeCubicSplineCurve3d varcur;
	varcur = AcGeCubicSplineCurve3d(AcGeCircArc3d(
	m_ptP,Facevect,Startvect,m_dDiameter/2,0,m_dAngle));

	AcGePoint3d pt;
	pt = varcur.startPoint();
	gripPoints.append(pt);
	pt = varcur.endPoint();
	gripPoints.append(pt);
	*/
	return Acad::eOk; 
}

/*
Acad::ErrorStatus PDSqutorus1::explode(AcDbVoidPtrArray& entitySet) const
{
assertReadEnabled();
if(m_dWidth1 == m_dWidth2)
{
Acad::ErrorStatus es = Acad::eOk;
AcGePoint3dArray pArray;
AcGeIntArray stdArcIdx, stdCirIdx;
int actPrecisionArc;
getVertices(m_dDividPrecision, pArray, 
stdArcIdx, actPrecisionArc);
int pArrayLen_5 = pArray.length() / 5;

AcDbCurve *pCurve = NULL;
pCurve = new AcDbLine(pArray[0], pArray[pArrayLen_5]);
pCurve->setPropertiesFrom(this);
if(pCurve != NULL)
{
if((es = addToDb(pCurve)) == Acad::eOk)
{
AcDbObjectId id;
id = pCurve->objectId();
pCurve->close();
acdbOpenObject((AcDbCurve*&)pCurve, id, AcDb::kForRead);
}
else
{
if(pCurve != NULL)
delete pCurve;
return explodeToSurface(entitySet);
}
}
else
return explodeToSurface(entitySet);


AcDbLine *pLine;
AcDbVoidPtrArray curveSegments;
for(int i = 0; i < pArrayLen_5 - 1; ++i)
{
pLine = new AcDbLine(pArray[i], pArray[i + 1]);
curveSegments.append(pLine);
}
pLine = new AcDbLine(pArray[pArrayLen_5 - 1], pArray[pArrayLen_5 * 4 - 1]);
curveSegments.append(pLine);
for(i = pArrayLen_5 * 4 - 1; i > pArrayLen_5 * 3; --i)
{
pLine = new AcDbLine(pArray[i], pArray[i - 1]);
curveSegments.append(pLine);
}
pLine = new AcDbLine(pArray[pArrayLen_5 * 3], pArray[0]);
curveSegments.append(pLine);

AcDbVoidPtrArray regions;
es = AcDbRegion::createFromCurves(curveSegments, regions);

if(es == Acad::eOk && !regions.isEmpty())
{
AcDb3dSolid *pBody;
pBody = new AcDb3dSolid;
es = pBody->extrudeAlongPath((AcDbRegion*)(regions[0]), pCurve);
if(es != Acad::eOk)
{
pCurve->upgradeOpen();
pCurve->erase();
pCurve->close();
delete pBody;
for(i = 0; i < regions.length(); i++)
delete (AcRxObject*)regions[i];
for(i = 0; i < curveSegments.length(); i++)
delete (AcRxObject*)curveSegments[i];
return explodeToSurface(entitySet);
}
pBody->setPropertiesFrom(this);
entitySet.append(pBody);
}
else
{
pCurve->upgradeOpen();
pCurve->erase();
pCurve->close();
for(i = 0; i < regions.length(); i++)
delete (AcRxObject*)regions[i];
for(i = 0; i < curveSegments.length(); i++)
delete (AcRxObject*)curveSegments[i];
return explodeToSurface(entitySet);

}

pCurve->upgradeOpen();
pCurve->erase();
pCurve->close();
for(i = 0; i < regions.length(); i++)
delete (AcRxObject*)regions[i];
for(i = 0; i < curveSegments.length(); i++)
delete (AcRxObject*)curveSegments[i];

return Acad::eOk;
}
else
{
#ifdef _OBJECTARX2004_

Acad::ErrorStatus es = Acad::eOk;

AcGeVector3d Facevect;
Facevect.set(m_normalP2.x, m_normalP2.y, m_normalP2.z).normalize();

if(Facevect == AcGeVector3d::kIdentity)
return Acad::eNotApplicable;

double startDist = m_ptP.distanceTo(m_ptP1);

AcDbCurve *pCurve = NULL;
double pathLen = 0.0;
if(m_dAngle <= PI * 2.0 + 0.0001 && m_dAngle >= PI * 2.0 - 0.0001)
if(m_dLength1==m_dLength2&&m_dWidth1==m_dWidth2)
{
AcDbCircle *pCir;
pCir = new AcDbCircle(m_ptP, Facevect, startDist);
pathLen = 2.0 * PI * startDist;
pCurve = pCir;
}
else
return explodeToSurface(entitySet);
else
if(m_dAngle <= PI * 1.0 + 0.0001 && m_dAngle >= PI * 1.0 - 0.0001) 
{
if(m_dLength1==m_dLength2&&m_dWidth1==m_dWidth2)
{
AcGeVector3d mnormal;
mnormal = Facevect;
AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax, Ay;
if (fabs(mnormal[X]) <1.0/64 && fabs(mnormal[Y])<1.0/64) 
Ax = Wy.crossProduct(mnormal);
else
Ax = Wz.crossProduct(mnormal);
Ax.normalize();
Ay = mnormal.crossProduct(Ax);
Ay.normalize();

double startAng, endAng;
startAng = Ax.angleTo(m_ptP1 - m_ptP, mnormal);
endAng = startAng + PI;
if(endAng > 2.0 * PI)
endAng -= 2.0 * PI;
AcDbArc *pArc;
pArc = new AcDbArc(m_ptP, mnormal, startDist, startAng, endAng);
pathLen = PI * startDist;
pCurve = pArc;

}
else
return explodeToSurface(entitySet);

}
else
{ 
if(m_dLength1==m_dLength2&&m_dWidth1==m_dWidth2)
{
AcGeVector3d mnormal;
mnormal = Facevect;
AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax, Ay;
if (fabs(mnormal[X]) <1.0/64 && fabs(mnormal[Y])<1.0/64) 
Ax = Wy.crossProduct(mnormal);
else
Ax = Wz.crossProduct(mnormal);
Ax.normalize();
Ay = mnormal.crossProduct(Ax);
Ay.normalize();

double startAng, endAng;
startAng = Ax.angleTo(m_ptP1 - m_ptP, mnormal);
endAng = startAng + m_dAngle;
if(endAng > 2.0 * PI)
endAng -= 2.0 * PI;
AcDbArc *pArc;
pArc = new AcDbArc(m_ptP, mnormal, startDist, startAng, endAng);
double endParam;
pArc->getEndParam(endParam);
pArc->getDistAtParam(endParam, pathLen);
pCurve = pArc;
}
else
return explodeToSurface(entitySet);
}

pCurve->setPropertiesFrom(this);
if(pCurve != NULL)
{
if((es = addToDb(pCurve)) == Acad::eOk)
{
AcDbObjectId id;
id = pCurve->objectId();
pCurve->close();
acdbOpenObject((AcDbCurve*&)pCurve, id, AcDb::kForRead);
}
else
{
if(pCurve != NULL)
delete pCurve;
return explodeToSurface(entitySet);
}
}
else
return explodeToSurface(entitySet);


AcGePoint3d point1, point2, point3, point4;
AcGeVector3d Startvect = (m_ptP1 - m_ptP).normal();

point1 = m_ptP1 - Startvect * m_dLength1 / 2.0 - Facevect * m_dWidth1 / 2.0;
point2 = point1 + Startvect * m_dLength1;
point3 = point2 + Facevect * m_dWidth1;
point4 = point3 - Startvect * m_dLength1;

AcDbLine line1(point1, point2);
AcDbLine line2(point2, point3);
AcDbLine line3(point3, point4);
AcDbLine line4(point4, point1);

AcDbVoidPtrArray curveSegments;
curveSegments.append(&line1);
curveSegments.append(&line2);
curveSegments.append(&line3);
curveSegments.append(&line4);

AcDbVoidPtrArray regions;
es = AcDbRegion::createFromCurves(curveSegments, regions);

if(es == Acad::eOk && !regions.isEmpty())
{
AcDb3dSolid *pBody;
pBody = new AcDb3dSolid;

es = pBody->extrudeAlongPath((AcDbRegion*)(regions[0]), pCurve,
atan((m_dLength1 / 2.0 - m_dLength2 / 2.0) / pathLen));

if(es != Acad::eOk)
{
pCurve->upgradeOpen();
pCurve->erase();
pCurve->close();
delete pBody;
for(int i = 0; i < regions.length(); i++)
delete (AcRxObject*)regions[i];
return explodeToSurface(entitySet);
}
pBody->setPropertiesFrom(this);
entitySet.append(pBody);
}
else
{
pCurve->upgradeOpen();
pCurve->erase();
pCurve->close();
for(int i = 0; i < regions.length(); i++)
delete (AcRxObject*)regions[i];
return explodeToSurface(entitySet);
}

pCurve->upgradeOpen();
pCurve->erase();
pCurve->close();
for(int i = 0; i < regions.length(); i++)
delete (AcRxObject*)regions[i];

return Acad::eOk;

#else
return explodeToSurface(entitySet);
#endif
}
}//added by  linlin 20050915
*/

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDSqutorus1::subExplode(AcDbVoidPtrArray& entitySet) const
#else
Acad::ErrorStatus PDSqutorus1::explode(AcDbVoidPtrArray& entitySet) const
#endif
{
	assertReadEnabled();

	//两边的矩形长宽按照同样比例缩放时用extrudeAlongPath函数拉伸
	if ( fabs((m_dWidth1/m_dWidth2)-(m_dLength1/m_dLength2)) <= SIGMA )
	{
#ifdef _OBJECTARX2004_
		//////////////////////////////////////////////////////////////////////////
		double minL = min(m_dLength1, m_dLength2);
		double minW = min(m_dWidth1, m_dWidth2);
		double arcRadius = m_ptP1.distanceTo(m_ptP);

		//path
		Acad::ErrorStatus es = Acad::eOk;
		AcDbCurve *pCurve = NULL;
		double pathLen;

		//path
		if (fabs(m_dAngle-2*PI) <= SIGMA)//=360
		{
			AcGePoint3d ptCenter(arcRadius, 0,0);
			AcGeVector3d faceNor(0,1,0);

			//不变径时，路径用圆环表示
			if (fabs(m_dLength1-m_dLength2) <= SIGMA && fabs(m_dWidth1-m_dWidth2) <= SIGMA)
			{
				AcDbCircle *pArc;
				pArc = new AcDbCircle(ptCenter, faceNor, arcRadius);
				pathLen = PI * arcRadius * 2;
				pCurve = pArc;
			} 
			//变径时路径用圆弧近似
			else
			{
				AcDbArc *pArc;
				pArc = new AcDbArc(ptCenter, faceNor, arcRadius, 0, 1.999*PI);
				pathLen = PI * arcRadius * 1.999;
				pCurve = pArc;
			}
		} 
		else//<360
		{
			AcGePoint3d ptCenter(arcRadius, 0,0);
			AcGeVector3d faceNor(0,1,0);
			AcDbArc *pArc;
			pArc = new AcDbArc(ptCenter, faceNor, arcRadius, 0, m_dAngle);
			pathLen = arcRadius * m_dAngle;
			pCurve = pArc;
		}

		pCurve->setPropertiesFrom(this);
		if(pCurve != NULL)
		{
			if((es = addToDb(pCurve)) == Acad::eOk)
			{
				AcDbObjectId id;
				id = pCurve->objectId();
				pCurve->close();
				acdbOpenObject((AcDbCurve*&)pCurve, id, AcDb::kForRead);
			}
			else
			{
				if(pCurve != NULL)
					delete pCurve;
				return explodeToSurface(entitySet);
			}
		}
		else
		{
			return explodeToSurface(entitySet);
		}


		//curve
		//AcGePoint3d p1(minW/2,-minL/2,0),p2(minW/2,minL/2,0),p3(-minW/2,minL/2,0),p4(-minW/2,-minL/2,0);
		AcGePoint3d p1(minL/2,-minW/2,0),p2(minL/2,minW/2,0),p3(-minL/2,minW/2,0),p4(-minL/2,-minW/2,0);

		AcDbLine line1(p1, p2);
		AcDbLine line2(p2, p3);
		AcDbLine line3(p3, p4);
		AcDbLine line4(p4, p1);

		AcDbVoidPtrArray curveSegments;
		curveSegments.append(&line1);
		curveSegments.append(&line2);
		curveSegments.append(&line3);
		curveSegments.append(&line4);

		AcDbVoidPtrArray regions;
		es = AcDbRegion::createFromCurves(curveSegments, regions);

		if(es == Acad::eOk && !regions.isEmpty())
		{
			AcDb3dSolid *pBody;
			pBody = new AcDb3dSolid;
			es = pBody->extrudeAlongPath((AcDbRegion*)(regions[0]), pCurve,
				-atan((max(m_dLength1,m_dLength2) / 2.0 - min(m_dLength1,m_dLength2) / 2.0) / pathLen)
				);
			if(es != Acad::eOk)
			{
				pCurve->upgradeOpen();
				pCurve->erase();
				pCurve->close();
				delete pBody;
				for(int i = 0; i < regions.length(); i++)
					delete (AcRxObject*)regions[i];
				return explodeToSurface(entitySet);
			}

			//transform
			//变换到原坐标系
			AcGeMatrix3d mat;
			AcGePoint3d ori(0,0,0);
			AcGeVector3d vecx(1,0,0);
			AcGeVector3d vecy(0,1,0);
			AcGeVector3d vecz(0,0,1);
			AcGePoint3d pOrign(0,0,0);
			AcGeVector3d axisX(1,0,0);
			AcGeVector3d axisY(0,1,0);
			AcGeVector3d axisZ(0,0,1);
			AcGeVector3d Facevect = getNormalP2();
			AcGeVector3d v1,v2;
			v1 = (m_ptP1 - m_ptP).normal();
			v2 = getNormalP2();
			if(isLeftCoord(v1,v2,m_v3))
				Facevect = -Facevect;

			if (fabs(m_dAngle-2*PI) <= SIGMA)//=360
			{
				pOrign = m_ptP1;
				axisX = (m_ptP-m_ptP1).normal();

				if (m_dLength1 >= m_dLength2)
				{
					axisY = -1*Facevect;

				}
				else
				{
					axisY = Facevect;
				}

				axisZ = axisX.crossProduct(axisY);
			}
			else//<360
			{
				if (m_dLength1 >= m_dLength2)
				{
					axisX = -1*getEndCenVect();
					pOrign = m_ptP - arcRadius*axisX;
					axisY = -1*Facevect;
					axisZ = axisX.crossProduct(axisY);
				}
				else
				{
					pOrign = m_ptP1;

					axisX = (m_ptP-m_ptP1).normal();
					axisY = Facevect;
					axisZ = axisX.crossProduct(axisY);
				}
			}

			mat.setToAlignCoordSys(ori, vecx, vecy, vecz,
				pOrign, axisX, axisY, axisZ);

			pBody->transformBy(mat);

			pBody->setPropertiesFrom(this);
			entitySet.append(pBody);
		}
		else
		{
			pCurve->upgradeOpen();
			pCurve->erase();
			pCurve->close();
			for(int i = 0; i < regions.length(); i++)
				delete (AcRxObject*)regions[i];
			return explodeToSurface(entitySet);

		}

		pCurve->upgradeOpen();
		pCurve->erase();
		pCurve->close();
		for(int i = 0; i < regions.length(); i++)
			delete (AcRxObject*)regions[i];

		return Acad::eOk;
#else
		return explodeToSurface(entitySet);
#endif
	} 
	//长宽变化比例不同时用面表示
	else
	{
		return explodeToSurface(entitySet);
	}
}//added by  linlin 20050915

//  [9/5/2007 suzhiyong]
Acad::ErrorStatus PDSqutorus1::explodeTo3DSolid(AcDb3dSolid* &p3dSolid) const
{
	assertReadEnabled();

	//两边的矩形长宽按照同样比例缩放时用extrudeAlongPath函数拉伸
	if ( fabs((m_dWidth1/m_dWidth2)-(m_dLength1/m_dLength2)) <= SIGMA )
	{
#ifdef _OBJECTARX2004_
		//////////////////////////////////////////////////////////////////////////
		double minL = min(m_dLength1, m_dLength2);
		double minW = min(m_dWidth1, m_dWidth2);
		double arcRadius = m_ptP1.distanceTo(m_ptP);

		//path
		Acad::ErrorStatus es = Acad::eOk;
		AcDbCurve *pCurve = NULL;
		double pathLen;

		//path
		if (fabs(m_dAngle-2*PI) <= SIGMA)//=360
		{
			AcGePoint3d ptCenter(arcRadius, 0,0);
			AcGeVector3d faceNor(0,1,0);

			//不变径时，路径用圆环表示
			if (fabs(m_dLength1-m_dLength2) <= SIGMA && fabs(m_dWidth1-m_dWidth2) <= SIGMA)
			{
				AcDbCircle *pArc;
				pArc = new AcDbCircle(ptCenter, faceNor, arcRadius);
				pathLen = PI * arcRadius * 2;
				pCurve = pArc;
			} 
			//变径时路径用圆弧近似
			else
			{
				AcDbArc *pArc;
				pArc = new AcDbArc(ptCenter, faceNor, arcRadius, 0, 1.999*PI);
				pathLen = PI * arcRadius * 1.999;
				pCurve = pArc;
			}
		} 
		else//<360
		{
			AcGePoint3d ptCenter(arcRadius, 0,0);
			AcGeVector3d faceNor(0,1,0);
			AcDbArc *pArc;
			pArc = new AcDbArc(ptCenter, faceNor, arcRadius, 0, m_dAngle);
			pathLen = arcRadius * m_dAngle;
			pCurve = pArc;
		}

		pCurve->setPropertiesFrom(this);
		if(pCurve != NULL)
		{
			if((es = addToDb(pCurve)) == Acad::eOk)
			{
				AcDbObjectId id;
				id = pCurve->objectId();
				pCurve->close();
				acdbOpenObject((AcDbCurve*&)pCurve, id, AcDb::kForRead);
			}
			else
			{
				if(pCurve != NULL)
					delete pCurve;
				return Acad::eNotApplicable;
			}
		}
		else
		{
			return Acad::eNotApplicable;
		}


		//curve
		AcGePoint3d p1(minL/2,-minW/2,0),p2(minL/2,minW/2,0),p3(-minL/2,minW/2,0),p4(-minL/2,-minW/2,0);

		AcDbLine line1(p1, p2);
		AcDbLine line2(p2, p3);
		AcDbLine line3(p3, p4);
		AcDbLine line4(p4, p1);

		AcDbVoidPtrArray curveSegments;
		curveSegments.append(&line1);
		curveSegments.append(&line2);
		curveSegments.append(&line3);
		curveSegments.append(&line4);

		AcDbVoidPtrArray regions;
		es = AcDbRegion::createFromCurves(curveSegments, regions);

		if(es == Acad::eOk && !regions.isEmpty())
		{
			AcDb3dSolid *pBody;
			pBody = new AcDb3dSolid;
			es = pBody->extrudeAlongPath((AcDbRegion*)(regions[0]), pCurve,
				-atan((max(m_dLength1,m_dLength2) / 2.0 - min(m_dLength1,m_dLength2) / 2.0) / pathLen)
				);
			if(es != Acad::eOk)
			{
				pCurve->upgradeOpen();
				pCurve->erase();
				pCurve->close();
				delete pBody;
				for(int i = 0; i < regions.length(); i++)
					delete (AcRxObject*)regions[i];
				return Acad::eNotApplicable;
			}

			//transform
			//变换到原坐标系
			AcGeMatrix3d mat;
			AcGePoint3d ori(0,0,0);
			AcGeVector3d vecx(1,0,0);
			AcGeVector3d vecy(0,1,0);
			AcGeVector3d vecz(0,0,1);
			AcGePoint3d pOrign(0,0,0);
			AcGeVector3d axisX(1,0,0);
			AcGeVector3d axisY(0,1,0);
			AcGeVector3d axisZ(0,0,1);
			AcGeVector3d Facevect = getNormalP2();
			AcGeVector3d v1,v2;
			v1 = (m_ptP1 - m_ptP).normal();
			v2 = getNormalP2();
			if(isLeftCoord(v1,v2,m_v3))
				Facevect = -Facevect;

			if (fabs(m_dAngle-2*PI) <= SIGMA)//=360
			{
				pOrign = m_ptP1;
				axisX = (m_ptP-m_ptP1).normal();

				if (m_dLength1 >= m_dLength2)
				{
					axisY = -1*Facevect;

				}
				else
				{
					axisY = Facevect;
				}

				axisZ = axisX.crossProduct(axisY);
			}
			else//<360
			{
				if (m_dLength1 >= m_dLength2)
				{
					axisX = -1*getEndCenVect();
					pOrign = m_ptP - arcRadius*axisX;
					axisY = -1*Facevect.normal();
					axisZ = axisX.crossProduct(axisY);
				}
				else
				{
					pOrign = m_ptP1;

					axisX = (m_ptP-m_ptP1).normal();
					axisY = Facevect.normal();
					axisZ = axisX.crossProduct(axisY);
				}
			}

			mat.setToAlignCoordSys(ori, vecx, vecy, vecz,
				pOrign, axisX, axisY, axisZ);

			pBody->transformBy(mat);

			pBody->setPropertiesFrom(this);

			p3dSolid = pBody;
		}
		else
		{
			pCurve->upgradeOpen();
			pCurve->erase();
			pCurve->close();
			for(int i = 0; i < regions.length(); i++)
				delete (AcRxObject*)regions[i];
			return Acad::eNotApplicable;

		}

		pCurve->upgradeOpen();
		pCurve->erase();
		pCurve->close();
		for(int i = 0; i < regions.length(); i++)
			delete (AcRxObject*)regions[i];

		return Acad::eOk;
#else
		return Acad::eNotApplicable;
#endif
	} 
	//长宽变化比例不同时用面表示
	else
	{
		return Acad::eNotApplicable;
	}
}

Acad::ErrorStatus PDSqutorus1::explodeToSurface(AcDbVoidPtrArray& entitySet) const
{
	assertReadEnabled();

	Acad::ErrorStatus es = Acad::eOk;
	int i;

	AcDbVoidPtrArray curveSegments,curveSegments1, resRegion;
	AcGePoint3d  ucs_org = acdbCurDwg()->ucsorg();
	AcGeVector3d ucs_x = acdbCurDwg()->ucsxdir();
	AcGeVector3d ucs_y = acdbCurDwg()->ucsydir();
	AcGeVector3d ucs_z = ucs_x.crossProduct(ucs_y);
	AcGeMatrix3d ucs_mat;

	AcGeVector3d faceVect;
	faceVect.set(m_normalP2.x, m_normalP2.y, m_normalP2.z).normalize();
	AcGeVector3d Startvect = (m_ptP1 - m_ptP).normal();
	AcGeVector3d Endvect=Startvect;
	Endvect=Endvect.rotateBy(m_dAngle,faceVect);
	AcGeVector3d curvect;

	curvect = Startvect.crossProduct(faceVect).normal();
	ucs_mat.setToWorldToPlane(curvect);
	//#ifdef _OBJECTARX14_
	//    acdbSetCurrentUCS(ucs_mat);
	//#else
	//    acedSetCurrentUCS(ucs_mat);
	//#endif

	AcGePoint3d point1, point2, point3, point4;

	point1 = m_ptP1 - Startvect * m_dLength1 / 2.0 - faceVect * m_dWidth1 / 2.0;
	point2 = point1 + Startvect * m_dLength1;
	point3 = point2 + faceVect * m_dWidth1;
	point4 = point3 - Startvect * m_dLength1;

	AcDbLine line1(point1, point2);
	AcDbLine line2(point2, point3);
	AcDbLine line3(point3, point4);
	AcDbLine line4(point4, point1);

	curveSegments.append(&line1);
	curveSegments.append(&line2);
	curveSegments.append(&line3);
	curveSegments.append(&line4);
	es = AcDbRegion::createFromCurves(curveSegments, resRegion);
	if(es != Acad::eOk)
	{
		for(i = 0; i < resRegion.length(); ++i)
			delete (AcRxObject*)resRegion[i];
	}
	else
	{
		for(i = 0; i < resRegion.length(); ++i)
			((AcDbEntity *)resRegion[i])->setPropertiesFrom(this);
		entitySet.append(resRegion);
	}

	resRegion.setLogicalLength(0);
	curvect = -(Endvect.crossProduct(faceVect).normal());
	ucs_mat.setToWorldToPlane(curvect);
	//#ifdef _OBJECTARX14_
	//    acdbSetCurrentUCS(ucs_mat);
	//#else
	//    acedSetCurrentUCS(ucs_mat);
	//#endif
	AcGePoint3d m_ptP2=m_ptP1;
	m_ptP2.rotateBy(m_dAngle,faceVect,m_ptP);
	point1 = m_ptP2 - Endvect * m_dLength2/ 2.0 - faceVect * m_dWidth2 / 2.0;
	point2 = point1 + Endvect * m_dLength2;
	point3 = point2 + faceVect * m_dWidth2;
	point4 = point3 - Endvect * m_dLength2;

	AcDbLine line5(point1, point2);
	AcDbLine line6(point2, point3);
	AcDbLine line7(point3, point4);
	AcDbLine line8(point4, point1);

	curveSegments1.append(&line5);
	curveSegments1.append(&line6);
	curveSegments1.append(&line7);
	curveSegments1.append(&line8);
	es = AcDbRegion::createFromCurves(curveSegments1, resRegion);
	if(es != Acad::eOk)
	{
		for(i = 0; i < resRegion.length(); ++i)
			delete (AcRxObject*)resRegion[i];
	}
	else
	{
		for(i = 0; i < resRegion.length(); ++i)
			((AcDbEntity *)resRegion[i])->setPropertiesFrom(this);
		entitySet.append(resRegion);
	}

	ucs_mat.setCoordSystem(ucs_org, ucs_x, ucs_y, ucs_z);
	//#ifdef _OBJECTARX14_
	//    acdbSetCurrentUCS(ucs_mat);
	//#else
	//    acedSetCurrentUCS(ucs_mat);
	//#endif

	AcGePoint3dArray pArray;
	AcGeIntArray stdArcIdx;
	int actPrecisionArc;
	getVertices(m_dDividPrecision, pArray, 
		stdArcIdx, actPrecisionArc);

	AcDbPolygonMesh *pMesh;
	pMesh = new AcDbPolygonMesh(AcDb::kSimpleMesh, 5,actPrecisionArc + 1,
		pArray, Adesk::kFalse, Adesk::kFalse);
	pMesh->setPropertiesFrom(this);
	entitySet.append(pMesh);

	return Acad::eOk;
}//added by  linlin 20050915


#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDSqutorus1::subGetGeomExtents(AcDbExtents& extents) const
#else
Acad::ErrorStatus PDSqutorus1::getGeomExtents(AcDbExtents& extents) const
#endif
{
	assertReadEnabled();
	extents.set(m_ptP1, m_ptP1);
	int  dividSegNum = m_dDividPrecision;
	if(dividSegNum < 2)
		dividSegNum = 20;
	AcGePoint3dArray pArray;
	AcGeIntArray stdIdx;
	int actPrecision;
	getVertices(dividSegNum, pArray, stdIdx, actPrecision);
	int i, arrlen = pArray.length();
	for(i = 0; i < arrlen; ++i)
		extents.addPoint(pArray[i]);
	return Acad::eOk;
}

#ifdef _OBJECTARX2010_
void PDSqutorus1::subList() const
#else
void PDSqutorus1::list() const
#endif
{
	assertReadEnabled();
#ifdef _OBJECTARX2010_
	PDPrimary3D::subList();
#else
	PDPrimary3D::list();
#endif
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4817/*"\n 法线方向(%f,%f,%f)\n"*/,m_normalP2.x,m_normalP2.y,m_normalP2.z);
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4818/*" 起始矩形的中心(%f,%f,%f)\n"*/,m_ptP1.x,m_ptP1.y,m_ptP1.z);
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4809/*" 起始矩形的长度=%.3f 起始矩形的宽度=%.3f\n"*/,m_dLength1,m_dWidth1);
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4812/*" 终止矩形的长度=%.3f 终止矩形的宽度=%.3f\n"*/,m_dLength2,m_dWidth2);
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4813/*" 圆环圆心(%f,%f,%f)\n"*/,m_ptP.x,m_ptP.y,m_ptP.z);
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4819/*" 圆环直径=%.3f\n"*/,m_dDiameter);
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4820/*" 角度:         %f\n\n"*/,m_dAngle);
	return ;
}// added by linlin 20050811

void PDSqutorus1::createBody()
{
	assertWriteEnabled();
#ifdef _OBJECTARX2004_

	/*   AcGePoint3dArray pArray;
	AcGeIntArray stdArcIdx, stdCirIdx;
	int actPrecisionArc;
	getVertices(m_dDividPrecision, pArray, 
	stdArcIdx, actPrecisionArc);
	AcGePoint3d* vertices;
	int n = pArray.length()/5;
	vertices = new AcGePoint3d[8];

	int i, j, k, m;

	for( i =0; i<n-1; ++i){
	for( m=0, j=0; j<2; ++j){
	for( k=0; k<4; ++k){
	vertices[m] = pArray[ n*k + i + j];
	m ++;
	}
	}
	m_3dGeom += Body::convexHull((Point3d*)vertices, 8);
	}

	delete [] vertices;
	*/

	AcGePoint3dArray pArray;
	AcGeIntArray stdArcIdx, stdCirIdx;
	int actPrecisionArc;
	m_no8 =true;
	getVertices(m_dDividPrecision, pArray, 
		stdArcIdx, actPrecisionArc);
	m_no8 =false;

	Body **profiles;
	AcGePoint3d* vertices;
	Face *f;

	int n = pArray.length()/5;
	profiles = new Body*[n];
	vertices = new AcGePoint3d[4];

	int i, j, type;
	for( i =0; i<n; ++i){
		profiles[i] =new Body();
		for( j=0; j<4; ++j){
			vertices[j] = pArray[ j*n+i];
		}
		if( i==0){
			AcGeVector3d vn = ( vertices[1]-vertices[0]).crossProduct( vertices[2]-vertices[0]);
			AcGeVector3d vp = ( pArray[1] - pArray[0]);
			if( vn.dotProduct( vp)>0) type =0;
			else type =1;

		}
		f = new Face( (Point3d*)vertices, 4, type, false, profiles[i]);
	}

	bool isclose =( fabs(m_dAngle-2*PI)<1E-3 ); // && fabs(m_dLength1-m_dLength2)<1E-3 && fabs(m_dWidth1-m_dWidth2)<1E-3);

	m_3dGeom = Body::skin(	profiles,
		isclose? n-1: n,
		isclose,
		true,
		NULL,
		true,
		true);
	// m_3dGeom.cleanUpNonManifoldEdgesAndCoincidentFaces();

	for( i=0; i<n; ++i) delete profiles[i];
	delete [] vertices;
	delete [] profiles;

#endif
}

// 取得圆环中心到圆环终点的矢量
AcGeVector3d PDSqutorus1::getEndCenVect() const
{
	assertReadEnabled();
	AcGeVector3d vecTemp;
	vecTemp=(m_ptP1 - m_ptP).normal();
	AcGeVector3d Facevect;
	Facevect.set(m_normalP2.x, m_normalP2.y, m_normalP2.z).normalize();
	AcGeVector3d v1,v2;
	v1 = (m_ptP1 - m_ptP).normal();
	v2 = getNormalP2();
	if(isLeftCoord(v1,v2,m_v3))
		vecTemp=vecTemp.rotateBy(m_dAngle,-Facevect);
	else
		vecTemp=vecTemp.rotateBy(m_dAngle,Facevect);
	return vecTemp.normal();
}

AcGePoint3d PDSqutorus1::getEndPoint() const
{
	assertReadEnabled();

	AcGeVector3d vect = getEndCenVect();
	double dist = m_ptP.distanceTo(m_ptP1);
	return m_ptP + vect*dist;
}