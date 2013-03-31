//==============================================================================
// Viewer plugin
//==============================================================================

#include "viewerplugin.h"

//==============================================================================

namespace OpenCOR {
namespace Viewer {

//==============================================================================

PLUGININFO_FUNC ViewerPluginInfo()
{
    Descriptions descriptions;

    descriptions.insert("en", QString::fromUtf8("A plugin to graphically visualise various modelling concepts (e.g. mathematical equations)"));
    descriptions.insert("fr", QString::fromUtf8("Une extension pour visualiser graphiquement différents concepts de modélisation (par exemple des équations mathématiques)"));

    return new PluginInfo(PluginInfo::InterfaceVersion001,
                          PluginInfo::Gui,
                          PluginInfo::Miscellaneous,
                          false,
                          QStringList() << "CoreEditing" << "QtMmlWidget",
                          descriptions);
}

//==============================================================================

}   // namespace Viewer
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
