//==============================================================================
// Help widget
//==============================================================================

#include "coreutils.h"
#include "helpwidget.h"

//==============================================================================

#include <QAction>
#include <QDesktopServices>
#include <QDir>
#include <QHelpEngine>
#include <QMouseEvent>
#include <QSettings>
#include <QTimer>

//==============================================================================

#include <QtSingleApplication>

//==============================================================================

namespace OpenCOR {
namespace Help {

//==============================================================================

HelpNetworkReply::HelpNetworkReply(const QNetworkRequest &pRequest,
                                   const QByteArray &pData,
                                   const QString &pMimeType) :
    mData(pData),
    mOrigLen(pData.length())
{
    // Set a few things for the network reply

    setRequest(pRequest);
    setOpenMode(QIODevice::ReadOnly);
    setHeader(QNetworkRequest::ContentTypeHeader, pMimeType);
    setHeader(QNetworkRequest::ContentLengthHeader,
              QByteArray::number(mOrigLen));

    // Let ourselves know immediately that data is available for reading

    QTimer::singleShot(0, this, SIGNAL(readyRead()));
}

//==============================================================================

void HelpNetworkReply::abort()
{
    // Do nothing on purpose...
}

//==============================================================================

qint64 HelpNetworkReply::bytesAvailable() const
{
    // Return the size of the data which is available for reading

    return mData.length()+QNetworkReply::bytesAvailable();
}

//==============================================================================

qint64 HelpNetworkReply::readData(char *pBuffer, qint64 pMaxlen)
{
    // Determine the lenght of the data to be read

    qint64 len = qMin(qint64(mData.length()), pMaxlen);

    // Read the data, should there be some to read

    if (len) {
        memcpy(pBuffer, mData.constData(), len);

        mData.remove(0, len);
    }

    // Should there be no data left to read, then let ourselves know immediately
    // that we are done

    if (!mData.length())
        QTimer::singleShot(0, this, SIGNAL(finished()));

    // Return the size of the data which was read

    return len;
}

//==============================================================================

HelpNetworkAccessManager::HelpNetworkAccessManager(QHelpEngine *pHelpEngine,
                                                   QObject *pParent) :
    QNetworkAccessManager(pParent),
    mHelpEngine(pHelpEngine)
{
    // Retrieve the error message template

    QFile helpWidgetErrorFile(":Help_helpWidgetErrorHtml");

    helpWidgetErrorFile.open(QIODevice::ReadOnly);

    mErrorMsgTemplate = QString(helpWidgetErrorFile.readAll());

    helpWidgetErrorFile.close();
}

//==============================================================================

QNetworkReply * HelpNetworkAccessManager::createRequest(Operation,
                                                        const QNetworkRequest &pRequest,
                                                        QIODevice*)
{
    // Retrieve, if possible, the requested document

    QUrl url = pRequest.url();
    QByteArray data = mHelpEngine->findFile(url).isValid()?
                          mHelpEngine->fileData(url):
                          QByteArray(mErrorMsgTemplate.arg(tr("Error"),
                                                           tr("The following help file could not be found:")+" <strong>"+url.toString()+"</strong>.",
                                                           tr("Please <a href=\"%1\">contact us</a> about this error.").arg("contactUs.html"),
                                                           Core::copyright()).toUtf8());

    // Return the requested document or an error message

    return new HelpNetworkReply(pRequest, data, "text/html");
}

//==============================================================================

HelpPage::HelpPage(QObject *pParent) :
    QWebPage(pParent)
{
}

//==============================================================================

bool HelpPage::acceptNavigationRequest(QWebFrame*,
                                       const QNetworkRequest &pRequest,
                                       QWebPage::NavigationType)
{
    // Requested URL

    QUrl url = pRequest.url();
    QString urlScheme = url.scheme();

    // Determine whether the URL refers to an OpenCOR document or an external
    // resource of sorts

    if (!urlScheme.compare("qthelp")) {
        // This an OpenCOR document, so...

        return true;
    } else if (!urlScheme.compare("gui")) {
        // This is an action which we want OpenCOR or one of its plugins to
        // execute
        // Note: a qobject_cast is not good enough on Windows and OS X, so...

        static_cast<SharedTools::QtSingleApplication *>(qApp)->handleAction(url);

        return false;
    } else {
        // This is an external resource of sorts, so we leave it to the user's
        // default web browser to open it for us

        QDesktopServices::openUrl(url);

        return false;
    }
}

//==============================================================================

enum {
    MinimumZoomLevel =  1,
    DefaultZoomLevel = 10
};

//==============================================================================

HelpWidget::HelpWidget(QHelpEngine *pHelpEngine, const QUrl &pHomePage,
                       QWidget *pParent) :
    QWebView(pParent),
    CommonWidget(pParent),
    mHelpEngine(pHelpEngine),
    mHomePage(pHomePage),
    mZoomLevel(-1)   // This will ensure that mZoomLevel gets initialised by our
                     // first call to setZoomLevel
{
    // Add a small margin to the widget, so that no visual trace of the border
    // drawn by drawBorderIfDocked is left when scrolling

    setStyleSheet("QWebView {"
                  "    margin: 1px;"
                  "}");
    // Note: not sure why, but no matter how many pixels are specified for the
    //       margin, no margin actually exists, but it addresses the issue with
    //       the border drawn by drawBorderIfDocked, so...

    // Use our own help page and help network access manager classes

    setPage(new HelpPage(this));

    page()->setNetworkAccessManager(new HelpNetworkAccessManager(pHelpEngine,
                                                                 this));

    // Prevent objects from being dropped on us
    // Note: by default, QWebView allows for objects to be dropped on itself,
    //       so...

    setAcceptDrops(false);

    // Prevent the widget from taking over the scrolling of other widgets

    setFocusPolicy(Qt::NoFocus);

    // Set our initial zoom level to the default value
    // Note: to set mZoomLevel directly is not good enough since one of the
    //       things setZoomLevel does is to set our zoom factor, so...

    setZoomLevel(DefaultZoomLevel);

    // Some connections

    connect(this, SIGNAL(urlChanged(const QUrl &)),
            this, SLOT(urlChanged(const QUrl &)));

    connect(page(), SIGNAL(selectionChanged()),
            this, SLOT(selectionChanged()));

    connect(pageAction(QWebPage::Back), SIGNAL(changed()),
            this, SLOT(documentChanged()));
    connect(pageAction(QWebPage::Forward), SIGNAL(changed()),
            this, SLOT(documentChanged()));

    // Go to the home page

    goToHomePage();
}

//==============================================================================

void HelpWidget::retranslateUi()
{
    // Retranslate the current document, but only if it is an error document
    // since a valid document is hard-coded and therefore cannot be translated
    // Note: we use setUrl() rather than reload() since the latter won't work
    //       upon starting OpenCOR with a non-system locale, so...

    if (!mHelpEngine->findFile(url()).isValid())
        setUrl(url());
}

//==============================================================================

static const QString SettingsZoomLevel = "ZoomLevel";

//==============================================================================

void HelpWidget::loadSettings(QSettings *pSettings)
{
    // Let the user know of a few default things about ourselves by emitting a
    // few signals

    emit notHomePage(false);

    emit backEnabled(false);
    emit forwardEnabled(false);

    emit copyTextEnabled(false);

    emitZoomRelatedSignals();

    // Retrieve the zoom level

    setZoomLevel(pSettings->value(SettingsZoomLevel, DefaultZoomLevel).toInt());
}

//==============================================================================

void HelpWidget::saveSettings(QSettings *pSettings) const
{
    // Keep track of the text size multiplier

    pSettings->setValue(SettingsZoomLevel, mZoomLevel);
}

//==============================================================================

void HelpWidget::goToHomePage()
{
    // Go to the home page
    // Note: we use setUrl() rather than load() since the former will ensure
    //       that url() becomes valid straightaway (which is important for
    //       retranslateUi()) and that the document gets loaded immediately...

    setUrl(mHomePage);
}

//==============================================================================

void HelpWidget::resetZoom()
{
    // Reset the zoom level

    setZoomLevel(DefaultZoomLevel);
}

//==============================================================================

void HelpWidget::zoomIn()
{
    // Zoom in the help document contents

    setZoomLevel(mZoomLevel+1);
}

//==============================================================================

void HelpWidget::zoomOut()
{
    // Zoom out the help document contents

    setZoomLevel(qMax(int(MinimumZoomLevel), mZoomLevel-1));
}

//==============================================================================

void HelpWidget::emitZoomRelatedSignals()
{
    // Let the user know whether we are not at the default zoom level and
    // whether we can still zoom out

    emit notDefaultZoomLevel(mZoomLevel != DefaultZoomLevel);
    emit zoomOutEnabled(mZoomLevel != MinimumZoomLevel);
}

//==============================================================================

void HelpWidget::setZoomLevel(const int &pZoomLevel)
{
    if (pZoomLevel == mZoomLevel)
        return;

    // Set the zoom level of the help document contents to a particular value

    mZoomLevel = pZoomLevel;

    setZoomFactor(0.1*mZoomLevel);

    // Emit a few zoom-related signals

    emitZoomRelatedSignals();
}

//==============================================================================

QSize HelpWidget::sizeHint() const
{
    // Suggest a default size for the help widget
    // Note: this is critical if we want a docked widget, with a help widget
    //       on it, to have a decent size when docked to the main window

    return defaultSize(0.2);
}

//==============================================================================

void HelpWidget::mouseReleaseEvent(QMouseEvent *pEvent)
{
    // Handle some special mouse buttons for navigating the help

    if (pEvent->button() == Qt::XButton1) {
        // Special mouse button #1 which is used to go to the previous help
        // document

        triggerPageAction(QWebPage::Back);

        // Accept the event

        pEvent->accept();
    } else if (pEvent->button() == Qt::XButton2) {
        // Special mouse button #2 which is used to go to the next help document

        triggerPageAction(QWebPage::Forward);

        // Accept the event

        pEvent->accept();
    } else {
        // Something else, so use the default handling of the event

        QWebView::mouseReleaseEvent(pEvent);
    }
}

//==============================================================================

void HelpWidget::wheelEvent(QWheelEvent *pEvent)
{
    // Handle the wheel mouse button for zooming in/out the help document
    // contents

    if (pEvent->modifiers() & Qt::ControlModifier) {
        // The user has pressed the Ctrl key (on Windows and Linux) or the Alt
        // key (on OS X)

        int delta = pEvent->delta();

        if (delta > 0)
            // We are going 'up' which means zooming in

            zoomIn();
        else if (delta < 0)
            // We are going 'down' which means zooming out

            zoomOut();

        // Accept the event

        pEvent->accept();
    } else {
        // Something else, so use the default handling of the event

        QWebView::wheelEvent(pEvent);
    }
}

//==============================================================================

void HelpWidget::paintEvent(QPaintEvent *pEvent)
{
    // Default handling of the event

    QWebView::paintEvent(pEvent);

    // Draw a border

    drawBorder(
#if defined(Q_OS_WIN) || defined(Q_OS_LINUX)
               true, true, true, true,
#elif defined(Q_OS_MAC)
               true, false, true, false,
#else
    #error Unsupported platform
#endif
               true, false, false, false
              );
}

//==============================================================================

void HelpWidget::urlChanged(const QUrl &pUrl)
{
    // The URL has changed, so let the user know whether it's the home page

    emit notHomePage(pUrl != mHomePage);
}

//==============================================================================

void HelpWidget::selectionChanged()
{
    // The text selection has changed, so let the user know whether some text is
    // now selected

    emit copyTextEnabled(!selectedText().isEmpty());
}

//==============================================================================

void HelpWidget::documentChanged()
{
    // A new help document has been selected, resulting in the previous or next
    // help document becoming either available or not

    QAction *action = qobject_cast<QAction *>(sender());

    if (action) {
        // The QObject casting was successful, so we can carry on

        if (action == pageAction(QWebPage::Back)) {
            // The current action is to tell us whether the previous help
            // document is available, so send a signal to let the user know
            // about it

            emit backEnabled(action->isEnabled());
        } else if (action == pageAction(QWebPage::Forward)) {
            // The current action is to tell us whether the next help document
            // is available, so send a signal to let the user know about it

            emit forwardEnabled(action->isEnabled());
        }
    }
}

//==============================================================================

}   // namespace Help
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
