// PDScylinder.h: interface for the PDScylinder class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PDSCYLINDER_H__0DD4A1A7_A868_425E_81D0_CFB94C213D1B__INCLUDED_)
#define AFX_PDSCYLINDER_H__0DD4A1A7_A868_425E_81D0_CFB94C213D1B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PDCylinder.h"


class PDScylinder : public PDCylinder  
{
public:
	ACRX_DECLARE_MEMBERS(PDScylinder);
	PDScylinder(bool HasSnap = false);
	PDScylinder(AcGePoint3d ptStart,AcGePoint3d ptEnd,double Diameter,AcGeVector3d vect,int Precision = 12, bool HasSnap = false);
	virtual ~PDScylinder();

public:
	virtual Acad::ErrorStatus getVertices(int Precision, AcGePoint3dArray& vertexArray, 
                                          AcGeIntArray &stdIdx, int &actPrecision) const;
	virtual Acad::ErrorStatus getVertices(int Precision, AcGePoint3dArray& vertexArray) const;
 // virtual Acad::ErrorStatus getStandardVertices(AcGePoint3dArray& vertexArray) const;
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
	virtual Acad::ErrorStatus subGetGripPoints(
		AcGePoint3dArray& gripPoints,
		AcDbIntArray& osnapModes,
		AcDbIntArray& geomIds) const;
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
	virtual Acad::ErrorStatus getGripPoints(
		AcGePoint3dArray& gripPoints,
		AcDbIntArray& osnapModes,
		AcDbIntArray& geomIds) const;
#endif

	virtual Acad::ErrorStatus	dwgInFields(AcDbDwgFiler* filer);
	virtual Acad::ErrorStatus	dwgOutFields(AcDbDwgFiler* filer) const;
	virtual Acad::ErrorStatus	dxfInFields(AcDbDxfFiler* filer);
	virtual Acad::ErrorStatus	dxfOutFields(AcDbDxfFiler* filer) const;


  Acad::ErrorStatus setParameters(const AcGePoint3d &ptStart, 
    const AcGePoint3d &ptEnd, 
    double Diameter, 
    const AcGeVector3d &vec,
    int Precision = PRIMARY3D_DEFAULT_PRECISION);

	Acad::ErrorStatus vect(AcGeVector3d& vect);
	Acad::ErrorStatus setVect(AcGeVector3d vect);

	/*!
	 * 获取P2指向P1向量与m_vect的夹角
	 * \return double 返回值为弧度值,小于或等于π/2
	 */
	double getLeanAngle()const;

    virtual void createBody();

	//  [9/5/2007 suzhiyong]
	virtual Acad::ErrorStatus explodeTo3DSolid(AcDb3dSolid* &p3dSolid) const;

private:
	AcGeVector3d m_vect;

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
MAKE_ACDBOPENOBJECT_FUNCTION(PDScylinder);

#endif // !defined(AFX_PDSCYLINDER_H__0DD4A1A7_A868_425E_81D0_CFB94C213D1B__INCLUDED_)
