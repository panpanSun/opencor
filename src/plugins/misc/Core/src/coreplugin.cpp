//==============================================================================
// Core plugin
//==============================================================================

#include "centralwidget.h"
#include "coreplugin.h"
#include "coreutils.h"
#include "fileinterface.h"
#include "organisationwidget.h"
#include "plugin.h"

//==============================================================================

#include <QAction>
#include <QApplication>
#include <QEvent>
#include <QMainWindow>
#include <QMenu>
#include <QMessageBox>
#include <QSettings>
#include <QStackedWidget>

//==============================================================================

namespace OpenCOR {
namespace Core {

//==============================================================================

PLUGININFO_FUNC CorePluginInfo()
{
    Descriptions descriptions;

    descriptions.insert("en", QString::fromUtf8("The core plugin"));
    descriptions.insert("fr", QString::fromUtf8("L'extension de base"));

    return new PluginInfo(PluginInfo::InterfaceVersion001,
                          PluginInfo::Gui,
                          PluginInfo::Miscellaneous,
                          false,
                          QStringList(),
                          descriptions);
}

//==============================================================================

void CorePlugin::initialize()
{
    // Create our central widget

    mCentralWidget = new CentralWidget(mMainWindow);

    // Create our different File actions

    mFileOpenAction = newAction(mMainWindow, false,
                                ":/oxygen/actions/document-open.png",
                                QKeySequence::Open);

    mFileSaveAction    = newAction(mMainWindow, false,
                                   ":/oxygen/actions/document-save.png",
                                   QKeySequence::Save);
    mFileSaveAsAction  = newAction(mMainWindow, false,
                                   ":/oxygen/actions/document-save-as.png",
                                   QKeySequence::SaveAs);
    mFileSaveAllAction = newAction(mMainWindow, false,
                                   ":/oxygen/actions/document-save-all.png",
                                   QKeySequence::UnknownKey);

    mFilePreviousAction = newAction(mMainWindow, false,
                                    ":/oxygen/actions/go-previous.png",
#if defined(Q_OS_WIN) || defined(Q_OS_LINUX)
    // Note: QKeySequence::PreviousChild should, on Windows and Linux,
    //       correspond to Ctrl+Shift+Tab, but somehow it corresponds to
    //       Ctrl+Shift+Backtab and though it works on Linux (but not on
    //       Windows), it doesn't look good, so...

                                    QList<QKeySequence>() << QKeySequence(Qt::CTRL|Qt::SHIFT|Qt::Key_Tab)
#elif defined(Q_OS_MAC)
                                    QKeySequence::PreviousChild
#else
    #error Unsupported platform
#endif
                                   );
    mFileNextAction     = newAction(mMainWindow, false,
                                    ":/oxygen/actions/go-next.png",
                                    QKeySequence::NextChild);

    mFileCloseAction    = newAction(mMainWindow, false,
                                    ":/oxygen/actions/document-close.png",
                                    QKeySequence::Close);
    mFileCloseAllAction = newAction(mMainWindow);

    // Create the separator before which we will insert our reopen sub-menu

    QAction *openSaveSeparator = newAction(mMainWindow);

    openSaveSeparator->setSeparator(true);

    // Create our reopen sub-menu

    mFileReopenSubMenu = newMenu(mMainWindow,
                                 ":/oxygen/actions/document-open-recent.png",
                                 false);

    mFileReopenSubMenuSeparator = newAction(mMainWindow);
    mFileClearReopenSubMenuAction = newAction(mMainWindow);

    mFileReopenSubMenuSeparator->setSeparator(true);

    mFileReopenSubMenu->addAction(mFileReopenSubMenuSeparator);
    mFileReopenSubMenu->addAction(mFileClearReopenSubMenuAction);

    // Some connections to handle our various actions

    connect(mFileOpenAction, SIGNAL(triggered(bool)),
            mCentralWidget, SLOT(openFile()));

    connect(mFileSaveAction, SIGNAL(triggered(bool)),
            mCentralWidget, SLOT(saveFile()));
    connect(mFileSaveAsAction, SIGNAL(triggered(bool)),
            mCentralWidget, SLOT(saveFileAs()));
    connect(mFileSaveAllAction, SIGNAL(triggered(bool)),
            mCentralWidget, SLOT(saveAllFiles()));

    connect(mFilePreviousAction, SIGNAL(triggered(bool)),
            mCentralWidget, SLOT(previousFile()));
    connect(mFileNextAction, SIGNAL(triggered(bool)),
            mCentralWidget, SLOT(nextFile()));

    connect(mFileCloseAction, SIGNAL(triggered(bool)),
            mCentralWidget, SLOT(closeFile()));
    connect(mFileCloseAllAction, SIGNAL(triggered(bool)),
            mCentralWidget, SLOT(closeAllFiles()));

    // Some connections to update the enabled state of our various actions

    connect(mCentralWidget, SIGNAL(canSave(const bool &)),
            mFileSaveAction, SLOT(setEnabled(bool)));
    connect(mCentralWidget, SIGNAL(canSaveAs(const bool &)),
            mFileSaveAsAction, SLOT(setEnabled(bool)));
    connect(mCentralWidget, SIGNAL(canSaveAll(const bool &)),
            mFileSaveAllAction, SLOT(setEnabled(bool)));

    connect(mCentralWidget, SIGNAL(atLeastTwoFiles(const bool &)),
            mFilePreviousAction, SLOT(setEnabled(bool)));
    connect(mCentralWidget, SIGNAL(atLeastTwoFiles(const bool &)),
            mFileNextAction, SLOT(setEnabled(bool)));

    connect(mCentralWidget, SIGNAL(atLeastOneFile(const bool &)),
            mFileCloseAction, SLOT(setEnabled(bool)));
    connect(mCentralWidget, SIGNAL(atLeastOneFile(const bool &)),
            mFileCloseAllAction, SLOT(setEnabled(bool)));

    // Some connections related to our reopen sub-menu

    connect(mCentralWidget, SIGNAL(fileOpened(const QString &)),
            this, SLOT(fileOpened(const QString &)));
    connect(mCentralWidget, SIGNAL(fileRenamed(const QString &, const QString &)),
            this, SLOT(fileRenamed(const QString &, const QString &)));
    connect(mCentralWidget, SIGNAL(fileClosed(const QString &)),
            this, SLOT(fileClosed(const QString &)));

    connect(mFileClearReopenSubMenuAction, SIGNAL(triggered()),
            this, SLOT(clearReopenSubMenu()));

    // Set our settings

    mGuiSettings->addMenuAction(GuiMenuActionSettings::File, mFileOpenAction);
    mGuiSettings->addMenuAction(GuiMenuActionSettings::File, openSaveSeparator);
    mGuiSettings->addMenuAction(GuiMenuActionSettings::File, mFileSaveAction);
    mGuiSettings->addMenuAction(GuiMenuActionSettings::File, mFileSaveAsAction);
    mGuiSettings->addMenuAction(GuiMenuActionSettings::File, mFileSaveAllAction);
    mGuiSettings->addMenuAction(GuiMenuActionSettings::File);
    mGuiSettings->addMenuAction(GuiMenuActionSettings::File, mFilePreviousAction);
    mGuiSettings->addMenuAction(GuiMenuActionSettings::File, mFileNextAction);
    mGuiSettings->addMenuAction(GuiMenuActionSettings::File);
    mGuiSettings->addMenuAction(GuiMenuActionSettings::File, mFileCloseAction);
    mGuiSettings->addMenuAction(GuiMenuActionSettings::File, mFileCloseAllAction);
    mGuiSettings->addMenuAction(GuiMenuActionSettings::File);

    mGuiSettings->addMenu(GuiMenuSettings::File, openSaveSeparator, mFileReopenSubMenu);

    mGuiSettings->setCentralWidget(mCentralWidget);

    // Miscellaneous

    mRecentFileNames = QStringList();

    retrieveColors();
}

//==============================================================================

void CorePlugin::initializationsDone(const Plugins &pLoadedPlugins)
{
    // Retrieve the different file types supported by our various plugins and
    // make our central widget aware of them

    FileTypes supportedFileTypes = FileTypes();

    foreach (Plugin *loadedPlugin, pLoadedPlugins) {
        FileInterface *fileInterface = qobject_cast<FileInterface *>(loadedPlugin->instance());

        if (fileInterface)
            // The plugin implements our file interface, so add the supported
            // file types, but only if they are not already in our list

            foreach (const FileType &fileType, fileInterface->fileTypes())
                if (!supportedFileTypes.contains(fileType))
                    supportedFileTypes << fileType;
    }

    mCentralWidget->setSupportedFileTypes(supportedFileTypes);

    // Check, based on the loaded plugins, which views, if any, our central
    // widget should support

    foreach (Plugin *loadedPlugin, pLoadedPlugins) {
        GuiInterface *guiInterface = qobject_cast<GuiInterface *>(loadedPlugin->instance());

        if (guiInterface && guiInterface->guiSettings()->view())
            // The plugin implements our GUI interface and it has a view, so add
            // it to our central widget

            mCentralWidget->addView(loadedPlugin,
                                    guiInterface->guiSettings()->view());
    }

    // Some connections to handle certain plugin's windows

    foreach (Plugin *loadedPlugin, pLoadedPlugins) {
        GuiInterface *guiInterface = qobject_cast<GuiInterface *>(loadedPlugin->instance());

        if (guiInterface) {
            foreach (GuiWindowSettings *windowSettings,
                     guiInterface->guiSettings()->windows())
                switch (windowSettings->type()) {
                case GuiWindowSettings::Organisation:
                    // The plugin's window is of organisation type, so we want
                    // its filesOpened signal to trigger our central widget's
                    // openFiles slot

                    connect(static_cast<OrganisationWidget *>(windowSettings->window()), SIGNAL(filesOpened(const QStringList &)),
                            mCentralWidget, SLOT(openFiles(const QStringList &)));

                    break;
                default:
                    // Uninteresting type, so...

                    ;
                }
        }
    }
}

//==============================================================================

static const QString SettingsRecentFiles = "RecentFiles";

//==============================================================================

void CorePlugin::loadSettings(QSettings *pSettings)
{
    // Retrieve the recent files
    // Note: it's import to retrieve the recent files before retrieving our
    //       central widget settings since mRecentFileNames gets updated as a
    //       result of opening/closing a file...

    mRecentFileNames = pSettings->value(SettingsRecentFiles).toStringList();

    updateFileReopenMenu();

    // Retrieve the central widget settings

    pSettings->beginGroup(mCentralWidget->objectName());
        mCentralWidget->loadSettings(pSettings);
    pSettings->endGroup();
}

//==============================================================================

void CorePlugin::saveSettings(QSettings *pSettings) const
{
    // Keep track of the recent files

    pSettings->setValue(SettingsRecentFiles, mRecentFileNames);

    // Keep track of the central widget settings

    pSettings->beginGroup(mCentralWidget->objectName());
        mCentralWidget->saveSettings(pSettings);
    pSettings->endGroup();
}

//==============================================================================

void CorePlugin::loadingOfSettingsDone(const Plugins &pLoadedPlugins)
{
    // Let our central widget know that all the other plugins have now loaded
    // their settings

    mCentralWidget->loadingOfSettingsDone(pLoadedPlugins);
}

//==============================================================================

void CorePlugin::handleArguments(const QStringList &pArguments)
{
    // Check the arguments and if any of them is an existing file, then open it

    foreach (const QString &argument, pArguments)
        if (QFileInfo(argument).exists())
            // The argument corresponds to the name of an existing file, so open
            // it

            mCentralWidget->openFile(argument);
}

//==============================================================================

void CorePlugin::retrieveBorderColor()
{
    // Retrieve the colour used for a 'normal' border
    // Note #1: we use a QStackedWidget object and retrieve the colour of the
    //          pixel which is in the middle of the right border...
    // Note #2: we don't rely on the top border because it may be rendered in a
    //          special way. Also, we don't rely on a corner as such in case
    //          it's rendered as a rounded corner...
    // Note #3: on OS X, our widget must be shown otherwise, the retrieved
    //          border colour will be black. So we show it off screen, even on
    //          Windows and Linux, in case their behaviour was to change in the
    //          future...

    // Create our widget and show it off screen

    QStackedWidget stackedWidget;

    stackedWidget.setFrameShape(QFrame::StyledPanel);

    stackedWidget.move(-2*stackedWidget.width(), -2*stackedWidget.height());
    stackedWidget.show();

    // Render the widget to an image

    QImage image = QImage(stackedWidget.size(),
                          QImage::Format_ARGB32_Premultiplied);

    stackedWidget.render(&image);

    // Retrieve the colour we are after

    QColor borderColor = QColor(image.pixel(image.width()-1, 0.5*image.height()));

    // Use our settings to keep track of the colour

    QSettings settings(SettingsOrganization, SettingsApplication);

    settings.beginGroup(SettingsGlobal);
        settings.setValue(SettingsBorderColor, borderColor);
    settings.endGroup();
}

//==============================================================================

void CorePlugin::retrieveColors()
{
    // Retrieve our border colour

    retrieveBorderColor();

    // Retrieve some other colours

    QSettings settings(SettingsOrganization, SettingsApplication);

    settings.beginGroup(SettingsGlobal);
        settings.setValue(SettingsBaseColor, qApp->palette().color(QPalette::Base));
        settings.setValue(SettingsWindowColor, qApp->palette().color(QPalette::Window));
        settings.setValue(SettingsHighlightColor, qApp->palette().color(QPalette::Highlight));
    settings.endGroup();
}

//==============================================================================

void CorePlugin::changeEvent(QEvent *pEvent)
{
    // Check whether the palette has changed and if so then retrieve some new
    // colours to be used

    if (pEvent->type() == QEvent::PaletteChange)
        retrieveColors();
}

//==============================================================================

void CorePlugin::retranslateUi()
{
    // Retranslate our different File actions

    retranslateAction(mFileOpenAction, tr("&Open..."), tr("Open a file"));

    retranslateAction(mFileSaveAction, tr("&Save"),
                      tr("Save the current file"));
    retranslateAction(mFileSaveAsAction, tr("Save &As..."),
                      tr("Save the current file under a different name"));
    retranslateAction(mFileSaveAllAction, tr("Save All"),
                      tr("Save all the files"));

    retranslateAction(mFilePreviousAction, tr("Previous"),
                      tr("Select the previous file"));
    retranslateAction(mFileNextAction, tr("Next"),
                      tr("Select the next file"));

    retranslateAction(mFileCloseAction, tr("Clos&e"),
                      tr("Close the current file"));
    retranslateAction(mFileCloseAllAction, tr("Close All"),
                      tr("Close all the files"));

    // Retranslate our File sub-menu and its action

    retranslateMenu(mFileReopenSubMenu, tr("Reopen"));

    retranslateAction(mFileClearReopenSubMenuAction, tr("Clear Menu"),
                      tr("Clear the menu"));

    // Retranslate our central widget

    mCentralWidget->retranslateUi();
}

//==============================================================================

bool CorePlugin::canClose()
{
    // To determine whether we can close, we must ask our central widget

    return mCentralWidget->canClose();
}

//==============================================================================

void CorePlugin::fileOpened(const QString &pFileName)
{
    // Remove the file from our list of recent files and update our reopen
    // sub-menu

    mRecentFileNames.removeOne(pFileName);

    updateFileReopenMenu();
}

//==============================================================================

void CorePlugin::fileRenamed(const QString &pOldFileName,
                             const QString &pNewFileName)
{
    Q_UNUSED(pNewFileName);

    // Remove the new file from our list of recent files, should it be there
    // Note: it's fine if the new file isn't in our list since nothing will be
    //       done in that case (thus avoiding us having to test for its
    //       presence)...

    mRecentFileNames.removeOne(pNewFileName);

    // A file has been renamed, so we want the old file name to be added to our
    // list of recent files, i.e. as if it had been closed

    fileClosed(pOldFileName);
}

//==============================================================================

void CorePlugin::fileClosed(const QString &pFileName)
{
    // Add the file to our list of recent files (making sure that we don't end
    // up with more than 10 recent file names) and update our reopen sub-menu
    // Note: the most recent file is to be shown first...

    mRecentFileNames.prepend(pFileName);

    while (mRecentFileNames.count() > 10)
        mRecentFileNames.removeLast();

    updateFileReopenMenu();
}

//==============================================================================

void CorePlugin::openRecentFile()
{
    // Check that the recent file still exists

    QString fileName = qobject_cast<QAction *>(sender())->text();

    if (!QFileInfo(fileName).exists()) {
        // The file doesn't exist anymore, so let the user know about it

        QMessageBox::warning(mMainWindow, tr("Reopen File"),
                             tr("Sorry, but <strong>%1</strong> does not exist anymore.").arg(fileName));

        // Remove the file from our list of recent files and update our reopen
        // sub-menu

        mRecentFileNames.removeOne(fileName);

        updateFileReopenMenu();

        // Leave since we couldn't open the recent file

        return;
    }

    // Open the recent file

    mCentralWidget->openFile(fileName);
}

//==============================================================================

void CorePlugin::clearReopenSubMenu()
{
    // Indirectly clear our reopen sub-menu

    mRecentFileNames.clear();

    updateFileReopenMenu();
}

//==============================================================================

void CorePlugin::updateFileReopenMenu()
{
    // Update the contents of our reopen sub-menu by first cleaning it

    foreach (QAction *action, mFileReopenSubMenu->actions()) {
        if (action != mFileReopenSubMenuSeparator)
            disconnect(action, SIGNAL(triggered()),
                       this, SLOT(openRecentFile()));
        else
            // We have reached our reopen sub-menu separator, so...

            break;

        mFileReopenSubMenu->removeAction(action);

        delete action;
    }

    // Add the recent files to our reopen sub-menu

    foreach (const QString &recentFile, mRecentFileNames) {
        QAction *action = newAction(mMainWindow);

        action->setText(recentFile);

        connect(action, SIGNAL(triggered()),
                this, SLOT(openRecentFile()));

        mFileReopenSubMenu->insertAction(mFileReopenSubMenuSeparator, action);
    }

    // Enable/disable our our reopen sub-menu depending on whether we have
    // recent file names

    mFileReopenSubMenu->setEnabled(!mRecentFileNames.isEmpty());
}

//==============================================================================

}   // namespace Core
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
