// PDBox.h: interface for the PDBox class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PDBOX_H__D16EE349_78FB_47EE_824A_E6AF00638BC0__INCLUDED_)
#define AFX_PDBOX_H__D16EE349_78FB_47EE_824A_E6AF00638BC0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PDPrimary3D.h"
#include <vector>


class PDBox : public PDPrimary3D  
{
public:
	ACRX_DECLARE_MEMBERS(PDBox);  
	PDBox(bool HasSnap = false);
	PDBox(const AcGePoint3d &orn, double L, double W, double H,
		  const AcGeVector3d &xVec, const AcGeVector3d &yVec, 
          bool HasSnap = false, int CorrType = 1/* 0: 左手系，1: 右手系*/);
	virtual ~PDBox();

#ifdef _OBJECTARX2010_
	virtual Adesk::Boolean    subWorldDraw(AcGiWorldDraw* mode);
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
	virtual Acad::ErrorStatus	subIntersectWith(
		const AcDbEntity* pEnt,
		AcDb::Intersect intType, 
		AcGePoint3dArray& points,
		Adesk::GsMarker thisGsMarker = 0, 
		Adesk::GsMarker otherGsMarker = 0) const;
#else
	virtual Adesk::Boolean    worldDraw(AcGiWorldDraw* mode);
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
	virtual Acad::ErrorStatus	intersectWith(
		const AcDbEntity*   ent,
		AcDb::Intersect     intType,
		AcGePoint3dArray&      points,
		int                 thisGsMarker  = 0,
		int                 otherGsMarker = 0)
		const;
#endif
//	virtual Acad::ErrorStatus getCoordSys(AcGeMatrix3d& mat) const;

	virtual Acad::ErrorStatus	dwgInFields(AcDbDwgFiler* filer);
	virtual Acad::ErrorStatus	dwgOutFields(AcDbDwgFiler* filer) const;
	virtual Acad::ErrorStatus	dxfInFields(AcDbDxfFiler* filer);
	virtual Acad::ErrorStatus	dxfOutFields(AcDbDxfFiler* filer) const;

/*	Acad::ErrorStatus intersectWith( const AcDbEntity* ent,
										AcDb::Intersect intType,
										AcGePoint3dArray& points,
										int ///*thisGsMarker,
										int ///*otherGsMarker) ;
*/
	Acad::ErrorStatus getVertices(AcGePoint3dArray& vertexArray) const;

/*
	Acad::ErrorStatus moveGripPointsAt(const AcDbIntArray& indices,
										const AcGeVector3d& offset);


	Acad::ErrorStatus   osnapEnd(const AcGePoint3d& pickPoint,
				          AcGePoint3dArray& snapPoints) const;
	Acad::ErrorStatus   osnapIns(const AcGePoint3d& pickPoint,
				          AcGePoint3dArray& snapPoints) const;
     

	virtual Acad::ErrorStatus getStretchPoints(AcGePoint3dArray& stretchPoints) const;

    virtual Acad::ErrorStatus moveStretchPointsAt(    const AcDbIntArray& indices,
      const AcGeVector3d& offset);

	Acad::ErrorStatus    deepClone(AcDbObject*    pOwner,
                    AcDbObject*&   pClonedObject,
                    AcDbIdMapping& idMap,
                    Adesk::Boolean isPrimary) const;
*/

	Acad::ErrorStatus setParameters(const AcGePoint3d &orn, 
                                    double L, double W, double H, 
                                    const AcGeVector3d &xVec, 
                                    const AcGeVector3d &yVec, 
                                    int CorrType = 1/* 0: ｚ左手系，1: 右手系*/);
 
	const AcGeVector3d &getXvec() const;
	const AcGeVector3d &getYvec() const;
    const AcGeVector3d &getZvec() const;
	const AcGePoint3d &getOrign() const;

	/*!
	 * 获取BOX中心点
	 * \return AcGePoint3d
	 */
	AcGePoint3d getCenter() const;

	double getlength() const;
	double getwidth() const;
	double getheight() const;

    ///< 取得世界坐标系中最底点的Z值
    double getLowestZ() const;

	// Added by SongNan 2007/6/22 : begin
	///< 取得BOX体元的六个有界平面
	const std::vector<AcGeBoundedPlane> &GetAllBndPlns
								(std::vector<AcGeBoundedPlane> &plnVec)const;
	// Added by SongNan 2007/6/22 : end

    Acad::ErrorStatus getRightHandCoord(AcGePoint3d &ori, 
                                        AcGeVector3d &vecx, 
                                        AcGeVector3d &vecy, 
                                        AcGeVector3d &vecz, 
                                        double &length, 
                                        double &width, 
                                        double &height) const;

    virtual void createBody();

	//  [9/5/2007 suzhiyong]
	virtual Acad::ErrorStatus explodeTo3DSolid(AcDb3dSolid* &p3dSolid) const;

private:
	double          m_dheight;  // 高
	double          m_dwidth;   // 宽
	double          m_dlength;  // 长
	AcGePoint3d     m_porign;   // 插入点
	AcGeVector3d    m_Vectx;    // 插入X轴
    AcGeVector3d    m_Vecty;    // 插入Y轴
    // version 2
    AcGeVector3d    m_Vectz;    // 插入Z轴

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
MAKE_ACDBOPENOBJECT_FUNCTION(PDBox);

#endif // !defined(AFX_PDBOX_H__D16EE349_78FB_47EE_824A_E6AF00638BC0__INCLUDED_)
