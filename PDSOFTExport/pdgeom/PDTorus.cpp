// PDTorus.cpp: implementation of the PDTorus class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "language25.h"
#include "PDTorus.h"
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
#define VERSION_PDTORUS 3
#define TOURS_SIMPLIZE_FACTOR 0.9

#define PI 3.1415926535897932384626433832795L

const double SIGMA = 0.01;

#ifdef _OBJECTARX2000_
ACRX_DXF_DEFINE_MEMBERS(PDTorus, PDPrimary3D,
                        AcDb::kDHL_CURRENT, AcDb::kMReleaseCurrent,
                        AcDbProxyEntity::kAllAllowedBits,
	                    PDTORUS, Gle);
#else
ACRX_DXF_DEFINE_MEMBERS(PDTorus, PDPrimary3D, AcDbProxyEntity::kAllAllowedBits, PDTORUS, Gle);
#endif

void PDTorus::setDefault()
{
  PDPrimary3D::setDefault();
#if defined(NDEBUG) || !defined(_DEBUG)
  m_dDiameter1 = m_dDiameter2 = 1;
  m_ptCenter = AcGePoint3d();
  m_ptP1.set(2, 0, 0);
  m_ptP2.set(0, 2, 0);
  m_nDirection = 0;
  m_nprecisionArc = m_nprecisionCir = PRIMARY3D_DEFAULT_PRECISION;
  m_v3.set(0,0,0);
#else
  m_dDiameter1 = m_dDiameter2 = 1000;
  m_ptCenter.set(0,0,0);
  m_ptP1.set(1000, 0, 0);
  m_ptP2.set(0, 1000, 0);
  m_nDirection = 0;
  m_nprecisionArc = m_nprecisionCir = PRIMARY3D_DEFAULT_PRECISION;
  m_v3.set(0,0,0);
#endif
}
bool PDTorus::isValidData(double &f)
{
  if(!PDPrimary3D::isValidData(f))
    return false;
  if(!isValidFloat(f=m_dDiameter1))
    return false;
  if(!isValidFloat(f=m_dDiameter2))
    return false;
  if(!isValidFloat(f=m_ptCenter.x) || !isValidFloat(f=m_ptCenter.y) || !isValidFloat(f=m_ptCenter.z))
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
Acad::ErrorStatus PDTorus::audit(AcDbAuditInfo* pAuditInfo)
{
  return auditPDPrimary3D(this,pAuditInfo,_T("PDTorus"));
}

PDTorus::PDTorus(bool HasSnap/* = false*/) : PDPrimary3D(HasSnap)
{
	m_dDiameter1 = m_dDiameter2 = 1;
	m_ptCenter = AcGePoint3d();
    m_ptP1.set(2, 0, 0);
    m_ptP2.set(0, 2, 0);
	m_nDirection = 0;
	m_nprecisionArc = m_nprecisionCir = PRIMARY3D_DEFAULT_PRECISION;
  m_v3.set(0,0,0);
#ifdef _USEAMODELER_
    createBody();
#endif
}

PDTorus::~PDTorus()
{

}

PDTorus::PDTorus(const AcGePoint3d &center, const AcGePoint3d &p1, 
                 const AcGePoint3d &p2, double dia1, double dia2, int d,
			     int prearc/* = PRIMARY3D_DEFAULT_PRECISION*/, 
                 int precir/* = PRIMARY3D_DEFAULT_PRECISION*/, 
                 bool HasSnap/* = false*/,int CorrType/*0-左手系，1-右手系*/) : PDPrimary3D(HasSnap)
{
    m_dDiameter1 =fabs(dia1);
	if(m_dDiameter1==0)
		m_dDiameter1=1;

    m_dDiameter2 =fabs(dia2);
	if(m_dDiameter2==0)
		m_dDiameter2=1;
	
	m_ptCenter = center;
	m_ptP1=p1;
	m_ptP2=p2;  
	
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
    p0p1 = p1-center;
    p0p2 = p2-center;
    if(p0p1.isZeroLength() && p0p2.isZeroLength())
    {
        m_ptP1 = center + AcGeVector3d(0, 1, 0);
        m_ptP2 = center + AcGeVector3d(1, 0, 0);
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
        m_ptP2 = center + p0p2;
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
        m_ptP1 = center+ p0p1;
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
        m_ptP1 = center + p0p1;
    }
	}
	break;

	case 2:
	case 3:
	{
    AcGeVector3d p0p1,facevect;
    p0p1 = p1-center;
	facevect=p2.asVector().normal();
    if(p0p1.isZeroLength() && facevect.isZeroLength())
    {
        m_ptP1 = center + AcGeVector3d(1, 0, 0);
        m_ptP2.set(0,0,1);
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
        m_ptP1 = center+ p0p1;
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
        m_ptP1 = center + p0p1;
    }
	}
	break;

	}

	double startDist, endDist;
    startDist = m_ptP1.distanceTo(center);
	AcGeVector3d startVect = (m_ptP1 - center).normal();
	double r1 = dia1 / 2.0;
	double r2 = dia2/2.0;

	switch(m_nDirection)
    {
  case 2:
		{
			if (startDist<r1 )
				m_ptP1=center+startVect*r1;

			AcGeVector3d facevect=m_ptP2.asVector().normal();
			AcGePoint3d P2=m_ptP1;
			P2.rotateBy(PI*2.0,facevect,center);
			AcGeVector3d endVect = (P2 - center).normal();
			endDist=P2.distanceTo(center);

			if (endDist<r2)
				m_dDiameter2=endDist*2.0;
		}
		break;
    case 3:
		{
			if (startDist<r1 )
				m_ptP1=center+startVect*r1;

			AcGeVector3d facevect=m_ptP2.asVector().normal();
			AcGePoint3d P2=m_ptP1;
			P2.rotateBy(PI,facevect,center);
			AcGeVector3d endVect = (P2 - center).normal();
			endDist=P2.distanceTo(center);

			if (endDist<r2)
				m_dDiameter2=endDist*2.0;
		}
        break;
    case 0:
    case 1:
        {
		  AcGeVector3d endVect = (m_ptP2 - center).normal();
		  endDist=m_ptP2.distanceTo(center);
		  if (startDist<r1 && endDist<r2)
		  {
			  m_ptP1=center+startVect*r1;
			  m_ptP2=center+endVect*r2;
		  }
		  else if (startDist<r1)
			       m_ptP1=center+startVect*r1;
		  else if (endDist<r2)
			       m_ptP2=center+endVect*r2;
		}
        break;
    }

    if(prearc > 2)
	    m_nprecisionArc = prearc;
    else
        m_nprecisionArc = PRIMARY3D_DEFAULT_PRECISION;
    if(precir > 2)
	    m_nprecisionCir = precir;
    else
        m_nprecisionCir = PRIMARY3D_DEFAULT_PRECISION;
    if(m_nprecisionArc > m_nprecisionCir)
        m_dDividPrecision = m_nprecisionArc;
    else
        m_dDividPrecision = m_nprecisionCir;

    AcGeVector3d v1,v2;
    v1 = (m_ptP1-m_ptCenter).normal();
    v2 = getNormal();
    //当方向为0,1时，由于是按点来处理，所以不存在法线方向，因此都应该是右手系
    if(CorrType || m_nDirection==0 || m_nDirection==1)
      m_v3 = v1.crossProduct(v2).normal();
    else
      m_v3 = v2.crossProduct(v1).normal();

#ifdef _USEAMODELER_
    createBody();
#endif
}//added by linlin 20050920


#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDTorus::subTransformBy(const AcGeMatrix3d& xform)
#else
Acad::ErrorStatus PDTorus::transformBy(const AcGeMatrix3d& xform)
#endif
{
	assertWriteEnabled();
	// TODO: implement this function.
	m_ptCenter.transformBy(xform);
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

	m_dDiameter1 *= xform.scale();
	m_dDiameter2 *= xform.scale();

#ifdef _OBJECTARX2010_
	return PDPrimary3D::subTransformBy(xform);
#else
	return PDPrimary3D::transformBy(xform);
#endif
}
/*Adesk::Boolean PDTorus::worldDraw(AcGiWorldDraw* mode)
{
	assertReadEnabled();
	if (mode->regenAbort()) {
		return Adesk::kTrue;
	}
#ifdef _USEAMODELER_
    PDPrimary3D::worldDraw(mode);
#else

    AcGePoint3dArray pArray;
    AcGeIntArray stdArcIdx, stdCirIdx;
    int actPrecisionArc, actPrecisionCir;
	getVertices(getMaxPrecisionArc(mode), getMaxPrecisionCir(mode), 
                pArray, stdArcIdx, stdCirIdx, 
                actPrecisionArc, actPrecisionCir);
    int PrecisionArc__1 = actPrecisionArc + 1;
    int PrecisionCir__1 = actPrecisionCir + 1;
    int stdArcIdxLen, stdCirIdxLen;
    stdArcIdxLen = stdArcIdx.length();
    stdCirIdxLen = stdCirIdx.length();

	switch(mode->regenType())
    {
	case kAcGiStandardDisplay:
    case kAcGiSaveWorldDrawForR12:
    case kAcGiSaveWorldDrawForProxy:
        {
            int gsIndex = 0;

            int stdCirIdxLen_1 = stdCirIdxLen - 1;
            for(int i = 0; i < stdArcIdxLen; ++i)
            {
                mode->subEntityTraits().setSelectionMarker(++gsIndex);
                mode->geometry().circle(pArray[stdArcIdx[i] * PrecisionCir__1], 
                                        pArray[stdArcIdx[i] * PrecisionCir__1 + 3] , 
                                        pArray[stdArcIdx[i] * PrecisionCir__1 + 6]);
            }

            if(m_nDirection == 2 && m_dDiameter1 == m_dDiameter2)
            {
                int PrecisionCir___3 = PrecisionCir__1 * 3;
                int PrecisionCir___6 = PrecisionCir__1 * 6;
                for(i = 0; i < stdCirIdxLen_1; ++i)
                {
                    mode->subEntityTraits().setSelectionMarker(++gsIndex);
                    mode->geometry().circle(pArray[stdCirIdx[i]], 
                                            pArray[stdCirIdx[i] + PrecisionCir___3], 
                                            pArray[stdCirIdx[i] + PrecisionCir___6]);
                }
            }
            else
            {
                if(getStartCenDist() == getEndCenDist() && m_dDiameter1 == m_dDiameter2)
                {
                    int PrecisionCir___4 = PrecisionCir__1 * 4;
                    int PrecisionCir___Arc = PrecisionCir__1 * actPrecisionArc;
                    for(i = 0; i < stdCirIdxLen_1; ++i)
                    {
                        mode->subEntityTraits().setSelectionMarker(++gsIndex);
                        mode->geometry().circularArc(pArray[stdCirIdx[i]], 
                                                     pArray[stdCirIdx[i] + PrecisionCir___4], 
                                                     pArray[stdCirIdx[i] + PrecisionCir___Arc]);
                    }
                }
                else
                {
                    int j;
                    for(i = 0; i < stdCirIdxLen_1; ++i)
                    {
                        AcGePoint3dArray pts;
                        for(j = 0; j < PrecisionArc__1; ++j)
                            pts.append(pArray[stdCirIdx[i] + j * PrecisionCir__1]);
                        mode->subEntityTraits().setSelectionMarker(++gsIndex);
                        mode->geometry().polyline(PrecisionArc__1, pts.asArrayPtr());
                    }
                }
            }
	    }
        break;
	case kAcGiHideOrShadeCommand:
	case kAcGiRenderCommand:
        {
		    mode->subEntityTraits().setFillType(kAcGiFillAlways);
            mode->geometry().polygon(actPrecisionCir, pArray.asArrayPtr());
            mode->geometry().polygon(actPrecisionCir, &(pArray.asArrayPtr()[actPrecisionArc * PrecisionCir__1]));

            AcGiEdgeData edgeData;
            int edgeDataLen = actPrecisionArc * PrecisionCir__1 + PrecisionArc__1 * actPrecisionCir;
            Adesk::UInt8* edgeVisibility = new Adesk::UInt8[edgeDataLen];
            int i, j;
	        for(i = 0; i < edgeDataLen; ++i)
                edgeVisibility[i] = kAcGiInvisible;//kAcGiSilhouette;
            for(i = 0; i < stdArcIdxLen; ++i)
            {
                for(j = 0; j < actPrecisionCir; ++j)
                    edgeVisibility[stdArcIdx[i] * actPrecisionCir + j] = kAcGiVisible;
            }
            int PrecisionArc___Cir = PrecisionArc__1 * actPrecisionCir;
            for(i = 0; i < stdCirIdxLen; ++i)
            {
                for(j = 0; j < actPrecisionArc; ++j)
                    edgeVisibility[PrecisionArc___Cir + stdCirIdx[i] * actPrecisionArc + j] = kAcGiVisible;
            }
            edgeData.setVisibility(edgeVisibility);

		   mode->geometry().mesh(PrecisionArc__1, PrecisionCir__1, pArray.asArrayPtr(), &edgeData);
            delete [] edgeVisibility;
	    }
        break;
    default:
        break;
	}
#endif
	return Adesk::kTrue;
}*/

#ifdef _OBJECTARX2010_
Adesk::Boolean PDTorus::subWorldDraw(AcGiWorldDraw* mode)
#else
Adesk::Boolean PDTorus::worldDraw(AcGiWorldDraw* mode)
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
	Adesk::UInt32 PrecisionA = m_nprecisionArc;
	Adesk::UInt32 PrecisionC = m_nprecisionCir;
	int PreStepA =1;
	int PreStepC = 1;   
	AcGePoint3dArray pArray;
	Adesk::UInt32 PrecisionArc__1 = PrecisionA + 1;
	Adesk::UInt32 PrecisionCir__1 = PrecisionC + 1;

	Adesk::Int16 nRating = getCalMaxPrecision();
	if(nRating == PRIMARY3D_SIMPLIZE_RATE1 ||
		nRating == PRIMARY3D_SIMPLIZE_RATE2)
	{
		double startDist = m_ptP1.distanceTo(m_ptCenter);
		PrecisionA = (int)(PI / acos(1.0 - mode->deviation(kAcGiMaxDevForCircle, m_ptP1) * 2 / m_dDiameter1));
		if(PrecisionA < PRIMARY3D_SIMPLIZE_SCALE)
		{
			if (nRating == PRIMARY3D_SIMPLIZE_RATE2)
			{
				switch(m_nDirection)
				{
				case 0:
				case 1:
					{
						AcGePoint3dArray pts = getPtOnCentAix(PrecisionA);
						if(pts.length() ==0)
						{
							AcGeVector3d tmpNormal = getNormal();
							AcGeVector3d tmpStartVect = m_ptP1 - m_ptCenter;
							double angle = getAngle();
							mode->geometry().circularArc(m_ptCenter,startDist,tmpNormal,tmpStartVect,angle);
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
						mode->geometry().circle(m_ptCenter,startDist,tmpNormal);
					}
					break;
				case 3:
					{
						//半圆
						AcGeVector3d tmpNormal = getNormal();
						AcGeVector3d tmpStartVect = m_ptP1 - m_ptCenter;
						double angle = getAngle();
						mode->geometry().circularArc(m_ptCenter,startDist,tmpNormal,tmpStartVect,angle);
					}
					break;
				default:
					break;
				}
			} 
			else if (nRating == PRIMARY3D_SIMPLIZE_RATE1)
			{
				PrecisionA = PrecisionC = PRIMARY3D_WIREFRAME_LINENUM4;
				getVertices(PrecisionA, PrecisionC,pArray);
				PrecisionArc__1 = PrecisionA + 1;
				PrecisionCir__1 = PrecisionC + 1;

				Adesk::UInt32 i;
				for(i = 0; i < PrecisionArc__1; i += PreStepA)
				{
					if(mode->geometry().circle(pArray[i * PrecisionCir__1],
						pArray[i * PrecisionCir__1 + 2] ,
						pArray[i * PrecisionCir__1 + 3]))
					{
						return Adesk::kTrue;
					}
				}

				if(m_nDirection == 2 && m_dDiameter1 == m_dDiameter2)
				{
					int PrecisionCir___3 = PrecisionCir__1 * 1;
					int PrecisionCir___6 = PrecisionCir__1 * 2;
					for(i = 0; i < PrecisionC; i += PreStepC)
					{
						if(mode->geometry().circle(pArray[i],pArray[i + PrecisionCir___3],pArray[i + PrecisionCir___6]))
							return Adesk::kTrue;
					}
				}
				else
				{
					if(getStartCenDist() == getEndCenDist() && m_dDiameter1 == m_dDiameter2)
					{
						int PrecisionCir___4 = PrecisionCir__1 * 2;
						int PrecisionCir___Arc = PrecisionCir__1 * PrecisionA;
						for(i = 0; i < PrecisionC; i += PreStepC)
						{
							if(mode->geometry().circularArc(pArray[i],pArray[i + PrecisionCir___4],pArray[i + PrecisionCir___Arc]))
								return Adesk::kTrue;
						}
					}
					else
					{
						Adesk::UInt32 j;    
						AcGePoint3dArray pts;
						for(i = 0; i < PrecisionC;  i += PreStepC)
						{
							pts.setLogicalLength(0);
							for(j = 0; j < PrecisionArc__1; ++j)
								pts.append(pArray[i + j * PrecisionCir__1]);
							if(mode->geometry().polyline(PrecisionArc__1, pts.asArrayPtr()))
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
			if(PrecisionA < PRIMARY3D_WIREFRAME_LINENUM * 2)
				PrecisionA = PRIMARY3D_WIREFRAME_LINENUM;
			else
			{
				if(PrecisionA > m_nprecisionArc)
					PrecisionA = m_nprecisionArc;
				PreStepA = PrecisionA / PRIMARY3D_WIREFRAME_LINENUM;
				PrecisionA = PreStepA * PRIMARY3D_WIREFRAME_LINENUM;
			}
			if(PrecisionC < PRIMARY3D_WIREFRAME_LINENUM * 2)
				PrecisionC = PRIMARY3D_WIREFRAME_LINENUM;
			else
			{
				if(PrecisionC > m_nprecisionCir)
					PrecisionC = m_nprecisionCir;
				PreStepC = PrecisionC / PRIMARY3D_WIREFRAME_LINENUM;
				PrecisionC = PreStepC * PRIMARY3D_WIREFRAME_LINENUM;
			}
			getVertices(PrecisionA, PrecisionC,pArray);
			PrecisionArc__1 = PrecisionA + 1;
			PrecisionCir__1 = PrecisionC + 1;
			//添加实体模型各面的GS标记
			//added by szw 2009.11.18 : begin
			int gsIndex = 0;
			mode->subEntityTraits().setSelectionMarker(++gsIndex);
			//added by szw 2009.11.18 : end
           if(mode->geometry().polygon(PrecisionC, pArray.asArrayPtr()))
				return Adesk::kTrue;
		   //added by szw 2009.11.18 : begin
			gsIndex += PRIMARY3D_WIREFRAME_LINENUM;
			mode->subEntityTraits().setSelectionMarker(gsIndex);
			//added by szw 2009.11.18 : end
			if(mode->geometry().polygon(PrecisionC, &(pArray.asArrayPtr()[PrecisionA * PrecisionCir__1])))
				return Adesk::kTrue;

            AcGiEdgeData edgeData;
            Adesk::UInt32 edgeDataLen = PrecisionA * PrecisionCir__1 + PrecisionArc__1 * PrecisionC;
            Adesk::UInt8* edgeVisibility = new Adesk::UInt8[edgeDataLen];
            Adesk::UInt32 i, j;
	        for(i = 0; i < edgeDataLen; ++i)
                edgeVisibility[i] = kAcGiInvisible;//kAcGiSilhouette;
            for(i = 0; i < PrecisionA; i += PreStepA)
            {
                for(j = 0; j < PrecisionC; j++)
                    edgeVisibility[i * PrecisionC + j] = kAcGiVisible;
            }
            int PrecisionArc___Cir = PrecisionArc__1 * PrecisionC;
            for(i = 0; i < PrecisionC; i+= PreStepC)
            {
                for(j = 0; j < PrecisionA; j ++)
                    edgeVisibility[PrecisionArc___Cir + i * PrecisionA + j] = kAcGiVisible;
            }
            edgeData.setVisibility(edgeVisibility);
			//added by szw 2009.11.18 : begin
			gsIndex += PRIMARY3D_WIREFRAME_LINENUM;
			mode->subEntityTraits().setSelectionMarker(gsIndex);
			//added by szw 2009.11.18 : end
		    mode->geometry().mesh(PrecisionArc__1, PrecisionCir__1, pArray.asArrayPtr(), &edgeData);
            delete [] edgeVisibility;
	    }
#ifndef _ALWAYSSHOWWIRE_
        break;
	case kAcGiStandardDisplay:
	case kAcGiSaveWorldDrawForR12:
	case kAcGiSaveWorldDrawForProxy:
#endif
		{
			PrecisionA = PrecisionC = PRIMARY3D_WIREFRAME_LINENUM;
			getVertices(PrecisionA, PrecisionC,pArray);
			PrecisionArc__1 = PrecisionA + 1;
			PrecisionCir__1 = PrecisionC + 1;
			int gsIndex = 0;
			//统一线框模型和实体模型的GS标记
			//modified by szw 2009.11.18 : begin
			Adesk::UInt32 i;
			for(i = 0; i < PrecisionArc__1; i += PreStepA)
			{
				mode->subEntityTraits().setSelectionMarker(++gsIndex);
				if(mode->geometry().circle(pArray[i * PrecisionCir__1],
					pArray[i * PrecisionCir__1 + 3] ,
					pArray[i * PrecisionCir__1 + 6]))
				{
					return Adesk::kTrue;
				}
			}

			if(m_nDirection == 2 && m_dDiameter1 == m_dDiameter2)
			{
				int PrecisionCir___3 = PrecisionCir__1 * 3;
				int PrecisionCir___6 = PrecisionCir__1 * 6;
				for(i = 0; i < PrecisionC; i += PreStepC)
				{
					mode->subEntityTraits().setSelectionMarker(++gsIndex);
					if(mode->geometry().circle(pArray[i],pArray[i + PrecisionCir___3],pArray[i + PrecisionCir___6]))
						return Adesk::kTrue;
				}
			}
			else
			{
				if(getStartCenDist() == getEndCenDist() && m_dDiameter1 == m_dDiameter2)
				{
					int PrecisionCir___4 = PrecisionCir__1 * 4;
					int PrecisionCir___Arc = PrecisionCir__1 * PrecisionA;
					for(i = 0; i < PrecisionC; i += PreStepC)
					{
						mode->subEntityTraits().setSelectionMarker(++gsIndex);
						if(mode->geometry().circularArc(pArray[i],pArray[i + PrecisionCir___4],pArray[i + PrecisionCir___Arc]))
							return Adesk::kTrue;
					}
				}
				else
				{
					Adesk::UInt32 j;    
					AcGePoint3dArray pts;
					for(i = 0; i < PrecisionC;  i += PreStepC)
					{
						pts.setLogicalLength(0);
						for(j = 0; j < PrecisionArc__1; ++j)
							pts.append(pArray[i + j * PrecisionCir__1]);
						mode->subEntityTraits().setSelectionMarker(++gsIndex);
						if(mode->geometry().polyline(PrecisionArc__1, pts.asArrayPtr()))
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

///////////////////////////////////////////////////////////////////////
//////	先将圆弧在角度方向上分成m_nPrecisionArc段				///////
//////	根据圆断面直径随角度递增线性变化，求出相应直径			///////
//////	再对每个圆求出m_nPrecisionCir+1个离散点，加入到数组中	///////
/////	得到 (m_nPrecisionArc+1)*(m_nPrecisonCir+1)个点			///////
////////////////////////////////////////////////////////////////////////
Acad::ErrorStatus PDTorus::getVertices(int precisionArc, int precisionCir, AcGePoint3dArray& vertexArray, 
                                       AcGeIntArray &stdArcIdx, AcGeIntArray &stdCirIdx, 
                                       int &actPrecisionArc, int &actPrecisionCir) const
{
	assertReadEnabled();

    AcGeDoubleArray dividDblArc, dividDblCir;
    getActDivid(precisionArc, dividDblArc, stdArcIdx);
    getActDivid(precisionCir, dividDblCir, stdCirIdx);

    int precisionArc__1 = dividDblArc.length();
    int precisionCir__1 = dividDblCir.length();
    actPrecisionArc = precisionArc__1 - 1;
    actPrecisionCir = precisionCir__1 - 1;
    vertexArray.setLogicalLength(precisionArc__1 * precisionCir__1);

    AcGeVector3d startVect = (m_ptP1 - m_ptCenter).normal();
	AcGeVector3d faceVect = getNormal();

    double startDist = getStartCenDist();
    double endDist = getEndCenDist();
    double dangle = getAngle();

    double r1 = m_dDiameter1 / 2.0;
    double deltaR = (m_dDiameter2 - m_dDiameter1) / precisionArc / 2.0;
    double deltaAngle = dangle / precisionArc;
    double deltaDist = (endDist - startDist) / precisionArc;
    double deltaP = 2.0 * PI / precisionCir;
    int j;

    AcGeVector3d v1,v2;
    v1 = (m_ptP1-m_ptCenter).normal();
    v2 = getNormal();
    if(isLeftCoord(v1,v2,m_v3))
      faceVect *= -1;

    for(int i = 0; i < precisionArc__1; ++i)
    {
        AcGePoint3d varCenter;
        AcGeVector3d P1CenterVec = startVect;
        varCenter = m_ptCenter + P1CenterVec.rotateBy(deltaAngle * dividDblArc[i], faceVect) * (startDist + deltaDist * dividDblArc[i]);
	    AcGeVector3d curvect = P1CenterVec.crossProduct(faceVect).normal();
        AcGePoint3d sp = varCenter + faceVect * (r1 + deltaR * dividDblArc[i]);
        vertexArray[i * precisionCir__1] = sp;
	    for(j = 1; j < actPrecisionCir; ++j)
        {
            AcGePoint3d point = sp;
            point.rotateBy(deltaP * dividDblCir[j], curvect, varCenter);
		    vertexArray[i * precisionCir__1 + j] = point;
	    }
	    vertexArray[i * precisionCir__1 + j] = sp;
    }

	return Acad::eOk;
}

Acad::ErrorStatus PDTorus::getVertices(int precisionArc, int precisionCir, AcGePoint3dArray& vertexArray) const
{
	assertReadEnabled();

	int precisionArc__1 = precisionArc + 1;
	int precisionCir__1 = precisionCir + 1;
	vertexArray.setLogicalLength(precisionArc__1 * precisionCir__1);

	AcGeVector3d startVect = (m_ptP1 - m_ptCenter).normal();
	AcGeVector3d faceVect = getNormal();

	double startDist = getStartCenDist();
	double endDist = getEndCenDist();
	double dangle = getAngle();

	double r1 = m_dDiameter1 / 2.0;
	double deltaR = (m_dDiameter2 - m_dDiameter1) / precisionArc / 2.0;
	double deltaAngle = dangle / precisionArc;
	double deltaDist = (endDist - startDist) / precisionArc;
	double deltaP = 2.0 * PI / precisionCir;
	int j;
	AcGePoint3d varCenter;
	AcGeVector3d P1CenterVec = startVect;
	varCenter  = m_ptCenter;
	AcGeVector3d curvect;
	AcGePoint3d sp,point;

    AcGeVector3d v1,v2;
    v1 = (m_ptP1-m_ptCenter).normal();
    v2 = getNormal();
    if(isLeftCoord(v1,v2,m_v3))
        faceVect *= -1;

	for(int i = 0; i < precisionArc__1; ++i)
	{	
		P1CenterVec.normalize();
		varCenter = m_ptCenter + P1CenterVec * startDist;
		curvect = P1CenterVec.crossProduct(faceVect).normal();
		sp = varCenter + faceVect * (r1 + deltaR * i);
		vertexArray[i * precisionCir__1] = sp;
		point = sp;
		for(j = 1; j < precisionCir; ++j)
		{
			point.rotateBy(deltaP , curvect, varCenter);
			vertexArray[i * precisionCir__1 + j] = point;
		}
		vertexArray[i * precisionCir__1 + j] = sp;	
		P1CenterVec.rotateBy(deltaAngle, faceVect);
		startDist += deltaDist;
	}

	return Acad::eOk;
}
Acad::ErrorStatus PDTorus::dwgOutFields(AcDbDwgFiler* filer)const
{
    assertReadEnabled();
	Acad::ErrorStatus es=Acad::eOk;

	// Call dwgOutFields from father class: PDPrimary3D
	if((es=PDPrimary3D::dwgOutFields(filer))!=Acad::eOk)
	{
		return es;
	}

	// Write version number.
	filer->writeItem((Adesk::UInt16) VERSION_PDTORUS);

    // version 3
    filer->writeItem(m_v3);

    // version 1
	// Write the data members.
	filer->writeItem(m_ptCenter);
	filer->writeItem(m_ptP1);
	filer->writeItem(m_ptP2);
	filer->writeItem(m_dDiameter1);
	filer->writeItem(m_dDiameter2);
	filer->writeItem(m_nDirection);
	filer->writeItem(m_nprecisionArc);
	filer->writeItem(m_nprecisionCir);

	return filer->filerStatus();
}

Acad::ErrorStatus PDTorus::dwgInFields(AcDbDwgFiler* filer)
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
	if (version > VERSION_PDTORUS)
		return Acad::eMakeMeProxy;

    if(version >= 3)
        filer->readItem(&m_v3);

	// Read the data members.
	if(version >= 1)
    {
        filer->readItem(&m_ptCenter);
        filer->readItem(&m_ptP1);
        filer->readItem(&m_ptP2);
        filer->readItem(&m_dDiameter1);
        filer->readItem(&m_dDiameter2);
    }

  if(version == 1)
    {
      double dblTemp;
		  filer->readItem(&dblTemp);
    }
  if(version >= 1)
    {
		  filer->readItem(&m_nDirection);
		  filer->readItem(&m_nprecisionArc);
		  filer->readItem(&m_nprecisionCir);
	  }

#ifdef _USEAMODELER_
	if(m_3dGeom.isNull())
		createBody();
#endif

	return filer->filerStatus();
}


Acad::ErrorStatus PDTorus::dxfOutFields(AcDbDxfFiler* filer) const
{
    assertReadEnabled();
    Acad::ErrorStatus es;

    if ((es = PDPrimary3D::dxfOutFields(filer))
        != Acad::eOk)
    {
        return es;
    }

	// Write subclass marker.
    filer->writeItem(AcDb::kDxfSubclass, _T("PDTorus"));

	// Write version number.
	filer->writeItem(AcDb::kDxfInt16, (Adesk::UInt16) VERSION_PDTORUS);

    // version 3
    filer->writeItem(AcDb::kDxfXCoord+3, m_v3);

    // version 1
	// Write data members.
    filer->writeItem(AcDb::kDxfXCoord, m_ptCenter);
	filer->writeItem(AcDb::kDxfXCoord+1, m_ptP1);
	filer->writeItem(AcDb::kDxfXCoord+2, m_ptP2);
	filer->writeItem(AcDb::kDxfReal,m_dDiameter1);
	filer->writeItem(AcDb::kDxfReal+1,m_dDiameter2);
	filer->writeItem(AcDb::kDxfInt32,m_nDirection);
	filer->writeItem(AcDb::kDxfInt32+1,m_nprecisionArc);
	filer->writeItem(AcDb::kDxfInt32+2,m_nprecisionCir);

    return filer->filerStatus();
}

Acad::ErrorStatus PDTorus::dxfInFields(AcDbDxfFiler* filer)
{
    assertWriteEnabled();
    Acad::ErrorStatus es = Acad::eOk;
    struct resbuf rb;

    if ((PDPrimary3D::dxfInFields(filer) != Acad::eOk)
        || !filer->atSubclassData(_T("PDTorus")))
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
		if (version > VERSION_PDTORUS)
			return Acad::eMakeMeProxy;
	}

    if(version >= 3)
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
	if(version >= 1)
    {
	    while ((es == Acad::eOk) && ((es = filer->readResBuf(&rb)) == Acad::eOk))
        {
			    switch (rb.restype)
            {
			      case AcDb::kDxfXCoord:
				      m_ptCenter = asPnt3d(rb.resval.rpoint);
				      break;

			      case AcDb::kDxfXCoord+1:
				      m_ptP1 = asPnt3d(rb.resval.rpoint);
				      break;

			      case AcDb::kDxfXCoord+2:
				      m_ptP2 = asPnt3d(rb.resval.rpoint);
				      break;

			      case AcDb::kDxfReal:
				      m_dDiameter1 = rb.resval.rreal;
				      break;

			      case AcDb::kDxfReal+1:
				      m_dDiameter2 = rb.resval.rreal;
				      break;

			      case AcDb::kDxfInt32:
				      m_nDirection = rb.resval.rlong;
				      break;

			      case AcDb::kDxfInt32+1:
				      m_nprecisionArc = rb.resval.rlong;
				      break;

			      case AcDb::kDxfInt32+2:
				      m_nprecisionCir = rb.resval.rlong;
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

Acad::ErrorStatus PDTorus::setParameters(const AcGePoint3d &center, 
                                         const AcGePoint3d &p1, 
                                         const AcGePoint3d &p2, 
                                         double dia1, double dia2, int d, 
                                         int prearc/* = PRIMARY3D_DEFAULT_PRECISION*/, 
                                         int precir/* = PRIMARY3D_DEFAULT_PRECISION*/,
                                         int CorrType/*0-左手系，1-右手系*/)
{
    assertWriteEnabled();

    m_dDiameter1 = fabs(dia1);
	if (m_dDiameter1==0)
		m_dDiameter1=1;
    
	m_dDiameter2 =fabs(dia2);
		if(m_dDiameter2==0)
		m_dDiameter2=1;

	m_ptCenter = center;
	m_ptP1=p1;
	m_ptP2=p2;  
	
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
    p0p1 = p1-center;
    p0p2 = p2-center;
    if(p0p1.isZeroLength() && p0p2.isZeroLength())
    {
        m_ptP1 = center + AcGeVector3d(0, 1, 0);
        m_ptP2 = center + AcGeVector3d(1, 0, 0);
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
        m_ptP2 = center + p0p2;
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
        m_ptP1 = center+ p0p1;
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
        m_ptP1 = center + p0p1;
    }
	}
	break;

	case 2:
	case 3:
	{
    AcGeVector3d p0p1,facevect;
    p0p1 = p1-center;
	facevect=p2.asVector().normal();
    if(p0p1.isZeroLength() && facevect.isZeroLength())
    {
        m_ptP1 = center + AcGeVector3d(1, 0, 0);
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
        m_ptP1 = center+ p0p1;
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
        m_ptP1 = center + p0p1;
    }
	}
	break;

	}

	double startDist, endDist;
    startDist = m_ptP1.distanceTo(center);
	AcGeVector3d startVect = (m_ptP1 - center).normal();
	double r1 = dia1 / 2.0;
	double r2 = dia2/2.0;

	switch(m_nDirection)
    {
    case 2:
		{
			if (startDist<r1 )
				m_ptP1=center+startVect*r1;

			AcGeVector3d facevect=m_ptP2.asVector().normal();
			AcGePoint3d P2=m_ptP1;
			P2.rotateBy(PI*2.0,facevect,center);
			AcGeVector3d endVect = (P2 - center).normal();
			endDist=P2.distanceTo(center);

			if (endDist<r2)
				m_dDiameter2=endDist*2.0;
		}
		break;
    case 3:
		{
			if (startDist<r1 )
				m_ptP1=center+startVect*r1;

			AcGeVector3d facevect=m_ptP2.asVector().normal();
			AcGePoint3d P2=m_ptP1;
			P2.rotateBy(PI,facevect,center);
			AcGeVector3d endVect = (P2 - center).normal();
			endDist=P2.distanceTo(center);

			if (endDist<r2)
				m_dDiameter2=endDist*2.0;
		}
        break;
    case 0:
    case 1:
        {
		  AcGeVector3d endVect = (m_ptP2 - center).normal();
		  endDist=m_ptP2.distanceTo(center);
		  if (startDist<r1 && endDist<r2)
		  {
			  m_ptP1=center+startVect*r1;
			  m_ptP2=center+endVect*r2;
		  }
		  else if (startDist<r1)
			       m_ptP1=center+startVect*r1;
		  else if (endDist<r2)
			       m_ptP2=center+endVect*r2;
		}
        break;
    }


    if(prearc > 2)
	    m_nprecisionArc = prearc;
    else
        m_nprecisionArc = PRIMARY3D_DEFAULT_PRECISION;
    if(precir > 2)
	    m_nprecisionCir = precir;
    else
        m_nprecisionCir = PRIMARY3D_DEFAULT_PRECISION;
    if(m_nprecisionArc > m_nprecisionCir)
        m_dDividPrecision = m_nprecisionArc;
    else
        m_dDividPrecision = m_nprecisionCir;

    AcGeVector3d v1,v2;
    v1 = (m_ptP1-m_ptCenter).normal();
    v2 = getNormal();
    if(CorrType)
      m_v3 = v1.crossProduct(v2).normal();
    else
      m_v3 = v2.crossProduct(v1).normal();


#ifdef _USEAMODELER_
	createBody();
#endif

    return Acad::eOk;
}//added by linlin 20050920

AcGePoint3d PDTorus::getpointCenter() const
{
	assertReadEnabled();
	return m_ptCenter;
}

AcGePoint3d PDTorus::getpointP1() const
{
	assertReadEnabled();
	return m_ptP1;
}

AcGePoint3d PDTorus::getpointP2() const
{
	assertReadEnabled();
	return m_ptP2;
}

double PDTorus::getDiameter1() const
{
	assertReadEnabled();
	return m_dDiameter1;
}

double PDTorus::getDiameter2() const
{
	assertReadEnabled();
	return m_dDiameter2;
}

Adesk::UInt32 PDTorus::getDirection() const
{
	assertReadEnabled();
	return m_nDirection;
}

Adesk::UInt32 PDTorus::getprecisionArc() const
{
	assertReadEnabled();
	return m_nprecisionArc;
}

Adesk::UInt32 PDTorus::getprecisionCir() const
{
	assertReadEnabled();
	return m_nprecisionCir;
}

// 取得圆环起点到圆环中心的距离
double PDTorus::getStartCenDist() const
{
    assertReadEnabled();
    return m_ptP1.distanceTo(m_ptCenter);
}

// 取得圆环终点到圆环中心的距离
double PDTorus::getEndCenDist() const
{
    assertReadEnabled();
    if(m_nDirection == 2 || m_nDirection == 3)
        return m_ptP1.distanceTo(m_ptCenter);
    else
        return m_ptP2.distanceTo(m_ptCenter);
}

// 取得圆环中心到圆环起点的矢量
AcGeVector3d PDTorus::getStartCenVect() const
{
    assertReadEnabled();
    return (m_ptP1 - m_ptCenter).normal();
}
// 取得圆环中心到圆环终点的矢量
AcGeVector3d PDTorus::getEndCenVect() const
{
    assertReadEnabled();
    if(m_nDirection == 2)
        return (m_ptP1 - m_ptCenter).normal();
    else if(m_nDirection == 3)
        return (m_ptCenter - m_ptP1).normal();
    else
        return (m_ptP2 - m_ptCenter).normal();
}

AcGePoint3d PDTorus::getEndPoint()const
{
	assertReadEnabled();
	if (m_nDirection == 2)
		return m_ptP1;
	else if (m_nDirection == 3)
		return m_ptCenter + (m_ptCenter - m_ptP1);
	else
		return m_ptP2;
}

// 取得圆环的法矢
AcGeVector3d PDTorus::getNormal() const
{
    assertReadEnabled();
    AcGeVector3d ret;
    switch(m_nDirection)
    {
    case 2:
    case 3:
        ret = m_ptP2.asVector().normal();
        break;
    case 0:
    case 1:
    default:
        {
            AcGeVector3d startVect = (m_ptP1 - m_ptCenter).normal();
            AcGeVector3d endVect = m_ptP2 - m_ptCenter;
            if(startVect.isParallelTo(endVect))
            {
                AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax;// Ay;
                if (fabs(startVect[X]) <1.0/64 && fabs(startVect[Y])<1.0/64) 
                    Ax = Wy.crossProduct(startVect);
                else
                    Ax = Wz.crossProduct(startVect);
                Ax.normalize();
                //Ay = vect.crossProduct(Ax);
                //Ay.normalize();

                ret = Ax;
                if(m_nDirection == 1)
                    ret = -ret;
            }
            else
            {
                ret =  startVect.crossProduct(endVect).normal();
                if(m_nDirection == 1)
                    ret = -ret;
            }
        }
        break;
    }
    return ret;
}

AcGeVector3d PDTorus::getStartPlaneNormal() const
{
	assertReadEnabled();

	AcGeVector3d torusNormal = getNormal();
	AcGeVector3d startVect = (m_ptP1 - m_ptCenter).normal();
	return startVect.crossProduct(torusNormal).normal();
}

AcGeVector3d PDTorus::getEndPlaneNormal() const
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
		AcGeVector3d endVect = (m_ptP2 - m_ptCenter).normal();
		result = torusNormal.crossProduct(endVect).normal();
		break;
	}
	return result;
}

// 取得圆环的扫过的角度
double PDTorus::getAngle() const
{
    assertReadEnabled();
    AcGeVector3d startVect = m_ptP1 - m_ptCenter;
    AcGeVector3d endVect = m_ptP2 - m_ptCenter;
    double dangle;
    switch(m_nDirection)
    {
    case 1:
        if(startVect.isCodirectionalTo(endVect))
            dangle = 2.0 * PI;
        else
            dangle = 2.0 * PI - endVect.angleTo(startVect);
        break;
    case 2:
        dangle = 2.0 * PI;
        break;
    case 3:
        dangle = PI;
        break;
    case 0:
    default:
        if(startVect.isCodirectionalTo(endVect))
            dangle = 2.0 * PI;
        else
            dangle = endVect.angleTo(startVect);
        break;
    }
    return dangle;
}

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDTorus::subGetOsnapPoints(
	AcDb::OsnapMode     osnapMode,
	Adesk::GsMarker     gsSelectionMark,
	const AcGePoint3d&  pickPoint,
	const AcGePoint3d&  lastPoint,
	const AcGeMatrix3d& viewXform,
	AcGePoint3dArray&   snapPoints,
	AcDbIntArray &   geomIds) const
#else
Acad::ErrorStatus PDTorus::getOsnapPoints(
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

    AcGePoint3dArray pArray;
    AcGeIntArray stdArcIdx, stdCirIdx;
    int actPrecisionArc, actPrecisionCir;
    getVertices(m_nprecisionArc, m_nprecisionCir, pArray, 
                stdArcIdx, stdCirIdx, actPrecisionArc, actPrecisionCir);
    int PrecisionArc__1 = actPrecisionArc + 1;
    int PrecisionCir__1 = actPrecisionCir + 1;
    int stdArcIdxLen, stdCirIdxLen;
    stdArcIdxLen = stdArcIdx.length();
    stdCirIdxLen = stdCirIdx.length();
    int stdArcIdxLen_1 = stdArcIdxLen - 1;
    int stdCirIdxLen_1 = stdCirIdxLen - 1;

    AcGeVector3d faceVect = getNormal();
    switch(osnapMode)
    {
    case AcDb::kOsModeEnd:
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
                pt.rotateBy(PI, faceVect, m_ptCenter);
                snapPoints.append(pt);
            }
            break;
        }
        if(!(m_nDirection == 2 && m_dDiameter1 == m_dDiameter2))
        {
            int PrecisionCir___Arc = PrecisionCir__1 * actPrecisionArc;
            for(int i = 0; i < stdCirIdxLen_1; i++)
            {
                snapPoints.append(pArray[stdCirIdx[i]]);
                snapPoints.append(pArray[stdCirIdx[i] + PrecisionCir___Arc]);
            }
        }
        break;
    case AcDb::kOsModeMid:
        if(!(m_nDirection == 2 && m_dDiameter1 == m_dDiameter2))
        {
            if(gsSelectionMark_int <= stdArcIdxLen)
                snapPoints.append(pArray[stdArcIdx[stdArcIdxLen / 2] * PrecisionCir__1] + 
                                 (pArray[stdArcIdx[stdArcIdxLen / 2] * PrecisionCir__1 + stdCirIdx[stdCirIdxLen / 2]] -
                                  pArray[stdArcIdx[stdArcIdxLen / 2] * PrecisionCir__1]) / 2.0);
            else if(gsSelectionMark_int < PRIMARY3D_WIREFRAME_LINENUM * 2)
                snapPoints.append(pArray[stdCirIdx[gsSelectionMark_int - stdArcIdxLen - 1] + stdArcIdx[stdArcIdxLen / 2] * PrecisionCir__1]);
        }
        break;
    case AcDb::kOsModeCen:
        if(gsSelectionMark_int <= stdArcIdxLen)
            snapPoints.append(pArray[PrecisionCir__1 * stdArcIdx[gsSelectionMark_int - 1]] + 
                              (pArray[PrecisionCir__1 * stdArcIdx[gsSelectionMark_int - 1] + stdCirIdx[stdCirIdxLen / 2]] - 
                              pArray[PrecisionCir__1 * stdArcIdx[gsSelectionMark_int - 1]]) / 2.0);
		else
		    snapPoints.append(m_ptCenter);
        break;
	case AcDb::kOsModeQuad:
	    if(gsSelectionMark_int <= stdArcIdxLen)
        {
          snapPoints.append(pArray[PrecisionCir__1 * stdArcIdx[gsSelectionMark_int - 1]]);
          snapPoints.append(pArray[PrecisionCir__1 * stdArcIdx[gsSelectionMark_int - 1] + stdCirIdx[stdCirIdxLen / 4]]);
          snapPoints.append(pArray[PrecisionCir__1 * stdArcIdx[gsSelectionMark_int - 1] + stdCirIdx[stdCirIdxLen / 2]]);
          snapPoints.append(pArray[PrecisionCir__1 * stdArcIdx[gsSelectionMark_int - 1] + stdCirIdx[stdCirIdxLen * 3 / 4]]);
        }
		else if(gsSelectionMark_int < PRIMARY3D_WIREFRAME_LINENUM * 2)
        {
            if(m_nDirection == 2)
            {
                snapPoints.append(pArray[stdCirIdx[gsSelectionMark_int - stdArcIdxLen - 1]]);
                snapPoints.append(pArray[stdCirIdx[gsSelectionMark_int - stdArcIdxLen - 1] + PrecisionCir__1 * stdArcIdx[stdArcIdxLen / 4]]);
                snapPoints.append(pArray[stdCirIdx[gsSelectionMark_int - stdArcIdxLen - 1] + PrecisionCir__1 * stdArcIdx[stdArcIdxLen / 2]]);
                snapPoints.append(pArray[stdCirIdx[gsSelectionMark_int - stdArcIdxLen - 1] + PrecisionCir__1 * stdArcIdx[stdArcIdxLen * 3 / 4]]);
            }
        }
	    break;
    case AcDb::kOsModeNode:
		break;
    case AcDb::kOsModeIns:
		snapPoints.append(m_ptCenter);
		break;
    case AcDb::kOsModePerp:
        {
			//重新定义对象垂直正交点的捕捉方式，同时满足实体模型和线框模型的捕捉 
			//modified by szw 2009.11.18 : begin
			AcGePoint3d pt;
            if(gsSelectionMark_int <= stdArcIdxLen)
            {
                AcGeCircArc3d cirArc(pArray[stdArcIdx[gsSelectionMark_int - 1] * PrecisionCir__1], 
                                     pArray[stdArcIdx[gsSelectionMark_int - 1] * PrecisionCir__1 + stdCirIdx[stdCirIdxLen / 3]] , 
                                     pArray[stdArcIdx[gsSelectionMark_int - 1] * PrecisionCir__1 + stdCirIdx[stdCirIdxLen * 2 / 3]]);
                AcGeCircArc3d cir(cirArc.center(), cirArc.normal(), cirArc.radius());
                pt = cir.closestPointTo(lastPoint);
                snapPoints.append(pt);
	        }
			else if(gsSelectionMark_int < PRIMARY3D_WIREFRAME_LINENUM * 2)
            {
                if(m_nDirection == 2 && m_dDiameter1 == m_dDiameter2)
                {
                    AcGeCircArc3d cirArc(pArray[stdCirIdx[gsSelectionMark_int - stdArcIdxLen - 1]], 
                                         pArray[stdCirIdx[gsSelectionMark_int - stdArcIdxLen - 1] + PrecisionCir__1 * stdArcIdx[stdArcIdxLen / 3]], 
                                         pArray[stdCirIdx[gsSelectionMark_int - stdArcIdxLen - 1] + PrecisionCir__1 * stdArcIdx[stdArcIdxLen * 2 / 3]]);
                    AcGeCircArc3d cir(cirArc.center(), cirArc.normal(), cirArc.radius());
                    pt = cir.closestPointTo(lastPoint);
                    snapPoints.append(pt);
                }
                else
                {
                    if(getStartCenDist() == getEndCenDist() && m_dDiameter1 == m_dDiameter2)
                    {
                        AcGeCircArc3d cir(pArray[stdCirIdx[gsSelectionMark_int - stdArcIdxLen - 1]], 
                                          pArray[stdCirIdx[gsSelectionMark_int - stdArcIdxLen - 1] + PrecisionCir__1 * stdArcIdx[stdArcIdxLen / 2]], 
                                          pArray[stdCirIdx[gsSelectionMark_int - stdArcIdxLen - 1] + PrecisionCir__1 * stdArcIdx[stdArcIdxLen - 1]]);
                        pt = cir.closestPointTo(lastPoint);
                        snapPoints.append(pt);
                    }
                    else
                    {
                        AcGePoint3dArray pts;
                        for(int j = 0; j < PrecisionArc__1; ++j)
                            pts.append(pArray[stdCirIdx[gsSelectionMark_int - stdArcIdxLen - 1] + j * PrecisionCir__1]);
                        AcGeCubicSplineCurve3d curve(pts);
                        pt = curve.closestPointTo(lastPoint);
                        snapPoints.append(pt);
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
			//重新定义对象最近点的捕捉方式，同时满足实体模型和线框模型的捕捉 
			//modified by szw 2009.11.18 : begin
			AcGePoint3d pt;
            if(gsSelectionMark_int <= stdArcIdxLen)
            {
                AcGeCircArc3d cirArc(pArray[stdArcIdx[gsSelectionMark_int - 1] * PrecisionCir__1], 
                                     pArray[stdArcIdx[gsSelectionMark_int - 1] * PrecisionCir__1 + stdCirIdx[stdCirIdxLen / 3]] , 
                                     pArray[stdArcIdx[gsSelectionMark_int - 1] * PrecisionCir__1 + stdCirIdx[stdCirIdxLen * 2 / 3]]);
                AcGeCircArc3d cir(cirArc.center(), cirArc.normal(), cirArc.radius());
                pt = cir.projClosestPointTo(pickPoint, viewDir);
                snapPoints.append(pt);
	        }
			else if(gsSelectionMark_int < PRIMARY3D_WIREFRAME_LINENUM * 2)
            {
                if(m_nDirection == 2 && m_dDiameter1 == m_dDiameter2)
                {
                    AcGeCircArc3d cirArc(pArray[stdCirIdx[gsSelectionMark_int - stdArcIdxLen - 1]], 
                                         pArray[stdCirIdx[gsSelectionMark_int - stdArcIdxLen - 1] + PrecisionCir__1 * stdArcIdx[stdArcIdxLen / 3]], 
                                         pArray[stdCirIdx[gsSelectionMark_int - stdArcIdxLen - 1] + PrecisionCir__1 * stdArcIdx[stdArcIdxLen * 2 / 3]]);
                    AcGeCircArc3d cir(cirArc.center(), cirArc.normal(), cirArc.radius());
                    pt = cir.projClosestPointTo(pickPoint, viewDir);
                    snapPoints.append(pt);
                }
                else
                {
                    if(getStartCenDist() == getEndCenDist() && m_dDiameter1 == m_dDiameter2)
                    {
                        AcGeCircArc3d cir(pArray[stdCirIdx[gsSelectionMark_int - stdArcIdxLen - 1]], 
                                          pArray[stdCirIdx[gsSelectionMark_int - stdArcIdxLen - 1] + PrecisionCir__1 * stdArcIdx[stdArcIdxLen / 2]], 
                                          pArray[stdCirIdx[gsSelectionMark_int - stdArcIdxLen - 1] + PrecisionCir__1 * stdArcIdx[stdArcIdxLen - 1]]);
                        pt = cir.projClosestPointTo(pickPoint, viewDir);
                        snapPoints.append(pt);
                    }
                    else
                    {
                        AcGePoint3dArray pts;
                        for(int j = 0; j < PrecisionArc__1; j++)
                            pts.append(pArray[stdCirIdx[gsSelectionMark_int - stdArcIdxLen - 1] + j * PrecisionCir__1]);
                        AcGeCubicSplineCurve3d curve;
                        curve = AcGeCubicSplineCurve3d(pts);
                        pt = curve.projClosestPointTo(pickPoint, viewDir);
                        snapPoints.append(pt);
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


#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDTorus::subGetGripPoints(
	AcGePoint3dArray& gripPoints,
	AcDbIntArray& osnapModes,
	AcDbIntArray& geomIds) const
#else
Acad::ErrorStatus PDTorus::getGripPoints(
	AcGePoint3dArray& gripPoints,
	AcDbIntArray& osnapModes,
	AcDbIntArray& geomIds) const
#endif
{
    assertReadEnabled();

/*    AcGeVector3d startVect = (m_ptP1-m_ptCenter).normal();
	AcGeVector3d endVect = (m_ptP2-m_ptCenter).normal();
	AcGeVector3d faceVect;
  switch(m_nDirection)
  {
  case 0:
  case 1:
    faceVect = startVect.crossProduct(endVect).normal();
    break;
  case 2:
  case 3:
    faceVect.set(m_ptP2.x, m_ptP2.y, m_ptP2.z).normalize();
    break;
  default:
    faceVect = startVect.crossProduct(endVect).normal();
    break;
  }
	AcGeCubicSplineCurve3d varcur;
	double Radius = m_ptP1.distanceTo(m_ptCenter);

	double   dangle;

	switch(m_nDirection){
	case 0:
		dangle=endVect.angleTo(startVect);
		varcur = AcGeCubicSplineCurve3d(AcGeCircArc3d(
			m_ptCenter,faceVect,startVect,Radius,0,dangle));
		break;
	case 1:
		dangle=2*PI-endVect.angleTo(startVect);
		varcur = AcGeCubicSplineCurve3d(AcGeCircArc3d(
			m_ptCenter,-faceVect,startVect,Radius,0,dangle));
		break;
	case 2:
		dangle=2*PI;
		//faceVect = endVect;
		varcur = AcGeCubicSplineCurve3d(AcGeCircArc3d(
			m_ptCenter,faceVect,Radius));
		break;
	case 3:
		dangle=PI;
		//faceVect = endVect;
		//startVect = startVect.orthoProject(faceVect);
		varcur = AcGeCubicSplineCurve3d(AcGeCircArc3d(
			m_ptCenter,faceVect,startVect,Radius,0,dangle));
		break;
	default:
		break;
	}
	AcGePoint3d pt;
	pt = varcur.startPoint();
	gripPoints.append(pt);
	AcGeVector3d curvect = AcGeVector3d(pt-m_ptCenter).crossProduct(faceVect).normal();
	AcGeCubicSplineCurve3d cir1 = AcGeCubicSplineCurve3d(AcGeCircArc3d(
			pt,curvect,AcGeVector3d(pt- m_ptCenter),m_dDiameter1/2,0,2*PI));

	pt = varcur.endPoint();
	gripPoints.append(pt);
	curvect = AcGeVector3d(pt-m_ptCenter).crossProduct(faceVect).normal();
	AcGeCubicSplineCurve3d cir2 = AcGeCubicSplineCurve3d(AcGeCircArc3d(
			pt,curvect,AcGeVector3d(pt- m_ptCenter),m_dDiameter2/2,0,2*PI));

	double parameter;
	AcGePoint3d pt1,pt2;
	for(int i =0;i<4;i++){
		parameter = PI/2*i;
		pt1 = cir1.evalPoint(parameter);
		gripPoints.append(pt1);
		pt2 = cir2.evalPoint(parameter);
		gripPoints.append(pt2);
	}
*/

	return Acad::eOk; 
}

/*
Acad::ErrorStatus PDTorus::explode(AcDbVoidPtrArray& entitySet) const
{ //AutoCAD2002版采用面方式，AutoCAD 2004采用体方式, AutoCAD R14版有问题。
assertReadEnabled();

#ifdef _OBJECTARX2004_
Acad::ErrorStatus es = Acad::eOk;

AcGeVector3d Startvect=(m_ptP1-m_ptCenter).normal();
AcGeVector3d Endvect=(m_ptP2-m_ptCenter).normal();
AcGeVector3d Facevect = getNormal();

if(Facevect == AcGeVector3d::kIdentity)
return Acad::eNotApplicable;

double startDist = getStartCenDist();
double endDist = getEndCenDist();

AcDbCurve *pCurve = NULL;
double pathLen;
if(fabs(startDist - endDist) < 1e-4)
{
switch(m_nDirection)
{
case 2:
if(m_dDiameter1 == m_dDiameter2)
{
AcDbCircle *pCir;
pCir = new AcDbCircle(m_ptCenter, Facevect, startDist);
pathLen = 2.0 * PI * startDist;
pCurve = pCir;
}
else
return explodeToSurface(entitySet);
break;
case 3:
if(m_dDiameter1 == m_dDiameter2)
{
AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax, Ay;
if (fabs(Facevect[X]) <1.0/64 && fabs(Facevect[Y])<1.0/64) 
Ax = Wy.crossProduct(Facevect);
else
Ax = Wz.crossProduct(Facevect);
Ax.normalize();
Ay = Facevect.crossProduct(Ax);
Ay.normalize();

double startAng, endAng;
startAng = Ax.angleTo(m_ptP1 - m_ptCenter, Facevect);
endAng = startAng + PI;
if(endAng > 2.0 * PI)
endAng -= 2.0 * PI;
AcDbArc *pArc;
pArc = new AcDbArc(m_ptCenter, Facevect, startDist, startAng, endAng);
pathLen = PI * startDist;
pCurve = pArc;
}
else
return explodeToSurface(entitySet);
break;
case 0:
case 1:
default:
if(m_dDiameter1 == m_dDiameter2)                   
{
AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax, Ay;
if (fabs(Facevect[X]) <1.0/64 && fabs(Facevect[Y])<1.0/64) 
Ax = Wy.crossProduct(Facevect);
else
Ax = Wz.crossProduct(Facevect);
Ax.normalize();
Ay = Facevect.crossProduct(Ax);
Ay.normalize();

double startAng, endAng;
startAng = Ax.angleTo(m_ptP1 - m_ptCenter, Facevect);
endAng = Ax.angleTo(m_ptP2 - m_ptCenter, Facevect);
AcDbArc *pArc;
pArc = new AcDbArc(m_ptCenter, Facevect, startDist, startAng, endAng);
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
getActDivid(m_nprecisionArc, dividDblArc, stdArcIdx);

int precisionArc__1 = dividDblArc.length();

AcDbPolyline *pPolyline = NULL;
pPolyline = new AcDbPolyline(precisionArc__1);

double dangle = getAngle();
double deltaAngle = dangle / m_nprecisionArc;
double deltaDist = (endDist - startDist) / m_nprecisionArc;
for(int i = 0; i < precisionArc__1; ++i)
{
AcGePoint3d varCenter;
AcGeVector3d P1CenterVec = Startvect;
varCenter = m_ptCenter + 
P1CenterVec.rotateBy(deltaAngle * dividDblArc[i], Facevect) * 
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

AcDbCircle cir;
cir.setCenter(m_ptP1);
cir.setNormal(Facevect.crossProduct(Startvect));
cir.setRadius(m_dDiameter1 / 2.0);

AcDbVoidPtrArray curveSegments;
curveSegments.append(&cir);

AcDbVoidPtrArray regions;
es = AcDbRegion::createFromCurves(curveSegments, regions);

if(es == Acad::eOk && !regions.isEmpty())
{
AcDb3dSolid *pBody;
pBody = new AcDb3dSolid;
es = pBody->extrudeAlongPath((AcDbRegion*)(regions[0]), pCurve,
atan((m_dDiameter1 / 2.0 - m_dDiameter2 / 2.0) / pathLen));
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

*/
#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDTorus::subExplode(AcDbVoidPtrArray& entitySet) const
#else
Acad::ErrorStatus PDTorus::explode(AcDbVoidPtrArray& entitySet) const
#endif
{ //AutoCAD2002版采用面方式，AutoCAD 2004采用体方式, AutoCAD R14版有问题。
    assertReadEnabled();

#ifdef _OBJECTARX2004_
    Acad::ErrorStatus es = Acad::eOk;

	AcGeVector3d Facevect = getNormal();
	AcGeVector3d v1,v2;
	v1 = (m_ptP1-m_ptCenter).normal();
	v2 = getNormal();
	if(isLeftCoord(v1,v2,m_v3))
		Facevect = -Facevect;
	if(Facevect == AcGeVector3d::kIdentity)
		return Acad::eNotApplicable;

	//将P1或者P2面放到XY平面（谁的半径小放谁），利用P1P,P2P的夹角生成圆弧路径

	//---------------------------------------------------------------------
	double diameterCircle = min(m_dDiameter1, m_dDiameter2);
	double radiusArc = getStartCenDist();
	AcDbCurve *pCurve = NULL;
	double pathLen = 0;

	////如果m_ptP1和m_ptP2到m_ptP的距离不等，则用面表示
	if (fabs(getStartCenDist()-getEndCenDist()) > SIGMA)
	{
		return explodeToSurface(entitySet);
	}

	//path
	switch(m_nDirection)
	{
	case 2:
		{
			//当变径且拉伸路径为圆环时，要将拉伸路径用圆弧近似表示，否则拉伸的结果是不变径
			AcGePoint3d ptCenter(radiusArc, 0,0);
			AcGeVector3d faceNor(0,1,0);

			//不变径时，路径用圆环表示
			if (fabs(m_dDiameter1-m_dDiameter2) <= SIGMA)
			{
				AcDbCircle *pArc;
				pArc = new AcDbCircle(ptCenter, faceNor, radiusArc);
				pathLen = PI * radiusArc * 2;
				pCurve = pArc;
			} 
			//变径时路径用圆弧近似
			else
			{
				AcDbArc *pArc;
				pArc = new AcDbArc(ptCenter, faceNor, radiusArc, 0, 1.999*PI);
				pathLen = PI * radiusArc * 1.999;
				pCurve = pArc;
			}
		}
		break;
	case 3:
		{
			AcGePoint3d ptCenter(radiusArc, 0,0);
			AcGeVector3d faceNor(0,1,0);
			AcDbArc *pArc;
			pArc = new AcDbArc(ptCenter, faceNor, radiusArc, 0, PI);
			pathLen = PI * radiusArc;
			pCurve = pArc;
		}
		break;
	case 0:
	case 1:
		{
			AcGePoint3d ptCenter(radiusArc, 0,0);
			AcGeVector3d faceNor(0,1,0);
			double angle = getAngle();
			AcDbArc *pArc;
			pArc = new AcDbArc(ptCenter, faceNor, radiusArc, 0, angle);
			pathLen = radiusArc * angle;
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
		return explodeToSurface(entitySet);

	//curve
	AcDbCircle cir;
	AcGePoint3d originPoint(0,0,0);
	AcGeVector3d nor(0, 0, 1);
	cir.setCenter(originPoint);
	cir.setNormal(nor);
	cir.setRadius(diameterCircle / 2.0);

	AcDbVoidPtrArray curveSegments;
	curveSegments.append(&cir);

	AcDbVoidPtrArray regions;
	es = AcDbRegion::createFromCurves(curveSegments, regions);

	//transform
	if(es == Acad::eOk && !regions.isEmpty())
	{
		AcDb3dSolid *pBody;
		pBody = new AcDb3dSolid;
		es = pBody->extrudeAlongPath((AcDbRegion*)(regions[0]), pCurve,
			-atan((max(m_dDiameter1, m_dDiameter2) / 2.0 -  diameterCircle/ 2.0) / pathLen));

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

		//变换到原坐标系
		AcGeMatrix3d mat;
		AcGePoint3d pOrign(0,0,0);
		AcGePoint3d ori(0,0,0);
		AcGeVector3d vecx(1,0,0);
		AcGeVector3d vecy(0,1,0);
		AcGeVector3d vecz(0,0,1);
		AcGeVector3d axisX(1,0,0);
		AcGeVector3d axisY(0,1,0);
		AcGeVector3d axisZ(0,0,1);

		if (m_nDirection==0 || m_nDirection==1)
		{
			if (m_dDiameter1 >= m_dDiameter2)
			{
				pOrign = m_ptP2;
			
				axisX = (m_ptCenter-m_ptP2).normal();
				axisY = -1*Facevect;
				axisZ = axisX.crossProduct(axisY);
			}
			else
			{
				pOrign = m_ptP1;
				
				axisX = (m_ptCenter-m_ptP1).normal();
				axisY = Facevect;
				axisZ = axisX.crossProduct(axisY);
			}
		} 
		else if(m_nDirection==2)
		{
			pOrign = m_ptP1;
			axisX = (m_ptCenter-m_ptP1).normal();

			if (m_dDiameter1 >= m_dDiameter2)
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
			if (m_dDiameter1 >= m_dDiameter2)
			{
				pOrign.x = 2*m_ptCenter.x - m_ptP1.x;
				pOrign.y = 2*m_ptCenter.y - m_ptP1.y;
				pOrign.z = 2*m_ptCenter.z - m_ptP1.z;

				axisX = (m_ptCenter-pOrign).normal();
				axisY = -1*Facevect;
			}
			else
			{
				pOrign = m_ptP1;

				axisX = (m_ptCenter-m_ptP1).normal();
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

//  [9/5/2007 suzhiyong]
Acad::ErrorStatus PDTorus::explodeTo3DSolid(AcDb3dSolid* &p3dSolid) const
{
	assertReadEnabled();

#ifdef _OBJECTARX2004_
	Acad::ErrorStatus es = Acad::eOk;

	AcGeVector3d Facevect = getNormal();
	AcGeVector3d v1,v2;
	v1 = (m_ptP1-m_ptCenter).normal();
	v2 = getNormal();
	if(isLeftCoord(v1,v2,m_v3))
		Facevect = -Facevect;
	if(Facevect == AcGeVector3d::kIdentity)
		return Acad::eNotApplicable;

	//将P1或者P2面放到XY平面（谁的半径小放谁），利用P1P,P2P的夹角生成圆弧路径

	//---------------------------------------------------------------------
	double diameterCircle = min(m_dDiameter1, m_dDiameter2);
	double radiusArc = getStartCenDist();
	AcDbCurve *pCurve = NULL;
	double pathLen = 0;

	////如果m_ptP1和m_ptP2到m_ptP的距离不等，则用面表示
	//if (fabs(getStartCenDist()-getEndCenDist()) > SIGMA)
	//{
	//	return Acad::eNotApplicable;
	//}

	//path
	switch(m_nDirection)
	{
	case 2:
		{
			//当变径且拉伸路径为圆环时，要将拉伸路径用圆弧近似表示，否则拉伸的结果是不变径
			AcGePoint3d ptCenter(radiusArc, 0,0);
			AcGeVector3d faceNor(0,1,0);

			//不变径时，路径用圆环表示
			if (fabs(m_dDiameter1-m_dDiameter2) <= SIGMA)
			{
				AcDbCircle *pArc;
				pArc = new AcDbCircle(ptCenter, faceNor, radiusArc);
				pathLen = PI * radiusArc * 2;
				pCurve = pArc;
			} 
			//变径时路径用圆弧近似
			else
			{
				AcDbArc *pArc;
				pArc = new AcDbArc(ptCenter, faceNor, radiusArc, 0, 1.999*PI);
				pathLen = PI * radiusArc * 1.999;
				pCurve = pArc;
			}
		}
		break;
	case 3:
		{
			AcGePoint3d ptCenter(radiusArc, 0,0);
			AcGeVector3d faceNor(0,1,0);
			AcDbArc *pArc;
			pArc = new AcDbArc(ptCenter, faceNor, radiusArc, 0, PI);
			pathLen = PI * radiusArc;
			pCurve = pArc;
		}
		break;
	case 0:
	case 1:
		{
			AcGePoint3d ptCenter(radiusArc, 0,0);
			AcGeVector3d faceNor(0,1,0);
			double angle = getAngle();
			AcDbArc *pArc;
			pArc = new AcDbArc(ptCenter, faceNor, radiusArc, 0, angle);
			pathLen = radiusArc * angle;
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
		return Acad::eNotApplicable;

	//curve
	AcDbCircle cir;
	AcGePoint3d originPoint(0,0,0);
	AcGeVector3d nor(0, 0, 1);
	cir.setCenter(originPoint);
	cir.setNormal(nor);
	cir.setRadius(diameterCircle / 2.0);

	AcDbVoidPtrArray curveSegments;
	curveSegments.append(&cir);

	AcDbVoidPtrArray regions;
	es = AcDbRegion::createFromCurves(curveSegments, regions);

	//transform
	if(es == Acad::eOk && !regions.isEmpty())
	{
		AcDb3dSolid *pBody;
		pBody = new AcDb3dSolid;
		es = pBody->extrudeAlongPath((AcDbRegion*)(regions[0]), pCurve,
			-atan((max(m_dDiameter1, m_dDiameter2) / 2.0 -  diameterCircle/ 2.0) / pathLen));

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

		//变换到原坐标系
		AcGeMatrix3d mat;
		AcGePoint3d pOrign(0,0,0);
		AcGePoint3d ori(0,0,0);
		AcGeVector3d vecx(1,0,0);
		AcGeVector3d vecy(0,1,0);
		AcGeVector3d vecz(0,0,1);
		AcGeVector3d axisX(1,0,0);
		AcGeVector3d axisY(0,1,0);
		AcGeVector3d axisZ(0,0,1);

		if (m_nDirection==0 || m_nDirection==1)
		{
			if (m_dDiameter1 >= m_dDiameter2)
			{
				pOrign = m_ptP2;

				axisX = (m_ptCenter-m_ptP2).normal();
				axisY = -1*Facevect;
				axisZ = axisX.crossProduct(axisY);
			}
			else
			{
				pOrign = m_ptP1;

				axisX = (m_ptCenter-m_ptP1).normal();
				axisY = Facevect;
				axisZ = axisX.crossProduct(axisY);
			}
		} 
		else if(m_nDirection==2)
		{
			pOrign = m_ptP1;
			axisX = (m_ptCenter-m_ptP1).normal();

			if (m_dDiameter1 >= m_dDiameter2)
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
			if (m_dDiameter1 >= m_dDiameter2)
			{
				pOrign.x = 2*m_ptCenter.x - m_ptP1.x;
				pOrign.y = 2*m_ptCenter.y - m_ptP1.y;
				pOrign.z = 2*m_ptCenter.z - m_ptP1.z;

				axisX = (m_ptCenter-pOrign).normal();
				axisY = -1*Facevect;
			}
			else
			{
				pOrign = m_ptP1;

				axisX = (m_ptCenter-m_ptP1).normal();
				axisY = Facevect;
			}
			axisZ = axisX.crossProduct(axisY);
		}
		
		if( m_nDirection<2 && axisY.isPerpendicularTo( vecz)){
			double ang = axisX.angleTo( vecz);
			if( fabs(ang-PI/2)<=PI/180){
				axisX =(axisX.orthoProject( vecz)).normal();
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

Acad::ErrorStatus PDTorus::explodeToSurface(AcDbVoidPtrArray& entitySet) const
{
    assertReadEnabled();

    Acad::ErrorStatus es = Acad::eOk;
    int i;

    AcDbVoidPtrArray curSeg, resRegion;
    AcDbCircle *pCir;
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
    pCir = new AcDbCircle(m_ptP1, curvect, m_dDiameter1 / 2.0);
    curSeg.append(pCir);
    es = AcDbRegion::createFromCurves(curSeg, resRegion);
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
    for(i = 0; i < curSeg.length(); ++i)
        delete (AcRxObject*)curSeg[i];

    curSeg.setLogicalLength(0);
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
           pCir = new AcDbCircle(m_ptP2, curvect, m_dDiameter2 / 2.0);
    break;
	case 2:
		   pCir = new AcDbCircle(m_ptP1, curvect, m_dDiameter2 / 2.0);
	break;
	case 3:
		{
		   	AcGePoint3d P2=m_ptP1;
        	P2.rotateBy(PI,faceVect,m_ptCenter);
            pCir = new AcDbCircle(P2, curvect, m_dDiameter2 / 2.0);
		}  
	break;
	}//added by linlin 20050922

    curSeg.append(pCir);
    es = AcDbRegion::createFromCurves(curSeg, resRegion);
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
    for(i = 0; i < curSeg.length(); ++i)
        delete (AcRxObject*)curSeg[i];

    ucs_mat.setCoordSystem(ucs_org, ucs_x, ucs_y, ucs_z);
//#ifdef _OBJECTARX14_
//    acdbSetCurrentUCS(ucs_mat);
//#else
//    acedSetCurrentUCS(ucs_mat);
//#endif

    AcGePoint3dArray pArray;
    AcGeIntArray stdArcIdx, stdCirIdx;
    int actPrecisionArc, actPrecisionCir;
    getVertices(m_nprecisionArc, m_nprecisionCir, pArray, 
        stdArcIdx, stdCirIdx, actPrecisionArc, actPrecisionCir);

    AcDbPolygonMesh *pMesh;
    pMesh = new AcDbPolygonMesh(AcDb::kSimpleMesh, actPrecisionArc + 1, actPrecisionCir + 1,
        pArray, Adesk::kFalse, Adesk::kFalse);
    pMesh->setPropertiesFrom(this);
    entitySet.append(pMesh);

    return Acad::eOk;
}

// 取得合适的剖分精度，如果实体较小，则剖分精度就会低，如果实体较大，则剖分
// 精度不大于实体限定的剖分精度m_dDividPrecision。
int PDTorus::getMaxPrecision(AcGiWorldDraw *mode) const
{
    assertReadEnabled();
    if(!getCalMaxPrecision())
        return m_dDividPrecision;
    double radius = m_ptCenter.distanceTo(m_ptP1);
    int pre = (int)(PI / acos(1.0 - mode->deviation(kAcGiMaxDevForCircle, m_ptP1)
              / radius));
    if(pre > m_dDividPrecision)
        return m_dDividPrecision;
    else if(pre > 8)
        return pre;
    else
        return 8;
}

// 取得合适的剖分精度，如果实体较小，则剖分精度就会低，如果实体较大，则剖分
// 精度不大于实体限定的剖分精度m_nprecisionArc、m_nprecisionCir。
int PDTorus::getMaxPrecisionArc(AcGiWorldDraw *mode) const
{
    assertReadEnabled();
    if(!getCalMaxPrecision())
        return m_nprecisionArc;
    double radius = m_ptCenter.distanceTo(m_ptP1);
    int pre = (int)(PI / acos(1.0 - mode->deviation(kAcGiMaxDevForCircle, m_ptP1)
              / radius));
    if(pre > (int)m_nprecisionArc)
        return m_nprecisionArc;
    else if(pre > 8)
        return pre;
    else
        return 8;
}
int PDTorus::getMaxPrecisionCir(AcGiWorldDraw *mode) const
{
    assertReadEnabled();
    if(!getCalMaxPrecision())
        return m_nprecisionCir;
    double dia;
    if(m_dDiameter1 > m_dDiameter2)
        dia = m_dDiameter1;
    else
        dia = m_dDiameter2;
    int pre = (int)(PI / acos(1.0 - mode->deviation(kAcGiMaxDevForCircle, m_ptP1)
        * 2.0 / dia));
    if(pre > (int)m_nprecisionCir)
        return m_nprecisionCir;
    else if(pre > 8)
        return pre;
    else
        return 8;
}

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDTorus::subGetGeomExtents(AcDbExtents& extents) const
#else
Acad::ErrorStatus PDTorus::getGeomExtents(AcDbExtents& extents) const
#endif
{
    assertReadEnabled();
    extents.set(m_ptP1, m_ptP1);
    AcGePoint3dArray pArray;
    AcGeIntArray stdArcIdx, stdCirIdx;
    int actPrecisionArc, actPrecisionCir;
    getVertices(m_nprecisionArc, m_nprecisionCir, pArray, 
        stdArcIdx, stdCirIdx, actPrecisionArc, actPrecisionCir);
    int i, arrlen = pArray.length();
    for(i = 0; i < arrlen; ++i)
        extents.addPoint(pArray[i]);
    return Acad::eOk;
}

#ifdef _OBJECTARX2010_
void PDTorus::subList() const
#else
void PDTorus::list() const
#endif
{
	assertReadEnabled();
#ifdef _OBJECTARX2010_
	PDPrimary3D::subList();
#else
	PDPrimary3D::list();
#endif
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4822/*"\n 起始断面的圆心(%f,%f,%f)\n"*/,m_ptP1.x,m_ptP1.y,m_ptP1.z);
    ads_printf(LANGUAGE_DEFINE_MACRO_V25_4823/*" 起始断面的直径=%.3f\n"*/,m_dDiameter1,m_dDiameter1);
	if(m_nDirection==2||m_nDirection==3)
		ads_printf(LANGUAGE_DEFINE_MACRO_V25_4810/*" 法线方向(%f,%f,%f)\n"*/,m_ptP2.x,m_ptP2.y,m_ptP2.z);
	else 
	    ads_printf(LANGUAGE_DEFINE_MACRO_V25_4824/*" 终止断面的圆心(%f,%f,%f)\n"*/,m_ptP2.x,m_ptP2.y,m_ptP2.z);
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4825/*" 终止断面的直径=%.3f\n"*/,m_dDiameter2,m_dDiameter2);
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4813/*" 圆环圆心(%f,%f,%f)\n"*/,m_ptCenter.x,m_ptCenter.y,m_ptCenter.z);
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4826/*" 弧度精度:     %d\n"*/,m_nprecisionArc);
    ads_printf(LANGUAGE_DEFINE_MACRO_V25_4827/*" 圆周方向精度: %d\n"*/,m_nprecisionCir);
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4828/*" 方向值:       %d\n\n"*/,m_nDirection);
	return ;
}// added by linlin 20050811

void PDTorus::createBody()
{
    assertWriteEnabled();
#ifdef _OBJECTARX2004_
/*
    AcGePoint3dArray pArray;
    AcGeIntArray stdArcIdx, stdCirIdx;
    int actPrecisionArc, actPrecisionCir;
	getVertices(m_nprecisionArc, m_nprecisionCir, 
                pArray, stdArcIdx, stdCirIdx, 
                actPrecisionArc, actPrecisionCir);

	AcGePoint3d* vertices;
	int n = actPrecisionArc+1;
	int p = actPrecisionCir;

    vertices = new AcGePoint3d[p*2];

	int i, j, k, m;

	for( i =0; i<n-1; ++i){
		for( m=0, j=0; j<2; ++j){
			for( k=0; k<p; ++k){
				vertices[m] = pArray[ (p+1)*(i+j) + k];
				m ++;
			}
		}
		m_3dGeom += Body::convexHull((Point3d*)vertices, p*2);
	}

    delete [] vertices;*/

	AcGeVector3d normal1, normaln;
    AcGePoint3dArray pArray;
    AcGeIntArray stdArcIdx, stdCirIdx;
    int actPrecisionArc, actPrecisionCir;
	m_no8 =true;
	getVertices(m_nprecisionArc, m_nprecisionCir, 
                pArray, stdArcIdx, stdCirIdx, 
                actPrecisionArc, actPrecisionCir);
	m_no8 =false;

	AcGePoint3d* vertices;
	int n = actPrecisionArc+1;
	int p = actPrecisionCir;

	Body **profiles;
	Face *f;

	profiles = new Body*[n];
    vertices = new AcGePoint3d[p];

	bool isclose =( m_nDirection==2); // && fabs( m_dDiameter1-m_dDiameter2)<1E-3);

	int i, j, type;
	for( i =0; i<n; ++i){
		profiles[i] =new Body();
		for( j=0; j<p; ++j){
			vertices[j] = pArray[ i*(p+1)+j];
		}
		if( i==0){
			AcGeVector3d vn = ( vertices[1]-vertices[0]).crossProduct( vertices[2]-vertices[0]);
			AcGeVector3d vp = ( pArray[p+2] - pArray[0]);
			if( vn.dotProduct( vp)>0) type =0;
			else type =1;
			
			normal1 = vn;
		}
		if( i==n-1){
			normaln = ( vertices[1]-vertices[0]).crossProduct( vertices[2]-vertices[0]);
		}

		f = new Face( (Point3d*)vertices, p, type, false, profiles[i]);
		
	}


	m_3dGeom = Body::skin(	profiles,
							isclose? n-1: n,
                               isclose,
                               true,
                               NULL,
                               true,
                               true);

	m_3dGeom.setEdgeFlags( kOn, AEF);

	if( !isclose){
		Face * pface = m_3dGeom.faceList();
		while( pface!=NULL){
			if( normal1.isParallelTo( pface->plane().normal) ||
				normaln.isParallelTo( pface->plane().normal)){
				
				Edge* eg = pface->edgeLoop();
				Edge* eg1 =eg;
				do{
					eg->setApproxFlag( kOff);
					eg = eg->next();
				}while( eg!=eg1);				
			}
			pface = pface->next();
		}
	}
	
//	m_3dGeom.cleanUpNonManifoldEdgesAndCoincidentFaces();

	for( i=0; i<n; ++i) delete profiles[i];
    delete [] vertices;
	delete [] profiles;

#endif
}
AcGePoint3dArray PDTorus::getPtOnCentAix(int Precision)
{
	assertReadEnabled();
	AcGePoint3dArray tmpPts;
	if(m_nDirection == 2 || m_nDirection == 3)
		return tmpPts;


	double startDist = getStartCenDist();
	double endDist = getEndCenDist();
	if(fabs(startDist -endDist) / (startDist + endDist) < TOURS_SIMPLIZE_FACTOR *2)
	{
		//如果可以近似就当成圆弧处理算了
		return tmpPts;
	}
	double dangle = getAngle();
	int precisionCir__1 = Precision + 1;
	AcGeVector3d startVect = (m_ptP1 - m_ptCenter).normal();
	AcGeVector3d faceVect = getNormal();

	double r1 = m_dDiameter1 / 2.0;
	double deltaAngle = dangle / Precision;
	double deltaDist = (endDist - startDist) / Precision;
	AcGePoint3d varCenter;
	AcGeVector3d P1CenterVec = startVect;
	varCenter  = m_ptCenter;
	AcGeVector3d curvect;
	AcGePoint3d sp,point;
	for(int i = 0; i <= Precision; ++i)
	{	
		P1CenterVec.normalize();
		varCenter = varCenter + P1CenterVec * startDist;
		tmpPts.append(varCenter);
		P1CenterVec.rotateBy(deltaAngle, faceVect);
		startDist += deltaDist;
	}
	return tmpPts;
}