#include "stdafx.h"
#include "CustAcGi.hpp"

#include <dbapserv.h>

//#define PRINT_FUNC() acutPrintf(_T("\n")_T(__FUNCTION__))
#define PRINT_FUNC()    (0)


// CustAcGiWorldGeometry begin {

ACRX_NO_CONS_DEFINE_MEMBERS(CustAcGiWorldGeometry, AcGiWorldGeometry)

void CustAcGiWorldGeometry::setExtents(AcGePoint3d *pNewExtents) const { PRINT_FUNC(); }

void CustAcGiWorldGeometry::getModelToWorldTransform(AcGeMatrix3d&) const { PRINT_FUNC(); }
void CustAcGiWorldGeometry::getWorldToModelTransform(AcGeMatrix3d&) const { PRINT_FUNC(); }

Adesk::Boolean CustAcGiWorldGeometry::pushModelTransform(const AcGeVector3d & vNormal) { PRINT_FUNC(); return Adesk::kFalse; }
Adesk::Boolean CustAcGiWorldGeometry::pushModelTransform(const AcGeMatrix3d & xMat) { PRINT_FUNC(); return Adesk::kFalse; }
Adesk::Boolean CustAcGiWorldGeometry::popModelTransform() { PRINT_FUNC(); return Adesk::kFalse; }
AcGeMatrix3d CustAcGiWorldGeometry::pushPositionTransform(AcGiPositionTransformBehavior behavior, const AcGePoint3d& offset)
{ PRINT_FUNC(); return AcGeMatrix3d(); }
AcGeMatrix3d CustAcGiWorldGeometry::pushPositionTransform(AcGiPositionTransformBehavior behavior, const AcGePoint2d& offset)
{ PRINT_FUNC(); return AcGeMatrix3d(); }
AcGeMatrix3d CustAcGiWorldGeometry::pushScaleTransform(AcGiScaleTransformBehavior behavior, const AcGePoint3d& extents)
{ PRINT_FUNC(); return AcGeMatrix3d(); }
AcGeMatrix3d CustAcGiWorldGeometry::pushScaleTransform(AcGiScaleTransformBehavior behavior, const AcGePoint2d& extents)
{ PRINT_FUNC(); return AcGeMatrix3d(); }
AcGeMatrix3d CustAcGiWorldGeometry::pushOrientationTransform(AcGiOrientationTransformBehavior behavior)
{ PRINT_FUNC(); return AcGeMatrix3d(); }


// For drawing various primitives.
//
Adesk::Boolean  CustAcGiWorldGeometry::circle(const AcGePoint3d& center,
                                              const double radius,
                                              const AcGeVector3d& normal) const { PRINT_FUNC(); return Adesk::kFalse; }

Adesk::Boolean  CustAcGiWorldGeometry::circle(const AcGePoint3d&, const AcGePoint3d&,
                                              const AcGePoint3d&) const { PRINT_FUNC(); return Adesk::kFalse; }

Adesk::Boolean  CustAcGiWorldGeometry::circularArc(const AcGePoint3d& center,
                                                   const double radius,
                                                   const AcGeVector3d& normal,
                                                   const AcGeVector3d& startVector,
                                                   const double sweepAngle,
                                                   const AcGiArcType arcType) const { PRINT_FUNC(); return Adesk::kFalse; }

Adesk::Boolean  CustAcGiWorldGeometry::circularArc(const AcGePoint3d& start,
                                                   const AcGePoint3d& point,
                                                   const AcGePoint3d& end,
                                                   const AcGiArcType arcType) const { PRINT_FUNC(); return Adesk::kFalse; }

Adesk::Boolean  CustAcGiWorldGeometry::polyline(const Adesk::UInt32 nbPoints,
                                                const AcGePoint3d* pVertexList,
                                                const AcGeVector3d* pNormal,
                                                Adesk::LongPtr lBaseSubEntMarker) const { PRINT_FUNC(); return Adesk::kFalse; }

Adesk::Boolean  CustAcGiWorldGeometry::polyline(const AcGiPolyline& polylineObj) const { PRINT_FUNC(); return Adesk::kFalse; }

Adesk::Boolean  CustAcGiWorldGeometry::polyPolyline(Adesk::UInt32 nbPolylines,
	const AcGiPolyline* pPolylines) const { PRINT_FUNC(); return Adesk::kFalse; }

Adesk::Boolean  CustAcGiWorldGeometry::polygon(const Adesk::UInt32 nbPoints, 
                                               const AcGePoint3d* pVertexList) const
{
    PRINT_FUNC();
    polygonEvent(nbPoints, pVertexList);
    return Adesk::kFalse;
}

Adesk::Boolean  CustAcGiWorldGeometry::polyPolygon(const Adesk::UInt32 numPolygonIndices,
	const Adesk::UInt32* numPolygonPositions,
	const AcGePoint3d* polygonPositions,
	const Adesk::UInt32* numPolygonPoints,
	const AcGePoint3d* polygonPoints,
	const AcCmEntityColor* outlineColors,
	const AcGiLineType* outlineTypes,
	const AcCmEntityColor* fillColors,
	const AcCmTransparency* fillOpacities) const
{
	PRINT_FUNC();
	return Adesk::kFalse;
}

Adesk::Boolean  CustAcGiWorldGeometry::mesh(const Adesk::UInt32 rows,
                                            const Adesk::UInt32 columns,
                                            const AcGePoint3d* pVertexList,
                                            const AcGiEdgeData* pEdgeData,
                                            const AcGiFaceData* pFaceData,
                                            const AcGiVertexData* pVertexData,
                                            const bool bAutoGenerateNormals) const
{
    PRINT_FUNC();
    meshEvent(rows, columns, pVertexList, pEdgeData, pFaceData, pVertexData, bAutoGenerateNormals);
    return Adesk::kFalse;
}

Adesk::Boolean  CustAcGiWorldGeometry::shell(const Adesk::UInt32 nbVertex,
                                             const AcGePoint3d* pVertexList,
                                             const Adesk::UInt32 faceListSize,
                                             const Adesk::Int32* pFaceList,
                                             const AcGiEdgeData* pEdgeData,
                                             const AcGiFaceData* pFaceData,
                                             const AcGiVertexData* pVertexData,
                                             const struct resbuf* pResBuf,
                                             const bool bAutoGenerateNormals) const
{
    PRINT_FUNC();
    shellEvent(nbVertex, pVertexList, faceListSize, pFaceList, pEdgeData, pFaceData, pVertexData, pResBuf, bAutoGenerateNormals);
    return Adesk::kFalse;
}

Adesk::Boolean  CustAcGiWorldGeometry::text(const AcGePoint3d& position,
                                            const AcGeVector3d& normal,
                                            const AcGeVector3d& direction,
                                            const double height,
                                            const double width,
                                            const double oblique,
                                            const TCHAR* pMsg) const { PRINT_FUNC(); return Adesk::kFalse; }

Adesk::Boolean  CustAcGiWorldGeometry::text(const AcGePoint3d& position,
                                            const AcGeVector3d& normal,
                                            const AcGeVector3d& direction,
                                            const TCHAR* pMsg,
                                            const Adesk::Int32 length,
                                            const Adesk::Boolean raw,
                                            const AcGiTextStyle &pTextStyle
                                            ) const { PRINT_FUNC(); return Adesk::kFalse; }

Adesk::Boolean  CustAcGiWorldGeometry::xline(const AcGePoint3d&,
                                             const AcGePoint3d&) const { PRINT_FUNC(); return Adesk::kFalse; }

Adesk::Boolean  CustAcGiWorldGeometry::ray(const AcGePoint3d&,
                                           const AcGePoint3d&) const { PRINT_FUNC(); return Adesk::kFalse; }

Adesk::Boolean  CustAcGiWorldGeometry::pline(const AcDbPolyline& lwBuf,
                                             Adesk::UInt32 fromIndex,
                                             Adesk::UInt32 numSegs) const { PRINT_FUNC(); return Adesk::kFalse; }

Adesk::Boolean  CustAcGiWorldGeometry::draw(AcGiDrawable*) const { PRINT_FUNC(); return Adesk::kFalse; }

Adesk::Boolean CustAcGiWorldGeometry::image(const AcGiImageBGRA32& imageSource,
	const AcGePoint3d& position,
	const AcGeVector3d& u, //orientation and magnitude of width
	const AcGeVector3d& v, //orientation and magnitude of height
	TransparencyMode transparencyMode
	) const
{
	PRINT_FUNC();
	return Adesk::kFalse;
}

Adesk::Boolean  CustAcGiWorldGeometry::rowOfDots(int count,
	const AcGePoint3d&     start,
	const AcGeVector3d&     step
	) const
{
	PRINT_FUNC();
	return Adesk::kFalse;
}

Adesk::Boolean  CustAcGiWorldGeometry::ellipticalArc(const AcGePoint3d&         center,
	const AcGeVector3d&        normal,
	double                     majorAxisLength,
	double                     minorAxisLength,
	double                     startDegreeInRads,
	double                     endDegreeInRads,
	double                     tiltDegreeInRads,
	AcGiArcType                arcType
	) const
{
	PRINT_FUNC();
	return Adesk::kFalse;
}

// If you push a clip boundary onto the stack you must pop it;
//
Adesk::Boolean          CustAcGiWorldGeometry::pushClipBoundary(AcGiClipBoundary * pBoundary) { PRINT_FUNC(); return Adesk::kFalse; }
void                    CustAcGiWorldGeometry::popClipBoundary() { PRINT_FUNC(); }

// } CustAcGiWorldGeometry end

// CustAcGiSubEntityTraits begin {

ACRX_NO_CONS_DEFINE_MEMBERS(CustAcGiSubEntityTraits, AcGiSubEntityTraits)

void              CustAcGiSubEntityTraits::setColor              (const Adesk::UInt16 color) { PRINT_FUNC(); color_ = color; }
void              CustAcGiSubEntityTraits::setTrueColor          (const AcCmEntityColor& color) { PRINT_FUNC(); trueColor_ = color; }
void              CustAcGiSubEntityTraits::setLayer              (const AcDbObjectId layerId) { PRINT_FUNC(); layerId_ = layerId; }
void              CustAcGiSubEntityTraits::setLineType           (const AcDbObjectId linetypeId) { PRINT_FUNC(); linetypeId_ = linetypeId; }
void              CustAcGiSubEntityTraits::setSelectionMarker    (const Adesk::LongPtr markerId) { PRINT_FUNC(); markerId_ = markerId; }
void              CustAcGiSubEntityTraits::setFillType           (const AcGiFillType filltype) { PRINT_FUNC(); filltype_ = filltype; }
void              CustAcGiSubEntityTraits::setLineWeight         (const AcDb::LineWeight lw) { PRINT_FUNC(); lw_ = lw; }
void              CustAcGiSubEntityTraits::setLineTypeScale      (double dScale) { PRINT_FUNC(); scale_ = dScale; }
void              CustAcGiSubEntityTraits::setThickness          (double dThickness) { PRINT_FUNC(); thickness_ = dThickness; }

// Return current settings.
//
Adesk::UInt16           CustAcGiSubEntityTraits::color               (void) const { PRINT_FUNC(); return color_; }
AcCmEntityColor         CustAcGiSubEntityTraits::trueColor           (void) const { PRINT_FUNC(); return trueColor_; }
AcDbObjectId            CustAcGiSubEntityTraits::layerId             (void) const { PRINT_FUNC(); return layerId_; }
AcDbObjectId            CustAcGiSubEntityTraits::lineTypeId          (void) const { PRINT_FUNC(); return linetypeId_;; }
AcGiFillType            CustAcGiSubEntityTraits::fillType            (void) const { PRINT_FUNC(); return filltype_; }
AcDb::LineWeight        CustAcGiSubEntityTraits::lineWeight          (void) const { PRINT_FUNC(); return lw_; }
double                  CustAcGiSubEntityTraits::lineTypeScale       (void) const { PRINT_FUNC(); return scale_; }
double                  CustAcGiSubEntityTraits::thickness           (void) const { PRINT_FUNC(); return thickness_; }

// } CustAcGiSubEntityTraits end

// CustAcGiContext begin {

ACRX_NO_CONS_DEFINE_MEMBERS(CustAcGiContext, AcGiContext)

Adesk::Boolean          CustAcGiContext::isPsOut() const { PRINT_FUNC(); return Adesk::kFalse; }
Adesk::Boolean          CustAcGiContext::isPlotGeneration() const { PRINT_FUNC(); return Adesk::kFalse; }
AcDbDatabase *          CustAcGiContext::database() const { PRINT_FUNC(); return acdbHostApplicationServices()->workingDatabase(); }
bool             CustAcGiContext::isBoundaryClipping() const { PRINT_FUNC(); return false; }

// } CustAcGiContext end

// CustAcGiWorldDraw begin {

ACRX_NO_CONS_DEFINE_MEMBERS(CustAcGiWorldDraw, AcGiWorldDraw)

CustAcGiWorldDraw::CustAcGiWorldDraw()
    : pGeometry_(new CustAcGiWorldGeometry),
      pTraits_(new CustAcGiSubEntityTraits), 
      pContext_(new CustAcGiContext), 
      deviation_(0.1) {}

AcGiWorldGeometry& CustAcGiWorldDraw::geometry() const { return *pGeometry_; }

AcGiRegenType CustAcGiWorldDraw::regenType() const { PRINT_FUNC(); return kAcGiHideOrShadeCommand; }
Adesk::Boolean CustAcGiWorldDraw::regenAbort() const { return Adesk::kFalse; }
AcGiSubEntityTraits& CustAcGiWorldDraw::subEntityTraits() const { return *pTraits_; }
AcGiGeometry* CustAcGiWorldDraw::rawGeometry() const { return pGeometry_.get(); }
Adesk::Boolean CustAcGiWorldDraw::isDragging() const { return Adesk::kFalse; }

double CustAcGiWorldDraw::deviation(const AcGiDeviationType, const AcGePoint3d&) const { PRINT_FUNC(); return deviation_; }
void CustAcGiWorldDraw::setDeviation(const double deviation) { PRINT_FUNC(); deviation_ = deviation; }
Adesk::UInt32 CustAcGiWorldDraw::numberOfIsolines() const { PRINT_FUNC(); return 1; }

AcGiContext* CustAcGiWorldDraw::context() { PRINT_FUNC(); return pContext_.get(); }

// } CustAcGiWorldDraw end

