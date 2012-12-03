//==============================================================================
// QtPropertyBrowserWidget class
//==============================================================================

#ifndef QTPROPERTYBROWSERWIDGET_H
#define QTPROPERTYBROWSERWIDGET_H

//==============================================================================

#include "qtpropertybrowsersupportglobal.h"

//==============================================================================

#include <QtTreePropertyBrowser>

//==============================================================================

class QtVariantProperty;
class QtVariantPropertyManager;

//==============================================================================

namespace OpenCOR {
namespace QtPropertyBrowserSupport {

//==============================================================================

class QTPROPERTYBROWSERSUPPORT_EXPORT QtPropertyBrowserWidget : public QtTreePropertyBrowser
{
public:
    explicit QtPropertyBrowserWidget(QWidget *pParent = 0);

    QtVariantProperty * addProperty(const int pType, const QString &pName);

private:
    QtVariantPropertyManager *mPropertyManager;
};

//==============================================================================

}   // namespace QtPropertyBrowserSupport
}   // namespace OpenCOR

//==============================================================================

#endif

//==============================================================================
// End of file
//==============================================================================