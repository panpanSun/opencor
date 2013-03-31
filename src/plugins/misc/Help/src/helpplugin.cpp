//==============================================================================
// Help plugin
//==============================================================================

#include "helpplugin.h"
#include "helpwindow.h"

//==============================================================================

#include <QAction>
#include <QMainWindow>

//==============================================================================

namespace OpenCOR {
namespace Help {

//==============================================================================

PLUGININFO_FUNC HelpPluginInfo()
{
    Descriptions descriptions;

    descriptions.insert("en", QString::fromUtf8("A plugin to provide help"));
    descriptions.insert("fr", QString::fromUtf8("Une extension pour fournir de l'aide"));

    return new PluginInfo(PluginInfo::InterfaceVersion001,
                          PluginInfo::Gui,
                          PluginInfo::Miscellaneous,
                          true,
                          QStringList() << "Core",
                          descriptions);
}

//==============================================================================

void HelpPlugin::initialize()
{
    // Create an action to show/hide our help window

    mHelpAction = newAction(mMainWindow, true,
                            ":/oxygen/apps/help-browser.png",
                            Qt::Key_F1);

    // Create our help window

    mHelpWindow = new HelpWindow(mMainWindow);

    // Set our settings

    mGuiSettings->addWindow(Qt::RightDockWidgetArea, mHelpWindow,
                            GuiWindowSettings::Help, mHelpAction);
}

//==============================================================================

void HelpPlugin::loadSettings(QSettings *pSettings)
{
    // Retrieve our help window settings

    loadWindowSettings(pSettings, mHelpWindow);
}

//==============================================================================

void HelpPlugin::saveSettings(QSettings *pSettings) const
{
    // Keep track of our help window settings

    saveWindowSettings(pSettings, mHelpWindow);
}

//==============================================================================

void HelpPlugin::retranslateUi()
{
    // Retranslate our help action

    retranslateAction(mHelpAction, tr("&Help"),
                      tr("Show/hide the OpenCOR help"));

    // Retranslate our help window

    mHelpWindow->retranslateUi();
}

//==============================================================================

}   // namespace Help
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
