// PDRevolve.cpp: implementation of the PDRevolve class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PDRevolve.h"

#include <math.h>

#include <dbproxy.h>
#include <gemat3d.h>
#include <Gedwgio.h>
#include <Gedxfio.h>
#include <acgi.h>
#include <dbents.h>
#include <dbregion.h>
#include <dbsol3d.h>
#include <dbapserv.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define VERSION_PDREVOLVE 1

#define PI 3.1415926535897932384626433832795L

#ifdef _OBJECTARX2000_
ACRX_DXF_DEFINE_MEMBERS(PDRevolve, PDPrimary3D,
                        AcDb::kDHL_CURRENT, AcDb::kMReleaseCurrent,
                        AcDbProxyEntity::kAllAllowedBits,
                        PDREVOLVE, Gle);
#else
ACRX_DXF_DEFINE_MEMBERS(PDRevolve, PDPrimary3D, AcDbProxyEntity::kAllAllowedBits, PDREVOLVE, Gle);
#endif

void PDRevolve::setDefault()
{
  PDPrimary3D::setDefault();
#if defined(NDEBUG) || !defined(_DEBUG)
  m_angle = 2.0 * PI;
  GetCirclePtsAndMaxRadius();
#else
  m_ptAxis.set(0,0,1);
  m_vecAxis.set(0,1,0);
  m_angle = 2.0 * PI;
  m_polylineVertex.clear();
  GetCirclePtsAndMaxRadius();
#endif
}
bool PDRevolve::isValidData(double &f)
{
  //m_polylineVertex未检查
  if(!PDPrimary3D::isValidData(f))
    return false;
  if(!isValidFloat(f=m_angle))
    return false;
  if(!isValidFloat(f=m_ptAxis.x) || !isValidFloat(f=m_ptAxis.y) || !isValidFloat(f=m_ptAxis.z))
    return false;
  if(!isValidFloat(f=m_vecAxis.x) || !isValidFloat(f=m_vecAxis.y) || !isValidFloat(f=m_vecAxis.z))
    return false;
  return true;
}

//zxb,20090113,增加audit，暂时只check自身成员数据
Acad::ErrorStatus PDRevolve::audit(AcDbAuditInfo* pAuditInfo)
{
  return auditPDPrimary3D(this,pAuditInfo,_T("PDRevolve"));
}

PDRevolve::PDRevolve()
{
	m_angle = 2.0 * PI;
	GetCirclePtsAndMaxRadius();
}

PDRevolve::PDRevolve(const AcGePoint3d& AxisPt, const AcGeVector3d& AxisVec, 
					 const std::vector<t_PolylineVertex>& PtArr, int Precision/* = PRIMARY3D_DEFAULT_PRECISION*/, bool HasSnap /*= false*/) : PDPrimary3D(HasSnap)
{
	if(Precision > 2)
		m_dDividPrecision = Precision;
	else
		m_dDividPrecision = PRIMARY3D_DEFAULT_PRECISION;

	m_ptAxis = AxisPt;
	m_vecAxis = AxisVec.normal();
	m_angle = 2.0 * PI;
	m_polylineVertex = PtArr;

#ifdef _USEAMODELER_
	createBody();
#endif
	GetCirclePtsAndMaxRadius();
}

PDRevolve::~PDRevolve()
{
}

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDRevolve::subTransformBy(const AcGeMatrix3d& xform)
#else
Acad::ErrorStatus PDRevolve::transformBy(const AcGeMatrix3d& xform)
#endif
{
	assertWriteEnabled();

	m_ptAxis.transformBy(xform);
	m_vecAxis.transformBy(xform).normalize();
    for(size_t i = 0; i < m_polylineVertex.size(); i++)
        m_polylineVertex[i].m_vertex.transformBy(xform);

#ifdef _OBJECTARX2010_
	return PDPrimary3D::subTransformBy(xform);
#else
	return PDPrimary3D::transformBy(xform);
#endif
}

Acad::ErrorStatus PDRevolve::dwgOutFields(AcDbDwgFiler* filer)const
{
    assertReadEnabled();
	Acad::ErrorStatus es=Acad::eOk;

	// Call dwgOutFields from father class: PDPrimary3D
	if((es=PDPrimary3D::dwgOutFields(filer))!=Acad::eOk)
	{
		return es;
	}

	// Write version number.
	filer->writeItem((Adesk::UInt16) VERSION_PDREVOLVE);

	// Write the data members.
	filer->writeItem(m_ptAxis);
	filer->writeItem(m_vecAxis);
	filer->writeItem(m_angle);

    filer->writeItem((Adesk::Int32)(m_polylineVertex.size()));
    for(size_t i = 0; i < m_polylineVertex.size(); i++)
	{
        filer->writeItem(m_polylineVertex[i].m_vertexFlag);
		filer->writeItem(m_polylineVertex[i].m_vertex);
	}

	return filer->filerStatus();
}

Acad::ErrorStatus PDRevolve::dwgInFields(AcDbDwgFiler* filer)
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
	if (version > VERSION_PDREVOLVE)
		return Acad::eMakeMeProxy;

	// Read the data members.
	if(version >= 1)
    {
		filer->readItem(&m_ptAxis);
		filer->readItem(&m_vecAxis);
		filer->readItem(&m_angle);

		while(!m_polylineVertex.empty())
			m_polylineVertex.pop_back();
		Adesk::Int32 len;
		filer->readItem(&len);
		for(Adesk::Int32 i = 0; i < len; i++)
        {
			t_PolylineVertex ptVertex;
			filer->readItem(&ptVertex.m_vertexFlag);
			filer->readItem(&ptVertex.m_vertex);
			m_polylineVertex.push_back(ptVertex);
        }
	}

#ifdef _USEAMODELER_
	if(m_3dGeom.isNull())
		createBody();
#endif
	GetCirclePtsAndMaxRadius();
	return filer->filerStatus();
}


Acad::ErrorStatus PDRevolve::dxfOutFields(AcDbDxfFiler* filer) const
{
	assertReadEnabled();
	Acad::ErrorStatus es;

	if ((es = PDPrimary3D::dxfOutFields(filer))
		!= Acad::eOk)
	{
		return es;
	}

	// Write subclass marker.
	filer->writeItem(AcDb::kDxfSubclass, _T("PDRevolve"));

	// Write version number.
	filer->writeItem(AcDb::kDxfInt16, (Adesk::UInt16) VERSION_PDREVOLVE);

	// Write data members.
	filer->writeItem(AcDb::kDxfXCoord, m_ptAxis);
	filer->writeItem(AcDb::kDxfXCoord + 1, m_vecAxis);
	filer->writeItem(AcDb::kDxfReal, m_angle);

	filer->writeItem(AcDb::kDxfInt32, (Adesk::Int32)(m_polylineVertex.size()));
	for(size_t i = 0; i < m_polylineVertex.size(); i++)
	{
		filer->writeItem(AcDb::kDxfInt16, m_polylineVertex[i].m_vertexFlag);
		filer->writeItem(AcDb::kDxfXCoord, m_polylineVertex[i].m_vertex);
	}

    return filer->filerStatus();
}

Acad::ErrorStatus PDRevolve::dxfInFields(AcDbDxfFiler* filer)
{
	assertWriteEnabled();
	Acad::ErrorStatus es = Acad::eOk;
	struct resbuf rb;

	if ((PDPrimary3D::dxfInFields(filer) != Acad::eOk)
		|| !filer->atSubclassData(_T("PDRevolve")))
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
		if (version > VERSION_PDREVOLVE)
			return Acad::eMakeMeProxy;
	}

	// Read the data members.
	if(version >= 1)
    {
		filer->readItem(&rb);
		if (rb.restype == AcDb::kDxfXCoord)
		{
			m_ptAxis.set(rb.resval.rpoint[0], rb.resval.rpoint[1], rb.resval.rpoint[2]);
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
			m_vecAxis.set(rb.resval.rpoint[0], rb.resval.rpoint[1], rb.resval.rpoint[2]);
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
		if (rb.restype == AcDb::kDxfReal)
		{
			m_angle = rb.resval.rreal;
		}
		else
		{
			filer->pushBackItem();
			filer->setError(Acad::eInvalidDxfCode,
				_T("\nError: expected group code %d"),
				AcDb::kDxfReal);
			return filer->filerStatus();
		}

		while(!m_polylineVertex.empty())
			m_polylineVertex.pop_back();
		Adesk::Int32 len;
		filer->readItem(&rb);
		if (rb.restype == AcDb::kDxfInt32)
		{
			len = rb.resval.rlong;
		}
		else
		{
			filer->pushBackItem();
			filer->setError(Acad::eInvalidDxfCode,
				_T("\nError: expected group code %d"),
				AcDb::kDxfInt32);
			return filer->filerStatus();
		}
		for(Adesk::Int32 i = 0; i < len; i++)
        {
			t_PolylineVertex ptVertex;

			filer->readItem(&rb);
			if (rb.restype != AcDb::kDxfInt16) {
				filer->pushBackItem();
				filer->setError(Acad::eInvalidDxfCode,
					_T("nError: expected object version group code %d"),
					AcDb::kDxfInt16);
				return filer->filerStatus();
			} 
			else {
				ptVertex.m_vertexFlag = rb.resval.rint;
			}

			filer->readItem(&rb);
			if (rb.restype == AcDb::kDxfXCoord)
			{
				ptVertex.m_vertex.set(rb.resval.rpoint[0], rb.resval.rpoint[1], rb.resval.rpoint[2]);
			}
			else
			{
				filer->pushBackItem();
				filer->setError(Acad::eInvalidDxfCode,
					_T("\nError: expected group code %d"),
					AcDb::kDxfXCoord);
				return filer->filerStatus();
			}

			m_polylineVertex.push_back(ptVertex);
        }
	  }

#ifdef _USEAMODELER_
	if(m_3dGeom.isNull())
		createBody();
#endif
	m_CirclePts.clear();
	//////////////////////////////////////////////////////////////////////////
	//计算得到旋转截面的圆弧半径和圆弧的坐标等
	GetCirclePtsAndMaxRadius();

    return filer->filerStatus();
}

#ifdef _OBJECTARX2010_
Adesk::Boolean PDRevolve::subWorldDraw(AcGiWorldDraw* mode)
#else
Adesk::Boolean PDRevolve::worldDraw(AcGiWorldDraw* mode)
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
	if(m_polylineVertex.empty())
		return Adesk::kTrue;
	int Precision =m_dDividPrecision;
	Adesk::Int16 nRating = getCalMaxPrecision();
	if(nRating == PRIMARY3D_SIMPLIZE_RATE1 ||
		nRating == PRIMARY3D_SIMPLIZE_RATE2)
	{
		//优化
		Precision =  (int)(PI / acos(1.0 - mode->deviation(kAcGiMaxDevForCircle, m_polylineVertex[0].m_vertex) / m_dMaxRadius));
		if(Precision < PRIMARY3D_SIMPLIZE_SCALE)
		{
			AcGePoint3dArray tmpPts;
			size_t i;
			for(i =0;i < m_CirclePts.size(); i++)
			{
				if(m_CirclePts[i].m_dRadius == 0)
				{
					continue;
				}
				if (mode->geometry().circularArc(m_CirclePts[i].m_cenPt,m_CirclePts[i].m_dRadius,
					m_CirclePts[i].m_normal,m_CirclePts[i].m_vectStart,m_angle))
				{
					return Adesk::kTrue;
				}
			}
			AcGePoint3dArray pts;
			for (i = 0; i < m_polylineVertex.size(); i ++)
			{
				pts.append(m_polylineVertex[i].m_vertex);
			}
			pts.append(pts.first());
			mode->geometry().polyline(pts.length(), pts.asArrayPtr());
			return Adesk::kTrue;
		}
	}
	std::vector<AcGePoint3dArray>  vertexArray;
	int PreStep = 1;
	if(Precision < PRIMARY3D_WIREFRAME_LINENUM * 2)
		Precision = PRIMARY3D_WIREFRAME_LINENUM;
	else
	{
		if(Precision > m_dDividPrecision)
			Precision = m_dDividPrecision;
		PreStep = Precision / PRIMARY3D_WIREFRAME_LINENUM;
		Precision = PreStep * PRIMARY3D_WIREFRAME_LINENUM;
	}
	if(getVertices(Precision, vertexArray) != Acad::eOk)
		return Adesk::kTrue;

	//switch (mode->regenType())
 //   {
 //   case kAcGiHideOrShadeCommand:
 //   case kAcGiRenderCommand:
		{
			for(size_t i = 0; i < vertexArray.size(); ++i)
			{
				AcGePoint3dArray ptarr = vertexArray[i];
				if(ptarr.length() < (Precision + 1) * 2)
				{
					if(mode->geometry().polyline(ptarr.length(), ptarr.asArrayPtr()))
						return Adesk::kTrue;
				}
				else
				{
					if(mode->geometry().mesh(ptarr.length() / (Precision + 1), Precision + 1, ptarr.asArrayPtr()))
						return Adesk::kTrue;
				}
			}
		}
		//break;
  //  case kAcGiStandardDisplay:
  //  case kAcGiSaveWorldDrawForR12:
  //  case kAcGiSaveWorldDrawForProxy:
  //    {
  //      
  //    }
  //    break;
  //  }
#endif
	return Adesk::kTrue;
}

/*Adesk::Boolean PDRevolve::worldDraw(AcGiWorldDraw* mode)
{
	assertReadEnabled();
#ifdef _USEAMODELER_
	PDPrimary3D::worldDraw(mode);
#endif
	if (mode->regenAbort()) {
		return Adesk::kTrue;
	}
#ifndef _USEAMODELER_

	std::vector<AcGePoint3dArray>  vertexArray;
	AcGeIntArray stdIdx;
	int actPrecision;
	if(getVertices(getMaxPrecision(mode), vertexArray, stdIdx, actPrecision) != Acad::eOk)
		return Adesk::kTrue;

	//switch (mode->regenType())
	//   {
	//   case kAcGiHideOrShadeCommand:
	//   case kAcGiRenderCommand:
	{
		for(int i = 0; i < vertexArray.size(); ++i)
		{
			AcGePoint3dArray ptarr = vertexArray[i];
			if(ptarr.length() < (actPrecision + 1) * 2)
			{
				mode->geometry().polyline(ptarr.length(), ptarr.asArrayPtr());
			}
			else
			{
				mode->geometry().mesh(ptarr.length() / (actPrecision + 1), actPrecision + 1, ptarr.asArrayPtr());
			}
		}
	}
	//break;
	//  case kAcGiStandardDisplay:
	//  case kAcGiSaveWorldDrawForR12:
	//  case kAcGiSaveWorldDrawForProxy:
	//    {
	//      
	//    }
	//    break;
	//  }
#endif
	return Adesk::kTrue;
}*/
#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDRevolve::subGetOsnapPoints(
	AcDb::OsnapMode     osnapMode,
	Adesk::GsMarker     gsSelectionMark,
	const AcGePoint3d&  pickPoint,
	const AcGePoint3d&  lastPoint,
	const AcGeMatrix3d& viewXform,
	AcGePoint3dArray&   snapPoints,
	AcDbIntArray &   geomIds) const
#else
Acad::ErrorStatus PDRevolve::getOsnapPoints(
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

  return Acad::eOk;
}

// 旋转轴上一点
AcGePoint3d               PDRevolve::getPtAxis() const
{
  assertReadEnabled();
  return m_ptAxis;
}

// 旋转轴方向
AcGeVector3d              PDRevolve::getVecAxis() const
{
  assertReadEnabled();
  return m_vecAxis;
}

double PDRevolve::getAngle() const
{
	assertReadEnabled();
	return m_angle;
}

std::vector<t_PolylineVertex> PDRevolve::getPolylineVertex() const
{
	assertReadEnabled();
	return m_polylineVertex;
}

Acad::ErrorStatus         PDRevolve::setVal(const AcGePoint3d &ptAxis, const AcGeVector3d &vecAxis, const std::vector<t_PolylineVertex>& val)
{
  assertWriteEnabled();

  m_ptAxis = ptAxis;
  m_vecAxis = vecAxis.normal();
  m_polylineVertex = val;

#ifdef _USEAMODELER_
  createBody();
#endif
  GetCirclePtsAndMaxRadius();
  return Acad::eOk;
}


void PDRevolve::createBody()
{
	assertWriteEnabled();
//#ifdef _OBJECTARX2004_    
	AcGePoint3d *vertices = NULL;
	PolygonVertexData** vertexData = NULL;
	int iVertices;
	AcGeVector3d normal;

	convertPolyVertexToAmodelerData(m_polylineVertex, vertices, vertexData, iVertices, normal);

	if(iVertices < 2)
		return ;

	m_3dGeom = Body::axisRevolution((Point3d*)(vertices), 
		vertexData,
		iVertices,       
		*(Vector3d*)&normal,
		Line3d((Point3d)m_ptAxis, (Vector3d)m_vecAxis), 
		m_angle,
		m_dDividPrecision);

	delete [] vertexData;
	vertexData = NULL;
	delete [] vertices;
	vertices = NULL;
//#endif
}

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDRevolve::subExplode(AcDbVoidPtrArray& entitySet) const
#else
Acad::ErrorStatus PDRevolve::explode(AcDbVoidPtrArray& entitySet) const
#endif
{
	assertReadEnabled();
	AcDb3dSolid *p3dSolid;
	if(explodeTo3DSolid(p3dSolid) != Acad::eOk)return Acad::eNotApplicable;
	entitySet.append(p3dSolid);
	return Acad::eOk;
}

//因为要与2004兼容,这样的化原本属于2007中的acdb3dsolid类中有CreateRevolvedSolid函数不能用(autocad2004中没有这个函数),所有在此封装.
//Acad::ErrorStatus CreateRevolvedSolid(AcDb3dSolid &m3dSolid,       //作为全局函数,需要传入一个acdb3dsolid类,生成的solid类就保存在这个对象中.
//
//									  AcDbEntity* pRevEnt,        //Input pointer to the planar curve, region, or planar surface that is to be revolved 
//
//									  const AcGePoint3d& axisPnt, //旋转轴上一点
//
//									  const AcGeVector3d& axisDir,//旋转轴向量
//
//									  double revAngle,            //旋转角度
//
//									  double startAngle)          //Input start angle of rotation. If 0, then rotation will start from current position of pRevEnt 
//{
//	AcDbVoidPtrArray PtrEnts;
//	AcDbVoidPtrArray regions;
//	PtrEnts.append(pRevEnt);
//	if (AcDbRegion::createFromCurves(PtrEnts,regions) != Acad::eOk)
//	{
//		for(int i = 0; i < regions.length(); i++)
//			delete (AcRxObject*)regions[i];
//		return Acad::eNotApplicable;
//	}
//	if (regions.length() != 1)
//	{
//		for(int i = 0; i < regions.length(); i++)
//			delete (AcRxObject*)regions[i];
//		return Acad::eNotApplicable;
//	}
//	AcDbRegion *region=(AcDbRegion *)(regions[0]);
//	if(m3dSolid.revolve(region,axisPnt,axisDir,revAngle) != Acad::eOk)
//	{
//		for(int i = 0; i < regions.length(); i++)
//			delete (AcRxObject*)regions[i];
//		return Acad::eNotApplicable;
//	}
//	for(int i = 0; i < regions.length(); i++)
//		delete (AcRxObject*)regions[i];
//	return Acad::eOk;
//}

Acad::ErrorStatus PDRevolve::explodeTo3DSolid(AcDb3dSolid* &p3dSolid) const
{
	////赵国强于200807311721插入 ()
	//start
	assertReadEnabled();

	AcGePoint3d pts[3];
	double ang = 0;
	AcGeVector3d vec0;
	AcGeVector3d vec1;
	AcGeVector3d vecZ;

	pts[0] = m_polylineVertex[0].m_vertex;
	pts[1] = m_polylineVertex[1].m_vertex;
	pts[2] = m_polylineVertex[2].m_vertex;

	vec0 = pts[1] - pts[0];
	vec1 = pts[2] - pts[0];

	vecZ = vec0.crossProduct(vec1).normal();
	int i=0;
	int j=0;

	AcDbVoidPtrArray curveSegments;
	convertPolyVertexToSegments(m_polylineVertex, vecZ, curveSegments);

	AcDbVoidPtrArray regions;
	Acad::ErrorStatus es = Acad::eOk;
	es = AcDbRegion::createFromCurves(curveSegments, regions);

	for(i = 0; i < curveSegments.length(); i++)
		delete (AcRxObject*)curveSegments[i];
	curveSegments.setLogicalLength(0);

	if(es == Acad::eOk && !regions.isEmpty())
	{
		AcDb3dSolid *pBody = NULL;
		pBody = new AcDb3dSolid;
		es = pBody->revolve((AcDbRegion*)(regions[0]), m_ptAxis, m_vecAxis, m_angle);
		if(es != Acad::eOk)
		{
			delete pBody;
			for(i = 0; i < regions.length(); i++)
				delete (AcRxObject*)regions[i];
			return Acad::eNotApplicable;
		}

		for(i = 0; i < regions.length(); i++)
			delete (AcRxObject*)regions[i];
		regions.setLogicalLength(0);

		pBody->setPropertiesFrom(this);
		p3dSolid = pBody;
	}
	else
	{
		for(i = 0; i < regions.length(); i++)
			delete (AcRxObject*)regions[i];
		return Acad::eNotApplicable;
	}

	for(i = 0; i < regions.length(); i++)
		delete (AcRxObject*)regions[i];

	return Acad::eOk;


	//int actPrecision__1 =m_dDividPrecision + 1;

	//if(m_polylineVertex.size() <= 0)
	//	return Acad::eInvalidInput;

	//double deltaangle = m_angle / m_dDividPrecision;

	//AcGePoint3dArray ptarr;
	//{
	//	AcGePoint3d pt = m_polylineVertex[0].m_vertex;
	//	ptarr.append(pt);
	//	
	//}
	//
	//AcGeLine3d AxisLine(m_ptAxis, m_vecAxis);
	//for(int k = 1; k < m_polylineVertex.size(); ++k)
	//{
	//	switch(m_polylineVertex[k - 1].m_vertexFlag)
	//	{
	//	case POLYLINEVERTE_FLAG_L:	// 直线的起点
	//		{
 //    			AcGePoint3d pt = m_polylineVertex[k].m_vertex;
	//			ptarr.append(pt);
 //           }
	//		break;
	//	case POLYLINEVERTE_FLAG_A:	// 圆弧的起点
	//		{

	//			if(m_polylineVertex.size() <= k + 1)
	//				break;
	//			AcGeCircArc3d geArc(m_polylineVertex[k - 1].m_vertex, m_polylineVertex[k].m_vertex, m_polylineVertex[k + 1].m_vertex);
	//			AcGeInterval geInterval;
	//			geArc.getInterval(geInterval);
	//			double sparam = geInterval.lowerBound();
	//			double eparam = geInterval.upperBound();
	//			double deltaParam = (eparam - sparam) / m_dDividPrecision;
	//			for(int j = 1; j <= m_dDividPrecision; ++j)
	//			{
	//				AcGePoint3d pt = geArc.evalPoint(sparam + deltaParam * j);
	//				ptarr.append(pt);
	//			}
	//		}
	//		break;
	//	case POLYLINEVERTE_FLAG_AM:	// 圆弧的中间一点
	//		break;
	//	case POLYLINEVERTE_FLAG_LC:	// 多义线的最后一点，该多义线封闭且封闭使用直线
	//		{
	//			AcGePoint3d pt = m_polylineVertex[0].m_vertex;
	//			ptarr.append(pt);
	//		}
	//		break;
	//	case POLYLINEVERTE_FLAG_AC:	//多义线的最后一点，该多义线封闭且封闭使用圆弧，其后还需跟随圆弧上的中间一点
	//		{
	//			AcGeCircArc3d geArc(m_polylineVertex[k - 1].m_vertex, m_polylineVertex[k].m_vertex, m_polylineVertex[0].m_vertex);
	//			AcGeInterval geInterval;
	//			geArc.getInterval(geInterval);
	//			double sparam = geInterval.lowerBound();
	//			double eparam = geInterval.upperBound();
	//			double deltaParam = (eparam - sparam) / m_dDividPrecision;
	//			for(int j = 1; j <= m_dDividPrecision; ++j)
	//			{
	//				AcGePoint3d pt = geArc.evalPoint(sparam + deltaParam * j);
	//				ptarr.append(pt);
	//				if(j == m_dDividPrecision)
	//					ptarr.append(pt);
	//			}
	//		}
	//		break;
	//	case POLYLINEVERTE_FLAG_C:
	//		break;
	//	default:
	//		break;
	//	}
	//	
	//}

	//AcDb3dPolyline *p3dPolyLine = new AcDb3dPolyline(AcDb::k3dSimplePoly,ptarr,Adesk::kTrue);

	//AcDb3dSolid *pBody = new AcDb3dSolid();

 //  // AcDbRevolveOptions revOptions;

	//if(CreateRevolvedSolid(*pBody,p3dPolyLine,m_ptAxis,m_vecAxis,m_angle,0) != Acad::eOk)
	//{
	//	delete pBody;
	//	delete p3dPolyLine;
	//	return Acad::eNotApplicable;
	//}
	//delete p3dPolyLine;

	//pBody->setPropertiesFrom(this);
	//p3dSolid=pBody;
	//return Acad::eOk;
	////end
}

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDRevolve::subGetGeomExtents(AcDbExtents& extents) const
#else
Acad::ErrorStatus PDRevolve::getGeomExtents(AcDbExtents& extents) const
#endif
{
	assertReadEnabled();

	std::vector<AcGePoint3dArray> pArray;
	AcGeIntArray stdIdx;
	int actPrecision;
	getVertices(m_dDividPrecision, pArray, stdIdx, actPrecision);
	if(pArray.size() > 0 && pArray[0].length() > 0)
	{
		extents.set(pArray[0][0], pArray[0][0]);
		for(size_t i = 0; i < pArray.size(); ++i)
		{
			for(int j = 0; j < pArray[i].length(); ++j)
				extents.addPoint(pArray[i][j]);
		}
	}
	return Acad::eOk;
}

#ifdef _OBJECTARX2010_
void PDRevolve::subList() const
#else
void PDRevolve::list() const
#endif
{
	assertReadEnabled();
#ifdef _OBJECTARX2010_
	PDPrimary3D::subList();
#else
	PDPrimary3D::list();
#endif
	return ;
}

Acad::ErrorStatus PDRevolve::getVertices(int actPrecision, std::vector<AcGePoint3dArray>& vertexArray) const
{
	assertReadEnabled();

	/*while(!vertexArray.empty())
		vertexArray.pop_back();*/
	vertexArray.clear();

	int actPrecision__1 =actPrecision + 1;

	if(m_polylineVertex.size() <= 0)
		return Acad::eInvalidInput;

	double deltaangle = m_angle / actPrecision;

	AcGePoint3dArray prePtarr;
	{
		AcGePoint3d pt = m_polylineVertex[0].m_vertex;
		prePtarr.append(pt);
		for(int i = 1; i < actPrecision__1; ++i)
		{
			pt.rotateBy(deltaangle, m_vecAxis, m_ptAxis);
			prePtarr.append(pt);
		}
	}
	if(m_polylineVertex.size() == 1)
	{
		vertexArray.push_back(prePtarr);
		return Acad::eOk;
	}
	AcGeLine3d AxisLine(m_ptAxis, m_vecAxis);
	for(size_t k = 1; k < m_polylineVertex.size(); ++k)
	{
		switch(m_polylineVertex[k - 1].m_vertexFlag)
		{
		case 1:	// 直线的起点
			{
				AcGePoint3dArray ptarr;
				ptarr.append(prePtarr);
				prePtarr.setLogicalLength(0);

				AcGePoint3d pt = m_polylineVertex[k].m_vertex;
				ptarr.append(pt);
				prePtarr.append(pt);
				for(int i = 1; i < actPrecision__1; ++i)
				{
					pt.rotateBy(deltaangle, m_vecAxis, m_ptAxis);
					ptarr.append(pt);
					prePtarr.append(pt);
				}
				if(!AxisLine.isOn(m_polylineVertex[k - 1].m_vertex) || !AxisLine.isOn(m_polylineVertex[k].m_vertex))
					vertexArray.push_back(ptarr);
			}
			break;
		case 2:	// 圆弧的起点
			{
				AcGePoint3dArray ptarr;
				ptarr.append(prePtarr);
				prePtarr.setLogicalLength(0);

				if(m_polylineVertex.size() <= k + 1)
					break;
				AcGeCircArc3d geArc(m_polylineVertex[k - 1].m_vertex, m_polylineVertex[k].m_vertex, m_polylineVertex[k + 1].m_vertex);
				AcGeInterval geInterval;
				geArc.getInterval(geInterval);
				double sparam = geInterval.lowerBound();
				double eparam = geInterval.upperBound();
				double deltaParam = (eparam - sparam) / actPrecision;
				for(int j = 1; j <= actPrecision; ++j)
				{
					AcGePoint3d pt = geArc.evalPoint(sparam + deltaParam * j);
					ptarr.append(pt);
					if(j == actPrecision)
						prePtarr.append(pt);
					for(int i = 1; i < actPrecision__1; ++i)
					{
						pt.rotateBy(deltaangle, m_vecAxis, m_ptAxis);
						ptarr.append(pt);
						if(j == actPrecision)
							prePtarr.append(pt);
					}
				}
				vertexArray.push_back(ptarr);
			}
			break;
		case 3:	// 圆弧的中间一点
			break;
		case 4:	// 多义线的最后一点，该多义线封闭且封闭使用直线
			{
				AcGePoint3dArray ptarr;
				ptarr.append(prePtarr);
				prePtarr.setLogicalLength(0);

				AcGePoint3d pt = m_polylineVertex[0].m_vertex;
				ptarr.append(pt);
				prePtarr.append(pt);
				for(int i = 1; i < actPrecision__1; ++i)
				{
					pt.rotateBy(deltaangle, m_vecAxis, m_ptAxis);
					ptarr.append(pt);
					prePtarr.append(pt);
				}
				if(!AxisLine.isOn(m_polylineVertex[k - 1].m_vertex) || !AxisLine.isOn(m_polylineVertex[0].m_vertex))
					vertexArray.push_back(ptarr);
			}
			break;
		case 5:	//多义线的最后一点，该多义线封闭且封闭使用圆弧，其后还需跟随圆弧上的中间一点
			{
				AcGePoint3dArray ptarr;
				ptarr.append(prePtarr);
				prePtarr.setLogicalLength(0);

				AcGeCircArc3d geArc(m_polylineVertex[k - 1].m_vertex, m_polylineVertex[k].m_vertex, m_polylineVertex[0].m_vertex);
				AcGeInterval geInterval;
				geArc.getInterval(geInterval);
				double sparam = geInterval.lowerBound();
				double eparam = geInterval.upperBound();
				double deltaParam = (eparam - sparam) / actPrecision;
				for(int j = 1; j <= actPrecision; ++j)
				{
					AcGePoint3d pt = geArc.evalPoint(sparam + deltaParam * j);
					ptarr.append(pt);
					if(j == actPrecision)
						prePtarr.append(pt);
					for(int i = 1; i < actPrecision__1; ++i)
					{
						pt.rotateBy(deltaangle, m_vecAxis, m_ptAxis);
						ptarr.append(pt);
						if(j == actPrecision)
							prePtarr.append(pt);
					}
				}
				vertexArray.push_back(ptarr);
			}
			break;
		default:
			break;
		}
		if(k == m_polylineVertex.size() - 1 && m_polylineVertex[k].m_vertexFlag == 4)
		{
			AcGePoint3dArray ptarr;
			ptarr.append(prePtarr);
			prePtarr.setLogicalLength(0);

			AcGePoint3d pt = m_polylineVertex[0].m_vertex;
			ptarr.append(pt);
			prePtarr.append(pt);
			for(int i = 1; i < actPrecision__1; ++i)
			{
				pt.rotateBy(deltaangle, m_vecAxis, m_ptAxis);
				ptarr.append(pt);
				prePtarr.append(pt);
			}
			if(!AxisLine.isOn(m_polylineVertex[k].m_vertex) || !AxisLine.isOn(m_polylineVertex[0].m_vertex))
				vertexArray.push_back(ptarr);
		}
	}

	return Acad::eOk;
}

Acad::ErrorStatus PDRevolve::getVertices(int Precision, std::vector<AcGePoint3dArray>& vertexArray, 
							  AcGeIntArray &stdIdx, int &actPrecision) const
{
	assertReadEnabled();

	while(!vertexArray.empty())
		vertexArray.pop_back();

	AcGeDoubleArray dividDbl;
	getActDivid(Precision, dividDbl, stdIdx);

	int actPrecision__1 = dividDbl.length();
	actPrecision = actPrecision__1 - 1;

	if(m_polylineVertex.size() <= 0)
		return Acad::eInvalidInput;

	double deltaangle = m_angle / Precision;

	AcGePoint3dArray prePtarr;
	{
		AcGePoint3dArray ptarr;
		AcGePoint3d pt = m_polylineVertex[0].m_vertex;
		ptarr.append(pt);
		for(int i = 1; i < actPrecision__1; ++i)
		{
			pt = m_polylineVertex[0].m_vertex;
			pt.rotateBy(deltaangle * dividDbl[i], m_vecAxis, m_ptAxis);
			ptarr.append(pt);
		}
		prePtarr = ptarr;
	}
	if(m_polylineVertex.size() == 1)
	{
		vertexArray.push_back(prePtarr);
		return Acad::eOk;
	}
	AcGeLine3d AxisLine(m_ptAxis, m_vecAxis);
	for(size_t k = 1; k < m_polylineVertex.size(); ++k)
	{
		switch(m_polylineVertex[k - 1].m_vertexFlag)
		{
		case 1:	// 直线的起点
			{
				AcGePoint3dArray ptarr;
				ptarr.append(prePtarr);
				prePtarr.setLogicalLength(0);
				AcGePoint3d pt = m_polylineVertex[k].m_vertex;
				ptarr.append(pt);
				prePtarr.append(pt);
				for(int i = 1; i < actPrecision__1; ++i)
				{
					pt = m_polylineVertex[k].m_vertex;
					pt.rotateBy(deltaangle * dividDbl[i], m_vecAxis, m_ptAxis);
					ptarr.append(pt);
					prePtarr.append(pt);
				}
				if(!AxisLine.isOn(m_polylineVertex[k - 1].m_vertex) || !AxisLine.isOn(m_polylineVertex[k].m_vertex))
					vertexArray.push_back(ptarr);
			}
			break;
		case 2:	// 圆弧的起点
			{
				AcGePoint3dArray ptarr;
				ptarr.append(prePtarr);
				prePtarr.setLogicalLength(0);

				if(m_polylineVertex.size() <= k + 1)
					break;
				AcGeCircArc3d geArc(m_polylineVertex[k - 1].m_vertex, m_polylineVertex[k].m_vertex, m_polylineVertex[k + 1].m_vertex);
				AcGeInterval geInterval;
				geArc.getInterval(geInterval);
				double sparam = geInterval.lowerBound();
				double eparam = geInterval.upperBound();
				double deltaParam = (eparam - sparam) / Precision;
				for(int j = 1; j <= Precision; ++j)
				{
					AcGePoint3d ptArc;
					AcGePoint3d pt = geArc.evalPoint(sparam + deltaParam * j);
					ptArc = pt;
					ptarr.append(pt);
					if(j == Precision)
						prePtarr.append(pt);
					for(int i = 1; i < actPrecision__1; ++i)
					{
						pt = ptArc;
						pt.rotateBy(deltaangle * dividDbl[i], m_vecAxis, m_ptAxis);
						ptarr.append(pt);
						if(j == Precision)
							prePtarr.append(pt);
					}
				}
				vertexArray.push_back(ptarr);
			}
			break;
		case 3:	// 圆弧的中间一点
			break;
		case 4:	// 多义线的最后一点，该多义线封闭且封闭使用直线
			{
				AcGePoint3dArray ptarr;
				ptarr.append(prePtarr);
				prePtarr.setLogicalLength(0);

				AcGePoint3d pt = m_polylineVertex[0].m_vertex;
				ptarr.append(pt);
				prePtarr.append(pt);
				for(int i = 1; i < actPrecision__1; ++i)
				{
					pt = m_polylineVertex[0].m_vertex;
					pt.rotateBy(deltaangle * dividDbl[i], m_vecAxis, m_ptAxis);
					ptarr.append(pt);
					prePtarr.append(pt);
				}
				if(!AxisLine.isOn(m_polylineVertex[k - 1].m_vertex) || !AxisLine.isOn(m_polylineVertex[0].m_vertex))
					vertexArray.push_back(ptarr);
			}
			break;
		case 5:	//多义线的最后一点，该多义线封闭且封闭使用圆弧，其后还需跟随圆弧上的中间一点
			{
				AcGePoint3dArray ptarr;
				ptarr.append(prePtarr);
				prePtarr.setLogicalLength(0);

				AcGeCircArc3d geArc(m_polylineVertex[k - 1].m_vertex, m_polylineVertex[k].m_vertex, m_polylineVertex[0].m_vertex);
				AcGeInterval geInterval;
				geArc.getInterval(geInterval);
				double sparam = geInterval.lowerBound();
				double eparam = geInterval.upperBound();
				double deltaParam = (eparam - sparam) / Precision;
				for(int j = 1; j <= Precision; ++j)
				{
					AcGePoint3d ptArc;
					AcGePoint3d pt = geArc.evalPoint(sparam + deltaParam * j);
					ptArc = pt;
					ptarr.append(pt);
					if(j == Precision)
						prePtarr.append(pt);
					for(int i = 1; i < actPrecision__1; ++i)
					{
						pt = ptArc;
						pt.rotateBy(deltaangle * dividDbl[i], m_vecAxis, m_ptAxis);
						ptarr.append(pt);
						if(j == Precision)
							prePtarr.append(pt);
					}
				}
				vertexArray.push_back(ptarr);
			}
			break;
		default:
			break;
		}
		if(k == m_polylineVertex.size() - 1 && m_polylineVertex[k].m_vertexFlag == 4)
		{
			AcGePoint3dArray ptarr;
			ptarr.append(prePtarr);
			prePtarr.setLogicalLength(0);
			AcGePoint3d pt = m_polylineVertex[0].m_vertex;
			ptarr.append(pt);
			prePtarr.append(pt);
			for(int i = 1; i < actPrecision__1; ++i)
			{
				pt = m_polylineVertex[0].m_vertex;
				pt.rotateBy(deltaangle * dividDbl[i], m_vecAxis, m_ptAxis);
				ptarr.append(pt);
				prePtarr.append(pt);
			}
			if(!AxisLine.isOn(m_polylineVertex[k].m_vertex) || !AxisLine.isOn(m_polylineVertex[0].m_vertex))
				vertexArray.push_back(ptarr);
		}
	}

	return Acad::eOk;
}

// 取得合适的剖分精度，如果实体较小，则剖分精度就会低，如果实体较大，则剖分
// 精度不大于实体限定的剖分精度m_dDividPrecision。
int PDRevolve::getMaxPrecision(AcGiWorldDraw *mode) const
{
	assertReadEnabled();
	if(!getCalMaxPrecision())
		return m_dDividPrecision;

	if(m_polylineVertex.size() <= 0)
		return m_dDividPrecision;
	AcGeLine3d axisLine(m_ptAxis, m_vecAxis);
	AcGePoint3d axisPt;	// 旋转轮廓起点投影到轴线上的点。
	axisPt = axisLine.closestPointTo(m_polylineVertex[0].m_vertex);
	double dist = axisLine.distanceTo(m_polylineVertex[0].m_vertex);
	double distTemp;
	for(size_t i = 1; i < m_polylineVertex.size(); ++i)
	{
		distTemp = axisLine.distanceTo(m_polylineVertex[i].m_vertex);
		if(dist < distTemp)
			dist = distTemp;
	}
	int pre = (int)(PI / acos(1.0 - mode->deviation(kAcGiMaxDevForCircle, axisPt) / dist));
	if(pre > m_dDividPrecision)
		return m_dDividPrecision;
	else if(pre > 8)
		return pre;
	else
		return 8;
}
void PDRevolve::GetCirclePtsAndMaxRadius()
{
	assertWriteEnabled();
	//////////////////////////////////////////////////////////////////////////
	//计算得到旋转截面的圆弧半径和圆弧的坐标等
	AcGeLine3d AxisLine(m_ptAxis, m_vecAxis); 
	m_dMaxRadius = 0;
	double  dTmp = 0;
	AcGePlane tmpPlan;
	_t_Arc_params tmpArcParams;
	AcGePoint3d pt;
	m_CirclePts.clear();
	for(size_t i = 0; i < m_polylineVertex.size(); i++)
	{
		pt = m_polylineVertex[i].m_vertex;
		if(AxisLine.isOn(pt) != Adesk::kTrue)
		{
			dTmp = AxisLine.distanceTo(m_polylineVertex[i].m_vertex);
			if(m_dMaxRadius < dTmp)
				m_dMaxRadius = dTmp;
			tmpArcParams.m_dRadius = dTmp;
			AxisLine.getPerpPlane(pt,tmpPlan);
			tmpPlan.intersectWith(AxisLine,tmpArcParams.m_cenPt);
			tmpArcParams.m_vectStart = pt - tmpArcParams.m_cenPt;
			tmpArcParams.m_normal = m_vecAxis;
			m_CirclePts.push_back(tmpArcParams);
		}
		else
		{
			dTmp = 0;
			if(m_dMaxRadius < dTmp)
				m_dMaxRadius = dTmp;
			tmpArcParams.m_cenPt = pt;
			tmpArcParams.m_dRadius = 0;
			tmpArcParams.m_vectStart = AcGeVector3d::kIdentity;
			m_CirclePts.push_back(tmpArcParams);
		}
	}
}
