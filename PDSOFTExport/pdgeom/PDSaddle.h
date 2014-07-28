// PDSaddle.h: interface for the PDSaddle class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PDSADDLE_H__D9D7F3E9_BD26_4AFD_AA65_82A5590954E3__INCLUDED_)
#define AFX_PDSADDLE_H__D9D7F3E9_BD26_4AFD_AA65_82A5590954E3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PDPrimary3D.h"


class PDSaddle : public PDPrimary3D  
{
public:
	struct SideFace //!< 记录马鞍形m_ptP所在侧面上的四个顶点以及圆弧两端点和中点
	{
		AcGePoint3d originPt;  //!< 即m_ptP;
		AcGePoint3d vertexHeight;   //!< m_ptP沿高度（即Z向）到达的那个顶点
		AcGePoint3d vertexWidth;	//!< m_ptP沿宽度方向到达的顶点
		AcGePoint3d vertexRemain;   //!< 除去originPt, vertexHeight,  vertexWidth 后剩下的最后一个顶点

		AcGePoint3d nearArcPt; //!< 靠近vertexHeight顶点的圆弧端点，可能与vertexHeight是同一个点
		AcGePoint3d farArcPt;  //!< 靠近vertexRemain顶点的圆弧端点，可能与vertexRemain是同一个点
		AcGePoint3d midArcPt;  //!< 圆弧中点

		bool flag;  //!< 若 flag == true，则 nearArcPt == vertexHeight 且 farArcPt == vertexRemain；
					//!< 反之，若flag == false, 则 nearArcPt != vertexHeight 且 farArcPt != vertexRemain
	};

public:
	ACRX_DECLARE_MEMBERS(PDSaddle);

	PDSaddle(bool HasSnap = false);
  // angle为角度
	PDSaddle( double l,double w,double h,AcGePoint3d p,double angle,double radius,int precision, 
            const AcGeVector3d &insX = AcGeVector3d(1, 0, 0), const AcGeVector3d &insY = AcGeVector3d(0, 1, 0),
			bool HasSnap = false,int CorrType = 1/* 0: 左手系，1: 右手系*/);

	virtual ~PDSaddle();


private:
	double m_dLength,m_dWidth,m_dHeight,m_dRadius;
	AcGePoint3d m_ptP;
	double m_dAngle;
	Adesk::UInt32 m_nPrecision;

  //version 2
  AcGeVector3d m_insX;  //插入点的X方向
  AcGeVector3d m_insY;  //插入点的Y方向
  AcGeVector3d m_insZ;  //插入点的Z方向
  	

public:
	Acad::ErrorStatus getVertices(int Precision, AcGePoint3dArray& vertexArray, 
                                          AcGeIntArray &stdIdx, int &actPrecision) const;
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

	Acad::ErrorStatus getpointP(AcGePoint3d& ptP);
	Acad::ErrorStatus getangle(double& angle);  // angle为角度
	Acad::ErrorStatus getlength(double& length);
	Acad::ErrorStatus getheight(double& height);
	Acad::ErrorStatus getwidth(double& width);
	Acad::ErrorStatus getradius(double& radius);
	Acad::ErrorStatus getlengthVwidthV(AcGeVector3d &lengthvect,AcGeVector3d &widthvect) const;
	Acad::ErrorStatus getprecision(Adesk::UInt32& precision);
	Acad::ErrorStatus getInsDir(AcGeVector3d& insX, AcGeVector3d& insY, AcGeVector3d& insZ);

	/*!
	 * 获取马鞍形的定位点（即m_ptP）所在侧面的四个顶点及圆弧中点
	 * \return  SideFace
	 */
	SideFace getSideFaceInfo() const;



  Acad::ErrorStatus getRightHandCoord(AcGePoint3d &ori, 
                                        AcGeVector3d &vecx, 
                                        AcGeVector3d &vecy, 
                                        AcGeVector3d &vecz, 
										AcGeVector3d &vlength,
                                        AcGeVector3d &vwidth,
                                        double &length, 
                                        double &width, 
                                        double &height) const;

  	Acad::ErrorStatus setParameters(AcGePoint3d ptP,double angle,double length,
		                            double height,double width,double radius,
									const AcGeVector3d& insX, const AcGeVector3d& insY,	
									Adesk::UInt32 precision,
                                    int CorrType = 1/* 0: ｚ左手系，1: 右手系*/);

  Acad::ErrorStatus getExtrudePolygonPts(AcGePoint3dArray& vertexArray) const;
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
MAKE_ACDBOPENOBJECT_FUNCTION(PDSaddle);

#endif // !defined(AFX_PDSADDLE_H__D9D7F3E9_BD26_4AFD_AA65_82A5590954E3__INCLUDED_)
