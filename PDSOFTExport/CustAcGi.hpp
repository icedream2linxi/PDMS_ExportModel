
#ifndef REVIEW_EXPORT_GENERAL_ACGI_HPP_
#define REVIEW_EXPORT_GENERAL_ACGI_HPP_

#include <boost/smart_ptr/scoped_ptr.hpp>
#include <boost/function.hpp>
#include <tchar.h>
#include <acgi.h>


class CustAcGiWorldGeometry : public AcGiWorldGeometry
{
    ACRX_DECLARE_MEMBERS(CustAcGiWorldGeometry);

public:
    boost::function<void (Adesk::UInt32 /* nbPoints */, const AcGePoint3d* /* pVertexList */)> polygonEvent;
	boost::function<void (Adesk::UInt32 /* rows */,
                        Adesk::UInt32 /* columns */, 
                        const AcGePoint3d* /* pVertexList */, 
                        const AcGiEdgeData* /* pEdgeData */,
                        const AcGiFaceData* /* pFaceData */,
                        const AcGiVertexData* /* pVertexData */,
                        bool /* bAutoGeneratNormals */)> meshEvent;
	boost::function<void (Adesk::UInt32 /* nbVertex */,
                        const AcGePoint3d* /* pVertexList */,
                        Adesk::UInt32 /* facelistSize */,
                        const Adesk::Int32* /* pFaceList */,
                        const AcGiEdgeData* /* pEdgeData */,
                        const AcGiFaceData* /* pFaceData */,
                        const AcGiVertexData* /* pVertexData */,
                        const resbuf* /* pResbuf */,
                        bool /* bAutoGenerateNormals */)> shellEvent;

public:
    virtual void setExtents(AcGePoint3d *pNewExtents) const;

public:
    virtual void getModelToWorldTransform(AcGeMatrix3d&) const;
    virtual void getWorldToModelTransform(AcGeMatrix3d&) const;

    virtual Adesk::Boolean pushModelTransform(const AcGeVector3d & vNormal);
    virtual Adesk::Boolean pushModelTransform(const AcGeMatrix3d & xMat);
    virtual Adesk::Boolean popModelTransform();
	virtual AcGeMatrix3d pushPositionTransform(AcGiPositionTransformBehavior behavior, const AcGePoint3d& offset);
	virtual AcGeMatrix3d pushPositionTransform(AcGiPositionTransformBehavior behavior, const AcGePoint2d& offset);
	virtual AcGeMatrix3d pushScaleTransform(AcGiScaleTransformBehavior behavior, const AcGePoint3d& extents);
	virtual AcGeMatrix3d pushScaleTransform(AcGiScaleTransformBehavior behavior, const AcGePoint2d& extents);
	virtual AcGeMatrix3d pushOrientationTransform(AcGiOrientationTransformBehavior behavior);


    // For drawing various primitives.
    //
    virtual Adesk::Boolean  circle(const AcGePoint3d& center,
                                   const double radius,
                                   const AcGeVector3d& normal) const;

    virtual Adesk::Boolean  circle(const AcGePoint3d&, const AcGePoint3d&,
                                   const AcGePoint3d&) const;

    virtual Adesk::Boolean  circularArc(const AcGePoint3d& center,
                                        const double radius,
                                        const AcGeVector3d& normal,
                                        const AcGeVector3d& startVector,
                                        const double sweepAngle,
                                        const AcGiArcType arcType = kAcGiArcSimple) const;

    virtual Adesk::Boolean  circularArc(const AcGePoint3d& start,
                                        const AcGePoint3d& point,
                                        const AcGePoint3d& end,
                                        const AcGiArcType arcType = kAcGiArcSimple) const;
      
    virtual Adesk::Boolean  polyline(const Adesk::UInt32 nbPoints,
                                     const AcGePoint3d* pVertexList,
                                     const AcGeVector3d* pNormal = NULL,
									 Adesk::LongPtr lBaseSubEntMarker = -1) const;

	virtual Adesk::Boolean  polyline(const AcGiPolyline& polylineObj) const;

	virtual Adesk::Boolean  polyPolyline(Adesk::UInt32 nbPolylines,
		const AcGiPolyline* pPolylines) const;
      
    virtual Adesk::Boolean  polygon(const Adesk::UInt32 nbPoints, 
                                    const AcGePoint3d* pVertexList) const;

	virtual Adesk::Boolean  polyPolygon(const Adesk::UInt32 numPolygonIndices,
		const Adesk::UInt32* numPolygonPositions,
		const AcGePoint3d* polygonPositions,
		const Adesk::UInt32* numPolygonPoints,
		const AcGePoint3d* polygonPoints,
		const AcCmEntityColor* outlineColors = NULL,
		const AcGiLineType* outlineTypes = NULL,
		const AcCmEntityColor* fillColors = NULL,
		const AcCmTransparency* fillOpacities = NULL) const;

    virtual Adesk::Boolean  mesh(const Adesk::UInt32 rows,
                                 const Adesk::UInt32 columns,
                                 const AcGePoint3d* pVertexList,
                                 const AcGiEdgeData* pEdgeData = NULL,
                                 const AcGiFaceData* pFaceData = NULL,
                                 const AcGiVertexData* pVertexData = NULL,
                                 const bool bAutoGenerateNormals = true) const;

    virtual Adesk::Boolean  shell(const Adesk::UInt32 nbVertex,
                                 const AcGePoint3d* pVertexList,
                                 const Adesk::UInt32 faceListSize,
                                 const Adesk::Int32* pFaceList,
                                 const AcGiEdgeData* pEdgeData = NULL,
                                 const AcGiFaceData* pFaceData = NULL,
                                 const AcGiVertexData* pVertexData = NULL,
                                 const struct resbuf* pResBuf = NULL,
                                 const bool bAutoGenerateNormals = true) const;

#ifdef ARX04
    virtual Adesk::Boolean  mesh(const Adesk::UInt32 rows,
                                 const Adesk::UInt32 columns,
                                 const AcGePoint3d* pVertexList,
                                 const AcGiEdgeData* pEdgeData = NULL,
                                 const AcGiFaceData* pFaceData = NULL,
								 const AcGiVertexData* pVertexData = NULL) const{ return mesh( rows, columns, pVertexList, pEdgeData, pFaceData, pVertexData, true); }

    virtual Adesk::Boolean  shell(const Adesk::UInt32 nbVertex,
                                 const AcGePoint3d* pVertexList,
                                 const Adesk::UInt32 faceListSize,
                                 const Adesk::Int32* pFaceList,
                                 const AcGiEdgeData* pEdgeData = NULL,
                                 const AcGiFaceData* pFaceData = NULL,
                                 const AcGiVertexData* pVertexData = NULL,
								 const struct resbuf* pResBuf = NULL) const{ return shell( nbVertex, pVertexList, faceListSize, pFaceList, pEdgeData, pFaceData, pVertexData, pResBuf,  true);}
#endif
    virtual Adesk::Boolean  text(const AcGePoint3d& position,
                                 const AcGeVector3d& normal,
                                 const AcGeVector3d& direction,
                                 const double height,
                                 const double width,
                                 const double oblique,
                                 const TCHAR* pMsg) const;

    virtual Adesk::Boolean  text(const AcGePoint3d& position,
                                 const AcGeVector3d& normal,
                                 const AcGeVector3d& direction,
                                 const TCHAR* pMsg,
                                 const Adesk::Int32 length,
                                 const Adesk::Boolean raw,
                                 const AcGiTextStyle &pTextStyle) const;

    virtual Adesk::Boolean  xline(const AcGePoint3d&,
                                  const AcGePoint3d&) const;

    virtual Adesk::Boolean  ray(const AcGePoint3d&,
                                const AcGePoint3d&) const;

    virtual Adesk::Boolean  pline(const AcDbPolyline& lwBuf,
                                  Adesk::UInt32 fromIndex = 0,
                                  Adesk::UInt32 numSegs = 0) const;

    virtual Adesk::Boolean  draw(AcGiDrawable*) const;

	virtual Adesk::Boolean  image(const AcGiImageBGRA32& imageSource,
		const AcGePoint3d& position,
		const AcGeVector3d& u, //orientation and magnitude of width
		const AcGeVector3d& v, //orientation and magnitude of height
		TransparencyMode transparencyMode = kTransparency8Bit
		) const;

	virtual Adesk::Boolean  rowOfDots(int count,
		const AcGePoint3d&     start,
		const AcGeVector3d&     step
		) const;

	virtual Adesk::Boolean  ellipticalArc(const AcGePoint3d&         center,
		const AcGeVector3d&        normal,
		double                     majorAxisLength,
		double                     minorAxisLength,
		double                     startDegreeInRads,
		double                     endDegreeInRads,
		double                     tiltDegreeInRads,
		AcGiArcType                arcType = kAcGiArcSimple
		) const;

    // If you push a clip boundary onto the stack you must pop it;
    //
    virtual Adesk::Boolean          pushClipBoundary(AcGiClipBoundary * pBoundary);
    virtual void                    popClipBoundary();
};

class CustAcGiSubEntityTraits : public AcGiSubEntityTraits
{
    ACRX_DECLARE_MEMBERS(CustAcGiSubEntityTraits);

public:
    virtual void              setColor              (const Adesk::UInt16 color);
    virtual void              setTrueColor          (const AcCmEntityColor& color);
    virtual void              setLayer              (const AcDbObjectId layerId);
    virtual void              setLineType           (const AcDbObjectId linetypeId);
    virtual void              setSelectionMarker    (const Adesk::LongPtr markerId);
    virtual void              setFillType           (const AcGiFillType filltype);
    virtual void              setLineWeight         (const AcDb::LineWeight lw);
    virtual void              setLineTypeScale      (double dScale = 1.0);
    virtual void              setThickness          (double dThickness);

    // Return current settings.
    //
    virtual Adesk::UInt16           color               () const;
    virtual AcCmEntityColor         trueColor           () const;
    virtual AcDbObjectId            layerId             () const;
    virtual AcDbObjectId            lineTypeId          () const;
    virtual AcGiFillType            fillType            () const;
    virtual AcDb::LineWeight        lineWeight          () const;
    virtual double                  lineTypeScale       () const;
    virtual double                  thickness           () const;

private:
    Adesk::UInt16 color_;
    AcCmEntityColor trueColor_;
    AcDbObjectId layerId_;
    AcDbObjectId linetypeId_;
	Adesk::LongPtr markerId_;
    AcGiFillType filltype_;
    AcDb::LineWeight lw_;
    double scale_;
    double thickness_;
};

class CustAcGiContext : public AcGiContext
{
    ACRX_DECLARE_MEMBERS(CustAcGiContext);

public:
    virtual Adesk::Boolean isPsOut() const;
    virtual Adesk::Boolean isPlotGeneration() const;
    virtual AcDbDatabase* database() const;
    virtual bool isBoundaryClipping() const;
};

class CustAcGiWorldDraw : public AcGiWorldDraw
{
    ACRX_DECLARE_MEMBERS(CustAcGiWorldDraw);

public:
    CustAcGiWorldGeometry& geom() { return *pGeometry_; }

public:
    CustAcGiWorldDraw();

public:
    virtual AcGiWorldGeometry& geometry() const;

    virtual AcGiRegenType regenType() const;
    virtual Adesk::Boolean regenAbort() const;
    virtual AcGiSubEntityTraits& subEntityTraits() const;
    virtual AcGiGeometry* rawGeometry() const;
    virtual Adesk::Boolean isDragging() const;

    // This function operates against the current active viewport
    //
    virtual double deviation(const AcGiDeviationType, const AcGePoint3d&) const;
    virtual void setDeviation(const double deviation);
    virtual Adesk::UInt32 numberOfIsolines() const;

    virtual AcGiContext* context();

private:
    boost::scoped_ptr<CustAcGiWorldGeometry> pGeometry_;
    boost::scoped_ptr<CustAcGiSubEntityTraits> pTraits_;
    boost::scoped_ptr<CustAcGiContext> pContext_;
    double deviation_;
};

#endif // REVIEW_EXPORT_GENERAL_ACGI_HPP_
