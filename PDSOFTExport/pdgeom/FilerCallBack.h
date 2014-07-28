#include <dbfiler.h>

#ifdef _OBJECTARX14_
#define	bool	int
#define	true	1
#define	false	0
#endif
#include <amodeler.h>
using namespace AModeler;

class DwgFilerCallBack : public SaveRestoreCallback
{
public:
    DwgFilerCallBack (AcDbDwgFiler*);

    virtual void saveBytes   (const void* buffer, int requiredLength);
    virtual void restoreBytes(void*       buffer, int requiredLength);

private:
    AcDbDwgFiler *m_filer;
};
