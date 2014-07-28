// PDSqucone.h: interface for the PDSqucone class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PDSQUCONE_H__58A03BC2_F749_425A_82C0_747524917720__INCLUDED_)
#define AFX_PDSQUCONE_H__58A03BC2_F749_425A_82C0_747524917720__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PDPrimary3D.h"


class PDSqucone : public PDPrimary3D  
{
public:
	ACRX_DECLARE_MEMBERS(PDSqucone);
	PDSqucone(bool HasSnap = false);
	PDSqucone( double l1,double w1,double l2,double w2,double h,AcGePoint3d p,
				AcGeVector3d vh,AcGeVector3d vl,AcGeVector3d v,double m, bool HasSnap = false);
	virtual ~PDSqucone();
private:
	double m_dlength1,m_dwidth1,m_dlength2,m_dwidth2;
	double m_dheight;
	AcGePoint3d   m_ptorign;
	AcGeVector3d  m_VectH,m_VectL,m_VectV;
	double m_dlean;
	double m_dMaxLength;
public:
	virtual void createBody();
    Acad::ErrorStatus getVertices(AcGePoint3dArray& vertexArray) const;
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


	Acad::ErrorStatus getpointOrign(AcGePoint3d& ptorign);
	Acad::ErrorStatus getLength1(double& length1);
	Acad::ErrorStatus getLength2(double& length2);
	Acad::ErrorStatus getWidth1(double& width1);
	Acad::ErrorStatus getWidth2(double& width2);
	Acad::ErrorStatus getHeight(double& height);
	Acad::ErrorStatus getLean(double& lean);
	Acad::ErrorStatus getVectH(AcGeVector3d& vectH);
	Acad::ErrorStatus getVectL(AcGeVector3d& vectL);
	Acad::ErrorStatus getVectV(AcGeVector3d& vectV);

	/*!
	 * 获取顶面矩形中心点
	 * \return AcGePoint3d
	 */
	AcGePoint3d getUpOrignPoint() const;

	/*!
	 * 获取顶面中心点在底面上的投影沿长（length）方向偏移底面中心的距离
	 * \return double
	 */
	double getOffsetAlongLength() const;

	/*!
	 * 获取顶面中心点在底面上的投影沿宽（width）方向偏移底面中心的距离
	 * \return double
	 */
	double getOffsetAlongWidth() const;


	Acad::ErrorStatus setParameters(AcGePoint3d ptorign,double length1,double length2,
	                                double width1,double width2,double height,
									double lean,AcGeVector3d vectH,AcGeVector3d vectL,
									AcGeVector3d vectV);
	double GetMaxLength();

	//  [9/5/2007 suzhiyong]
	virtual Acad::ErrorStatus explodeTo3DSolid(AcDb3dSolid* &p3dSolid) const;

	//当矩形断截面底部位于XY平面，方向朝Z正向时各点的坐标求
	Acad::ErrorStatus getVertices_XY(AcGePoint3dArray& vertexArray) const;

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
MAKE_ACDBOPENOBJECT_FUNCTION(PDSqucone);

#endif // !defined(AFX_PDSQUCONE_H__58A03BC2_F749_425A_82C0_747524917720__INCLUDED_)
