// PDSphere.h: interface for the PDSphere class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PDSPHERE_H__193B2E82_03ED_4AAD_8479_BF02FF67BE33__INCLUDED_)
#define AFX_PDSPHERE_H__193B2E82_03ED_4AAD_8479_BF02FF67BE33__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PDPrimary3D.h"


class PDSphere : public PDPrimary3D  
{
public:
	ACRX_DECLARE_MEMBERS(PDSphere);
	PDSphere(bool HasSnap = false);
	PDSphere(AcGePoint3d ptcenter, double r, int precision = 12, bool HasSnap = false);
	virtual ~PDSphere();

private:
	double m_dRadius;
	AcGePoint3d m_ptCenter;
	Adesk::UInt32 m_nprecision;

public:
    /*---------------------------------------------------------------------------
	 * 名称: _make_eqovalpt
	 * 功能: 完成 "在设备椭球封头上画出点" 的功能
	 * 注意: 无
	 */
	int  _make_eqovalpt(double lengthR,AcGeVector3d vec,ads_point pt,ads_point normal,AcGeVector3d v) ;
	//Acad::ErrorStatus getVertices(AcGePoint3dArray& vertexArray) const;
	Acad::ErrorStatus getVertices(int Precision, AcGePoint3dArray& vertexArray, 
                                          AcGeIntArray &stdIdx, int &actPrecision) const;
	Acad::ErrorStatus getVertices(int Precision, AcGePoint3dArray& vertexArray, AcGeVector3dArray &vertexNors) const;

#ifdef _OBJECTARX2010_
	virtual Adesk::Boolean subWorldDraw(AcGiWorldDraw* mode);
	virtual Acad::ErrorStatus subTransformBy(const AcGeMatrix3d &xform);
	virtual void subList() const;
	virtual Acad::ErrorStatus   subGetGeomExtents(AcDbExtents& extents) const;
	virtual Acad::ErrorStatus subExplode(AcDbVoidPtrArray& entitySet) const;
	virtual Acad::ErrorStatus   subGetOsnapPoints(
		AcDb::OsnapMode     osnapMode,
		Adesk::GsMarker     gsSelectionMark,
		const AcGePoint3d&  pickPoint,
		const AcGePoint3d&  lastPoint,
		const AcGeMatrix3d& viewXform,
		AcGePoint3dArray&   snapPoints,
		AcDbIntArray &   geomIds) const;
	virtual Acad::ErrorStatus subGetGripPoints( AcGePoint3dArray& gripPoints,
		AcDbIntArray& osnapModes,
		AcDbIntArray& geomIds) const;
	virtual Acad::ErrorStatus subMoveGripPointsAt(const AcDbIntArray& indices,
		const AcGeVector3d& offset);
#else
	virtual Adesk::Boolean worldDraw(AcGiWorldDraw* mode);
	virtual Acad::ErrorStatus transformBy(const AcGeMatrix3d &xform);
	virtual void list() const;
	virtual Acad::ErrorStatus   getGeomExtents(AcDbExtents& extents) const;
	virtual Acad::ErrorStatus explode(AcDbVoidPtrArray& entitySet) const;
	virtual Acad::ErrorStatus getOsnapPoints(  AcDb::OsnapMode osnapMode,
		int gsSelectionMark,
		const AcGePoint3d&    pickPoint,
		const AcGePoint3d&    lastPoint,
		const AcGeMatrix3d&   viewXform,
		AcGePoint3dArray&     snapPoints,
		AcDbIntArray&         geomIds) const;
	virtual Acad::ErrorStatus getGripPoints( AcGePoint3dArray& gripPoints,
		AcDbIntArray& osnapModes,
		AcDbIntArray& geomIds) const;
	virtual Acad::ErrorStatus moveGripPointsAt(const AcDbIntArray& indices,
		const AcGeVector3d& offset);
#endif

	virtual Acad::ErrorStatus	dwgInFields(AcDbDwgFiler* filer);
	virtual Acad::ErrorStatus	dwgOutFields(AcDbDwgFiler* filer) const;
	virtual Acad::ErrorStatus	dxfInFields(AcDbDxfFiler* filer);
	virtual Acad::ErrorStatus	dxfOutFields(AcDbDxfFiler* filer) const;

    
	//////////// adding function
	Acad::ErrorStatus getgrippoints(AcGePoint3dArray& gripArray) const;

	Acad::ErrorStatus getRadius(double& radius);
	Acad::ErrorStatus getpointCenter(AcGePoint3d& ptcenter);
	Acad::ErrorStatus getprecision(Adesk::UInt32& precision);

	Acad::ErrorStatus setParameters(AcGePoint3d ptcenter, double radius,Adesk::UInt32 precision);
    virtual void createBody();

	//  [9/5/2007 suzhiyong]
	virtual Acad::ErrorStatus explodeTo3DSolid(AcDb3dSolid* &p3dSolid) const;

#ifndef _OBJECTARX2010_
private:
	void *operator new[](unsigned nSize) { return 0; }
	void operator delete[](void *p) {};
	void *operator new[](unsigned nSize, const char *file, int line) { return 0; }
#endif

public:
  //zxb,20090113,增加audit，暂时只check自身成员数据
  virtual Acad::ErrorStatus audit(AcDbAuditInfo* pAuditInfo);
  virtual void setDefault(); //设置缺省的对象数据
  virtual bool isValidData(double &f); //检查是否成员数据均是合法数据, 并返回出错数

};
MAKE_ACDBOPENOBJECT_FUNCTION(PDSphere);

#endif // !defined(AFX_PDSPHERE_H__193B2E82_03ED_4AAD_8479_BF02FF67BE33__INCLUDED_)