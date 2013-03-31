//==============================================================================
// Help widget
//==============================================================================

#ifndef HELPWIDGET_H
#define HELPWIDGET_H

//==============================================================================

#include "commonwidget.h"

//==============================================================================

#include <QNetworkReply>
#include <QWebView>

//==============================================================================

class QHelpEngine;

//==============================================================================

namespace OpenCOR {
namespace Help {

//==============================================================================

class HelpWidget;

//==============================================================================

class HelpNetworkReply : public QNetworkReply
{
public:
    explicit HelpNetworkReply(const QNetworkRequest &pRequest,
                              const QByteArray &pData,
                              const QString &pMimeType);

    virtual void abort();
    virtual qint64 bytesAvailable() const;

protected:
    virtual qint64 readData(char *pData, qint64 pMaxlen);

private:
    QByteArray mData;
    qint64 mOrigLen;
};

//==============================================================================

class HelpNetworkAccessManager : public QNetworkAccessManager
{
    Q_OBJECT

public:
    explicit HelpNetworkAccessManager(QHelpEngine *pHelpEngine,
                                      QObject *pParent);

protected:
    virtual QNetworkReply * createRequest(Operation pOperation,
                                          const QNetworkRequest &pRequest,
                                          QIODevice *pOutgoingData = 0);

private:
    QHelpEngine *mHelpEngine;

    QString mErrorMsgTemplate;
};

//==============================================================================

class HelpPage : public QWebPage
{
public:
    explicit HelpPage(QObject *pParent);

protected:
    virtual bool acceptNavigationRequest(QWebFrame*,
                                         const QNetworkRequest &pRequest,
                                         QWebPage::NavigationType);

private:
    QMap<QString, QString> mFileNames;
};

//==============================================================================

class HelpWidget : public QWebView, public Core::CommonWidget
{
    Q_OBJECT

public:
    explicit HelpWidget(QHelpEngine *pHelpEngine, const QUrl &pHomePage,
                        QWidget *pParent = 0);

    virtual void retranslateUi();

    virtual void loadSettings(QSettings *pSettings);
    virtual void saveSettings(QSettings *pSettings) const;

    void goToHomePage();

    void resetZoom();

    void zoomIn();
    void zoomOut();

protected:
    virtual QSize sizeHint() const;

    virtual void mouseReleaseEvent(QMouseEvent *pEvent);
    virtual void wheelEvent(QWheelEvent *pEvent);
    virtual void paintEvent(QPaintEvent *pEvent);

private:
    QHelpEngine *mHelpEngine;

    QUrl mHomePage;

    int mZoomLevel;

    void emitZoomRelatedSignals();

    void setZoomLevel(const int &pZoomLevel);

Q_SIGNALS:
    void notHomePage(const bool &pNotHomePage);

    void backEnabled(const bool &pEnabled);
    void forwardEnabled(const bool &pEnabled);

    void copyTextEnabled(const bool &pEnabled);

    void notDefaultZoomLevel(const bool &pEnabled);
    void zoomOutEnabled(const bool &pEnabled);

private Q_SLOTS:
    void urlChanged(const QUrl &pUrl);

    void selectionChanged();

    void documentChanged();
};

//==============================================================================

}   // namespace Help
}   // namespace OpenCOR

//==============================================================================

#endif

//==============================================================================
// End of file
//==============================================================================
