// PDSqucir.cpp: implementation of the PDSqucir class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "language25.h"
#include "PDSqucir.h"
#include "dbproxy.h"
#include "acgi.h"
#include "math.h"
#include "gecspl3d.h"
#include "geassign.h"
#include <dbsol3d.h>
#include <dbents.h>
#include <dbregion.h>
#include < geintarr.h >
#include <aced.h>

#ifdef _OBJECTARX2000_
#include <migrtion.h>
#include <dbapserv.h>
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define VERSION_PDSQUCIR 2
#define  DIVIDE_MIN_NUM 3

const double SIGMA = 0.01;

#define PI 3.1415926535897932384626433832795L

#ifdef _OBJECTARX2000_
ACRX_DXF_DEFINE_MEMBERS(PDSqucir, PDPrimary3D,
						AcDb::kDHL_CURRENT, AcDb::kMReleaseCurrent,
						AcDbProxyEntity::kAllAllowedBits,
						PDSQUCIR, Gle);
#else
ACRX_DXF_DEFINE_MEMBERS(PDSqucir, PDPrimary3D, AcDbProxyEntity::kAllAllowedBits, PDSQUCIR, Gle);
#endif

void PDSqucir::setDefault()
{
	PDPrimary3D::setDefault();
#if defined(NDEBUG) || !defined(_DEBUG)
	m_dDividPrecision = PRIMARY3D_DEFAULT_PRECISION;
	m_dLength = m_dWidth = m_dRadius = 1.0;
	m_ptP2 = m_ptP1 = AcGePoint3d();
	m_VectCir = m_VectSqu = AcGeVector3d();
	m_VectW.set(0,0,0);
#else
	m_dDividPrecision = PRIMARY3D_DEFAULT_PRECISION;
	m_dLength = m_dWidth = m_dRadius = 1000;
	m_ptP2.set(0,0,0);
	m_ptP1.set(1000,0,0);
	m_VectCir.set(0,0,1);
	m_VectSqu.set(0,0,1);
	m_VectW.set(0,1,0);
#endif
}
bool PDSqucir::isValidData(double &f)
{
	if(!PDPrimary3D::isValidData(f))
		return false;
	if(!isValidFloat(f=m_dLength))
		return false;
	if(!isValidFloat(f=m_dWidth))
		return false;
	if(!isValidFloat(f=m_dRadius))
		return false;
	if(!isValidFloat(f=m_ptP2.x) || !isValidFloat(f=m_ptP2.y) || !isValidFloat(f=m_ptP2.z))
		return false;
	if(!isValidFloat(f=m_ptP1.x) || !isValidFloat(f=m_ptP1.y) || !isValidFloat(f=m_ptP1.z))
		return false;
	if(!isValidFloat(f=m_VectCir.x) || !isValidFloat(f=m_VectCir.y) || !isValidFloat(f=m_VectCir.z))
		return false;
	if(!isValidFloat(f=m_VectSqu.x) || !isValidFloat(f=m_VectSqu.y) || !isValidFloat(f=m_VectSqu.z))
		return false;
	if(!isValidFloat(f=m_VectW.x) || !isValidFloat(f=m_VectW.y) || !isValidFloat(f=m_VectW.z))
		return false;
	return true;
}

//zxb,20090113,增加audit，暂时只check自身成员数据
Acad::ErrorStatus PDSqucir::audit(AcDbAuditInfo* pAuditInfo)
{
	return auditPDPrimary3D(this,pAuditInfo,_T("PDSqucir"));
}

PDSqucir::PDSqucir(bool HasSnap/* = false*/) : PDPrimary3D(HasSnap)
{
	m_dDividPrecision = PRIMARY3D_DEFAULT_PRECISION;
	m_dLength = m_dWidth = m_dRadius = 1.0;
	m_ptP2 = m_ptP1 = AcGePoint3d();
	m_VectCir = m_VectSqu = AcGeVector3d();
	m_VectW.set(0,0,0);
#ifdef _USEAMODELER_
	createBody();
#endif
	GetMaxLength();
}

PDSqucir::~PDSqucir()
{

}
PDSqucir::PDSqucir(AcGePoint3d p1,AcGeVector3d v1,double l,double w,
				   AcGePoint3d p2,AcGeVector3d v2,double r,int precision/* = 12*/,
				   bool HasSnap/* = false*/,int CorrType /*1: 右手系*/) : PDPrimary3D(HasSnap)
{
	m_ptP1 = p1;
	m_ptP2 = p2;

	m_VectSqu = v1.normal();
	m_VectCir = v2.normal();

	m_dLength = l;
	m_dWidth = w;
	m_dRadius = r;

	if (v1.isZeroLength()&& v2.isZeroLength())
	{
		m_VectCir.set(0,0,1);
		m_VectSqu.set(1,0,0);
	}
	else
		if(v2.isZeroLength())
		{
			AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax, Ay;
			if (fabs(v1[X]) <1.0/64 && fabs(v1[Y])<1.0/64) 
				Ax = Wy.crossProduct(v1);
			else
				Ax = Wz.crossProduct(v1);
			Ax.normalize();
			m_VectCir=Ax;
		}
		else
			if(v1.isZeroLength())
			{
				AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax, Ay;
				if (fabs(v2[X]) <1.0/64 && fabs(v2[Y])<1.0/64) 
					Ax = Wy.crossProduct(v2);
				else
					Ax = Wz.crossProduct(v2);
				Ax.normalize();
				m_VectSqu=Ax;
			} 
			else
				if(v1.isParallelTo(v2))
				{
					AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax, Ay;
					if (fabs(v2[X]) <1.0/64 && fabs(v2[Y])<1.0/64) 
						Ax = Wy.crossProduct(v2);
					else
						Ax = Wz.crossProduct(v2);
					Ax.normalize();
					m_VectSqu=Ax;
				}

				if(precision > 2)
					m_dDividPrecision = precision;
				else
					m_dDividPrecision = PRIMARY3D_DEFAULT_PRECISION;

				if(CorrType)
					m_VectW = m_VectCir.crossProduct(m_VectSqu).normalize();
				else
					m_VectW = m_VectSqu.crossProduct(m_VectCir).normalize();

#ifdef _USEAMODELER_
				createBody();
#endif
				GetMaxLength();
}//added by linlin 20050929

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDSqucir::subTransformBy(const AcGeMatrix3d& xform)
#else
Acad::ErrorStatus PDSqucir::transformBy(const AcGeMatrix3d& xform)
#endif
{
	assertWriteEnabled();
	// TODO: implement this function.
	m_ptP1.transformBy(xform);
	m_ptP2.transformBy(xform);
	m_VectCir.transformBy(xform).normalize();
	m_VectSqu.transformBy(xform).normalize();
	m_VectW.transformBy(xform).normalize();

	m_dLength *= xform.scale();
	m_dRadius *= xform.scale();
	m_dWidth *= xform.scale();
	GetMaxLength();

#ifdef _OBJECTARX2010_
	return PDPrimary3D::subTransformBy(xform);
#else
	return PDPrimary3D::transformBy(xform);
#endif
}

#ifdef _OBJECTARX2010_
Adesk::Boolean PDSqucir::subWorldDraw(AcGiWorldDraw* mode)
#else
Adesk::Boolean PDSqucir::worldDraw(AcGiWorldDraw* mode)
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
	AcGeVector3d lengthvect = m_VectSqu.normal() * m_dLength;
	AcGeVector3d widthvect= m_VectW * m_dWidth;
	int PreStep = 1;	
	AcGePoint3dArray pArray;
	int actPrecision__1 = Precision + 1;

	AcGePoint3d ptsqu[5];
	ptsqu[0] = m_ptP1;
	ptsqu[1] = m_ptP1 + lengthvect;
	ptsqu[2] = ptsqu[1] + widthvect;
	ptsqu[3] = ptsqu[0] + widthvect;
	ptsqu[4] = m_ptP1;

	Adesk::Int16 nRating = getCalMaxPrecision();
	if(nRating == PRIMARY3D_SIMPLIZE_RATE1 ||
		nRating == PRIMARY3D_SIMPLIZE_RATE2)
	{
		Precision = (int)(PI / acos(1.0 - mode->deviation(kAcGiMaxDevForCircle, m_ptP1)* 2.0 / m_dMaxLength));
		if(Precision < PRIMARY3D_SIMPLIZE_SCALE)
		{
			if (nRating == PRIMARY3D_SIMPLIZE_RATE2)
			{	
				AcGePoint3d pts[2];
				pts[0] = m_ptP1 + (lengthvect + widthvect) / 2.0;
				pts[1] = m_ptP2;
				mode->geometry().polyline(2,pts);
			}
			else if (nRating == PRIMARY3D_SIMPLIZE_RATE1)
			{
				/////////////////////////////////////////////////////////////////////
				///////////		draw the lower square face line			/////////////
				int i;
				for(i = 0; i < 4; i++)
				{
					if(mode->geometry().polyline(2, &(ptsqu[i])))
						return Adesk::kTrue;
				}
				/////////////////////////////////////////////////////////////////////
				//////////		draw the upper circle					/////////////
				if(mode->geometry().circle(m_ptP2, m_dRadius, m_VectCir))
					return Adesk::kTrue;
				/////////////////////////////////////////////////////////////////////
				//////////		draw the sorrounding lines				/////////////
				Precision = PRIMARY3D_WIREFRAME_LINENUM4;
				getVertices(Precision, pArray);
				actPrecision__1 = Precision + 1;

				AcGePoint3d pt[2];
				int Step = Precision /2;
				for(int j=0;j<4;j++)
				{
					for (i = 0; i < DIVIDE_MIN_NUM; i ++)
					{     
						pt[0] = pArray[i* Step + j*actPrecision__1];
						pt[1] = pArray[i * Step + (j + 4)*actPrecision__1];
						if(mode->geometry().polyline(2, pt))
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
			mode->subEntityTraits().setFillType(kAcGiFillAlways);
			if(Precision > m_dDividPrecision)
				Precision = m_dDividPrecision;
			if(Precision < PRIMARY3D_WIREFRAME_LINENUM * 2)
				Precision = PRIMARY3D_WIREFRAME_LINENUM;
			else
			{
				PreStep = Precision / PRIMARY3D_WIREFRAME_LINENUM;
				Precision = PreStep * PRIMARY3D_WIREFRAME_LINENUM;
			}

			getVertices(Precision, pArray);
			actPrecision__1 = Precision + 1;
			//下地面
			//添加实体模型各面的GS标记
			//added by szw 2009.11.18 : begin
			int gsIndex = 0;
			mode->subEntityTraits().setSelectionMarker(++gsIndex);
			//added by szw 2009.11.18 : end
			if(mode->geometry().polygon(4, ptsqu))
				return Adesk::kTrue;
			//顶面
			//added by szw 2009.11.18 : begin
			mode->subEntityTraits().setSelectionMarker(++gsIndex);	
			//added by szw 2009.11.18 : end
			if(mode->geometry().polygon(actPrecision__1*4, pArray.asArrayPtr()))
				return Adesk::kTrue;

			AcGePoint3d pt[3];
			for(int j=0;j<4;j++)
			{			        
				pt[0] = ptsqu[j];
				pt[1] = ptsqu[j+ 1];
				pt[2] = pArray[actPrecision__1*j + Precision];
				if(mode->geometry().polygon(3, pt))
					return Adesk::kTrue;
			}
			int i = 0;
			////////////////////////////////////////////////////////////////////////
			//////		using the mesh function to draw the sorrounding		////////
			for( i = 0; i < 4; i++)
			{
				AcGiEdgeData edgeData;
				int edgeDataLen = actPrecision__1 + 2 * (Precision);
				Adesk::UInt8* edgeVisibility = new Adesk::UInt8[edgeDataLen];
				for(int k = 0; k < edgeDataLen; k++)
					edgeVisibility[k] = kAcGiInvisible;
				edgeVisibility[2 * Precision + Precision / 2] = kAcGiSilhouette;

				edgeData.setVisibility(edgeVisibility);

				AcGePoint3dArray pts, pts1;
				for(int j = 0; j <actPrecision__1; j++)
				{
					pts.append(pArray[j+actPrecision__1*i]);
					pts1.append(pArray[j+actPrecision__1*(i+4)]);
				}
				pts.append(pts1);
				//added by szw 2009.11.18 : begin
				mode->subEntityTraits().setSelectionMarker(++gsIndex);
				//added by szw 2009.11.18 : end
				if(mode->geometry().mesh(2, pts.length() / 2, pts.asArrayPtr(), &edgeData))
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
			int gsIndex = 0;
			/////////////////////////////////////////////////////////////////////
			///////////		draw the lower square face line			/////////////
			int i;
			//统一线框模型和实体模型的GS标记
			//modified by szw 2009.11.18 : begin
			++gsIndex;
			for(i = 0; i < 4; i++)
			{
				mode->subEntityTraits().setSelectionMarker(gsIndex);
				if(mode->geometry().polyline(2, &(ptsqu[i])))
					return Adesk::kTrue;
			}
			/////////////////////////////////////////////////////////////////////
			//////////		draw the upper circle					/////////////
			mode->subEntityTraits().setSelectionMarker(++gsIndex);
			if(mode->geometry().circle(m_ptP2, m_dRadius, m_VectCir))
				return Adesk::kTrue;
			/////////////////////////////////////////////////////////////////////
			//////////		draw the sorrounding lines				/////////////
			Precision = PRIMARY3D_WIREFRAME_LINENUM;
			getVertices(Precision, pArray);
			actPrecision__1 = Precision + 1;

			AcGePoint3d pt[2];
			int Step = Precision /2;
			for(int j=0;j<4;j++)
			{
				++gsIndex;
				for (i = 0; i < DIVIDE_MIN_NUM; i ++)
				{     
					pt[0] = pArray[i* Step + j*actPrecision__1];
					pt[1] = pArray[i * Step + (j + 4)*actPrecision__1];
					mode->subEntityTraits().setSelectionMarker(gsIndex);
					if(mode->geometry().polyline(2, pt))
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
}//added by linlin 20050818

/*Adesk::Boolean PDSqucir::worldDraw(AcGiWorldDraw* mode)
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

AcGeVector3d lengthvect = m_VectSqu.normal();
AcGeVector3d widthvect= m_VectW;

AcGePoint3d ptsqu[5];
ptsqu[0] = m_ptP1;
ptsqu[1] = m_ptP1 + lengthvect * m_dLength;
ptsqu[2] = ptsqu[1] + widthvect * m_dWidth;
ptsqu[3] = ptsqu[0] + widthvect * m_dWidth;
ptsqu[4] = m_ptP1;

switch(mode->regenType())
{
case kAcGiStandardDisplay:
case kAcGiSaveWorldDrawForR12:
case kAcGiSaveWorldDrawForProxy:
{
int gsIndex = 0;
/////////////////////////////////////////////////////////////////////
///////////		draw the lower square face line			/////////////
for(int i = 0; i < 4; i++)
{
mode->subEntityTraits().setSelectionMarker(++gsIndex);
mode->geometry().polyline(2, &(ptsqu[i]));
}
/////////////////////////////////////////////////////////////////////
//////////		draw the upper circle					/////////////
mode->subEntityTraits().setSelectionMarker(++gsIndex);
mode->geometry().circle(m_ptP2, m_dRadius, m_VectCir);
/////////////////////////////////////////////////////////////////////
//////////		draw the sorrounding lines				/////////////
AcGePoint3d pt[2];
for(int j=0;j<4;j++)
{
for (i = 0; i < stdIdxLen; i++)
{     
pt[0] = pArray[stdIdx[i]+j*actPrecision__1];
pt[1] = pArray[stdIdx[i]+j*actPrecision__1+ actPrecision__1*4];
mode->subEntityTraits().setSelectionMarker(++gsIndex);
mode->geometry().polyline(2, pt);	
}
}
}
break;
case kAcGiHideOrShadeCommand:
case kAcGiRenderCommand:
{
mode->subEntityTraits().setFillType(kAcGiFillAlways);
mode->geometry().polygon(4, ptsqu);
mode->geometry().polygon(actPrecision__1*4, pArray.asArrayPtr());	

AcGePoint3d pt[3];
for(int j=0;j<4;j++)
{			        
pt[0] = ptsqu[j];
pt[1] = ptsqu[j+ 1];
pt[2] = pArray[stdIdx[stdIdxLen-1]+actPrecision__1*j];
mode->geometry().polygon(3, pt);		

}
int i = 0;
AcGePoint3d pt4[4];
for(i = 0 ; i < 4; i++)
{
pt4[0] = pArray[stdIdx[1] +actPrecision__1*i];  
pt4[1] = pArray[stdIdx[1] +actPrecision__1*i];  
pt4[2] =ptsqu[i];
pt4[3] =ptsqu[i];
mode->geometry().polygon(4, pt4);
}
////////////////////////////////////////////////////////////////////////
//////		using the mesh function to draw the sorrounding		////////

for( i = 0; i < 4; i++)
{
AcGiEdgeData edgeData;
int edgeDataLen = actPrecision__1 + 2 * (actPrecision);
Adesk::UInt8* edgeVisibility = new Adesk::UInt8[edgeDataLen];
if(edgeDataLen > 0)
edgeVisibility[0] = kAcGiSilhouette;
for(int k = 1; k < edgeDataLen; k++)
edgeVisibility[k] = kAcGiInvisible;

edgeData.setVisibility(edgeVisibility);

AcGePoint3dArray pts, pts1;
for(int j = 0; j <actPrecision__1; j++)
{
pts.append(pArray[j+actPrecision__1*i]);
pts1.append(pArray[j+actPrecision__1*(i+4)]);
}
pts.append(pts1);

mode->geometry().mesh(2, pts.length() / 2, pts.asArrayPtr(), &edgeData);
delete [] edgeVisibility;
}   

}
break;
default:
break;
}
#endif
return Adesk::kTrue;
}//added by linlin 20050818*/
//////////////////////////////////////////////////////////////////////
//////	divide the circle to m_nPrecision*4 points				//////
//////	add each of the point of squ m_nPrecision times			//////
//////	connect each point to the corresponding point of squ	//////
//////	so the number of vertex is 2* (4*m_nPrecision+1)		///////
//////	the "+1" in (m_nPrecision+1) is important					//////
//////////////////////////////////////////////////////////////////////

Acad::ErrorStatus  PDSqucir::getVertices(int Precision, AcGePoint3dArray& vertexArray, 
										 AcGeIntArray &stdIdx, int &actPrecision) const
{
	assertReadEnabled();

	while(!vertexArray.isEmpty())
		vertexArray.removeLast();


	AcGeDoubleArray dividDbl;
	PDSqucir:: getActDivid(Precision, dividDbl, stdIdx);

	int actPrecision__1 = dividDbl.length();
	actPrecision = actPrecision__1 - 1;

	AcGeVector3d lengthvect = m_VectSqu.normal();
	AcGeVector3d widthvect = m_VectW;

	AcGePoint3d ptsqu1 = m_ptP1;
	AcGePoint3d ptsqu2 = m_ptP1 + lengthvect * m_dLength;
	AcGePoint3d ptsqu3 = ptsqu2 + widthvect * m_dWidth;
	AcGePoint3d ptsqu4 = ptsqu1 + widthvect * m_dWidth;

	AcGePoint3d point = m_ptP2 - lengthvect * m_dRadius;
	vertexArray.append(point);
	AcGePoint3d VecTemp;
	AcGeVector3d y;
	double ang;
	y = m_VectCir.crossProduct(m_VectSqu).normal();
	ang = y.angleTo(m_VectW, m_VectSqu);

	int i;
	for(i = 1; i < actPrecision; i++)
	{
		VecTemp=point	;	  

		if(ang > PI / 2.0 && ang < PI * 3.0 / 2.0)
			VecTemp.rotateBy(PI / 2.0 / Precision*dividDbl[i], -m_VectCir, m_ptP2);
		else
			VecTemp.rotateBy(PI / 2.0 / Precision*dividDbl[i], m_VectCir, m_ptP2);

		vertexArray.append(VecTemp);
	}

	vertexArray.append(m_ptP2-widthvect*m_dRadius);
	vertexArray.append(m_ptP2-widthvect*m_dRadius);

	for( i = 1; i < actPrecision ; i++)
	{
		VecTemp= m_ptP2 - widthvect * m_dRadius	;

		if(ang > PI / 2.0 && ang < PI * 3.0 / 2.0)
			VecTemp.rotateBy(PI / 2.0 / Precision*dividDbl[i], -m_VectCir, m_ptP2);
		else
			VecTemp.rotateBy(PI / 2.0 / Precision*dividDbl[i], m_VectCir, m_ptP2);

		vertexArray.append(VecTemp);
	}

	vertexArray.append(m_ptP2+lengthvect*m_dRadius);
	vertexArray.append(m_ptP2+lengthvect*m_dRadius);

	for( i = 1; i < actPrecision ; i++)
	{
		VecTemp= m_ptP2 + lengthvect * m_dRadius;	

		if(ang > PI / 2.0 && ang < PI * 3.0 / 2.0)
			VecTemp.rotateBy(PI / 2.0 / Precision*dividDbl[i], -m_VectCir, m_ptP2);
		else
			VecTemp.rotateBy(PI / 2.0 / Precision*dividDbl[i], m_VectCir, m_ptP2);

		vertexArray.append(VecTemp);
	}	 

	vertexArray.append(m_ptP2+widthvect * m_dRadius);
	vertexArray.append(m_ptP2+widthvect * m_dRadius);

	for( i = 1; i < actPrecision ; i++)
	{
		VecTemp= m_ptP2 + widthvect * m_dRadius;

		if(ang > PI / 2.0 && ang < PI * 3.0 / 2.0)
			VecTemp.rotateBy(PI / 2.0 / Precision*dividDbl[i], -m_VectCir, m_ptP2);
		else
			VecTemp.rotateBy(PI / 2.0 / Precision*dividDbl[i], m_VectCir, m_ptP2);

		vertexArray.append(VecTemp);    
	}
	vertexArray.append(point);
	//////////////////////////////////////////////////////	
	//////	append the point on squ				//////////
	for(i = 0; i <= actPrecision; i++)
		vertexArray.append(ptsqu1);
	for(i = 0; i <= actPrecision; i++)
		vertexArray.append(ptsqu2);
	for(i = 0; i <= actPrecision; i++)
		vertexArray.append(ptsqu3);
	for(i = 0; i <= actPrecision; i++)
		vertexArray.append(ptsqu4);

	return Acad::eOk;
}//added by linlin 20050818

Acad::ErrorStatus  PDSqucir::getVertices(int Precision, AcGePoint3dArray& vertexArray) const
{
	assertReadEnabled();
	//while(!vertexArray.isEmpty())
	//	vertexArray.removeLast();
	vertexArray.setLogicalLength(0);

	int actPrecision__1 = Precision + 1;

	AcGeVector3d lengthvect = m_VectSqu.normal();
	AcGeVector3d widthvect = m_VectW.normal();
	AcGeVector3d vectAlongRL = lengthvect * m_dRadius;
	AcGeVector3d vectAlongRW = widthvect * m_dRadius;
	AcGeVector3d vectAlongL = lengthvect * m_dLength;
	AcGeVector3d vectAlongW = widthvect * m_dWidth;

	AcGePoint3d ptsqu1 = m_ptP1;
	AcGePoint3d ptsqu2 = m_ptP1 + vectAlongL;
	AcGePoint3d ptsqu3 = ptsqu2 + vectAlongW;
	AcGePoint3d ptsqu4 = ptsqu1 + vectAlongW;

	AcGePoint3d point = m_ptP2 - vectAlongRL;
	vertexArray.append(point);
	AcGePoint3d VecTemp;
	AcGeVector3d y;
	double ang;
	y = m_VectCir.crossProduct(m_VectSqu).normal();
	ang = y.angleTo(m_VectW, m_VectSqu);
	double dAngle = PI / 2.0 / Precision;
	VecTemp=point	;
	int i;
	for(i = 1; i < Precision; i++)
	{
		if(ang > PI / 2.0 && ang < PI * 3.0 / 2.0)
			VecTemp.rotateBy(dAngle, -m_VectCir, m_ptP2);
		else
			VecTemp.rotateBy(dAngle, m_VectCir, m_ptP2);

		vertexArray.append(VecTemp);
	}
	VecTemp= m_ptP2 - vectAlongRW;
	vertexArray.append(VecTemp);
	vertexArray.append(VecTemp);
	for( i = 1; i < Precision ; i++)
	{

		if(ang > PI / 2.0 && ang < PI * 3.0 / 2.0)
			VecTemp.rotateBy(dAngle, -m_VectCir, m_ptP2);
		else
			VecTemp.rotateBy(dAngle, m_VectCir, m_ptP2);

		vertexArray.append(VecTemp);
	}

	VecTemp= m_ptP2 + vectAlongRL;
	vertexArray.append(VecTemp);
	vertexArray.append(VecTemp);

	for( i = 1; i < Precision ; i++)
	{
		if(ang > PI / 2.0 && ang < PI * 3.0 / 2.0)
			VecTemp.rotateBy(dAngle, -m_VectCir, m_ptP2);
		else
			VecTemp.rotateBy(dAngle, m_VectCir, m_ptP2);

		vertexArray.append(VecTemp);
	}	 

	VecTemp= m_ptP2 + vectAlongRW;
	vertexArray.append(VecTemp);
	vertexArray.append(VecTemp);

	for( i = 1; i < Precision ; i++)
	{
		if(ang > PI / 2.0 && ang < PI * 3.0 / 2.0)
			VecTemp.rotateBy(dAngle, -m_VectCir, m_ptP2);
		else
			VecTemp.rotateBy(dAngle, m_VectCir, m_ptP2);

		vertexArray.append(VecTemp);    
	}
	vertexArray.append(point);
	//////////////////////////////////////////////////////	
	//////	append the point on squ				//////////
	for(i = 0; i <= Precision; i++)
		vertexArray.append(ptsqu1);
	for(i = 0; i <= Precision; i++)
		vertexArray.append(ptsqu2);
	for(i = 0; i <= Precision; i++)
		vertexArray.append(ptsqu3);
	for(i = 0; i <= Precision; i++)
		vertexArray.append(ptsqu4);
	//////////////////////////////////////////////////////////////////////////
	return Acad::eOk;
}//added by linlin 20050818
Acad::ErrorStatus PDSqucir::dwgOutFields(AcDbDwgFiler* filer)const
{
	assertReadEnabled();
	Acad::ErrorStatus es=Acad::eOk;

	// Call dwgOutFields from father class: PDPrimary3D
	if((es=PDPrimary3D::dwgOutFields(filer))!=Acad::eOk)
	{
		return es;
	}

	// Write version number.
	filer->writeItem((Adesk::UInt16) VERSION_PDSQUCIR);

	// version 2
	filer->writeItem(m_VectW);

	// Write the data members.
	filer->writeItem(m_ptP1);
	filer->writeItem(m_ptP2);
	filer->writeItem(m_dLength);
	filer->writeItem(m_dWidth);
	filer->writeItem(m_dRadius);
	filer->writeItem(m_VectCir);
	filer->writeItem(m_VectSqu);
	filer->writeItem(m_nPrecision);

	return filer->filerStatus();
}

Acad::ErrorStatus PDSqucir::dwgInFields(AcDbDwgFiler* filer)
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
	if (version > VERSION_PDSQUCIR)
		return Acad::eMakeMeProxy;

	// Read the data members.
	if (version>=2)
		filer->readItem(&m_VectW);
	if (version>=1)
	{
		filer->readItem(&m_ptP1);
		filer->readItem(&m_ptP2);
		filer->readItem(&m_dLength);
		filer->readItem(&m_dWidth);
		filer->readItem(&m_dRadius);
		filer->readItem(&m_VectCir);
		filer->readItem(&m_VectSqu);
		filer->readItem(&m_nPrecision);

		if(version==1)
			m_VectW=m_VectCir.crossProduct(m_VectSqu).normalize();
	}

#ifdef _USEAMODELER_
	if(m_3dGeom.isNull())
		createBody();
#endif
	GetMaxLength();
	return filer->filerStatus();
}


Acad::ErrorStatus PDSqucir::dxfOutFields(AcDbDxfFiler* filer) const
{
	assertReadEnabled();
	Acad::ErrorStatus es;

	if ((es = PDPrimary3D::dxfOutFields(filer))
		!= Acad::eOk)
	{
		return es;
	}

	// Write subclass marker.
	filer->writeItem(AcDb::kDxfSubclass, _T("PDSqucir"));

	// Write version number.
	filer->writeItem(AcDb::kDxfInt16, (Adesk::UInt16) VERSION_PDSQUCIR);

	// version 2
	filer->writeItem(AcDb::kDxfNormalX+2,m_VectW);

	// Write data members.
	filer->writeItem(AcDb::kDxfXCoord, m_ptP1);
	filer->writeItem(AcDb::kDxfXCoord+1, m_ptP2);
	filer->writeItem(AcDb::kDxfReal,m_dLength);
	filer->writeItem(AcDb::kDxfReal+1,m_dWidth);
	filer->writeItem(AcDb::kDxfReal+2,m_dRadius);
	filer->writeItem(AcDb::kDxfNormalX,m_VectCir);
	filer->writeItem(AcDb::kDxfNormalX+1,m_VectSqu);
	filer->writeItem(AcDb::kDxfInt32,m_nPrecision);

	return filer->filerStatus();
}

Acad::ErrorStatus PDSqucir::dxfInFields(AcDbDxfFiler* filer)
{
	assertWriteEnabled();
	Acad::ErrorStatus es = Acad::eOk;
	struct resbuf rb;

	if ((PDPrimary3D::dxfInFields(filer) != Acad::eOk)
		|| !filer->atSubclassData(_T("PDSqucir")))
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
		if (version > VERSION_PDSQUCIR)
			return Acad::eMakeMeProxy;
	}

	if(version >= 2)
	{
		filer->readItem(&rb);
		if (rb.restype != AcDb::kDxfNormalX+2) {
			filer->pushBackItem();
			filer->setError(Acad::eInvalidDxfCode,
				_T("nError: expected object version group code %d"),
				AcDb::kDxfNormalX+2);
			return filer->filerStatus();
		} 
		else {
			m_VectW = asVec3d(rb.resval.rpoint);
		}
	}
	// Read the data members.
	if(version>=1)
	{
		while ((es == Acad::eOk) && ((es = filer->readResBuf(&rb)) == Acad::eOk)){
			switch (rb.restype){
	case AcDb::kDxfXCoord:
		m_ptP1 = asPnt3d(rb.resval.rpoint);
		break;

	case AcDb::kDxfXCoord+1:
		m_ptP2 = asPnt3d(rb.resval.rpoint);
		break;

	case AcDb::kDxfReal:
		m_dLength = rb.resval.rreal;
		break;

	case AcDb::kDxfReal+1:
		m_dWidth = rb.resval.rreal;
		break;

	case AcDb::kDxfReal+2:
		m_dRadius = rb.resval.rreal;
		break;

	case AcDb::kDxfNormalX:
		m_VectCir = asVec3d(rb.resval.rpoint);
		break;

	case AcDb::kDxfNormalX+1:
		m_VectSqu = asVec3d(rb.resval.rpoint);
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
		if(version == 1)
			m_VectW = m_VectCir.crossProduct(m_VectSqu).normalize();
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

const AcGePoint3d & PDSqucir::getRectOrg() const
{
	assertReadEnabled();
	return m_ptP1;
}

const AcGePoint3d & PDSqucir::getCircCenter() const
{
	assertReadEnabled();
	return m_ptP2;
}

double PDSqucir::getLength() const
{
	assertReadEnabled();
	return m_dLength;
}

double PDSqucir::getWidth() const
{
	assertReadEnabled();
	return m_dWidth;
}

double PDSqucir::getRadius() const
{
	assertReadEnabled();
	return m_dRadius;
}

Acad::ErrorStatus PDSqucir::getprecision(Adesk::UInt32& precision)
{
	assertReadEnabled();
	precision =  m_dDividPrecision ;
	return Acad::eOk;
}

const AcGeVector3d & PDSqucir::getVectCir() const
{
	assertReadEnabled();
	return m_VectCir;
}

const AcGeVector3d & PDSqucir::getVectSqu() const
{
	assertReadEnabled();
	return m_VectSqu;
}

const AcGeVector3d & PDSqucir::getVectWidth() const
{
	assertReadEnabled();
	return m_VectW;
}


Acad::ErrorStatus PDSqucir:: setParameters(AcGePoint3d ptP1,AcGePoint3d ptP2,
										   double length,double width,double radius,
										   Adesk::UInt32 precision,AcGeVector3d vectCir,
										   AcGeVector3d vectSqu,int CorrType)

{
	assertWriteEnabled();
	m_ptP1 = ptP1 ;
	m_ptP2 = ptP2 ;
	if(m_ptP1.isEqualTo(m_ptP2))
		m_ptP2=m_ptP1+AcGeVector3d(0,0,1);
	m_dLength =fabs(length)  ;
	if(m_dLength==0)
		m_dLength=1;
	m_dWidth =fabs(width)  ;
	if(m_dWidth==0)
		m_dWidth=1;
	m_dRadius =fabs(radius)  ;
	if(m_dRadius==0)
		m_dRadius=1;
	if(precision > 2)
		m_dDividPrecision = precision;
	else
		m_dDividPrecision = PRIMARY3D_DEFAULT_PRECISION;

	m_VectCir = vectCir;
	m_VectSqu = vectSqu;

	if (vectCir.isZeroLength()&& vectSqu.isZeroLength())
	{
		m_VectCir.set(0,0,1);
		m_VectSqu.set(1,0,0);
	}
	else
		if(vectCir.isZeroLength())
		{
			AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax, Ay;
			if (fabs(vectSqu[X]) <1.0/64 && fabs(vectSqu[Y])<1.0/64) 
				Ax = Wy.crossProduct(vectSqu);
			else
				Ax = Wz.crossProduct(vectSqu);
			Ax.normalize();
			m_VectCir=Ax;
		}
		else
			if(vectSqu.isZeroLength())
			{
				AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax, Ay;
				if (fabs(vectCir[X]) <1.0/64 && fabs(vectCir[Y])<1.0/64) 
					Ax = Wy.crossProduct(vectCir);
				else
					Ax = Wz.crossProduct(vectCir);
				Ax.normalize();
				m_VectSqu=Ax;
			} 
			else
				if(vectCir.isParallelTo(vectSqu))
				{
					AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax, Ay;
					if (fabs(vectCir[X]) <1.0/64 && fabs(vectCir[Y])<1.0/64) 
						Ax = Wy.crossProduct(vectCir);
					else
						Ax = Wz.crossProduct(vectCir);
					Ax.normalize();
					m_VectSqu=Ax;
				}


				if (CorrType)
					m_VectW=m_VectCir.crossProduct(m_VectSqu);
				else
					m_VectW=m_VectSqu.crossProduct(m_VectCir);

#ifdef _USEAMODELER_
				createBody();
#endif
				GetMaxLength();
				return Acad::eOk ;
}//added by linlin 20050929

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDSqucir::subGetOsnapPoints(
	AcDb::OsnapMode     osnapMode,
	Adesk::GsMarker     gsSelectionMark,
	const AcGePoint3d&  pickPoint,
	const AcGePoint3d&  lastPoint,
	const AcGeMatrix3d& viewXform,
	AcGePoint3dArray&   snapPoints,
	AcDbIntArray &   geomIds) const
#else
Acad::ErrorStatus PDSqucir::getOsnapPoints(
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

	AcGePoint3dArray pStdArray;
	AcGePoint3dArray pArray;
	AcGeIntArray stdIdx;
	int actPrecision;
	getVertices(m_dDividPrecision, pArray, stdIdx, actPrecision);
	int actPrecision__1 = actPrecision + 1;
	int stdIdxLen = stdIdx.length();

	int j;
	AcGePoint3d pt;
	for (j=0;j<4; j++)
	{
		for (int i=0; i<stdIdxLen;i++)
		{
			pt = pArray[stdIdx[i]+j*actPrecision__1];
			pStdArray.append(pt);
		}
	}
	for (j=0;j<4;j++)
	{ 
		for (int i=0;i<stdIdxLen;i++)
			pStdArray.append(pArray[stdIdx[i]+j*actPrecision__1+actPrecision__1*4]);
	}


	AcGeVector3d lengthvect = m_VectSqu.normal();
	AcGeVector3d widthvect= m_VectW.normal();

	AcGePoint3d ptsqu[5];
	ptsqu[0] = m_ptP1;
	ptsqu[1] = m_ptP1 + lengthvect * m_dLength;
	ptsqu[2] = ptsqu[1] + widthvect * m_dWidth;
	ptsqu[3] = ptsqu[0] + widthvect * m_dWidth;
	ptsqu[4] = m_ptP1;

	switch(osnapMode)
	{
	case AcDb::kOsModeEnd:
		{
			for (int j=0;j<4;j++)
			{  
				for(int i = 0; i < stdIdxLen; i++)
				{
					snapPoints.append(pArray[stdIdx[i]+actPrecision__1*j]);
				}
			}
			snapPoints.append(ptsqu[0]);
			snapPoints.append(ptsqu[1]);
			snapPoints.append(ptsqu[2]);
			snapPoints.append(ptsqu[3]);
		}
		break;
	case AcDb::kOsModeMid:
		{
			int i;
			for(i = 0; i < 4; i++)
				snapPoints.append(ptsqu[i] + (ptsqu[i + 1] - ptsqu[i]) / 2.0);
			for (int j=0;j<4;j++)
			{
				for(i = 0; i < stdIdxLen; i++)		
					snapPoints.append(pArray[stdIdx[i]+actPrecision__1*j] + (pArray[stdIdx[i]+actPrecision__1*j +actPrecision__1*4] - pArray[stdIdx[i]+actPrecision__1*j]) / 2.0);
			}
		}
		break;
	case AcDb::kOsModeCen:
		snapPoints.append(m_ptP1 + lengthvect * m_dLength / 2.0 + widthvect * m_dWidth / 2.0);
		snapPoints.append(m_ptP2);
		break;
	case AcDb::kOsModeQuad:
		{
			for(int i = 0; i < 4; i++)
				snapPoints.append(pArray[stdIdx[0]+actPrecision__1*i]);
		}
		break;
	case AcDb::kOsModeNode:
		break;
	case AcDb::kOsModeIns:
		snapPoints.append(m_ptP1);
		break;
	case AcDb::kOsModePerp:
		{
			AcGePoint3d pt;
			AcGeLine3d line;
			//重新定义对象垂直正交点的捕捉方式，同时满足实体模型和线框模型的捕捉 
			//modified by szw 2009.11.18 : begin
			if(gsSelectionMark_int == 1)
			{
				for(int i = 0; i < 4; ++i)
				{
					line.set(ptsqu[i], ptsqu[i + 1]);
					pt = line.closestPointTo(lastPoint);
					snapPoints.append(pt);
				}
			}
			else if(gsSelectionMark_int == 2)
			{
				AcGeCircArc3d cir(m_ptP2, m_VectCir.normal(), m_dRadius);
				pt = cir.closestPointTo(lastPoint);
				snapPoints.append(pt);
			}
			else
			{
				for(int i = 0; i < stdIdxLen; ++i)
				{
					line.set(pStdArray[i + (gsSelectionMark_int - 3) * stdIdxLen],
						pStdArray[i + (gsSelectionMark_int - 3) * stdIdxLen + pStdArray.length() / 2]);
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
			AcGePoint3d pt;
			AcGeLine3d lineSeg;
			//重新定义对象最近点的捕捉方式，同时满足实体模型和线框模型的捕捉 
			//modified by szw 2009.11.18 : begin
			if(gsSelectionMark_int == 1)
			{
				for(int i = 0; i < 4; ++i)
				{
					lineSeg.set(ptsqu[i], ptsqu[i + 1]);
					pt = lineSeg.projClosestPointTo(pickPoint, viewDir);
					snapPoints.append(pt);
				}
			}
			else if(gsSelectionMark_int == 2)
			{
				AcGeCircArc3d cir(m_ptP2, m_VectCir.normal(), m_dRadius);
				pt = cir.projClosestPointTo(pickPoint, viewDir);
				snapPoints.append(pt);
			}
			else
			{
				for(int i = 0; i < stdIdxLen; ++i)
				{
					lineSeg.set(pStdArray[i + (gsSelectionMark_int - 3) * stdIdxLen], 
						pStdArray[i + (gsSelectionMark_int - 3) * stdIdxLen + pStdArray.length() / 2]);
					pt = lineSeg.projClosestPointTo(pickPoint, viewDir);
					snapPoints.append(pt);
				}
			}
			//modified by szw 2009.11.18 : end
		}
		break;
	default:
		break;
	}

	return Acad::eOk;
}//added by linlin 20050818


#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDSqucir::subGetGripPoints(
	AcGePoint3dArray& gripPoints,
	AcDbIntArray& osnapModes,
	AcDbIntArray& geomIds) const
#else
Acad::ErrorStatus PDSqucir::getGripPoints(
	AcGePoint3dArray& gripPoints,
	AcDbIntArray& osnapModes,
	AcDbIntArray& geomIds) const
#endif
{
	assertReadEnabled();

	/*	getgrippoints(gripPoints);*/
	return Acad::eOk; 
}


Acad::ErrorStatus PDSqucir::getgrippoints(AcGePoint3dArray& gripArray) const
{
	AcGeVector3d lengthvect = m_VectSqu;
	AcGeVector3d widthvect= m_VectW.normal();

	////////////////////////////////////////////////////////
	//////	矩形的四个角点
	gripArray.append(m_ptP1);
	gripArray.append(m_ptP1 + lengthvect * m_dLength);
	gripArray.append(m_ptP1 + lengthvect * m_dLength + widthvect * m_dWidth);
	gripArray.append(m_ptP1 + widthvect * m_dWidth);

	/////////////////////////////////////////////////////////
	/////	圆的圆心
	gripArray.append(m_ptP2);

	/////////////////////////////////////////////////////////
	/////	圆的四个四分之一点

	AcGeCubicSplineCurve3d circur = AcGeCubicSplineCurve3d(AcGeCircArc3d(
		m_ptP2,m_VectCir,-lengthvect,m_dRadius,0,2*PI));

	double parameter;
	AcGePoint3d pt1;
	for(int i =0;i<4;i++){
		parameter = PI/2*i;
		pt1 = circur.evalPoint(parameter);
		gripArray.append(pt1);
	}

	return Acad::eOk; 
}


#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDSqucir::subMoveGripPointsAt(
	const AcDbIntArray& indices,
	const AcGeVector3d& offset)
#else
Acad::ErrorStatus PDSqucir::moveGripPointsAt(
	const AcDbIntArray& indices,
	const AcGeVector3d& offset)
#endif
{
	assertWriteEnabled();
	/*	if( (indices.length()>1) || (indices[0] <4) ){
	m_ptP1.transformBy(offset);
	m_ptP2.transformBy(offset);
	m_VectCir.transformBy(offset);
	m_VectSqu.transformBy(offset);
	return Acad::eOk;
	}
	if(indices[0] <5 )
	m_ptP2.transformBy(offset);
	else{
	AcGePoint3dArray grippoint;
	getgrippoints(grippoint);
	grippoint[indices[0]].transformBy(offset);
	m_dRadius = grippoint[indices[0]].distanceTo(m_ptP2);
	}*/
	return Acad::eOk;
}

/*
Acad::ErrorStatus PDSqucir::explode(AcDbVoidPtrArray& entitySet) const
{
assertReadEnabled();
Acad::ErrorStatus es = Acad::eOk;
int i;

AcGePoint3d P1;
AcGeVector3d lengthvect; 
AcGeVector3d Vcir; 
AcGeVector3d widthvect; 
double Length;
double Width;
getRightHandCoord(P1,lengthvect,Vcir,widthvect,Length,Width);

AcGePoint3dArray pArray;
AcGeIntArray stdIdx;
int actPrecision;
getVertices(m_dDividPrecision, pArray, stdIdx, actPrecision);
int  actPrecision__1=actPrecision+1;

AcGeVector3d faceVect =  m_VectCir;

AcDbVoidPtrArray curSeg, resRegion;
AcDbCircle *pCir = NULL;
AcGePoint3d  ucs_org = acdbCurDwg()->ucsorg();
AcGeVector3d ucs_x = acdbCurDwg()->ucsxdir();
AcGeVector3d ucs_y = acdbCurDwg()->ucsydir();
AcGeVector3d ucs_z = ucs_x.crossProduct(ucs_y);
AcGeMatrix3d ucs_mat;

ucs_mat.setToWorldToPlane(-faceVect);
//#ifdef _OBJECTARX14_
//    acdbSetCurrentUCS(ucs_mat);
//#else
//    acedSetCurrentUCS(ucs_mat);
//#endif
pCir = new AcDbCircle(m_ptP2, -faceVect, m_dRadius);
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

resRegion.setLogicalLength(0);

ucs_mat.setToWorldToPlane(faceVect);
//#ifdef _OBJECTARX14_
//    acdbSetCurrentUCS(ucs_mat);
//#else
//    acedSetCurrentUCS(ucs_mat);
//#endif
AcGePoint3d point1, point2, point3, point4;

point1 = m_ptP1;
point2 = point1 + lengthvect * Length;
point3 = point2 + widthvect * Width;
point4 = point1 + widthvect * Width;

AcDbLine line1(point1, point2);
AcDbLine line2(point2, point3);
AcDbLine line3(point3, point4);
AcDbLine line4(point4, point1);

AcDbVoidPtrArray curveSegments, resRegions;

curveSegments.append(&line1);
curveSegments.append(&line2);
curveSegments.append(&line3);
curveSegments.append(&line4);

es = AcDbRegion::createFromCurves(curveSegments, resRegions);
if(es != Acad::eOk)
{
for(i = 0; i < resRegions.length(); ++i)
delete (AcRxObject*)resRegions[i];
}
else
{
for(i = 0; i < resRegions.length(); ++i)
((AcDbEntity *)resRegions[i])->setPropertiesFrom(this);
entitySet.append(resRegions);
}

ucs_mat.setCoordSystem(ucs_org, ucs_x, ucs_y, ucs_z);

//#ifdef _OBJECTARX14_
//    acdbSetCurrentUCS(ucs_mat);
//#else
//    acedSetCurrentUCS(ucs_mat);
//#endif

AcDbPolygonMesh *pMesh;
pMesh = new AcDbPolygonMesh(AcDb::kSimpleMesh, 2, actPrecision__1*4,
pArray, Adesk::kFalse, Adesk::kFalse);
pMesh->setPropertiesFrom(this);
entitySet.append(pMesh);

return Acad::eOk;
}//added by  linlin 20050920
*/

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDSqucir::subExplode(AcDbVoidPtrArray& entitySet) const
#else
Acad::ErrorStatus PDSqucir::explode(AcDbVoidPtrArray& entitySet) const
#endif
{
	assertReadEnabled();
	Acad::ErrorStatus es = Acad::eOk;

#ifdef _OBJECTARX2004_
	//用布尔运算做天方地圆
	AcGePoint3d squCen = m_ptP1;
	squCen = squCen+m_dLength*m_VectSqu/2+m_dWidth*m_VectW/2;
	AcGeVector3d strechVec = (m_ptP2-squCen).normal();
	double pathLen = m_ptP2.distanceTo(squCen);

	double bottomRadius = min(m_dRadius, sqrt(m_dLength*m_dLength+m_dWidth*m_dWidth)/2);
	double topRadius = max(m_dRadius, sqrt(m_dLength*m_dLength+m_dWidth*m_dWidth)/2);

	AcGePoint3d ptorign(0,0,0);
	AcGeVector3d vectY(0,1,0);
	AcGeVector3d vectX(1,0,0);
	AcGeVector3d vectZ(0,0,1);

	//path
	//从小圆拉伸成大圆，求拉伸路径
	if (fabs(bottomRadius-m_dRadius) < SIGMA)//下圆上方
	{
		strechVec = (squCen-m_ptP2).normal();
		AcGeVector3d vectw = (-m_VectCir).crossProduct(m_VectSqu).normal();
		AcGeMatrix3d mat;
		mat.setToAlignCoordSys(squCen, m_VectSqu, vectw, -m_VectCir,
			ptorign, vectX, vectY, vectZ);
		strechVec.transformBy(mat);
	} 
	else//下方上圆
	{
		AcGeVector3d vectw = m_VectCir.crossProduct(m_VectSqu).normal();
		AcGeMatrix3d mat;
		mat.setToAlignCoordSys(squCen, m_VectSqu, vectw, m_VectCir,
			ptorign, vectX, vectY, vectZ);
		strechVec.transformBy(mat);
	}

	AcDbCurve *pCurve = NULL;
	AcGePoint3d endPoint = ptorign + strechVec*pathLen;
	AcDbLine *pLine = new AcDbLine(ptorign, endPoint);
	pLine->setPropertiesFrom(this);
	pCurve = pLine;	

	int i=0;
	if(pCurve != NULL)
	{
		if((es = addToDb(pCurve)) == Acad::eOk)
		{
			AcDbObjectId id;
			id = pCurve->objectId();
			pCurve->close();
			if((es = acdbOpenObject((AcDbCurve*&)pCurve, id, AcDb::kForRead)) != Acad::eOk)
				return Acad::eNotApplicable;
		}
		else
		{
			if(pCurve != NULL)
				delete pCurve;
			return Acad::eNotApplicable;
		}
	}

	//curve
	AcDbVoidPtrArray curveSegments;
	AcDbCircle sCir(ptorign, vectZ, bottomRadius);
	curveSegments.append(&sCir);

	AcDbVoidPtrArray regions;
	es = AcDbRegion::createFromCurves(curveSegments, regions);

	if(es == Acad::eOk && !regions.isEmpty())
	{
		AcDb3dSolid *pBody;
		pBody = new AcDb3dSolid;

		es = pBody->extrudeAlongPath((AcDbRegion*)(regions[0]), pCurve,
			-1*atan((topRadius-bottomRadius) / pathLen));

		if(es != Acad::eOk)
		{
			if(pCurve != NULL)
			{
				pCurve->upgradeOpen();
				pCurve->erase();
				pCurve->close();
			}
			delete pBody;
			for(i = 0; i < regions.length(); i++)
				delete (AcRxObject*)regions[i];
			return Acad::eNotApplicable;
		}

		AcGePoint3d cp1(bottomRadius,0,0);
		AcGePoint3d cp2(0,bottomRadius,0);
		AcGePoint3d cp3(-bottomRadius,0,0);
		AcGePoint3d cp4(0,-bottomRadius,0);

		AcGePoint3d sp1 = endPoint+vectX*m_dLength/2-vectY*m_dWidth/2;
		AcGePoint3d sp2 = sp1 + vectY*m_dWidth;
		AcGePoint3d sp3 = sp2 - vectX*m_dLength;
		AcGePoint3d sp4 = sp3 - vectY*m_dWidth;

		if (fabs(bottomRadius-m_dRadius) > SIGMA)////上圆下方
		{
			sp1 = ptorign+vectX*m_dLength/2-vectY*m_dWidth/2;
			sp2 = cp1 + vectY*m_dWidth;
			sp3 = cp2 - vectX*m_dLength;
			sp4 = cp3 - vectY*m_dWidth;

			cp1 = endPoint+vectX*m_dRadius;
			cp2 = endPoint+vectY*m_dRadius;
			cp3 = endPoint-vectX*m_dRadius;
			cp4 = endPoint-vectY*m_dRadius;
		}

		AcGePlane slicePlane;
		AcDb3dSolid *pHalfSolid = NULL;
		slicePlane.set(cp1, -((sp2 - cp1).crossProduct(sp1 - cp1)));
		pBody->getSlice(slicePlane, Adesk::kFalse, pHalfSolid);
		slicePlane.set(cp2, -((sp3 - cp2).crossProduct(sp2 - cp2)));
		pBody->getSlice(slicePlane, Adesk::kFalse, pHalfSolid);
		slicePlane.set(cp3, -((sp4 - cp3).crossProduct(sp3 - cp3)));
		pBody->getSlice(slicePlane, Adesk::kFalse, pHalfSolid);
		slicePlane.set(cp4, -((sp1 - cp4).crossProduct(sp4 - cp4)));
		pBody->getSlice(slicePlane, Adesk::kFalse, pHalfSolid);

		//变换到原坐标系
		AcGeMatrix3d mat;

		if (fabs(bottomRadius-m_dRadius) > SIGMA)//上圆下方
		{
			AcGeVector3d vectw = m_VectCir.crossProduct(m_VectSqu).normal();
			AcGePoint3d pp = m_ptP1-m_dLength*m_VectSqu/2-m_dWidth*vectw/2;
			mat.setCoordSystem(pp, m_VectSqu, vectw, m_VectCir);
		}
		else//下圆上方
		{
			AcGeVector3d vectw = (-m_VectCir).crossProduct(m_VectSqu).normal();
			mat.setCoordSystem(m_ptP2, m_VectSqu, vectw, -m_VectCir);
		}

		pBody->transformBy(mat);

		pBody->setPropertiesFrom(this);
		entitySet.append(pBody);
	}
	else
	{
		if(pCurve != NULL)
		{
			pCurve->upgradeOpen();
			pCurve->erase();
			pCurve->close();
		}
		for(i = 0; i < regions.length(); i++)
			delete (AcRxObject*)regions[i];
		return Acad::eNotApplicable;
	}

	if(pCurve != NULL)
	{
		pCurve->upgradeOpen();
		pCurve->erase();
		pCurve->close();
	}
	for(i = 0; i < regions.length(); i++)
		delete (AcRxObject*)regions[i];
#endif
	return Acad::eOk;
}

//  [9/5/2007 suzhiyong]
Acad::ErrorStatus PDSqucir::explodeTo3DSolid(AcDb3dSolid* &p3dSolid) const
{
	assertReadEnabled();
	Acad::ErrorStatus es = Acad::eOk;

#ifdef _OBJECTARX2004_
	//用布尔运算做天方地圆
	AcGePoint3d squCen = m_ptP1;
	squCen = squCen+m_dLength*m_VectSqu/2+m_dWidth*m_VectW/2;
	AcGeVector3d strechVec = (m_ptP2-squCen).normal();
	double pathLen = m_ptP2.distanceTo(squCen);

	double bottomRadius = min(m_dRadius, sqrt(m_dLength*m_dLength+m_dWidth*m_dWidth)/2);
	double topRadius = max(m_dRadius, sqrt(m_dLength*m_dLength+m_dWidth*m_dWidth)/2);

	AcGePoint3d ptorign(0,0,0);
	AcGeVector3d vectY(0,1,0);
	AcGeVector3d vectX(1,0,0);
	AcGeVector3d vectZ(0,0,1);

	//path
	//从小圆拉伸成大圆，求拉伸路径
	if (fabs(bottomRadius-m_dRadius) < SIGMA)//下圆上方
	{
		strechVec = (squCen-m_ptP2).normal();
		AcGeVector3d vectw = (-m_VectCir).crossProduct(m_VectSqu).normal();
		AcGeMatrix3d mat;
		mat.setToAlignCoordSys(squCen, m_VectSqu, vectw, -m_VectCir,
			ptorign, vectX, vectY, vectZ);
		strechVec.transformBy(mat);
	} 
	else//下方上圆
	{
		AcGeVector3d vectw = m_VectCir.crossProduct(m_VectSqu).normal();
		AcGeMatrix3d mat;
		mat.setToAlignCoordSys(squCen, m_VectSqu, vectw, m_VectCir,
			ptorign, vectX, vectY, vectZ);
		strechVec.transformBy(mat);
	}

	AcDbCurve *pCurve = NULL;
	AcGePoint3d endPoint = ptorign + strechVec*pathLen;
	AcDbLine *pLine = new AcDbLine(ptorign, endPoint);
	pLine->setPropertiesFrom(this);
	pCurve = pLine;	

	int i=0;
	if(pCurve != NULL)
	{
		if((es = addToDb(pCurve)) == Acad::eOk)
		{
			AcDbObjectId id;
			id = pCurve->objectId();
			pCurve->close();
			if((es = acdbOpenObject((AcDbCurve*&)pCurve, id, AcDb::kForRead)) != Acad::eOk)
				return Acad::eNotApplicable;
		}
		else
		{
			if(pCurve != NULL)
				delete pCurve;
			return Acad::eNotApplicable;
		}
	}

	//curve
	AcDbVoidPtrArray curveSegments;
	AcDbCircle sCir(ptorign, vectZ, bottomRadius);
	curveSegments.append(&sCir);

	AcDbVoidPtrArray regions;
	es = AcDbRegion::createFromCurves(curveSegments, regions);

	if(es == Acad::eOk && !regions.isEmpty())
	{
		AcDb3dSolid *pBody;
		pBody = new AcDb3dSolid;

		es = pBody->extrudeAlongPath((AcDbRegion*)(regions[0]), pCurve,
			-1*atan((topRadius-bottomRadius) / pathLen));

		if(es != Acad::eOk)
		{
			if(pCurve != NULL)
			{
				pCurve->upgradeOpen();
				pCurve->erase();
				pCurve->close();
			}
			delete pBody;
			for(i = 0; i < regions.length(); i++)
				delete (AcRxObject*)regions[i];
			return Acad::eNotApplicable;
		}

		AcGePoint3d cp1(bottomRadius,0,0);
		AcGePoint3d cp2(0,bottomRadius,0);
		AcGePoint3d cp3(-bottomRadius,0,0);
		AcGePoint3d cp4(0,-bottomRadius,0);

		AcGePoint3d sp1 = endPoint+vectX*m_dLength/2-vectY*m_dWidth/2;
		AcGePoint3d sp2 = sp1 + vectY*m_dWidth;
		AcGePoint3d sp3 = sp2 - vectX*m_dLength;
		AcGePoint3d sp4 = sp3 - vectY*m_dWidth;

		if (fabs(bottomRadius-m_dRadius) > SIGMA)////上圆下方
		{
			sp1 = ptorign+vectX*m_dLength/2-vectY*m_dWidth/2;
			sp2 = cp1 + vectY*m_dWidth;
			sp3 = cp2 - vectX*m_dLength;
			sp4 = cp3 - vectY*m_dWidth;

			cp1 = endPoint+vectX*m_dRadius;
			cp2 = endPoint+vectY*m_dRadius;
			cp3 = endPoint-vectX*m_dRadius;
			cp4 = endPoint-vectY*m_dRadius;
		}

		AcGePlane slicePlane;
		AcDb3dSolid *pHalfSolid = NULL;
		slicePlane.set(cp1, -((sp2 - cp1).crossProduct(sp1 - cp1)));
		pBody->getSlice(slicePlane, Adesk::kFalse, pHalfSolid);
		slicePlane.set(cp2, -((sp3 - cp2).crossProduct(sp2 - cp2)));
		pBody->getSlice(slicePlane, Adesk::kFalse, pHalfSolid);
		slicePlane.set(cp3, -((sp4 - cp3).crossProduct(sp3 - cp3)));
		pBody->getSlice(slicePlane, Adesk::kFalse, pHalfSolid);
		slicePlane.set(cp4, -((sp1 - cp4).crossProduct(sp4 - cp4)));
		pBody->getSlice(slicePlane, Adesk::kFalse, pHalfSolid);

		//变换到原坐标系
		AcGeMatrix3d mat;

		if (fabs(bottomRadius-m_dRadius) > SIGMA)//上圆下方
		{
			AcGeVector3d vectw = m_VectCir.crossProduct(m_VectSqu).normal();
			AcGePoint3d pp = m_ptP1-m_dLength*m_VectSqu/2-m_dWidth*vectw/2;
			mat.setCoordSystem(pp, m_VectSqu, vectw, m_VectCir);
		}
		else//下圆上方
		{
			AcGeVector3d vectw = (-m_VectCir).crossProduct(m_VectSqu).normal();
			mat.setCoordSystem(m_ptP2, m_VectSqu, vectw, -m_VectCir);
		}

		pBody->transformBy(mat);

		pBody->setPropertiesFrom(this);

		p3dSolid = pBody;
	}
	else
	{
		if(pCurve != NULL)
		{
			pCurve->upgradeOpen();
			pCurve->erase();
			pCurve->close();
		}
		for(i = 0; i < regions.length(); i++)
			delete (AcRxObject*)regions[i];
		return Acad::eNotApplicable;
	}

	if(pCurve != NULL)
	{
		pCurve->upgradeOpen();
		pCurve->erase();
		pCurve->close();
	}
	for(i = 0; i < regions.length(); i++)
		delete (AcRxObject*)regions[i];
#endif
	return Acad::eOk;
}

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDSqucir::subGetGeomExtents(AcDbExtents& extents) const
#else
Acad::ErrorStatus PDSqucir::getGeomExtents(AcDbExtents& extents) const
#endif
{
	assertReadEnabled();
	extents.set(m_ptP1, m_ptP1);
	AcGePoint3dArray pArray;
	AcGeIntArray stdIdx;
	int actPrecision;
	getVertices(m_dDividPrecision, pArray, stdIdx, actPrecision);
	int i, arrlen = pArray.length();
	for(i = 0; i < arrlen; ++i)
		extents.addPoint(pArray[i]);
	return Acad::eOk;
}

#ifdef _OBJECTARX2010_
void PDSqucir::subList() const
#else
void PDSqucir::list() const
#endif
{
	assertReadEnabled();
#ifdef _OBJECTARX2010_
	PDPrimary3D::subList();
#else
	PDPrimary3D::list();
#endif
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4732/*"\n 插入点(%f,%f,%f)\n"*/,m_ptP1.x,m_ptP1.y,m_ptP1.z);
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4796/*" 矩形的长度=%.3f 矩形的宽度=%.3f\n"*/,m_dLength,m_dWidth);
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4797/*" 矩形长轴方向(%f,%f,%f)\n"*/,m_VectSqu.x,m_VectSqu.y,m_VectSqu.z);
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4798/*" 地圆的圆心(%f,%f,%f)\n"*/,m_ptP2.x,m_ptP2.y,m_ptP2.z);
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4799/*" 地圆的半径=%.3f\n"*/,m_dRadius);
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4800/*" 天方与地圆所在平面的法向量(%f,%f,%f)\n\n"*/,m_VectCir.x,m_VectCir.y,m_VectCir.z);
	return ;
}//added by linin 20050810

Acad::ErrorStatus PDSqucir::getRightHandCoord(AcGePoint3d &p1, 
											  AcGeVector3d &vlength, 
											  AcGeVector3d &vcir, 
											  AcGeVector3d &vwidth, 
											  double &length, 
											  double &width) const
{
	assertReadEnabled();
	p1=m_ptP1;
	vlength=m_VectSqu;
	vcir=m_VectCir;
	vwidth=m_VectW;
	length=m_dLength;
	width=m_dWidth;
	AcGeVector3d y;
	double ang;
	y = m_VectCir.crossProduct(m_VectSqu).normal();
	ang = y.angleTo(m_VectW, m_VectSqu);
	if(ang > PI / 2.0 && ang < PI * 3.0 / 2.0)
	{  
		length = m_dWidth;
		width = m_dLength;
		vlength=m_VectW;
		vwidth=m_VectSqu;
	}
	return Acad::eOk;
}//added by linlin 20050829

PDSqucir::SquCirInfo PDSqucir::getSquCirInfo() const
{
	assertReadEnabled();

	SquCirInfo result;
	result.cirCen = m_ptP2;
	AcGePoint3d p1;
	AcGeVector3d vwidth;

	getRightHandCoord(p1, result.vLength, result.normal, vwidth, result.length, result.width);

	result.squCen = p1 + result.vLength*(result.length/2);
	result.squCen = result.squCen + vwidth*(result.width/2);

	result.midCen.set((result.cirCen.x+result.squCen.x)/2, (result.cirCen.y+result.squCen.y)/2, (result.cirCen.z+result.squCen.z)/2);

	result.radius = m_dRadius;

	result.orhtpt = result.cirCen.orthoProject(AcGePlane(p1, result.normal));

	return result;
}

void PDSqucir::getActDivid(int Precision, AcGeDoubleArray &dividDbl, AcGeIntArray &stdIdx) const
{
	assertReadEnabled();

	int precisionTemp = Precision;
	// 如果剖分进度小于2，则强制将剖分精度设置为2
	if(precisionTemp < 2)
		precisionTemp = 2;
	int precision__1 = precisionTemp + 1;

	dividDbl.setLogicalLength(0);
	stdIdx.setLogicalLength(3);

	int divid_2_num = precisionTemp / 2;
	if(precisionTemp % 2 == 0)
	{
		int i;
		for(i = 0; i < 3; ++i)
			stdIdx[i] = i * divid_2_num;
		for(i = 0; i < precision__1; ++i)
			dividDbl.append(i);
	}
	else
	{
		double insertIdxDbl[3];
		double divid_seg = precisionTemp % 2 / 2.0;
		int i;
		for(i = 0; i < 3; ++i)
			insertIdxDbl[i] = i * divid_2_num + i * divid_seg;
		int insertIdxDbl_i = 0;
		for(i = 0; i < precision__1; ++i)
		{
			dividDbl.append(i);
			if(insertIdxDbl_i > 2)
				continue;
			if(insertIdxDbl[insertIdxDbl_i] == i)
			{
				stdIdx[insertIdxDbl_i] = dividDbl.length() - 1;
				++insertIdxDbl_i;
			}
			else if(insertIdxDbl[insertIdxDbl_i] > i && insertIdxDbl[insertIdxDbl_i] < i + 1)
			{
				dividDbl.append(insertIdxDbl[insertIdxDbl_i]);
				stdIdx[insertIdxDbl_i] = dividDbl.length() - 1;
				++insertIdxDbl_i;
			}
		}
	}

	return ;
}//added by linlin 20050818

void PDSqucir::createBody()
{
	assertWriteEnabled();
	// #ifdef _OBJECTARX2004_
	AcGePoint3d ori, t1, t2;
	AcGeVector3d Vectx, Vecty, Vectz;
	double length, width;
	getRightHandCoord(ori, Vectx, Vectz, Vecty, length, width);
	AcGeMatrix3d mat;
	mat.setCoordSystem(ori, Vectx, Vecty, Vectz);

	Vector2d GeomVec(length, width);
	t1 = mat.inverse()*m_ptP2;
	t2 = t1 + m_VectCir;

	m_3dGeom = Body::rectangleToCircleReducer(Point3d(0,0,0),  GeomVec,
		Circle3d( Line3d(t1, t2), m_dRadius),
		m_dDividPrecision*4);	
	m_3dGeom.transform(*(Transf3d*)&mat);
	// #endif
}
double PDSqucir::GetMaxLength()
{
	assertWriteEnabled();
	m_dMaxLength = m_ptP1.distanceTo(m_ptP2);
	if(m_dMaxLength < m_dLength)
		m_dMaxLength = m_dLength;
	if(m_dMaxLength < m_dWidth)
		m_dMaxLength = m_dWidth;
	if(m_dMaxLength < m_dRadius *2)
		m_dMaxLength = m_dRadius *2;
	return m_dMaxLength;
}
