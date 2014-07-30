// PDPrism1.h: interface for the PDPrism1 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PDPRISM_H__2FC3CCD2_AC05_4FDD_8162_B63A67808FD0__INCLUDED_)
#define AFX_PDPRISM_H__2FC3CCD2_AC05_4FDD_8162_B63A67808FD0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PDPrimary3D.h"


class PDPrism1 : public PDPrimary3D  
{
public:
	ACRX_DECLARE_MEMBERS(PDPrism1); 
	PDPrism1(AcGePoint3d cP1,AcGePoint3d cP2,AcGePoint3d eP3,Adesk::UInt32 eNum, bool HasSnap = false);
//	GlePrism(GlePrism&);
	PDPrism1(bool HasSnap = false);
	virtual ~PDPrism1();
public:
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
	Acad::ErrorStatus getVertices(AcGePoint3dArray& vertexArray)const;
	/*!
	* 获取m_ptcenP1所在底面的所有顶点,顶点按顺序排列
	* \param [in,out] vertexArray 
	* \return Acad::ErrorStatus
	*/
	Acad::ErrorStatus getVerticesInP1Plane(AcGePoint3dArray& vertexArray) const;
	/*!
	* 获取m_ptcenP2所在底面的所有顶点,顶点按顺序排列
	* \param [in,out] vertexArray 
	* \return Acad::ErrorStatus
	*/
	Acad::ErrorStatus getVerticesInP2Plane(AcGePoint3dArray& vertexArray) const;

	virtual Acad::ErrorStatus	dwgInFields(AcDbDwgFiler* filer);
	virtual Acad::ErrorStatus	dwgOutFields(AcDbDwgFiler* filer) const;
	virtual Acad::ErrorStatus	dxfInFields(AcDbDxfFiler* filer);
	virtual Acad::ErrorStatus	dxfOutFields(AcDbDxfFiler* filer) const;


	Acad::ErrorStatus getpointCenP1(AcGePoint3d& ptcenP1);
	Acad::ErrorStatus getpointCenP2(AcGePoint3d& ptcenP2);
	Acad::ErrorStatus getpointEdgP3(AcGePoint3d& ptedgP3);
	Acad::ErrorStatus getedgeNum(Adesk::UInt32& edgeNum);

	const AcGePoint3d &getBottomCenter() const;
	const AcGePoint3d &getTopCenter() const;
	const AcGePoint3d &getBottomEdgeCenter() const;
	Adesk::UInt32 getEdgeNum() const;

	Acad::ErrorStatus setParameters(AcGePoint3d ptcenP1,AcGePoint3d ptcenP2,
		                            AcGePoint3d ptedgP3,Adesk::UInt32 edgeNum);

    virtual void createBody();

	//  [9/5/2007 suzhiyong]
	virtual Acad::ErrorStatus explodeTo3DSolid(AcDb3dSolid* &p3dSolid) const;

private:
	AcGePoint3d        m_ptcenP1,m_ptcenP2,m_ptedgP3;
	Adesk::UInt32       m_nedgeNum;
	//AcGeVector3d       xVector;
	//double             height;

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
MAKE_ACDBOPENOBJECT_FUNCTION(PDPrism1);

#endif // !defined(AFX_PDPRISM_H__2FC3CCD2_AC05_4FDD_8162_B63A67808FD0__INCLUDED_)
