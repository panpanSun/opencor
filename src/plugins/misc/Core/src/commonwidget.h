//==============================================================================
// Common widget
//==============================================================================

#ifndef COMMONWIDGET_H
#define COMMONWIDGET_H

//==============================================================================

#include "coreglobal.h"
#include "plugin.h"

//==============================================================================

#include <QtGlobal>

//==============================================================================

#include <QColor>
#include <QString>

//==============================================================================

class QSettings;
class QSize;

//==============================================================================

namespace OpenCOR {
namespace Core {

//==============================================================================
// Note: the default font family and size below were taken from Qt Creator...

#if defined(Q_OS_WIN)
    static const QString DefaultFontFamily = "Courier";
    enum {
        DefaultFontSize = 10
    };
#elif defined(Q_OS_LINUX)
    static const QString DefaultFontFamily = "Monospace";
    enum {
        DefaultFontSize = 9
    };
#elif defined(Q_OS_MAC)
    static const QString DefaultFontFamily = "Monaco";
    enum {
        DefaultFontSize = 12
    };
#else
    #error Unsupported platform
#endif

//==============================================================================

class CORE_EXPORT CommonWidget
{
public:
    explicit CommonWidget(QWidget *pParent = 0);

    virtual void retranslateUi();

    virtual void loadSettings(QSettings *pSettings);
    virtual void saveSettings(QSettings *pSettings) const;

    virtual void loadingOfSettingsDone(const Plugins &pLoadedPlugins);

    static QColor borderColor();

    static QColor baseColor();
    static QColor windowColor();
    static QColor highlightColor();

protected:
    QSize defaultSize(const double &pRatio) const;

    void drawBorder(const bool &pDockedTop, const bool &pDockedLeft,
                    const bool &pDockedBottom, const bool &pDockedRight,
                    const bool &pFloatingTop, const bool &pFloatingLeft,
                    const bool &pFloatingBottom, const bool &pFloatingRight);

private:
    QWidget *mParent;

    static QColor specificColor(const QString &pColor);
};

//==============================================================================

}   // namespace Core
}   // namespace OpenCOR

//==============================================================================

#endif

//==============================================================================
// End of file
//==============================================================================
