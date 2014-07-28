// PDSqutorus.cpp: implementation of the PDSqutorus class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "language25.h"
#include "PDSqutorus.h"
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
#define VERSION_PDSQUTORUS 2
#define TOURS_SIMPLIZE_FACTOR 0.9

const double SIGMA = 0.01;

#define PI 3.1415926535897932384626433832795L

#ifdef _OBJECTARX2000_
ACRX_DXF_DEFINE_MEMBERS(PDSqutorus, PDPrimary3D,
						AcDb::kDHL_CURRENT, AcDb::kMReleaseCurrent,
						AcDbProxyEntity::kAllAllowedBits,
						PDSQUTORUS, Gle);
#else
ACRX_DXF_DEFINE_MEMBERS(PDSqutorus, PDPrimary3D, AcDbProxyEntity::kAllAllowedBits, PDSQUTORUS, Gle);
#endif

void PDSqutorus::setDefault()
{
	PDPrimary3D::setDefault();
#if defined(NDEBUG) || !defined(_DEBUG)
	m_dDividPrecision = PRIMARY3D_DEFAULT_PRECISION;
	m_dLength1 = m_dLength2 = m_dWidth1 = m_dWidth2 = m_dRadius = 1.0;
	m_ptP= m_ptP1 = m_ptP2 = AcGePoint3d();
	m_nDirection = 0;
	m_v3.set(0,0,0);
#else
	m_dDividPrecision = PRIMARY3D_DEFAULT_PRECISION;
	m_dLength1 = m_dLength2 = m_dWidth1 = m_dWidth2 = m_dRadius = 1000.0;
	m_ptP.set(0,0,0);
	m_ptP1.set(1000,0,0);
	m_ptP2.set(1000,1000,0);
	m_nDirection = 0;
	m_v3.set(0,0,0);
#endif
}
bool PDSqutorus::isValidData(double &f)
{
	if(!PDPrimary3D::isValidData(f))
		return false;
	if(!isValidFloat(f=m_dLength1))
		return false;
	if(!isValidFloat(f=m_dLength2))
		return false;
	if(!isValidFloat(f=m_dWidth1))
		return false;
	if(!isValidFloat(f=m_dWidth2))
		return false;
	if(!isValidFloat(f=m_dRadius))
		return false;
	if(!isValidFloat(f=m_ptP.x) || !isValidFloat(f=m_ptP.y) || !isValidFloat(f=m_ptP.z))
		return false;
	if(!isValidFloat(f=m_ptP1.x) || !isValidFloat(f=m_ptP1.y) || !isValidFloat(f=m_ptP1.z))
		return false;
	if(!isValidFloat(f=m_ptP2.x) || !isValidFloat(f=m_ptP2.y) || !isValidFloat(f=m_ptP2.z))
		return false;
	if(!isValidFloat(f=m_v3.x) || !isValidFloat(f=m_v3.y) || !isValidFloat(f=m_v3.z))
		return false;
	return true;
}

//zxb,20090113,增加audit，暂时只check自身成员数据
Acad::ErrorStatus PDSqutorus::audit(AcDbAuditInfo* pAuditInfo)
{
	return auditPDPrimary3D(this,pAuditInfo,_T("PDSqutorus"));
}

PDSqutorus::PDSqutorus(bool HasSnap/* = false*/) : PDPrimary3D(HasSnap)
{
	m_dDividPrecision = PRIMARY3D_DEFAULT_PRECISION;
	m_dLength1 = m_dLength2 = m_dWidth1 = m_dWidth2 = m_dRadius = 1.0;
	m_ptP= m_ptP1 = m_ptP2 = AcGePoint3d();
	m_nDirection = 0;
	m_v3.set(0,0,0);
#ifdef _USEAMODELER_
	createBody();
#endif
}

PDSqutorus::~PDSqutorus()
{

}

PDSqutorus::PDSqutorus( AcGePoint3d p, AcGePoint3d p1, AcGePoint3d p2,
					   double l1,double w1,double l2,double w2,
					   int d, int n/* = 20*/, bool HasSnap/* = false*/,int CorrType/*0-左手系，1-右手系*/) : PDPrimary3D(HasSnap)
{
	m_dLength1 =fabs(l1) ;
	if(m_dLength1==0)
		m_dLength1=1;

	m_dWidth1 =fabs(w1) ;
	if(m_dWidth1==0)
		m_dWidth1=1;

	//	if ( d == 2 ){
	//		m_dLength2 = m_dLength1;
	//		m_dWidth2 = m_dWidth1;
	//	}
	//	else{
	m_dLength2 =fabs(l2) ;
	if(m_dLength2==0)
		m_dLength2=1;

	m_dWidth2 =fabs(w2) ;
	if(m_dWidth2==0)
		m_dWidth2=1;

	//	}
	m_ptP = p;
	m_ptP1 = p1;
	m_ptP2 = p2;

	if(n > 2)
		m_dDividPrecision = n;
	else
		m_dDividPrecision = PRIMARY3D_DEFAULT_PRECISION;

	if(d >= 0 && d <= 3)
		m_nDirection = d;
	else
		m_nDirection = 0;

	switch(m_nDirection)
	{
	case 0:
	case 1:
		{
			AcGeVector3d p0p1, p0p2;
			p0p1 = p1-p;
			p0p2 = p2-p;
			if(p0p1.isZeroLength() && p0p2.isZeroLength())
			{
				m_ptP1 = p + AcGeVector3d(0, 1, 0);
				m_ptP2 = p + AcGeVector3d(1, 0, 0);
			}
			else if(p0p2.isZeroLength())
			{
				AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax, Ay;
				if (fabs(p0p1[X]) <1.0/64 && fabs(p0p1[Y])<1.0/64) 
					Ax = Wy.crossProduct(p0p1);
				else
					Ax = Wz.crossProduct(p0p1);
				Ax.normalize();

				p0p2 = Ax;
				m_ptP2 = p + p0p2;
			}
			else if(p0p1.isZeroLength())
			{
				AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax, Ay;
				if (fabs(p0p2[X]) <1.0/64 && fabs(p0p2[Y])<1.0/64) 
					Ax = Wy.crossProduct(p0p2);
				else
					Ax = Wz.crossProduct(p0p2);
				Ax.normalize();

				p0p1 = Ax;
				m_ptP1 = p+ p0p1;
			}
			else if(p0p1.isParallelTo(p0p2))
			{
				AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax, Ay;
				if (fabs(p0p2[X]) <1.0/64 && fabs(p0p2[Y])<1.0/64) 
					Ax = Wy.crossProduct(p0p2);
				else
					Ax = Wz.crossProduct(p0p2);
				Ax.normalize();

				p0p1 = Ax *p0p1.length();
				m_ptP1 = p + p0p1;
			}
		}
		break;

	case 2:
	case 3:
		{
			AcGeVector3d p0p1,facevect;
			p0p1 = p1-p;
			facevect=p2.asVector().normal();
			if(p0p1.isZeroLength() && facevect.isZeroLength())
			{
				m_ptP1 = p + AcGeVector3d(1, 0, 0);
				m_ptP2.set(0,0,1);
				facevect=m_ptP2.asVector().normal();
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
				m_ptP2.set(facevect.x,facevect.y,facevect.z);
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
		}
		break;

	}

	double startDist, endDist;
	startDist = m_ptP1.distanceTo(p);
	AcGeVector3d startVect = (m_ptP1 - p).normal();
	double r1 = l1 / 2.0;
	double r2 = l2/2.0;

	switch(m_nDirection)
	{
	case 2:
		{
			if (startDist<r1 )
				m_ptP1=p+startVect*r1;

			AcGeVector3d facevect=m_ptP2.asVector().normal();
			AcGePoint3d P2=m_ptP1;
			P2.rotateBy(PI*2.0,facevect,p);
			AcGeVector3d endVect = (P2 - p).normal();
			endDist=P2.distanceTo(p);

			if (endDist<r2)
				m_dLength2=endDist*2.0;
		}
		break;
	case 3:
		{
			if (startDist<r1 )
				m_ptP1=p+startVect*r1;

			AcGeVector3d facevect=m_ptP2.asVector().normal();
			AcGePoint3d P2=m_ptP1;
			P2.rotateBy(PI,facevect,p);
			AcGeVector3d endVect = (P2 - p).normal();
			endDist=P2.distanceTo(p);

			if (endDist<r2)
				m_dLength2=endDist*2.0;
		}
		break;
	case 0:
	case 1:
		{
			AcGeVector3d endVect = (m_ptP2 - p).normal();
			endDist=m_ptP2.distanceTo(p);
			if (startDist<r1 && endDist<r2)
			{
				m_ptP1=p+startVect*r1;
				m_ptP2=p+endVect*r2;
			}
			else if (startDist<r1)
				m_ptP1=p+startVect*r1;
			else if (endDist<r2)
				m_ptP2=p+endVect*r2;
		}
		break;
	}
	m_dRadius = m_ptP1.distanceTo(m_ptP);

	AcGeVector3d v1,v2;
	v1 = (m_ptP1-m_ptP).normal();
	v2 = getNormal();
	if(CorrType || m_nDirection==0 || m_nDirection==1)
		m_v3 = v1.crossProduct(v2).normal();
	else
		m_v3 = v2.crossProduct(v1).normal();

#ifdef _USEAMODELER_
	createBody();
#endif

}//added by linlin 20050926

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDSqutorus::subTransformBy(const AcGeMatrix3d& xform)
#else
Acad::ErrorStatus PDSqutorus::transformBy(const AcGeMatrix3d& xform)
#endif
{
	assertWriteEnabled();
	// TODO: implement this function.
	m_ptP.transformBy(xform);
	m_ptP1.transformBy(xform);
	switch(m_nDirection)
	{
	case 0:
	case 1:
		m_ptP2.transformBy(xform);
		break;
	case 2:
	case 3:
		{
			AcGeVector3d vecTemp(m_ptP2.x, m_ptP2.y, m_ptP2.z);
			vecTemp.transformBy(xform);
			m_ptP2.set(vecTemp.x, vecTemp.y, vecTemp.z);
		}
		break;
	default:
		m_ptP2.transformBy(xform);
		break;
	}
	m_v3.transformBy(xform);

	m_dLength1 *= xform.scale();
	m_dLength2 *= xform.scale();
	m_dWidth1 *= xform.scale();
	m_dWidth2 *= xform.scale();
	m_dRadius *= xform.scale();

#ifdef _OBJECTARX2010_
	return PDPrimary3D::subTransformBy(xform);
#else
	return PDPrimary3D::transformBy(xform);
#endif
}

#ifdef _OBJECTARX2010_
Adesk::Boolean PDSqutorus::subWorldDraw(AcGiWorldDraw* mode)
#else
Adesk::Boolean PDSqutorus::worldDraw(AcGiWorldDraw* mode)
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
	int PreStep = 1;  
	AcGePoint3dArray pArray;
	int actPrecision__1 = Precision + 1;

	Adesk::Int16 nRating = getCalMaxPrecision();
	if(nRating == PRIMARY3D_SIMPLIZE_RATE1 ||
		nRating == PRIMARY3D_SIMPLIZE_RATE2)
	{
		Precision = (int)(PI / acos(1.0 - mode->deviation(kAcGiMaxDevForCircle, m_ptP1) / m_dRadius));
		if(Precision < PRIMARY3D_SIMPLIZE_SCALE)
		{
			if (nRating == PRIMARY3D_SIMPLIZE_RATE2)
			{
				switch(m_nDirection)
				{
				case 0:
				case 1:
					{
						AcGePoint3dArray pts = getPtOnCentAix(Precision);
						if(pts.length() ==0)
						{
							AcGeVector3d tmpNormal = getNormal();
							AcGeVector3d tmpStartVect = m_ptP1 - m_ptP;
							double angle = getAngle();
							mode->geometry().circularArc(m_ptP,m_dRadius,tmpNormal,tmpStartVect,angle);
						}
						else
						{
							mode->geometry().polyline(pts.length(),pts.asArrayPtr());
						}
					}
					break;
				case 2:
					{
						//整圆
						AcGeVector3d tmpNormal = getNormal();
						mode->geometry().circle(m_ptP,m_dRadius,tmpNormal);
					}
					break;
				case 3:
					{
						//半圆
						AcGeVector3d tmpNormal = getNormal();
						AcGeVector3d tmpStartVect = m_ptP1 - m_ptP;
						double angle = getAngle();
						mode->geometry().circularArc(m_ptP,m_dRadius,tmpNormal,tmpStartVect,angle);
					}
					break;
				default:
					break;
				}
			}
			else if (nRating == PRIMARY3D_SIMPLIZE_RATE1)
			{
				Precision = PRIMARY3D_WIREFRAME_LINENUM4;
				getVertices(Precision, pArray);
				actPrecision__1 = Precision + 1;

				AcGePoint3d pts[2];
				int i = 0,j = 0;
				for(i = 0; i < actPrecision__1; i += PreStep)
				{
					for(j =0; j <4; j++)
					{
						pts[0] = pArray[i+ actPrecision__1  * j];
						if (j==3)
							pts[1] = pArray[i];
						else
							pts[1] = pArray[i + actPrecision__1  * (j + 1)];
						if(mode->geometry().polyline(2, pts))
							return Adesk::kTrue;
					}
				}
				int nIndex = 0;
				if(m_nDirection == 2 && m_dLength1 == m_dLength2 && m_dWidth1 == m_dWidth2)
				{
					for(i =0; i <4; i++)
					{
						nIndex = i*actPrecision__1;
						if(mode->geometry().circle(pArray[nIndex], pArray[1 + nIndex], pArray[2 + nIndex]))
							return Adesk::kTrue;
					}
				}
				else
				{
					if(m_dLength1 == m_dLength2 && m_dWidth1 == m_dWidth2)
					{
						for(i =0; i <4; i++)
						{ 
							nIndex = i*actPrecision__1;
							if(mode->geometry().circularArc(pArray[nIndex], pArray[2 + nIndex], pArray[Precision + nIndex]))
								return Adesk::kTrue;
						}
					}
					else
					{	
						AcGePoint3dArray pts;
						for(i = 0; i < 4; i++)
						{
							pts.setLogicalLength(0);
							for(j = 0; j <actPrecision__1; j++)
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
			if(!(m_nDirection == 2 && m_dLength1 == m_dLength2 && m_dWidth1 == m_dWidth2))
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
				pts[1] = pArray[Precision+actPrecision__1];
				pts[2] = pArray[Precision+actPrecision__1*2];
				pts[3] = pArray[Precision+actPrecision__1*3];
				gsIndex += PRIMARY3D_WIREFRAME_LINENUM;
				//added by szw 2009.11.18 : begin
				mode->subEntityTraits().setSelectionMarker(gsIndex);
				//added by szw 2009.11.18 : end
				if(mode->geometry().polygon(4, pts))
					return Adesk::kTrue;
			}
			else//圆环的情况，使GS标记保持一致
				//added by szw 2009.11.18 : begin
				gsIndex += PRIMARY3D_WIREFRAME_LINENUM + 1;
				//added by szw 2009.11.18 : end

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
						edgeVisibility[i + actPrecision___2] = kAcGiVisible;

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

			int gsIndex = 0;
			AcGePoint3d pts[2];
			int i = 0,j = 0;
			//统一线框模型和实体模型的GS标记
			//modified by szw 2009.11.18 : begin
			for(i = 0; i < actPrecision__1; i += PreStep)
			{
				++gsIndex;
				for(j =0; j <4; j++)
				{
					pts[0] = pArray[i+ actPrecision__1  * j];
					if (j==3)
						pts[1] = pArray[i];
					else
						pts[1] = pArray[i + actPrecision__1  * (j + 1)];
					mode->subEntityTraits().setSelectionMarker(gsIndex);
					if(mode->geometry().polyline(2, pts))
						return Adesk::kTrue;
				}
			}

			int nIndex = 0;
			if(m_nDirection == 2 && m_dLength1 == m_dLength2 && m_dWidth1 == m_dWidth2)
			{
				++gsIndex;
				for(i =0; i <4; i++)
				{
					nIndex = i*actPrecision__1;
					mode->subEntityTraits().setSelectionMarker(gsIndex);
					if(mode->geometry().circle(pArray[nIndex], pArray[3 + nIndex], pArray[6 + nIndex]))
						return Adesk::kTrue;
				}
			}
			else
			{
				if(m_dLength1 == m_dLength2 && m_dWidth1 == m_dWidth2)
				{
					++gsIndex;
					for(i =0; i <4; i++)
					{ 
						nIndex = i*actPrecision__1;
						mode->subEntityTraits().setSelectionMarker(gsIndex);
						if(mode->geometry().circularArc(pArray[nIndex], pArray[4 + nIndex], pArray[Precision + nIndex]))
							return Adesk::kTrue;
					}
				}
				else
				{	
					AcGePoint3dArray pts;
					++gsIndex;
					for(i = 0; i < 4; i++)
					{
						pts.setLogicalLength(0);
						for(j = 0; j <actPrecision__1; j++)
							pts.append(pArray[i * (actPrecision__1) + j]);
						mode->subEntityTraits().setSelectionMarker(gsIndex);
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
/*Adesk::Boolean PDSqutorus::worldDraw(AcGiWorldDraw* mode)
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
int gsIndex = 0;
for(int i = 0; i <stdIdxLen; i++)
{
AcGePoint3d pts[2];
for(int j =0; j <4; j++)
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

if(m_nDirection == 2 && m_dLength1 == m_dLength2 && m_dWidth1 == m_dWidth2)
{
for(i =0; i <4; i++)
{
mode->subEntityTraits().setSelectionMarker(++gsIndex);
mode->geometry().circle(pArray[stdIdx[0]+i*actPrecision__1], pArray[stdIdx[3]+i*actPrecision__1], pArray[stdIdx[6]+i*actPrecision__1]);
}
}
else
{
if(m_dLength1 == m_dLength2 && m_dWidth1 == m_dWidth2)
{
for(i =0; i <4; i++)
{
mode->subEntityTraits().setSelectionMarker(++gsIndex);
mode->geometry().circularArc(pArray[stdIdx[0]+i*actPrecision__1], pArray[stdIdx[4]+i*actPrecision__1], pArray[stdIdx[stdIdxLen_1]+i*actPrecision__1]);
}
}
else
{
for(i = 0; i < 4; i++)
{
AcGePoint3dArray pts;
for(int j = 0; j <actPrecision__1; j++)
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
if(!(m_nDirection == 2 && m_dLength1 == m_dLength2 && m_dWidth1 == m_dWidth2))
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
// added by linlin 20050816

///////////////////////////////////////////////////////////////////////
//////	先将圆弧在角度方向上分成m_nN段							///////
//////	矩形的长度和宽度随角度递增线性变化，求出每段矩形长宽	///////
//////	再求出矩形的5个顶点，加入到输出数组中					///////
/////	得到5个棱线上的m_nN个离散点，共5*m_nN个点				///////
////////////////////////////////////////////////////////////////////////
Acad::ErrorStatus PDSqutorus::getVertices(int Precision, AcGePoint3dArray& vertexArray, 
										  AcGeIntArray &stdIdx, int &actPrecision) const
{
	assertReadEnabled();

	AcGeDoubleArray dividDbl;
	getActDivid(Precision, dividDbl, stdIdx);

	int actPrecision__1 = dividDbl.length();
	actPrecision = actPrecision__1 - 1;

	while(!vertexArray.isEmpty())
		vertexArray.removeLast();

	AcGeVector3d Facevect=getNormal();
	double dangle=getAngle();

	double startDist, endDist;
	startDist = m_ptP1.distanceTo(m_ptP);
	if(m_nDirection == 2 || m_nDirection == 3)
		endDist = startDist;
	else
		endDist = m_ptP2.distanceTo(m_ptP);

	double deltaL = (m_dLength2 - m_dLength1) /Precision;
	double deltaW = (m_dWidth2 - m_dWidth1) /Precision;

	AcGePoint3dArray vertexArray1,vertexArray2,vertexArray3,vertexArray4;
	AcGeVector3d v1,v2;
	v1 = (m_ptP1 - m_ptP).normal();
	v2 = getNormal();
	if(isLeftCoord(v1,v2,m_v3))
		Facevect *= -1;
	for(int i = 0; i <=actPrecision; i++)
	{
		AcGePoint3d varCenter;
		double varL,varW;
		AcGePoint3d point, point1;

		varCenter = m_ptP + 
			(m_ptP1 - m_ptP).rotateBy(dangle / Precision * dividDbl[i], Facevect).normal() * 
			(startDist + (endDist - startDist) / Precision * dividDbl[i]);


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
Acad::ErrorStatus PDSqutorus::getVertices(int Precision, AcGePoint3dArray& vertexArray) const
{
	assertReadEnabled();
	vertexArray.setLogicalLength(0);

	AcGeVector3d Facevect=getNormal();
	double dangle=getAngle();

	double startDist, endDist;
	startDist = m_ptP1.distanceTo(m_ptP);
	if(m_nDirection == 2 || m_nDirection == 3)
		endDist = startDist;
	else
		endDist = m_ptP2.distanceTo(m_ptP);

	double deltaL = (m_dLength2 - m_dLength1) /Precision;
	double deltaW = (m_dWidth2 - m_dWidth1) /Precision;

	AcGePoint3dArray vertexArray1,vertexArray2,vertexArray3,vertexArray4;
	AcGeVector3d v1,v2;
	v1 = (m_ptP1 - m_ptP).normal();
	v2 = getNormal();
	if(isLeftCoord(v1,v2,m_v3))
		Facevect *= -1;
	AcGePoint3d varCenter;
	double varL,varW;
	AcGePoint3d point, point1;
	double tmpAnglr = dangle / Precision;
	double tmpDeta = (endDist - startDist) / Precision;
	AcGeVector3d tmpVec = m_ptP1 - m_ptP;
	//double tmpLeng = startDist;
	AcGeVector3d Centervect;
	varL = m_dLength1;
	varW =m_dWidth1;
	for(int i = 0; i <=Precision; i++)
	{
		tmpVec.normalize();
		varCenter = m_ptP + tmpVec*startDist/*tmpLeng*/; 

		Centervect= (varCenter-m_ptP).normal();
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
		//之前顶点获取的方法存在问题，矩形框尺寸存在偏差
		//modified by szw 2009.11.18 : begin
		varL +=deltaL;
		varW +=deltaW;
		//modified by szw 2009.11.18 : end

		tmpVec = tmpVec.rotateBy(tmpAnglr,Facevect);
		//tmpLeng += tmpDeta;
		startDist += tmpDeta;
	}
	//////////////////////////////////////////////////////
	//////	将5个棱线离散点合并到输出数组中

	vertexArray.append(vertexArray1);
	vertexArray.append(vertexArray2);
	vertexArray.append(vertexArray3);
	vertexArray.append(vertexArray4);

	return Acad::eOk;
}
// added by linlin 20050816

Acad::ErrorStatus PDSqutorus::dwgOutFields(AcDbDwgFiler* filer)const
{
	assertReadEnabled();
	Acad::ErrorStatus es=Acad::eOk;

	// Call dwgOutFields from father class: PDPrimary3D
	if((es=PDPrimary3D::dwgOutFields(filer))!=Acad::eOk)
	{
		return es;
	}

	// Write version number.
	filer->writeItem((Adesk::UInt16) VERSION_PDSQUTORUS);

	// version 2
	filer->writeItem(m_v3);

	// version 1
	// Write the data members.
	filer->writeItem(m_ptP);
	filer->writeItem(m_ptP1);
	filer->writeItem(m_ptP2);
	filer->writeItem(m_dLength1);
	filer->writeItem(m_dLength2);
	filer->writeItem(m_dWidth1);
	filer->writeItem(m_dWidth2);
	filer->writeItem(m_dRadius);
	filer->writeItem(m_nDirection);
	filer->writeItem(m_nN);

	return filer->filerStatus();
}

Acad::ErrorStatus PDSqutorus::dwgInFields(AcDbDwgFiler* filer)
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
	if (version > VERSION_PDSQUTORUS)
		return Acad::eMakeMeProxy;

	if(version >= 2)
		filer->readItem(&m_v3);
	// Read the data members.
	if (version >= 1)
	{
		filer->readItem(&m_ptP);
		filer->readItem(&m_ptP1);
		filer->readItem(&m_ptP2);
		filer->readItem(&m_dLength1);
		filer->readItem(&m_dLength2);
		filer->readItem(&m_dWidth1);
		filer->readItem(&m_dWidth2);
		filer->readItem(&m_dRadius);
		filer->readItem(&m_nDirection);
		filer->readItem(&m_nN);
		if(version==1)
			m_v3 = (m_ptP1-m_ptP).normal().crossProduct(getNormal());

	}

#ifdef _USEAMODELER_
	if(m_3dGeom.isNull())
		createBody();
#endif

	return filer->filerStatus();
}


Acad::ErrorStatus PDSqutorus::dxfOutFields(AcDbDxfFiler* filer) const
{
	assertReadEnabled();
	Acad::ErrorStatus es;

	if ((es = PDPrimary3D::dxfOutFields(filer))
		!= Acad::eOk)
	{
		return es;
	}

	// Write subclass marker.
	filer->writeItem(AcDb::kDxfSubclass, _T("PDSqutorus"));

	// Write version number.
	filer->writeItem(AcDb::kDxfInt16, (Adesk::UInt16) VERSION_PDSQUTORUS);

	// version 2
	filer->writeItem(AcDb::kDxfXCoord+3, m_v3);

	// version 1
	// Write data members.
	filer->writeItem(AcDb::kDxfXCoord, m_ptP);
	filer->writeItem(AcDb::kDxfXCoord+1, m_ptP1);
	filer->writeItem(AcDb::kDxfXCoord+2, m_ptP2);
	filer->writeItem(AcDb::kDxfReal,m_dLength1);
	filer->writeItem(AcDb::kDxfReal+1,m_dLength2);
	filer->writeItem(AcDb::kDxfReal+2,m_dWidth1);
	filer->writeItem(AcDb::kDxfReal+3,m_dWidth2);
	filer->writeItem(AcDb::kDxfReal+4,m_dRadius);
	filer->writeItem(AcDb::kDxfInt32,m_nDirection);
	filer->writeItem(AcDb::kDxfInt32+1,m_nN);

	return filer->filerStatus();
}

Acad::ErrorStatus PDSqutorus::dxfInFields(AcDbDxfFiler* filer)
{
	assertWriteEnabled();
	Acad::ErrorStatus es = Acad::eOk;
	struct resbuf rb;

	if ((PDPrimary3D::dxfInFields(filer) != Acad::eOk)
		|| !filer->atSubclassData(_T("PDSqutorus")))
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
		if (version > VERSION_PDSQUTORUS)
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
			m_v3 = asVec3d(rb.resval.rpoint);
		}
	}
	// Read the data members.
	if(version >= 1){
		while ((es == Acad::eOk) && ((es = filer->readResBuf(&rb)) == Acad::eOk)){
			switch (rb.restype){
			case AcDb::kDxfXCoord:
				m_ptP = asPnt3d(rb.resval.rpoint);
				break;

			case AcDb::kDxfXCoord+1:
				m_ptP1 = asPnt3d(rb.resval.rpoint);
				break;

			case AcDb::kDxfXCoord+2:
				m_ptP2 = asPnt3d(rb.resval.rpoint);
				break;

			case AcDb::kDxfReal:
				m_dLength1 = rb.resval.rreal;
				break;

			case AcDb::kDxfReal+1:
				m_dLength2 = rb.resval.rreal;
				break;

			case AcDb::kDxfReal+2:
				m_dWidth1 = rb.resval.rreal;
				break;

			case AcDb::kDxfReal+3:
				m_dWidth2 = rb.resval.rreal;
				break;

			case AcDb::kDxfReal+4:
				m_dRadius = rb.resval.rreal;
				break;

			case AcDb::kDxfInt32:
				m_nDirection = rb.resval.rlong;
				break;

			case AcDb::kDxfInt32+1:
				m_nN = rb.resval.rlong;
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

	return filer->filerStatus();
}


Acad::ErrorStatus PDSqutorus::getpointP(AcGePoint3d& ptP)
{
	assertReadEnabled();
	ptP = m_ptP;
	return Acad::eOk;
}


Acad::ErrorStatus PDSqutorus::getpointP1(AcGePoint3d& ptP1)
{
	assertReadEnabled();
	ptP1 = m_ptP1;
	return Acad::eOk;
}

Acad::ErrorStatus PDSqutorus::getpointP2(AcGePoint3d& ptP2)
{
	assertReadEnabled();
	ptP2 = m_ptP2;
	return Acad::eOk;
}

Acad::ErrorStatus PDSqutorus::getLength1(double& length1)
{
	assertReadEnabled();
	length1 = m_dLength1;
	return Acad::eOk;
}

Acad::ErrorStatus PDSqutorus::getLength2(double& length2)
{
	assertReadEnabled();
	length2 = m_dLength2;
	return Acad::eOk;
}

Acad::ErrorStatus PDSqutorus::getWidth1(double& width1)
{
	assertReadEnabled();
	width1 = m_dWidth1;
	return Acad::eOk;
}

Acad::ErrorStatus PDSqutorus::getWidth2(double& width2)
{
	assertReadEnabled();
	width2 = m_dWidth2;
	return Acad::eOk;
}

Acad::ErrorStatus PDSqutorus::getRadius(double& radius)
{
	assertReadEnabled();
	radius = m_dRadius;
	return Acad::eOk;
}

Acad::ErrorStatus PDSqutorus::getDirection(Adesk::UInt32& direction)
{
	assertReadEnabled();
	direction = m_nDirection;
	return Acad::eOk;
}

Acad::ErrorStatus PDSqutorus::getN(Adesk::UInt32& n)
{
	assertReadEnabled();
	n = m_dDividPrecision;
	return Acad::eOk;
}

// 取得环中心到圆环起点的矢量
AcGeVector3d PDSqutorus::getStartCenVect() const
{
	assertReadEnabled();
	return (m_ptP1 - m_ptP).normal();
}//added by linlin 20050913

// 取得环中心到圆环终点的矢量
AcGeVector3d PDSqutorus::getEndCenVect() const
{
	assertReadEnabled();
	if(m_nDirection == 2)
		return (m_ptP1 - m_ptP).normal();
	else if(m_nDirection == 3)
		return (m_ptP - m_ptP1).normal();
	else
		return (m_ptP2 - m_ptP).normal();
}//added by linlin 20050913

AcGePoint3d PDSqutorus::getEndPoint()const
{
	assertReadEnabled();
	if (m_nDirection == 2)
		return m_ptP1;
	else if (m_nDirection == 3)
		return m_ptP + (m_ptP - m_ptP1);
	else
		return m_ptP2;
}

Acad::ErrorStatus PDSqutorus:: setParameters(AcGePoint3d ptP,AcGePoint3d ptP1,AcGePoint3d ptP2,
											 double length1,double length2,double width1,
											 double width2,double radius,Adesk::UInt32 direction,
											 Adesk::UInt32 n,int CorrType/*0-左手系，1-右手系*/)
{

	assertWriteEnabled();
	m_ptP = ptP ;
	m_ptP1 = ptP1 ;
	m_ptP2 = ptP2 ;
	m_dLength1 =fabs(length1) ;
	if(m_dLength1==0)
		m_dLength1=1;

	m_dLength2 =fabs(length2)  ;
	if(m_dLength2==0)
		m_dLength2=1;

	m_dWidth1 =fabs(width1)  ;
	if(width1==0)
		m_dWidth1=1;

	m_dWidth2 =fabs(width2)  ;
	if(m_dWidth2==0)
		m_dWidth2=1;

	m_dRadius = radius ;

	if(n > 2)
		m_dDividPrecision = n;
	else
		m_dDividPrecision = PRIMARY3D_DEFAULT_PRECISION;


	if(direction >= 0 && direction <= 3)
		m_nDirection = direction;
	else
		m_nDirection = 0;

	switch(m_nDirection)
	{
	case 0:
	case 1:
		{
			AcGeVector3d p0p1, p0p2;
			p0p1 = ptP1-ptP;
			p0p2 = ptP2-ptP;
			if(p0p1.isZeroLength() && p0p2.isZeroLength())
			{
				m_ptP1 = ptP + AcGeVector3d(0, 1, 0);
				m_ptP2 = ptP + AcGeVector3d(1, 0, 0);
			}
			else if(p0p2.isZeroLength())
			{
				AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax, Ay;
				if (fabs(p0p1[X]) <1.0/64 && fabs(p0p1[Y])<1.0/64) 
					Ax = Wy.crossProduct(p0p1);
				else
					Ax = Wz.crossProduct(p0p1);
				Ax.normalize();

				p0p2 = Ax;
				m_ptP2 = ptP + p0p2;
			}
			else if(p0p1.isZeroLength())
			{
				AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax, Ay;
				if (fabs(p0p2[X]) <1.0/64 && fabs(p0p2[Y])<1.0/64) 
					Ax = Wy.crossProduct(p0p2);
				else
					Ax = Wz.crossProduct(p0p2);
				Ax.normalize();

				p0p1 = Ax;
				m_ptP1 = ptP+ p0p1;
			}
			else if(p0p1.isParallelTo(p0p2))
			{
				AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax, Ay;
				if (fabs(p0p2[X]) <1.0/64 && fabs(p0p2[Y])<1.0/64) 
					Ax = Wy.crossProduct(p0p2);
				else
					Ax = Wz.crossProduct(p0p2);
				Ax.normalize();

				p0p1 = Ax *p0p1.length();
				m_ptP1 = ptP + p0p1;
			}
		}
		break;

	case 2:
	case 3:
		{
			AcGeVector3d p0p1,facevect;
			p0p1 = ptP1-ptP;
			facevect=ptP2.asVector().normal();
			if(p0p1.isZeroLength() && facevect.isZeroLength())
			{
				m_ptP1 = ptP + AcGeVector3d(1, 0, 0);
				m_ptP2.set(0,0,1);
				facevect=m_ptP2.asVector().normal();
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
				m_ptP2.set(facevect.x,facevect.y,facevect.z);
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
		}
		break;

	}

	double startDist, endDist;
	startDist = m_ptP1.distanceTo(ptP);
	AcGeVector3d startVect = (m_ptP1 - ptP).normal();
	double r1 = length1 / 2.0;
	double r2 = length2/2.0;

	switch(m_nDirection)
	{
	case 2:
		{
			if (startDist<r1 )
				m_ptP1=ptP+startVect*r1;

			AcGeVector3d facevect=m_ptP2.asVector().normal();
			AcGePoint3d P2=m_ptP1;
			P2.rotateBy(PI*2.0,facevect,ptP);
			AcGeVector3d endVect = (P2 - ptP).normal();
			endDist=P2.distanceTo(ptP);

			if (endDist<r2)
				m_dLength2=endDist*2.0;
		}
		break;
	case 3:
		{
			if (startDist<r1 )
				m_ptP1=ptP+startVect*r1;

			AcGeVector3d facevect=m_ptP2.asVector().normal();
			AcGePoint3d P2=m_ptP1;
			P2.rotateBy(PI,facevect,ptP);
			AcGeVector3d endVect = (P2 - ptP).normal();
			endDist=P2.distanceTo(ptP);

			if (endDist<r2)
				m_dLength2=endDist*2.0;
		}
		break;
	case 0:
	case 1:
		{
			AcGeVector3d endVect = (m_ptP2 - ptP).normal();
			endDist=m_ptP2.distanceTo(ptP);
			if (startDist<r1 && endDist<r2)
			{
				m_ptP1=ptP+startVect*r1;
				m_ptP2=ptP+endVect*r2;
			}
			else if (startDist<r1)
				m_ptP1=ptP+startVect*r1;
			else if (endDist<r2)
				m_ptP2=ptP+endVect*r2;
		}
		break;
	}
	AcGeVector3d v1,v2;
	v1 = (m_ptP1-m_ptP).normal();
	v2 = getNormal();
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
Acad::ErrorStatus PDSqutorus::subGetOsnapPoints(
	AcDb::OsnapMode     osnapMode,
	Adesk::GsMarker     gsSelectionMark,
	const AcGePoint3d&  pickPoint,
	const AcGePoint3d&  lastPoint,
	const AcGeMatrix3d& viewXform,
	AcGePoint3dArray&   snapPoints,
	AcDbIntArray &   geomIds) const
#else
Acad::ErrorStatus PDSqutorus::getOsnapPoints(
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

	AcGeVector3d Startvect=(m_ptP1-m_ptP).normal();
	AcGeVector3d Endvect=(m_ptP2-m_ptP).normal();
	AcGeVector3d Facevect=getNormal();

	int arcN, lineN;
	arcN = (gsSelectionMark_int - 1) / 4;
	lineN = (gsSelectionMark_int - 1) % 4;
	AcGePoint3d p1,p2;

	switch(osnapMode)
	{
	case AcDb::kOsModeEnd:
		{
			switch(m_nDirection)
			{
			case 0:
			case 1:
				snapPoints.append(m_ptP1);
				snapPoints.append(m_ptP2);
				break;
			case 3:
				{
					snapPoints.append(m_ptP1);
					AcGePoint3d pt = m_ptP1;
					pt.rotateBy(PI, Facevect, m_ptP);
					snapPoints.append(pt);
				}
				break;
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
		if(m_nDirection == 2 && gsSelectionMark_int > stdIdxLen && m_dLength1 == m_dLength2 && m_dWidth1 == m_dWidth2)
		{
			snapPoints.append(pArray[stdIdx[0]+(gsSelectionMark_int -  stdIdxLen -1) * actPrecision__1]);
			snapPoints.append(pArray[stdIdx[2]+(gsSelectionMark_int -  stdIdxLen -1) * actPrecision__1]);
			snapPoints.append(pArray[stdIdx[4]+(gsSelectionMark_int -  stdIdxLen -1) * actPrecision__1]);
			snapPoints.append(pArray[stdIdx[6]+(gsSelectionMark_int -  stdIdxLen -1) * actPrecision__1]);
		}
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
				if(m_nDirection == 2 && m_dLength1 == m_dLength2 && m_dWidth1 == m_dWidth2)
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
				if(m_nDirection == 2 && m_dLength1 == m_dLength2 && m_dWidth1 == m_dWidth2)
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
// added by linlin 20050816


#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDSqutorus::subGetGripPoints(
	AcGePoint3dArray& gripPoints,
	AcDbIntArray& osnapModes,
	AcDbIntArray& geomIds) const
#else
Acad::ErrorStatus PDSqutorus::getGripPoints(
	AcGePoint3dArray& gripPoints,
	AcDbIntArray& osnapModes,
	AcDbIntArray& geomIds) const
#endif
{
	assertReadEnabled();
	/*	AcGeVector3d Startvect=(m_ptP1-m_ptP).normal();
	AcGeVector3d Endvect=(m_ptP2-m_ptP).normal();
	AcGeVector3d Facevect;
	switch(m_nDirection)
	{
	case 0:
	case 1:
	Facevect = Startvect.crossProduct(Endvect).normal();
	break;
	case 2:
	case 3:
	Facevect.set(m_ptP2.x, m_ptP2.y, m_ptP2.z).normalize();
	break;
	default:
	Facevect = Startvect.crossProduct(Endvect).normal();
	break;
	}
	//	AcGeVector3d Centervect;
	AcGeCubicSplineCurve3d varcur;

	double   angle;
	/////////////////////////////////////////////////////////////////////
	/////	根据控制变量的不同，得到不同的圆环形式					/////
	/////////////////////////////////////////////////////////////////////
	switch(m_nDirection){
	case 0:
	angle=Endvect.angleTo(Startvect);
	varcur = AcGeCubicSplineCurve3d(AcGeCircArc3d(
	m_ptP,Facevect,Startvect,m_dRadius,0,angle));
	break;
	case 1:
	angle=2*PI-Endvect.angleTo(Startvect);
	varcur = AcGeCubicSplineCurve3d(AcGeCircArc3d(
	m_ptP,-Facevect,Startvect,m_dRadius,0,angle));
	break;
	case 2:
	angle=2*PI;
	//Facevect = Endvect;
	varcur = AcGeCubicSplineCurve3d(AcGeCircArc3d(
	m_ptP,Facevect,m_dRadius));
	break;
	case 3:
	angle=PI;
	//Facevect = Endvect;
	//Startvect = Startvect.orthoProject(Facevect);
	varcur = AcGeCubicSplineCurve3d(AcGeCircArc3d(
	m_ptP,Facevect,Startvect,m_dRadius,0,angle));
	break;
	default:
	break;
	}
	AcGePoint3d pt;
	pt = varcur.startPoint();
	gripPoints.append(pt);
	pt = varcur.endPoint();
	gripPoints.append(pt);
	*/
	return Acad::eOk; 
}

/*
Acad::ErrorStatus PDSqutorus::explode(AcDbVoidPtrArray& entitySet) const
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

AcGeVector3d Startvect=(m_ptP1-m_ptP).normal();
AcGeVector3d Endvect=(m_ptP2-m_ptP).normal();
AcGeVector3d Facevect=getNormal();

if(Facevect == AcGeVector3d::kIdentity)
return Acad::eNotApplicable;

double startDist, endDist;
startDist = m_ptP1.distanceTo(m_ptP);
if(m_nDirection == 2 || m_nDirection == 3)
endDist = startDist;
else
endDist = m_ptP2.distanceTo(m_ptP);

AcDbCurve *pCurve = NULL;
double pathLen;
if(fabs(startDist - endDist) < 1e-4)
{
switch(m_nDirection)
{
case 2: 
if(m_dLength1==m_dLength2&&m_dWidth1==m_dWidth2)
{
AcDbCircle *pCir;
pCir = new AcDbCircle(m_ptP, Facevect, startDist);
pathLen = 2.0 * PI * startDist;
pCurve = pCir;
}
else
return explodeToSurface(entitySet);
break;
case 3:
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
break;

case 0:
case 1:

default:

if(m_dLength1==m_dLength2&&m_dWidth1==m_dWidth2)
{
AcGeVector3d mnormal=Facevect;

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
endAng = Ax.angleTo(m_ptP2 - m_ptP, mnormal);

AcDbArc *pArc;
pArc = new AcDbArc(m_ptP, mnormal, startDist, startAng, endAng);
double endParam;
pArc->getEndParam(endParam);
pArc->getDistAtParam(endParam, pathLen);
pCurve = pArc;
}
else
return explodeToSurface(entitySet);
break;
}
}
else
{
AcGeDoubleArray dividDblArc;
AcGeIntArray stdArcIdx;
getActDivid(m_dDividPrecision, dividDblArc, stdArcIdx);
double dangle=getAngle();

int precisionArc__1 = dividDblArc.length();
AcDbPolyline *pPolyline = NULL;
pPolyline = new AcDbPolyline(precisionArc__1);

double deltaAngle = dangle / m_dDividPrecision;
double deltaDist = (endDist - startDist) / m_dDividPrecision;

for(int i = 0; i <precisionArc__1; ++i)
{
AcGePoint3d varCenter;

varCenter = m_ptP + 
(m_ptP1-m_ptP).rotateBy(deltaAngle * dividDblArc[i], Facevect) * 
(startDist + deltaDist * dividDblArc[i]);

ads_point wcs_p, ecs_p, norm;
norm[0] = Facevect.x;
norm[1] = Facevect.y;
norm[2] = Facevect.z;
wcs_p[0] = varCenter.x;
wcs_p[1] = varCenter.y;
wcs_p[2] = varCenter.z;
acdbWcs2Ecs(wcs_p, ecs_p, norm, 0);
AcGePoint2d verPt(ecs_p[0], ecs_p[1]);

pPolyline->addVertexAt(i, verPt);	  
}
pPolyline->setNormal(Facevect);
double endParam;
pPolyline->getEndParam(endParam);
pPolyline->getDistAtParam(endParam, pathLen);
pCurve = pPolyline;

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
atan((m_dLength1 / 2.0 - m_dLength2 / 2.0) / pathLen)
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
}//added by linlin 20050915
*/

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDSqutorus::subExplode(AcDbVoidPtrArray& entitySet) const
#else
Acad::ErrorStatus PDSqutorus::explode(AcDbVoidPtrArray& entitySet) const
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
		double val = m_ptP2.distanceTo(m_ptP);//m_nDirection!=2,3

		//如果m_ptP1和m_ptP2到m_ptP的距离不等，则用面表示
		if (m_nDirection<2 && (fabs(arcRadius-val)>SIGMA))
		{
			return explodeToSurface(entitySet);
		}

		//path
		Acad::ErrorStatus es = Acad::eOk;
		AcDbCurve *pCurve = NULL;
		double pathLen;

		switch(m_nDirection)
		{
		case 2: 
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
			break;
		case 3:
			{
				{
					AcDbArc *pArc;
					AcGePoint3d ptCenter(arcRadius, 0,0);
					AcGeVector3d faceNor(0,1,0);
					pArc = new AcDbArc(ptCenter, faceNor, arcRadius, 0, PI);
					pathLen = PI * arcRadius;
					pCurve = pArc;
				}
			}
			break;

		case 0:
		case 1:
		default:
			{
				AcGePoint3d ptCenter(arcRadius, 0,0);
				AcGeVector3d faceNor(0,1,0);
				double angle = getAngle();
				AcDbArc *pArc;
				pArc = new AcDbArc(ptCenter, faceNor, arcRadius, 0, angle);
				pathLen = arcRadius * angle;
				pCurve = pArc;
			}
			break;
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
			AcGeVector3d Facevect = getNormal();
			AcGeVector3d v1,v2;
			v1 = (m_ptP1 - m_ptP).normal();
			v2 = getNormal();
			if(isLeftCoord(v1,v2,m_v3))
				Facevect = -Facevect;

			if (m_nDirection==0 || m_nDirection==1)
			{
				if (m_dLength1 >= m_dLength2)
				{
					pOrign = m_ptP2;

					axisX = (m_ptP-m_ptP2).normal();
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
			else if(m_nDirection==2)
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
			else//==3
			{
				if (m_dLength1 >= m_dLength2)
				{
					pOrign.x = 2*m_ptP.x - m_ptP1.x;
					pOrign.y = 2*m_ptP.y - m_ptP1.y;
					pOrign.z = 2*m_ptP.z - m_ptP1.z;

					axisX = (m_ptP-pOrign).normal();
					axisY = -1*Facevect;
				}
				else
				{
					pOrign = m_ptP1;

					axisX = (m_ptP-m_ptP1).normal();
					axisY = Facevect;
				}
				axisZ = axisX.crossProduct(axisY);
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
}

//  [9/5/2007 suzhiyong]
Acad::ErrorStatus PDSqutorus::explodeTo3DSolid(AcDb3dSolid* &p3dSolid) const
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
		double val = m_ptP2.distanceTo(m_ptP);//m_nDirection!=2,3

		//如果m_ptP1和m_ptP2到m_ptP的距离不等，则用面表示
		if (m_nDirection<2 && (fabs(arcRadius-val)>SIGMA))
		{
			return Acad::eNotApplicable;
		}

		//path
		Acad::ErrorStatus es = Acad::eOk;
		AcDbCurve *pCurve = NULL;
		double pathLen;

		switch(m_nDirection)
		{
		case 2: 
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
			break;
		case 3:
			{
				{
					AcDbArc *pArc;
					AcGePoint3d ptCenter(arcRadius, 0,0);
					AcGeVector3d faceNor(0,1,0);
					pArc = new AcDbArc(ptCenter, faceNor, arcRadius, 0, PI);
					pathLen = PI * arcRadius;
					pCurve = pArc;
				}
			}
			break;

		case 0:
		case 1:
		default:
			{
				AcGePoint3d ptCenter(arcRadius, 0,0);
				AcGeVector3d faceNor(0,1,0);
				double angle = getAngle();
				AcDbArc *pArc;
				pArc = new AcDbArc(ptCenter, faceNor, arcRadius, 0, angle);
				pathLen = arcRadius * angle;
				pCurve = pArc;
			}
			break;
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
			AcGeVector3d Facevect = getNormal();
			AcGeVector3d v1,v2;
			v1 = (m_ptP1 - m_ptP).normal();
			v2 = getNormal();
			if(isLeftCoord(v1,v2,m_v3))
				Facevect = -Facevect;

			if (m_nDirection==0 || m_nDirection==1)
			{
				if (m_dLength1 >= m_dLength2)
				{
					pOrign = m_ptP2;

					axisX = (m_ptP-m_ptP2).normal();
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
			else if(m_nDirection==2)
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
			else//==3
			{
				if (m_dLength1 >= m_dLength2)
				{
					pOrign.x = 2*m_ptP.x - m_ptP1.x;
					pOrign.y = 2*m_ptP.y - m_ptP1.y;
					pOrign.z = 2*m_ptP.z - m_ptP1.z;

					axisX = (m_ptP-pOrign).normal();
					axisY = -1*Facevect;
				}
				else
				{
					pOrign = m_ptP1;

					axisX = (m_ptP-m_ptP1).normal();
					axisY = Facevect;
				}
				axisZ = axisX.crossProduct(axisY);
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

Acad::ErrorStatus PDSqutorus::explodeToSurface(AcDbVoidPtrArray& entitySet) const
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

	AcGeVector3d faceVect = getNormal();
	AcGeVector3d curvect;

	curvect = getStartCenVect().crossProduct(faceVect).normal();
	ucs_mat.setToWorldToPlane(curvect);
	//#ifdef _OBJECTARX14_
	//    acdbSetCurrentUCS(ucs_mat);
	//#else
	//    acedSetCurrentUCS(ucs_mat);
	//#endif

	AcGePoint3d point1, point2, point3, point4;
	AcGeVector3d Startvect=getStartCenVect();
	AcGeVector3d Endvect=getEndCenVect();

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

	curvect = -(getEndCenVect().crossProduct(faceVect).normal());
	ucs_mat.setToWorldToPlane(curvect);
	//#ifdef _OBJECTARX14_
	//    acdbSetCurrentUCS(ucs_mat);
	//#else
	//    acedSetCurrentUCS(ucs_mat);
	//#endif

	switch(m_nDirection)
	{
	case 0:
	case 1:
		point1 = m_ptP2 - Endvect * m_dLength2/ 2.0 - faceVect * m_dWidth2 / 2.0;
		break;

	case 2:
		point1 = m_ptP1 - Endvect * m_dLength2/ 2.0 - faceVect * m_dWidth2 / 2.0;
		break;

	case 3:
		{
			AcGePoint3d P2=m_ptP1;
			P2.rotateBy(PI,faceVect,m_ptP);
			point1 = P2 - Endvect * m_dLength2/ 2.0 - faceVect * m_dWidth2 / 2.0;
		}  
		break;

	}

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
	AcGeIntArray stdArcIdx, stdCirIdx;
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
Acad::ErrorStatus PDSqutorus::subGetGeomExtents(AcDbExtents& extents) const
#else
Acad::ErrorStatus PDSqutorus::getGeomExtents(AcDbExtents& extents) const
#endif
{
	assertReadEnabled();
	extents.set(m_ptP1, m_ptP1);

	int   dividSegNum = m_dDividPrecision;
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
void PDSqutorus::subList() const
#else
void PDSqutorus::list() const
#endif
{
	assertReadEnabled();
#ifdef _OBJECTARX2010_
	PDPrimary3D::subList();
#else
	PDPrimary3D::list();
#endif
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4808/*"\n 起始矩形的中心(%f,%f,%f)\n"*/,m_ptP1.x,m_ptP1.y,m_ptP1.z);
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4809/*" 起始矩形的长度=%.3f 起始矩形的宽度=%.3f\n"*/,m_dLength1,m_dWidth1);
	if (m_nDirection==2||m_nDirection==3)
		ads_printf(LANGUAGE_DEFINE_MACRO_V25_4810/*" 法线方向(%f,%f,%f)\n"*/,m_ptP2.x,m_ptP2.y,m_ptP2.z);
	else
		ads_printf(LANGUAGE_DEFINE_MACRO_V25_4811/*" 终止矩形的中心(%f,%f,%f)\n"*/,m_ptP2.x,m_ptP2.y,m_ptP2.z);
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4812/*" 终止矩形的长度=%.3f 终止矩形的宽度=%.3f\n"*/,m_dLength2,m_dWidth2);
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4813/*" 圆环圆心(%f,%f,%f)\n"*/,m_ptP.x,m_ptP.y,m_ptP.z);
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4814/*" 圆环半径=%.3f\n"*/,m_dRadius);
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4815/*" 方向值:       %d\n"*/,m_nDirection);
	return ;
}// added by linlin 20050810

// 取得圆环的法矢
AcGeVector3d PDSqutorus::getNormal() const
{
	assertReadEnabled();

	AcGeVector3d Startvect=(m_ptP1-m_ptP).normal();
	AcGeVector3d Endvect=(m_ptP2-m_ptP).normal();
	AcGeVector3d Facevect;
	switch(m_nDirection)
	{
	case 2:
	case 3:
		Facevect.set(m_ptP2.x, m_ptP2.y, m_ptP2.z).normalize();
		break;
	case 0:
	case 1:
	default:
		{
			if(Startvect.isParallelTo(Endvect))
			{
				AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax;// Ay;
				if (fabs(Startvect[X]) <1.0/64 && fabs(Startvect[Y])<1.0/64) 
					Ax = Wy.crossProduct(Startvect);
				else
					Ax = Wz.crossProduct(Startvect);
				Ax.normalize();

				Facevect = Ax;
				if(m_nDirection == 1)
					Facevect = -Facevect;
			}
			else
			{
				Facevect =  Startvect.crossProduct(Endvect).normal();
				if(m_nDirection == 1)
					Facevect = -Facevect;
			}
		}
		break;
	}
	return Facevect;

}//added by linlin 20050824

AcGeVector3d PDSqutorus::getStartPlaneNormal() const
{
	assertReadEnabled();

	AcGeVector3d torusNormal = getNormal();
	AcGeVector3d startVect = (m_ptP1 - m_ptP).normal();
	return startVect.crossProduct(torusNormal).normal();
}

AcGeVector3d PDSqutorus::getEndPlaneNormal() const
{
	assertReadEnabled();

	AcGeVector3d result;
	switch (m_nDirection)
	{
	case 2:
		result = -getStartPlaneNormal();
		break;
	case 3:
		result = getStartPlaneNormal();
		break;
	case 0:
	case 1:
		AcGeVector3d torusNormal = getNormal();
		AcGeVector3d endVect = (m_ptP2 - m_ptP).normal();
		result = torusNormal.crossProduct(endVect).normal();
		break;
	}
	return result;
}


// 取得圆环的扫过的角度
double PDSqutorus::getAngle() const
{
	assertReadEnabled();
	AcGeVector3d Startvect=(m_ptP1-m_ptP).normal();
	AcGeVector3d Endvect=(m_ptP2-m_ptP).normal();
	double dangle;
	switch(m_nDirection)
	{
	case 1:
		if(Startvect.isCodirectionalTo(Endvect))
			dangle = 2.0 * PI;
		else
			dangle = 2.0 * PI - Endvect.angleTo(Startvect);
		break;
	case 2:
		dangle = 2.0 * PI;
		break;
	case 3:
		dangle = PI;
		break;
	case 0:
	default:
		if(Startvect.isCodirectionalTo(Endvect))
			dangle = 2.0 * PI;
		else
			dangle = Endvect.angleTo(Startvect);
		break;
	}
	return dangle;
}//added by linlin 20050824

void PDSqutorus::createBody()
{
	assertWriteEnabled();
#ifdef _OBJECTARX2004_

	/*    AcGePoint3dArray pArray;
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
	/*
	AcGePoint3dArray pArray;
	AcGeIntArray stdArcIdx, stdCirIdx;
	int actPrecisionArc;
	getVertices(m_dDividPrecision, pArray, 
	stdArcIdx, actPrecisionArc);

	AcGePoint3d* vertices;
	PolygonVertexData** vertexData;
	Body startProfile, endProfile;

	int n = pArray.length()/5;
	vertices = new AcGePoint3d[n];
	vertexData = new PolygonVertexData*[n];

	int i;
	for( i =0; i<4; ++i){
	vertices[i] = pArray[ i*n];
	vertexData[i] =NULL;
	}

	Face *f;
	int type;

	AcGeVector3d vn = ( vertices[1]-vertices[0]).crossProduct( vertices[2]-vertices[0]);
	AcGeVector3d vp = ( pArray[1] - pArray[0]);
	if( vn.dotProduct( vp)>0) type =0;
	else{
	type =1;
	vn = -vn;
	}

	f = new Face( (Point3d*)vertices, vertexData, 4, vn, type, FALSE,  &startProfile);

	for( i =0; i<4; ++i){
	vertices[i] = pArray[ i*(n-1)];
	vertexData[i] =NULL;
	}
	vn = ( vertices[1]-vertices[0]).crossProduct( vertices[2]-vertices[0]);
	if(type==1) vn =-vn;
	f = new Face( (Point3d*)vertices, vertexData, 4, vn,  type, FALSE,  &endProfile);


	PolygonVertexData vd(PolygonVertexData::kUnspecifiedCurve);

	for( i=0; i<n; ++i){
	vertices[i] = (pArray[i] + pArray[i+2*n].asVector())/2;
	vertexData[i] = &vd;
	}

	m_3dGeom = Body::extrusionAlongPath(
	startProfile,
	endProfile,
	(Point3d*)vertices, 
	vertexData,
	n,
	m_nDirection == 2
	);

	delete [] vertices;
	delete [] vertexData;
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

	bool isclose = ( m_nDirection==2 ); // && fabs(m_dLength1-m_dLength2)<1E-3 && fabs(m_dWidth1-m_dWidth2)<1E-3);

	m_3dGeom = Body::skin(	profiles,
		isclose? n-1: n,
		isclose,
		true,
		NULL,
		true,
		true);

	//m_3dGeom.cleanUpNonManifoldEdgesAndCoincidentFaces();

	for( i=0; i<n; ++i) delete profiles[i];
	delete [] vertices;
	delete [] profiles;
#endif
}
AcGePoint3dArray PDSqutorus::getPtOnCentAix(int Precision)
{
	assertReadEnabled();
	AcGePoint3dArray tmpPts;

	if(m_nDirection == 2 || m_nDirection == 3)
	{
		//////////////////////////////////////////////////////////////////////////
		//直接返回
		return tmpPts;
	}	
	AcGeVector3d Facevect=getNormal();
	double dangle=getAngle();
	double startDist, endDist;
	startDist = m_ptP1.distanceTo(m_ptP);
	endDist = m_ptP2.distanceTo(m_ptP);
	//////////////////////////////////////////////////////////////////////////
	//如果两个半径差的太接近就近似为圆弧
	if(fabs(startDist - endDist)/(startDist + endDist)/2.0 < TOURS_SIMPLIZE_FACTOR)
		return tmpPts;

	double deltaL = (m_dLength2 - m_dLength1) /Precision;
	double deltaW = (m_dWidth2 - m_dWidth1) /Precision;

	AcGePoint3d varCenter;
	double varL,varW;
	double tmpAnglr = dangle / Precision;
	double tmpDeta = (endDist - startDist) / Precision;
	AcGeVector3d tmpVec = m_ptP1 - m_ptP;
	varL = m_dLength1;
	varW =m_dWidth1;
	for(int i = 0; i <=Precision; i++)
	{
		tmpVec.normalize();
		varCenter = m_ptP + tmpVec*startDist; 
		tmpPts.append(varCenter);
		startDist += tmpDeta;
		tmpVec = tmpVec.rotateBy(tmpAnglr,Facevect);
	}
	return tmpPts;
}

