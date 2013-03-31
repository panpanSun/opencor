//==============================================================================
// SingleCellSimulationView plugin
//==============================================================================

#include "cellmlfilemanager.h"
#include "cellmlsupportplugin.h"
#include "singlecellsimulationviewplugin.h"
#include "singlecellsimulationviewwidget.h"
#include "solverinterface.h"

//==============================================================================

#include <QMainWindow>

//==============================================================================

namespace OpenCOR {
namespace SingleCellSimulationView {

//==============================================================================

PLUGININFO_FUNC SingleCellSimulationViewPluginInfo()
{
    Descriptions descriptions;

    descriptions.insert("en", QString::fromUtf8("A plugin to run single cell simulations"));
    descriptions.insert("fr", QString::fromUtf8("Une extension pour exécuter des simulations unicellulaires"));

    return new PluginInfo(PluginInfo::InterfaceVersion001,
                          PluginInfo::Gui,
                          PluginInfo::Simulation,
                          true,
                          QStringList() << "CellMLSupport" << "Qwt",
                          descriptions);
}

//==============================================================================

SingleCellSimulationViewPlugin::SingleCellSimulationViewPlugin()
{
    // Set our settings

    mGuiSettings->setView(GuiViewSettings::Simulation,
                          QStringList() << CellMLSupport::CellmlMimeType);
}

//==============================================================================

void SingleCellSimulationViewPlugin::initialize()
{
    // Create our single view widget

    mViewWidget = new SingleCellSimulationViewWidget(this, mMainWindow);

    // Hide our single view widget since it may not initially be shown in our
    // central widget

    mViewWidget->setVisible(false);
}

//==============================================================================

void SingleCellSimulationViewPlugin::initializationsDone(const Plugins &pLoadedPlugins)
{
    // Retrieve the different solvers that are available to us

    SolverInterfaces solverInterfaces = SolverInterfaces();

    foreach (Plugin *loadedPlugin, pLoadedPlugins) {
        SolverInterface *solverInterface = qobject_cast<SolverInterface *>(loadedPlugin->instance());

        if (solverInterface)
            // The plugin implements our solver interface, so...

            solverInterfaces << solverInterface;
    }

    // Initialise our view widget with the different solvers that are available
    // to us

    mViewWidget->setSolverInterfaces(solverInterfaces);
}

//==============================================================================

void SingleCellSimulationViewPlugin::loadSettings(QSettings *pSettings)
{
    // Retrieve our single cell simulation view settings

    loadViewSettings(pSettings, mViewWidget);
}

//==============================================================================

void SingleCellSimulationViewPlugin::saveSettings(QSettings *pSettings) const
{
    // Retrieve our single cell simulation view settings

    saveViewSettings(pSettings, mViewWidget);
}

//==============================================================================

QWidget * SingleCellSimulationViewPlugin::viewWidget(const QString &pFileName)
{
    // Check that we are dealing with a CellML file and, if so, return our
    // generic simulation view widget after having initialised it

    if (!CellMLSupport::CellmlFileManager::instance()->cellmlFile(pFileName))
        // We are not dealing with a CellML file, so...

        return 0;

    // We are dealing with a CellML file, so update our generic simulation view
    // widget using the given CellML file

    mViewWidget->initialize(pFileName);

    // Our generic simulation view widget is now ready, so...

    return mViewWidget;
}

//==============================================================================

QWidget * SingleCellSimulationViewPlugin::removeViewWidget(const QString &pFileName)
{
    // Ask our generic view widget to finalise the given file

    mViewWidget->finalize(pFileName);

    // We don't want to give people the address of the widget that we removed
    // since it would have to be mViewWidget and we want to be able to reuse it

    return 0;
}

//==============================================================================

QString SingleCellSimulationViewPlugin::viewName() const
{
    // Return our single cell view's name

    return tr("Single Cell");
}

//==============================================================================

QIcon SingleCellSimulationViewPlugin::fileTabIcon(const QString &pFileName) const
{
    // Return the requested file tab icon

    return mViewWidget->fileTabIcon(pFileName);
}

//==============================================================================

void SingleCellSimulationViewPlugin::retranslateUi()
{
    // Retranslate our single cell simulation view

    mViewWidget->retranslateUi();
}

//==============================================================================

}   // namespace SingleCellSimulationView
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
