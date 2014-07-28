// PDSaddle.cpp: implementation of the PDSaddle class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "language25.h"
#include "PDSaddle.h"
#include "dbproxy.h"
#include "acgi.h"
#include "math.h"
#include "gecspl3d.h"
#include "geassign.h"
#include <dbents.h>
#include <dbregion.h>
#include <dbsol3d.h>
#include < geintarr.h >
#include "dbapserv.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define VERSION_PDSADDLE 2
#define PI 3.1415926535897932384626433832795L

#ifdef _OBJECTARX2000_
ACRX_DXF_DEFINE_MEMBERS(PDSaddle, PDPrimary3D,
						AcDb::kDHL_CURRENT, AcDb::kMReleaseCurrent,
						AcDbProxyEntity::kAllAllowedBits,
						PDSADDLE, Gle);
#else
ACRX_DXF_DEFINE_MEMBERS(PDSaddle, PDPrimary3D, AcDbProxyEntity::kAllAllowedBits, PDSADDLE, Gle);
#endif

void PDSaddle::setDefault()
{
	PDPrimary3D::setDefault();
#if defined(NDEBUG) || !defined(_DEBUG)
	m_dDividPrecision = PRIMARY3D_DEFAULT_PRECISION;
	m_dLength = m_dWidth = m_dHeight = m_dAngle = m_dRadius = 1.0;
	m_ptP = AcGePoint3d ();
	m_insX = AcGeVector3d(1, 0, 0);  //插入点的X方向
	m_insY = AcGeVector3d(0, 1, 0);  //插入点的Y方向
	m_insZ = AcGeVector3d(0, 0, 1);  //插入点的Z方向
#else
	m_dDividPrecision = PRIMARY3D_DEFAULT_PRECISION;
	m_dLength = m_dWidth = m_dHeight = m_dAngle = m_dRadius = 1000;
	m_ptP.set(0,0,0);
	m_insX = AcGeVector3d(1, 0, 0);  //插入点的X方向
	m_insY = AcGeVector3d(0, 1, 0);  //插入点的Y方向
	m_insZ = AcGeVector3d(0, 0, 1);  //插入点的Z方向
#endif
}
bool PDSaddle::isValidData(double &f)
{
	if(!PDPrimary3D::isValidData(f))
		return false;
	if(!isValidFloat(f=m_dLength))
		return false;
	if(!isValidFloat(f=m_dWidth))
		return false;
	if(!isValidFloat(f=m_dHeight))
		return false;
	if(!isValidFloat(f=m_dAngle))
		return false;
	if(!isValidFloat(f=m_dRadius))
		return false;
	if(!isValidFloat(f=m_ptP.x) || !isValidFloat(f=m_ptP.y) || !isValidFloat(f=m_ptP.z))
		return false;
	if(!isValidFloat(f=m_insX.x) || !isValidFloat(f=m_insX.y) || !isValidFloat(f=m_insX.z))
		return false;
	if(!isValidFloat(f=m_insY.x) || !isValidFloat(f=m_insY.y) || !isValidFloat(f=m_insY.z))
		return false;
	if(!isValidFloat(f=m_insZ.x) || !isValidFloat(f=m_insZ.y) || !isValidFloat(f=m_insZ.z))
		return false;
	return true;
}

//zxb,20090113,增加audit，暂时只check自身成员数据
Acad::ErrorStatus PDSaddle::audit(AcDbAuditInfo* pAuditInfo)
{
	return auditPDPrimary3D(this,pAuditInfo,_T("PDSaddle"));
}

PDSaddle::PDSaddle(bool HasSnap/* = false*/) : PDPrimary3D(HasSnap)
{
	m_dDividPrecision = PRIMARY3D_DEFAULT_PRECISION;
	m_dLength = m_dWidth = m_dHeight = m_dAngle = m_dRadius = 1.0;
	m_ptP = AcGePoint3d ();
	m_insX = AcGeVector3d(1, 0, 0);  //插入点的X方向
	m_insY = AcGeVector3d(0, 1, 0);  //插入点的Y方向
	m_insZ = AcGeVector3d(0, 0, 1);  //插入点的Z方向
}

PDSaddle::~PDSaddle()
{

}

PDSaddle::PDSaddle( double l,double w,double h,AcGePoint3d p,double angle,double radius,int precision, 
				   const AcGeVector3d &insX /*= AcGeVector3d(1, 0, 0)*/, const AcGeVector3d &insY /*= AcGeVector3d(0, 1, 0)*/, 
				   bool HasSnap/* = false*/,int CorrType/* = 1*/) : PDPrimary3D(HasSnap)
{
	m_dLength =fabs(l) ;
	if(m_dLength==0)
		m_dLength=1;
	m_dWidth =fabs(w) ;
	if(m_dWidth==0)
		m_dWidth=1;
	m_dHeight = fabs(h);
	if(m_dHeight==0)
		m_dHeight=1;
	if(angle>360)
		angle-=360;
	if(angle<-360)
		angle+=360;
	m_dAngle = angle / 180.0 * PI;
	m_dRadius =fabs(radius) ;
	if(m_dRadius==0)
		m_dRadius=1;

	m_ptP = p;
	if(precision > 2)
		m_dDividPrecision = precision;
	else
		m_dDividPrecision = PRIMARY3D_DEFAULT_PRECISION;
	m_insX = insX.normal();  //插入点的X方向
	m_insY = insY.normal();  //插入点的Y方向
	if(CorrType)
		m_insZ = insX.crossProduct(insY).normal();  //插入点的Z方向
	else
		m_insZ = insY.crossProduct(insX).normal();  //插入点的Z方向

#ifdef _USEAMODELER_
	createBody();
#endif

}//added by linlin 20050929

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDSaddle::subTransformBy(const AcGeMatrix3d& xform)
#else
Acad::ErrorStatus PDSaddle::transformBy(const AcGeMatrix3d& xform)
#endif
{
	assertWriteEnabled();
	// TODO: implement this function.
	m_ptP.transformBy(xform);
	m_insX.transformBy(xform);  //插入点的X方向
	m_insY.transformBy(xform);  //插入点的Y方向
	m_insZ.transformBy(xform);  //插入点的Z方向

	m_dHeight *= xform.scale();
	m_dWidth *= xform.scale();
	m_dLength *= xform.scale();
	m_dRadius *= xform.scale();

#ifdef _OBJECTARX2010_
	return PDPrimary3D::subTransformBy(xform);
#else
	return PDPrimary3D::transformBy(xform);
#endif
}

PDSaddle::SideFace PDSaddle::getSideFaceInfo()const
{
	SideFace result;
	result.originPt = m_ptP;

	AcGeVector3d vectAlongZ = m_insZ * m_dHeight; //!< 获取高度方向矢量
	result.vertexHeight = m_ptP + vectAlongZ;

	AcGeVector3d vectL, vectW;
	getlengthVwidthV(vectL, vectW);

	result.vertexWidth = m_ptP + vectW * m_dWidth;
	result.vertexRemain = result.vertexHeight + vectW * m_dWidth;

	if (m_dWidth > m_dRadius * 2.0)
	{
		double dist = (m_dWidth - m_dRadius * 2.0) / 2;
		result.nearArcPt = result.vertexHeight + vectW * dist;
		result.farArcPt = result.vertexHeight + vectW * (m_dWidth - dist);

		//!< 获取圆弧中点
		AcGePoint3d temp = result.vertexHeight + vectW * (dist + m_dRadius);
		result.midArcPt = temp - m_insZ * m_dRadius;
		result.flag = false;
	}
	else
	{
		result.nearArcPt = result.vertexHeight;
		result.farArcPt = result.vertexRemain;

		//!< 获取圆弧中点
		double half = m_dWidth / 2;
		AcGePoint3d temp = result.vertexHeight + vectW * half;
		result.midArcPt = temp - m_insZ * (m_dRadius - sqrt(m_dRadius*m_dRadius - half*half));

		result.flag = true;
	}

	return result;
}

#ifdef _OBJECTARX2010_
Adesk::Boolean PDSaddle::subWorldDraw(AcGiWorldDraw* mode)
#else
Adesk::Boolean PDSaddle::worldDraw(AcGiWorldDraw* mode)
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
	AcGeVector3d lengthvect,widthvect;
	getlengthVwidthV(lengthvect,widthvect);
	AcGeVector3d vectAlongLength = lengthvect * m_dLength;
	AcGeVector3d vectAlongWidth = widthvect * m_dWidth;
	AcGeVector3d vectAlongZ = m_insZ * m_dHeight;
	Adesk::Int16 nRating = getCalMaxPrecision();
	if(nRating == PRIMARY3D_SIMPLIZE_RATE1 ||
		nRating == PRIMARY3D_SIMPLIZE_RATE2)
	{
		double dMax = m_dLength;
		if(dMax < m_dWidth)
			dMax = m_dWidth;
		if(dMax < m_dRadius*2)
			dMax = m_dRadius*2;
		if(dMax < m_dHeight)
			dMax = m_dHeight;
		Precision = (int)(PI / acos(1.0 - mode->deviation(kAcGiMaxDevForCircle, m_ptP) * 2.0 /dMax));
		if(Precision < PRIMARY3D_SIMPLIZE_SCALE)
		{
			//需要简化
			//下底面上的四条线
			AcGePoint3d pt0 = m_ptP;
			AcGePoint3d pt1 = pt0 + vectAlongWidth;
			AcGePoint3d pt3 = pt0 + vectAlongLength;
			AcGePoint3d pt2 = pt1 + vectAlongLength;
			AcGePoint3d pts[5];
			pts[0] = pt0;
			pts[1] = pt1 ;
			pts[2] = pt2;
			pts[3] = pt3;
			pts[4] = pt0;
			if(mode->geometry().polyline(5, pts))
				return Adesk::kTrue;

			//上底面上的四条线
			AcGePoint3d pts1[5];
			pts1[0] = pts[0] + vectAlongZ;
			pts1[1] = pts[1] + vectAlongZ;
			pts1[2] = pts[2] + vectAlongZ;
			pts1[3] = pts[3] + vectAlongZ;
			pts1[4] = pts[4] + vectAlongZ;
			if(mode->geometry().polyline(5, pts1))
				return Adesk::kTrue;

			//四条竖向的线条
			AcGePoint3d pts2[2];
			pts2[0] = pts[0];
			pts2[1] = pts1[0];
			if(mode->geometry().polyline(2, pts2))
				return Adesk::kTrue;

			pts2[0] = pts[1];
			pts2[1] = pts1[1];
			if(mode->geometry().polyline(2, pts2))
				return Adesk::kTrue;

			pts2[0] = pts[2];
			pts2[1] = pts1[2];
			if(mode->geometry().polyline(2, pts2))
				return Adesk::kTrue;

			pts2[0] = pts[3];
			pts2[1] = pts1[3];
			mode->geometry().polyline(2, pts2);
			return Adesk::kTrue;
		}
	}
	int PreStep = 1; 
	AcGePoint3dArray pArray;
	int actPrecision__1 = Precision + 1;

	AcGePoint3d strArcPt = m_ptP + vectAlongZ;

	double dist = (m_dWidth - m_dRadius * 2.0) / 2.0;
#ifndef _ALWAYSSHOWWIRE_
	switch(mode->regenType())
	{
	case kAcGiHideOrShadeCommand:
	case kAcGiRenderCommand:
#endif
		{
			//////////////////////////////////////////////////////////////////////////
			//       33----------------------22
			//       ||                                ||
			//    3----------------------2   ||
			//    |  ||                           |   ||
			//    |  00----------------------11
			//    0----------------------1
			//////////////////////////////////////////////////////////////////////////
			int gsIndex = 0;
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
			AcGePoint3d pt0 = m_ptP;
			AcGePoint3d pt1 = pt0 + vectAlongLength;
			AcGePoint3d pt2 = pt1 + vectAlongWidth;
			AcGePoint3d pt3 = pt2 - vectAlongLength;
			AcGePoint3d pt00 = pt0 + vectAlongZ;
			AcGePoint3d pt11 = pt1 + vectAlongZ;
			AcGePoint3d pt22 = pt2 + vectAlongZ;
			AcGePoint3d pt33 = pt3 + vectAlongZ;


			//下底面
			pts[0] = pt0;
			pts[1] = pt1;
			pts[2] = pt2;
			pts[3] = pt3;
			//添加实体模型各面的GS标记
			//added by szw 2009.11.18 : begin
			mode->subEntityTraits().setSelectionMarker(++gsIndex);
			//added by szw 2009.11.18 : end
			if(mode->geometry().polygon(4, pts))
				return Adesk::kTrue;

			//四面
			//1.侧面
			pts[0] = pt0;
			pts[1] = pt1;
			pts[2] = pt11;
			pts[3] = pt00;
			//added by szw 2009.11.18 : begin
			mode->subEntityTraits().setSelectionMarker(++gsIndex);
			//added by szw 2009.11.18 : end
			if(mode->geometry().polygon(4, pts))
				return Adesk::kTrue;

			AcGePoint3dArray ptArray;

			ptArray.append(pt1);
			if(m_dWidth > m_dRadius * 2.0)
				ptArray.append(pt11);
			int i;
			for(i = actPrecision__1; i < 2.0*actPrecision__1; i++)
				ptArray.append(pArray[i]);
			if(m_dWidth > m_dRadius * 2.0)
				ptArray.append(pt22);
			ptArray.append(pt2);
			//added by szw 2009.11.18 : begin
			mode->subEntityTraits().setSelectionMarker(++gsIndex);
			//added by szw 2009.11.18 : end
			if(mode->geometry().polygon(ptArray.length(), ptArray.asArrayPtr()))
				return Adesk::kTrue;

			//3。侧面
			pts[0] = pt2;
			pts[1] = pt3;
			pts[2] = pt33;
			pts[3] = pt22;
			//added by szw 2009.11.18 : begin
			mode->subEntityTraits().setSelectionMarker(++gsIndex);
			//added by szw 2009.11.18 : end
			if(mode->geometry().polygon(4, pts))
				return Adesk::kTrue;

			ptArray.setLogicalLength(0);
			ptArray.append(pt3);
			ptArray.append(pt0);
			if(m_dWidth > m_dRadius * 2.0)
				ptArray.append(pt00);
			for(i = 0; i <actPrecision__1; i++)
				ptArray.append(pArray[i]);
			if(m_dWidth > m_dRadius * 2.0)
				ptArray.append(pt33);
			//added by szw 2009.11.18 : begin
			mode->subEntityTraits().setSelectionMarker(++gsIndex);
			//added by szw 2009.11.18 : end
			if(mode->geometry().polygon(ptArray.length(), ptArray.asArrayPtr()))
				return Adesk::kTrue;

			//顶面
			//added by szw 2009.11.18 : begin
			++gsIndex;
			//added by szw 2009.11.18 : end
			if(m_dWidth > m_dRadius * 2.0)//顶面上连接马鞍面的两段平面
			{
				pts[0] = pt00;
				pts[1] = pt11;
				pts[2] = pts[1] + widthvect * dist;
				pts[3] = pts[2] - vectAlongLength;
				//added by szw 2009.11.18 : begin
				mode->subEntityTraits().setSelectionMarker(gsIndex);
				//added by szw 2009.11.18 : end
				if(mode->geometry().polygon(4, pts))
					return Adesk::kTrue;

				pts[0] = pt33;
				pts[1] = pt22;
				pts[2] = pts[1] - widthvect * dist;
				pts[3] = pts[2] - vectAlongLength;
				//added by szw 2009.11.18 : begin
				mode->subEntityTraits().setSelectionMarker(gsIndex);
				//added by szw 2009.11.18 : end
				if(mode->geometry().polygon(4, pts))
					return Adesk::kTrue;
			}
			//////////////////////////////////////////////////////////////////
			//////	用mesh函数构造马鞍面								//////
			AcGiEdgeData edgeData;
			int edgeDataLen =Precision + 1 + 2 * Precision;
			Adesk::UInt8* edgeVisibility = new Adesk::UInt8[edgeDataLen];

			for(i = 0; i < edgeDataLen; ++i)
				edgeVisibility[i] = kAcGiInvisible;//kAcGiSilhouette;
			int actPrecision___2 = Precision * 2;
			for(i = 0; i < Precision; i += PreStep)
				edgeVisibility[i + actPrecision___2] = kAcGiVisible;
			edgeData.setVisibility(edgeVisibility);
			//added by szw 2009.11.18 : begin
			mode->subEntityTraits().setSelectionMarker(gsIndex);
			//added by szw 2009.11.18 : end
			mode->geometry().mesh(2, actPrecision__1, pArray.asArrayPtr(), &edgeData);
			delete [] edgeVisibility;
		}
#ifndef _ALWAYSSHOWWIRE_
		break;
	case kAcGiStandardDisplay:
	case kAcGiSaveWorldDrawForR12:
	case kAcGiSaveWorldDrawForProxy:
#endif
		{
			/////////////////////////////////////////////////////////////////
			AcGePoint3d pts[2];
			int gsIndex = 0;
			//统一线框模型和实体模型的GS标记
			//modified by szw 2009.11.18 : begin
			//下底面上的四条线
			pts[0] = m_ptP;
			pts[1] = m_ptP + vectAlongLength;
			++gsIndex;
				mode->subEntityTraits().setSelectionMarker(gsIndex);
			if(mode->geometry().polyline(2, pts))
				return Adesk::kTrue;

			pts[0] = pts[1];
			pts[1] = pts[0] + vectAlongWidth;
			mode->subEntityTraits().setSelectionMarker(gsIndex);
			if(mode->geometry().polyline(2, pts))
				return Adesk::kTrue;

			pts[0] = pts[1];
			pts[1] = pts[0] - vectAlongLength;
			mode->subEntityTraits().setSelectionMarker(gsIndex);
			if(mode->geometry().polyline(2, pts))
				return Adesk::kTrue;

			pts[0] = pts[1];
			pts[1] = m_ptP;
			mode->subEntityTraits().setSelectionMarker(gsIndex);
			if(mode->geometry().polyline(2, pts))
				return Adesk::kTrue;

			//四条竖向的线条
			pts[0] = m_ptP;
			pts[1] = pts[0] + vectAlongZ;
			mode->subEntityTraits().setSelectionMarker(++gsIndex);
			if(mode->geometry().polyline(2, pts))
				return Adesk::kTrue;

			pts[0] = pts[0] + vectAlongLength;
			pts[1] = pts[0] + vectAlongZ;
			mode->subEntityTraits().setSelectionMarker(++gsIndex);
			if(mode->geometry().polyline(2, pts))
				return Adesk::kTrue;

			pts[0] = pts[0] + vectAlongWidth;
			pts[1] = pts[0] + vectAlongZ;
			mode->subEntityTraits().setSelectionMarker(++gsIndex);
			if(mode->geometry().polyline(2, pts))
				return Adesk::kTrue;

			pts[0] = pts[0] - vectAlongLength;
			pts[1] = pts[0] + vectAlongZ;
			mode->subEntityTraits().setSelectionMarker(++gsIndex);
			if(mode->geometry().polyline(2, pts))
				return Adesk::kTrue;

			Precision = PRIMARY3D_WIREFRAME_LINENUM;
			getVertices(Precision, pArray);
			actPrecision__1 = Precision + 1;

			//////////////////////////////////////////////////////////////////////
			/////////////		two arc of the upper face			//////////////
			++gsIndex;		 
			if(m_dWidth <= m_dRadius * 2.0)
			{
				double arcangle = 2.0 * asin( m_dWidth / 2.0 / m_dRadius);  // 弧的角度 
				AcGePoint3d arccenterpoint = strArcPt + (vectAlongWidth / 2.0) + ((-m_insZ) * m_dRadius * (1 - cos(arcangle / 2.0)));
				mode->subEntityTraits().setSelectionMarker(++gsIndex);
				AcGePoint3d endPt = strArcPt+vectAlongWidth;
				if(mode->geometry().circularArc(strArcPt,arccenterpoint,endPt))
					return Adesk::kTrue;

				arccenterpoint += vectAlongLength;
				strArcPt += vectAlongLength;
				endPt += vectAlongLength;
				mode->subEntityTraits().setSelectionMarker(gsIndex);
				if(mode->geometry().circularArc(strArcPt,arccenterpoint,endPt))
					return Adesk::kTrue;
			}
			else
			{
				AcGePoint3d arccenterpoint = strArcPt + (vectAlongWidth / 2.0) + (-m_insZ) * m_dRadius;
				mode->subEntityTraits().setSelectionMarker(gsIndex);
				strArcPt += widthvect * dist;
				AcGePoint3d endPt = strArcPt+widthvect * m_dRadius * 2.0;
				if(mode->geometry().circularArc(strArcPt, arccenterpoint, endPt))
					return Adesk::kTrue;

				arccenterpoint += vectAlongLength;
				strArcPt += vectAlongLength;
				endPt += vectAlongLength;
				mode->subEntityTraits().setSelectionMarker(gsIndex);
				if(mode->geometry().circularArc(strArcPt, arccenterpoint, endPt))
					return Adesk::kTrue;
			}
			for(int i = 0; i < actPrecision__1; i += PreStep)
			{
				pts[0] = pArray[i];
				pts[1] = pArray[i + actPrecision__1];
				mode->subEntityTraits().setSelectionMarker(gsIndex);
				if(mode->geometry().polyline(2, pts))
					return Adesk::kTrue;
			}

			if(m_dWidth > m_dRadius * 2.0)
			{
				//上底面上的线
				pts[0] = m_ptP + vectAlongZ;
				pts[1] = pts[0] + vectAlongLength;
				mode->subEntityTraits().setSelectionMarker(gsIndex);
				if(mode->geometry().polyline(2, pts))
					return Adesk::kTrue;
				vectAlongWidth = widthvect * dist;

				pts[0] = pts[1];
				pts[1] = pts[0] + vectAlongWidth;
				mode->subEntityTraits().setSelectionMarker(gsIndex);
				if(mode->geometry().polyline(2, pts))
					return Adesk::kTrue;

				pts[0] = pts[1] + widthvect * m_dRadius * 2.0;
				pts[1] = pts[0] + vectAlongWidth;
				mode->subEntityTraits().setSelectionMarker(gsIndex);
				if(mode->geometry().polyline(2, pts))
					return Adesk::kTrue;

				pts[0] = pts[1];
				pts[1] = pts[0] - vectAlongLength;
				mode->subEntityTraits().setSelectionMarker(gsIndex);
				if(mode->geometry().polyline(2, pts))
					return Adesk::kTrue;

				pts[0] = pts[1];
				pts[1] = pts[0] -vectAlongWidth;
				mode->subEntityTraits().setSelectionMarker(gsIndex);
				if(mode->geometry().polyline(2, pts))
					return Adesk::kTrue;

				pts[0] = pts[1] - widthvect * m_dRadius * 2.0;
				pts[1] = m_ptP + vectAlongZ;
				mode->subEntityTraits().setSelectionMarker(gsIndex);
				if(mode->geometry().polyline(2, pts))
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
}//added by linlin 20050817



/*Adesk::Boolean PDSaddle::worldDraw(AcGiWorldDraw* mode)
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

AcGePoint3d strArcPt = m_ptP + m_insZ * m_dHeight;
AcGeVector3d lengthvect,widthvect;
getlengthVwidthV(lengthvect,widthvect);

double dist = (m_dWidth - m_dRadius * 2.0) / 2.0;

switch(mode->regenType())
{
case kAcGiStandardDisplay:
case kAcGiSaveWorldDrawForR12:
case kAcGiSaveWorldDrawForProxy:
{
/////////////////////////////////////////////////////////////////
AcGePoint3d pts[2];
int gsIndex = 0;

//下底面上的四条线
pts[0] = m_ptP;
pts[1] = m_ptP + lengthvect * m_dLength;
mode->subEntityTraits().setSelectionMarker(++gsIndex);
mode->geometry().polyline(2, pts);

pts[0] = pts[1];
pts[1] = pts[0] + widthvect * m_dWidth;
mode->subEntityTraits().setSelectionMarker(++gsIndex);
mode->geometry().polyline(2, pts);

pts[0] = pts[1];
pts[1] = pts[0] - lengthvect * m_dLength;
mode->subEntityTraits().setSelectionMarker(++gsIndex);
mode->geometry().polyline(2, pts);

pts[0] = pts[1];
pts[1] = m_ptP;
mode->subEntityTraits().setSelectionMarker(++gsIndex);
mode->geometry().polyline(2, pts);

//四条竖向的线条
pts[0] = m_ptP;
pts[1] = pts[0] + m_insZ * m_dHeight;
mode->subEntityTraits().setSelectionMarker(++gsIndex);
mode->geometry().polyline(2, pts);

pts[0] = pts[0] + lengthvect * m_dLength;
pts[1] = pts[0] + m_insZ * m_dHeight;
mode->subEntityTraits().setSelectionMarker(++gsIndex);
mode->geometry().polyline(2, pts);

pts[0] = pts[0] + widthvect * m_dWidth;
pts[1] = pts[0] + m_insZ * m_dHeight;
mode->subEntityTraits().setSelectionMarker(++gsIndex);
mode->geometry().polyline(2, pts);

pts[0] = pts[0] - lengthvect * m_dLength;
pts[1] = pts[0] + m_insZ * m_dHeight;
mode->subEntityTraits().setSelectionMarker(++gsIndex);
mode->geometry().polyline(2, pts);

//////////////////////////////////////////////////////////////////////
/////////////		two arc of the upper face			//////////////
if(m_dWidth <= m_dRadius * 2.0)
{
double arcangle = 2.0 * asin( m_dWidth / 2.0 / m_dRadius);  // 弧的角度 
AcGePoint3d arccenterpoint = strArcPt + (widthvect * m_dWidth / 2.0) + ((-m_insZ) * m_dRadius * (1 - cos(arcangle / 2.0)));
mode->subEntityTraits().setSelectionMarker(++gsIndex);
mode->geometry().circularArc(strArcPt,arccenterpoint,strArcPt+widthvect*m_dWidth);

arccenterpoint += lengthvect*m_dLength;
mode->subEntityTraits().setSelectionMarker(++gsIndex);
mode->geometry().circularArc(strArcPt+lengthvect*m_dLength,arccenterpoint,strArcPt+widthvect*m_dWidth+lengthvect*m_dLength);
}
else
{
double arcangle = PI;  // 弧的角度 
AcGePoint3d arccenterpoint = strArcPt + (widthvect * m_dWidth / 2.0) + (-m_insZ) * m_dRadius;
mode->subEntityTraits().setSelectionMarker(++gsIndex);
mode->geometry().circularArc(strArcPt + widthvect * dist, arccenterpoint, strArcPt + widthvect * dist + widthvect * m_dRadius * 2.0);

arccenterpoint += lengthvect * m_dLength;
mode->subEntityTraits().setSelectionMarker(++gsIndex);
mode->geometry().circularArc(strArcPt + widthvect * dist + lengthvect * m_dLength,
arccenterpoint, strArcPt + widthvect * dist + widthvect * m_dRadius * 2.0 + lengthvect * m_dLength);
}
for(int i = 0; i < stdIdxLen; i++)
{
pts[0] = pArray[stdIdx[i]];
pts[1] = pArray[stdIdx[i] + actPrecision__1];
mode->subEntityTraits().setSelectionMarker(++gsIndex);
mode->geometry().polyline(2, pts);
}

if(m_dWidth > m_dRadius * 2.0)
{
//上底面上的线
pts[0] = m_ptP + m_insZ * m_dHeight;
pts[1] = pts[0] + lengthvect * m_dLength;
mode->subEntityTraits().setSelectionMarker(++gsIndex);
mode->geometry().polyline(2, pts);

pts[0] = pts[1];
pts[1] = pts[0] + widthvect * dist;
mode->subEntityTraits().setSelectionMarker(++gsIndex);
mode->geometry().polyline(2, pts);

pts[0] = pts[1] + widthvect * m_dRadius * 2.0;
pts[1] = pts[0] + widthvect * dist;
mode->subEntityTraits().setSelectionMarker(++gsIndex);
mode->geometry().polyline(2, pts);

pts[0] = pts[1];
pts[1] = pts[0] - lengthvect * m_dLength;
mode->subEntityTraits().setSelectionMarker(++gsIndex);
mode->geometry().polyline(2, pts);

pts[0] = pts[1];
pts[1] = pts[0] - widthvect * dist;
mode->subEntityTraits().setSelectionMarker(++gsIndex);
mode->geometry().polyline(2, pts);

pts[0] = pts[1] - widthvect * m_dRadius * 2.0;
pts[1] = m_ptP + m_insZ * m_dHeight;
mode->subEntityTraits().setSelectionMarker(++gsIndex);
mode->geometry().polyline(2, pts);
}
}
break;
case kAcGiHideOrShadeCommand:
case kAcGiRenderCommand:
{
mode->subEntityTraits().setFillType(kAcGiFillAlways);
AcGePoint3d pts[4];
AcGePoint3dArray ptArray;

//下底面
pts[0] = m_ptP;
pts[1] = m_ptP + lengthvect * m_dLength;
pts[2] = pts[1] + widthvect * m_dWidth;
pts[3] = pts[2] - lengthvect * m_dLength;
mode->geometry().polygon(4, pts);

//四面
pts[0] = m_ptP;
pts[1] = m_ptP + lengthvect * m_dLength;
pts[2] = pts[1] + m_insZ * m_dHeight;
pts[3] = pts[0] + m_insZ * m_dHeight;
mode->geometry().polygon(4, pts);

while(!ptArray.isEmpty())
ptArray.removeLast();
ptArray.append(m_ptP + lengthvect * m_dLength);
if(m_dWidth > m_dRadius * 2.0)
ptArray.append(m_ptP + lengthvect * m_dLength + m_insZ * m_dHeight);
for(int i = actPrecision__1; i < 2.0*actPrecision__1; i++)
ptArray.append(pArray[i]);
if(m_dWidth > m_dRadius * 2.0)
ptArray.append(m_ptP + lengthvect * m_dLength + widthvect * m_dWidth + m_insZ * m_dHeight);
ptArray.append(m_ptP + lengthvect * m_dLength + widthvect * m_dWidth);
mode->geometry().polygon(ptArray.length(), ptArray.asArrayPtr());

pts[0] = m_ptP + lengthvect * m_dLength + widthvect * m_dWidth;
pts[1] = m_ptP + widthvect * m_dWidth;
pts[2] = pts[1] + m_insZ * m_dHeight;
pts[3] = pts[0] + m_insZ * m_dHeight;
mode->geometry().polygon(4, pts);

while(!ptArray.isEmpty())
ptArray.removeLast();
ptArray.append(m_ptP + widthvect * m_dWidth);
ptArray.append(m_ptP);
if(m_dWidth > m_dRadius * 2.0)
ptArray.append(m_ptP + m_insZ * m_dHeight);
for(i = 0; i <actPrecision__1; i++)
ptArray.append(pArray[i]);
if(m_dWidth > m_dRadius * 2.0)
ptArray.append(m_ptP + widthvect * m_dWidth + m_insZ * m_dHeight);
mode->geometry().polygon(ptArray.length(), ptArray.asArrayPtr());

//顶面
if(m_dWidth > m_dRadius * 2.0)
{
pts[0] = m_ptP + m_insZ * m_dHeight;
pts[1] = pts[0] + lengthvect * m_dLength;
pts[2] = pts[1] + widthvect * dist;
pts[3] = pts[2] - lengthvect * m_dLength;
mode->geometry().polygon(4, pts);

pts[0] = m_ptP + widthvect * m_dWidth + m_insZ * m_dHeight;
pts[1] = pts[0] + lengthvect * m_dLength;
pts[2] = pts[1] - widthvect * dist;
pts[3] = pts[2] - lengthvect * m_dLength;
mode->geometry().polygon(4, pts);
}

//////////////////////////////////////////////////////////////////
//////	用mesh函数构造马鞍面								//////
AcGiEdgeData edgeData;
int edgeDataLen =actPrecision + 1 + 2 * actPrecision;
Adesk::UInt8* edgeVisibility = new Adesk::UInt8[edgeDataLen];

for(i = 0; i < edgeDataLen; ++i)
edgeVisibility[i] = kAcGiInvisible;//kAcGiSilhouette;
int actPrecision___2 = actPrecision * 2;
for(i = 0; i < stdIdxLen; ++i)
edgeVisibility[stdIdx[i] + actPrecision___2] = kAcGiVisible;
edgeData.setVisibility(edgeVisibility);
mode->geometry().mesh(2, actPrecision__1, pArray.asArrayPtr(), &edgeData);
delete [] edgeVisibility;
}
break;
default:
break;
}
#endif
return Adesk::kTrue;
}//added by linlin 20050817

*/
Acad::ErrorStatus PDSaddle::getVertices(int Precision, AcGePoint3dArray& vertexArray, 
										AcGeIntArray &stdIdx, int &actPrecision) const
{
	/////////////////////////////////////////////////////////////////////////
	//////	数组2*m_nPrecision个点表示马鞍曲面上的点				/////////
	/////////////////////////////////////////////////////////////////////////
	assertReadEnabled();

	while(!vertexArray.isEmpty())
		vertexArray.removeLast();

	AcGeDoubleArray dividDbl;
	getActDivid(Precision, dividDbl, stdIdx);

	int actPrecision__1 = dividDbl.length();
	actPrecision = actPrecision__1 - 1;

	double arcangle;
	AcGePoint3d strArcPt;
	AcGeVector3d lengthvect,widthvect;
	getlengthVwidthV(lengthvect,widthvect);

	AcGePoint3d arcCenPt;
	if(m_dWidth <= m_dRadius * 2.0)
	{
		arcangle = 2.0 * asin(m_dWidth / 2.0 / m_dRadius);  // 弧的角度 
		strArcPt = m_ptP + m_insZ * m_dHeight;
		arcCenPt = strArcPt + (widthvect * m_dWidth / 2.0) + (m_insZ * m_dRadius * cos(arcangle / 2.0));
	}
	else
	{
		arcangle = PI;  // 弧的角度 
		double dist = (m_dWidth - m_dRadius * 2.0) / 2.0;
		strArcPt = m_ptP + m_insZ * m_dHeight;
		strArcPt += widthvect * dist;
		arcCenPt = strArcPt + widthvect * m_dRadius;
	}

	AcGePoint3dArray vertexArray1;
	AcGePoint3d point = strArcPt;
	AcGePoint3d point1;
	double deltaangle = arcangle /Precision;

	vertexArray.append(point);
	point1 = point + lengthvect * m_dLength;
	vertexArray1.append(point1);
	AcGePoint3d VecTemp;
	AcGeVector3d z;
	double ang;
	z = m_insX.crossProduct(m_insY).normal();
	ang = z.angleTo(m_insZ, m_insX);
	for(int j = 1; j <actPrecision__1; j++)
	{
		VecTemp=point;		
		if(ang > PI / 2.0 && ang < PI * 3.0 / 2.0)
			VecTemp.rotateBy(deltaangle* dividDbl[j], -lengthvect, arcCenPt);
		else
			VecTemp.rotateBy(deltaangle*dividDbl[j],lengthvect,arcCenPt);

		vertexArray.append(VecTemp);

		point1 = VecTemp + lengthvect * m_dLength;
		vertexArray1.append(point1);
	}
	vertexArray.append(vertexArray1);

	return Acad::eOk;
}//added by linlin 20050817


Acad::ErrorStatus PDSaddle::getVertices(int actPrecision, AcGePoint3dArray& vertexArray) const
{
	/////////////////////////////////////////////////////////////////////////
	//////	数组2*m_nPrecision个点表示马鞍曲面上的点				/////////
	/////////////////////////////////////////////////////////////////////////
	assertReadEnabled();

	/*while(!vertexArray.isEmpty())
	vertexArray.removeLast();*/

	int actPrecision__1 = actPrecision + 1;
	vertexArray.setLogicalLength(actPrecision__1 * 2);

	double arcangle;
	AcGePoint3d strArcPt;
	AcGeVector3d lengthvect,widthvect;
	getlengthVwidthV(lengthvect,widthvect);

	AcGePoint3d arcCenPt;
	if(m_dWidth <= m_dRadius * 2.0)
	{
		arcangle = 2.0 * asin(m_dWidth / 2.0 / m_dRadius);  // 弧的角度 
		strArcPt = m_ptP + m_insZ * m_dHeight;
		arcCenPt = strArcPt + (widthvect * m_dWidth / 2.0) + (m_insZ * m_dRadius * cos(arcangle / 2.0));
	}
	else
	{
		arcangle = PI;  // 弧的角度 
		double dist = (m_dWidth - m_dRadius * 2.0) / 2.0;
		strArcPt = m_ptP + m_insZ * m_dHeight;
		strArcPt += widthvect * dist;
		arcCenPt = strArcPt + widthvect * m_dRadius;
	}

	AcGePoint3dArray vertexArray1;
	AcGePoint3d point = strArcPt;
	AcGePoint3d point1;
	double deltaangle = arcangle /actPrecision;

	vertexArray[0] = point;
	AcGeVector3d tmpVectAlongLength = lengthvect * m_dLength;
	point1 = point + tmpVectAlongLength;
	vertexArray[actPrecision__1] = point1;
	AcGeVector3d z;
	double ang;
	z = m_insX.crossProduct(m_insY).normal();
	ang = z.angleTo(m_insZ, m_insX);
	for(int j = 1; j <actPrecision__1; j++)
	{
		if(ang > PI / 2.0 && ang < PI * 3.0 / 2.0)
			point.rotateBy(deltaangle, -lengthvect, arcCenPt);
		else
			point.rotateBy(deltaangle,lengthvect,arcCenPt);

		vertexArray[j] =point;
		point1 = point + tmpVectAlongLength;
		vertexArray[j + actPrecision__1] = point1;
	}
	return Acad::eOk;
}//added by linlin 20050817

Acad::ErrorStatus PDSaddle::dwgOutFields(AcDbDwgFiler* filer)const
{
	assertReadEnabled();
	Acad::ErrorStatus es=Acad::eOk;

	// Call dwgOutFields from father class: PDPrimary3D
	if((es=PDPrimary3D::dwgOutFields(filer))!=Acad::eOk)
	{
		return es;
	}

	// Write version number.
	filer->writeItem((Adesk::UInt16) VERSION_PDSADDLE);

	//version 2
	filer->writeItem(m_insX);
	filer->writeItem(m_insY);
	filer->writeItem(m_insZ);

	// Write the data members.
	filer->writeItem(m_ptP);
	filer->writeItem(m_dLength);
	filer->writeItem(m_dWidth);
	filer->writeItem(m_dHeight);
	filer->writeItem(m_dRadius);
	filer->writeItem(m_dAngle);
	filer->writeItem(m_nPrecision);

	return filer->filerStatus();
}

Acad::ErrorStatus PDSaddle::dwgInFields(AcDbDwgFiler* filer)
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
	if (version > VERSION_PDSADDLE)
		return Acad::eMakeMeProxy;

	// Read the data members.
	if(version >= 2)
	{
		filer->readItem(&m_insX);
		filer->readItem(&m_insY);
		filer->readItem(&m_insZ);

	}
	if(version >= 1)
	{
		filer->readItem(&m_ptP);
		filer->readItem(&m_dLength);
		filer->readItem(&m_dWidth);
		filer->readItem(&m_dHeight);
		filer->readItem(&m_dRadius);
		filer->readItem(&m_dAngle);
		filer->readItem(&m_nPrecision);

	}

#ifdef _USEAMODELER_
	if(m_3dGeom.isNull())
		createBody();
#endif

	return filer->filerStatus();
}


Acad::ErrorStatus PDSaddle::dxfOutFields(AcDbDxfFiler* filer) const
{
	assertReadEnabled();
	Acad::ErrorStatus es;

	if ((es = PDPrimary3D::dxfOutFields(filer))
		!= Acad::eOk)
	{
		return es;
	}

	// Write subclass marker.
	filer->writeItem(AcDb::kDxfSubclass, _T("PDSaddle"));

	// Write version number.
	filer->writeItem(AcDb::kDxfInt16, (Adesk::UInt16) VERSION_PDSADDLE);

	//version 2
	filer->writeItem(AcDb::kDxfXCoord, m_insX);
	filer->writeItem(AcDb::kDxfXCoord, m_insY);
	filer->writeItem(AcDb::kDxfXCoord, m_insZ);

	// Write data members.
	filer->writeItem(AcDb::kDxfXCoord, m_ptP);
	filer->writeItem(AcDb::kDxfReal, m_dLength);
	filer->writeItem(AcDb::kDxfReal+1, m_dWidth);
	filer->writeItem(AcDb::kDxfReal+2, m_dHeight);
	filer->writeItem(AcDb::kDxfReal+3, m_dRadius);
	filer->writeItem(AcDb::kDxfReal+4, m_dAngle);
	filer->writeItem(AcDb::kDxfInt32,m_nPrecision);


	return filer->filerStatus();
}

Acad::ErrorStatus PDSaddle::dxfInFields(AcDbDxfFiler* filer)
{
	assertWriteEnabled();
	Acad::ErrorStatus es = Acad::eOk;
	struct resbuf rb;

	if ((PDPrimary3D::dxfInFields(filer) != Acad::eOk)
		|| !filer->atSubclassData(_T("PDSaddle")))
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
		if (version > VERSION_PDSADDLE)
			return Acad::eMakeMeProxy;
	}

	// Read the data members.

	if(version >= 2)
	{
		filer->readItem(&rb);                                  //插入点的X方向
		if (rb.restype == AcDb::kDxfXCoord)
		{
			m_insX.set(rb.resval.rpoint[0], rb.resval.rpoint[1], rb.resval.rpoint[2]);
		}
		else
		{
			filer->pushBackItem();
			filer->setError(Acad::eInvalidDxfCode,
				_T("\nError: expected group code %d"),
				AcDb::kDxfXCoord);
			return filer->filerStatus();
		}
		filer->readItem(&rb);                                  //插入点的Y方向
		if (rb.restype == AcDb::kDxfXCoord)
		{
			m_insY.set(rb.resval.rpoint[0], rb.resval.rpoint[1], rb.resval.rpoint[2]);
		}
		else
		{
			filer->pushBackItem();
			filer->setError(Acad::eInvalidDxfCode,
				_T("\nError: expected group code %d"),
				AcDb::kDxfXCoord);
			return filer->filerStatus();
		}
		filer->readItem(&rb);                                  //插入点的Z方向
		if (rb.restype == AcDb::kDxfXCoord)
		{
			m_insZ.set(rb.resval.rpoint[0], rb.resval.rpoint[1], rb.resval.rpoint[2]);
		}
		else
		{
			filer->pushBackItem();
			filer->setError(Acad::eInvalidDxfCode,
				_T("\nError: expected group code %d"),
				AcDb::kDxfXCoord);
			return filer->filerStatus();
		}


	}

	if(version >= 1)
	{
		while ((es == Acad::eOk) && ((es = filer->readResBuf(&rb)) == Acad::eOk)){

			switch (rb.restype){
	case AcDb::kDxfXCoord:
		m_ptP = asPnt3d(rb.resval.rpoint);
		break;

	case AcDb::kDxfReal:
		m_dLength = rb.resval.rreal;
		break;

	case AcDb::kDxfReal+1:
		m_dWidth = rb.resval.rreal;
		break;

	case AcDb::kDxfReal+2:
		m_dHeight = rb.resval.rreal;
		break;

	case AcDb::kDxfReal+3:
		m_dRadius = rb.resval.rreal;
		break;

	case AcDb::kDxfReal+4:
		m_dAngle = rb.resval.rreal;
		break;

	case AcDb::kDxfInt32:
		m_nPrecision=rb.resval.rlong;
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
	//  if (es != Acad::eEndOfFile)
	//    return Acad::eInvalidResBuf;

	return filer->filerStatus();
}

Acad::ErrorStatus PDSaddle::getpointP(AcGePoint3d& ptP)
{
	assertReadEnabled();
	ptP = m_ptP;
	return Acad::eOk;
}


Acad::ErrorStatus PDSaddle::setParameters(AcGePoint3d ptP,double angle,double length,
										  double height,double width,double radius,
										  const AcGeVector3d& insX, const AcGeVector3d& insY,	
										  Adesk::UInt32 precision,
										  int CorrType/* = 1*/)
{
	assertWriteEnabled();
	m_ptP = ptP;

	if(angle>360)
		angle-=360;
	if(angle<-360)
		angle+=360;	 
	m_dAngle = angle / 180.0 * PI;	
	m_dLength =fabs(length) ;
	if(m_dLength==0)
		m_dLength=1;
	m_dHeight =fabs(height) ;
	if(m_dHeight==0)
		m_dHeight=1;
	m_dWidth =fabs(width) ;
	if(m_dWidth==0)
		m_dWidth=1;
	m_dRadius =fabs(radius) ;
	if(m_dRadius==0)
		m_dRadius=1;
	if(precision > 2)
		m_dDividPrecision = precision;
	else
		m_dDividPrecision = PRIMARY3D_DEFAULT_PRECISION;
	m_insX = insX.normal();  //插入点的X方向
	m_insY = insY.normal();  //插入点的Y方向
	AcGeVector3d vecTemp;
	vecTemp = m_insX; 
	if(CorrType)
		m_insZ = insX.crossProduct(insY).normal();  //插入点的Z方向
	else
		m_insZ = insY.crossProduct(insX).normal();  //插入点的Z方向
#ifdef _USEAMODELER_
	createBody();
#endif
	return Acad::eOk;
}//added by linlin 20050929

Acad::ErrorStatus PDSaddle:: getlengthVwidthV(AcGeVector3d &lengthvect,AcGeVector3d &widthvect) const
{
	assertReadEnabled();
	AcGeVector3d vecTemp;
	vecTemp= m_insX;
	AcGeVector3d z;
	double ang;
	z = m_insX.crossProduct(m_insY).normal();
	ang = z.angleTo(m_insZ, m_insX);
	if(ang > PI / 2.0 && ang < PI * 3.0 / 2.0)
	{
		lengthvect = vecTemp.rotateBy(m_dAngle, -m_insZ).normal();
		vecTemp = m_insY;
		//widthvect = lengthvect.crossProduct(m_insZ).normal();	
		widthvect = vecTemp.rotateBy(m_dAngle, -m_insZ).normal();
	}
	else
	{
		lengthvect = vecTemp.rotateBy(m_dAngle, m_insZ).normal();
		vecTemp = m_insY;
		widthvect = vecTemp.rotateBy(m_dAngle, m_insZ).normal();
		//widthvect = m_insZ.crossProduct(lengthvect).normal();
	}
	return Acad::eOk;
}//added by linlin 20050909


Acad::ErrorStatus PDSaddle::getRightHandCoord(AcGePoint3d &ori, 
											  AcGeVector3d &vecx, 
											  AcGeVector3d &vecy, 
											  AcGeVector3d &vecz,
											  AcGeVector3d &vlength,
											  AcGeVector3d &vwidth,
											  double &length, 
											  double &width, 
											  double &height) const
{
	assertReadEnabled();
	ori = m_ptP;
	vecx = m_insX;
	vecy = m_insY;
	vecz = m_insZ;
	length = m_dLength;
	width = m_dWidth;
	height = m_dHeight;	
	AcGeVector3d z,vecTemp;
	double ang;
	vecTemp=m_insX;
	z = m_insX.crossProduct(m_insY).normal();
	ang = z.angleTo(m_insZ, m_insX);
	vlength = vecTemp.rotateBy(m_dAngle, m_insZ).normal();
	vwidth= m_insZ.crossProduct(vlength).normal();
	if(ang > PI / 2.0 && ang < PI * 3.0 / 2.0)
	{
		vlength = vecTemp.rotateBy(m_dAngle, -m_insZ).normal();
		vecTemp = m_insY;
		//widthvect = lengthvect.crossProduct(m_insZ).normal();	
		vwidth = vecTemp.rotateBy(m_dAngle, -m_insZ).normal();
	}
	return Acad::eOk;
}//added by linlin 20050831

Acad::ErrorStatus PDSaddle::getangle(double& angle )
{
	assertReadEnabled();
	angle = m_dAngle / PI * 180;
	return Acad::eOk;
}

Acad::ErrorStatus PDSaddle::getheight(double& height)
{
	assertReadEnabled();
	height = m_dHeight;
	return Acad::eOk;
}

Acad::ErrorStatus PDSaddle::getlength(double& length)
{
	assertReadEnabled();
	length = m_dLength;
	return Acad::eOk;
}

Acad::ErrorStatus PDSaddle::getradius(double& radius)
{
	assertReadEnabled();
	radius = m_dRadius;
	return Acad::eOk;
}

Acad::ErrorStatus PDSaddle::getwidth(double& width)
{
	assertReadEnabled();
	width = m_dWidth;
	return Acad::eOk;
}

Acad::ErrorStatus PDSaddle::getprecision(Adesk::UInt32& precision)
{
	assertReadEnabled();
	precision = m_dDividPrecision;
	return Acad::eOk;
}

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDSaddle::subGetOsnapPoints(
	AcDb::OsnapMode     osnapMode,
	Adesk::GsMarker     gsSelectionMark,
	const AcGePoint3d&  pickPoint,
	const AcGePoint3d&  lastPoint,
	const AcGeMatrix3d& viewXform,
	AcGePoint3dArray&   snapPoints,
	AcDbIntArray &   geomIds) const
#else
Acad::ErrorStatus PDSaddle::getOsnapPoints(
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
	AcGeIntArray stdIdx;
	int actPrecision;
	AcGeVector3d lengthvect,widthvect;
	getVertices(m_dDividPrecision, pArray, stdIdx, actPrecision);
	getlengthVwidthV(lengthvect,widthvect) ;
	int actPrecision__1 = actPrecision + 1;
	int stdIdxLen = stdIdx.length();

	AcGePoint3d UpP = m_ptP + m_insZ * m_dHeight;

	double dist = (m_dWidth - m_dRadius * 2.0) / 2.0;

	double arcangle;
	AcGePoint3d strArcPt, arcCenPt;
	if(m_dWidth <= m_dRadius * 2.0)
	{
		arcangle = 2.0 * asin(m_dWidth / 2.0 / m_dRadius);  // 弧的角度 
		strArcPt = m_ptP + m_insZ * m_dHeight;
		arcCenPt = strArcPt + (widthvect * m_dWidth / 2.0) + (m_insZ * m_dRadius * cos(arcangle / 2.0));
	}
	else
	{
		arcangle = PI;  // 弧的角度 
		double dist = (m_dWidth - m_dRadius * 2.0) / 2.0;
		strArcPt = m_ptP + m_insZ * m_dHeight;
		strArcPt += widthvect * dist;
		arcCenPt = strArcPt + widthvect * m_dRadius;
	}

	switch(osnapMode) 
	{
	case AcDb::kOsModeEnd:
		{
			snapPoints.append(m_ptP);
			snapPoints.append(m_ptP + lengthvect * m_dLength);
			snapPoints.append(m_ptP + lengthvect * m_dLength + widthvect * m_dWidth);
			snapPoints.append(m_ptP + widthvect * m_dWidth);
			if(m_dWidth > m_dRadius * 2.0)
			{
				snapPoints.append(UpP);
				snapPoints.append(UpP + lengthvect * m_dLength);
				snapPoints.append(UpP + lengthvect * m_dLength + widthvect * m_dWidth);
				snapPoints.append(UpP + widthvect * m_dWidth);
			}
			for(int i = 0; i < stdIdxLen; i++)
			{
				snapPoints.append(pArray[stdIdx[i]]);
				snapPoints.append(pArray[stdIdx[i]+actPrecision__1]);
			}
		}
		break;
	case AcDb::kOsModeMid:
		{
			snapPoints.append(m_ptP + lengthvect * m_dLength / 2.0);
			snapPoints.append(m_ptP + lengthvect * m_dLength + widthvect * m_dWidth / 2.0);
			snapPoints.append(m_ptP + lengthvect * m_dLength / 2.0 + widthvect * m_dWidth);
			snapPoints.append(m_ptP + widthvect * m_dWidth / 2.0);
			snapPoints.append(m_ptP + m_insZ * m_dHeight / 2.0);
			snapPoints.append(m_ptP + lengthvect * m_dLength + m_insZ * m_dHeight / 2.0);
			snapPoints.append(m_ptP + lengthvect * m_dLength + widthvect * m_dWidth + m_insZ * m_dHeight / 2.0);
			snapPoints.append(m_ptP + widthvect * m_dWidth + m_insZ * m_dHeight / 2.0);
			if(m_dWidth > m_dRadius * 2.0)
			{
				snapPoints.append(UpP + lengthvect * m_dLength / 2.0);
				snapPoints.append(UpP + lengthvect * m_dLength + widthvect * dist / 2.0);
				snapPoints.append(UpP + widthvect * dist / 2.0);
				snapPoints.append(UpP + lengthvect * m_dLength + widthvect * (m_dWidth - dist / 2.0));
				snapPoints.append(UpP + lengthvect * m_dLength / 2.0 + widthvect * m_dWidth);
				snapPoints.append(UpP + widthvect * (m_dWidth - dist / 2.0));
			}
			for(int i = 0; i < stdIdxLen; i++)
				snapPoints.append(pArray[stdIdx[i]] + (pArray[stdIdx[i] + actPrecision__1] - pArray[stdIdx[i]]) / 2.0);
			snapPoints.append(pArray[stdIdx[4]]);
			snapPoints.append(pArray[stdIdx[4]+actPrecision__1]);
		}
		break;
	case AcDb::kOsModeCen:
		{
			//重新定义对象中心点的捕捉方式，同时满足实体模型和线框模型的捕捉 
			//modified by szw 2009.11.18 : begin
			if(gsSelectionMark_int <= 5)
			{
				snapPoints.append(m_ptP + lengthvect * m_dLength / 2.0 + widthvect * m_dWidth / 2.0);
				snapPoints.append(m_ptP + lengthvect * m_dLength / 2.0 + m_insZ * m_dHeight / 2.0);
				snapPoints.append(m_ptP + lengthvect * m_dLength + widthvect * m_dWidth / 2.0 + m_insZ * m_dHeight / 2.0);
				snapPoints.append(m_ptP + lengthvect * m_dLength / 2.0 + widthvect * m_dWidth + m_insZ * m_dHeight / 2.0);
				snapPoints.append(m_ptP + widthvect * m_dWidth / 2.0 + m_insZ * m_dHeight / 2.0);
			}
			else
			{
				snapPoints.append(arcCenPt);
				snapPoints.append(arcCenPt + lengthvect * m_dLength);
			}
			//modified by szw 2009.11.18 : end
		}
		break;
	case AcDb::kOsModeQuad:
		snapPoints.append(pArray[stdIdx[4]]);
		snapPoints.append(pArray[stdIdx[4]+actPrecision__1]);
		if(m_dWidth > m_dRadius * 2.0)
		{
			snapPoints.append(pArray[stdIdx[0]]);
			snapPoints.append(pArray[stdIdx[stdIdxLen-1]]);
			snapPoints.append(pArray[stdIdx[0]+actPrecision__1]);
			snapPoints.append(pArray[stdIdx[stdIdxLen-1]+actPrecision__1]);
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
			AcGeLine3d line;
			AcGeCircArc3d cir;
			//重新定义对象垂直正交点的捕捉方式，同时满足实体模型和线框模型的捕捉 
			//modified by szw 2009.11.18 : begin
			if(gsSelectionMark_int == 1)
			{
				line.set(m_ptP, lengthvect);
				pt = line.closestPointTo(lastPoint);
				snapPoints.append(pt);
				
				line.set(m_ptP + lengthvect * m_dLength, widthvect);
				pt = line.closestPointTo(lastPoint);
				snapPoints.append(pt);
				
				line.set(m_ptP + widthvect * m_dWidth, lengthvect);
				pt = line.closestPointTo(lastPoint);
				snapPoints.append(pt);

				line.set(m_ptP, widthvect);
				pt = line.closestPointTo(lastPoint);
				snapPoints.append(pt);
			}
			else if(gsSelectionMark_int >= 2 && gsSelectionMark_int <= 5)
			{
				line.set(m_ptP, m_insZ);
				pt = line.closestPointTo(lastPoint);
				snapPoints.append(pt);			
					
				line.set(m_ptP + lengthvect * m_dLength, m_insZ);
				pt = line.closestPointTo(lastPoint);
				snapPoints.append(pt);

				line.set(m_ptP + lengthvect * m_dLength + widthvect * m_dWidth, m_insZ);
				pt = line.closestPointTo(lastPoint);
				snapPoints.append(pt);

				line.set(m_ptP + widthvect * m_dWidth, m_insZ);
				pt = line.closestPointTo(lastPoint);
				snapPoints.append(pt);
			}
			else
			{
				cir.set(pArray[stdIdx[0]], pArray[stdIdx[4]], pArray[stdIdx[8]]);
				pt = cir.closestPointTo(lastPoint);
				snapPoints.append(pt);

				cir.set(pArray[stdIdx[0]+actPrecision__1], pArray[stdIdx[4]+actPrecision__1], pArray[stdIdx[8]+actPrecision__1]);
				pt = cir.closestPointTo(lastPoint);
				snapPoints.append(pt);

				for(int i = 0; i < stdIdxLen; ++i)
				{
					line.set(pArray[stdIdx[i]], lengthvect);
					pt = line.closestPointTo(lastPoint);
					snapPoints.append(pt);
				}

				line.set(UpP, lengthvect);
				pt = line.closestPointTo(lastPoint);
				snapPoints.append(pt);

				line.set(UpP + lengthvect * m_dLength, widthvect);
				pt = line.closestPointTo(lastPoint);
				snapPoints.append(pt);

				line.set(UpP + widthvect * m_dWidth, lengthvect);
				pt = line.closestPointTo(lastPoint);
				snapPoints.append(pt);


				line.set(UpP, widthvect);
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
			AcGePoint3d pt;
			AcGeLineSeg3d lineSeg;
			AcGeCircArc3d cir;
			//重新定义对象最近点的捕捉方式，同时满足实体模型和线框模型的捕捉 
			//modified by szw 2009.11.18 : begin
			if(gsSelectionMark_int == 1)
			{
				lineSeg.set(m_ptP, m_ptP + lengthvect * m_dLength);
				pt = lineSeg.projClosestPointTo(pickPoint, viewDir);
				snapPoints.append(pt);

				lineSeg.set(m_ptP + lengthvect * m_dLength, m_ptP + lengthvect * m_dLength + widthvect * m_dWidth);
				pt = lineSeg.projClosestPointTo(pickPoint, viewDir);
				snapPoints.append(pt);

				lineSeg.set(m_ptP + lengthvect * m_dLength + widthvect * m_dWidth, m_ptP + widthvect * m_dWidth);
				pt = lineSeg.projClosestPointTo(pickPoint, viewDir);
				snapPoints.append(pt);

				lineSeg.set(m_ptP, m_ptP + widthvect * m_dWidth);
				pt = lineSeg.projClosestPointTo(pickPoint, viewDir);
				snapPoints.append(pt);
			}
			else if(gsSelectionMark_int >= 2 && gsSelectionMark_int <= 5)
			{
				lineSeg.set(m_ptP, m_ptP + m_insZ * m_dHeight);
				pt = lineSeg.projClosestPointTo(pickPoint, viewDir);
				snapPoints.append(pt);

				lineSeg.set(m_ptP + lengthvect * m_dLength, m_ptP + lengthvect * m_dLength + m_insZ * m_dHeight);
				pt = lineSeg.projClosestPointTo(pickPoint, viewDir);
				snapPoints.append(pt);

				lineSeg.set(m_ptP + lengthvect * m_dLength + widthvect * m_dWidth, m_ptP + lengthvect * m_dLength + widthvect * m_dWidth + m_insZ * m_dHeight);
				pt = lineSeg.projClosestPointTo(pickPoint, viewDir);
				snapPoints.append(pt);

				lineSeg.set(m_ptP + widthvect * m_dWidth, m_ptP + widthvect * m_dWidth + m_insZ * m_dHeight);
				pt = lineSeg.projClosestPointTo(pickPoint, viewDir);
				snapPoints.append(pt);
			}
			else
			{
				cir.set(pArray[stdIdx[0]], pArray[stdIdx[4]], pArray[stdIdx[8]]);
				pt = cir.projClosestPointTo(pickPoint, viewDir);
				snapPoints.append(pt);

				cir.set(pArray[stdIdx[0]+actPrecision__1], pArray[stdIdx[4]+actPrecision__1], pArray[stdIdx[8]+actPrecision__1]);
				pt = cir.projClosestPointTo(pickPoint, viewDir);
				snapPoints.append(pt);

				for(int i = 0; i < stdIdxLen; ++i)
				{
					lineSeg.set(pArray[stdIdx[i]], pArray[stdIdx[i]+ actPrecision__1]);
					pt = lineSeg.projClosestPointTo(pickPoint, viewDir);
					snapPoints.append(pt);
				}

				lineSeg.set(UpP, UpP + lengthvect * m_dLength);
				pt = lineSeg.projClosestPointTo(pickPoint, viewDir);
				snapPoints.append(pt);

				lineSeg.set(UpP + lengthvect * m_dLength, UpP + lengthvect * m_dLength + widthvect * dist);
				pt = lineSeg.projClosestPointTo(pickPoint, viewDir);
				snapPoints.append(pt);

				lineSeg.set(UpP + lengthvect * m_dLength + widthvect * (dist + m_dRadius * 2.0), UpP + lengthvect * m_dLength + widthvect * m_dWidth);
				pt = lineSeg.projClosestPointTo(pickPoint, viewDir);
				snapPoints.append(pt);

				lineSeg.set(UpP + lengthvect * m_dLength + widthvect * m_dWidth, UpP + widthvect * m_dWidth);
				pt = lineSeg.projClosestPointTo(pickPoint, viewDir);
				snapPoints.append(pt);

				lineSeg.set(UpP + widthvect * m_dWidth, UpP + widthvect * (m_dWidth - dist));
				pt = lineSeg.projClosestPointTo(pickPoint, viewDir);
				snapPoints.append(pt);

				lineSeg.set(UpP, UpP + widthvect * dist);
				pt = lineSeg.projClosestPointTo(pickPoint, viewDir);
				snapPoints.append(pt);
			}
			//modified by szw 2009.11.18 : end
		}
		break;
	default:
		break;
	}

	return Acad::eOk;
}//added by linlin 20050817


#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDSaddle::subGetGripPoints(
	AcGePoint3dArray& gripPoints,
	AcDbIntArray& osnapModes,
	AcDbIntArray& geomIds) const
#else
Acad::ErrorStatus PDSaddle::getGripPoints(
	AcGePoint3dArray& gripPoints,
	AcDbIntArray& osnapModes,
	AcDbIntArray& geomIds) const
#endif
{
	assertReadEnabled();
	/*	AcGePoint3d strArcPt=m_ptP+m_insZ*m_dHeight;
	AcGeVector3d vecTemp;
	vecTemp = m_insX;
	AcGeVector3d lengthvect=vecTemp.rotateBy(m_dAngle,m_insZ).normal();
	AcGeVector3d widthvect=m_insZ.crossProduct(lengthvect).normal();
	gripPoints.append(m_ptP);
	gripPoints.append(m_ptP+lengthvect*m_dLength);
	gripPoints.append(m_ptP+ lengthvect*m_dLength + widthvect*m_dWidth);
	gripPoints.append(m_ptP + widthvect*m_dWidth);

	gripPoints.append(strArcPt);
	gripPoints.append(strArcPt + lengthvect*m_dLength);
	gripPoints.append(strArcPt + lengthvect*m_dLength + widthvect*m_dWidth);
	gripPoints.append(strArcPt + widthvect*m_dWidth);
	*/
	return Acad::eOk; 
}

Acad::ErrorStatus PDSaddle::getInsDir(AcGeVector3d& insX, AcGeVector3d& insY, AcGeVector3d& insZ)
{
	assertReadEnabled();
	insX = m_insX;
	insY = m_insY;
	insZ = m_insZ;
	return Acad::eOk;
}


#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDSaddle::subExplode(AcDbVoidPtrArray& entitySet) const
#else
Acad::ErrorStatus PDSaddle::explode(AcDbVoidPtrArray& entitySet) const
#endif
{
	assertReadEnabled();

	double arcangle;
	AcGePoint3d strArcPt;
	AcGeVector3d vecTemp;
	AcGeVector3d lengthvect,widthvect;
	getlengthVwidthV(lengthvect,widthvect);

	AcGeVector3d y;
	double ang;
	y = m_insZ.crossProduct(lengthvect).normal();
	ang = y.angleTo(widthvect, lengthvect);

	AcGeVector3d mnormal = -lengthvect;
	AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax, Ay;

	if (fabs(mnormal[X]) <1.0/64 && fabs(mnormal[Y])<1.0/64) 
		Ax = Wy.crossProduct(mnormal);
	else
		Ax = Wz.crossProduct(mnormal);
	Ax.normalize();
	Ay = mnormal.crossProduct(Ax);
	Ay.normalize();

	AcGePoint3d arcCenPt;
	AcDbVoidPtrArray regions;
	Acad::ErrorStatus es = Acad::eOk;
	if(m_dWidth <= m_dRadius * 2.0)
	{
		arcangle = 2.0 * asin(m_dWidth / 2.0 / m_dRadius);  // 弧的角度 
		double downDist = m_dRadius - m_dRadius * cos(arcangle / 2.0);
		strArcPt = m_ptP + m_insZ * m_dHeight;
		arcCenPt = strArcPt + (widthvect * m_dWidth / 2.0) + (m_insZ * m_dRadius * cos(arcangle / 2.0));
		AcDbLine line1(m_ptP, m_ptP + widthvect * m_dWidth);
		AcDbLine line2(m_ptP + widthvect * m_dWidth, strArcPt + widthvect * m_dWidth);
		AcDbLine line3(strArcPt, m_ptP);
		AcDbArc arc;
		AcDbLine line4;
		AcDbVoidPtrArray curveSegments;

		if(downDist < m_dHeight)
		{
			double startAng, endAng;
			startAng = Ax.angleTo(strArcPt + widthvect * m_dWidth - arcCenPt, mnormal);
			endAng = Ax.angleTo(strArcPt - arcCenPt, mnormal);
			arc.setCenter(arcCenPt);
			if(ang > PI / 2.0 && ang < PI * 3.0 / 2.0)
			{	
				arc.setStartAngle(endAng);         
				arc.setEndAngle(startAng);
			}
			else
			{           
				arc.setStartAngle(startAng);         
				arc.setEndAngle(endAng);
			}
			arc.setNormal(mnormal);
			arc.setRadius(m_dRadius);         
		}
		else

		{
			line4.setStartPoint(strArcPt + widthvect * m_dWidth);
			line4.setEndPoint(strArcPt);
			curveSegments.append(&line4);
		}

		curveSegments.append(&line1);
		curveSegments.append(&line2);
		curveSegments.append(&arc);
		curveSegments.append(&line3);

		es = AcDbRegion::createFromCurves(curveSegments, regions);	  
	}
	else
	{
		arcangle = PI;  // 弧的角度 
		double dist = (m_dWidth - m_dRadius * 2.0) / 2.0;
		strArcPt = m_ptP + m_insZ * m_dHeight;
		strArcPt += widthvect * dist;
		arcCenPt = strArcPt + widthvect * m_dRadius;

		AcDbLine line1(m_ptP, m_ptP + widthvect * m_dWidth);
		AcDbLine line2(m_ptP, m_ptP + m_insZ * m_dHeight);
		AcDbLine line3(m_ptP + widthvect * m_dWidth, 
			m_ptP + widthvect * m_dWidth + m_insZ * m_dHeight);
		AcDbLine line4(m_ptP + m_insZ * m_dHeight, strArcPt);
		AcDbLine line5(m_ptP + widthvect * m_dWidth + m_insZ * m_dHeight, 
			m_ptP + m_insZ * m_dHeight + widthvect * (m_dRadius * 2.0 + dist));
		AcDbArc arc;
		AcDbLine line6;
		AcDbVoidPtrArray curveSegments;
		curveSegments.append(&line1);
		curveSegments.append(&line2);
		curveSegments.append(&line3);
		curveSegments.append(&line4);
		curveSegments.append(&line5);
		if(m_dRadius < m_dHeight)
		{
			double startAng, endAng;
			startAng = Ax.angleTo(strArcPt + widthvect * m_dRadius * 2.0 - arcCenPt, mnormal);
			endAng = Ax.angleTo(strArcPt - arcCenPt, mnormal);
			if(ang > PI / 2.0 && ang < PI * 3.0 / 2.0)
			{	
				arc.setStartAngle(endAng);         
				arc.setEndAngle(startAng);
			}
			else
			{           
				arc.setStartAngle(startAng);         
				arc.setEndAngle(endAng);
			}
			arc.setCenter(arcCenPt);
			arc.setNormal(mnormal);
			arc.setRadius(m_dRadius);
			curveSegments.append(&arc);
		}
		else
		{
			line6.setStartPoint(m_ptP + m_insZ * m_dHeight + widthvect * (m_dRadius * 2.0 + dist));
			line6.setEndPoint(strArcPt);
			curveSegments.append(&line6);
		}
		es = AcDbRegion::createFromCurves(curveSegments, regions);
	}

	// [9/5/2007 suzhiyong]
	//((AcDbRegion*)regions[0])->getNormal(vecTemp); 

	if(es == Acad::eOk && !regions.isEmpty())
	{
		// [9/5/2007 suzhiyong]
		((AcDbRegion*)regions[0])->getNormal(vecTemp); 

		AcDb3dSolid *pBody;
		pBody = new AcDb3dSolid;
		if( vecTemp.isCodirectionalTo(lengthvect) )
			es = pBody->extrude((AcDbRegion*)(regions[0]), m_dLength, 0);
		else
			es = pBody->extrude((AcDbRegion*)(regions[0]), -m_dLength, 0);
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


	return Acad::eOk;
}

//  [9/5/2007 suzhiyong]
Acad::ErrorStatus PDSaddle::explodeTo3DSolid(AcDb3dSolid* &p3dSolid) const
{
	assertReadEnabled();

	double arcangle;
	AcGePoint3d strArcPt;
	AcGeVector3d vecTemp;
	AcGeVector3d lengthvect,widthvect;
	getlengthVwidthV(lengthvect,widthvect);

	AcGeVector3d y;
	double ang;
	y = m_insZ.crossProduct(lengthvect).normal();
	ang = y.angleTo(widthvect, lengthvect);

	AcGeVector3d mnormal = -lengthvect;
	AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax, Ay;

	if (fabs(mnormal[X]) <1.0/64 && fabs(mnormal[Y])<1.0/64) 
		Ax = Wy.crossProduct(mnormal);
	else
		Ax = Wz.crossProduct(mnormal);
	Ax.normalize();
	Ay = mnormal.crossProduct(Ax);
	Ay.normalize();

	AcGePoint3d arcCenPt;
	AcDbVoidPtrArray regions;
	Acad::ErrorStatus es = Acad::eOk;
	if(m_dWidth <= m_dRadius * 2.0)
	{
		arcangle = 2.0 * asin(m_dWidth / 2.0 / m_dRadius);  // 弧的角度 
		double downDist = m_dRadius - m_dRadius * cos(arcangle / 2.0);
		strArcPt = m_ptP + m_insZ.normal() * m_dHeight;
		arcCenPt = strArcPt + (widthvect * m_dWidth / 2.0) + (m_insZ.normal() * m_dRadius * cos(arcangle / 2.0));
		AcDbLine line1(m_ptP, m_ptP + widthvect * m_dWidth);
		AcDbLine line2(m_ptP + widthvect * m_dWidth, strArcPt + widthvect * m_dWidth);
		AcDbLine line3(strArcPt, m_ptP);
		AcDbArc arc;
		AcDbLine line4;
		AcDbVoidPtrArray curveSegments;

		if(downDist < m_dHeight)
		{
			double startAng, endAng;
			startAng = Ax.angleTo(strArcPt + widthvect * m_dWidth - arcCenPt, mnormal);
			endAng = Ax.angleTo(strArcPt - arcCenPt, mnormal);
			arc.setCenter(arcCenPt);
			if(ang > PI / 2.0 && ang < PI * 3.0 / 2.0)
			{	
				arc.setStartAngle(endAng);         
				arc.setEndAngle(startAng);
			}
			else
			{           
				arc.setStartAngle(startAng);         
				arc.setEndAngle(endAng);
			}
			arc.setNormal(mnormal);
			arc.setRadius(m_dRadius);         
		}
		else

		{
			line4.setStartPoint(strArcPt + widthvect * m_dWidth);
			line4.setEndPoint(strArcPt);
			curveSegments.append(&line4);
		}

		curveSegments.append(&line1);
		curveSegments.append(&line2);
		curveSegments.append(&arc);
		curveSegments.append(&line3);

		es = AcDbRegion::createFromCurves(curveSegments, regions);	  
	}
	else
	{
		arcangle = PI;  // 弧的角度 
		double dist = (m_dWidth - m_dRadius * 2.0) / 2.0;
		strArcPt = m_ptP + m_insZ * m_dHeight;
		strArcPt += widthvect * dist;
		arcCenPt = strArcPt + widthvect * m_dRadius;

		AcDbLine line1(m_ptP, m_ptP + widthvect * m_dWidth);
		AcDbLine line2(m_ptP, m_ptP + m_insZ * m_dHeight);
		AcDbLine line3(m_ptP + widthvect * m_dWidth, 
			m_ptP + widthvect * m_dWidth + m_insZ * m_dHeight);
		AcDbLine line4(m_ptP + m_insZ * m_dHeight, strArcPt);
		AcDbLine line5(m_ptP + widthvect * m_dWidth + m_insZ * m_dHeight, 
			m_ptP + m_insZ * m_dHeight + widthvect * (m_dRadius * 2.0 + dist));
		AcDbArc arc;
		AcDbLine line6;
		AcDbVoidPtrArray curveSegments;
		curveSegments.append(&line1);
		curveSegments.append(&line2);
		curveSegments.append(&line3);
		curveSegments.append(&line4);
		curveSegments.append(&line5);
		if(m_dRadius < m_dHeight)
		{
			double startAng, endAng;
			startAng = Ax.angleTo(strArcPt + widthvect * m_dRadius * 2.0 - arcCenPt, mnormal);
			endAng = Ax.angleTo(strArcPt - arcCenPt, mnormal);
			if(ang > PI / 2.0 && ang < PI * 3.0 / 2.0)
			{	
				arc.setStartAngle(endAng);         
				arc.setEndAngle(startAng);
			}
			else
			{           
				arc.setStartAngle(startAng);         
				arc.setEndAngle(endAng);
			}
			arc.setCenter(arcCenPt);
			arc.setNormal(mnormal);
			arc.setRadius(m_dRadius);
			curveSegments.append(&arc);
		}
		else
		{
			line6.setStartPoint(m_ptP + m_insZ * m_dHeight + widthvect * (m_dRadius * 2.0 + dist));
			line6.setEndPoint(strArcPt);
			curveSegments.append(&line6);
		}
		es = AcDbRegion::createFromCurves(curveSegments, regions);
	}

	// [9/5/2007 suzhiyong]
	//((AcDbRegion*)regions[0])->getNormal(vecTemp); 

	if(es == Acad::eOk && !regions.isEmpty())
	{
		// [9/5/2007 suzhiyong]
		((AcDbRegion*)regions[0])->getNormal(vecTemp); 

		AcDb3dSolid *pBody;
		pBody = new AcDb3dSolid;
		if( vecTemp.isCodirectionalTo(lengthvect) )
			es = pBody->extrude((AcDbRegion*)(regions[0]), m_dLength, 0);
		else
			es = pBody->extrude((AcDbRegion*)(regions[0]), -m_dLength, 0);
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


	return Acad::eOk;
}

Acad::ErrorStatus PDSaddle::getExtrudePolygonPts(AcGePoint3dArray& vertexArray) const
{
	/////////////////////////////////////////////////////////////////////////
	//////	数组2*m_nPrecision个点表示马鞍曲面上的点				/////////
	/////////////////////////////////////////////////////////////////////////
	assertReadEnabled();

	while(!vertexArray.isEmpty())
		vertexArray.removeLast();

	double arcangle;
	AcGePoint3d strArcPt;
	AcGeVector3d lengthvect,widthvect;
	getlengthVwidthV(lengthvect,widthvect) ;

	AcGePoint3d arcCenPt;
	double arcInSadHight;
	if(m_dWidth <= m_dRadius * 2.0)
	{
		arcangle = 2.0 * asin(m_dWidth / 2.0 / m_dRadius);  // 弧的角度 
		strArcPt = m_ptP + m_insZ * m_dHeight;
		arcCenPt = strArcPt + (widthvect * m_dWidth / 2.0) + (m_insZ * m_dRadius * cos(arcangle / 2.0));
		arcInSadHight = m_dWidth / 2.0 * tan(arcangle / 2.0);
	}
	else
	{
		arcangle = PI;  // 弧的角度 
		double dist = (m_dWidth - m_dRadius * 2.0) / 2.0;
		strArcPt = m_ptP + m_insZ * m_dHeight;
		strArcPt += widthvect * dist;
		arcCenPt = strArcPt + widthvect * m_dRadius;
		arcInSadHight = m_dRadius;
	}

	double extendDownHight = 0.0;
	if(arcInSadHight > m_dHeight)
		extendDownHight = arcInSadHight - m_dHeight;
	else if(arcInSadHight == m_dHeight)
		extendDownHight = m_dHeight / 1000;

	AcGePoint3dArray vertexArray1;
	AcGePoint3d point = strArcPt;
	double deltaangle = arcangle / m_dDividPrecision;

	vertexArray.append(point);
	AcGeVector3d z;
	double ang;
	z = m_insX.crossProduct(m_insY).normal();
	ang = z.angleTo(m_insZ, m_insX);
	if(ang > PI / 2.0 && ang < PI * 3.0 / 2.0)
		for(Adesk::Int32 j = 1; j <= m_dDividPrecision; j++)
		{ 
			point.rotateBy(deltaangle, -lengthvect, arcCenPt);
			vertexArray.append(point);
		}
	else
		for(Adesk::Int32 j = 1; j <= m_dDividPrecision; j++)
		{ 
			point.rotateBy(deltaangle, lengthvect, arcCenPt);
			vertexArray.append(point);
		}

		if(m_dWidth > m_dRadius * 2.0)
			vertexArray.append(m_ptP + widthvect * m_dWidth + m_insZ * m_dHeight);
		vertexArray.append(m_ptP - m_insZ * extendDownHight + widthvect * m_dWidth);
		vertexArray.append(m_ptP - m_insZ * extendDownHight);
		if(m_dWidth > m_dRadius * 2.0)
			vertexArray.append(m_ptP + m_insZ * m_dHeight);

		return Acad::eOk;
}

void PDSaddle::createBody()
{
	assertWriteEnabled();
#ifdef _OBJECTARX2004_

	AcGePoint3d ori;
	AcGeVector3d Vectx, Vecty, Vectz,Vlength,Vwidth;
	double length, width, height;
	getRightHandCoord(ori, Vectx, Vecty, Vectz, Vlength,Vwidth, length, width, height);

	AcGePoint3dArray vertexArray;
	getExtrudePolygonPts(vertexArray);
	int vertexNum = vertexArray.length();
	if(vertexNum < 3)
		return;

	AcGePoint3d* vertices;
	PolygonVertexData** vertexData;
	PolygonVertexData vd(PolygonVertexData::kUnspecifiedCurve);

	vertices = new AcGePoint3d[vertexNum];
	vertexData = new PolygonVertexData*[vertexNum];

	for(int i = 0; i < vertexNum; ++i)
	{
		vertices[i] = vertexArray[i];
		vertexData[i] = &vd;
	}

	m_3dGeom = Body::extrusion((Point3d*)vertices, vertexData, vertexNum, *(Vector3d*)&Vlength, *(Vector3d*)&(Vlength * length));
	delete [] vertices;
	delete [] vertexData;
#endif
}

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDSaddle::subGetGeomExtents(AcDbExtents& extents) const
#else
Acad::ErrorStatus PDSaddle::getGeomExtents(AcDbExtents& extents) const
#endif
{
	assertReadEnabled();
	extents.set(m_ptP, m_ptP);
	AcGeVector3d lengthvect,widthvect, heightvect;
	getlengthVwidthV(lengthvect,widthvect);
	extents.addPoint(m_ptP + m_insZ * m_dHeight);
	AcGePoint3d pt[3];
	pt[0] = m_ptP + lengthvect * m_dLength;
	pt[1] = pt[0] + widthvect * m_dWidth;
	pt[2] = m_ptP + widthvect * m_dWidth;
	int i;
	for(i = 0; i < 3; ++i)
	{
		extents.addPoint(pt[i]);
		extents.addPoint(pt[i] + m_insZ * m_dHeight);
	}
	return Acad::eOk;
}

#ifdef _OBJECTARX2010_
void PDSaddle::subList() const
#else
void PDSaddle::list() const
#endif
{
	assertReadEnabled();
#ifdef _OBJECTARX2010_
	PDPrimary3D::subList();
#else
	PDPrimary3D::list();
#endif
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4732/*"\n 插入点(%f,%f,%f)\n"*/,m_ptP.x,m_ptP.y,m_ptP.z);
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4770/*" 长度=%.3f 宽度=%.3f 高度=%.3f\n"*/,m_dLength,m_dWidth,m_dHeight);
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4771/*" 底面长轴和X轴的夹角=%.3f\n"*/,m_dAngle);
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4772/*" 上面圆弧的半径=%.3f\n"*/,m_dRadius);    
	return ;
}// added by linlin 20050810
