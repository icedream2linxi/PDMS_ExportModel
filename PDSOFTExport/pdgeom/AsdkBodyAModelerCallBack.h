#include <acgi.h>

#ifdef _OBJECTARX14_
#define	bool	int
#define	true	1
#define	false	0
#endif
#include <amodeler.h>
using namespace AModeler;

class AsdkBodyAModelerCallBack : public OutputTriangleCallback
{
public:
    AsdkBodyAModelerCallBack(AcGiWorldDraw* pCurWorldDraw) {m_pCurWorldDraw = pCurWorldDraw;}
    virtual ~AsdkBodyAModelerCallBack(){;}

    // 
    // Body::triangulate() callback function
    // 
    void outputTriangle(Edge* edges[], int numSides);
    void outputTriStrip(Edge* edgeArray[], int arrayLength,
                        bool firstTriangleIsCcw);

private:
    AcGiWorldDraw* m_pCurWorldDraw;
};

void drawAllEdges(const Body& b, AcGiWorldDraw *pWorldDraw);
