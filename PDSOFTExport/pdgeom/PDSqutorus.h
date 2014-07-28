// PDSqutorus.h: interface for the PDSqutorus class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PDSQUTORUS_H__D7206200_2E72_4AE0_9B31_C4D918524FFB__INCLUDED_)
#define AFX_PDSQUTORUS_H__D7206200_2E72_4AE0_9B31_C4D918524FFB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PDPrimary3D.h"


class PDSqutorus : public PDPrimary3D  
{
public:
	ACRX_DECLARE_MEMBERS(PDSqutorus);

	PDSqutorus(bool HasSnap = false);
	PDSqutorus( AcGePoint3d p, AcGePoint3d p1, AcGePoint3d p2,
		double l1,double w1,double l2,double w2,int d,
				int n = 20, bool HasSnap = false,int CorrType = 1 /*0-左手系，1-右手系*/);
	virtual ~PDSqutorus();

private:
	double m_dLength1,m_dWidth1,m_dLength2,m_dWidth2,m_dRadius;
	Adesk::UInt32 m_nDirection,m_nN;
	AcGePoint3d m_ptP,m_ptP1,m_ptP2;
	AcGeVector3d m_v3; //另一向量用于判断左右手系
public:
	virtual void createBody();
	Acad::ErrorStatus getVertices(int Precision, AcGePoint3dArray& vertexArray, AcGeIntArray &stdIdx, int &actPrecision) const; 
	Acad::ErrorStatus getVertices(int Precision, AcGePoint3dArray& vertexArray) const; 
                                       
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
	virtual Acad::ErrorStatus explodeToSurface(AcDbVoidPtrArray& entitySet) const;


	Acad::ErrorStatus getpointP(AcGePoint3d& ptP);
	Acad::ErrorStatus getpointP1(AcGePoint3d& ptP1);
	Acad::ErrorStatus getpointP2(AcGePoint3d& ptP2);
	Acad::ErrorStatus getLength1(double& length1);
	Acad::ErrorStatus getLength2(double& length2);
	Acad::ErrorStatus getWidth1(double& width1);
	Acad::ErrorStatus getWidth2(double& width2);
	Acad::ErrorStatus getRadius(double& radius);
	Acad::ErrorStatus getDirection(Adesk::UInt32& direction);
	Acad::ErrorStatus getN(Adesk::UInt32& n);

	Acad::ErrorStatus setParameters(AcGePoint3d ptP,AcGePoint3d ptP1,AcGePoint3d ptP2,
		                            double length1,double length2,double width1,
									double width2,double radius,Adesk::UInt32 direction,
									Adesk::UInt32 n,int CorrType = 1 /*0-左手系，1-右手系*/);
   // 取得圆环的法矢
    AcGeVector3d getNormal() const;
    // 取得圆环的扫过的角度
    double getAngle() const;
 // 取得环中心到圆环起点的矢量
    AcGeVector3d getStartCenVect() const;
    // 取得环中心到圆环终点的矢量
    AcGeVector3d getEndCenVect() const;
	AcGePoint3dArray getPtOnCentAix(int Precision);

	/*!
	 * 获取终点坐标
	 * \return AcGePoint3d
	 */
	AcGePoint3d getEndPoint() const;

	/*!
	* 获取起点平面法向
	* \return  AcGeVector3d
	*/
	AcGeVector3d getStartPlaneNormal() const;

	/*!
	* 获取终点平面法向
	* \return  AcGeVector3d
	*/
	AcGeVector3d getEndPlaneNormal() const;


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
MAKE_ACDBOPENOBJECT_FUNCTION(PDSqutorus);

#endif // !defined(AFX_PDSQUTORUS_H__D7206200_2E72_4AE0_9B31_C4D918524FFB__INCLUDED_)
