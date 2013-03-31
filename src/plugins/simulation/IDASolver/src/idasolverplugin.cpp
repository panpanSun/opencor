//==============================================================================
// IDASolver plugin
//==============================================================================

#include "idasolver.h"
#include "idasolverplugin.h"

//==============================================================================

namespace OpenCOR {
namespace IDASolver {

//==============================================================================

PLUGININFO_FUNC IDASolverPluginInfo()
{
    Descriptions descriptions;

    descriptions.insert("en", QString::fromUtf8("A plugin which uses <a href=\"https://computation.llnl.gov/casc/sundials/description/description.html#descr_ida\">IDA</a> to solve DAEs"));
    descriptions.insert("fr", QString::fromUtf8("Une extension qui utilise <a href=\"https://computation.llnl.gov/casc/sundials/description/description.html#descr_ida\">IDA</a> pour résoudre des EADs"));

    return new PluginInfo(PluginInfo::InterfaceVersion001,
                          PluginInfo::General,
                          PluginInfo::Simulation,
                          true,
                          QStringList() << "CoreSolver" << "SUNDIALS",
                          descriptions);
}

//==============================================================================

Solver::Type IDASolverPlugin::type() const
{
    // Return the type of the solver

    return Solver::Dae;
}

//==============================================================================

QString IDASolverPlugin::name() const
{
    // Return the name of the solver

    return "IDA";
}

//==============================================================================

Solver::Properties IDASolverPlugin::properties() const
{
    // Return the properties supported by the solver

    Solver::Properties res = Solver::Properties();

    res.append(Solver::Property(Solver::Double, MaximumStepProperty, DefaultMaximumStep, true));
    res.append(Solver::Property(Solver::Integer, MaximumNumberOfStepsProperty, DefaultMaximumNumberOfSteps));
    res.append(Solver::Property(Solver::Double, RelativeToleranceProperty, DefaultRelativeTolerance));
    res.append(Solver::Property(Solver::Double, AbsoluteToleranceProperty, DefaultAbsoluteTolerance));

    return res;
}

//==============================================================================

void * IDASolverPlugin::instance() const
{
    // Create and return an instance of the solver

    return new IdaSolver();
}

//==============================================================================

}   // namespace IDASolver
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
