//==============================================================================
// Central widget
//==============================================================================

#ifndef CENTRALWIDGET_H
#define CENTRALWIDGET_H

//==============================================================================

#include "fileinterface.h"
#include "guiinterface.h"
#include "widget.h"

//==============================================================================

#include <QDir>
#include <QMap>
#include <QTabBar>

//==============================================================================

namespace Ui {
    class CentralWidget;
}

//==============================================================================

class QLabel;
class QStackedWidget;

//==============================================================================

namespace OpenCOR {

//==============================================================================

class Plugin;

//==============================================================================

namespace Core {

//==============================================================================

typedef QMap<int, Plugin *> CentralWidgetViewPlugins;
typedef QMap<int, GuiViewSettings *> CentralWidgetViewSettings;

//==============================================================================

class UserMessageWidget;

//==============================================================================

class CentralWidgetMode
{
public:
    explicit CentralWidgetMode(CentralWidget *pOwner);
    ~CentralWidgetMode();

    bool isEnabled() const;
    void setEnabled(const bool &pEnabled);

    QTabBar * views() const;

    CentralWidgetViewPlugins * viewPlugins() const;
    CentralWidgetViewSettings * viewSettings() const;

private:
    bool mEnabled;

    QTabBar *mViews;

    CentralWidgetViewPlugins *mViewPlugins;
    CentralWidgetViewSettings *mViewSettings;
};

//==============================================================================

class CentralWidget : public Widget
{
    Q_OBJECT

public:
    explicit CentralWidget(QMainWindow *pMainWindow);
    ~CentralWidget();

    virtual void retranslateUi();

    virtual void loadSettings(QSettings *pSettings);
    virtual void saveSettings(QSettings *pSettings) const;

    virtual void loadingOfSettingsDone(const Plugins &pLoadedPlugins);

    void setSupportedFileTypes(const FileTypes &pSupportedFileTypes);

    void addView(Plugin *pPlugin, GuiViewSettings *pSettings);

    QTabBar * newTabBar(const QTabBar::Shape &pShape,
                        const bool &pMovable = false,
                        const bool &pTabsClosable = false);

    void openFile(const QString &pFileName);

    bool canClose();

protected:
    virtual void dragEnterEvent(QDragEnterEvent *pEvent);
    virtual void dragMoveEvent(QDragMoveEvent *pEvent);
    virtual void dropEvent(QDropEvent *pEvent);

private:
    enum Status {
        Starting,
        Idling,
        UpdatingGui,
        Stopping
    };

    QMainWindow *mMainWindow;

    Ui::CentralWidget *mGui;

    Status mStatus;

    Plugins mLoadedPlugins;

    QTabBar *mModeTabs;
    QTabBar *mFileTabs;

    FileTypes mSupportedFileTypes;

    QStringList mFileNames;

    QStackedWidget *mContents;

    QWidget *mLogoView;

    UserMessageWidget *mNoViewMsg;

    QMap<GuiViewSettings::Mode, CentralWidgetMode *> mModes;

    Plugin *mPlugin;

    int modeTabIndex(const GuiViewSettings::Mode &pMode) const;

    void addMode(const GuiViewSettings::Mode &pMode);

    void loadModeSettings(QSettings *pSettings,
                          const GuiViewSettings::Mode &pCurrentMode,
                          const GuiViewSettings::Mode &pMode);
    void saveModeSettings(QSettings *pSettings,
                          const GuiViewSettings::Mode &pMode) const;

    void addModeView(Plugin *pPlugin, GuiViewSettings *pSettings,
                     const GuiViewSettings::Mode &pMode);

    void updateModeGui(const GuiViewSettings::Mode &pMode);

    QString modeViewName(const GuiViewSettings::Mode &pMode);

    void updateNoViewMsg();

    bool activateFile(const QString &pFileName);

    bool saveFile(const int &pIndex, const bool &pNeedNewFileName = false);

    bool canCloseFile(const int &pIndex);

Q_SIGNALS:
    void guiUpdated(Plugin *pViewPlugin);

    void fileOpened(const QString &pFileName);
    void fileSaved(const QString &pFileName);
    void fileRenamed(const QString &pOldFileName, const QString &pNewFileName);
    void fileClosed(const QString &pFileName);

    void canSave(const bool &pEnabled);
    void canSaveAs(const bool &pEnabled);
    void canSaveAll(const bool &pEnabled);

    void atLeastOneFile(const bool &pAtLeastOneFile);
    void atLeastTwoFiles(const bool &pAtLeastTwoFiles);

private Q_SLOTS:
    void updateGui();

    void openFile();
    void openFiles(const QStringList &pFileNames);

    void updateModifiedSettings();

    void saveFile();
    void saveFileAs();
    void saveAllFiles();

    void previousFile();
    void nextFile();

    bool closeFile(const int &pIndex = -1);
    void closeAllFiles(const bool &pForceClosing = false);

    void moveFile(const int &pFromIndex, const int &pToIndex);

    void updateFileTabIcons();
    void updateFileTabIcon(const QString &pFileName, const QIcon &pIcon);
};

//==============================================================================

}   // namespace Core
}   // namespace OpenCOR

//==============================================================================

#endif

//==============================================================================
// End of file
//==============================================================================
