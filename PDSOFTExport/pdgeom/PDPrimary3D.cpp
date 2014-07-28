/////////////////////////////////////////////
// PDPrimary3D custom object.

#include "StdAfx.h"
#include "language25.h"
#include "PDPrimary3d.h"
#include "dbproxy.h"
#include "geassign.h"
#include "acgi.h"
#include <dbsymtb.h>
#include <GEINTARR.H>
#include <adscodes.h>
#include <geextsf.h>
#include <gexbndsf.h>

#include <brnode.h>
#include <brm2dctl.h>
#include <brmesh2d.h>
#include <brbrep.h>
#include <brmetrav.h>
#include <brentrav.h>
#include <brbctrav.h>
#include <brcstrav.h>
#include <brshell.h>
#include <brsftrav.h>
#include <brface.h>

//#include "ArxDbgWorldDraw.h"

#include "FilerCallBack.h"
#include "AsdkBodyAModelerCallBack.h"

#ifdef _OBJECTARX2000_
#include <migrtion.h>
#include <dbapserv.h>
#endif

#define VERSION_PDPRIMARY3D 4

#ifdef _OBJECTARX2000_
ACRX_DXF_DEFINE_MEMBERS(PDPrimary3D, AcDbCurve,
						            AcDb::kDHL_CURRENT, AcDb::kMReleaseCurrent,
                        AcDbProxyEntity::kAllAllowedBits,
						            PDPRIMARY3D, Gle);

#else
ACRX_DXF_DEFINE_MEMBERS(PDPrimary3D, AcDbCurve, AcDbProxyEntity::kAllAllowedBits, PDPRIMARY3D, Gle);
#endif

// Constructor / Destructor

PDPrimary3D::PDPrimary3D(bool HasSnap/* = false*/)
{
	//{{AFX_ARX_DATA_INIT(PDPrimary3D)
	m_dDividPrecision = PRIMARY3D_DEFAULT_PRECISION;
	//}}AFX_ARX_DATA_INIT

	// TODO: do your initialization.

	setHasSnap(HasSnap);

	m_bCalMaxPrecision = PRIMARY3D_SIMPLIZE_RATE1 ;
	m_no8 =false;
	m_drawType = 0;
}

PDPrimary3D::~PDPrimary3D()
{
	//for(int i = 0; i < m_shelldata.size(); ++i)
	//{
	//	delete m_shelldata[i].pEdgeData;
	//	delete m_shelldata[i].pFaceData;
	//	delete m_shelldata[i].pVertexData;
	//}
}

Acad::ErrorStatus auditPDPrimary3D(PDPrimary3D *pent, AcDbAuditInfo* pAuditInfo,const TCHAR *prompt)
{
  if(!pent || !pAuditInfo)
    return Acad::eOk;

  Acad::ErrorStatus baseEs = pent->AcDbEntity::audit(pAuditInfo);
  TCHAR tmp[256];
  TCHAR hdl[18];
  AcDbHandle dbhdl;
  double errorflt=0;

  //如果实体检查有错且未修复，则不再数据检查
  if(baseEs != Acad::eOk && baseEs != Acad::eFixedAllErrors)
    return baseEs;

  switch(pAuditInfo->auditPass())
  {
  case AcDbAuditInfo::PASS1:
    // Check self
    if(!pent->isValidData(errorflt))
    {
      pAuditInfo->errorsFound(1);
      if(pAuditInfo->fixErrors()==Adesk::kTrue)
      {
        pent->assertWriteEnabled();
        _stprintf(tmp,_T("%f"),errorflt);
        hdl[0]=0;
        pent->getAcDbHandle(dbhdl);
        dbhdl.getIntoAsciiBuffer(hdl);
        pAuditInfo->printError(prompt,tmp,hdl,_T("0.1"));
        pent->setDefault();
        pAuditInfo->errorsFixed(1);
        return Acad::eFixedAllErrors;
      }
      return Acad::eLeftErrorsUnfixed;
    }
    break;
  case AcDbAuditInfo::PASS2:
    // Check references
    break;
  default:
    //maybe check xdata
    break;
  }
  return Acad::eOk;
}
void PDPrimary3D::setDefault()
{
  m_dDividPrecision = PRIMARY3D_DEFAULT_PRECISION;
  m_bCalMaxPrecision = PRIMARY3D_SIMPLIZE_RATE1 ;
  m_no8 =false;
  return;
}
bool PDPrimary3D::isValidData(double &f)
{
  return true;
}

bool PDPrimary3D::isLeftCoord(AcGeVector3d x,AcGeVector3d y, AcGeVector3d z)
{
  if(x.isZeroLength()==Adesk::kTrue||y.isZeroLength()==Adesk::kTrue||z.isZeroLength()==Adesk::kTrue)
    return false;
  AcGeVector3d nz;
  double ang=0;
  nz = x.crossProduct(y).normal();
  ang = nz.angleTo(z, x);
  if(ang > PI / 2.0 && ang < PI * 3.0 / 2.0)
    return true;
  else
    return false;
}

Acad::ErrorStatus PDPrimary3D::dwgInFields(AcDbDwgFiler* filer)
{
	assertWriteEnabled();
	Acad::ErrorStatus es;

	m_no8 =false;

	// Call dwgInFields from AcDbCurve
	if ((es = AcDbCurve::dwgInFields(filer)) != Acad::eOk) {
		return es;
	}

	// Read version number.
	Adesk::UInt16 version;
	filer->readItem(&version);
	if (version > VERSION_PDPRIMARY3D)
		return Acad::eMakeMeProxy;

	if(version >= 4)
	{
		m_ptsArr.clear();
		Adesk::UInt32 i, j, ptsArrLen, ptsLen;
		filer->readItem(&ptsArrLen);
		for(i = 0; i < ptsArrLen; ++i)
		{
			filer->readItem(&ptsLen);
			AcGePoint3dArray ptarr;
			for(j = 0; j < ptsLen; ++j)
			{
				AcGePoint3d pt;
				filer->readItem(&pt);
				ptarr.append(pt);
			}
			m_ptsArr.push_back(ptarr);
		}
	}

    if(version == 2)
    {
        Body geom3d;
        switch (filer->filerType()) 
        {
        case AcDb::kCopyFiler:    
        case AcDb::kDeepCloneFiler:
        case AcDb::kWblockCloneFiler:
            {
            void *i, *j;
        
            filer->readAddress(&i);
            filer->readAddress(&j);
        
            AcDbStub*   stub = (AcDbStub*)i;
            AcDbEntity* ent  = (PDPrimary3D*)j;
        
            if (NULL != stub) 
                acdbOpenAcDbEntity(ent, AcDbObjectId(stub), AcDb::kForRead, Adesk::kTrue);
            geom3d = ((PDPrimary3D*)ent)->m_3dGeom.copy();
            if (NULL != stub)
                ent->close();
            }
            break;
        
        case AcDb::kFileFiler:     
        case AcDb::kUndoFiler:
        case AcDb::kPageFiler:

            geom3d = Body::restore( &DwgFilerCallBack( filer ) );
            break;
        
        default:
            break;
        }
    }
	if(version >= 1)
    {
		// TODO: here you can file datamembers not
		//       created by the ObjectARX Add-In.
		filer->readItem(&m_dDividPrecision);
		filer->readItem(&m_bCalMaxPrecision);	
		filer->readItem(&m_bHasSnap);
	}

	return filer->filerStatus();
}

Acad::ErrorStatus PDPrimary3D::dwgOutFields(AcDbDwgFiler* filer) const
{
	assertReadEnabled();
	Acad::ErrorStatus es;

	// Call dwgOutFields from AcDbCurve
	if ((es = AcDbCurve::dwgOutFields(filer)) != Acad::eOk) {
		return es;
	}

	// Write version number.
	filer->writeItem((Adesk::UInt16) VERSION_PDPRIMARY3D);

	//version 4
	filer->writeItem((Adesk::UInt32)(m_ptsArr.size()));
	for(Adesk::UInt32 i = 0; i < (Adesk::UInt32)(m_ptsArr.size()); ++i)
	{
		filer->writeItem((Adesk::UInt32)(m_ptsArr[i].length()));
		for(Adesk::UInt32 j = 0; j < (Adesk::UInt32)(m_ptsArr[i].length()); ++j)
			filer->writeItem(m_ptsArr[i][j]);
	}

    // version 2
    // version 3 不对m_3dGeom进行存储
    //switch (filer->filerType()) 
    //{
    //case AcDb::kCopyFiler:     
    //case AcDb::kDeepCloneFiler:
    //case AcDb::kWblockCloneFiler:
    //    
    //    filer->writeAddress( objectId() );
    //    filer->writeAddress( this );
    //    break;
    //    
    //case AcDb::kFileFiler:     
    //case AcDb::kUndoFiler:
    //case AcDb::kPageFiler:
    //    
    //    m_3dGeom.save( &DwgFilerCallBack( filer ) );
    //    break;
    //    
    //default:
    //    break;
    //}

	// Write the data members.
	// TODO: here you can file datamembers not
	//       created by the ObjectARX Add-In.
	filer->writeItem(m_dDividPrecision);
	filer->writeItem(m_bCalMaxPrecision);
	filer->writeItem(m_bHasSnap);

	return filer->filerStatus();
	
}

Acad::ErrorStatus PDPrimary3D::dxfInFields(AcDbDxfFiler* pFiler)
{
	assertWriteEnabled();
	struct resbuf rb;

	m_no8 =false;

	if ((AcDbCurve::dxfInFields(pFiler) != Acad::eOk) ||
	    !pFiler->atSubclassData(_T("PDPrimary3D")))
		return pFiler->filerStatus();

	// Read version number.
	Adesk::UInt16 version;
	pFiler->readItem(&rb);
	if (rb.restype != AcDb::kDxfInt16)
    {
		  pFiler->pushBackItem();
		  pFiler->setError(Acad::eInvalidDxfCode,
			  _T("nError: expected object version group code %d"),
		                   AcDb::kDxfInt16);
		  return pFiler->filerStatus();
	  }
  else
    {
		  version = rb.resval.rint;
		  if (version > VERSION_PDPRIMARY3D)
			  return Acad::eMakeMeProxy;
	  }

	// TODO: here you can file datamembers not
	//       created by the ObjectARX Add-In.

	if(version >= 4)
	{
		m_ptsArr.clear();
		long i, j, ptsArrLen, ptsLen;
		pFiler->readItem(&rb);
		if (rb.restype == AcDb::kDxfInt32)
		{
			ptsArrLen = rb.resval.rlong;
		}
		else
		{
			pFiler->pushBackItem();
			pFiler->setError(Acad::eInvalidDxfCode,
				_T("\nError: expected group code %d"),
				AcDb::kDxfInt32);
			return pFiler->filerStatus();
		}
		for(i = 0; i < ptsArrLen; ++i)
		{
			pFiler->readItem(&rb);
			if (rb.restype == AcDb::kDxfInt32)
			{
				ptsLen = rb.resval.rlong;
			}
			else
			{
				pFiler->pushBackItem();
				pFiler->setError(Acad::eInvalidDxfCode,
					_T("\nError: expected group code %d"),
					AcDb::kDxfInt32);
				return pFiler->filerStatus();
			}
			AcGePoint3dArray ptarr;
			for(j = 0; j < ptsLen; ++j)
			{
				pFiler->readItem(&rb);
				if (rb.restype == AcDb::kDxfXCoord)
				{
					AcGePoint3d pt;
					pt.set(rb.resval.rpoint[X], rb.resval.rpoint[Y], rb.resval.rpoint[Z]);
					ptarr.append(pt);
				}
				else
				{
					pFiler->pushBackItem();
					pFiler->setError(Acad::eInvalidDxfCode,
						_T("\nError: expected group code %d"),
						AcDb::kDxfXCoord);
					return pFiler->filerStatus();
				}
			}
			m_ptsArr.push_back(ptarr);
		}
	}

	pFiler->readItem(&rb);
	if (rb.restype != AcDb::kDxfInt32)
    {
		  pFiler->pushBackItem();
		  pFiler->setError(Acad::eInvalidDxfCode,
			  _T("nError: expected DividPrecision %d"),
		                   AcDb::kDxfInt32);
		  return pFiler->filerStatus();
	  }
  else
    {
		  m_dDividPrecision = rb.resval.rlong;
	  }

	pFiler->readItem(&rb);
	if (rb.restype != AcDb::kDxfInt16)
    {
		  pFiler->pushBackItem();
		  pFiler->setError(Acad::eInvalidDxfCode,
			  _T("nError: expected rendermodel %d"),
		                   AcDb::kDxfInt16);
		  return pFiler->filerStatus();
	  }
  else
    {
		  m_bCalMaxPrecision = rb.resval.rint;
	  }

	pFiler->readItem(&rb);
	if (rb.restype != AcDb::kDxfInt16 + 1)
    {
		  pFiler->pushBackItem();
		  pFiler->setError(Acad::eInvalidDxfCode,
			  _T("nError: expected group code %d"),
		                   AcDb::kDxfInt16 + 1);
		  return pFiler->filerStatus();
	  }
  else
    {
		  m_bHasSnap = rb.resval.rint;
	  }

	return pFiler->filerStatus();
	
}

Acad::ErrorStatus PDPrimary3D::dxfOutFields(AcDbDxfFiler* pFiler) const
{
	assertReadEnabled();
	Acad::ErrorStatus es;

	if ((es = AcDbCurve::dxfOutFields(pFiler)) != Acad::eOk)
		return es;

	// Write subclass marker.
	pFiler->writeItem(AcDb::kDxfSubclass, _T("PDPrimary3D"));

	// Write version number.
	pFiler->writeItem(AcDb::kDxfInt16, (Adesk::Int16) VERSION_PDPRIMARY3D);


	// TODO: here you can file datamembers not
	//       created by the ObjectARX Add-In.

	// version 4
	pFiler->writeItem(AcDb::kDxfInt32, (Adesk::Int32)(m_ptsArr.size()));
	for(int i = 0; i < m_ptsArr.size(); ++i)
	{
		pFiler->writeItem(AcDb::kDxfInt32, (Adesk::Int32)(m_ptsArr[i].length()));
		for(int j = 0; j < m_ptsArr[i].length(); ++j)
			pFiler->writeItem(AcDb::kDxfXCoord, m_ptsArr[i][j]);
	}

	pFiler->writeItem(AcDb::kDxfInt32, (Adesk::Int32) m_dDividPrecision);
	pFiler->writeItem(AcDb::kDxfInt16, m_bCalMaxPrecision);
	pFiler->writeItem(AcDb::kDxfInt16 + 1, m_bHasSnap);

	return es;
}

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDPrimary3D::subTransformBy(const AcGeMatrix3d& xform)
#else
Acad::ErrorStatus PDPrimary3D::transformBy(const AcGeMatrix3d& xform)
#endif
{
	assertWriteEnabled();

	if(!m_3dGeom.isNull())
		m_3dGeom.transform( *(Transf3d*)&xform );

	if(!m_3dSolid.isNull())
		m_3dSolid.transformBy(xform);

	for(int i = 0; i < m_ptsArr.size(); ++i)
	{
		for(int j = 0; j < m_ptsArr[i].length(); ++j)
			m_ptsArr[i][j].transformBy(xform);
	}

#ifdef _OBJECTARX2010_
	return AcDbCurve::subTransformBy(xform);
#else
	return AcDbCurve::transformBy(xform);
#endif
}

#ifdef _OBJECTARX2010_
Adesk::Boolean PDPrimary3D::subWorldDraw(AcGiWorldDraw* mode)
#else
Adesk::Boolean PDPrimary3D::worldDraw(AcGiWorldDraw* mode)
#endif
{
	assertReadEnabled();

    if (mode->regenAbort()) {
        return Adesk::kTrue;
    }

	//if(m_shelldata.size() > 0)
	//{
	//	for(int i = 0; i < m_shelldata.size(); ++i)
	//	{
	//		Adesk::Int32 *pFaceList = new Adesk::Int32[m_shelldata[i].faceListSize];
	//		for(int j = 0; j < m_shelldata[i].faceListSize; ++j)
	//			pFaceList[j] = m_shelldata[i].pFaceList[j];
	//		mode->geometry().shell(m_shelldata[i].nbVertex, 
	//			m_shelldata[i].pVertexList.asArrayPtr(), 
	//			m_shelldata[i].faceListSize, 
	//			pFaceList, 
	//			m_shelldata[i].pEdgeData, 
	//			m_shelldata[i].pFaceData, 
	//			m_shelldata[i].pVertexData, 
	//			NULL, 
	//			m_shelldata[i].bAutoGenerateNormals);
	//	}
	//}

	if(m_drawType == 3 && m_ptsArr.size() > 0)
	{
		for(int i = 0; i < m_ptsArr.size(); ++i)
		{
			mode->geometry().polygon(m_ptsArr[i].length(), m_ptsArr[i].asArrayPtr());
		}
	}

	if(!m_3dSolid.isNull())
	{
		m_3dSolid.setPropertiesFrom(this);
		m_3dSolid.worldDraw(mode);
	}
    else if(!m_3dGeom.isNull())
	{
/*    int surfaceNum;
		surfaceNum = m_3dGeom.surfaceCount();
		if(surfaceNum > 0)
		{
			Surface *pSurface;
			pSurface = m_3dGeom.surfaceList();
			Surface::Type SurfType;
			SurfType = pSurface->type();
			switch(SurfType)
			{
			case Surface::kCylinder:
				{
					CylinderSurface *pCylSurf;
					pCylSurf = (CylinderSurface *)pSurface;
					double r;
					int Approx;
					r = pCylSurf->radius();
					Approx = pCylSurf->approx();
				}
				break;
			}
			int i = 0;
		}
*/
		// 
		// Evaluate the graphics
		// 
		//graphRep.evaluateWorldDraw(body, worldDraw, defaultColor);

#ifndef _ALWAYSSHOWWIRE_
		switch (mode->regenType()) {

		case kAcGiHideOrShadeCommand:
		case kAcGiRenderCommand:
#endif        
			// 
			// Draw shells
			// 
			{
				AsdkBodyAModelerCallBack AModelerCallBack( mode );
				m_3dGeom.triangulate( &AModelerCallBack );
			}
#ifndef _ALWAYSSHOWWIRE_
			break;

		case kAcGiStandardDisplay:
		case kAcGiSaveWorldDrawForR12:
#endif
			// 
			// Draw wireframe with visible hidden lines
			// 
			drawAllEdges( m_3dGeom, mode );
#ifndef _ALWAYSSHOWWIRE_
			break;

		default:
			// 
			// Unknown regenType
			// 
			//ASSERT(0);
			break;
		} /*switch*/
#endif
	}
	return Adesk::kTrue;
}


Adesk::Int32 PDPrimary3D::dividPrecision() const
{
	assertReadEnabled();
	return m_dDividPrecision;
}

Acad::ErrorStatus PDPrimary3D::setDividPrecision(const Adesk::Int32 dDividPrecision)
{
	assertWriteEnabled();
    if(dDividPrecision < 3)
        m_dDividPrecision = PRIMARY3D_DEFAULT_PRECISION;
    else
	    m_dDividPrecision = dDividPrecision;
	return Acad::eOk;
}

////用来控制所有子类是否输出捕捉点////
bool PDPrimary3D::hasSnap() const
{
  assertReadEnabled();
  if(m_bHasSnap)
    return true;
  else
    return false;
}

void PDPrimary3D::setHasSnap(bool val)
{
  assertWriteEnabled();
  if(val)
    m_bHasSnap = 1;
  else
    m_bHasSnap = 0;
  return;
}

/** 取得是否计算最大剖分精度
 * @return bool
 *
 * 如果计算最大剖分精度，则可以提高体元的显示速度，但在render命令中
 * 生成的图片可能效果不理想，如果不计算最大剖分精度，则render命令生成
 * 的图片效果好，但体元的显示速度有可能下降。
 */ 
Adesk::Int16 PDPrimary3D::getCalMaxPrecision() const
{
    assertReadEnabled();
    // 由于在AutoCAD2006版本中打开两个视口时，进行3dorbit，退出3dorbit时死机。
	// 所以在AutoCAD2006版本运行时体元永远不进行优化显示。
    resbuf verrb;
    acedGetVar(_T("ACADVER"), &verrb);
    if(_tcsncmp(verrb.resval.rstring, _T("16.2"), 4) == 0)
        return PRIMARY3D_SIMPLIZE_RATE0;
    //if(m_bCalMaxPrecision)
    //    return true;
    //else
    //    return false;
	return m_bCalMaxPrecision;
}

/** 设置是否计算最大剖分精度
 * @param val - 输入：是否计算最大剖分精度
 * @return void
 *
 * 如果计算最大剖分精度，则可以提高体元的显示速度，但在render命令中
 * 生成的图片可能效果不理想，如果不计算最大剖分精度，则render命令生成
 * 的图片效果好，但体元的显示速度有可能下降。
 */ 
void PDPrimary3D::setCalMaxPrecision(Adesk::Int16 val)
{
    assertWriteEnabled();
	//if(val)
 //       m_bCalMaxPrecision = 1;
 //   else
 //       m_bCalMaxPrecision = 0;
	m_bCalMaxPrecision = val;
    return;
}


Acad::ErrorStatus addToDb(AcDbEntity *pEnt)
{
  Acad::ErrorStatus es = Acad::eOk;	

//	判断pEnt不为空
  if(pEnt)
    {
		  AcDbBlockTable *pBlockTable = NULL;
		  AcDbBlockTableRecord  *pBlockTableRecord = NULL;
		  ///////////////		get block table		////////
      AcDbDatabase *pDb = NULL;
		  if( (es = acdbCurDwg()->getBlockTable(pBlockTable,AcDb::kForRead)) == Acad::eOk)
        {
			  //////////////		get block table record		/////////////
			    if( ( es = pBlockTable->getAt(ACDB_MODEL_SPACE, pBlockTableRecord,AcDb::kForWrite)) == Acad::eOk)
            {
				      ////	id is NULL,so add this Entity to tablerecord
				      es = pBlockTableRecord->appendAcDbEntity(pEnt);
				      pBlockTableRecord->close();
			      }
			    pBlockTable->close();
		    }
		  return (es) ;
	  }
	else
		return Acad::eInvalidInput;
}

void PDPrimary3D::createBody()
{

}

// 取得合适的剖分精度，如果实体较小，则剖分精度就会低，如果实体较大，则剖分
// 精度不大于实体限定的剖分精度m_dDividPrecision。
int PDPrimary3D::getMaxPrecision(AcGiWorldDraw *mode) const
{
    assertReadEnabled();
    return m_dDividPrecision;    
}

void PDPrimary3D::getActDivid(int Precision, AcGeDoubleArray &dividDbl, AcGeIntArray &stdIdx) const
{
    assertReadEnabled();

    int precisionTemp = Precision;
    // 如果剖分进度小于8，则强制将剖分精度设置为8
    if(precisionTemp < 8)
        precisionTemp = 8;
    int precision__1 = precisionTemp + 1;

    dividDbl.setLogicalLength(0);
    stdIdx.setLogicalLength(9);

    int divid_8_num = precisionTemp / 8;
    if(precisionTemp % 8 == 0  || m_no8 )
    {
		int i;
        for(i = 0; i < 9; ++i)
            stdIdx[i] = i * divid_8_num;
        for(i = 0; i < precision__1; ++i)
            dividDbl.append(i);
    }
    else
    {
        double insertIdxDbl[9];
        double divid_seg = precisionTemp % 8 / 8.0;
		int i;
        for(i = 0; i < 9; ++i)
            insertIdxDbl[i] = i * divid_8_num + i * divid_seg;
        int insertIdxDbl_i = 0;
        for(i = 0; i < precision__1; ++i)
        {
            dividDbl.append(i);
            if(insertIdxDbl_i > 8)
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
}

#ifdef _OBJECTARX2010_
void PDPrimary3D::subList() const
#else
void PDPrimary3D::list() const
#endif
{
    assertReadEnabled();
#ifdef _OBJECTARX2010_
    AcDbCurve::subList();
#else
	AcDbCurve::list();
#endif
    
    ads_printf(LANGUAGE_DEFINE_MACRO_V25_4759/*"\n 剖分精度:     %d"*/, m_dDividPrecision);
    if(hasSnap())
        ads_printf(LANGUAGE_DEFINE_MACRO_V25_4760/*"\n 捕捉点控制:   开"*/);
    else
        ads_printf(LANGUAGE_DEFINE_MACRO_V25_4761/*"\n 捕捉点控制:   关"*/);

    if(getCalMaxPrecision())
        ads_printf(LANGUAGE_DEFINE_MACRO_V25_4762/*"\n 优化剖分精度: 开"*/);
    else
        ads_printf(LANGUAGE_DEFINE_MACRO_V25_4763/*"\n 优化剖分精度: 关"*/);
}

#ifdef _OBJECTARX2010_
Acad::ErrorStatus PDPrimary3D::subGetGeomExtents(AcDbExtents& extents) const
#else
Acad::ErrorStatus PDPrimary3D::getGeomExtents(AcDbExtents& extents) const
#endif
{
	assertReadEnabled();
	if(m_ptsArr.size() > 0)
	{
		for(int i = 0; i < m_ptsArr.size(); ++i)
		{
			for(int j = 0; j < m_ptsArr[i].length(); ++j)
			{
				extents.addPoint(m_ptsArr[i][j]);
			}
		}
	}
	if(!m_3dGeom.isNull())
	{
		int vertexNum = m_3dGeom.vertexCount();
		Vertex* vertextList = m_3dGeom.vertexList();
		Vertex* vertextNext;
		int i;
		if(vertexNum > 0)
			extents.set((AcGePoint3d)(vertextList->point()), (AcGePoint3d)(vertextList->point()));
		vertextNext = vertextList->next();
		for(i = 1; i < vertexNum; ++i)
		{
			extents.addPoint((AcGePoint3d)(vertextNext->point()));
			vertextNext = vertextNext->next();
		}
	}
	else if(!m_3dSolid.isNull())
	{
		m_3dSolid.getGeomExtents(extents);
	}
	return Acad::eOk;
}

// 取得Amodeler 模型的实体
const Body* PDPrimary3D::getBody() const
{
	//assertReadEnabled();
	return &m_3dGeom;
}

// 清空Amodeler模型的实体
Acad::ErrorStatus PDPrimary3D::clearBody()
{
	//assertWriteEnabled();
	m_3dGeom.init(true);
	return Acad::eOk;
}

 Acad::ErrorStatus PDPrimary3D::explodeTo3DSolid(AcDb3dSolid* &p3dSolid) const
{
	if(!m_3dSolid.isNull())
	{
		p3dSolid = (AcDb3dSolid*)(m_3dSolid.clone());
		if(p3dSolid != NULL)
			return Acad::eOk;
	}
	return Acad::eNotApplicable;
}

Acad::ErrorStatus PDPrimary3D::convertPolyVertexToAmodelerData(const std::vector<t_PolylineVertex> &vertexArr, AcGePoint3d* &vertices, PolygonVertexData** &vertexData, 
															  int &iVertices, AcGeVector3d &normal) const
{
	assertReadEnabled();

  if(vertexArr.size() >= 3){
    if(vertexArr[1].m_vertex == vertexArr[0].m_vertex
      || vertexArr[2].m_vertex == vertexArr[1].m_vertex)
      return Acad::eInvalidInput;
		normal = (vertexArr[1].m_vertex - vertexArr[0].m_vertex).crossProduct(vertexArr[2].m_vertex - vertexArr[1].m_vertex).normal();
  }
	else
		return Acad::eInvalidInput;
	AcGePlane polyPlane(AcGePoint3d(0, 0, 0), normal);
	AcGeMatrix3d mat;
	AcGePoint3d planeOri;
	AcGeVector3d planeX, planeY, planeZ;
	polyPlane.getCoordSystem(planeOri, planeX, planeY);
	planeZ = planeX.crossProduct(planeY).normal();
	mat.setCoordSystem(planeOri, planeX, planeY, planeZ);
	mat.invert();

	Acad::ErrorStatus es = Acad::eOk;
	AcGePoint3dArray pts;
	AcGeDoubleArray bulges;
	size_t i;
	for(i = 0; i < vertexArr.size(); ++i)
	{
		bool breakFlag = false;
		switch(vertexArr[i].m_vertexFlag)
		{
		case POLYLINEVERTE_FLAG_A:
			if(i <= vertexArr.size() - 3)
			{
				AcGePoint3d pt1, pt2, pt3;
				pt1 = vertexArr[i].m_vertex;
				pt2 = vertexArr[i + 1].m_vertex;
				pt3 = vertexArr[i + 2].m_vertex;
				pt1.transformBy(mat);
				pt2.transformBy(mat);
				pt3.transformBy(mat);
				AcGeCircArc3d geArc(pt1, pt2, pt3);
				double sAng, eAng, inAng, bulge;
				AcGeVector3d norm = geArc.normal();
				sAng = geArc.startAng();
				eAng = geArc.endAng();
				inAng = eAng - sAng;
				bulge = tan(inAng / 4.0);
				if(norm.z < 0)
					bulge = -bulge;
				bulges.append(bulge);
				pts.append(vertexArr[i].m_vertex);
				i += 1;
			}
			else if(i <= vertexArr.size() - 2)
			{
				AcGePoint3d pt1, pt2, pt3;
				pt1 = vertexArr[i].m_vertex;
				pt2 = vertexArr[i + 1].m_vertex;
				pt3 = vertexArr[0].m_vertex;
				pt1.transformBy(mat);
				pt2.transformBy(mat);
				pt3.transformBy(mat);
				AcGeCircArc3d geArc(pt1, pt2, pt3);
				double sAng, eAng, inAng, bulge;
				AcGeVector3d norm = geArc.normal();
				sAng = geArc.startAng();
				eAng = geArc.endAng();
				inAng = eAng - sAng;
				bulge = tan(inAng / 4.0);
				if(norm.z < 0)
					bulge = -bulge;
				bulges.append(bulge);
				pts.append(vertexArr[i].m_vertex);
				i += 1;
			}
			break;
		case POLYLINEVERTE_FLAG_AC:
			if(i <= vertexArr.size() - 2)
			{
				AcGePoint3d pt1, pt2, pt3;
				pt1 = vertexArr[i].m_vertex;
				pt2 = vertexArr[i + 1].m_vertex;
				pt3 = vertexArr[0].m_vertex;
				pt1.transformBy(mat);
				pt2.transformBy(mat);
				pt3.transformBy(mat);
				AcGeCircArc3d geArc(pt1, pt2, pt3);
				double sAng, eAng, inAng, bulge;
				AcGeVector3d norm = geArc.normal();
				sAng = geArc.startAng();
				eAng = geArc.endAng();
				inAng = eAng - sAng;
				bulge = tan(inAng / 4.0);
				if(norm.z < 0)
					bulge = -bulge;
				bulges.append(bulge);
				pts.append(vertexArr[i].m_vertex);
				i += 1;
			}
			breakFlag = true;
			break;
		case POLYLINEVERTE_FLAG_AM:
			break;
		case POLYLINEVERTE_FLAG_C:
			if(i <= vertexArr.size() - 3)
			{
				AcGePoint3d pt1, pt2, pt3;
				pt1 = vertexArr[i].m_vertex;
				pt2 = vertexArr[i + 1].m_vertex;
				pt3 = vertexArr[i + 2].m_vertex;
				pt1.transformBy(mat);
				pt2.transformBy(mat);
				pt3.transformBy(mat);
				AcGeCircArc3d geArc(pt1, pt2, pt3);
				AcGePoint3d cen = geArc.center();
				pt2 = pt1 + (cen - pt1) * 2.0;
				bulges.append(1);
				pts.append(vertexArr[i].m_vertex);
				bulges.append(1);
				pts.append(vertexArr[i + 1].m_vertex);
				i += 1;
			}
			breakFlag = true;
			break;
		case POLYLINEVERTE_FLAG_L:
		case POLYLINEVERTE_FLAG_LC:
		default:
			AcGePoint3d pt1;
			pt1 = vertexArr[i].m_vertex;
			pt1.transformBy(mat);
			pts.append(vertexArr[i].m_vertex);
			bulges.append(0);
			if(vertexArr[i].m_vertexFlag == POLYLINEVERTE_FLAG_LC)
				breakFlag = true;
			break;
		}
		if(breakFlag)
			break;
	}
    iVertices = pts.length();
    vertices = new AcGePoint3d[iVertices];
    for(int j = 0; j < iVertices; ++j)
        vertices[j] = pts[j];
    getPolylineVertices(vertices, bulges, normal, 1, vertexData);

	return Acad::eOk;
}

//
// circleRadiusFromBulge()
//
// ARX expresses arcs in terms of their "bulges", while the Facet Modeler
// uses circle radii.  The function gets the radius from the bulge.
//
double PDPrimary3D::circleRadiusFromBulge(const AcGePoint3d& p1, const AcGePoint3d& p2, double bulge) const
{
	return 0.25 * p1.distanceTo(p2) * (bulge + 1 / bulge);  
} 

//
// getPolylineVertices()
//
// Gets a polyline entity from the user, and converts it into an array of
// vertices, vertex data, and a normal, which are passed back through the
// parameters.  If something goes wrong, vertices is set to NULL.
//
// Note that PolygonVertexData is an AModeler class.
//
void PDPrimary3D::getPolylineVertices(AcGePoint3d* vertices, const AcGeDoubleArray &bulge, const AcGeVector3d normal, int isClosed, PolygonVertexData** &vertexData) const
{
	int i;
	int approx = m_dDividPrecision;
	double filletRadius;

	if (bulge.length() == 0) 
	{
		vertexData = NULL;
		return;
	}

	vertexData = new PolygonVertexData*[bulge.length()];

	// Loop 3:
	// Create the vertexData for the polyline.
	// This is in its own loop because we need to peek at the next vertex each time.
	// 
	filletRadius = 0.0;

	for (i = 0; i < bulge.length(); i++)
	{
		if (bulge[i] != 0.0 && (isClosed || i + 1 < bulge.length()))
			// i.e., vertex is beginning of an arc
			// AND this is not the last vertex of an open polyline
		{
			if (approx < 0)
			{
				approx = 32;
				//
				// I'm deliberately allowing invalid values here,
				// i.e., numbers below 4
			}
			AcGePoint3d nextVertex = i + 1 < bulge.length() ? vertices[i + 1] : vertices[0];
			// nextVertex might be the first vertex

			double radius = circleRadiusFromBulge(vertices[i], nextVertex, bulge[i]);

			bool leftOfCenter;

			if (bulge[i] > 0 && bulge[i] < 1) 
				leftOfCenter = TRUE;
			else if (bulge[i] < -1)
				leftOfCenter = TRUE;
			else
				leftOfCenter = FALSE;

			Circle3d circle(
				(Point3d)vertices[i], 
				(Point3d)nextVertex, 
				(Vector3d)normal, 
				radius, 
				leftOfCenter);

			vertexData[i] = new PolygonVertexData(PolygonVertexData::kArc3d,
				circle, approx);
		}
		else // vertex is the first of a staight line segment
		{
			if (filletRadius != 0.0)
			{
				if (approx < 0)
					approx = 32;

				vertexData[i] = new PolygonVertexData(
					PolygonVertexData::kFilletByRadius,
					filletRadius,
					approx);
			}
			else
			{
				vertexData[i] = NULL;
			}
		}
	}
}

Acad::ErrorStatus PDPrimary3D::convertPolyVertexToSegments(const std::vector<t_PolylineVertex> &vertexArr, const AcGeVector3d &norm, AcDbVoidPtrArray &curveSegments) const
{
	assertReadEnabled();
	AcDbPolyline *pPoly;
	AcGeVector3d PolyNorm = norm;
	pPoly = new AcDbPolyline();
	if(pPoly == NULL)
		return Acad::eOk;

	if(vertexArr.size() <= 2)
		return Acad::eOk;

	AcGeVector3d vec1 = vertexArr[1].m_vertex - vertexArr[0].m_vertex;
	AcGeVector3d vec2 = AcGeVector3d::kIdentity;
	size_t i;
	for(i = 2; i < vertexArr.size(); ++i)
	{
		AcGeVector3d vecTemp = vertexArr[i].m_vertex - vertexArr[i - 1].m_vertex;
		if(!vecTemp.isParallelTo(vec1))
		{
			vec2 = vecTemp;
			break;
		}
	}
	if(vec2 != AcGeVector3d::kIdentity)
	{
		AcGeVector3d Vec1Vec2 = vec1.crossProduct(vec2);
		if(norm.angleTo(Vec1Vec2) < PI / 2.0)
			PolyNorm = Vec1Vec2;
		else
			PolyNorm = -Vec1Vec2;
	}
	pPoly->setNormal(PolyNorm);

	AcGeVector3d vect = PolyNorm.normal();
	AcGeVector3d Wy(0, 1, 0), Wz(0, 0, 1), Ax, Ay;
	if (fabs(vect[X]) <1.0/64 && fabs(vect[Y])<1.0/64) 
		Ax = Wy.crossProduct(vect);
	else
		Ax = Wz.crossProduct(vect);
	Ax.normalize();
	Ay = vect.crossProduct(Ax);
	Ay.normalize();

	AcGeMatrix3d mat;
	mat.setCoordSystem(AcGePoint3d::kOrigin, Ax, Ay, vect);
	mat.invert();

	AcGePoint3d pt1, pt3;
	AcGePoint2d pt1_2D, pt3_2D;
	pt1 = vertexArr[0].m_vertex;
	pt1.transformBy(mat);
	pt1_2D.set(pt1.x, pt1.y);
	pPoly->setElevation(pt1.z);
	for(i = 0; i < vertexArr.size(); i++)
	{
		pt1 = vertexArr[i].m_vertex;
		pt1.transformBy(mat);
		pt1_2D.set(pt1.x, pt1.y);
		switch(vertexArr[i].m_vertexFlag)
		{
		case POLYLINEVERTE_FLAG_L:
			pPoly->addVertexAt(pPoly->numVerts(), pt1_2D, 0, 0, 0);
			break;
		case POLYLINEVERTE_FLAG_A:
			if(i == vertexArr.size() - 2)
			{
				AcGeCircArc3d geArc(vertexArr[i].m_vertex, vertexArr[i + 1].m_vertex, vertexArr[0].m_vertex);
				AcGePoint3d cen = geArc.center();
				AcGeVector3d spCen, epCen;
				spCen = vertexArr[i].m_vertex - cen;
				epCen = vertexArr[0].m_vertex - cen;
				double angle = spCen.angleTo(epCen, geArc.normal());
				double bulge = tan(angle / 4.0);
				if(geArc.normal().angleTo(PolyNorm) > PI / 2.0)
					bulge = -bulge;
				pPoly->addVertexAt(pPoly->numVerts(), pt1_2D, bulge, 0, 0);
				pPoly->setClosed(Adesk::kTrue);
			}
			else if(i < vertexArr.size() - 2)
			{
				AcGeCircArc3d geArc(vertexArr[i].m_vertex, vertexArr[i + 1].m_vertex, vertexArr[i + 2].m_vertex);
				AcGePoint3d cen = geArc.center();
				AcGeVector3d spCen, epCen;
				spCen = vertexArr[i].m_vertex - cen;
				epCen = vertexArr[i + 2].m_vertex - cen;
				double angle = spCen.angleTo(epCen, geArc.normal());
				double bulge = tan(angle / 4.0);
				if(geArc.normal().angleTo(PolyNorm) > PI / 2.0)
					bulge = -bulge;
				pPoly->addVertexAt(pPoly->numVerts(), pt1_2D, bulge, 0, 0);
			}
			++i;
			break;
		case POLYLINEVERTE_FLAG_AM:
			break;
		case POLYLINEVERTE_FLAG_LC:
			pPoly->addVertexAt(pPoly->numVerts(), pt1_2D, 0, 0, 0);
			pPoly->setClosed(Adesk::kTrue);
			break;
		case POLYLINEVERTE_FLAG_AC:
			if(i <= vertexArr.size() - 2)
			{
				AcGeCircArc3d geArc(vertexArr[i].m_vertex, vertexArr[i + 1].m_vertex, vertexArr[0].m_vertex);
				AcGePoint3d cen = geArc.center();
				AcGeVector3d spCen, epCen;
				spCen = vertexArr[i].m_vertex - cen;
				epCen = vertexArr[0].m_vertex - cen;
				double angle = spCen.angleTo(epCen, geArc.normal());
				double bulge = tan(angle / 4.0);
				if(geArc.normal().angleTo(PolyNorm) > PI / 2.0)
					bulge = -bulge;
				pPoly->addVertexAt(pPoly->numVerts(), pt1_2D, bulge, 0, 0);
				pPoly->setClosed(Adesk::kTrue);
			}
			i = vertexArr.size() - 1;
			break;
		case POLYLINEVERTE_FLAG_C:
			if(i == 0 && vertexArr.size() >= 3)
			{
				pt3 = vertexArr[i + 2].m_vertex;
				pt3.transformBy(mat);
				pt3_2D.set(pt3.x, pt3.y);
				AcGeCircArc3d geArc(vertexArr[i].m_vertex, vertexArr[i + 1].m_vertex, vertexArr[i + 2].m_vertex);
				AcGePoint3d cen = geArc.center();
				AcGeVector3d spCen, epCen;
				spCen = vertexArr[i].m_vertex - cen;
				epCen = vertexArr[0].m_vertex - cen;
				double angle = spCen.angleTo(epCen, geArc.normal());
				double bulge = tan(angle / 4.0);
				double angle2 = PI * 2.0 - angle;
				double bulge2 = tan(angle2 / 4.0);
				if(geArc.normal().angleTo(PolyNorm) > PI / 2.0)
				{
					bulge = -bulge;
					bulge2 = -bulge2;
				}
				pPoly->addVertexAt(pPoly->numVerts(), pt1_2D, bulge, 0, 0);
				pPoly->addVertexAt(pPoly->numVerts(), pt3_2D, bulge2, 0, 0);
				pPoly->setClosed(Adesk::kTrue);
			}
			i = vertexArr.size() - 1;
			break;
		default:
			break;
		}
	}
	curveSegments.append(pPoly);
	return Acad::eOk;
}

// Within the custom entity, add the kDrawableViewIndependentViewportDraw flag to 
// the base-class version of setAttributes()...
//Adesk::UInt32 PDPrimary3D::setAttributes(AcGiDrawableTraits* pTraits)
//{
//    return AcDbEntity::setAttributes(pTraits) | kDrawableViewIndependentViewportDraw;
//}

//int getOrbitFlag()
//{
//    DWORD IsInOrbit = 1;
//    HKEY hKey;
//    if (RegOpenKeyEx(HKEY_CURRENT_USER, "SOFTWARE\\PDSoft\\Piping", 0,
//        KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS)
//        return 0;
//    DWORD IsInOrbitVal;
//    DWORD IsInOrbitType;
//    DWORD IsInOrbitSize = sizeof(IsInOrbitVal);
//    //get "MenuType" key's value.
//    if (RegQueryValueEx(hKey, "IsInOrbit", NULL, &IsInOrbitType, (LPBYTE)&IsInOrbitVal, &IsInOrbitSize) == ERROR_SUCCESS)
//    {
//        RegCloseKey(hKey);
//        return (int)IsInOrbitVal;
//    }
//    else
//    {
//        RegCloseKey(hKey);
//        return 0;
//    }
//}

AcBr::ErrorStatus nodeDisplay(const AcBrNode& node, AcGePoint3dArray& pts)
{
	AcBr::ErrorStatus returnValue = AcBr::eOk;

	AcGePoint3d nodePoint;	
	returnValue = node.getPoint(nodePoint);
	if (returnValue != AcBr::eOk) {
		acutPrintf(_T("\n Error in AcBrNode::getPoint:"));
		return returnValue;
	}
	pts.append((const AcGePoint3d&)nodePoint);

	return returnValue;
}

// Utility function to extract a useful, unbounded surface with native
// surface definition data, from the external bounded surface
AcBr::ErrorStatus
getNativeSurface(const AcBrFace& faceEntity,
				 AcGeSurface*&   surfaceGeometry,
				 AcGeSurface*&   nativeGeometry)
{
	AcBr::ErrorStatus returnValue = faceEntity.getSurface(surfaceGeometry);  
	if (returnValue != AcBr::eOk) {
		acutPrintf(_T("\n Error in AcBrFace::getSurface:"));
		return returnValue;
	}
	if (surfaceGeometry == NULL) {
		acutPrintf(_T("\n getNativeSurface: external bounded surface is undefined\n"));
		returnValue = AcBr::eMissingGeometry;
		return returnValue;
	}
	if (surfaceGeometry->type() != kExternalBoundedSurface) {
		acutPrintf(_T("\n getNativeSurface: surface is not an external bounded surface\n"));
		returnValue = AcBr::eMissingGeometry;
		return returnValue;
	}
	AcGeExternalSurface baseGeometry;
	((AcGeExternalBoundedSurface*)surfaceGeometry)->getBaseSurface(baseGeometry);
	if (!baseGeometry.isDefined()) {
		acutPrintf(_T("\n getNativeSurface: external surface is undefined\n"));
		returnValue = AcBr::eMissingGeometry;
		return returnValue;
	}
	if (!baseGeometry.isNativeSurface(nativeGeometry)
		|| (nativeGeometry == NULL)) {
			acutPrintf(_T("\n getNativeSurface: native surface is undefined\n"));
			returnValue = AcBr::eMissingGeometry;
			return returnValue;
	}
	return returnValue;
}

void PDPrimary3D::getSolidInfo(const AcDbObjectId &solidId)
{
	assertWriteEnabled();
	AcDb::SubentType subType = AcDb::kNullSubentType;
	AcDbFullSubentPath subPath(kNullSubent);
	AcDbObjectIdArray objIdList;
	objIdList.append(solidId);
	subPath = kNullSubent;
	subPath.objectIds() = objIdList;
	AcBrEntity* pEnt = new AcBrBrep();
	AcBr::ErrorStatus returnValue = AcBr::eOk;
	returnValue = pEnt->set(subPath);
	if (returnValue != AcBr::eOk) {
		acutPrintf(_T("\n Error in AcBrEntity::set:"));
		return;
	}
	AcBr::ValidationLevel vlevel = AcBr::kFullValidation;
	returnValue = pEnt->setValidationLevel(vlevel);
	if (returnValue != AcBr::eOk) {
		acutPrintf(_T("\n Error in AcBrEntity::setValidationLevel:"));
		return ;
	}

	AcBrBrep *brepEntity = (AcBrBrep *)pEnt;
	AcBrBrep brepOwner;
	returnValue = brepEntity->getBrep(brepOwner);
	if (returnValue != eOk) {
		acutPrintf(_T("\n Error in AcBrBrep::getBrep:"));
		return ;
	}
	//if (!brepEntity->isEqualTo(&brepOwner)) {
	//	acutPrintf(_T("\n Brep owner is out of sync with brep!"));
	//	return ;
	//}

	// make a global complex traverser
	AcBrBrepComplexTraverser brepComplexTrav;
	returnValue = brepComplexTrav.setBrep(*brepEntity);
	if (returnValue != AcBr::eOk) {
		acutPrintf(_T("\n Error in AcBrBrepComplexTraverser::setBrep:"));
		return ;
	}

	m_ptsArr.clear();

	// dump the complexes
	while (!brepComplexTrav.done() && (returnValue == AcBr::eOk)) {
		// make a complex shell traverser
		AcBrComplexShellTraverser complexShellTrav;
		returnValue = complexShellTrav.setComplex(brepComplexTrav);
		if (returnValue != AcBr::eOk) {
			acutPrintf(_T("\n Error in AcBrComplexShellTraverser::setComplex:"));
			return ;
		}

		// dump the shells
		while (!complexShellTrav.done() && (returnValue == AcBr::eOk)) {
			AcBrShell shell;
			returnValue = complexShellTrav.getShell(shell);
			if (returnValue != AcBr::eOk) {	
				acutPrintf(_T("\n Error in AcBrComplexShellTraverser::getShell:")); 
				return ;
			}
			// make a shell face traverser
			AcBrShellFaceTraverser shellFaceTrav;
			returnValue = shellFaceTrav.setShell(complexShellTrav);
			if (returnValue != AcBr::eOk) {
				acutPrintf(_T("\n Error in AcBrShellFaceTraverser::setShell:"));
				return ;
			}

			// count the faces
			while (!shellFaceTrav.done() && (returnValue == AcBr::eOk)) {
				AcBrFace currentFace;
				returnValue = shellFaceTrav.getFace(currentFace);
				if (returnValue != AcBr::eOk) {
					acutPrintf(_T("\n Error in AcBrShellFaceTraverser::getFace:"));
					return ;
				}

				// make sure that comparing different types returns kFalse
				// rather than crashing!
				if (currentFace.isEqualTo(brepEntity)) {
					acutPrintf(_T("\n Brep and face have the same contents (impossible!)"));
					return ;
				}

				AcGe::EntityId entId;
				returnValue = currentFace.getSurfaceType(entId);  
				if (returnValue != AcBr::eOk) {
					acutPrintf(_T("\n Error in AcBrFace::getSurfaceType:"));
					return ;
				}

				AcGeSurface* surfaceGeometry = NULL;
				AcGeSurface* nativeGeometry = NULL;

				// NOTE: ignore unsupported geometry types for now, since we already know 
				// that elliptic cylinders and elliptic cones are rejected by AcGe, but we
				// can still perform useful evaluations on the external bounded surface.
				returnValue = getNativeSurface(currentFace, surfaceGeometry, nativeGeometry);  
				if ((returnValue != AcBr::eOk) && (returnValue
					!= (AcBr::ErrorStatus)Acad::eInvalidInput)) {
						acutPrintf(_T("\n Error in getNativeSurface:"));
						delete surfaceGeometry;
						delete nativeGeometry;
						return ;
				}

				// conditionally set the mesh controls based on surface type
				AcBrMesh2dControl meshCtrl;
				switch (entId) {
				case(kPlane):
					if ((returnValue = meshCtrl.setMinSubdivisionsInU()) != eOk) return ;
					if ((returnValue = meshCtrl.setMinSubdivisionsInV()) != eOk) return ;
					if ((returnValue = meshCtrl.setMaxSubdivisions()) != eOk) return ;
					if ((returnValue = meshCtrl.setMaxNodeSpacing(2.0)) != eOk) return ;
					if ((returnValue = meshCtrl.setAngTol(90.0 * kDeg2Rad)) != eOk) return ;
					if ((returnValue = meshCtrl.setDistTol(0.5)) != eOk) return ;
					if ((returnValue = meshCtrl.setMaxAspectRatio(8.0)) != eOk) return ;
					if ((returnValue = meshCtrl.setElementShape(AcBr::kAllQuadrilaterals/*kAllPolygons*/)) != eOk)
						return ;
					break;
				case(kSphere):
					if ((returnValue = meshCtrl.setMinSubdivisionsInU(2L)) != eOk) return ;
					if ((returnValue = meshCtrl.setMinSubdivisionsInV(2L)) != eOk) return ;
					if ((returnValue = meshCtrl.setMaxSubdivisions()) != eOk) return ;
					if ((returnValue = meshCtrl.setMaxNodeSpacing()) != eOk) return ;
					if ((returnValue = meshCtrl.setAngTol()) != eOk) return ;
					if ((returnValue = meshCtrl.setDistTol(0.5)) != eOk) return ;
					if ((returnValue = meshCtrl.setMaxAspectRatio(2.0)) != eOk) return ;
					if ((returnValue = meshCtrl.setElementShape(AcBr::kAllQuadrilaterals)) != eOk)
						return ;
					break;
				case(kTorus):
					if ((returnValue = meshCtrl.setMinSubdivisionsInU(2L)) != eOk) return ;
					if ((returnValue = meshCtrl.setMinSubdivisionsInV(2L)) != eOk) return ;
					if ((returnValue = meshCtrl.setMaxSubdivisions()) != eOk) return ;
					if ((returnValue = meshCtrl.setMaxNodeSpacing()) != eOk) return ;
					if ((returnValue = meshCtrl.setAngTol()) != eOk) return ;
					if ((returnValue = meshCtrl.setDistTol(0.5)) != eOk) return ;
					if ((returnValue = meshCtrl.setMaxAspectRatio(4.0)) != eOk) return ;
					if ((returnValue = meshCtrl.setElementShape(AcBr::kAllQuadrilaterals)) != eOk)
						return ;
					break;
				case(kCylinder):
					if ((returnValue = meshCtrl.setMinSubdivisionsInU(2L)) != eOk) return ;
					if ((returnValue = meshCtrl.setMinSubdivisionsInV(2L)) != eOk) return ;
					if ((returnValue = meshCtrl.setMaxSubdivisions()) != eOk) return ;
					if ((returnValue = meshCtrl.setMaxNodeSpacing()) != eOk) return ;
					if ((returnValue = meshCtrl.setAngTol()) != eOk) return ;
					if ((returnValue = meshCtrl.setDistTol(0.5)) != eOk) return ;
					if ((returnValue = meshCtrl.setMaxAspectRatio(2.0)) != eOk) return ;
					if ((returnValue = meshCtrl.setElementShape(AcBr::kAllQuadrilaterals)) != eOk)
						return ;
					break;
				case(kCone):
					if ((returnValue = meshCtrl.setMinSubdivisionsInU(2L)) != eOk) return ;
					if ((returnValue = meshCtrl.setMinSubdivisionsInV(2L)) != eOk) return ;
					if ((returnValue = meshCtrl.setMaxSubdivisions()) != eOk) return ;
					if ((returnValue = meshCtrl.setMaxNodeSpacing()) != eOk) return ;
					if ((returnValue = meshCtrl.setAngTol()) != eOk) return ;
					if ((returnValue = meshCtrl.setDistTol(0.5)) != eOk) return ;
					if ((returnValue = meshCtrl.setMaxAspectRatio(2.0)) != eOk) return ;
					if ((returnValue = meshCtrl.setElementShape(AcBr::kAllQuadrilaterals)) != eOk)
						return ;
					break;
				case(kNurbSurface):
					if ((returnValue = meshCtrl.setMinSubdivisionsInU(2L)) != eOk) return ;
					if ((returnValue = meshCtrl.setMinSubdivisionsInV(2L)) != eOk) return ;
					if ((returnValue = meshCtrl.setMaxSubdivisions()) != eOk) return ;
					if ((returnValue = meshCtrl.setMaxNodeSpacing()) != eOk) return ;
					if ((returnValue = meshCtrl.setAngTol(30.0 * kDeg2Rad)) != eOk) return ;
					if ((returnValue = meshCtrl.setDistTol(0.5)) != eOk) return ;
					if ((returnValue = meshCtrl.setMaxAspectRatio(2.0)) != eOk) return ;
					if ((returnValue = meshCtrl.setElementShape(AcBr::kAllQuadrilaterals)) != eOk)
						return ;
					break;
					// NOTE: This surface is not yet supported in AcGe, so we infer the definition
					// data by analysing evaluated data on the external bounded surface.
				case(kEllipCylinder):
					if ((returnValue = meshCtrl.setMinSubdivisionsInU(2L)) != eOk) return ;
					if ((returnValue = meshCtrl.setMinSubdivisionsInV(2L)) != eOk) return ;
					if ((returnValue = meshCtrl.setMaxSubdivisions()) != eOk) return ;
					if ((returnValue = meshCtrl.setMaxNodeSpacing()) != eOk) return ;
					if ((returnValue = meshCtrl.setAngTol()) != eOk) return ;
					if ((returnValue = meshCtrl.setDistTol(0.5)) != eOk) return ;
					if ((returnValue = meshCtrl.setMaxAspectRatio(4.0)) != eOk) return ;
					if ((returnValue = meshCtrl.setElementShape(AcBr::kAllQuadrilaterals)) != eOk)
						return ;
					break;
					// NOTE: This surface is not yet supported in AcGe, so we infer the definition
					// data by analysing evaluated data on the external bounded surface.
				case(kEllipCone):
					if ((returnValue = meshCtrl.setMinSubdivisionsInU(2L)) != eOk) return ;
					if ((returnValue = meshCtrl.setMinSubdivisionsInV(2L)) != eOk) return ;
					if ((returnValue = meshCtrl.setMaxSubdivisions()) != eOk) return ;
					if ((returnValue = meshCtrl.setMaxNodeSpacing()) != eOk) return ;
					if ((returnValue = meshCtrl.setAngTol()) != eOk) return ;
					if ((returnValue = meshCtrl.setDistTol(0.5)) != eOk) return ;
					if ((returnValue = meshCtrl.setMaxAspectRatio(4.0)) != eOk) return ;
					if ((returnValue = meshCtrl.setElementShape(AcBr::kAllQuadrilaterals)) != eOk)
						return ;
					break;
				default:
					return ;
				} // end switch(entId)	
				if(nativeGeometry != NULL)
					delete nativeGeometry;
				if(surfaceGeometry != NULL)
					delete surfaceGeometry;

				// make the mesh filter from the topology entity and the mesh controls
#if _MSC_VER >= 1400
				const AcBrEntity* meshEnt = (AcBrEntity*)&currentFace;//VC8:Used const pointer to resolve convserion error
#else
				AcBrEntity* meshEnt = (AcBrEntity*)&currentFace;
#endif
				AcBrMesh2dFilter meshFilter;
				meshFilter.insert(make_pair(meshEnt, (const AcBrMesh2dControl)meshCtrl));

				// generate the mesh, display any errors and attempt to dump all
				// generated elements (most errors are not fatal so we want to do
				// the best we can with whatever subset of the face was meshed).
				AcBrMesh2d faceMesh;
				if ((returnValue = faceMesh.generate(meshFilter)) != eOk) {
					acutPrintf(_T("\n Error in AcBrMesh2d::generate:"));
				}

				// check to see if the mesh owner is the face we used in the filter
				AcBrEntity* meshOwner = NULL;
				if ((returnValue = faceMesh.getEntityAssociated(meshOwner)) != eOk) {
					acutPrintf(_T("\n Error in AcBrMesh2d::getEntityAssociate:"));
				}
				//if (!meshEnt->isEqualTo(meshOwner)) {
				//	acutPrintf(_T("\nMesh owner is not the face we asked to mesh!"));
				//}

				// make a global element traverser
				AcBrMesh2dElement2dTraverser meshElemTrav;
				returnValue = meshElemTrav.setMesh(faceMesh);
				if (returnValue != AcBr::eOk) {
					acutPrintf(_T("\n Error in AcBrMesh2dElement2dTraverser::setMesh:"));
					return ;
				}

				// display the elements
				while (!meshElemTrav.done() && (returnValue == AcBr::eOk) && !acedUsrBrk()) {
					// convert the nodes into a 3d point array for AcDbPolyline
					AcGePoint3dArray pts;

					AcBrElement2dNodeTraverser elemNodeTrav;
					returnValue = elemNodeTrav.setElement(meshElemTrav);
					if (returnValue != AcBr::eOk) {
						acutPrintf(_T("\n Error in AcBrElement2dNodeTraverser::setElement:")); 
						return ;
					} 

					while (!elemNodeTrav.done() && (returnValue == AcBr::eOk)) { 
						AcBrNode node;
						returnValue = elemNodeTrav.getNode(node);
						if (returnValue != AcBr::eOk) {	
							acutPrintf(_T("\n Error in AcBrElement2dNodeTraverser::getNode:")); 
							return ;
						}

						// add the node geometry data to the lightweight polyline point array
						returnValue = nodeDisplay(node, pts);
						if (returnValue != AcBr::eOk) {
							acutPrintf(_T("\n Error in nodeDisplay:"));
							return ;
						}

						returnValue = elemNodeTrav.next();
						if (returnValue != AcBr::eOk) {
							acutPrintf(_T("\n Error in AcBrElement2dNodeTraverser::next:"));
							return ;
						}
					} // end element while

					//AcGeVector3d sufNormal;
					//elemNodeTrav.getSurfaceNormal(sufNormal);
					//if(pts.length() > 2)
					//{
					//	AcGeVector3d firstVec = pts[1] - pts[0];
					//	AcGeVector3d secondVec = pts[2] - pts[1];
					//	if(firstVec.isParallelTo(secondVec))
					//	{
					//		for(int i = 2; i < pts.length(); ++i)
					//		{
					//			secondVec = pts[i + 1] - pts[i];
					//			if(!firstVec.isParallelTo(secondVec))
					//				break;
					//		}
					//	}
					//	if(!firstVec.isParallelTo(secondVec))
					//	{
					//		AcGeVector3d ptsNormal = firstVec.crossProduct(secondVec);
					//		double angDelta = sufNormal.angleTo(ptsNormal);
					//		if(angDelta > PI / 2.0)
					//			pts.reverse();
					//	}
					//}
					m_ptsArr.push_back(pts);

					returnValue = meshElemTrav.next();
					if (returnValue != AcBr::eOk) {
						acutPrintf(_T("\n Error in AcBrMesh2dElement2dTraverser::next:"));  
						return ;
					}

				}  // end mesh while


				returnValue = shellFaceTrav.next();
				if (returnValue != AcBr::eOk) {
					acutPrintf(_T("\n Error in AcBrShellFaceTraverser::next:"));  
					return ;
				}

			} // end face while

			returnValue = complexShellTrav.next();
			if (returnValue != AcBr::eOk) {
				acutPrintf(_T("\n Error in AcBrComplexShellTraverser::next:"));  
				return ;
			}

		} // end shell while

		returnValue = brepComplexTrav.next();
		if (returnValue != AcBr::eOk) {
			acutPrintf(_T("\n Error in AcBrBrepComplexTraverser::next:"));  
			return ;
		}

	} // end complex while

/*
	Adesk::UInt32 minSubdivisionsInU = 0L;//8;
	Adesk::UInt32 minSubdivisionsInV = 0L;//8;
	Adesk::UInt32 maxSubdivisions = 0L;//32;
	double maxNodeSpacing = 0.0;//1;
	double angTol = 0.0;//0.1;
	double distTol = 0.0;//0.1;
	double maxAspectRatio = 0.0;//2;
	AcBr::Element2dShape elementShape = AcBr::kDefault;//kAllPolygons;

	AcBrMesh2dControl meshCtrl;
	if ((returnValue = meshCtrl.setMinSubdivisionsInU(minSubdivisionsInU)) != eOk)
		return ;
	if ((returnValue = meshCtrl.setMinSubdivisionsInV(minSubdivisionsInV)) != eOk)
		return ;
	if ((returnValue = meshCtrl.setMaxSubdivisions(maxSubdivisions)) != eOk)
		return ;
	if ((returnValue = meshCtrl.setMaxNodeSpacing(maxNodeSpacing)) != eOk)
		return ;
	if ((returnValue = meshCtrl.setAngTol(angTol)) != eOk) return ;
	if ((returnValue = meshCtrl.setDistTol(distTol)) != eOk) return ;
	if ((returnValue = meshCtrl.setMaxAspectRatio(maxAspectRatio)) != eOk)
		return ;
	if ((returnValue = meshCtrl.setElementShape(elementShape)) != eOk)
		return ;

	// make the mesh filter from the topology entity and the mesh controls
#if _MSC_VER >= 1400
	const AcBrEntity* meshEnt = (AcBrEntity*)&currentFace;//VC8:Used const pointer to resolve convserion error
#else
	AcBrEntity* meshEnt = (AcBrEntity*)&currentFace;
#endif
	AcBrMesh2dFilter meshFilter;
	meshFilter.insert(make_pair(meshEnt, (const AcBrMesh2dControl)meshCtrl));

	// generate the mesh, display any errors and attempt to dump all
	// generated elements (most errors are not fatal so we want to do
	// the best we can with whatever subset of the brep was meshed).
	AcBrMesh2d brepMesh;
	if ((returnValue = brepMesh.generate(meshFilter)) != eOk) {
		acutPrintf(_T("\n Error in AcBrMesh2d::generate:"));
	}

	// check to see if the mesh owner is the brep we used in the filter
	AcBrEntity* meshOwner = NULL;
	if ((returnValue = brepMesh.getEntityAssociated(meshOwner)) != eOk) {
		acutPrintf(_T("\n Error in AcBrMesh2d::getEntityAssociate:"));
	}
	if (!meshEnt->isEqualTo(meshOwner)) {
		acutPrintf(_T("\nMesh owner is not the brep we asked to mesh!"));
	}

	// make a global element traverser
	AcBrMesh2dElement2dTraverser meshElemTrav;
	returnValue = meshElemTrav.setMesh(brepMesh);
	if (returnValue != AcBr::eOk) {
		acutPrintf(_T("\n Error in AcBrMesh2dElement2dTraverser::setMesh:"));
		return ;
	}

	m_ptsArr.clear();

	// display the elements
	while (!meshElemTrav.done() && (returnValue == AcBr::eOk) && !acedUsrBrk()) {
		// convert the nodes into a 3d point array for AcDbPolyline
		AcGePoint3dArray pts;

		AcBrElement2dNodeTraverser elemNodeTrav;
		returnValue = elemNodeTrav.setElement(meshElemTrav);
		if (returnValue != AcBr::eOk) {
			acutPrintf(_T("\n Error in AcBrElement2dNodeTraverser::setElement:")); 
			return ;
		} 

		while (!elemNodeTrav.done() && (returnValue == AcBr::eOk)) { 
			AcBrNode node;
			returnValue = elemNodeTrav.getNode(node);
			if (returnValue != AcBr::eOk) {	
				acutPrintf(_T("\n Error in AcBrElement2dNodeTraverser::getNode:")); 
				return ;
			}

			// add the node geometry data to the lightweight polyline point array
			returnValue = nodeDisplay(node, pts);
			if (returnValue != AcBr::eOk) {
				acutPrintf(_T("\n Error in nodeDisplay:"));
				return ;
			}

			returnValue = elemNodeTrav.next();
			if (returnValue != AcBr::eOk) {
				acutPrintf(_T("\n Error in AcBrElement2dNodeTraverser::next:"));
				return ;
			}
		} // end element while

		pts.reverse();
		m_ptsArr.push_back(pts);

		returnValue = meshElemTrav.next();
		if (returnValue != AcBr::eOk) {
			acutPrintf(_T("\n Error in AcBrMesh2dElement2dTraverser::next:"));  
			return ;
		}

	}  // end mesh while
*/
}

//void PDPrimary3D::getSolidWorldDrawInfo(const AcDbObjectId &solidId)
//{
//	ArxDbgGiWorldDraw wd;
//	wd.setIsDragging(FALSE);
//	wd.setDeviation(0.5);
//	wd.setIsolines(8);
//	wd.setRegenType(kAcGiRenderCommand);
//	((ArxDbgGiWorldGeometry*)(&(wd.geometry())))->pPrimary3D = this;
//
//	int i;
//	for(i = 0; i < m_shelldata.size(); ++i)
//	{
//		delete m_shelldata[i].pEdgeData;
//		delete m_shelldata[i].pFaceData;
//		delete m_shelldata[i].pVertexData;
//	}
//	m_shelldata.clear();
//
//	Acad::ErrorStatus es = Acad::eOk;
//	AcDbEntity *pEnt = NULL;
//	if((es = acdbOpenAcDbEntity(pEnt, solidId, AcDb::kForRead)) == Acad::eOk)
//	{
//		pEnt->worldDraw(&wd);
//		pEnt->close();
//	}
//}