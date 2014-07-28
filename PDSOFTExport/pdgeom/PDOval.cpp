// PDOval.cpp: implementation of the PDOval class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "language25.h"
#include "PDOval.h"

#include <math.h>
#include <minmax.h>

#include <dbproxy.h>
#include <acgi.h>
#include <gecspl3d.h>
#include <geassign.h>
#include <geell3d.h>
#include <dbelipse.h>
#include <dbents.h>
#include <dbregion.h>
#include <dbsol3d.h>
#include <GEINTARR.H>
#include <adscodes.h>
#include <dbapserv.h>
#include <dbbody.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define VERSION_PDOVAL 1
#define PI 3.1415926535897932384626433832795L

//用于判断椭球封头长短轴的长度差
const double SIGMA = 1.0;

#ifdef _OBJECTARX2000_
ACRX_DXF_DEFINE_MEMBERS(PDOval, PDPrimary3D,
                        AcDb::kDHL_CURRENT, AcDb::kMReleaseCurrent,
                        AcDbProxyEntity::kAllAllowedBits,
	                      PDOVAL, Gle);
#else
ACRX_DXF_DEFINE_MEMBERS(PDOval, PDPrimary3D, AcDbProxyEntity::kAllAllowedBits, PDOVAL, Gle);
#endif

void PDOval::setDefault()
{
  PDPrimary3D::setDefault();
#if defined(NDEBUG) || !defined(_DEBUG)
  m_dlengthA = m_dlengthB = m_dlengthR = 1;
  m_ptCenter = AcGePoint3d::kOrigin;
  m_vect = AcGeVector3d::kZAxis;
  m_nprecisionA = m_nprecisionB = PRIMARY3D_DEFAULT_PRECISION;
#else
  m_dlengthA = m_dlengthB = m_dlengthR = 1000;
  m_ptCenter = AcGePoint3d::kOrigin;
  m_vect = AcGeVector3d::kZAxis;
  m_nprecisionA = m_nprecisionB = PRIMARY3D_DEFAULT_PRECISION;
#endif
}
bool PDOval::isValidData(double &f)
{
  if(!PDPrimary3D::isValidData(f))
    return false;
  if(!isValidFloat(f=m_dlengthA))
    return false;
  if(!isValidFloat(f=m_dlengthB))
    return false;
  if(!isValidFloat(f=m_dlengthR))
    return false;
  return true;
}

//zxb,20090113,增加audit，暂时只check自身成员数据
Acad::ErrorStatus PDOval::audit(AcDbAuditInfo* pAuditInfo)
{
  return auditPDPrimary3D(this,pAuditInfo,_T("PDOval"));
}

PDOval::PDOval(bool HasSnap/* = false*/) : PDPrimary3D(HasSnap)
{
	m_dlengthA = m_dlengthB = m_dlengthR = 1;
    m_ptCenter = AcGePoint3d::kOrigin;
    m_vect = AcGeVector3d::kZAxis;
	m_nprecisionA = m_nprecisionB = PRIMARY3D_DEFAULT_PRECISION;
#ifdef _USEAMODELER_
    createBody();
#endif
}

PDOval::PDOval(double lengthA, double lengthB, double lengthR, 
               const AcGePoint3d pt, const AcGeVector3d vect, 
               Adesk::UInt32 precisionA/* = PRIMARY3D_DEFAULT_PRECISION*/, 
               Adesk::UInt32 precisionB/* = PRIMARY3D_DEFAULT_PRECISION*/, 
               bool HasSnap/* = false*/) : PDPrimary3D(HasSnap)
{
	m_dlengthA = lengthA;
    if(m_dlengthA == 0)
        m_dlengthA = 1;
	m_dlengthB = lengthB;
    if(m_dlengthB == 0)
        m_dlengthB = 1;
	/////////////////////////////////////////////////////
	//// m_dlengthR should not long than m_dlengthB	/////
    m_dlengthR = lengthR;
    if(m_dlengthR == 0)
        m_dlengthR = 1;
	if(m_dlengthR > m_dlengthB)
		m_dlengthR = m_dlengthB;

	m_ptCenter = pt;
	m_vect = vect.normal();

    if(precisionA > 2)
        m_nprecisionA = precisionA;
    else
        m_nprecisionA = PRIMARY3D_DEFAULT_PRECISION;
    if(precisionB > 2)
        m_nprecisionB = precisionB;
    else
        m_nprecisionB = PRIMARY3D_DEFAULT_PRECISION;

    if(m_nprecisionA > m_nprecisionB)
        m_dDividPrecision = m_nprecisionA;
    else
        m_dDividPrecision = m_nprecisionB;

#ifdef _USEAMODELER_
    createBody();
#endif
}

PDOval::~PDOval()
{

}

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDOval::subTransformBy(const AcGeMatrix3d &xform)
#else
Acad::ErrorStatus PDOval::transformBy(const AcGeMatrix3d &xform)
#endif
{
	assertWriteEnabled();
	// TODO: implement this function.
	m_ptCenter.transformBy(xform);
	m_vect.transformBy(xform);
    m_vect.normalize();

	m_dlengthA *= xform.scale();
	m_dlengthB *= xform.scale();
	m_dlengthR *= xform.scale();

#ifdef _OBJECTARX2010_
	return PDPrimary3D::subTransformBy(xform);
#else
	return PDPrimary3D::transformBy(xform);
#endif
}

#ifdef _OBJECTARX2010_
Adesk::Boolean PDOval::subWorldDraw(AcGiWorldDraw* mode)
#else
Adesk::Boolean PDOval::worldDraw(AcGiWorldDraw* mode)
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

	int Precision = 0;
	Adesk::Int16 nRating = getCalMaxPrecision();
	AcGePoint3dArray pArray;
	int actPrecisionA = m_nprecisionA, actPrecisionB = m_nprecisionB;
	int nStepA = 1,nStepB = 1;
	int PrecisionA__1 = actPrecisionA + 1;
	int PrecisionB__1 = actPrecisionB + 1;

	if(nRating == PRIMARY3D_SIMPLIZE_RATE1 ||
		nRating == PRIMARY3D_SIMPLIZE_RATE2)
	{
		//优化
		double dMax = m_dlengthA;
		if(dMax < m_dlengthB)
			dMax = m_dlengthB;
		if(dMax < m_dlengthR)
			dMax = m_dlengthR;
		Precision = (int)(PI / acos(1.0 - mode->deviation(kAcGiMaxDevForCircle, m_ptCenter) / dMax));
		if(Precision < PRIMARY3D_SIMPLIZE_SCALE)
		{
			if (nRating == PRIMARY3D_SIMPLIZE_RATE2)
			{
				//二级简化
				double hgt = getOvalHeight();
				AcGePoint3d pTop;
				AcGePoint3d pt[2];
				pTop = m_ptCenter + hgt * m_vect;
				pt[0] = m_ptCenter;
				pt[1] = pTop;
				if(mode->geometry().polyline(2,pt))
				{
					return Adesk::kTrue;
				}
				mode->geometry().circle(m_ptCenter, m_dlengthR, m_vect);
			}
			else if (nRating == PRIMARY3D_SIMPLIZE_RATE1)
			{
				//一级简化
				int i, j;
				if(mode->geometry().circle(m_ptCenter, m_dlengthR, m_vect))
				{
					return Adesk::kTrue;
				}
				actPrecisionA = actPrecisionB = PRIMARY3D_WIREFRAME_LINENUM4;

				getVertices(actPrecisionA,actPrecisionB,pArray);
				PrecisionA__1 = actPrecisionA + 1;
				PrecisionB__1 = actPrecisionB + 1;

				AcGePoint3dArray pointArray;
				pointArray.setLogicalLength(PrecisionA__1);
				for(i = 0; i < actPrecisionB; i += nStepB)
				{
					for(j = 0; j <= actPrecisionA; ++j)
					{
						pointArray[j] = pArray[i + j * PrecisionB__1];
					}
					if(mode->geometry().polyline(PrecisionA__1, pointArray.asArrayPtr()))
					{
						return Adesk::kTrue;
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
			int gsIndex = 0;
			mode->subEntityTraits().setFillType(kAcGiFillAlways);
			if(m_nprecisionA < PRIMARY3D_WIREFRAME_LINENUM * 2)
				actPrecisionA = PRIMARY3D_WIREFRAME_LINENUM;
			else
			{
				nStepA = actPrecisionA / PRIMARY3D_WIREFRAME_LINENUM;
				actPrecisionA = nStepA * PRIMARY3D_WIREFRAME_LINENUM;
			}
			if(m_nprecisionB < PRIMARY3D_WIREFRAME_LINENUM * 2)
				actPrecisionB = PRIMARY3D_WIREFRAME_LINENUM;
			else
			{
				nStepB = actPrecisionB / PRIMARY3D_WIREFRAME_LINENUM;
				actPrecisionB = nStepB * PRIMARY3D_WIREFRAME_LINENUM;
			} 
			getVertices(actPrecisionA,actPrecisionB,pArray);
			PrecisionA__1 = actPrecisionA + 1;
			PrecisionB__1 = actPrecisionB + 1;
			//添加实体模型各面的GS标记
			//added by szw 2009.11.18 : begin
			mode->subEntityTraits().setSelectionMarker(++gsIndex);
			//added by szw 2009.11.18 : end
            if( mode->geometry().polygon(actPrecisionB, pArray.asArrayPtr()))
		    {
			    return Adesk::kTrue;
		    }

            AcGiEdgeData edgeData;
            int edgeDataLen = actPrecisionA * PrecisionB__1 + PrecisionA__1 * actPrecisionB;
            Adesk::UInt8* edgeVisibility = new Adesk::UInt8[edgeDataLen];
            int i, j;
            for(i = 0; i < actPrecisionB; ++i)
                edgeVisibility[i] = kAcGiVisible;
            for(i = actPrecisionB; i < edgeDataLen; ++i)
                edgeVisibility[i] = kAcGiInvisible;//kAcGiSilhouette;
            int PrecisionA___B = PrecisionA__1 * actPrecisionB;
            for(i = 0; i < actPrecisionB; i += nStepB)
            {
                for(j = 0; j < actPrecisionA; ++j)
				{
					edgeVisibility[PrecisionA___B + i * actPrecisionA + j] = kAcGiVisible;
				}
            }
            edgeData.setVisibility(edgeVisibility);
			//added by szw 2009.11.18 : begin
			mode->subEntityTraits().setSelectionMarker(++gsIndex);
			//added by szw 2009.11.18 : end
            mode->geometry().mesh(PrecisionA__1, PrecisionB__1, pArray.asArrayPtr(), &edgeData);
            delete [] edgeVisibility;
		}
#ifndef _ALWAYSSHOWWIRE_
        break;
	case kAcGiStandardDisplay:
	case kAcGiSaveWorldDrawForR12:
	case kAcGiSaveWorldDrawForProxy:
#endif
		{
			int i, j, gsIndex = 0;
			mode->subEntityTraits().setSelectionMarker(++gsIndex);
			if(mode->geometry().circle(m_ptCenter, m_dlengthR, m_vect))
			{
				return Adesk::kTrue;
			}
			actPrecisionA = actPrecisionB = PRIMARY3D_WIREFRAME_LINENUM;

			getVertices(actPrecisionA,actPrecisionB,pArray);
			PrecisionA__1 = actPrecisionA + 1;
			PrecisionB__1 = actPrecisionB + 1;

			AcGePoint3dArray pointArray;
			pointArray.setLogicalLength(PrecisionA__1);
			//统一线框模型和实体模型的GS标记
			//modified by szw 2009.11.18 : begin
			++gsIndex;
			for(i = 0; i < actPrecisionB; i += nStepB)
			{
				for(j = 0; j <= actPrecisionA; ++j)
				{
					pointArray[j] = pArray[i + j * PrecisionB__1];
				}
				mode->subEntityTraits().setSelectionMarker(gsIndex);
				if(mode->geometry().polyline(PrecisionA__1, pointArray.asArrayPtr()))
				{
					return Adesk::kTrue;
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

/*Adesk::Boolean PDOval::worldDraw(AcGiWorldDraw* mode)
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
	AcGeIntArray stdBIdx;
	int actPrecisionA, actPrecisionB;
	getVertices(getMaxPrecisionA(mode), getMaxPrecisionB(mode), 
		pArray, stdBIdx, actPrecisionA, actPrecisionB);
	int PrecisionA__1 = actPrecisionA + 1;
	int PrecisionB__1 = actPrecisionB + 1;
	int stdBIdxLen = stdBIdx.length();

	switch(mode->regenType())
	{
	case kAcGiStandardDisplay:
	case kAcGiSaveWorldDrawForR12:
	case kAcGiSaveWorldDrawForProxy:
		{
			int i, j, gsIndex = 0;
			mode->subEntityTraits().setSelectionMarker(++gsIndex);
			mode->geometry().circle(m_ptCenter, m_dlengthR, m_vect);

			int stdBIdxLen_1 = stdBIdxLen - 1;
			for(i = 0; i < stdBIdxLen_1; ++i)
			{
				AcGePoint3dArray pointArray;
				for(j = 0; j <= actPrecisionA; ++j)
					pointArray.append(pArray[stdBIdx[i] + j * PrecisionB__1]);
				mode->subEntityTraits().setSelectionMarker(++gsIndex);
				mode->geometry().polyline(PrecisionA__1, pointArray.asArrayPtr());
			}
		}
		break;
	case kAcGiHideOrShadeCommand:
	case kAcGiRenderCommand:
		{
		    mode->subEntityTraits().setFillType(kAcGiFillAlways);
			mode->geometry().polygon(actPrecisionB, pArray.asArrayPtr());

			AcGiEdgeData edgeData;
			int edgeDataLen = actPrecisionA * PrecisionB__1 + PrecisionA__1 * actPrecisionB;
			Adesk::UInt8* edgeVisibility = new Adesk::UInt8[edgeDataLen];
			int i, j;
			for(i = 0; i < actPrecisionB; ++i)
				edgeVisibility[i] = kAcGiVisible;
			for(i = actPrecisionB; i < edgeDataLen; ++i)
				edgeVisibility[i] = kAcGiInvisible;//kAcGiSilhouette;
			int PrecisionA___B = PrecisionA__1 * actPrecisionB;
			for(i = 0; i < stdBIdxLen; ++i)
			{
				for(j = 0; j < actPrecisionA; ++j)
					edgeVisibility[PrecisionA___B + stdBIdx[i] * actPrecisionA + j] = kAcGiVisible;
			}
			edgeData.setVisibility(edgeVisibility);

			mode->geometry().mesh(PrecisionA__1, PrecisionB__1, pArray.asArrayPtr(), &edgeData);
			delete [] edgeVisibility;
		}
		break;
	default:
		break;
	}
#endif
	return Adesk::kTrue;
}*/


Acad::ErrorStatus PDOval::getVertices(int precisionA, int precisionB, 
                                      AcGePoint3dArray &vertexArray, 
                                      AcGeIntArray &stdBIdx, 
                                      int &actPrecisionA, int &actPrecisionB) const
{
    assertReadEnabled();

    AcGeDoubleArray dividDblB;
    getActDivid(precisionB, dividDblB, stdBIdx);

    int precisionA__1 = precisionA + 1;
    int precisionB__1 = dividDblB.length();
    actPrecisionA = precisionA__1 - 1;
    actPrecisionB = precisionB__1 - 1;
    vertexArray.setLogicalLength(precisionA__1 * precisionB__1);

	double hgt = getOvalHeight(); //hgt椭球封头的高度
    AcGePoint3d ellipCen = m_ptCenter - m_vect * (m_dlengthA - hgt);

    AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax;// Ay;
    if (fabs(m_vect[X]) <1.0/64 && fabs(m_vect[Y])<1.0/64) 
        Ax = Wy.crossProduct(m_vect);
    else
        Ax = Wz.crossProduct(m_vect);
    Ax.normalize();
    //Ay = vect.crossProduct(Ax);
    //Ay.normalize();

    AcGePoint3d downCirSp = m_ptCenter + Ax * m_dlengthR;
    AcGePoint3d upPt = m_ptCenter + m_vect * hgt;
    AcGeEllipArc3d ellip(ellipCen, m_vect, Ax, m_dlengthA, m_dlengthB);
    double ellipSParam, ellipEParam;
    ellipSParam = ellip.paramOf(downCirSp);
    ellipEParam = ellip.paramOf(upPt);
    if(upPt == ellip.startPoint())
    {
        if(fabs(ellip.paramOf(ellip.endPoint()) - ellipSParam) < fabs(ellip.paramOf(upPt) - ellipSParam))
            ellipEParam = ellip.paramOf(ellip.endPoint());
    }
    else if(downCirSp == ellip.startPoint())
    {
        if(fabs(ellip.paramOf(ellip.endPoint()) - ellipEParam) < fabs(ellip.paramOf(downCirSp) - ellipEParam))
            ellipSParam = ellip.paramOf(ellip.endPoint());
    }
    if(fabs(ellipEParam - ellipSParam) > PI)
    {
        if(fabs(ellipSParam - 2.0 * PI) < 1e-4)
            ellipSParam = 0;
        else if(fabs(ellipEParam - 2.0 * PI) < 1e-4)
            ellipEParam = 0;
    }

    double deltaParam = (ellipEParam - ellipSParam) / precisionA;
    double everyParam = ellipSParam;
    double deltaAng = 2.0 * PI / precisionB;
    int i, j, i___precisionB__1;
    for(i = 0; i < precisionA__1; ++i)
    {
        i___precisionB__1 = i * precisionB__1;
        vertexArray[i___precisionB__1] = ellip.evalPoint(everyParam);
        for(j = 1; j < precisionB__1; ++j)
        {
            AcGePoint3d tmpPt = vertexArray[i___precisionB__1];
            vertexArray[i___precisionB__1 + j] = tmpPt.rotateBy(deltaAng * dividDblB[j], m_vect, m_ptCenter);
        }
        everyParam += deltaParam;
    }

	return Acad::eOk;
}

Acad::ErrorStatus PDOval::getVertices(const int& precisionA, const AcGeVector3d& xAxis,
									  AcGePoint3dArray & vertexArray)const
{
	assertReadEnabled();

	int precisionA__1 = precisionA + 1;
	vertexArray.setLogicalLength(precisionA__1);

	double hgt = getOvalHeight(); //hgt椭球封头的高度
	AcGePoint3d ellipCen = m_ptCenter - m_vect * (m_dlengthA - hgt);

	AcGeVector3d Ax = xAxis;
	Ax.normalize();

	AcGePoint3d downCirSp = m_ptCenter + Ax * m_dlengthR;
	AcGePoint3d upPt = m_ptCenter + m_vect * hgt;
	AcGeEllipArc3d ellip(ellipCen, m_vect, Ax, m_dlengthA, m_dlengthB);
	double ellipSParam, ellipEParam;
	ellipSParam = ellip.paramOf(downCirSp);
	ellipEParam = ellip.paramOf(upPt);
	double tmpD = 0;
	if(upPt == ellip.startPoint())
	{
		tmpD = ellip.paramOf(ellip.endPoint());
		if(tmpD < ellipEParam)
			ellipEParam = tmpD;
	}
	else if(downCirSp == ellip.startPoint())
	{
		tmpD = ellip.paramOf(ellip.endPoint());
		if(tmpD < ellipSParam)
			ellipSParam = tmpD;
	}
	if(fabs(ellipEParam - ellipSParam) > PI)
	{
		if(fabs(ellipSParam - 2.0 * PI) < 1e-4)
			ellipSParam = 0;
		else if(fabs(ellipEParam - 2.0 * PI) < 1e-4)
			ellipEParam = 0;
	}

	double deltaParam = (ellipEParam - ellipSParam) / precisionA;
	double everyParam = ellipSParam;
	for(int i = 0; i < precisionA__1; ++i)
	{
		vertexArray[i] = ellip.evalPoint(everyParam);
		everyParam += deltaParam;
	}

	return Acad::eOk;
}

Acad::ErrorStatus PDOval::getVertices(int precisionA, int precisionB, 
									  AcGePoint3dArray &vertexArray) const
{
	assertReadEnabled();

	int precisionA__1 = precisionA + 1;
	int precisionB__1 = precisionB + 1;
	vertexArray.setLogicalLength(precisionA__1 * precisionB__1);

	double hgt = getOvalHeight(); //hgt椭球封头的高度
	AcGePoint3d ellipCen = m_ptCenter - m_vect * (m_dlengthA - hgt);

	AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax;// Ay;
	if (fabs(m_vect[X]) <1.0/64 && fabs(m_vect[Y])<1.0/64) 
		Ax = Wy.crossProduct(m_vect);
	else
		Ax = Wz.crossProduct(m_vect);
	Ax.normalize();
	//Ay = vect.crossProduct(Ax);
	//Ay.normalize();

	AcGePoint3d downCirSp = m_ptCenter + Ax * m_dlengthR;
	AcGePoint3d upPt = m_ptCenter + m_vect * hgt;
	AcGeEllipArc3d ellip(ellipCen, m_vect, Ax, m_dlengthA, m_dlengthB);
	double ellipSParam, ellipEParam;
	ellipSParam = ellip.paramOf(downCirSp);
	ellipEParam = ellip.paramOf(upPt);
	double tmpD = 0;
	if(upPt == ellip.startPoint())
	{
		tmpD = ellip.paramOf(ellip.endPoint());
		if(tmpD < ellipEParam)
			ellipEParam = tmpD;
	}
	else if(downCirSp == ellip.startPoint())
	{
		tmpD = ellip.paramOf(ellip.endPoint());
		if(tmpD < ellipSParam)
			ellipSParam = tmpD;
	}
	if(fabs(ellipEParam - ellipSParam) > PI)
	{
		if(fabs(ellipSParam - 2.0 * PI) < 1e-4)
			ellipSParam = 0;
		else if(fabs(ellipEParam - 2.0 * PI) < 1e-4)
			ellipEParam = 0;
	}

	double deltaParam = (ellipEParam - ellipSParam) / precisionA;
	double everyParam = ellipSParam;
	double deltaAng = 2.0 * PI / precisionB;
	int i, j, i___precisionB__1;
	AcGePoint3d tmpPt;
	for(i = 0; i < precisionA__1; ++i)
	{
		i___precisionB__1 = i * precisionB__1;
		vertexArray[i___precisionB__1] = ellip.evalPoint(everyParam);
		for(j = 1; j < precisionB__1; ++j)
		{
			tmpPt = vertexArray[i___precisionB__1];
			vertexArray[i___precisionB__1 + j] = tmpPt.rotateBy(deltaAng * j, m_vect, m_ptCenter);
		}
		everyParam += deltaParam;
	}

	return Acad::eOk;
}

Acad::ErrorStatus PDOval::dwgOutFields(AcDbDwgFiler* filer)const
{
    assertReadEnabled();
	Acad::ErrorStatus es=Acad::eOk;

	// Call dwgOutFields from father class: PDPrimary3D
	if((es=PDPrimary3D::dwgOutFields(filer))!=Acad::eOk)
	{
		return es;
	}

	// Write version number.
	filer->writeItem((Adesk::UInt16) VERSION_PDOVAL);

	// Write the data members.
	filer->writeItem(m_ptCenter);
	filer->writeItem(m_dlengthA);
	filer->writeItem(m_dlengthB);
	filer->writeItem(m_dlengthR);
	filer->writeItem(m_nprecisionA);
	filer->writeItem(m_nprecisionB);
	filer->writeItem(m_vect);

	return filer->filerStatus();
}

Acad::ErrorStatus PDOval::dwgInFields(AcDbDwgFiler* filer)
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
	if (version > VERSION_PDOVAL)
		return Acad::eMakeMeProxy;

	// Read the data members.
	switch (version)
	{
	case (1):

		filer->readItem(&m_ptCenter);
		filer->readItem(&m_dlengthA);
		filer->readItem(&m_dlengthB);
		filer->readItem(&m_dlengthR);
		filer->readItem(&m_nprecisionA);
		filer->readItem(&m_nprecisionB);
		filer->readItem(&m_vect);
    m_vect.normalize();
		break;
	}

#ifdef _USEAMODELER_
    if(m_3dGeom.isNull())
        createBody();
#endif

	return filer->filerStatus();
}


Acad::ErrorStatus PDOval::dxfOutFields(AcDbDxfFiler* filer) const
{
    assertReadEnabled();
    Acad::ErrorStatus es;

    if ((es = PDPrimary3D::dxfOutFields(filer))
        != Acad::eOk)
    {
        return es;
    }

	// Write subclass marker.
    filer->writeItem(AcDb::kDxfSubclass, _T("PDOval"));

	// Write version number.
	filer->writeItem(AcDb::kDxfInt16, (Adesk::UInt16) VERSION_PDOVAL);

	// Write data members.
    filer->writeItem(AcDb::kDxfXCoord, m_ptCenter);
	filer->writeItem(AcDb::kDxfReal,m_dlengthA);
	filer->writeItem(AcDb::kDxfReal+1,m_dlengthB);
	filer->writeItem(AcDb::kDxfReal+2,m_dlengthR);
	filer->writeItem(AcDb::kDxfInt32,m_nprecisionA);
	filer->writeItem(AcDb::kDxfInt32+1,m_nprecisionB);
	filer->writeItem(AcDb::kDxfNormalX, m_vect);

    return filer->filerStatus();
}

Acad::ErrorStatus PDOval::dxfInFields(AcDbDxfFiler* filer)
{
    assertWriteEnabled();
    Acad::ErrorStatus es = Acad::eOk;
    struct resbuf rb;

    if ((PDPrimary3D::dxfInFields(filer) != Acad::eOk)
        || !filer->atSubclassData(_T("PDOval")))
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
		if (version > VERSION_PDOVAL)
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
				m_dlengthA=rb.resval.rreal;
				break;

			 case AcDb::kDxfReal+1:
				m_dlengthB=rb.resval.rreal;
				break;

			 case AcDb::kDxfReal+2:
				m_dlengthR=rb.resval.rreal;
				break;

			 case AcDb::kDxfInt32:
				m_nprecisionA=rb.resval.rlong;
				break;

			 case AcDb::kDxfInt32+1:
				m_nprecisionB=rb.resval.rlong;
				break;

			 case AcDb::kDxfNormalX:
			    m_vect = asVec3d(rb.resval.rpoint);
          m_vect.normalize();
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

Acad::ErrorStatus PDOval::setParameters(double lengthA, double lengthB, 
                                double lengthR, const AcGePoint3d pt, const AcGeVector3d vect, 
                                Adesk::UInt32 precisionA/* = PRIMARY3D_DEFAULT_PRECISION*/, 
                                Adesk::UInt32 precisionB/* = PRIMARY3D_DEFAULT_PRECISION*/)
{
    assertWriteEnabled();

    m_dlengthA = lengthA;
    if(m_dlengthA == 0)
        m_dlengthA = 1;
    m_dlengthB = lengthB;
    if(m_dlengthB == 0)
        m_dlengthB = 1;
    /////////////////////////////////////////////////////
    //// m_dlengthR should not long than m_dlengthB	/////
    m_dlengthR = lengthR;
    if(m_dlengthR == 0)
        m_dlengthR = 1;
    if(m_dlengthR > m_dlengthB)
        m_dlengthR = m_dlengthB;

    m_ptCenter = pt;
    m_vect = vect.normal();

    if(precisionA > 2)
        m_nprecisionA = precisionA;
    else
        m_nprecisionA = PRIMARY3D_DEFAULT_PRECISION;
    if(precisionB > 2)
        m_nprecisionB = precisionB;
    else
        m_nprecisionB = PRIMARY3D_DEFAULT_PRECISION;

    if(m_nprecisionA > m_nprecisionB)
        m_dDividPrecision = m_nprecisionA;
    else
        m_dDividPrecision = m_nprecisionB;

#ifdef _USEAMODELER_
    createBody();
#endif
    return Acad::eOk;
}

AcGePoint3d PDOval::getpointCenter() const
{
	assertReadEnabled();
	return m_ptCenter;
}

double PDOval::getlengthA() const
{
	assertReadEnabled();
	return m_dlengthA;
}

double PDOval::getlengthB() const
{
	assertReadEnabled();
	return m_dlengthB;
}

double PDOval::getlengthR() const
{
	assertReadEnabled();
	return m_dlengthR;
}

Adesk::UInt32 PDOval::getprecisionA() const
{
	assertReadEnabled();
	return m_nprecisionA;
}

Adesk::UInt32 PDOval::getprecisionB() const
{
	assertReadEnabled();
	return m_nprecisionB;
}

AcGeVector3d PDOval::getVect() const
{
    assertReadEnabled();
	return m_vect;
}

// 取得椭球封头的高度
double PDOval::getOvalHeight() const
{
    assertReadEnabled();
    double kk = m_dlengthB * m_dlengthB - m_dlengthR * m_dlengthR;
    if(kk < 0)
        kk = 0;
    return m_dlengthA - (m_dlengthA / m_dlengthB * sqrt(kk));
}

// 取得椭圆的圆心
AcGePoint3d PDOval::getEllipseCen() const
{
    assertReadEnabled();
    return m_ptCenter - m_vect * (m_dlengthA - getOvalHeight());
}

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDOval::subGetOsnapPoints(
	AcDb::OsnapMode     osnapMode,
	Adesk::GsMarker     gsSelectionMark,
	const AcGePoint3d&  pickPoint,
	const AcGePoint3d&  lastPoint,
	const AcGeMatrix3d& viewXform,
	AcGePoint3dArray&   snapPoints,
	AcDbIntArray &   geomIds) const
#else
Acad::ErrorStatus PDOval::getOsnapPoints(
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

    AcGePoint3dArray pArray;
    AcGeIntArray stdBIdx;
    int actPrecisionA, actPrecisionB;
    getVertices(m_nprecisionA, m_nprecisionB, pArray, 
                stdBIdx, actPrecisionA, actPrecisionB);
    int PrecisionA__1 = actPrecisionA + 1;
    int PrecisionB__1 = actPrecisionB + 1;
    int stdBIdxLen = stdBIdx.length();
    int stdBIdxLen_1 = stdBIdxLen - 1;

    switch(osnapMode)
    {
    case AcDb::kOsModeEnd:
        {
		    snapPoints.append(m_ptCenter);
            snapPoints.append(pArray[pArray.length() - 1]);
            int PrecisionB___A = PrecisionB__1 * actPrecisionA;
            for(int i = 0; i < stdBIdxLen_1; i++)
            {
                snapPoints.append(pArray[stdBIdx[i]]);
                snapPoints.append(pArray[stdBIdx[i] + PrecisionB___A]);
            }
        }
        break;
    case AcDb::kOsModeMid:
        snapPoints.append(m_ptCenter + (pArray[pArray.length() - 1] - m_ptCenter) / 2.0);
        break;
    case AcDb::kOsModeCen:
        if(gsSelectionMark == 1)
		    snapPoints.append(m_ptCenter);
        else
            snapPoints.append(getEllipseCen());
		break;
	case AcDb::kOsModeQuad:
	    {
            AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax;// Ay;
            if (fabs(m_vect[X]) <1.0/64 && fabs(m_vect[Y])<1.0/64) 
                Ax = Wy.crossProduct(m_vect);
            else
                Ax = Wz.crossProduct(m_vect);
            Ax.normalize();
            //Ay = vect.crossProduct(Ax);
            //Ay.normalize();

            AcGePoint3d downCirSp = m_ptCenter + Ax * m_dlengthR;
            double deltaAng = PI / 2.0;
            snapPoints.append(downCirSp);
		    for(int i = 1; i < 4; ++i)
                snapPoints.append(downCirSp.rotateBy(deltaAng, m_vect, m_ptCenter));
	    }
        break;
    case AcDb::kOsModeNode:
		  break;
    case AcDb::kOsModeIns:
		  snapPoints.append(m_ptCenter);
		  break;
    case AcDb::kOsModePerp:
        {
            AcGePoint3d pt;
            if(gsSelectionMark == 1)
            {
                AcGeCircArc3d cir(m_ptCenter, m_vect, m_dlengthR);
                pt = cir.closestPointTo(lastPoint);
                snapPoints.append(pt);
            }
			//重新定义对象垂直正交点的捕捉方式，同时满足实体模型和线框模型的捕捉 
			//modified by szw 2009.11.18 : begin
            else
            {
                double hgt = getOvalHeight(); //hgt椭球封头的高度
                AcGePoint3d ellipCen = m_ptCenter - m_vect * (m_dlengthA - hgt);

                AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax;// Ay;
                if (fabs(m_vect[X]) <1.0/64 && fabs(m_vect[Y])<1.0/64) 
                    Ax = Wy.crossProduct(m_vect);
                else
                    Ax = Wz.crossProduct(m_vect);
                Ax.normalize();
                //Ay = vect.crossProduct(Ax);
                //Ay.normalize();

                AcGePoint3d downCirSp = m_ptCenter + Ax * m_dlengthR;
                AcGePoint3d upPt = m_ptCenter + m_vect * hgt;
                AcGeEllipArc3d ellip(ellipCen, m_vect, Ax, m_dlengthA, m_dlengthB);
                double ellipSParam, ellipEParam;
                ellipSParam = ellip.paramOf(downCirSp);
                ellipEParam = ellip.paramOf(upPt);
                if(upPt == ellip.startPoint())
                {
                    if(fabs(ellip.paramOf(ellip.endPoint()) - ellipSParam) < fabs(ellip.paramOf(upPt) - ellipSParam))
                    ellipEParam = ellip.paramOf(ellip.endPoint());
                }
                else if(downCirSp == ellip.startPoint())
                {
                    if(fabs(ellip.paramOf(ellip.endPoint()) - ellipEParam) < fabs(ellip.paramOf(downCirSp) - ellipEParam))
                    ellipSParam = ellip.paramOf(ellip.endPoint());
                }
                AcGeEllipArc3d ellipArc;
                if(ellipEParam > ellipSParam)
                    ellipArc = AcGeEllipArc3d(ellipCen, m_vect, Ax, m_dlengthA, m_dlengthB, ellipSParam, ellipEParam);
                else
                    ellipArc = AcGeEllipArc3d(ellipCen, m_vect, Ax, m_dlengthA, m_dlengthB, ellipEParam, ellipSParam);

				for(int i = 0; i < 8; ++i)
				{
 					ellipArc.rotateBy(i * PI / 4.0, m_vect, m_ptCenter);
					pt = ellipArc.closestPointTo(lastPoint);
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
            AcGePoint3d pt;
            if(gsSelectionMark == 1)
            {
                AcGeCircArc3d cir;
                //下底面
                cir.set(m_ptCenter, m_vect, m_dlengthR);
                pt = cir.projClosestPointTo(pickPoint, viewDir);
                snapPoints.append(pt);
            }
			//重新定义对象最近点的捕捉方式，同时满足实体模型和线框模型的捕捉 
			//modified by szw 2009.11.18 : begin
            else
            {
                double hgt = getOvalHeight(); //hgt椭球封头的高度
                AcGePoint3d ellipCen = m_ptCenter - m_vect * (m_dlengthA - hgt);

                AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax;// Ay;
                if (fabs(m_vect[X]) <1.0/64 && fabs(m_vect[Y])<1.0/64) 
                    Ax = Wy.crossProduct(m_vect);
                else
                    Ax = Wz.crossProduct(m_vect);
                Ax.normalize();
                //Ay = vect.crossProduct(Ax);
                //Ay.normalize();

                AcGePoint3d downCirSp = m_ptCenter + Ax * m_dlengthR;
                AcGePoint3d upPt = m_ptCenter + m_vect * hgt;
                AcGeEllipArc3d ellip(ellipCen, m_vect, Ax, m_dlengthA, m_dlengthB);
                double ellipSParam, ellipEParam;
                ellipSParam = ellip.paramOf(downCirSp);
                ellipEParam = ellip.paramOf(upPt);
                if(upPt == ellip.startPoint())
                {
                    if(fabs(ellip.paramOf(ellip.endPoint()) - ellipSParam) < fabs(ellip.paramOf(upPt) - ellipSParam))
                    ellipEParam = ellip.paramOf(ellip.endPoint());
                }
                else if(downCirSp == ellip.startPoint())
                {
                    if(fabs(ellip.paramOf(ellip.endPoint()) - ellipEParam) < fabs(ellip.paramOf(downCirSp) - ellipEParam))
                    ellipSParam = ellip.paramOf(ellip.endPoint());
                }
				for(int i = 0; i < 8; ++i)
				{
					ellip.rotateBy(i * PI / 4.0, m_vect, m_ptCenter);
					pt = ellip.projClosestPointTo(pickPoint, viewDir);
					double ptParam = ellip.paramOf(pt);
					if(ellipEParam > ellipSParam)
					{
						if(ptParam >= ellipSParam && ptParam <= ellipEParam)
							snapPoints.append(pt);
					}
					else
					{
						if(ptParam >= ellipEParam && ptParam <= ellipSParam)
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
Acad::ErrorStatus PDOval::subGetGripPoints(
	AcGePoint3dArray& gripPoints,
	AcDbIntArray& osnapModes,
	AcDbIntArray& geomIds) const
#else
Acad::ErrorStatus PDOval::getGripPoints(
	AcGePoint3dArray& gripPoints,
	AcDbIntArray& osnapModes,
	AcDbIntArray& geomIds) const
#endif
{
    assertReadEnabled();
/*	getgrippoints(gripPoints);*/
	return Acad::eOk; 
}



Acad::ErrorStatus PDOval::getgrippoints(AcGePoint3dArray& gripArray) const
{
    assertReadEnabled();
	gripArray.append(m_ptCenter);
	AcGeCubicSplineCurve3d cir = AcGeCubicSplineCurve3d(AcGeCircArc3d(m_ptCenter,m_vect,m_dlengthR));
	double parameter;
	AcGePoint3d pt;
	for(int i =0;i<4;i++){
		parameter = PI/2*i;
		pt = cir.evalPoint(parameter);
		gripArray.append(pt);
	}
	return Acad::eOk; 
}


#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDOval::subMoveGripPointsAt(
	const AcDbIntArray& indices,
	const AcGeVector3d& offset)
#else
Acad::ErrorStatus PDOval::moveGripPointsAt(
	const AcDbIntArray& indices,
	const AcGeVector3d& offset)
#endif
{
	assertWriteEnabled();

/*	if(indices[0] ==0 )
		m_ptCenter.transformBy(offset);

	else{
		AcGePoint3dArray grippoint;
		getgrippoints(grippoint);
		grippoint[indices[0]].transformBy(offset);

		///////////////////////////////////////////////
		/////	oval require that: m_dlengthR < m_dlengthB
		/////	so the function was canceled temprary
		m_dlengthR = grippoint[indices[0]].distanceTo(m_ptCenter);
	}
*/
	return Acad::eOk;
}

//suzhiyong's method
//Acad::ErrorStatus PDOval::explode(AcDbVoidPtrArray& entitySet) const
//{
//	assertReadEnabled();
//
//	//先做个椭圆，转换成面域，将面域延中线旋转180度成为椭球体。再对椭球体裁剪
//
//	double hgt = getOvalHeight(); //hgt椭球封头的高度
//	AcGePoint3d ellipCen = m_ptCenter - m_vect * (m_dlengthA - hgt);
//
//	AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax;// Ay;
//	if (fabs(m_vect[X]) <1.0/64 && fabs(m_vect[Y])<1.0/64) 
//		Ax = Wy.crossProduct(m_vect);
//	else
//		Ax = Wz.crossProduct(m_vect);
//	Ax.normalize();
//	//Ay = vect.crossProduct(Ax);
//	//Ay.normalize();
//
//	AcGePoint3d downCirSp = m_ptCenter + Ax * m_dlengthR;
//	AcGeVector3d unitNormal = Ax.crossProduct(m_vect).normal();
//	AcGePoint3d upPt = m_ptCenter + m_vect * hgt;
//	AcGeVector3d majorAxis;
//	double startAngle, endAngle, ratio;
//	if(m_dlengthA > m_dlengthB)
//	{
//		majorAxis = m_vect.normal() * m_dlengthA;
//		startAngle = 2.0 * PI - majorAxis.angleTo(downCirSp - ellipCen);
//		endAngle = 2.0 * PI;
//		ratio = m_dlengthB / m_dlengthA;
//	}
//	else
//	{
//		majorAxis = Ax * m_dlengthB;
//		startAngle = majorAxis.angleTo(downCirSp - ellipCen);
//		endAngle = PI / 2.0;
//		ratio = m_dlengthA / m_dlengthB;
//	}
//
//	AcDbEllipse elip(ellipCen, unitNormal, majorAxis, ratio, 0, PI);
//
//	AcDbVoidPtrArray curveSegments;
//	AcGePoint3d endPoint = upPt -m_vect*2*m_dlengthA;
//	AcDbLine pLine(upPt, endPoint);
//	curveSegments.append(&elip);
//	curveSegments.append(&pLine);
//
//	AcDbVoidPtrArray regions;
//	Acad::ErrorStatus es = Acad::eOk;
//	es = AcDbRegion::createFromCurves(curveSegments, regions);
//
//	if(es == Acad::eOk && !regions.isEmpty())
//	{
//		AcDb3dSolid *pBody;
//		pBody = new AcDb3dSolid;
//		AcGePoint3d ptCenter(0,0,0);
//		AcGeVector3d vect(0,1,0);
//		es = pBody->revolve((AcDbRegion*)(regions[0]), ptCenter, vect, 2*PI);//m_ptCenter, m_vect
//		if(es != Acad::eOk)
//		{
//			delete pBody;
//			for(int i = 0; i < regions.length(); i++)
//				delete (AcRxObject*)regions[i];
//			return Acad::eNotApplicable;
//		}
//
//		AcGePlane slicePlane;
//		AcDb3dSolid *pHalfSolid = NULL;
//		AcGeVector3d nor(0,1,0);
//		slicePlane.set(m_ptCenter, m_vect);
//		pBody->getSlice(slicePlane, Adesk::kFalse, pHalfSolid);
//
//		pBody->setPropertiesFrom(this);
//		entitySet.append(pBody);
//	}
//	else
//	{
//		for(int i = 0; i < regions.length(); i++)
//			delete (AcRxObject*)regions[i];
//		return Acad::eNotApplicable;
//	}
//
//	for(int i = 0; i < regions.length(); i++)
//		delete (AcRxObject*)regions[i];
//
//	return Acad::eOk;
//}

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDOval::subExplode(AcDbVoidPtrArray& entitySet) const
#else
Acad::ErrorStatus PDOval::explode(AcDbVoidPtrArray& entitySet) const
#endif
{
    assertReadEnabled();

    double hgt = getOvalHeight(); //hgt椭球封头的高度
    AcGePoint3d ellipCen = m_ptCenter - m_vect * (m_dlengthA - hgt);

	//如果椭球封头的长短轴相差不大，则用球封头近似
	if (fabs(m_dlengthA-m_dlengthB) <= SIGMA)
	{
		AcDb3dSolid *pBody;
		pBody = new AcDb3dSolid;
		if(pBody->createSphere(m_dlengthA) != Acad::eOk)
		{
			delete pBody;
			return Acad::eNotApplicable;
		}

		AcGeMatrix3d mat;
		mat.setTranslation(ellipCen.asVector());

		pBody->transformBy(mat);
		pBody->setPropertiesFrom(this);

		AcGePlane slicePlane;
		AcDb3dSolid *pHalfSolid = NULL;
		slicePlane.set(m_ptCenter, m_vect);
		pBody->getSlice(slicePlane, Adesk::kFalse, pHalfSolid);

		entitySet.append(pBody);
	} 
	else
	{
		AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax;// Ay;
		if (fabs(m_vect[X]) <1.0/64 && fabs(m_vect[Y])<1.0/64) 
			Ax = Wy.crossProduct(m_vect);
		else
			Ax = Wz.crossProduct(m_vect);
		Ax.normalize();
		//Ay = vect.crossProduct(Ax);
		//Ay.normalize();

		AcGePoint3d downCirSp = m_ptCenter + Ax * m_dlengthR;
		AcGeVector3d unitNormal = Ax.crossProduct(m_vect).normal();
		AcGePoint3d upPt = m_ptCenter + m_vect * hgt;
		AcGeVector3d majorAxis;
		double startAngle, endAngle, ratio;
		if(m_dlengthA > m_dlengthB)
		{
			majorAxis = m_vect.normal() * m_dlengthA;
			startAngle = 2.0 * PI - majorAxis.angleTo(downCirSp - ellipCen);
			endAngle = 2.0 * PI;
			ratio = m_dlengthB / m_dlengthA;
		}
		else
		{
			majorAxis = Ax * m_dlengthB;
			startAngle = majorAxis.angleTo(downCirSp - ellipCen);
			endAngle = PI / 2.0;
			ratio = m_dlengthA / m_dlengthB;
		}

		AcDbEllipse elip(ellipCen, unitNormal, majorAxis, ratio, startAngle, endAngle);

		AcDbLine lineV(upPt, m_ptCenter);
		lineV.setNormal(unitNormal);
		AcDbLine lineH(m_ptCenter, downCirSp);
		lineH.setNormal(unitNormal);
		AcDbVoidPtrArray curveSegments;
		curveSegments.append(&elip);
		curveSegments.append(&lineV);
		curveSegments.append(&lineH);
		AcDbVoidPtrArray regions;
		Acad::ErrorStatus es = Acad::eOk;
		es = AcDbRegion::createFromCurves(curveSegments, regions);

		if(es == Acad::eOk && !regions.isEmpty())
		{
			AcDb3dSolid *pBody;
			pBody = new AcDb3dSolid;
			es = pBody->revolve((AcDbRegion*)(regions[0]), m_ptCenter, m_vect, 7);
			if(es != Acad::eOk)
			{
				delete pBody;
				for(int i = 0; i < regions.length(); i++)
					delete (AcRxObject*)regions[i];
				return Acad::eNotApplicable;
			}
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
	}

	return Acad::eOk;
}

Acad::ErrorStatus PDOval::explodeTo3DSolid(AcDb3dSolid* &p3dSolid) const
{
	assertReadEnabled();
	//createBody();
	//TCHAR fn[]=_T("c:\\pdptempsat.sat");
	//m_3dGeom.saveToSat( fn);
	//clearBody();
	//AcDbBody *pb = new AcDbBody;
	//AcDbVoidPtrArray ids;
	//pb->acisIn( fn, ids);
	//delete pb;

	//if( ids.length()>0){
	//	p3dSolid = (AcDb3dSolid*)(ids[0]);
	//	return Acad::eOk;
	//}

	double hgt = getOvalHeight(); //hgt椭球封头的高度
	AcGePoint3d ellipCen = m_ptCenter - m_vect * (m_dlengthA - hgt);

	//如果椭球封头的长短轴相差不大，则用球封头近似
	if (fabs(m_dlengthA-m_dlengthB) <= SIGMA)
	{
		AcDb3dSolid *pBody;
		pBody = new AcDb3dSolid;
		if(pBody->createSphere(m_dlengthA) != Acad::eOk)
		{
			delete pBody;
			return Acad::eNotApplicable;
		}

		AcGeMatrix3d mat;
		mat.setTranslation(ellipCen.asVector());

		pBody->transformBy(mat);
		pBody->setPropertiesFrom(this);

		AcGePlane slicePlane;
		AcDb3dSolid *pHalfSolid = NULL;
		slicePlane.set(m_ptCenter, m_vect);
		pBody->getSlice(slicePlane, Adesk::kFalse, pHalfSolid);

		p3dSolid = pBody;
	} 
	else
	{
		AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax;// Ay;
		if (fabs(m_vect[X]) <1.0/64 && fabs(m_vect[Y])<1.0/64) 
			Ax = Wy.crossProduct(m_vect);
		else
			Ax = Wz.crossProduct(m_vect);
		Ax.normalize();
		//Ay = vect.crossProduct(Ax);
		//Ay.normalize();

		AcGePoint3d downCirSp = m_ptCenter + Ax * m_dlengthR;
		AcGeVector3d unitNormal = Ax.crossProduct(m_vect).normal();
		AcGePoint3d upPt = m_ptCenter + m_vect * hgt;
		AcGeVector3d majorAxis;
		double startAngle, endAngle, ratio;
		if(m_dlengthA > m_dlengthB)
		{
			majorAxis = m_vect.normal() * m_dlengthA;
			startAngle = 2.0 * PI - majorAxis.angleTo(downCirSp - ellipCen);
			endAngle = 2.0 * PI;
			ratio = m_dlengthB / m_dlengthA;
		}
		else
		{
			majorAxis = Ax * m_dlengthB;
			startAngle = majorAxis.angleTo(downCirSp - ellipCen);
			endAngle = PI / 2.0;
			ratio = m_dlengthA / m_dlengthB;
		}

		AcDbEllipse elip(ellipCen, unitNormal, majorAxis, ratio, startAngle, endAngle);

		AcDbLine lineV(upPt, m_ptCenter);
		lineV.setNormal(unitNormal);
		AcDbLine lineH(m_ptCenter, downCirSp);
		lineH.setNormal(unitNormal);
		AcDbLine lineA(upPt, downCirSp);
		lineH.setNormal(unitNormal);
		AcDbVoidPtrArray curveSegments;
		if( unitNormal.isPerpendicularTo( AcGeVector3d::kZAxis))
			curveSegments.append( &lineA);
		else
			curveSegments.append(&elip);
		curveSegments.append(&lineV);
		curveSegments.append(&lineH);
		AcDbVoidPtrArray regions;
		Acad::ErrorStatus es = Acad::eOk;
		es = AcDbRegion::createFromCurves(curveSegments, regions);

		if(es == Acad::eOk && !regions.isEmpty())
		{
			AcDb3dSolid *pBody;
			pBody = new AcDb3dSolid;
			es = pBody->revolve((AcDbRegion*)(regions[0]), m_ptCenter, m_vect, 7);

			if(es != Acad::eOk)
			{
				delete pBody;
				for(int i = 0; i < regions.length(); i++)
					delete (AcRxObject*)regions[i];
				return Acad::eNotApplicable;
			}

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
	}

	return Acad::eOk;
}

void PDOval::createBody()
{
    assertWriteEnabled();
#ifdef _OBJECTARX2004_
    AcGePoint3d* vertices;
    PolygonVertexData** vertexData;
	PolygonVertexData vd(PolygonVertexData::kUnspecifiedCurve);

    vertices = new AcGePoint3d[m_nprecisionA + 2];
    vertexData = new PolygonVertexData*[m_nprecisionA + 2];

    double hgt = getOvalHeight(); //hgt椭球封头的高度
    AcGePoint3d ellipCen = m_ptCenter - m_vect * (m_dlengthA - hgt);

    AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax;// Ay;
    if (fabs(m_vect[X]) <1.0/64 && fabs(m_vect[Y])<1.0/64) 
        Ax = Wy.crossProduct(m_vect);
    else
        Ax = Wz.crossProduct(m_vect);
    Ax.normalize();
    //Ay = vect.crossProduct(Ax);
    //Ay.normalize();

    AcGePoint3d downCirSp = m_ptCenter + Ax * m_dlengthR;
    AcGePoint3d upPt = m_ptCenter + m_vect * hgt;
    AcGeEllipArc3d ellip(ellipCen, m_vect, Ax, m_dlengthA, m_dlengthB);
    double ellipSParam, ellipEParam;
    ellipSParam = ellip.paramOf(downCirSp);
    ellipEParam = ellip.paramOf(upPt);
/*
	if(upPt == ellip.startPoint())
    {
        if(fabs(ellip.paramOf(ellip.endPoint()) - ellipSParam) < fabs(ellip.paramOf(upPt) - ellipSParam))
            ellipEParam = ellip.paramOf(ellip.endPoint());
    }
    else if(downCirSp == ellip.startPoint())
    {
        if(fabs(ellip.paramOf(ellip.endPoint()) - ellipEParam) < fabs(ellip.paramOf(downCirSp) - ellipEParam))
            ellipSParam = ellip.paramOf(ellip.endPoint());
    }

    bool upIsSp = true;
    if(upPt.distanceTo(ellip.evalPoint(ellipSParam)) <= upPt.distanceTo(ellip.evalPoint(ellipEParam)))
        upIsSp = true;
    else
        upIsSp = false;
*/
	if( fabs( ellipEParam - ellipSParam)>PI){
		if( ellipEParam > ellipSParam) ellipEParam -=2*PI;
		else ellipSParam -=2*PI;
	}

    double deltaParam = (ellipEParam - ellipSParam) / m_nprecisionA;
    vertices[0] = m_ptCenter;
    vertexData[0] = &vd;

        double everyParam = ellipSParam;
        for(Adesk::UInt32 i = 0; i <= m_nprecisionA; ++i)
        {
            vertices[i + 1] = ellip.evalPoint(everyParam);
            vertexData[i + 1] = &vd;
            everyParam += deltaParam;
        }

	/*  if(upIsSp)
    {
        double everyParam = ellipEParam;
        for(Adesk::UInt32 i = m_nprecisionA; i >= 0; --i)
        {
            vertices[m_nprecisionA - i + 1] = ellip.evalPoint(everyParam);
            vertexData[m_nprecisionA - i + 1] = NULL;
            everyParam -= deltaParam;
        }
    }
    else
    {
        double everyParam = ellipSParam;
        for(Adesk::UInt32 i = 0; i <= m_nprecisionA; ++i)
        {
            vertices[i + 1] = ellip.evalPoint(everyParam);
            vertexData[i + 1] = NULL;
            everyParam += deltaParam;
        }
    }
*/
    m_3dGeom = Body::endpointRevolution((Point3d*)vertices, vertexData, m_nprecisionA + 2, *(Vector3d*)&(ellip.normal()), 
                2 * PI, m_dDividPrecision);
    delete [] vertices;
    delete [] vertexData;
#endif
}

// 取得合适的剖分精度，如果实体较小，则剖分精度就会低，如果实体较大，则剖分
// 精度不大于实体限定的剖分精度m_dDividPrecision。
int PDOval::getMaxPrecision(AcGiWorldDraw *mode) const
{
    assertReadEnabled();
    if(!getCalMaxPrecision())
        return m_dDividPrecision;
    double radius = max(m_dlengthA, m_dlengthB);
    int pre = (int)(PI / acos(1.0 - mode->deviation(kAcGiMaxDevForCircle, m_ptCenter)
              / radius));
    if(pre > m_dDividPrecision)
        return m_dDividPrecision;
    else if(pre > 8)
        return pre;
    else
        return 8;
}

// 取得合适的剖分精度，如果实体较小，则剖分精度就会低，如果实体较大，则剖分
// 精度不大于实体限定的剖分精度m_nprecisionA、m_nprecisionB。
int PDOval::getMaxPrecisionA(AcGiWorldDraw *mode) const
{
    assertReadEnabled();
    if(!getCalMaxPrecision())
        return m_nprecisionA;
    int pre = (int)(PI / acos(1.0 - mode->deviation(kAcGiMaxDevForCircle, m_ptCenter)
              / m_dlengthA));
    if(pre > (int)m_nprecisionA)
        return m_nprecisionA;
    else if(pre > 8)
        return pre;
    else
        return 8;
}
int PDOval::getMaxPrecisionB(AcGiWorldDraw *mode) const
{
    assertReadEnabled();
    if(!getCalMaxPrecision())
        return m_nprecisionB;
    int pre = (int)(PI / acos(1.0 - mode->deviation(kAcGiMaxDevForCircle, m_ptCenter)
              / m_dlengthR));
    if(pre > (int)m_nprecisionB)
        return m_nprecisionB;
    else if(pre > 8)
        return pre;
    else
        return 8;
}

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDOval::subGetGeomExtents(AcDbExtents& extents) const
#else
Acad::ErrorStatus PDOval::getGeomExtents(AcDbExtents& extents) const
#endif
{
    assertReadEnabled();
    extents.set(m_ptCenter, m_ptCenter);
    AcGePoint3dArray pArray;
    AcGeIntArray stdBIdx;
    int actPrecisionA, actPrecisionB;
    getVertices(m_nprecisionA, m_nprecisionB, pArray, 
        stdBIdx, actPrecisionA, actPrecisionB);
    int i, arrlen = pArray.length();
    for(i = 0; i < arrlen; ++i)
        extents.addPoint(pArray[i]);
    return Acad::eOk;
}

#ifdef _OBJECTARX2010_
void PDOval::subList() const
#else
void PDOval::list() const
#endif
{
	assertReadEnabled();
#ifdef _OBJECTARX2010_
	PDPrimary3D::subList();
#else
	PDPrimary3D::list();
#endif
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4752/*"\n 椭球的长轴=%.3f 椭球的短轴=%.3f\n"*/,m_dlengthA,m_dlengthB);
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4753/*" 截口圆的圆心(%f,%f,%f)\n"*/,m_ptCenter.x,m_ptCenter.y,m_ptCenter.z);   
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4754/*" 截口圆的半径=%.3f\n"*/,m_dlengthR);
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4755/*" 圆心指向封头的截口面法向量(%f,%f,%f)\n"*/,m_vect.x,m_vect.y,m_vect.z);
    ads_printf(LANGUAGE_DEFINE_MACRO_V25_4756/*" 经线剖分精度: %d\n"*/,m_nprecisionA);
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4757/*" 纬线剖分精度: %d\n\n"*/,m_nprecisionB);
	return ;
}// added by linlin 20050810



/*---------------------------------------------------------------------------
* 名称: _make_eqovalpt
* 功能: 完成 "在设备椭球封头上画出点" 的功能
* 注意: 无
*/
int  PDOval:: _make_eqovalpt(double lengthR,AcGeVector3d vec,ads_point pt,ads_point normal) 
{
	if(lengthR > m_dlengthR ||-lengthR>getOvalHeight())
		return RTERROR;

	double z,c,H,ang;
	AcGePoint3d center1, point, F1,F2;
	AcGeVector3d L1,L2,L3,Lnormal;
	if(m_dlengthA >= m_dlengthB)
	{  
		c=sqrt(m_dlengthA*m_dlengthA-m_dlengthB*m_dlengthB);
		F1=getEllipseCen()+m_vect*c;
		F2=getEllipseCen()-m_vect*c;
	}
	else
	{
		c=sqrt(m_dlengthB*m_dlengthB-m_dlengthA*m_dlengthA);
		F1=getEllipseCen()+vec*c;
		F2=getEllipseCen()-vec*c;
	}

	if (lengthR>=0) 
	{
		z=m_dlengthA/m_dlengthB *sqrt( m_dlengthB*m_dlengthB-lengthR*lengthR);
		H=z-(m_dlengthA-getOvalHeight());
		center1=m_ptCenter+m_vect*H;
		point=center1+vec*lengthR;	
	}
	else
	{
	   center1=m_ptCenter+m_vect*(-lengthR);
	   H=(m_dlengthA-getOvalHeight()-lengthR)*m_dlengthB/m_dlengthA;
	   z=sqrt(m_dlengthB*m_dlengthB-H*H);
      point=center1+vec*z;
	}

	pt[0]=point.x;
	pt[1]=point.y;
	pt[2]=point.z;

	 L1=(point-F1).normalize();
	 L2=(point-F2).normalize();
	 ang=L1.angleTo(L2);
     L3=L1.crossProduct(L2);
	 Lnormal=L1;
	 Lnormal=Lnormal.rotateBy(ang/2,L3);
	 Lnormal.normalize();
 
	 normal[0]=Lnormal.x;
	 normal[1]=Lnormal.y;
	 normal[2]=Lnormal.z;

	return  RTNORM;
	
}//added by linlin 20051008