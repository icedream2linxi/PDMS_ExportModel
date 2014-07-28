// PDSpolygon.cpp: implementation of the PDSpolygon class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "language25.h"
#include "PDSpolygon.h"

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
#include <actrans.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define VERSION_PDSPOLYGON 2

#define PI 3.1415926535897932384626433832795L

#ifdef _OBJECTARX2000_
ACRX_DXF_DEFINE_MEMBERS(PDSpolygon, PDPrimary3D,
                        AcDb::kDHL_CURRENT, AcDb::kMReleaseCurrent,
                        AcDbProxyEntity::kAllAllowedBits,
                        PDSPOLYGON, Gle);
#else
ACRX_DXF_DEFINE_MEMBERS(PDSpolygon, PDPrimary3D, AcDbProxyEntity::kAllAllowedBits, PDSPOLYGON, Gle);
#endif

PDSpolygon::PDSpolygon()
{
}

PDSpolygon::PDSpolygon(const AcGePoint3d& oriPt, const AcGeVector3d& V1, const AcGeVector3d& V2, 
                       const AcGeVector3d& Vh, double H, const AcGePoint3dArray& OutPtArr, 
                       const std::vector<AcGePoint3dArray>& InPtArrArr, bool HasSnap /*= false*/, int CorrType/* = 1*/) : PDPrimary3D(HasSnap)
{
	m_ptOri = oriPt;      // 空间任意平面坐标系原点
	m_vecV1 = V1.normal();      // X轴方向
	m_vecV2 = V2.normal();      // Y轴方向

	AcGeMatrix3d mat;
	mat.setCoordSystem(m_ptOri, m_vecV1.normal(), m_vecV2.normal(), m_vecV1.crossProduct(m_vecV2).normal());

	m_vecVh = Vh;
	if(m_vecVh.z == 0)
    {
		ads_printf(LANGUAGE_DEFINE_MACRO_V25_4779/*"\n 拉伸体的拉伸方向错误，拉伸方向在拉伸平面内。"*/);
    }
	if(!CorrType)
		m_vecVh.set(m_vecVh.x, m_vecVh.y, -m_vecVh.z);
	m_vecVh.transformBy(mat).normalize();  // 拉伸方向
  
	m_dH = fabs(H);         // 拉伸高度
	if(m_dH == 0)
    {
		ads_printf(LANGUAGE_DEFINE_MACRO_V25_4780/*"\n 拉伸体的拉伸高度为零。"*/);
    }

	if(OutPtArr.length() < 3)
    {
		ads_printf(LANGUAGE_DEFINE_MACRO_V25_4781/*"\n 拉伸体的拉伸轮廓的外环顶点数小于3。"*/);
    }
	else
    {
		size_t i;
		int j;
		for(j = 0; j < OutPtArr.length(); j++)
        {
			t_PolylineVertex ptTemp;
			if(j == OutPtArr.length() - 1)
				ptTemp.m_vertexFlag = POLYLINEVERTE_FLAG_LC;
			else
				ptTemp.m_vertexFlag = POLYLINEVERTE_FLAG_L;
			if(CorrType)
				ptTemp.m_vertex.set(OutPtArr[j].x, OutPtArr[j].y, OutPtArr[j].z);
			else
				ptTemp.m_vertex.set(OutPtArr[j].x, OutPtArr[j].y, -OutPtArr[j].z);
			ptTemp.m_vertex.transformBy(mat);
			m_vertexArrOut.push_back(ptTemp);   // 外环的顶点
        }
		for(i = 0; i < InPtArrArr.size(); i++)
        {
			if(InPtArrArr[i].length() < 3)
				ads_printf(LANGUAGE_DEFINE_MACRO_V25_4782/*"\n 拉伸体的拉伸轮廓的内环顶点数小于3。"*/);
			else
            {
				std::vector<t_PolylineVertex> inPtArr;
				for(int j = 0; j < InPtArrArr[i].length(); j++)
                {
					t_PolylineVertex ptTemp;
					if(j == InPtArrArr[i].length() - 1)
						ptTemp.m_vertexFlag = POLYLINEVERTE_FLAG_LC;
					else
						ptTemp.m_vertexFlag = POLYLINEVERTE_FLAG_L;
					if(CorrType)
						ptTemp.m_vertex.set(InPtArrArr[i][j].x, InPtArrArr[i][j].y, InPtArrArr[i][j].z);
					else
						ptTemp.m_vertex.set(InPtArrArr[i][j].x, InPtArrArr[i][j].y, -InPtArrArr[i][j].z);
					ptTemp.m_vertex.transformBy(mat);
					inPtArr.push_back(ptTemp);   // 内环的顶点
                }
				m_vertexArrArrIn.push_back(inPtArr);
            }
        }
    }
	//createBody();
	GetMaxLen();
}

PDSpolygon::PDSpolygon(int OutPtNum, const std::vector<int>& InPtNumArr, const AcGePoint3d& oriPt, 
             const AcGeVector3d& V1, const AcGeVector3d& V2, const AcGeVector3d& Vh, 
             double H, const AcGePoint3dArray& PtArr, bool HasSnap /*= false*/, int CorrType/* = 1*/) : PDPrimary3D(HasSnap)
{
	m_ptOri = oriPt;      // 空间任意平面坐标系原点
	m_vecV1 = V1.normal();      // X轴方向
	m_vecV2 = V2.normal();      // Y轴方向

	AcGeMatrix3d mat;
	mat.setCoordSystem(m_ptOri, m_vecV1.normal(), m_vecV2.normal(), m_vecV1.crossProduct(m_vecV2).normal());

	m_vecVh = Vh;
	if(m_vecVh.z == 0)
    {
		ads_printf(LANGUAGE_DEFINE_MACRO_V25_4779/*"\n 拉伸体的拉伸方向错误，拉伸方向在拉伸平面内。"*/);
    }
	if(!CorrType)
		m_vecVh.set(m_vecVh.x, m_vecVh.y, -m_vecVh.z);
	m_vecVh.transformBy(mat).normalize();  // 拉伸方向
  
	m_dH = fabs(H);         // 拉伸高度
	if(m_dH == 0)
    {
		ads_printf(LANGUAGE_DEFINE_MACRO_V25_4780/*"\n 拉伸体的拉伸高度为零。"*/);
    }

	if(OutPtNum < 3)
    {
		ads_printf(LANGUAGE_DEFINE_MACRO_V25_4781/*"\n 拉伸体的拉伸轮廓的外环顶点数小于3。"*/);
    }
	else
    {
		size_t i;
		int j;
		for(j = 0; j < OutPtNum; j++)
        {
			t_PolylineVertex ptTemp;
			if(j == OutPtNum - 1)
				ptTemp.m_vertexFlag = POLYLINEVERTE_FLAG_LC;
			else
				ptTemp.m_vertexFlag = POLYLINEVERTE_FLAG_L;
			if(CorrType)
				ptTemp.m_vertex.set(PtArr[j].x, PtArr[j].y, PtArr[j].z);
			else
				ptTemp.m_vertex.set(PtArr[j].x, PtArr[j].y, -PtArr[j].z);
			ptTemp.m_vertex.transformBy(mat);
			m_vertexArrOut.push_back(ptTemp);   // 外环的顶点
        }
		int startIndex = OutPtNum;
		for(i = 0; i < InPtNumArr.size(); i++)
        {
			AcGePoint3dArray ptarr;
			if(InPtNumArr[i] < 3)
				ads_printf(LANGUAGE_DEFINE_MACRO_V25_4782/*"\n 拉伸体的拉伸轮廓的内环顶点数小于3。"*/);
			else
            {
				std::vector<t_PolylineVertex> inPtArr;
				for(int j = 0; j < InPtNumArr[i]; j++)
                {
					t_PolylineVertex ptTemp;
					if(j == InPtNumArr[i] - 1)
						ptTemp.m_vertexFlag = POLYLINEVERTE_FLAG_LC;
					else
						ptTemp.m_vertexFlag = POLYLINEVERTE_FLAG_L;
					if(CorrType)
						ptTemp.m_vertex.set(PtArr[startIndex + j].x, PtArr[startIndex + j].y, PtArr[startIndex + j].z);
					else
						ptTemp.m_vertex.set(PtArr[startIndex + j].x, PtArr[startIndex + j].y, -PtArr[startIndex + j].z);
					ptTemp.m_vertex.transformBy(mat);
					inPtArr.push_back(ptTemp);   // 内环的顶点
                }
				m_vertexArrArrIn.push_back(inPtArr);
            }
			startIndex += InPtNumArr[i];
        }
    }
	//createBody();
	GetMaxLen();
}

PDSpolygon::PDSpolygon(const AcGePoint3d& oriPt, const AcGeVector3d& V1, const AcGeVector3d& V2, 
		   const AcGeVector3d& Vh, double H, 
		   const std::vector<t_PolylineVertex>& OutPtArr,
		   const std::vector< std::vector<t_PolylineVertex> >& InPtArrArr, 
		   int Precision/* = PRIMARY3D_DEFAULT_PRECISION*/, bool HasSnap/* = false*/, int CorrType/* = 1*/) : PDPrimary3D(HasSnap)
{
	if(Precision > 2)
		m_dDividPrecision = Precision;
	else
		m_dDividPrecision = PRIMARY3D_DEFAULT_PRECISION;

	m_ptOri = oriPt;      // 空间任意平面坐标系原点
	m_vecV1 = V1.normal();      // X轴方向
	m_vecV2 = V2.normal();      // Y轴方向

	AcGeMatrix3d mat;
	mat.setCoordSystem(m_ptOri, m_vecV1.normal(), m_vecV2.normal(), m_vecV1.crossProduct(m_vecV2).normal());

	m_vecVh = Vh;
	if(m_vecVh.z == 0)
	{
		ads_printf(LANGUAGE_DEFINE_MACRO_V25_4779/*"\n 拉伸体的拉伸方向错误，拉伸方向在拉伸平面内。"*/);
	}
	if(!CorrType)
		m_vecVh.set(m_vecVh.x, m_vecVh.y, -m_vecVh.z);
	m_vecVh.transformBy(mat).normalize();  // 拉伸方向

	m_dH = fabs(H);         // 拉伸高度
	if(m_dH == 0)
	{
		ads_printf(LANGUAGE_DEFINE_MACRO_V25_4780/*"\n 拉伸体的拉伸高度为零。"*/);
	}

	if(OutPtArr.size() < 2)
	{
		ads_printf(LANGUAGE_DEFINE_MACRO_V25_4783/*"\n 拉伸体的拉伸轮廓的外环顶点数小于2。"*/);
	}
	else
	{
		m_vertexArrOut = OutPtArr;
		m_vertexArrArrIn = InPtArrArr;
		size_t i;
		for(i = 0; i < m_vertexArrOut.size(); ++i)
		{
			if(!CorrType)
				m_vertexArrOut[i].m_vertex.set(m_vertexArrOut[i].m_vertex.x, m_vertexArrOut[i].m_vertex.y, -m_vertexArrOut[i].m_vertex.z);
			m_vertexArrOut[i].m_vertex.transformBy(mat);
		}
		for(i = 0; i < m_vertexArrArrIn.size(); ++i)
		{
			for(size_t j = 0; j < m_vertexArrArrIn[i].size(); ++j)
			{
				if(!CorrType)
					m_vertexArrArrIn[i][j].m_vertex.set(m_vertexArrArrIn[i][j].m_vertex.x, m_vertexArrArrIn[i][j].m_vertex.y, -m_vertexArrArrIn[i][j].m_vertex.z);
				m_vertexArrArrIn[i][j].m_vertex.transformBy(mat);
			}
		}
	}
	//createBody();
	GetMaxLen();
}

PDSpolygon::~PDSpolygon()
{
}

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDSpolygon::subTransformBy(const AcGeMatrix3d& xform)
#else
Acad::ErrorStatus PDSpolygon::transformBy(const AcGeMatrix3d& xform)
#endif
{
	assertWriteEnabled();

	m_ptOri.transformBy(xform);
	m_vecV1.transformBy(xform).normalize();
	m_vecV2.transformBy(xform).normalize();
    m_vecVh.transformBy(xform).normalize();
	size_t i;
    for(i = 0; i < m_vertexArrOut.size(); i++)
        m_vertexArrOut[i].m_vertex.transformBy(xform);
    for(i = 0; i < m_vertexArrArrIn.size(); i++)
        for(size_t j = 0; j < m_vertexArrArrIn[i].size(); j++)
            m_vertexArrArrIn[i][j].m_vertex.transformBy(xform);

	m_dH *= xform.scale();

#ifdef _OBJECTARX2010_
	return PDPrimary3D::subTransformBy(xform);
#else
	return PDPrimary3D::transformBy(xform);
#endif
}

Acad::ErrorStatus PDSpolygon::dwgOutFields(AcDbDwgFiler* filer)const
{
    assertReadEnabled();
	Acad::ErrorStatus es=Acad::eOk;

	// Call dwgOutFields from father class: PDPrimary3D
	if((es=PDPrimary3D::dwgOutFields(filer))!=Acad::eOk)
	{
		return es;
	}

	// Write version number.
	filer->writeItem((Adesk::UInt16) VERSION_PDSPOLYGON);

	// version 2
	filer->writeItem((Adesk::Int32)(m_vertexArrOut.size()));
	size_t i;
	for(i = 0; i < m_vertexArrOut.size(); i++)
	{
		filer->writeItem(m_vertexArrOut[i].m_vertexFlag);
		filer->writeItem(m_vertexArrOut[i].m_vertex);
	}

	filer->writeItem((Adesk::Int32)(m_vertexArrArrIn.size()));
	for(i = 0; i < m_vertexArrArrIn.size(); i++)
	{
		filer->writeItem((Adesk::Int32)(m_vertexArrArrIn[i].size()));
		for(size_t j = 0; j < m_vertexArrArrIn[i].size(); j++)
		{
			filer->writeItem(m_vertexArrArrIn[i][j].m_vertexFlag);
			filer->writeItem(m_vertexArrArrIn[i][j].m_vertex);
		}
	}

	// Write the data members.
	filer->writeItem(m_ptOri);
	filer->writeItem(m_vecV1);
    filer->writeItem(m_vecV2);
    filer->writeItem(m_vecVh);
	filer->writeItem(m_dH);
    AcGeDwgIO::outFields(filer, m_ptarrOut);

    filer->writeItem((Adesk::Int32)(m_ptarrarrIn.size()));
    for(i = 0; i < m_ptarrarrIn.size(); i++)
        AcGeDwgIO::outFields(filer, m_ptarrarrIn[i]);

	return filer->filerStatus();
}

Acad::ErrorStatus PDSpolygon::dwgInFields(AcDbDwgFiler* filer)
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
	if (version > VERSION_PDSPOLYGON)
		return Acad::eMakeMeProxy;

	// Read the data members.
	if(version >= 2)
	{
		Adesk::Int32 len1, len2;
		while(!m_vertexArrOut.empty())
			m_vertexArrOut.pop_back();
		while(!m_vertexArrArrIn.empty())
			m_vertexArrArrIn.pop_back();

		filer->readItem(&len1);
		Adesk::Int32 i;
		for(i = 0; i < len1; ++i)
		{
			t_PolylineVertex ptVertex;
			filer->readItem(&ptVertex.m_vertexFlag);
			filer->readItem(&ptVertex.m_vertex);
			m_vertexArrOut.push_back(ptVertex);
		}

		filer->readItem(&len1);
		for(i = 0; i < len1; ++i)
		{
			std::vector<t_PolylineVertex> inPtArr;
			filer->readItem(&len2);
			for(Adesk::Int32 j = 0; j < len2; ++j)
			{
				t_PolylineVertex ptVertex;
				filer->readItem(&ptVertex.m_vertexFlag);
				filer->readItem(&ptVertex.m_vertex);
				inPtArr.push_back(ptVertex);
			}
			m_vertexArrArrIn.push_back(inPtArr);
		}
	}
	if(version >= 1)
    {
		  filer->readItem(&m_ptOri);
		  filer->readItem(&m_vecV1);
      filer->readItem(&m_vecV2);
      filer->readItem(&m_vecVh);
		  filer->readItem(&m_dH);
      AcGeDwgIO::inFields(filer, m_ptarrOut);

      while(!m_ptarrarrIn.empty())
        m_ptarrarrIn.pop_back();
      Adesk::Int32 len;
      filer->readItem(&len);
      for(Adesk::Int32 i = 0; i < len; i++)
        {
          AcGePoint3dArray ptarr;
          AcGeDwgIO::inFields(filer, ptarr);
          m_ptarrarrIn.push_back(ptarr);
        }

		if(version == 1)
		{
			while(!m_vertexArrOut.empty())
				m_vertexArrOut.pop_back();
			while(!m_vertexArrArrIn.empty())
				m_vertexArrArrIn.pop_back();

			size_t i;
			int j;
			for(j = 0; j < m_ptarrOut.length(); j++)
			{
				t_PolylineVertex ptTemp;
				if(j == m_ptarrOut.length() - 1)
					ptTemp.m_vertexFlag = POLYLINEVERTE_FLAG_LC;
				else
					ptTemp.m_vertexFlag = POLYLINEVERTE_FLAG_L;
				ptTemp.m_vertex = m_ptarrOut[j];
				m_vertexArrOut.push_back(ptTemp);   // 外环的顶点
			}
			for(i = 0; i < m_ptarrarrIn.size(); i++)
			{
				std::vector<t_PolylineVertex> inPtArr;
				for(int j = 0; j < m_ptarrarrIn[i].length(); j++)
				{
					t_PolylineVertex ptTemp;
					if(j == m_ptarrarrIn[i].length() - 1)
						ptTemp.m_vertexFlag = POLYLINEVERTE_FLAG_LC;
					else
						ptTemp.m_vertexFlag = POLYLINEVERTE_FLAG_L;
					ptTemp.m_vertex = m_ptarrarrIn[i][j];
					inPtArr.push_back(ptTemp);   // 内环的顶点
				}
				m_vertexArrArrIn.push_back(inPtArr);
			}
		}
	}

    //if(m_3dGeom.isNull())
    //    createBody();
    GetMaxLen();
	return filer->filerStatus();
}


Acad::ErrorStatus PDSpolygon::dxfOutFields(AcDbDxfFiler* filer) const
{
	assertReadEnabled();
	Acad::ErrorStatus es;

	if ((es = PDPrimary3D::dxfOutFields(filer))
	  != Acad::eOk)
	{
		return es;
	}

	// Write subclass marker.
	filer->writeItem(AcDb::kDxfSubclass, _T("PDSpolygon"));

	// Write version number.
	filer->writeItem(AcDb::kDxfInt16, (Adesk::UInt16) VERSION_PDSPOLYGON);

	// version 2
	filer->writeItem(AcDb::kDxfInt32, (Adesk::Int32)(m_vertexArrOut.size()));
	size_t i;
	for(i = 0; i < m_vertexArrOut.size(); i++)
	{
		filer->writeItem(AcDb::kDxfInt16, m_vertexArrOut[i].m_vertexFlag);
		filer->writeItem(AcDb::kDxfXCoord, m_vertexArrOut[i].m_vertex);
	}

	filer->writeItem(AcDb::kDxfInt32, (Adesk::Int32)(m_vertexArrArrIn.size()));
	for(i = 0; i < m_vertexArrArrIn.size(); i++)
	{
		filer->writeItem(AcDb::kDxfInt32, (Adesk::Int32)(m_vertexArrArrIn[i].size()));
		for(size_t j = 0; j < m_vertexArrArrIn[i].size(); j++)
		{
			filer->writeItem(AcDb::kDxfInt16, m_vertexArrArrIn[i][j].m_vertexFlag);
			filer->writeItem(AcDb::kDxfXCoord, m_vertexArrArrIn[i][j].m_vertex);
		}
	}

	// Write data members.
	filer->writeItem(AcDb::kDxfXCoord, m_ptOri);
	filer->writeItem(AcDb::kDxfXCoord + 1, m_vecV1);
	filer->writeItem(AcDb::kDxfXCoord + 2, m_vecV2);
	filer->writeItem(AcDb::kDxfXCoord + 3, m_vecVh);
	filer->writeItem(AcDb::kDxfReal, m_dH);
	AcGeDxfIO::outFields(filer, m_ptarrOut);

	filer->writeItem(AcDb::kDxfInt32, (Adesk::Int32)(m_ptarrarrIn.size()));
	for(i = 0; i < m_ptarrarrIn.size(); i++)
		AcGeDxfIO::outFields(filer, m_ptarrarrIn[i]);

    return filer->filerStatus();
}

Acad::ErrorStatus PDSpolygon::dxfInFields(AcDbDxfFiler* filer)
{
  assertWriteEnabled();
  Acad::ErrorStatus es = Acad::eOk;
  struct resbuf rb;

  if ((PDPrimary3D::dxfInFields(filer) != Acad::eOk)
      || !filer->atSubclassData(_T("PDSpolygon")))
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
		if (version > VERSION_PDSPOLYGON)
			return Acad::eMakeMeProxy;
	}

	// Read the data members.
	if(version >= 2)
	{
		Adesk::Int32 len1, len2;
		while(!m_vertexArrOut.empty())
			m_vertexArrOut.pop_back();
		while(!m_vertexArrArrIn.empty())
			m_vertexArrArrIn.pop_back();

		filer->readItem(&rb);
		if (rb.restype == AcDb::kDxfInt32)
		{
			len1 = rb.resval.rlong;
		}
		else
		{
			filer->pushBackItem();
			filer->setError(Acad::eInvalidDxfCode,
				_T("\nError: expected group code %d"),
				AcDb::kDxfInt32);
			return filer->filerStatus();
		}

		Adesk::Int32 i;
		for(i = 0; i < len1; ++i)
		{
			t_PolylineVertex ptVertex;

			filer->readItem(&rb);
			if (rb.restype == AcDb::kDxfInt16)
			{
				ptVertex.m_vertexFlag = rb.resval.rint;
			}
			else
			{
				filer->pushBackItem();
				filer->setError(Acad::eInvalidDxfCode,
					_T("\nError: expected group code %d"),
					AcDb::kDxfInt16);
				return filer->filerStatus();
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

			m_vertexArrOut.push_back(ptVertex);
		}

		filer->readItem(&rb);
		if (rb.restype == AcDb::kDxfInt32)
		{
			len1 = rb.resval.rlong;
		}
		else
		{
			filer->pushBackItem();
			filer->setError(Acad::eInvalidDxfCode,
				_T("\nError: expected group code %d"),
				AcDb::kDxfInt32);
			return filer->filerStatus();
		}
		for(i = 0; i < len1; ++i)
		{
			std::vector<t_PolylineVertex> inPtArr;
			filer->readItem(&rb);
			if (rb.restype == AcDb::kDxfInt32)
			{
				len2 = rb.resval.rlong;
			}
			else
			{
				filer->pushBackItem();
				filer->setError(Acad::eInvalidDxfCode,
					_T("\nError: expected group code %d"),
					AcDb::kDxfInt32);
				return filer->filerStatus();
			}
			for(Adesk::Int32 j = 0; j < len2; ++j)
			{
				t_PolylineVertex ptVertex;

				filer->readItem(&rb);
				if (rb.restype == AcDb::kDxfInt16)
				{
					ptVertex.m_vertexFlag = rb.resval.rint;
				}
				else
				{
					filer->pushBackItem();
					filer->setError(Acad::eInvalidDxfCode,
						_T("\nError: expected group code %d"),
						AcDb::kDxfInt16);
					return filer->filerStatus();
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

				inPtArr.push_back(ptVertex);
			}
			m_vertexArrArrIn.push_back(inPtArr);
		}
	}
	if(version >= 1)
    {
      filer->readItem(&rb);
		  if (rb.restype == AcDb::kDxfXCoord)
		  {
			  m_ptOri.set(rb.resval.rpoint[0], rb.resval.rpoint[1], rb.resval.rpoint[2]);
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
			  m_vecV1.set(rb.resval.rpoint[0], rb.resval.rpoint[1], rb.resval.rpoint[2]);
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
			  m_vecV2.set(rb.resval.rpoint[0], rb.resval.rpoint[1], rb.resval.rpoint[2]);
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
		  if (rb.restype == AcDb::kDxfXCoord + 3)
		  {
			  m_vecVh.set(rb.resval.rpoint[0], rb.resval.rpoint[1], rb.resval.rpoint[2]);
		  }
		  else
		  {
			  filer->pushBackItem();
			  filer->setError(Acad::eInvalidDxfCode,
				  _T("\nError: expected group code %d"),
				  AcDb::kDxfXCoord + 3);
			  return filer->filerStatus();
		  }

      filer->readItem(&rb);
		  if (rb.restype == AcDb::kDxfReal)
		  {
			  m_dH = rb.resval.rreal;
		  }
		  else
		  {
			  filer->pushBackItem();
			  filer->setError(Acad::eInvalidDxfCode,
				  _T("\nError: expected group code %d"),
				  AcDb::kDxfReal);
			  return filer->filerStatus();
		  }

      AcGeDxfIO::inFields(filer, m_ptarrOut);

      while(!m_ptarrarrIn.empty())
        m_ptarrarrIn.pop_back();
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
          AcGePoint3dArray ptarr;
          AcGeDxfIO::inFields(filer, ptarr);
          m_ptarrarrIn.push_back(ptarr);
        }

		if(version == 1)
		{
			while(!m_vertexArrOut.empty())
				m_vertexArrOut.pop_back();
			while(!m_vertexArrArrIn.empty())
				m_vertexArrArrIn.pop_back();

			size_t i;
			int j;
			for(j = 0; j < m_ptarrOut.length(); j++)
			{
				t_PolylineVertex ptTemp;
				if(j == m_ptarrOut.length() - 1)
					ptTemp.m_vertexFlag = POLYLINEVERTE_FLAG_LC;
				else
					ptTemp.m_vertexFlag = POLYLINEVERTE_FLAG_L;
				ptTemp.m_vertex = m_ptarrOut[j];
				m_vertexArrOut.push_back(ptTemp);   // 外环的顶点
			}
			for(i = 0; i < m_ptarrarrIn.size(); i++)
			{
				std::vector<t_PolylineVertex> inPtArr;
				for(int j = 0; j < m_ptarrarrIn[i].length(); j++)
				{
					t_PolylineVertex ptTemp;
					if(j == m_ptarrarrIn[i].length() - 1)
						ptTemp.m_vertexFlag = POLYLINEVERTE_FLAG_LC;
					else
						ptTemp.m_vertexFlag = POLYLINEVERTE_FLAG_L;
					ptTemp.m_vertex = m_ptarrarrIn[i][j];
					inPtArr.push_back(ptTemp);   // 内环的顶点
				}
				m_vertexArrArrIn.push_back(inPtArr);
			}
		}
	  }

    //if(m_3dGeom.isNull())
    //    createBody();
	GetMaxLen();

    return filer->filerStatus();
}

#ifdef _OBJECTARX2010_
Adesk::Boolean PDSpolygon::subWorldDraw(AcGiWorldDraw* mode)
#else
Adesk::Boolean PDSpolygon::worldDraw(AcGiWorldDraw* mode)
#endif
{
    assertReadEnabled();
    if (mode->regenAbort()) {
        return Adesk::kTrue;
    }

    //if(acdbHostApplicationServices()->workingDatabase()->useri1() == 1)
	//	return Adesk::kTrue;
	int Precision = m_dDividPrecision;
	Adesk::Int16 nRating = getCalMaxPrecision();
	if(nRating == PRIMARY3D_SIMPLIZE_RATE1 ||
		nRating == PRIMARY3D_SIMPLIZE_RATE2)
	{
		if(m_vertexArrOut.size() > 0)
		{
			Precision = (int)	(PI / acos(1.0 - mode->deviation(kAcGiMaxDevForCircle, m_vertexArrOut[0].m_vertex)* 2.0 / m_dMaxLength));
			if(Precision < PRIMARY3D_SIMPLIZE_SCALE)
			{
				AcGePoint3d pts[2];
				AcGeVector3d offset = m_vecVh.normal() * m_dH;
				size_t  i =0, j = 0;
				AcGePoint3dArray pts1,pts2;
				for (i =0; i < m_vertexArrOut.size(); i++)
				{
					pts[0] = m_vertexArrOut[i].m_vertex;
					pts1.append(pts[0]);
					pts[1] = pts[0] + offset;
					pts2.append(pts[1]);
					if(mode->geometry().polyline(2,pts))
						return Adesk::kTrue;
				}
				pts1.append(pts1.first());
				pts2.append(pts2.first());
				if (pts1.length() > 0 && pts2.length() > 0)
				{
					if (mode->geometry().polyline(pts1.length(),pts1.asArrayPtr()))
					{
						return Adesk::kTrue;
					}
					if (mode->geometry().polyline(pts2.length(),pts2.asArrayPtr()))
					{
						return Adesk::kTrue;
					}
				}
				/*for (i = 0 ; i < m_vertexArrArrIn.size(); i ++)
				{
					pts1.setLogicalLength(0);
					pts2.setLogicalLength(0);
					for (j = 0 ;j <m_vertexArrArrIn[i].size(); j++)
					{
						pts[0]= m_vertexArrArrIn[i][j].m_vertex;
						pts1.append(pts[0]);
						pts[1] = pts[0] + offset;
						pts2.append(pts[1]);
						if (mode->geometry().polyline(2,pts))
						{
							return Adesk::kTrue;
						}
					}
					pts1.append(pts1.first());
					pts2.append(pts2.first());
					if (pts1.length() > 0 && pts2.length() > 0)
					{
						if (mode->geometry().polyline(pts1.length(),pts1.asArrayPtr()))
						{
							return Adesk::kTrue;
						}
						if (mode->geometry().polyline(pts2.length(),pts2.asArrayPtr()))
						{
							return Adesk::kTrue;
						}
					}
				}*/
				return Adesk::kTrue;
			}
		}
	}

    //PDPrimary3D::worldDraw(mode);
    
    if(m_vertexArrArrIn.size() <= 0)
    {
        std::vector<t_PolylineVertex>  OutUpPtarr;   // 外环的顶点(顶面)

        AcGeVector3d extrudeDistVec = m_vecVh * m_dH;

		size_t i;
        for(i = 0; i < m_vertexArrOut.size(); i++)
        {
            t_PolylineVertex vertexTemp = m_vertexArrOut[i];
            vertexTemp.m_vertex += extrudeDistVec;
            OutUpPtarr.push_back(vertexTemp);
        }
	
        int gsIndex = 0;
#ifndef _ALWAYSSHOWWIRE_
        switch (mode->regenType())
        {
        case kAcGiHideOrShadeCommand:
        case kAcGiRenderCommand:
#endif
			{
				mode->subEntityTraits().setFillType(kAcGiFillAlways);
				//下底面
				if(drawPolylineVertex(mode, m_vertexArrOut, gsIndex, Precision))
					return Adesk::kTrue;
				//上底面
				if(drawPolylineVertex(mode, OutUpPtarr, gsIndex, Precision))
					return Adesk::kTrue;
				//侧面
				AcGePoint3d pt[4];
				for(i = 0; i < m_vertexArrOut.size(); i++)
				{
					switch(m_vertexArrOut[i].m_vertexFlag)
					{
					case POLYLINEVERTE_FLAG_L:
						if(i == m_vertexArrOut.size() - 1)
						{
							pt[0] = m_vertexArrOut[i].m_vertex;
							pt[1] = m_vertexArrOut[0].m_vertex;
							pt[2] = OutUpPtarr[0].m_vertex;
							pt[3] = OutUpPtarr[i].m_vertex;
							if(mode->geometry().polygon(4, pt))
								return Adesk::kTrue;
						}
						else
						{
							pt[0] = m_vertexArrOut[i].m_vertex;
							pt[1] = m_vertexArrOut[i + 1].m_vertex;
							pt[2] = OutUpPtarr[i + 1].m_vertex;
							pt[3] = OutUpPtarr[i].m_vertex;
							if(mode->geometry().polygon(4, pt))
								return Adesk::kTrue;
						}
						break;
					case POLYLINEVERTE_FLAG_A:
						if(i == m_vertexArrOut.size() - 2)
						{
							AcGeCircArc3d geArc1(m_vertexArrOut[i].m_vertex, m_vertexArrOut[i + 1].m_vertex, m_vertexArrOut[0].m_vertex);
							AcGeCircArc3d geArc2(OutUpPtarr[i].m_vertex, OutUpPtarr[i + 1].m_vertex, OutUpPtarr[0].m_vertex);
							double sp1, ep1, deltaParam1 = 0;
							double sp2, ep2, deltaParam2 = 0;
							sp1 = geArc1.paramOf(m_vertexArrOut[i].m_vertex);
							ep1 = geArc1.paramOf(m_vertexArrOut[0].m_vertex);
							deltaParam1 = (ep1 - sp1) / Precision;
							sp2 = geArc2.paramOf(OutUpPtarr[i].m_vertex);
							ep2 = geArc2.paramOf(OutUpPtarr[0].m_vertex);
							deltaParam2 = (ep2 - sp2) / Precision;
							if(deltaParam1 != 0)
							{
								AcGePoint3dArray meshPts;
								double paramTemp = sp1;
								int j;
								for(j = 0; j <= Precision; ++j)
								{
									meshPts.append(geArc1.evalPoint(paramTemp));
									paramTemp += deltaParam1;
								}
								paramTemp = sp2;
								for(j = 0; j <= Precision; ++j)
								{
									meshPts.append(geArc2.evalPoint(paramTemp));
									paramTemp += deltaParam2;
								}
								if(mode->geometry().mesh(2, Precision + 1, meshPts.asArrayPtr()))
									return Adesk::kTrue;
							}
						}
						else if(i < m_vertexArrOut.size() - 2)
						{
							AcGeCircArc3d geArc1(m_vertexArrOut[i].m_vertex, m_vertexArrOut[i + 1].m_vertex, m_vertexArrOut[i + 2].m_vertex);
							AcGeCircArc3d geArc2(OutUpPtarr[i].m_vertex, OutUpPtarr[i + 1].m_vertex, OutUpPtarr[i + 2].m_vertex);
							double sp1, ep1, deltaParam1 = 0;
							double sp2, ep2, deltaParam2 = 0;
							sp1 = geArc1.paramOf(m_vertexArrOut[i].m_vertex);
							ep1 = geArc1.paramOf(m_vertexArrOut[i + 2].m_vertex);
							deltaParam1 = (ep1 - sp1) / Precision;
							sp2 = geArc2.paramOf(OutUpPtarr[i].m_vertex);
							ep2 = geArc2.paramOf(OutUpPtarr[i + 2].m_vertex);
							deltaParam2 = (ep2 - sp2) / Precision;
							if(deltaParam1 != 0)
							{
								AcGePoint3dArray meshPts;
								double paramTemp = sp1;
								int j;
								for(j = 0; j <= Precision; ++j)
								{
									meshPts.append(geArc1.evalPoint(paramTemp));
									paramTemp += deltaParam1;
								}
								paramTemp = sp2;
								for(j = 0; j <= Precision; ++j)
								{
									meshPts.append(geArc2.evalPoint(paramTemp));
									paramTemp += deltaParam2;
								}
								if(mode->geometry().mesh(2, Precision + 1, meshPts.asArrayPtr()))
									return Adesk::kTrue;
							}
						}
						i += 1;
						break;
					case POLYLINEVERTE_FLAG_AM:
						break;
					case POLYLINEVERTE_FLAG_LC:
						pt[0] = m_vertexArrOut[i].m_vertex;
						pt[1] = m_vertexArrOut[0].m_vertex;
						pt[2] = OutUpPtarr[0].m_vertex;
						pt[3] = OutUpPtarr[i].m_vertex;
						if(mode->geometry().polygon(4, pt))
							return Adesk::kTrue;
						i = m_vertexArrOut.size() - 1;
						break;
					case POLYLINEVERTE_FLAG_AC:
						if(i <= m_vertexArrOut.size() - 2)
						{
							AcGeCircArc3d geArc1(m_vertexArrOut[i].m_vertex, m_vertexArrOut[i + 1].m_vertex, m_vertexArrOut[0].m_vertex);
							AcGeCircArc3d geArc2(OutUpPtarr[i].m_vertex, OutUpPtarr[i + 1].m_vertex, OutUpPtarr[0].m_vertex);
							double sp1, ep1, deltaParam1 = 0;
							double sp2, ep2, deltaParam2 = 0;
							sp1 = geArc1.paramOf(m_vertexArrOut[i].m_vertex);
							ep1 = geArc1.paramOf(m_vertexArrOut[0].m_vertex);
							deltaParam1 = (ep1 - sp1) / Precision;
							sp2 = geArc2.paramOf(OutUpPtarr[i].m_vertex);
							ep2 = geArc2.paramOf(OutUpPtarr[0].m_vertex);
							deltaParam2 = (ep2 - sp2) / Precision;
							if(deltaParam1 != 0)
							{
								AcGePoint3dArray meshPts;
								double paramTemp = sp1;
								int j;
								for(j = 0; j <= Precision; ++j)
								{
									meshPts.append(geArc1.evalPoint(paramTemp));
									paramTemp += deltaParam1;
								}
								paramTemp = sp2;
								for(j = 0; j <= Precision; ++j)
								{
									meshPts.append(geArc2.evalPoint(paramTemp));
									paramTemp += deltaParam2;
								}
								if(mode->geometry().mesh(2, Precision + 1, meshPts.asArrayPtr()))
									return Adesk::kTrue;
							}
						}
						i = m_vertexArrOut.size() - 1;
						break;
					case POLYLINEVERTE_FLAG_C:
						if(i == 0 && m_vertexArrOut.size() >= 3)
						{
							AcGeCircArc3d geArc1(m_vertexArrOut[i].m_vertex, m_vertexArrOut[i + 1].m_vertex, m_vertexArrOut[i + 2].m_vertex);
							AcGeCircArc3d geArc2(OutUpPtarr[i].m_vertex, OutUpPtarr[i + 1].m_vertex, OutUpPtarr[i + 2].m_vertex);
							AcGeCircArc3d geCir1(geArc1.center(), geArc1.normal(), geArc1.radius());
							AcGeCircArc3d geCir2(geArc2.center(), geArc2.normal(), geArc2.radius());
							double sp = 0, deltaParam = 0;
							deltaParam = PI * 2.0 / Precision;
							if(deltaParam != 0)
							{
								AcGePoint3dArray meshPts;
								double paramTemp = sp;
								int j;
								for(j = 0; j <= Precision; ++j)
								{
									meshPts.append(geArc1.evalPoint(paramTemp));
									paramTemp += deltaParam;
								}
								paramTemp = sp;
								for(j = 0; j <= Precision; ++j)
								{
									meshPts.append(geArc2.evalPoint(paramTemp));
									paramTemp += deltaParam;
								}
								if(mode->geometry().mesh(2, Precision + 1, meshPts.asArrayPtr()))
									return Adesk::kTrue;
							}
						}
						i = m_vertexArrOut.size() - 1;
						break;
					default:
						break;
					}
				}
			}
#ifndef _ALWAYSSHOWWIRE_
            break;
		case kAcGiStandardDisplay:
		case kAcGiSaveWorldDrawForR12:
		case kAcGiSaveWorldDrawForProxy:
#endif
			{
				AcGePoint3d pt[2];

				//下底面的边(外环)
				if(drawPolylineVertex(mode, m_vertexArrOut, gsIndex, Precision))
					return Adesk::kTrue;
				//上底面的边(外环)
				if(drawPolylineVertex(mode, OutUpPtarr, gsIndex, Precision))
					return Adesk::kTrue;

				//侧面的边(外环)
				for(i = 0; i < m_vertexArrOut.size(); i++)
				{
					pt[0] = m_vertexArrOut[i].m_vertex;
					pt[1] = OutUpPtarr[i].m_vertex;
					mode->subEntityTraits().setSelectionMarker(++gsIndex);
					mode->geometry().polyline(2, pt);
				}
			}
#ifndef _ALWAYSSHOWWIRE_
			break;
        }
#endif
    }
    else
    {
        createBody();
#ifdef _OBJECTARX2010_
		PDPrimary3D::subWorldDraw(mode);
#else
        PDPrimary3D::worldDraw(mode);
#endif
    }

	return Adesk::kTrue;
}

Adesk::Boolean PDSpolygon::drawPolylineVertex(AcGiWorldDraw* mode, 
                                 const std::vector<t_PolylineVertex> &vertexArr, 
                                 int &gsIndex, 
                                 int Precision) const
{
    Adesk::Boolean ret = Adesk::kFalse;
    switch (mode->regenType())
    {
    case kAcGiStandardDisplay:
    case kAcGiSaveWorldDrawForR12:
    case kAcGiSaveWorldDrawForProxy:
        {
            AcGePoint3d pt[2];
            for(size_t i = 0; i < vertexArr.size(); i++)
            {
                switch(vertexArr[i].m_vertexFlag)
                {
                case POLYLINEVERTE_FLAG_L:
                    if(i == vertexArr.size() - 1)
                    {
                        pt[0] = vertexArr[i].m_vertex;
                        pt[1] = vertexArr[0].m_vertex;
                        mode->subEntityTraits().setSelectionMarker(++gsIndex);
                        if(ret = mode->geometry().polyline(2, pt))
                            return ret;
                    }
                    else
                    {
                        pt[0] = vertexArr[i].m_vertex;
                        pt[1] = vertexArr[i + 1].m_vertex;
                        mode->subEntityTraits().setSelectionMarker(++gsIndex);
                        if(ret = mode->geometry().polyline(2, pt))
                            return ret;
                    }
                    break;
                case POLYLINEVERTE_FLAG_A:
                    if(i == vertexArr.size() - 2)
                    {
                        mode->subEntityTraits().setSelectionMarker(++gsIndex);
                        if(ret = mode->geometry().circularArc(vertexArr[i].m_vertex, vertexArr[i + 1].m_vertex, vertexArr[0].m_vertex))
                            return ret;
                    }
                    else if(i < vertexArr.size() - 2)
                    {
                        mode->subEntityTraits().setSelectionMarker(++gsIndex);
                        if(ret = mode->geometry().circularArc(vertexArr[i].m_vertex, vertexArr[i + 1].m_vertex, vertexArr[i + 2].m_vertex))
                            return ret;
                    }
                    i += 1;
                    break;
                case POLYLINEVERTE_FLAG_AM:
                    break;
                case POLYLINEVERTE_FLAG_LC:
                    pt[0] = vertexArr[i].m_vertex;
                    pt[1] = vertexArr[0].m_vertex;
                    mode->subEntityTraits().setSelectionMarker(++gsIndex);
                    if(ret = mode->geometry().polyline(2, pt))
                        return ret;
                    i = vertexArr.size() - 1;
                    break;
                case POLYLINEVERTE_FLAG_AC:
                    if(i <= vertexArr.size() - 2)
                    {
                        mode->subEntityTraits().setSelectionMarker(++gsIndex);
                        if(ret = mode->geometry().circularArc(vertexArr[i].m_vertex, vertexArr[i + 1].m_vertex, vertexArr[0].m_vertex))
                            return ret;
                    }
                    i = vertexArr.size() - 1;
                    break;
                case POLYLINEVERTE_FLAG_C:
                    if(i == 0 && vertexArr.size() >= 3)
                    {
                        mode->subEntityTraits().setSelectionMarker(++gsIndex);
                        if(ret = mode->geometry().circle(vertexArr[i].m_vertex, vertexArr[i + 1].m_vertex, vertexArr[i + 2].m_vertex))
                            return ret;
                    }
                    i = vertexArr.size() - 1;
                    break;
                default:
                    break;
                }
            }
        }
        break;
    case kAcGiHideOrShadeCommand:
    case kAcGiRenderCommand:
        {
			mode->subEntityTraits().setFillType(kAcGiFillAlways);
            AcGePoint3dArray shellPts;
            for(size_t i = 0; i < vertexArr.size(); i++)
            {
                switch(vertexArr[i].m_vertexFlag)
                {
                case POLYLINEVERTE_FLAG_L:
                    shellPts.append(vertexArr[i].m_vertex);
                    break;
                case POLYLINEVERTE_FLAG_A:
                    if(i == vertexArr.size() - 2)
                    {
                        AcGeCircArc3d geArc(vertexArr[i].m_vertex, vertexArr[i + 1].m_vertex, vertexArr[0].m_vertex);
                        double sp, ep, deltaParam = 0;
                        sp = geArc.paramOf(vertexArr[i].m_vertex);
                        ep = geArc.paramOf(vertexArr[0].m_vertex);
                        deltaParam = (ep - sp) / Precision;
                        if(deltaParam != 0)
                        {
                            double paramTemp = sp;
                            for(int j = 0; j < Precision; ++j)
                            {
                                shellPts.append(geArc.evalPoint(paramTemp));
                                paramTemp += deltaParam;
                            }
                        }
                    }
                    else if(i < vertexArr.size() - 2)
                    {
                        AcGeCircArc3d geArc(vertexArr[i].m_vertex, vertexArr[i + 1].m_vertex, vertexArr[i + 2].m_vertex);
                        double sp, ep, deltaParam = 0;
                        sp = geArc.paramOf(vertexArr[i].m_vertex);
                        ep = geArc.paramOf(vertexArr[i + 2].m_vertex);
                        deltaParam = (ep - sp) / Precision;
                        if(deltaParam != 0)
                        {
                            double paramTemp = sp;
                            for(int j = 0; j < Precision; ++j)
                            {
                                shellPts.append(geArc.evalPoint(paramTemp));
                                paramTemp += deltaParam;
                            }
                        }
                    }
                    i += 1;
                    break;
                case POLYLINEVERTE_FLAG_AM:
                    break;
                case POLYLINEVERTE_FLAG_LC:
                    shellPts.append(vertexArr[i].m_vertex);
                    i = vertexArr.size() - 1;
                    break;
                case POLYLINEVERTE_FLAG_AC:
                    if(i <= vertexArr.size() - 2)
                    {
                        AcGeCircArc3d geArc(vertexArr[i].m_vertex, vertexArr[i + 1].m_vertex, vertexArr[0].m_vertex);
                        double sp, ep, deltaParam = 0;
                        sp = geArc.paramOf(vertexArr[i].m_vertex);
                        ep = geArc.paramOf(vertexArr[0].m_vertex);
                        deltaParam = (ep - sp) / Precision;
                        if(deltaParam != 0)
                        {
                            double paramTemp = sp;
                            for(int j = 0; j < Precision; ++j)
                            {
                                shellPts.append(geArc.evalPoint(paramTemp));
                                paramTemp += deltaParam;
                            }
                        }
                    }
                    i = vertexArr.size() - 1;
                    break;
                case POLYLINEVERTE_FLAG_C:
                    if(i == 0 && vertexArr.size() >= 3)
                    {
                        AcGeCircArc3d geArc(vertexArr[i].m_vertex, vertexArr[i + 1].m_vertex, vertexArr[i + 2].m_vertex);
                        AcGeCircArc3d geCir(geArc.center(), geArc.normal(), geArc.radius());
                        double sp = 0, deltaParam = 0;
                        deltaParam = PI * 2.0 / Precision;
                        if(deltaParam != 0)
                        {
                            double paramTemp = sp;
                            for(int j = 0; j < Precision; ++j)
                            {
                                shellPts.append(geCir.evalPoint(paramTemp));
                                paramTemp += deltaParam;
                            }
                        }
                    }
                    i = vertexArr.size() - 1;
                    break;
                default:
                    break;
                }
            }
            if(ret = mode->geometry().polygon(shellPts.length(), shellPts.asArrayPtr()))
                return ret;
        }
        break;
    }
    return ret;
}

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDSpolygon::subGetOsnapPoints(
	AcDb::OsnapMode     osnapMode,
	Adesk::GsMarker     gsSelectionMark,
	const AcGePoint3d&  pickPoint,
	const AcGePoint3d&  lastPoint,
	const AcGeMatrix3d& viewXform,
	AcGePoint3dArray&   snapPoints,
	AcDbIntArray &   geomIds) const
#else
Acad::ErrorStatus PDSpolygon::getOsnapPoints(
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

    //if(gsSelectionMark == 0)
    //    return Acad::eOk;

    AcGeVector3d viewDir(viewXform(Z, 0), viewXform(Z, 1),
                viewXform(Z, 2));

    std::vector<AcGePoint3d>          OutUpPtarr;   // 外环的顶点(顶面)
	std::vector< std::vector<AcGePoint3d> >  InUpPtarrarr; // 内环的顶点(顶面)

    AcGeVector3d extrudeDistVec = m_vecVh * m_dH;

    size_t i;
    for(i = 0; i < m_vertexArrOut.size(); i++)
    {
        AcGePoint3d ptTemp = m_vertexArrOut[i].m_vertex;
        ptTemp += extrudeDistVec;
        OutUpPtarr.push_back(ptTemp);
    }
    for(i = 0; i < m_vertexArrArrIn.size(); i++)
    {
        std::vector<AcGePoint3d> ptarr;
        for(size_t j = 0; j < m_vertexArrArrIn[i].size(); j++)
        {
            AcGePoint3d ptTemp = m_vertexArrArrIn[i][j].m_vertex;
            ptTemp += extrudeDistVec;
            ptarr.push_back(ptTemp);
        }
        InUpPtarrarr.push_back(ptarr);
    }

    switch(osnapMode)
    {
    case AcDb::kOsModeEnd:
        for(i = 0; i < m_vertexArrOut.size(); i++)
            snapPoints.append(m_vertexArrOut[i].m_vertex);
        for(i = 0; i < m_vertexArrArrIn.size(); i++)
            for(size_t j = 0; j < m_vertexArrArrIn[i].size(); ++j)
                snapPoints.append(m_vertexArrArrIn[i][j].m_vertex);
		for(i = 0; i < OutUpPtarr.size(); ++i)
			snapPoints.append(OutUpPtarr[i]);
        for(i = 0; i < InUpPtarrarr.size(); i++)
			for(size_t j = 0; j < InUpPtarrarr[i].size(); ++j)
				snapPoints.append(InUpPtarrarr[i][j]);
        break;
    case AcDb::kOsModeMid:
        {
            AcGePoint3d pt;
            //下上底面边的中点
            for(i = 0; i < m_vertexArrOut.size(); i++)
            {
                switch(m_vertexArrOut[i].m_vertexFlag)
                {
                case POLYLINEVERTE_FLAG_L:
                    if(i == m_vertexArrOut.size() - 1)
                        pt = m_vertexArrOut[i].m_vertex + (m_vertexArrOut[0].m_vertex - m_vertexArrOut[i].m_vertex) / 2.0;
                    else
                        pt = m_vertexArrOut[i].m_vertex + (m_vertexArrOut[i + 1].m_vertex - m_vertexArrOut[i].m_vertex) / 2.0;
                    snapPoints.append(pt);
                    if(i == m_vertexArrOut.size() - 1)
                        pt = OutUpPtarr[i] + (OutUpPtarr[0] - OutUpPtarr[i]) / 2.0;
                    else
                        pt = OutUpPtarr[i] + (OutUpPtarr[i + 1] - OutUpPtarr[i]) / 2.0;
                    snapPoints.append(pt);
                    break;
                case POLYLINEVERTE_FLAG_A:
                    if(i == m_vertexArrOut.size() - 2)
                    {
                        AcGeCircArc3d geArc(m_vertexArrOut[i].m_vertex, m_vertexArrOut[i + 1].m_vertex, m_vertexArrOut[0].m_vertex);
                        double sp, ep;
                        sp = geArc.paramOf(m_vertexArrOut[i].m_vertex);
                        ep = geArc.paramOf(m_vertexArrOut[0].m_vertex);
                        snapPoints.append(geArc.evalPoint(sp + (ep - sp) / 2.0));
                        geArc.set(OutUpPtarr[i], OutUpPtarr[i + 1], OutUpPtarr[0]);
                        sp = geArc.paramOf(OutUpPtarr[i]);
                        ep = geArc.paramOf(OutUpPtarr[0]);
                        snapPoints.append(geArc.evalPoint(sp + (ep - sp) / 2.0));
                    }
                    else if(i < m_vertexArrOut.size() - 2)
                    {
                        AcGeCircArc3d geArc(m_vertexArrOut[i].m_vertex, m_vertexArrOut[i + 1].m_vertex, m_vertexArrOut[i + 2].m_vertex);
                        double sp, ep;
                        sp = geArc.paramOf(m_vertexArrOut[i].m_vertex);
                        ep = geArc.paramOf(m_vertexArrOut[i + 2].m_vertex);
                        snapPoints.append(geArc.evalPoint(sp + (ep - sp) / 2.0));
                        geArc.set(OutUpPtarr[i], OutUpPtarr[i + 1], OutUpPtarr[i + 2]);
                        sp = geArc.paramOf(OutUpPtarr[i]);
                        ep = geArc.paramOf(OutUpPtarr[i + 2]);
                        snapPoints.append(geArc.evalPoint(sp + (ep - sp) / 2.0));
                    }
                    i += 1;
                    break;
                case POLYLINEVERTE_FLAG_AM:
                    break;
                case POLYLINEVERTE_FLAG_LC:
                    pt = m_vertexArrOut[i].m_vertex + (m_vertexArrOut[0].m_vertex - m_vertexArrOut[i].m_vertex) / 2.0;
                    snapPoints.append(pt);
                    pt = OutUpPtarr[i] + (OutUpPtarr[0] - OutUpPtarr[i]) / 2.0;
                    snapPoints.append(pt);
                    i = (int)m_vertexArrOut.size() - 1;
                    break;
                case POLYLINEVERTE_FLAG_AC:
                    if(i <= m_vertexArrOut.size() - 2)
                    {
                        AcGeCircArc3d geArc(m_vertexArrOut[i].m_vertex, m_vertexArrOut[i + 1].m_vertex, m_vertexArrOut[0].m_vertex);
                        double sp, ep;
                        sp = geArc.paramOf(m_vertexArrOut[i].m_vertex);
                        ep = geArc.paramOf(m_vertexArrOut[0].m_vertex);
                        snapPoints.append(geArc.evalPoint(sp + (ep - sp) / 2.0));
                        geArc.set(OutUpPtarr[i], OutUpPtarr[i + 1], OutUpPtarr[0]);
                        sp = geArc.paramOf(OutUpPtarr[i]);
                        ep = geArc.paramOf(OutUpPtarr[0]);
                        snapPoints.append(geArc.evalPoint(sp + (ep - sp) / 2.0));
                    }
                    i = (int)m_vertexArrOut.size() - 1;
                    break;
                case POLYLINEVERTE_FLAG_C:
                    i = (int)m_vertexArrOut.size() - 1;
                    break;
                default:
                    break;
                }
            }
			size_t j;
            for(j = 0; j < m_vertexArrArrIn.size(); j++)
            {
                for(i = 0; i < m_vertexArrArrIn[j].size(); i++)
                {
                    switch(m_vertexArrArrIn[j][i].m_vertexFlag)
                    {
                    case POLYLINEVERTE_FLAG_L:
                        if(i == m_vertexArrArrIn[j].size() - 1)
                            pt = m_vertexArrArrIn[j][i].m_vertex + (m_vertexArrArrIn[j][0].m_vertex - m_vertexArrArrIn[j][i].m_vertex) / 2.0;
                        else
                            pt = m_vertexArrArrIn[j][i].m_vertex + (m_vertexArrArrIn[j][i + 1].m_vertex - m_vertexArrArrIn[j][i].m_vertex) / 2.0;
                        snapPoints.append(pt);
                        if(i == m_vertexArrArrIn[j].size() - 1)
                            pt = InUpPtarrarr[j][i] + (InUpPtarrarr[j][0] - InUpPtarrarr[j][i]) / 2.0;
                        else
                            pt = InUpPtarrarr[j][i] + (InUpPtarrarr[j][i + 1] - InUpPtarrarr[j][i]) / 2.0;
                        snapPoints.append(pt);
                        break;
                    case POLYLINEVERTE_FLAG_A:
                        if(i == m_vertexArrArrIn[j].size() - 2)
                        {
                            AcGeCircArc3d geArc(m_vertexArrArrIn[j][i].m_vertex, m_vertexArrArrIn[j][i + 1].m_vertex, m_vertexArrArrIn[j][0].m_vertex);
                            double sp, ep;
                            sp = geArc.paramOf(m_vertexArrArrIn[j][i].m_vertex);
                            ep = geArc.paramOf(m_vertexArrArrIn[j][0].m_vertex);
                            snapPoints.append(geArc.evalPoint(sp + (ep - sp) / 2.0));
                            geArc.set(InUpPtarrarr[j][i], InUpPtarrarr[j][i + 1], InUpPtarrarr[j][0]);
                            sp = geArc.paramOf(InUpPtarrarr[j][i]);
                            ep = geArc.paramOf(InUpPtarrarr[j][0]);
                            snapPoints.append(geArc.evalPoint(sp + (ep - sp) / 2.0));
                        }
                        else if(i < m_vertexArrArrIn[j].size() - 2)
                        {
                            AcGeCircArc3d geArc(m_vertexArrArrIn[j][i].m_vertex, m_vertexArrArrIn[j][i + 1].m_vertex, m_vertexArrArrIn[j][i + 2].m_vertex);
                            double sp, ep;
                            sp = geArc.paramOf(m_vertexArrArrIn[j][i].m_vertex);
                            ep = geArc.paramOf(m_vertexArrArrIn[j][i + 2].m_vertex);
                            snapPoints.append(geArc.evalPoint(sp + (ep - sp) / 2.0));
                            geArc.set(InUpPtarrarr[j][i], InUpPtarrarr[j][i + 1], InUpPtarrarr[j][i + 2]);
                            sp = geArc.paramOf(InUpPtarrarr[j][i]);
                            ep = geArc.paramOf(InUpPtarrarr[j][i + 2]);
                            snapPoints.append(geArc.evalPoint(sp + (ep - sp) / 2.0));
                        }
                        i += 1;
                        break;
                    case POLYLINEVERTE_FLAG_AM:
                        break;
                    case POLYLINEVERTE_FLAG_LC:
                        pt = m_vertexArrArrIn[j][i].m_vertex + (m_vertexArrArrIn[j][0].m_vertex - m_vertexArrArrIn[j][i].m_vertex) / 2.0;
                        snapPoints.append(pt);
                        pt = InUpPtarrarr[j][i] + (InUpPtarrarr[j][0] - InUpPtarrarr[j][i]) / 2.0;
                        snapPoints.append(pt);
                        i = (int)m_vertexArrArrIn[j].size() - 1;
                        break;
                    case POLYLINEVERTE_FLAG_AC:
                        if(i <= m_vertexArrArrIn[j].size() - 2)
                        {
                            AcGeCircArc3d geArc(m_vertexArrArrIn[j][i].m_vertex, m_vertexArrArrIn[j][i + 1].m_vertex, m_vertexArrArrIn[j][0].m_vertex);
                            double sp, ep;
                            sp = geArc.paramOf(m_vertexArrArrIn[j][i].m_vertex);
                            ep = geArc.paramOf(m_vertexArrArrIn[j][0].m_vertex);
                            snapPoints.append(geArc.evalPoint(sp + (ep - sp) / 2.0));
                            geArc.set(InUpPtarrarr[j][i], InUpPtarrarr[j][i + 1], InUpPtarrarr[j][0]);
                            sp = geArc.paramOf(InUpPtarrarr[j][i]);
                            ep = geArc.paramOf(InUpPtarrarr[j][0]);
                            snapPoints.append(geArc.evalPoint(sp + (ep - sp) / 2.0));
                        }
                        i = (int)m_vertexArrArrIn[j].size() - 1;
                        break;
                    case POLYLINEVERTE_FLAG_C:
                        i = (int)m_vertexArrArrIn[j].size() - 1;
                        break;
                    default:
                        break;
                    }
                }
            }
            //棱边的中点
            for(i = 0; i < m_vertexArrOut.size(); i++)
            {
                pt = m_vertexArrOut[i].m_vertex + (OutUpPtarr[i] - m_vertexArrOut[i].m_vertex) / 2.0;
                snapPoints.append(pt);
            }
            for(j = 0; j < m_vertexArrArrIn.size(); j++)
            {
                for(i = 0; i < m_vertexArrArrIn[j].size(); i++)
                {
                    pt = m_vertexArrArrIn[j][i].m_vertex + (InUpPtarrarr[j][i] - m_vertexArrArrIn[j][i].m_vertex) / 2.0;
                    snapPoints.append(pt);
                }
            }
        }
        break;
    case AcDb::kOsModeCen:
        break;
	case AcDb::kOsModeQuad:
		  break;
    case AcDb::kOsModeNode:
		  break;
    case AcDb::kOsModeIns:
		  snapPoints.append(m_ptOri);
		  break;
    case AcDb::kOsModePerp:
/*		{
        // Create a semi-infinite line and find a point on it.
        //
        AcGeLine3d line;
        AcGeVector3d vec;
        AcGePoint3d pt;
        int gsStart = 0;

        //下底面外环
        if(gsSelectionMark < m_ptarrOut.length())
          {
            vec = m_ptarrOut[gsSelectionMark - 1] - m_ptarrOut[gsSelectionMark];
            vec.normalize();
            line.set(m_ptarrOut[gsSelectionMark], vec);
            pt = line.closestPointTo(lastPoint);
            snapPoints.append(pt);
          }
        else if(gsSelectionMark == m_ptarrOut.length())
          {
            vec = m_ptarrOut[gsSelectionMark - 1] - m_ptarrOut[0];
            vec.normalize();
            line.set(m_ptarrOut[0], vec);
            pt = line.closestPointTo(lastPoint);
            snapPoints.append(pt);
          }
        else
          {
            //下底面内环
            gsStart = m_ptarrOut.length();
            int inIndex = 0;
            while((inIndex < m_ptarrarrIn.size()) && (gsSelectionMark - gsStart > m_ptarrarrIn[inIndex].length()))
              gsStart = gsStart + m_ptarrarrIn[inIndex++].length();
            if(inIndex < m_ptarrarrIn.size())
              {
                if(gsSelectionMark - gsStart < m_ptarrarrIn[inIndex].length())
                  {
                    vec = m_ptarrarrIn[inIndex][gsSelectionMark - gsStart - 1] - m_ptarrarrIn[inIndex][gsSelectionMark - gsStart];
                    vec.normalize();
                    line.set(m_ptarrarrIn[inIndex][gsSelectionMark - gsStart], vec);
                    pt = line.closestPointTo(lastPoint);
                    snapPoints.append(pt);
                  }
                else if(gsSelectionMark - gsStart == m_ptarrarrIn[inIndex].length())
                  {
                    vec = m_ptarrarrIn[inIndex][gsSelectionMark - gsStart - 1] - m_ptarrarrIn[inIndex][0];
                    vec.normalize();
                    line.set(m_ptarrarrIn[inIndex][0], vec);
                    pt = line.closestPointTo(lastPoint);
                    snapPoints.append(pt);
                  }
              }
            //上底面外环
            else if(gsSelectionMark - gsStart < OutUpPtarr.length())
              {
                vec = OutUpPtarr[gsSelectionMark - gsStart - 1] - OutUpPtarr[gsSelectionMark - gsStart];
                vec.normalize();
                line.set(OutUpPtarr[gsSelectionMark - gsStart], vec);
                pt = line.closestPointTo(lastPoint);
                snapPoints.append(pt);
              }
            else if(gsSelectionMark - gsStart == OutUpPtarr.length())
              {
                vec = OutUpPtarr[gsSelectionMark - gsStart - 1] - OutUpPtarr[0];
                vec.normalize();
                line.set(OutUpPtarr[0], vec);
                pt = line.closestPointTo(lastPoint);
                snapPoints.append(pt);
              }
            else
              {
                //上底面内环
                gsStart += m_ptarrOut.length();
                inIndex = 0;
                while((inIndex < InUpPtarrarr.size()) && (gsSelectionMark - gsStart > InUpPtarrarr[inIndex].length()))
                  gsStart = gsStart + InUpPtarrarr[inIndex++].length();
                if(inIndex < InUpPtarrarr.size())
                  {
                    if(gsSelectionMark - gsStart < InUpPtarrarr[inIndex].length())
                      {
                        vec = InUpPtarrarr[inIndex][gsSelectionMark - gsStart - 1] - InUpPtarrarr[inIndex][gsSelectionMark - gsStart];
                        vec.normalize();
                        line.set(InUpPtarrarr[inIndex][gsSelectionMark - gsStart], vec);
                        pt = line.closestPointTo(lastPoint);
                        snapPoints.append(pt);
                      }
                    else if(gsSelectionMark - gsStart == InUpPtarrarr[inIndex].length())
                      {
                        vec = InUpPtarrarr[inIndex][gsSelectionMark - gsStart - 1] - InUpPtarrarr[inIndex][0];
                        vec.normalize();
                        line.set(InUpPtarrarr[inIndex][0], vec);
                        pt = line.closestPointTo(lastPoint);
                        snapPoints.append(pt);
                      }
                  }
                // 外环侧面
                else if(gsSelectionMark - gsStart <= m_ptarrOut.length())
                  {
                    vec = m_ptarrOut[gsSelectionMark - gsStart - 1] - OutUpPtarr[gsSelectionMark - gsStart - 1];
                    vec.normalize();
                    line.set(OutUpPtarr[gsSelectionMark - gsStart - 1], vec);
                    pt = line.closestPointTo(lastPoint);
                    snapPoints.append(pt);
                  }
                else
                  {
                    // 内环侧面
                    gsStart += m_ptarrOut.length();
                    inIndex = 0;
                    while((inIndex < m_ptarrarrIn.size()) && (gsSelectionMark - gsStart > m_ptarrarrIn[inIndex].length()))
                      gsStart = gsStart + m_ptarrarrIn[inIndex++].length();
                    if(inIndex < m_ptarrarrIn.size())
                      {
                        if(gsSelectionMark - gsStart <= m_ptarrarrIn[inIndex].length())
                          {
                            vec = m_ptarrarrIn[inIndex][gsSelectionMark - gsStart - 1] - InUpPtarrarr[inIndex][gsSelectionMark - gsStart - 1];
                            vec.normalize();
                            line.set(InUpPtarrarr[inIndex][gsSelectionMark - gsStart - 1], vec);
                            pt = line.closestPointTo(lastPoint);
                            snapPoints.append(pt);
                          }
                      }
                  }
              }
          }
      }*/
	    break;
    case AcDb::kOsModeTan:
		  break;
    case AcDb::kOsModeNear:
		{
			AcGePoint3d pt;
			//下上底面边
			for(i = 0; i < m_vertexArrOut.size(); i++)
			{
				switch(m_vertexArrOut[i].m_vertexFlag)
				{
				case POLYLINEVERTE_FLAG_L:
					{
						AcGeLineSeg3d lnsg;
						AcGePoint3d closedPt;
						if(i == m_vertexArrOut.size() - 1)
							lnsg.set(m_vertexArrOut[i].m_vertex, m_vertexArrOut[0].m_vertex);
						else
							lnsg.set(m_vertexArrOut[i].m_vertex, m_vertexArrOut[i + 1].m_vertex);
						closedPt = lnsg.projClosestPointTo(pickPoint, viewDir);
						snapPoints.append(closedPt);

						if(i == m_vertexArrOut.size() - 1)
							lnsg.set(OutUpPtarr[i], OutUpPtarr[0]);
						else
							lnsg.set(OutUpPtarr[i], OutUpPtarr[i + 1]);
						closedPt = lnsg.projClosestPointTo(pickPoint, viewDir);
						snapPoints.append(closedPt);
					}
					break;
				case POLYLINEVERTE_FLAG_A:
					if(i == m_vertexArrOut.size() - 2)
					{
						AcGeCircArc3d geArc(m_vertexArrOut[i].m_vertex, m_vertexArrOut[i + 1].m_vertex, m_vertexArrOut[0].m_vertex);
						AcGePoint3d closedPt;
						closedPt = geArc.projClosestPointTo(pickPoint, viewDir);
						snapPoints.append(closedPt);
						geArc.set(OutUpPtarr[i], OutUpPtarr[i + 1], OutUpPtarr[0]);
						closedPt = geArc.projClosestPointTo(pickPoint, viewDir);
						snapPoints.append(closedPt);
					}
					else if(i < m_vertexArrOut.size() - 2)
					{
						AcGeCircArc3d geArc(m_vertexArrOut[i].m_vertex, m_vertexArrOut[i + 1].m_vertex, m_vertexArrOut[i + 2].m_vertex);
						AcGePoint3d closedPt;
						closedPt = geArc.projClosestPointTo(pickPoint, viewDir);
						snapPoints.append(closedPt);
						geArc.set(OutUpPtarr[i], OutUpPtarr[i + 1], OutUpPtarr[i + 2]);
						closedPt = geArc.projClosestPointTo(pickPoint, viewDir);
						snapPoints.append(closedPt);
					}
					i += 1;
					break;
				case POLYLINEVERTE_FLAG_AM:
					break;
				case POLYLINEVERTE_FLAG_LC:
					{
						AcGeLineSeg3d lnsg;
						AcGePoint3d closedPt;
						lnsg.set(m_vertexArrOut[i].m_vertex, m_vertexArrOut[0].m_vertex);
						closedPt = lnsg.projClosestPointTo(pickPoint, viewDir);
						snapPoints.append(closedPt);
						lnsg.set(OutUpPtarr[i], OutUpPtarr[0]);
						closedPt = lnsg.projClosestPointTo(pickPoint, viewDir);
						snapPoints.append(closedPt);
					}
					i = (int)m_vertexArrOut.size() - 1;
					break;
				case POLYLINEVERTE_FLAG_AC:
					if(i <= m_vertexArrOut.size() - 2)
					{
						AcGeCircArc3d geArc(m_vertexArrOut[i].m_vertex, m_vertexArrOut[i + 1].m_vertex, m_vertexArrOut[0].m_vertex);
						AcGePoint3d closedPt;
						closedPt = geArc.projClosestPointTo(pickPoint, viewDir);
						snapPoints.append(closedPt);
						geArc.set(OutUpPtarr[i], OutUpPtarr[i + 1], OutUpPtarr[0]);
						closedPt = geArc.projClosestPointTo(pickPoint, viewDir);
						snapPoints.append(closedPt);
					}
					i = (int)m_vertexArrOut.size() - 1;
					break;
				case POLYLINEVERTE_FLAG_C:
					if(i == 0 && m_vertexArrOut.size() >= 3)
					{
						AcGeCircArc3d geArc(m_vertexArrOut[i].m_vertex, m_vertexArrOut[i + 1].m_vertex, m_vertexArrOut[i + 2].m_vertex);
						AcGeCircArc3d geCir(geArc.center(), geArc.normal(), geArc.radius());
						AcGePoint3d closedPt;
						closedPt = geCir.projClosestPointTo(pickPoint, viewDir);
						snapPoints.append(closedPt);
						geArc.set(OutUpPtarr[i], OutUpPtarr[i + 1], OutUpPtarr[i + 2]);
						geCir.set(geArc.center(), geArc.normal(), geArc.radius());
						closedPt = geCir.projClosestPointTo(pickPoint, viewDir);
						snapPoints.append(closedPt);
					}
					i = (int)m_vertexArrOut.size() - 1;
					break;
				default:
					break;
				}
			}
			size_t j;
			for(j = 0; j < m_vertexArrArrIn.size(); j++)
			{
				for(i = 0; i < m_vertexArrArrIn[j].size(); i++)
				{
					switch(m_vertexArrArrIn[j][i].m_vertexFlag)
					{
					case POLYLINEVERTE_FLAG_L:
						{
							AcGeLineSeg3d lnsg;
							AcGePoint3d closedPt;
							if(i == m_vertexArrArrIn[j].size() - 1)
								lnsg.set(m_vertexArrArrIn[j][i].m_vertex, m_vertexArrArrIn[j][0].m_vertex);
							else
								lnsg.set(m_vertexArrArrIn[j][i].m_vertex, m_vertexArrArrIn[j][i + 1].m_vertex);
							closedPt = lnsg.projClosestPointTo(pickPoint, viewDir);
							snapPoints.append(closedPt);
							if(i == m_vertexArrArrIn[j].size() - 1)
								lnsg.set(InUpPtarrarr[j][i], InUpPtarrarr[j][0]);
							else
								lnsg.set(InUpPtarrarr[j][i], InUpPtarrarr[j][i + 1]);
							closedPt = lnsg.projClosestPointTo(pickPoint, viewDir);
							snapPoints.append(closedPt);
						}
						break;
					case POLYLINEVERTE_FLAG_A:
						if(i == m_vertexArrArrIn[j].size() - 2)
						{
							AcGeCircArc3d geArc(m_vertexArrArrIn[j][i].m_vertex, m_vertexArrArrIn[j][i + 1].m_vertex, m_vertexArrArrIn[j][0].m_vertex);
							AcGePoint3d closedPt;
							closedPt = geArc.projClosestPointTo(pickPoint, viewDir);
							snapPoints.append(closedPt);
							geArc.set(InUpPtarrarr[j][i], InUpPtarrarr[j][i + 1], InUpPtarrarr[j][0]);
							closedPt = geArc.projClosestPointTo(pickPoint, viewDir);
							snapPoints.append(closedPt);
						}
						else if(i < m_vertexArrArrIn[j].size() - 2)
						{
							AcGeCircArc3d geArc(m_vertexArrArrIn[j][i].m_vertex, m_vertexArrArrIn[j][i + 1].m_vertex, m_vertexArrArrIn[j][i + 2].m_vertex);
							AcGePoint3d closedPt;
							closedPt = geArc.projClosestPointTo(pickPoint, viewDir);
							snapPoints.append(closedPt);
							geArc.set(InUpPtarrarr[j][i], InUpPtarrarr[j][i + 1], InUpPtarrarr[j][i + 2]);
							closedPt = geArc.projClosestPointTo(pickPoint, viewDir);
							snapPoints.append(closedPt);
						}
						i += 1;
						break;
					case POLYLINEVERTE_FLAG_AM:
						break;
					case POLYLINEVERTE_FLAG_LC:
						{
							AcGeLineSeg3d lnsg;
							AcGePoint3d closedPt;
							lnsg.set(m_vertexArrArrIn[j][i].m_vertex, m_vertexArrArrIn[j][0].m_vertex);
							closedPt = lnsg.projClosestPointTo(pickPoint, viewDir);
							snapPoints.append(closedPt);
							lnsg.set(InUpPtarrarr[j][i], InUpPtarrarr[j][0]);
							closedPt = lnsg.projClosestPointTo(pickPoint, viewDir);
							snapPoints.append(closedPt);
						}
						i = (int)m_vertexArrArrIn[j].size() - 1;
						break;
					case POLYLINEVERTE_FLAG_AC:
						if(i <= m_vertexArrArrIn[j].size() - 2)
						{
							AcGeCircArc3d geArc(m_vertexArrArrIn[j][i].m_vertex, m_vertexArrArrIn[j][i + 1].m_vertex, m_vertexArrArrIn[j][0].m_vertex);
							AcGePoint3d closedPt;
							closedPt = geArc.projClosestPointTo(pickPoint, viewDir);
							snapPoints.append(closedPt);
							geArc.set(InUpPtarrarr[j][i], InUpPtarrarr[j][i + 1], InUpPtarrarr[j][0]);
							closedPt = geArc.projClosestPointTo(pickPoint, viewDir);
							snapPoints.append(closedPt);
						}
						i = (int)m_vertexArrArrIn[j].size() - 1;
						break;
					case POLYLINEVERTE_FLAG_C:
						if(i == 0 && m_vertexArrArrIn[j].size() >= 3)
						{
							AcGeCircArc3d geArc(m_vertexArrArrIn[j][i].m_vertex, m_vertexArrArrIn[j][i + 1].m_vertex, m_vertexArrArrIn[j][i + 2].m_vertex);
							AcGeCircArc3d geCir(geArc.center(), geArc.normal(), geArc.radius());
							AcGePoint3d closedPt;
							closedPt = geCir.projClosestPointTo(pickPoint, viewDir);
							snapPoints.append(closedPt);
							geArc.set(InUpPtarrarr[j][i], InUpPtarrarr[j][i + 1], InUpPtarrarr[j][i + 2]);
							geCir.set(geArc.center(), geArc.normal(), geArc.radius());
							closedPt = geCir.projClosestPointTo(pickPoint, viewDir);
							snapPoints.append(closedPt);
						}
						i = (int)m_vertexArrArrIn[j].size() - 1;
						break;
					default:
						break;
					}
				}
			}
			//棱边
			for(i = 0; i < m_vertexArrOut.size(); i++)
			{
				AcGeLineSeg3d lnsg;
				AcGePoint3d closedPt;
				lnsg.set(m_vertexArrOut[i].m_vertex, OutUpPtarr[i]);
				closedPt = lnsg.projClosestPointTo(pickPoint, viewDir);
				snapPoints.append(closedPt);
			}
			for(j = 0; j < m_vertexArrArrIn.size(); j++)
			{
				for(i = 0; i < m_vertexArrArrIn[j].size(); i++)
				{
					AcGeLineSeg3d lnsg;
					AcGePoint3d closedPt;
					lnsg.set(m_vertexArrArrIn[j][i].m_vertex, InUpPtarrarr[j][i]);
					closedPt = lnsg.projClosestPointTo(pickPoint, viewDir);
					snapPoints.append(closedPt);
				}
			}
		}
      /*{
        AcGeLineSeg3d lnsg;
        AcGePoint3d pt;
        int gsStart = 0;

        //下底面外环
        if(gsSelectionMark < m_ptarrOut.length())
          {
            lnsg.set(m_ptarrOut[gsSelectionMark - 1], m_ptarrOut[gsSelectionMark]);
            pt = lnsg.projClosestPointTo(pickPoint, viewDir);
            snapPoints.append(pt);
          }
        else if(gsSelectionMark == m_ptarrOut.length())
          {
            lnsg.set(m_ptarrOut[gsSelectionMark - 1], m_ptarrOut[0]);
            pt = lnsg.projClosestPointTo(pickPoint, viewDir);
            snapPoints.append(pt);
          }
        else
          {
            //下底面内环
            gsStart = m_ptarrOut.length();
            int inIndex = 0;
            while((inIndex < m_ptarrarrIn.size()) && (gsSelectionMark - gsStart > m_ptarrarrIn[inIndex].length()))
              gsStart = gsStart + m_ptarrarrIn[inIndex++].length();
            if(inIndex < m_ptarrarrIn.size())
              {
                if(gsSelectionMark - gsStart < m_ptarrarrIn[inIndex].length())
                  {
                    lnsg.set(m_ptarrarrIn[inIndex][gsSelectionMark - gsStart - 1], m_ptarrarrIn[inIndex][gsSelectionMark - gsStart]);
                    pt = lnsg.projClosestPointTo(pickPoint, viewDir);
                    snapPoints.append(pt);
                  }
                else if(gsSelectionMark - gsStart == m_ptarrarrIn[inIndex].length())
                  {
                    lnsg.set(m_ptarrarrIn[inIndex][gsSelectionMark - gsStart - 1], m_ptarrarrIn[inIndex][0]);
                    pt = lnsg.projClosestPointTo(pickPoint, viewDir);
                    snapPoints.append(pt);
                  }
              }
            //上底面外环
            else if(gsSelectionMark - gsStart < OutUpPtarr.length())
              {
                lnsg.set(OutUpPtarr[gsSelectionMark - gsStart - 1], OutUpPtarr[gsSelectionMark - gsStart]);
                pt = lnsg.projClosestPointTo(pickPoint, viewDir);
                snapPoints.append(pt);
              }
            else if(gsSelectionMark - gsStart == OutUpPtarr.length())
              {
                lnsg.set(OutUpPtarr[gsSelectionMark - gsStart - 1], OutUpPtarr[0]);
                pt = lnsg.projClosestPointTo(pickPoint, viewDir);
                snapPoints.append(pt);
              }
            else
              {
                //上底面内环
                gsStart += m_ptarrOut.length();
                inIndex = 0;
                while((inIndex < InUpPtarrarr.size()) && (gsSelectionMark - gsStart > InUpPtarrarr[inIndex].length()))
                  gsStart = gsStart + InUpPtarrarr[inIndex++].length();
                if(inIndex < InUpPtarrarr.size())
                  {
                    if(gsSelectionMark - gsStart < InUpPtarrarr[inIndex].length())
                      {
                        lnsg.set(InUpPtarrarr[inIndex][gsSelectionMark - gsStart - 1], InUpPtarrarr[inIndex][gsSelectionMark - gsStart]);
                        pt = lnsg.projClosestPointTo(pickPoint, viewDir);
                        snapPoints.append(pt);
                      }
                    else if(gsSelectionMark - gsStart == InUpPtarrarr[inIndex].length())
                      {
                        lnsg.set(InUpPtarrarr[inIndex][gsSelectionMark - gsStart - 1], InUpPtarrarr[inIndex][0]);
                        pt = lnsg.projClosestPointTo(pickPoint, viewDir);
                        snapPoints.append(pt);
                      }
                  }
                // 外环侧面
                else if(gsSelectionMark - gsStart <= m_ptarrOut.length())
                  {
                    lnsg.set(m_ptarrOut[gsSelectionMark - gsStart - 1], OutUpPtarr[gsSelectionMark - gsStart - 1]);
                    pt = lnsg.projClosestPointTo(pickPoint, viewDir);
                    snapPoints.append(pt);
                  }
                else
                  {
                    // 内环侧面
                    gsStart += m_ptarrOut.length();
                    inIndex = 0;
                    while((inIndex < m_ptarrarrIn.size()) && (gsSelectionMark - gsStart > m_ptarrarrIn[inIndex].length()))
                      gsStart = gsStart + m_ptarrarrIn[inIndex++].length();
                    if(inIndex < m_ptarrarrIn.size())
                      {
                        if(gsSelectionMark - gsStart <= m_ptarrarrIn[inIndex].length())
                          {
                            lnsg.set(m_ptarrarrIn[inIndex][gsSelectionMark - gsStart - 1], InUpPtarrarr[inIndex][gsSelectionMark - gsStart - 1]);
                            pt = lnsg.projClosestPointTo(pickPoint, viewDir);
                            snapPoints.append(pt);
                          }
                      }
                  }
              }
          }
      }*/
      break;
    default:
      break;
    }

  return Acad::eOk;
}

// 空间任意平面坐标系原点
AcGePoint3d               PDSpolygon::getOri() const
{
  assertReadEnabled();
  return m_ptOri;
}

// X轴方向
AcGeVector3d              PDSpolygon::getV1() const
{
  assertReadEnabled();
  return m_vecV1;
}

// Y轴方向
AcGeVector3d              PDSpolygon::getV2() const
{
  assertReadEnabled();
  return m_vecV2;
}

// 拉伸方向
AcGeVector3d              PDSpolygon::getVh() const
{
  assertReadEnabled();
  return m_vecVh;
}

// 拉伸高度
double                    PDSpolygon::getH() const
{
  assertReadEnabled();
  return m_dH;
}

Acad::ErrorStatus         PDSpolygon::setH(double val)
{
  assertWriteEnabled();
  if(val == 0)
    return Acad::eInvalidInput;
  m_dH = fabs(val);
  GetMaxLen();
  return Acad::eOk;
}

// 外环的顶点
std::vector<t_PolylineVertex>           PDSpolygon::getOut() const
{
  assertReadEnabled();
  return m_vertexArrOut;
}

// 内环的顶点
std::vector< std::vector<t_PolylineVertex> >  PDSpolygon::getIn() const
{
  assertReadEnabled();
  return m_vertexArrArrIn;
}

Acad::ErrorStatus         PDSpolygon::setVal(const AcGePoint3d& oriPt, const AcGeVector3d& V1, const AcGeVector3d& V2, 
                                             const AcGeVector3d& Vh, double H, const AcGePoint3dArray& OutPtArr, 
                                             const std::vector<AcGePoint3dArray>& InPtArrArr, int CorrType/* = 1*/)
{
	assertWriteEnabled();

	m_ptOri = oriPt;      // 空间任意平面坐标系原点
	m_vecV1 = V1.normal();      // X轴方向
	m_vecV2 = V2.normal();      // Y轴方向

	AcGeMatrix3d mat;
	mat.setCoordSystem(m_ptOri, m_vecV1.normal(), m_vecV2.normal(), m_vecV1.crossProduct(m_vecV2).normal());

	m_vecVh = Vh;
	if(m_vecVh.z == 0)
    {
		ads_printf(LANGUAGE_DEFINE_MACRO_V25_4779/*"\n 拉伸体的拉伸方向错误，拉伸方向在拉伸平面内。"*/);
		return Acad::eInvalidInput;
    }
	if(!CorrType)
		m_vecVh.set(m_vecVh.x, m_vecVh.y, -m_vecVh.z);
	m_vecVh.transformBy(mat).normalize();  // 拉伸方向
  
	m_dH = fabs(H);         // 拉伸高度
	if(m_dH == 0)
    {
		ads_printf(LANGUAGE_DEFINE_MACRO_V25_4780/*"\n 拉伸体的拉伸高度为零。"*/);
		return Acad::eInvalidInput;
    }

	while(!m_ptarrOut.isEmpty())
		m_ptarrOut.removeLast();
	 while(!m_ptarrarrIn.empty())
		m_ptarrarrIn.pop_back();
	if(OutPtArr.length() < 3)
    {
		ads_printf(LANGUAGE_DEFINE_MACRO_V25_4781/*"\n 拉伸体的拉伸轮廓的外环顶点数小于3。"*/);
		return Acad::eInvalidInput;
    }
	else
    {
		size_t i;
		int j;
		for(j = 0; j < OutPtArr.length(); j++)
		{
			t_PolylineVertex ptTemp;
			if(j == OutPtArr.length() - 1)
				ptTemp.m_vertexFlag = POLYLINEVERTE_FLAG_LC;
			else
				ptTemp.m_vertexFlag = POLYLINEVERTE_FLAG_L;
			if(CorrType)
				ptTemp.m_vertex.set(OutPtArr[j].x, OutPtArr[j].y, OutPtArr[j].z);
			else
				ptTemp.m_vertex.set(OutPtArr[j].x, OutPtArr[j].y, -OutPtArr[j].z);
			ptTemp.m_vertex.transformBy(mat);
			m_vertexArrOut.push_back(ptTemp);   // 外环的顶点
		}
		for(i = 0; i < InPtArrArr.size(); i++)
		{
			if(InPtArrArr[i].length() < 3)
				ads_printf(LANGUAGE_DEFINE_MACRO_V25_4782/*"\n 拉伸体的拉伸轮廓的内环顶点数小于3。"*/);
			else
			{
				std::vector<t_PolylineVertex> inPtArr;
				for(int j = 0; j < InPtArrArr[i].length(); j++)
				{
					t_PolylineVertex ptTemp;
					if(j == InPtArrArr[i].length() - 1)
						ptTemp.m_vertexFlag = POLYLINEVERTE_FLAG_LC;
					else
						ptTemp.m_vertexFlag = POLYLINEVERTE_FLAG_L;
					if(CorrType)
						ptTemp.m_vertex.set(InPtArrArr[i][j].x, InPtArrArr[i][j].y, InPtArrArr[i][j].z);
					else
						ptTemp.m_vertex.set(InPtArrArr[i][j].x, InPtArrArr[i][j].y, -InPtArrArr[i][j].z);
					ptTemp.m_vertex.transformBy(mat);
					inPtArr.push_back(ptTemp);   // 内环的顶点
				}
				m_vertexArrArrIn.push_back(inPtArr);
			}
		}
    }

	//createBody();
	GetMaxLen();

	return Acad::eOk;
}

Acad::ErrorStatus         PDSpolygon::setVal(const AcGePoint3d& oriPt, const AcGeVector3d& V1, const AcGeVector3d& V2, 
								 const AcGeVector3d& Vh, double H, const std::vector<t_PolylineVertex>& OutPtArr,
								 const std::vector< std::vector<t_PolylineVertex> >& InPtArrArr, int CorrType/* = 1*/)
{
	assertWriteEnabled();

	m_ptOri = oriPt;      // 空间任意平面坐标系原点
	m_vecV1 = V1.normal();      // X轴方向
	m_vecV2 = V2.normal();      // Y轴方向

	AcGeMatrix3d mat;
	mat.setCoordSystem(m_ptOri, m_vecV1.normal(), m_vecV2.normal(), m_vecV1.crossProduct(m_vecV2).normal());

	m_vecVh = Vh;
	if(m_vecVh.z == 0)
	{
		ads_printf(LANGUAGE_DEFINE_MACRO_V25_4779/*"\n 拉伸体的拉伸方向错误，拉伸方向在拉伸平面内。"*/);
	}
	if(!CorrType)
		m_vecVh.set(m_vecVh.x, m_vecVh.y, -m_vecVh.z);
	m_vecVh.transformBy(mat).normalize();  // 拉伸方向

	m_dH = fabs(H);         // 拉伸高度
	if(m_dH == 0)
	{
		ads_printf(LANGUAGE_DEFINE_MACRO_V25_4780/*"\n 拉伸体的拉伸高度为零。"*/);
	}

	if(OutPtArr.size() < 2)
	{
		ads_printf(LANGUAGE_DEFINE_MACRO_V25_4783/*"\n 拉伸体的拉伸轮廓的外环顶点数小于2。"*/);
	}
	else
	{
		m_vertexArrOut = OutPtArr;
		m_vertexArrArrIn = InPtArrArr;
		size_t i;
		for(i = 0; i < m_vertexArrOut.size(); ++i)
		{
			if(!CorrType)
				m_vertexArrOut[i].m_vertex.set(m_vertexArrOut[i].m_vertex.x, m_vertexArrOut[i].m_vertex.y, -m_vertexArrOut[i].m_vertex.z);
			m_vertexArrOut[i].m_vertex.transformBy(mat);
		}
		for(i = 0; i < m_vertexArrArrIn.size(); ++i)
		{
			for(size_t j = 0; j < m_vertexArrArrIn[i].size(); ++j)
			{
				if(!CorrType)
					m_vertexArrArrIn[i][j].m_vertex.set(m_vertexArrArrIn[i][j].m_vertex.x, m_vertexArrArrIn[i][j].m_vertex.y, -m_vertexArrArrIn[i][j].m_vertex.z);
				m_vertexArrArrIn[i][j].m_vertex.transformBy(mat);
			}
		}
	}

	//createBody();
	GetMaxLen();

	return Acad::eOk;
}

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDSpolygon::subExplode(AcDbVoidPtrArray& entitySet) const
#else
Acad::ErrorStatus PDSpolygon::explode(AcDbVoidPtrArray& entitySet) const
#endif
{
	assertReadEnabled();

	//  [4/8/2008 suzhiyong]
	//////////////////////////////////////////////////////////////////////////
	AcGePoint3d pts[3];
	double ang = 0;
	AcGeVector3d vec0;
	AcGeVector3d vec1;
	AcGeVector3d vecZ;

	pts[0] = m_vertexArrOut[0].m_vertex;
	pts[1] = m_vertexArrOut[1].m_vertex;
	pts[2] = m_vertexArrOut[2].m_vertex;

	vec0 = pts[1] - pts[0];
	vec1 = pts[2] - pts[0];

	vecZ = vec0.crossProduct(vec1).normal();
	ang = vecZ.angleTo(m_vecVh);
	int i=0;
	size_t j=0;

	AcDbVoidPtrArray curveSegments;
	convertPolyVertexToSegments(m_vertexArrOut, m_vecVh, curveSegments);

	AcDbVoidPtrArray regions;
	Acad::ErrorStatus es = Acad::eOk;
	es = AcDbRegion::createFromCurves(curveSegments, regions);

	for(i = 0; i < curveSegments.length(); i++)
		delete (AcRxObject*)curveSegments[i];
	curveSegments.setLogicalLength(0);

	if(es == Acad::eOk && !regions.isEmpty())
    {
		AcDbCurve *pCurve = new AcDbLine(m_vertexArrOut[0].m_vertex, m_vertexArrOut[0].m_vertex + m_vecVh * m_dH);
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
			return Acad::eNotApplicable;
        }

		for(i = 0; i < regions.length(); i++)
			delete (AcRxObject*)regions[i];
		regions.setLogicalLength(0);

		for(j = 0; j < m_vertexArrArrIn.size(); j++)
			convertPolyVertexToSegments(m_vertexArrArrIn[j], m_vecVh, curveSegments);

		es = AcDbRegion::createFromCurves(curveSegments, regions);

		for(i = 0; i < curveSegments.length(); i++)
			delete (AcRxObject*)curveSegments[i];
		curveSegments.setLogicalLength(0);

		if(es == Acad::eOk && !regions.isEmpty())
        {
			for(i = 0; i < regions.length(); i++)
			{
				AcDb3dSolid *pBodySub;
				pBodySub = new AcDb3dSolid;
				es = pBodySub->extrudeAlongPath((AcDbRegion*)(regions[i]), pCurve);
				if(es != Acad::eOk)
					delete pBodySub;
				else
				{
					pBody->booleanOper(AcDb::kBoolSubtract, pBodySub);
					delete pBodySub;
				}
			}
        }

		pBody->setPropertiesFrom(this);
		entitySet.append(pBody);

		pCurve->upgradeOpen();
		pCurve->erase();
		pCurve->close();
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
}

//  [9/5/2007 suzhiyong]
Acad::ErrorStatus PDSpolygon::explodeTo3DSolid(AcDb3dSolid* &p3dSolid) const
{
	assertReadEnabled();

	//  [4/8/2008 suzhiyong]
	//////////////////////////////////////////////////////////////////////////
	AcGePoint3d pts[3];
	double ang = 0;
	AcGeVector3d vec0;
	AcGeVector3d vec1;
	AcGeVector3d vecZ;

	pts[0] = m_vertexArrOut[0].m_vertex;
	pts[1] = m_vertexArrOut[1].m_vertex;
	pts[2] = m_vertexArrOut[2].m_vertex;

	vec0 = pts[1] - pts[0];
	vec1 = pts[2] - pts[0];

	vecZ = vec0.crossProduct(vec1).normal();
	ang = vecZ.angleTo(m_vecVh);
	int i=0;
	size_t j=0;

	AcDbVoidPtrArray curveSegments;
	convertPolyVertexToSegments(m_vertexArrOut, m_vecVh, curveSegments);

	AcDbVoidPtrArray regions;
	Acad::ErrorStatus es = Acad::eOk;
	es = AcDbRegion::createFromCurves(curveSegments, regions);

	for(i = 0; i < curveSegments.length(); i++)
		delete (AcRxObject*)curveSegments[i];
	curveSegments.setLogicalLength(0);

	if(es == Acad::eOk && !regions.isEmpty())
	{
		AcDbCurve *pCurve = new AcDbLine(m_vertexArrOut[0].m_vertex, m_vertexArrOut[0].m_vertex + m_vecVh * m_dH);
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
			return Acad::eNotApplicable;
		}

		for(i = 0; i < regions.length(); i++)
			delete (AcRxObject*)regions[i];
		regions.setLogicalLength(0);

		for(j = 0; j < m_vertexArrArrIn.size(); j++)
			convertPolyVertexToSegments(m_vertexArrArrIn[j], m_vecVh, curveSegments);

		es = AcDbRegion::createFromCurves(curveSegments, regions);

		for(i = 0; i < curveSegments.length(); i++)
			delete (AcRxObject*)curveSegments[i];
		curveSegments.setLogicalLength(0);

		if(es == Acad::eOk && !regions.isEmpty())
		{
			for(i = 0; i < regions.length(); i++)
			{
				AcDb3dSolid *pBodySub;
				pBodySub = new AcDb3dSolid;
				es = pBodySub->extrudeAlongPath((AcDbRegion*)(regions[i]), pCurve);
				if(es != Acad::eOk)
					delete pBodySub;
				else
				{
					pBody->booleanOper(AcDb::kBoolSubtract, pBodySub);
					delete pBodySub;
				}
			}
		}

		pBody->setPropertiesFrom(this);
		p3dSolid = pBody;

		pCurve->upgradeOpen();
		pCurve->erase();
		pCurve->close();
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
}

void PDSpolygon::createBody()
{
    assertReadEnabled();
//#ifdef _OBJECTARX2004_    
	AcGePoint3d *vertices = NULL;
	PolygonVertexData** vertexData = NULL;
	int iVertices;
	AcGeVector3d normal;

    if(convertPolyVertexToAmodelerData(m_vertexArrOut, vertices, vertexData, iVertices, normal) != Acad::eOk)
        return ;

    if(iVertices < 2)
        return ;

	m_3dGeom = Body::extrusion((Point3d*)(vertices), 
                                vertexData, 
                                iVertices, 
                                *(Vector3d*)&normal, 
                                *(Vector3d*)&(m_vecVh.normal() * m_dH));

    delete [] vertexData;
	vertexData = NULL;
	delete [] vertices;
	vertices = NULL;

    if(m_3dGeom.isValid())
    {
        for(size_t i = 0; i < m_vertexArrArrIn.size(); i++)
        {
            if(convertPolyVertexToAmodelerData(m_vertexArrArrIn[i], vertices, vertexData, iVertices, normal)!=Acad::eOk)
              continue;
            if(iVertices < 2)
                continue;

            Body SubBody;
            SubBody = Body::extrusion((Point3d*)(vertices), 
                                vertexData, 
                                iVertices, 
                                *(Vector3d*)&normal, 
                                *(Vector3d*)&(m_vecVh.normal() * m_dH));
//#ifndef NDEBUG
//                if(SubBody.isValid())
//#endif
//                {
                BodyInterferenceType intType;
				intType = m_3dGeom.interfere(SubBody);
				if(intType == kBodiesInterfere)
                    m_3dGeom = m_3dGeom.boolOper(SubBody, kBoolOperSubtract);
//                }

			delete [] vertexData;
			vertexData = NULL;
			delete [] vertices;
			vertices = NULL;
        }
    }
//#endif
}

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDSpolygon::subGetGeomExtents(AcDbExtents& extents) const
#else
Acad::ErrorStatus PDSpolygon::getGeomExtents(AcDbExtents& extents) const
#endif
{
    assertReadEnabled();
#ifdef _OBJECTARX2010_
    return PDPrimary3D::subGetGeomExtents(extents);
#else
	return PDPrimary3D::getGeomExtents(extents);
#endif
}

#ifdef _OBJECTARX2010_
void PDSpolygon::subList() const
#else
void PDSpolygon::list() const
#endif
{
	assertReadEnabled();
#ifdef _OBJECTARX2010_
	PDPrimary3D::subList();
#else
	PDPrimary3D::list();
#endif
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4732/*"\n 插入点(%f,%f,%f)\n"*/,m_ptOri.x,m_ptOri.y,m_ptOri.z);
    ads_printf(LANGUAGE_DEFINE_MACRO_V25_4733/*" 插入X轴(%f,%f,%f)\n"*/,m_vecV1.x,m_vecV1.y,m_vecV1.z);
    ads_printf(LANGUAGE_DEFINE_MACRO_V25_4734/*" 插入Y轴(%f,%f,%f)\n"*/,m_vecV2.x,m_vecV2.y,m_vecV2.z);
    ads_printf(LANGUAGE_DEFINE_MACRO_V25_4784/*" 拉伸方向(%f,%f,%f)\n"*/,m_vecVh.x,m_vecVh.y,m_vecVh.z);
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4785/*" 拉伸高度=%f\n"*/,m_dH);
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4786/*" 外环的顶点:"*/);
	TCHAR strTemp[256];
	size_t i;
	for(i = 0; i < m_vertexArrOut.size(); i++)
	{
		switch(m_vertexArrOut[i].m_vertexFlag)
		{
		case POLYLINEVERTE_FLAG_A:
			_tcscpy(strTemp, LANGUAGE_DEFINE_MACRO_V25_4787/*"圆弧起点"*/);
			break;
		case POLYLINEVERTE_FLAG_AC:
			_tcscpy(strTemp, LANGUAGE_DEFINE_MACRO_V25_4788/*"圆弧起点，闭合"*/);
			break;
		case POLYLINEVERTE_FLAG_AM:
			_tcscpy(strTemp, LANGUAGE_DEFINE_MACRO_V25_4789/*"圆弧中点"*/);
			break;
		case POLYLINEVERTE_FLAG_C:
			_tcscpy(strTemp, LANGUAGE_DEFINE_MACRO_V25_4790/*"圆上的点"*/);
			break;
		case POLYLINEVERTE_FLAG_L:
			_tcscpy(strTemp, LANGUAGE_DEFINE_MACRO_V25_4791/*"直线起点"*/);
			break;
		case POLYLINEVERTE_FLAG_LC:
			_tcscpy(strTemp, LANGUAGE_DEFINE_MACRO_V25_4792/*"直线起点，闭合"*/);
		default:
			_tcscpy(strTemp, LANGUAGE_DEFINE_MACRO_V25_4791/*"直线起点"*/);
			break;
		}
        ads_printf(_T("\n (%f,%f,%f), %s"),m_vertexArrOut[i].m_vertex.x, m_vertexArrOut[i].m_vertex.y, m_vertexArrOut[i].m_vertex.z, strTemp);
	}
	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4793/*"\n 内环的顶点:"*/);
    for(i = 0; i < m_vertexArrArrIn.size(); i++)
	{	ads_printf(LANGUAGE_DEFINE_MACRO_V25_4794/*"\n 第%d个内环的顶点"*/,i+1);
        for(size_t j = 0; j < m_vertexArrArrIn[i].size(); j++)
		{
			switch(m_vertexArrArrIn[i][j].m_vertexFlag)
			{
			case POLYLINEVERTE_FLAG_A:
				_tcscpy(strTemp, LANGUAGE_DEFINE_MACRO_V25_4787/*"圆弧起点"*/);
				break;
			case POLYLINEVERTE_FLAG_AC:
				_tcscpy(strTemp, LANGUAGE_DEFINE_MACRO_V25_4788/*"圆弧起点，闭合"*/);
				break;
			case POLYLINEVERTE_FLAG_AM:
				_tcscpy(strTemp, LANGUAGE_DEFINE_MACRO_V25_4789/*"圆弧中点"*/);
				break;
			case POLYLINEVERTE_FLAG_C:
				_tcscpy(strTemp, LANGUAGE_DEFINE_MACRO_V25_4790/*"圆上的点"*/);
				break;
			case POLYLINEVERTE_FLAG_L:
				_tcscpy(strTemp, LANGUAGE_DEFINE_MACRO_V25_4791/*"直线起点"*/);
				break;
			case POLYLINEVERTE_FLAG_LC:
				_tcscpy(strTemp, LANGUAGE_DEFINE_MACRO_V25_4792/*"直线起点，闭合"*/);
			default:
				_tcscpy(strTemp, LANGUAGE_DEFINE_MACRO_V25_4791/*"直线起点"*/);
				break;
			}
			ads_printf(_T("\n (%f,%f,%f), %s"),m_vertexArrArrIn[i][j].m_vertex.x, m_vertexArrArrIn[i][j].m_vertex.y, m_vertexArrArrIn[i][j].m_vertex.z, strTemp);
		}
	}
    ads_printf(_T("\n"));
	return ;
}// added by linlin 20050811
double PDSpolygon::GetMaxLen()
{
	assertWriteEnabled();
	double tmpD = 0;
	m_dMaxLength = 0;
	for (size_t i =0 ;i < m_vertexArrOut.size() - 1; i++)
	{
		tmpD = m_vertexArrOut[i].m_vertex.distanceTo(m_vertexArrOut[i + 1].m_vertex);
		if(m_dMaxLength < tmpD)
			m_dMaxLength = tmpD;
	}
	if(m_dMaxLength < m_dH)
		m_dMaxLength = m_dH;

	return m_dMaxLength;
}

void PDSpolygon::setDefault()
{
	PDPrimary3D::setDefault();
#if defined(NDEBUG) || !defined(_DEBUG)

#else

#endif
}
bool PDSpolygon::isValidData(double &f)
{
	//未处理
	if(!PDPrimary3D::isValidData(f))
		return false;
	return true;
}

//zxb,20090113,增加audit，暂时只check自身成员数据
Acad::ErrorStatus PDSpolygon::audit(AcDbAuditInfo* pAuditInfo)
{
	return auditPDPrimary3D(this,pAuditInfo,_T("PDSpolygon"));
}
