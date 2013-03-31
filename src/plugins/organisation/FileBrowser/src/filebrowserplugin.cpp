//==============================================================================
// FileBrowser plugin
//==============================================================================

#include "filebrowserplugin.h"
#include "filebrowserwindow.h"

//==============================================================================

#include <QMainWindow>

//==============================================================================

namespace OpenCOR {
namespace FileBrowser {

//==============================================================================

PLUGININFO_FUNC FileBrowserPluginInfo()
{
    Descriptions descriptions;

    descriptions.insert("en", QString::fromUtf8("A plugin to access your local files"));
    descriptions.insert("fr", QString::fromUtf8("Une extension pour accéder à vos fichiers locaux"));

    return new PluginInfo(PluginInfo::InterfaceVersion001,
                          PluginInfo::Gui,
                          PluginInfo::Organisation,
                          true,
                          QStringList() << "Core",
                          descriptions);
}

//==============================================================================

void FileBrowserPlugin::initialize()
{
    // Create an action to show/hide our file browser window

    mFileBrowserAction = newAction(mMainWindow, true);

    // Create our file browser window

    mFileBrowserWindow = new FileBrowserWindow(mMainWindow);

    // Set our settings

    mGuiSettings->addWindow(Qt::LeftDockWidgetArea, mFileBrowserWindow,
                            GuiWindowSettings::Organisation,
                            mFileBrowserAction);
}

//==============================================================================

void FileBrowserPlugin::loadSettings(QSettings *pSettings)
{
    // Retrieve our file browser window settings

    loadWindowSettings(pSettings, mFileBrowserWindow);
}

//==============================================================================

void FileBrowserPlugin::saveSettings(QSettings *pSettings) const
{
    // Keep track of our file browser window settings

    saveWindowSettings(pSettings, mFileBrowserWindow);
}

//==============================================================================

void FileBrowserPlugin::retranslateUi()
{
    // Retranslate our file browser action

    retranslateAction(mFileBrowserAction, tr("File Browser"),
                      tr("Show/hide the File Browser window"));

    // Retranslate our file browser window

    mFileBrowserWindow->retranslateUi();
}

//==============================================================================

}   // namespace FileBrowser
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
