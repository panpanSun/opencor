//==============================================================================
// Collapsable widget
//==============================================================================

#ifndef COLLAPSABLEWIDGET_H
#define COLLAPSABLEWIDGET_H

//==============================================================================

#include "commonwidget.h"
#include "coreglobal.h"

//==============================================================================

#include <QWidget>

//==============================================================================

class QFrame;
class QLabel;
class QToolButton;

//==============================================================================

namespace OpenCOR {
namespace Core {

//==============================================================================

class CORE_EXPORT CollapsableWidget : public QWidget, public CommonWidget
{
    Q_OBJECT

public:
    explicit CollapsableWidget(const QString &pTitle, QWidget *pBody,
                               QWidget *pParent = 0);
    explicit CollapsableWidget(const QString &pTitle, QWidget *pParent = 0);
    explicit CollapsableWidget(QWidget *pParent = 0);

    QString title() const;
    void setTitle(const QString &pTitle);

    QWidget * body() const;
    void setBody(QWidget *pBody);

protected:
    virtual QSize sizeHint() const;

private:
    QLabel *mTitle;
    QToolButton *mButton;

    QFrame *mSeparator;

    QWidget *mBody;

    void constructor(const QString &pTitle = QString(), QWidget *pBody = 0);
};

//==============================================================================

}   // namespace Core
}   // namespace OpenCOR

//==============================================================================

#endif

//==============================================================================
// End of file
//==============================================================================
