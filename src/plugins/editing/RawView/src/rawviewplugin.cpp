//==============================================================================
// RawView plugin
//==============================================================================

#include "rawviewplugin.h"
#include "rawviewwidget.h"

//==============================================================================

#include <QMainWindow>

//==============================================================================

namespace OpenCOR {
namespace RawView {

//==============================================================================

PLUGININFO_FUNC RawViewPluginInfo()
{
    Descriptions descriptions;

    descriptions.insert("en", QString::fromUtf8("A plugin to edit files using a text editor"));
    descriptions.insert("fr", QString::fromUtf8("Une extension pour éditer des fichiers à l'aide d'un éditeur de texte"));

    return new PluginInfo(PluginInfo::InterfaceVersion001,
                          PluginInfo::Gui,
                          PluginInfo::Editing,
                          true,
                          QStringList() << "CoreEditing" << "QScintillaSupport",
                          descriptions);
}

//==============================================================================

RawViewPlugin::RawViewPlugin() :
    mViewWidgets(QMap<QString, RawViewWidget *>())
{
    // Set our settings

    mGuiSettings->setView(GuiViewSettings::Editing, QStringList());
}

//==============================================================================

RawViewPlugin::~RawViewPlugin()
{
    // Delete our view widgets

    foreach (QWidget *viewWidget, mViewWidgets)
        delete viewWidget;
}

//==============================================================================

QWidget * RawViewPlugin::viewWidget(const QString &pFileName)
{
    // Retrieve from our list the view widget associated with the file name

    RawViewWidget *res = mViewWidgets.value(pFileName);

    // Create a new view widget, if none could be retrieved

    if (!res) {
        res = new RawViewWidget(pFileName, mMainWindow);

        // Keep track of our new view widget

        mViewWidgets.insert(pFileName, res);
    }

    // Return our view widget

    return res;
}

//==============================================================================

QWidget * RawViewPlugin::removeViewWidget(const QString &pFileName)
{
    // Remove the view widget from our list, should there be one for the given
    // file name

    RawViewWidget *res = mViewWidgets.value(pFileName);

    if (res) {
        // There is a view widget for the given file name, so delete it and
        // remove it from our list

        delete res;

        mViewWidgets.remove(pFileName);
    }

    return res;
}

//==============================================================================

QString RawViewPlugin::viewName() const
{
    // Return our raw view's name

    return tr("Raw");
}

//==============================================================================

}   // namespace RawView
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
