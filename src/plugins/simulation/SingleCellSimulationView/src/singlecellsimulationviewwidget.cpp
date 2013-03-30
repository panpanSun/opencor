//==============================================================================
// Single cell simulation view widget
//==============================================================================

#include "cellmlfilemanager.h"
#include "cellmlfileruntime.h"
#include "coreutils.h"
#include "progressbarwidget.h"
#include "propertyeditorwidget.h"
#include "singlecellsimulationviewcontentswidget.h"
#include "singlecellsimulationviewgraphpanelplotwidget.h"
#include "singlecellsimulationviewgraphpanelswidget.h"
#include "singlecellsimulationviewgraphpanelwidget.h"
#include "singlecellsimulationviewinformationparameterswidget.h"
#include "singlecellsimulationviewinformationsimulationwidget.h"
#include "singlecellsimulationviewinformationsolverswidget.h"
#include "singlecellsimulationviewinformationwidget.h"
#include "singlecellsimulationviewplugin.h"
#include "singlecellsimulationviewsimulation.h"
#include "singlecellsimulationviewwidget.h"
#include "toolbarwidget.h"
#include "usermessagewidget.h"

//==============================================================================

#include "ui_singlecellsimulationviewwidget.h"

//==============================================================================

#include <QBrush>
#include <QDesktopWidget>
#include <QDir>
#include <QFileDialog>
#include <QFrame>
#include <QImage>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QScrollBar>
#include <QSettings>
#include <QSplitter>
#include <QTextEdit>
#include <QTimer>
#include <QVariant>

//==============================================================================

#include "float.h"

//==============================================================================

#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "qwt_wheel.h"

//==============================================================================

namespace OpenCOR {
namespace SingleCellSimulationView {

//==============================================================================

static const QString OutputTab  = "&nbsp;&nbsp;&nbsp;&nbsp;";
static const QString OutputGood = " style=\"color: green;\"";
static const QString OutputInfo = " style=\"color: navy;\"";
static const QString OutputBad  = " style=\"color: maroon;\"";
static const QString OutputBrLn = "<br/>\n";

//==============================================================================

SingleCellSimulationViewWidgetCurveData::SingleCellSimulationViewWidgetCurveData(const QString &pFileName,
                                                                                 SingleCellSimulationViewSimulation *pSimulation,
                                                                                 CellMLSupport::CellmlFileRuntimeModelParameter *pModelParameter,
                                                                                 SingleCellSimulationViewGraphPanelPlotCurve *pCurve) :
    mFileName(pFileName),
    mSimulation(pSimulation),
    mModelParameter(pModelParameter),
    mCurve(pCurve),
    mAttached(true)
{
}

//==============================================================================

QString SingleCellSimulationViewWidgetCurveData::fileName() const
{
    // Return our file name

    return mFileName;
}

//==============================================================================

CellMLSupport::CellmlFileRuntimeModelParameter * SingleCellSimulationViewWidgetCurveData::modelParameter() const
{
    // Return our model parameter

    return mModelParameter;
}

//==============================================================================

SingleCellSimulationViewGraphPanelPlotCurve * SingleCellSimulationViewWidgetCurveData::curve() const
{
    // Return our curve

    return mCurve;
}

//==============================================================================

double * SingleCellSimulationViewWidgetCurveData::yData() const
{
    // Return our Y data array

    if (   (mModelParameter->type() == CellMLSupport::CellmlFileRuntimeModelParameter::Constant)
        || (mModelParameter->type() == CellMLSupport::CellmlFileRuntimeModelParameter::ComputedConstant))
        return mSimulation->results()->constants()?mSimulation->results()->constants()[mModelParameter->index()]:0;
    else if (mModelParameter->type() == CellMLSupport::CellmlFileRuntimeModelParameter::State)
        return mSimulation->results()->states()?mSimulation->results()->states()[mModelParameter->index()]:0;
    else if (mModelParameter->type() == CellMLSupport::CellmlFileRuntimeModelParameter::Rate)
        return mSimulation->results()->rates()?mSimulation->results()->rates()[mModelParameter->index()]:0;
    else
        return mSimulation->results()->algebraic()?mSimulation->results()->algebraic()[mModelParameter->index()]:0;
}

//==============================================================================

bool SingleCellSimulationViewWidgetCurveData::isAttached() const
{
    // Return our attached status

    return mAttached;
}

//==============================================================================

void SingleCellSimulationViewWidgetCurveData::setAttached(const bool &pAttached)
{
    // Set our attached status

    mAttached = pAttached;
}

//==============================================================================

SingleCellSimulationViewWidget::SingleCellSimulationViewWidget(SingleCellSimulationViewPlugin *pPluginParent,
                                                               QWidget *pParent) :
    ViewWidget(pParent),
    mGui(new Ui::SingleCellSimulationViewWidget),
    mPluginParent(pPluginParent),
    mSolverInterfaces(SolverInterfaces()),
    mSimulation(0),
    mSimulations(QMap<QString, SingleCellSimulationViewSimulation *>()),
    mStoppedSimulations(QList<SingleCellSimulationViewSimulation *>()),
    mProgresses(QMap<QString, int>()),
    mResets(QMap<QString, bool>()),
    mDelays(QMap<QString, int>()),
    mAxesSettings(QMap<QString, AxisSettings>()),
    mSplitterWidgetSizes(QList<int>()),
    mCurvesData(QMap<QString, SingleCellSimulationViewWidgetCurveData *>()),
    mOldSimulationResultsSizes(QMap<SingleCellSimulationViewSimulation *, qulonglong>()),
    mCheckResultsSimulations(QList<SingleCellSimulationViewSimulation *>())
{
    // Set up the GUI

    mGui->setupUi(this);

    // Create a wheel (and a label to show its value) to specify the delay (in
    // milliseconds) between the output of two data points

    mDelayWidget = new QwtWheel(this);
#if defined(Q_OS_WIN) || defined(Q_OS_LINUX)
    QWidget *delaySpaceWidget = new QWidget(this);
#endif
    mDelayValueWidget = new QLabel(this);

    mDelayWidget->setBorderWidth(0);
    mDelayWidget->setFixedSize(0.07*qApp->desktop()->screenGeometry().width(),
                               0.5*mDelayWidget->height());
    mDelayWidget->setFocusPolicy(Qt::NoFocus);
    mDelayWidget->setRange(0.0, 50.0);
    mDelayWidget->setWheelBorderWidth(0);

#if defined(Q_OS_WIN) || defined(Q_OS_LINUX)
    delaySpaceWidget->setFixedWidth(4);
#endif

    connect(mDelayWidget, SIGNAL(valueChanged(double)),
            this, SLOT(updateDelayValue(const double &)));

    setDelayValue(0);

    // Create a tool bar widget with different buttons

    mToolBarWidget = new Core::ToolBarWidget(this);

    mToolBarWidget->addAction(mGui->actionRun);
    mToolBarWidget->addAction(mGui->actionPause);
    mToolBarWidget->addAction(mGui->actionStop);
    mToolBarWidget->addSeparator();
    mToolBarWidget->addAction(mGui->actionReset);
    mToolBarWidget->addSeparator();
    mToolBarWidget->addWidget(mDelayWidget);
#if defined(Q_OS_WIN) || defined(Q_OS_LINUX)
    mToolBarWidget->addWidget(delaySpaceWidget);
#endif
    mToolBarWidget->addWidget(mDelayValueWidget);
/*---GRY--- THE BELOW SHOULD BE RE-ENABLED AT SOME POINT...
    mToolBarWidget->addSeparator();
    mToolBarWidget->addAction(mGui->actionDebugMode);
    mToolBarWidget->addSeparator();
    mToolBarWidget->addAction(mGui->actionAdd);
    mToolBarWidget->addAction(mGui->actionRemove);
*/
    mToolBarWidget->addSeparator();
    mToolBarWidget->addAction(mGui->actionCsvExport);

    mTopSeparator = Core::newLineWidget(this);

    mGui->layout->addWidget(mToolBarWidget);
    mGui->layout->addWidget(mTopSeparator);

    // Create our splitter widget and keep track of its movement
    // Note: we need to keep track of its movement so that saveSettings() can
    //       work fine even when mContentsWidget is not visible (which happens
    //       when a CellML file cannot be run for some reason or another)...

    mSplitterWidget = new QSplitter(Qt::Vertical, this);

    connect(mSplitterWidget, SIGNAL(splitterMoved(int,int)),
            this, SLOT(splitterWidgetMoved()));

    // Create our contents widget

    mContentsWidget = new SingleCellSimulationViewContentsWidget(this);

    mContentsWidget->setObjectName("Contents");

    // Keep track of changes to some of our simulation and solvers properties

    connect(mContentsWidget->informationWidget()->simulationWidget(), SIGNAL(propertyChanged(Core::Property *)),
            this, SLOT(simulationPropertyChanged(Core::Property *)));
    connect(mContentsWidget->informationWidget()->solversWidget(), SIGNAL(propertyChanged(Core::Property *)),
            this, SLOT(solversPropertyChanged(Core::Property *)));

    // Keep track of whether we can remove graph panels

    connect(mContentsWidget->graphPanelsWidget(), SIGNAL(removeGraphPanelsEnabled(const bool &)),
            mGui->actionRemove, SLOT(setEnabled(bool)));

    // Keep track of which model parameters to show/hide

    connect(mContentsWidget->informationWidget()->parametersWidget(), SIGNAL(showModelParameter(const QString &, CellMLSupport::CellmlFileRuntimeModelParameter *, const bool &)),
            this, SLOT(showModelParameter(const QString &, CellMLSupport::CellmlFileRuntimeModelParameter *, const bool &)));

    // Create and add our invalid simulation message widget

    mInvalidModelMessageWidget = new Core::UserMessageWidget(":/oxygen/actions/help-about.png",
                                                             pParent);

    mGui->layout->addWidget(mInvalidModelMessageWidget);

    // Create our simulation output widget with a layout on which we put a
    // separating line and our simulation output list view
    // Note: the separating line is because we remove, for aesthetical reasons,
    //       the border of our simulation output list view...

    QWidget *simulationOutputWidget = new QWidget(this);
    QVBoxLayout *simulationOutputLayout= new QVBoxLayout(simulationOutputWidget);

    simulationOutputLayout->setMargin(0);
    simulationOutputLayout->setSpacing(0);

    simulationOutputWidget->setLayout(simulationOutputLayout);

    mOutputWidget = new QTextEdit(this);

    mOutputWidget->setFrameStyle(QFrame::NoFrame);

    simulationOutputLayout->addWidget(Core::newLineWidget(this));
    simulationOutputLayout->addWidget(mOutputWidget);

    // Populate our splitter and use as much space as possible for it by asking
    // for its height to be that of the desktop's, and then add our splitter to
    // our single cell simulation view widget

    mSplitterWidget->addWidget(mContentsWidget);
    mSplitterWidget->addWidget(simulationOutputWidget);

    mSplitterWidget->setSizes(QList<int>() << qApp->desktop()->screenGeometry().height() << 1);

    mGui->layout->addWidget(mSplitterWidget);

    // Create our (thin) simulation progress widget

    mProgressBarWidget = new Core::ProgressBarWidget(this);

    mProgressBarWidget->setFixedHeight(3);
    mProgressBarWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    mBottomSeparator = Core::newLineWidget(this);

    mGui->layout->addWidget(mBottomSeparator);
    mGui->layout->addWidget(mProgressBarWidget);

    // Make our contents widget our focus proxy

    setFocusProxy(mContentsWidget);
}

//==============================================================================

SingleCellSimulationViewWidget::~SingleCellSimulationViewWidget()
{
    // Delete our simulation objects

    foreach (SingleCellSimulationViewSimulation *simulation, mSimulations)
        delete simulation;

    // Delete our curves' data

    foreach (SingleCellSimulationViewWidgetCurveData *curveData, mCurvesData) {
        delete curveData->curve();
        delete curveData;
    }

    // Delete the GUI

    delete mGui;
}

//==============================================================================

void SingleCellSimulationViewWidget::retranslateUi()
{
    // Retranslate our GUI

    mGui->retranslateUi(this);

    // Retranslate our delay and delay value widgets

    mDelayWidget->setToolTip(tr("Delay"));
    mDelayValueWidget->setToolTip(mDelayWidget->toolTip());

    mDelayWidget->setStatusTip(tr("Delay in milliseconds between two data points"));
    mDelayValueWidget->setStatusTip(mDelayWidget->statusTip());

    // Retranslate our invalid model message

    updateInvalidModelMessageWidget();

    // Retranslate our contents widget

    mContentsWidget->retranslateUi();
}

//==============================================================================

static const QString SettingsSizesCount = "SizesCount";
static const QString SettingsSize       = "Size%1";

//==============================================================================

void SingleCellSimulationViewWidget::loadSettings(QSettings *pSettings)
{
    // Retrieve and set the sizes of our splitter

    int sizesCount = pSettings->value(SettingsSizesCount, 0).toInt();

    if (sizesCount) {
        mSplitterWidgetSizes = QList<int>();

        for (int i = 0; i < sizesCount; ++i)
            mSplitterWidgetSizes << pSettings->value(SettingsSize.arg(i)).toInt();

        mSplitterWidget->setSizes(mSplitterWidgetSizes);
    }

    // Retrieve the settings of our contents widget

    pSettings->beginGroup(mContentsWidget->objectName());
        mContentsWidget->loadSettings(pSettings);
    pSettings->endGroup();

    // Retrieve our active graph panel
//---GRY--- WE SHOULD STOP USING mActiveGraphPanel ONCE WE HAVE RE-ENABLED THE
//          ADDITION/REMOVAL OF GRAPH PANELS...

    mActiveGraphPanel = mContentsWidget->graphPanelsWidget()->activeGraphPanel();

    mActiveGraphPanel->plot()->setFixedAxisX(true);
}

//==============================================================================

void SingleCellSimulationViewWidget::saveSettings(QSettings *pSettings) const
{
    // Keep track of our splitter sizes

    pSettings->setValue(SettingsSizesCount, mSplitterWidgetSizes.count());

    for (int i = 0, iMax = mSplitterWidgetSizes.count(); i < iMax; ++i)
        pSettings->setValue(SettingsSize.arg(i), mSplitterWidgetSizes[i]);

    // Keep track of the settings of our contents widget

    pSettings->beginGroup(mContentsWidget->objectName());
        mContentsWidget->saveSettings(pSettings);
    pSettings->endGroup();
}

//==============================================================================

void SingleCellSimulationViewWidget::setSolverInterfaces(const SolverInterfaces &pSolverInterfaces)
{
    // Let our solvers widget know about the solver interfaces

    mContentsWidget->informationWidget()->solversWidget()->setSolverInterfaces(pSolverInterfaces);

    // Keep track of the solver interfaces

    mSolverInterfaces = pSolverInterfaces;
}

//==============================================================================

void SingleCellSimulationViewWidget::output(const QString &pMessage)
{
    // Move to the end of the output (just in case...)

    mOutputWidget->moveCursor(QTextCursor::End);

    // Make sure that the output ends as expected and if not then add BrLn to it

    static const QString EndOfOutput = "<br /></p></body></html>";

    if (mOutputWidget->toHtml().right(EndOfOutput.size()).compare(EndOfOutput))
        mOutputWidget->insertHtml(OutputBrLn);

    // Output the message and make sure that it's visible

    mOutputWidget->insertHtml(pMessage);
    mOutputWidget->ensureCursorVisible();
}

//==============================================================================

void SingleCellSimulationViewWidget::updateSimulationMode()
{
    bool simulationModeEnabled = mSimulation->isRunning() || mSimulation->isPaused();

    // Show/hide our run and pause actions

    mGui->actionRun->setVisible(!mSimulation->isRunning() || mSimulation->isPaused());
    mGui->actionPause->setVisible(!mGui->actionRun->isVisible());

    // Enable/disable our stop action

    mGui->actionStop->setEnabled(simulationModeEnabled);

    // Enable or disable our simulation and solvers widgets

    mContentsWidget->informationWidget()->simulationWidget()->setEnabled(!simulationModeEnabled);
    mContentsWidget->informationWidget()->solversWidget()->setEnabled(!simulationModeEnabled);

    // Enable/disable our export to CSV

    mGui->actionCsvExport->setEnabled(   mSimulation->results()->size()
                                      && !simulationModeEnabled);

    // Give the focus to our focus proxy, in case we leave our simulation mode
    // (so that the user can modify simulation data, etc.)

    if (!simulationModeEnabled)
        focusProxy()->setFocus();
}

//==============================================================================

void SingleCellSimulationViewWidget::updateInvalidModelMessageWidget()
{
    // Update our invalid model message

    mInvalidModelMessageWidget->setMessage("<div align=center>"
                                           "    <p>"
                                          +((mErrorType == InvalidCellmlFile)?
                                                "        "+tr("Sorry, but the <strong>%1</strong> view requires a valid CellML file to work...").arg(mPluginParent->viewName()):
                                                "        "+tr("Sorry, but the <strong>%1</strong> view requires a valid simulation environment to work...").arg(mPluginParent->viewName())
                                           )
                                          +"    </p>"
                                           "    <p>"
                                           "        <small><em>("+tr("See below for more information.")+")</em></small>"
                                           "    </p>"
                                           "</div>");
}

//==============================================================================

void SingleCellSimulationViewWidget::initialize(const QString &pFileName)
{
    // Keep track of our simulation data for our previous model and finalise a
    // few things, if needed

    SingleCellSimulationViewSimulation *previousSimulation = mSimulation;
    SingleCellSimulationViewInformationWidget *informationWidget = mContentsWidget->informationWidget();
    SingleCellSimulationViewInformationSimulationWidget *simulationWidget = informationWidget->simulationWidget();
    SingleCellSimulationViewInformationSolversWidget *solversWidget = informationWidget->solversWidget();
    SingleCellSimulationViewInformationParametersWidget *parametersWidget = informationWidget->parametersWidget();

    if (previousSimulation) {
        // There is a previous simulation, so backup a few things

        QString previousFileName = previousSimulation->fileName();

        simulationWidget->backup(previousFileName);
        solversWidget->backup(previousFileName);

        // Keep track of the status of the reset action and of the value of the
        // delay widget

        mResets.insert(previousFileName, mGui->actionReset->isEnabled());
        mDelays.insert(previousFileName, mDelayWidget->value());

        // Keept rack of our graph panel's plot's axes settings

        AxisSettings axisSettings;

        axisSettings.minX = mActiveGraphPanel->plot()->minX();
        axisSettings.maxX = mActiveGraphPanel->plot()->maxX();
        axisSettings.minY = mActiveGraphPanel->plot()->minY();
        axisSettings.maxY = mActiveGraphPanel->plot()->maxY();

        axisSettings.localMinX = mActiveGraphPanel->plot()->localMinX();
        axisSettings.localMaxX = mActiveGraphPanel->plot()->localMaxX();
        axisSettings.localMinY = mActiveGraphPanel->plot()->localMinY();
        axisSettings.localMaxY = mActiveGraphPanel->plot()->localMaxY();

        mAxesSettings.insert(previousFileName, axisSettings);

        // Keep track of the attachment status of our curves

        foreach (SingleCellSimulationViewWidgetCurveData *curveData, mCurvesData)
            if (!curveData->fileName().compare(previousFileName))
                curveData->setAttached(curveData->curve()->plot());
    }

    // Retrieve our simulation object for the current model, if any

    bool newSimulation = false;

    CellMLSupport::CellmlFile *cellmlFile = CellMLSupport::CellmlFileManager::instance()->cellmlFile(pFileName);
    CellMLSupport::CellmlFileRuntime *cellmlFileRuntime = cellmlFile->runtime();

    mSimulation = mSimulations.value(pFileName);

    if (!mSimulation) {
        // No simulation object currently exists for the model, so create one

        mSimulation = new SingleCellSimulationViewSimulation(pFileName, cellmlFileRuntime, mSolverInterfaces);

        newSimulation = true;

        // Set our simulation object's delay

        mSimulation->setDelay(mDelayWidget->value());

        // Create a few connections

        connect(mSimulation, SIGNAL(running(const bool &)),
                this, SLOT(simulationRunning(const bool &)));
        connect(mSimulation, SIGNAL(paused()),
                this, SLOT(simulationPaused()));
        connect(mSimulation, SIGNAL(stopped(const int &)),
                this, SLOT(simulationStopped(const int &)));

        connect(mSimulation, SIGNAL(error(const QString &)),
                this, SLOT(simulationError(const QString &)));

        connect(mSimulation->data(), SIGNAL(modified(const bool &)),
                this, SLOT(simulationDataModified(const bool &)));

        // Keep track of our simulation object

        mSimulations.insert(pFileName, mSimulation);
    }

    // Retrieve the status of the reset action and the value of the delay widget

    mGui->actionReset->setEnabled(mResets.value(pFileName, false));

    setDelayValue(mDelays.value(pFileName, 0));

    // Reset our file tab icon and stop tracking our simulation progress, in
    // case our simulation is running

    if (mSimulation->isRunning()) {
        mProgresses.remove(mSimulation->fileName());

        emit updateFileTabIcon(mSimulation->fileName(), QIcon());
    }

    // Output some information about our CellML file

    QString information = QString();

    if (!mOutputWidget->document()->isEmpty())
        information += "<hr/>\n";

    information += "<strong>"+pFileName+"</strong>"+OutputBrLn;
    information += OutputTab+"<strong>"+tr("Runtime:")+"</strong> ";

    bool validCellmlFileRuntime = cellmlFileRuntime && cellmlFileRuntime->isValid();

    if (validCellmlFileRuntime) {
        // A valid runtime could be retrieved for the CellML file

        QString additionalInformation = QString();

        if (cellmlFileRuntime->needNlaSolver())
            additionalInformation = " + "+tr("NLA system(s)");

        information += "<span"+OutputGood+">"+tr("valid")+"</span>."+OutputBrLn;
        information += QString(OutputTab+"<strong>"+tr("Model type:")+"</strong> <span"+OutputInfo+">%1%2</span>."+OutputBrLn).arg((cellmlFileRuntime->modelType() == CellMLSupport::CellmlFileRuntime::Ode)?tr("ODE"):tr("DAE"),
                                                                                                                                   additionalInformation);
    } else {
        // We couldn't retrieve a runtime for the CellML file or we could, but
        // it's not valid

        information += "<span"+OutputBad+">"+(cellmlFileRuntime?tr("invalid"):tr("none"))+"</span>."+OutputBrLn;

        foreach (const CellMLSupport::CellmlFileIssue &issue,
                 cellmlFileRuntime?cellmlFileRuntime->issues():cellmlFile->issues())
            information += QString(OutputTab+"<span"+OutputBad+"><strong>%1</strong> %2</span>."+OutputBrLn).arg((issue.type() == CellMLSupport::CellmlFileIssue::Error)?tr("Error:"):tr("Warning:"),
                                                                                                                 issue.message());
    }

    output(information);

    // Retrieve our variable of integration, if any

    CellMLSupport::CellmlFileRuntimeModelParameter *variableOfIntegration = validCellmlFileRuntime?cellmlFileRuntime->variableOfIntegration():0;

    // Enable/disable our run action depending on whether we have a variable of
    // integration

    mGui->actionRun->setEnabled(variableOfIntegration);

    // Update our simulation mode

    updateSimulationMode();

    // Update our previous (if any) and current simulation results

    if (   previousSimulation
        && (previousSimulation->isRunning() || previousSimulation->isPaused()))
        updateResults(previousSimulation, previousSimulation->results()->size());

    updateResults(mSimulation, mSimulation->results()->size(), true);

    // Check that we have a valid runtime

    bool hasError = true;

    if (validCellmlFileRuntime) {
        // We have a valid runtime
        // Note: if we didn't have a valid runtime, then there would be no need
        //       to output an error message since one would have already been
        //       generated while trying to get the runtime (see above)...

        // Retrieve the unit of the variable of integration, if any

        if (!variableOfIntegration) {
            // We don't have a variable of integration, so...

            simulationError(tr("the model must have at least one ODE or DAE"),
                            InvalidCellmlFile);
        } else {
            // Show our contents widget in case it got previously hidden
            // Note: indeed, if it was to remain hidden then some
            //       initialisations wouldn't work (e.g. the solvers widget has
            //       property editor which all properties need to be removed and
            //       if the contents widget is not visible, then upon
            //       repopulating the property editor, scrollbars will be shown
            //       even though they are not needed), so...

            mContentsWidget->setVisible(true);

            // Initialise our GUI's simulation, solvers and parameters widgets
            // Note: this will also initialise some of our simulation data (i.e.
            //       our simulation's starting point and simulation's NLA
            //       solver's properties) which is needed since we want to be
            //       able to reset our simulation below...

            simulationWidget->initialize(pFileName, cellmlFileRuntime, mSimulation->data());
            solversWidget->initialize(pFileName, cellmlFileRuntime, mSimulation->data());
            parametersWidget->initialize(pFileName, cellmlFileRuntime, mSimulation->data());

#ifdef QT_DEBUG
            // Output the type of solvers that are available to run the model

            qDebug("---------------------------------------");
            qDebug("%s", qPrintable(pFileName));

            information = QString();

            if (cellmlFileRuntime->needOdeSolver()) {
                information += "\n - ODE solver(s): ";

                int solverCounter = 0;

                foreach (const QString &odeSolver, solversWidget->odeSolvers())
                    if (++solverCounter == 1)
                        information += odeSolver;
                    else
                        information += " | "+odeSolver;

                if (!solverCounter)
                    information += "none available";
            }

            if (cellmlFileRuntime->needDaeSolver()) {
                information += "\n - DAE solver(s): ";

                int solverCounter = 0;

                foreach (const QString &daeSolver, solversWidget->daeSolvers())
                    if (++solverCounter == 1)
                        information += daeSolver;
                    else
                        information += " | "+daeSolver;

                if (!solverCounter)
                    information += "none available";
            }

            if (cellmlFileRuntime->needNlaSolver()) {
                information += "\n - NLA solver(s): ";

                int solverCounter = 0;

                foreach (const QString &nlaSolver, solversWidget->nlaSolvers())
                    if (++solverCounter == 1)
                        information += nlaSolver;
                    else
                        information += " | "+nlaSolver;

                if (!solverCounter)
                    information += "none available";
            }

            qDebug("%s", qPrintable(information.remove(0, 1)));
            // Note: we must remove the leading '\n'...
#endif

            // Check whether we have at least one ODE or DAE solver and, if
            // needed, at least one NLA solver

            if (cellmlFileRuntime->needNlaSolver()) {
                if (solversWidget->nlaSolvers().isEmpty()) {
                    if (cellmlFileRuntime->needOdeSolver()) {
                        if (solversWidget->odeSolvers().isEmpty())
                            simulationError(tr("the model needs both an ODE and an NLA solver, but none are available"),
                                            InvalidSimulationEnvironment);
                        else
                            simulationError(tr("the model needs both an ODE and an NLA solver, but no NLA solver is available"),
                                            InvalidSimulationEnvironment);
                    } else {
                        if (solversWidget->daeSolvers().isEmpty())
                            simulationError(tr("the model needs both a DAE and an NLA solver, but none are available"),
                                            InvalidSimulationEnvironment);
                        else
                            simulationError(tr("the model needs both a DAE and an NLA solver, but no NLA solver is available"),
                                            InvalidSimulationEnvironment);
                    }
                } else if (   cellmlFileRuntime->needOdeSolver()
                           && solversWidget->odeSolvers().isEmpty()) {
                    simulationError(tr("the model needs both an ODE and an NLA solver, but no ODE solver is available"),
                                    InvalidSimulationEnvironment);
                } else if (   cellmlFileRuntime->needDaeSolver()
                           && solversWidget->daeSolvers().isEmpty()) {
                        simulationError(tr("the model needs both a DAE and an NLA solver, but no DAE solver is available"),
                                        InvalidSimulationEnvironment);
                } else {
                    // We have the solver(s) we need, so...

                    hasError = false;
                }
            } else if (   cellmlFileRuntime->needOdeSolver()
                       && solversWidget->odeSolvers().isEmpty()) {
                simulationError(tr("the model needs an ODE solver, but none is available"),
                                InvalidSimulationEnvironment);
            } else if (   cellmlFileRuntime->needDaeSolver()
                       && solversWidget->daeSolvers().isEmpty()) {
                simulationError(tr("the model needs a DAE solver, but none is available"),
                                InvalidSimulationEnvironment);
            } else {
                // We have the solver(s) we need, so...

                hasError = false;
            }
        }
    }

    // Check if an error occurred and, if so, show/hide some widgets

    bool previousHasError = mInvalidModelMessageWidget->isVisible();

    mToolBarWidget->setVisible(!hasError);
    mTopSeparator->setVisible(!hasError);

    mContentsWidget->setVisible(!hasError);
    mInvalidModelMessageWidget->setVisible(hasError);

    mBottomSeparator->setVisible(!hasError);
    mProgressBarWidget->setVisible(!hasError);

    // Make sure that the last output message is visible
    // Note: indeed, to (re)show some widgets (see above) will change the height
    //       of our output widget, messing up the vertical scroll bar a bit (if
    //       visible), resulting in the output being shifted a bit, so...

    if (previousHasError != hasError) {
        qApp->processEvents();

        mOutputWidget->ensureCursorVisible();
    }

    // If no error occurred and if we are dealing with a new simulation, then
    // reset both its data and its results (well, initialise in the case of its
    // data)

    if (!hasError && newSimulation) {
        mSimulation->data()->reset();
        mSimulation->results()->reset(false);
    }

    // Attach/detach the curves, based on whether they are associated with then
    // given file name

    foreach (SingleCellSimulationViewWidgetCurveData *curveData, mCurvesData)
        if (    curveData->isAttached()
            && !curveData->fileName().compare(pFileName)) {
            curveData->curve()->setRawSamples(mSimulation->results()->points(),
                                              curveData->yData(),
                                              mSimulation->results()->size());

            mActiveGraphPanel->plot()->attach(curveData->curve());
        } else {
            mActiveGraphPanel->plot()->detach(curveData->curve());
        }

    // Retrieve our graph panel's plot's axes settings and replot our graph
    // panel's plot, if available

    if (mAxesSettings.contains(pFileName)) {
        // We have some axes settings for the given file name, so retrieve its
        // graph panel's plot's axes settings

        AxisSettings axisSettings = mAxesSettings.value(pFileName);

        mActiveGraphPanel->plot()->setMinX(axisSettings.minX);
        mActiveGraphPanel->plot()->setMaxX(axisSettings.maxX);
        mActiveGraphPanel->plot()->setMinY(axisSettings.minY);
        mActiveGraphPanel->plot()->setMaxY(axisSettings.maxY);

        mActiveGraphPanel->plot()->setLocalMinX(axisSettings.localMinX);
        mActiveGraphPanel->plot()->setLocalMaxX(axisSettings.localMaxX);
        mActiveGraphPanel->plot()->setLocalMinY(axisSettings.localMinY);
        mActiveGraphPanel->plot()->setLocalMaxY(axisSettings.localMaxY);
    } else {
        // We don't have any axes settings for the given file name, so first
        // initialise our simulation's properties

        simulationPropertyChanged(mContentsWidget->informationWidget()->simulationWidget()->startingPointProperty());
        simulationPropertyChanged(mContentsWidget->informationWidget()->simulationWidget()->endingPointProperty());
        simulationPropertyChanged(mContentsWidget->informationWidget()->simulationWidget()->pointIntervalProperty());

        // Now, initialise our graph panel's plot's axes settings

        mActiveGraphPanel->plot()->setMinY(0.0);
        mActiveGraphPanel->plot()->setMaxY(1000.0);

        mActiveGraphPanel->plot()->setLocalMinX(mActiveGraphPanel->plot()->minX());
        mActiveGraphPanel->plot()->setLocalMaxX(mActiveGraphPanel->plot()->maxX());
        mActiveGraphPanel->plot()->setLocalMinY(mActiveGraphPanel->plot()->minY());
        mActiveGraphPanel->plot()->setLocalMaxY(mActiveGraphPanel->plot()->maxY());
    }

    // Check our graph panel's plot's local axes and then replot our graph
    // panel's plot
    // Note: we always want to replot everything, hence our passing false to
    //       checkLocalAxes()...

    mActiveGraphPanel->plot()->checkLocalAxes(false);
    mActiveGraphPanel->plot()->replotNow();

    // Allow/prevent interaction with our graph panel's plot

    mActiveGraphPanel->plot()->setInteractive(!mSimulation->isRunning());
}

//==============================================================================

bool SingleCellSimulationViewWidget::isManaged(const QString &pFileName) const
{
    // Return whether the given file name is managed

    return mSimulations.value(pFileName);
}

//==============================================================================

void SingleCellSimulationViewWidget::finalize(const QString &pFileName)
{
    // Remove our simulation object, should there be one for the given file name

    SingleCellSimulationViewSimulation *simulation = mSimulations.value(pFileName);

    if (simulation) {
        // There is a simulation object for the given file name, so delete it
        // and remove it from our list

        delete simulation;

        mSimulations.remove(pFileName);

        // Reset our memory of the current simulation object, but only if it's
        // the same as our simulation object

        if (simulation == mSimulation)
            mSimulation = 0;
    }

    // Remove our curves' data associated with the given file name, if any

    QList<QString> fileNames = QList<QString>();
    QList<CellMLSupport::CellmlFileRuntimeModelParameter *> modelParameters = QList<CellMLSupport::CellmlFileRuntimeModelParameter *>();

    foreach (SingleCellSimulationViewWidgetCurveData *curveData, mCurvesData)
        if (!curveData->fileName().compare(pFileName)) {
            // Keep track of the file name and model parameter of the curve data

            fileNames << curveData->fileName();
            modelParameters << curveData->modelParameter();

            // Delete the curve and the curve data themselves

            delete curveData->curve();
            delete curveData;
        }

    for (int i = 0, iMax = fileNames.count(); i < iMax; ++i)
        mCurvesData.remove(modelParameterKey(fileNames[i], modelParameters[i]));

    // Remove various information associated with the given file name

    mProgresses.remove(pFileName);

    mResets.remove(pFileName);
    mDelays.remove(pFileName);

    mAxesSettings.remove(pFileName);

    // Finalize a few things in our simulation and solvers widgets

    mContentsWidget->informationWidget()->simulationWidget()->finalize(pFileName);
    mContentsWidget->informationWidget()->solversWidget()->finalize(pFileName);
}

//==============================================================================

int SingleCellSimulationViewWidget::tabBarIconSize() const
{
    // Return the size of a file tab icon

    return style()->pixelMetric(QStyle::PM_TabBarIconSize, 0, this);
}

//==============================================================================

QIcon SingleCellSimulationViewWidget::fileTabIcon(const QString &pFileName) const
{
    // Return a file tab icon which shows the given file's simulation progress

    SingleCellSimulationViewSimulation *simulation = mSimulations.value(pFileName);
    int progress = simulation?mProgresses.value(simulation->fileName(), -1):-1;

    if (simulation && (progress != -1)) {
        // Create an image that shows the progress of our simulation

        QImage tabBarIcon = QImage(tabBarIconSize(), mProgressBarWidget->height()+2,
                                   QImage::Format_ARGB32_Premultiplied);
        QPainter tabBarIconPainter(&tabBarIcon);

        tabBarIconPainter.setBrush(QBrush(CommonWidget::windowColor()));
        tabBarIconPainter.setPen(QPen(CommonWidget::borderColor()));

        tabBarIconPainter.drawRect(0, 0, tabBarIcon.width()-1, tabBarIcon.height()-1);
        tabBarIconPainter.fillRect(1, 1, progress, tabBarIcon.height()-2,
                                   CommonWidget::highlightColor());

        return QIcon(QPixmap::fromImage(tabBarIcon));
    } else {
        // No simulation object currently exists for the model, so...

        return QIcon();
    }
}

//==============================================================================

void SingleCellSimulationViewWidget::checkAxisY()
{
    // Reset the Y axis, if needed

    if (   (mActiveGraphPanel->plot()->minY() ==  DBL_MAX)
        && (mActiveGraphPanel->plot()->maxY() == -DBL_MAX)) {
        // The Y axis still has the values we set up before running a simulation
        // (so that the automatic rescaling of the Y axis could work) which
        // means that either the model couldn't be run or no model parameters
        // were plotted, so we need to reset the Y axis

        mActiveGraphPanel->plot()->setMinY(0);
        mActiveGraphPanel->plot()->setMaxY(1000.0);

        mActiveGraphPanel->plot()->setLocalMinY(mActiveGraphPanel->plot()->minY());
        mActiveGraphPanel->plot()->setLocalMaxY(mActiveGraphPanel->plot()->maxY());
    }
}

//==============================================================================

void SingleCellSimulationViewWidget::on_actionRun_triggered()
{
    // Run or resume our simulation

    if (mSimulation->isPaused()) {
        // Our simulation is paused, so resume it

        mSimulation->resume();
    } else {
        // Our simulation is not paused, so cancel any editing of our simulation
        // information

        mContentsWidget->informationWidget()->cancelEditing();

        // Now, we would normally retrieve our simulation properties, but there
        // is no need for it since they have already been retrieved (see
        // simulationPropertyChanged())...

        // Retrieve our solvers' properties
        // Note: we don't need to retrieve the NLA solver's properties since we
        //       already have them (see solversPropertyChanged())...

        SingleCellSimulationViewSimulationData *simulationData = mSimulation->data();
        SingleCellSimulationViewInformationSolversWidget *solversWidget = mContentsWidget->informationWidget()->solversWidget();

        simulationData->setOdeSolverName(solversWidget->odeSolverData()->solversListProperty()->value()->text());
        simulationData->setDaeSolverName(solversWidget->daeSolverData()->solversListProperty()->value()->text());

        foreach (Core::Property *property, solversWidget->odeSolverData()->solversProperties().value(simulationData->odeSolverName()))
            simulationData->addOdeSolverProperty(property->name()->text(),
                                                 (property->value()->type() == Core::PropertyItem::Integer)?
                                                     Core::PropertyEditorWidget::integerPropertyItem(property->value()):
                                                     Core::PropertyEditorWidget::doublePropertyItem(property->value()));

        foreach (Core::Property *property, solversWidget->daeSolverData()->solversProperties().value(simulationData->daeSolverName()))
            simulationData->addDaeSolverProperty(property->name()->text(),
                                                 (property->value()->type() == Core::PropertyItem::Integer)?
                                                     Core::PropertyEditorWidget::integerPropertyItem(property->value()):
                                                     Core::PropertyEditorWidget::doublePropertyItem(property->value()));

        // Check how much memory is needed to run our simulation

        bool runSimulation = true;

        double freeMemory = Core::freeMemory();
        double requiredMemory = mSimulation->requiredMemory();

        if (requiredMemory > freeMemory) {
            // More memory is required to run our simulation than is currently
            // available, so let our user know about it

            QMessageBox::warning(qApp->activeWindow(), tr("Run the simulation"),
                                 tr("Sorry, but the simulation requires %1 of memory while you have %2 left.").arg(Core::sizeAsString(requiredMemory), Core::sizeAsString(freeMemory)));

            runSimulation = false;
        }

        // Run our simulation, if possible/wanted

        if (runSimulation) {
            // Reset our local X axis

            mActiveGraphPanel->plot()->setLocalMinX(mActiveGraphPanel->plot()->minX());
            mActiveGraphPanel->plot()->setLocalMaxX(mActiveGraphPanel->plot()->maxX());

            // Check (and reset, if needed) our Y axis
            // Note: this is in case we tried to run a simulation (and therefore
            //       set the Y axis so the automatic rescaling could work; see
            //       below) and the model cannot be run, in which case the Y
            //       axis would be completely messed up...

            checkAxisY();

            // Reset our simulation settings

            mOldSimulationResultsSizes.insert(mSimulation, 0);

            runSimulation = mSimulation->results()->reset();

            updateResults(mSimulation, 0);

            // Effectively run our simulation, if possible

            if (runSimulation) {
                // Set our Y axis, so that it will automatically rescale
                // Note: we definitely don't want to check our axes since this
                //       will inevitably generate a replot which with the Y axis
                //       values we are using would really mess things up, so...

                mActiveGraphPanel->plot()->setMinY(DBL_MAX);
                mActiveGraphPanel->plot()->setMaxY(-DBL_MAX);

                mActiveGraphPanel->plot()->setLocalMinY(mActiveGraphPanel->plot()->minY());
                mActiveGraphPanel->plot()->setLocalMaxY(mActiveGraphPanel->plot()->maxY());

                // Now, we really run our simulation

                mSimulation->run();
            } else {
                QMessageBox::warning(qApp->activeWindow(), tr("Run the simulation"),
                                     tr("Sorry, but we could not allocate all the memory required for the simulation."));
            }
        }
    }
}

//==============================================================================

void SingleCellSimulationViewWidget::on_actionPause_triggered()
{
    // Pause our simulation

    mSimulation->pause();
}

//==============================================================================

void SingleCellSimulationViewWidget::on_actionStop_triggered()
{
    // Stop our simulation

    mSimulation->stop();
}

//==============================================================================

void SingleCellSimulationViewWidget::on_actionReset_triggered()
{
    // Reset our simulation parameters

    mSimulation->data()->reset();

    // Reset our worker
    // Note: indeed, if we are running a simulation then we may need to
    //       reinitialise our solver (e.g. CVODE)...

    mSimulation->resetWorker();
}

//==============================================================================

void SingleCellSimulationViewWidget::on_actionDebugMode_triggered()
{
//---GRY--- TO BE DONE...
}

//==============================================================================

void SingleCellSimulationViewWidget::on_actionAdd_triggered()
{
    // Ask our graph panels widget to add a new graph panel

    mContentsWidget->graphPanelsWidget()->addGraphPanel();
}

//==============================================================================

void SingleCellSimulationViewWidget::on_actionRemove_triggered()
{
    // Ask our graph panels widget to remove the current graph panel

    mContentsWidget->graphPanelsWidget()->removeGraphPanel();
}

//==============================================================================

void SingleCellSimulationViewWidget::on_actionCsvExport_triggered()
{
    // Export our simulation data results to a CSV file

    QString fileName = Core::getSaveFileName(tr("Export to a CSV file"),
                                             QString(),
                                             tr("CSV File")+" (*.csv)");

    if (!fileName.isEmpty())
        mSimulation->results()->exportToCsv(fileName);
}

//==============================================================================

void SingleCellSimulationViewWidget::updateDelayValue(const double &pDelayValue)
{
    // Update our delay value widget

    mDelayValueWidget->setText(QLocale().toString(pDelayValue)+" ms");

    // Also update our simulation object

    if (mSimulation)
        mSimulation->setDelay(pDelayValue);
}

//==============================================================================

void SingleCellSimulationViewWidget::simulationRunning(const bool &pIsResuming)
{
    // Our simulation is running, so do a few things, but only we are dealing
    // with the active simulation

    if (qobject_cast<SingleCellSimulationViewSimulation *>(sender()) == mSimulation) {
        // Reset our local axes' values, if resuming (since the user might have
        // been zooming in/out, etc.)

        if (pIsResuming)
            mActiveGraphPanel->plot()->resetLocalAxes();

        // Update our simulation mode and check for results

        updateSimulationMode();

        checkResults(mSimulation);

        // Prevent interaction with our graph panel's plot

        mActiveGraphPanel->plot()->setInteractive(false);
    }
}

//==============================================================================

void SingleCellSimulationViewWidget::simulationPaused()
{
    // Our simulation is paused, so do a few things, but only we are dealing
    // with the active simulation

    if (qobject_cast<SingleCellSimulationViewSimulation *>(sender()) == mSimulation) {
        // Update our simulation mode and parameters, and check for results

        updateSimulationMode();

        mContentsWidget->informationWidget()->parametersWidget()->updateParameters();

        checkResults(mSimulation);

        // Allow interaction with our graph panel's plot

        mActiveGraphPanel->plot()->setInteractive(true);
    }
}

//==============================================================================

void SingleCellSimulationViewWidget::simulationStopped(const int &pElapsedTime)
{
    // We want a short delay before resetting the progress bar and the file tab
    // icon, so that the user can really see when our simulation has completed

    enum {
        ResetDelay = 169
    };

    // Our simulation worker has stopped, so do a few things, but only we are dealing
    // with the active simulation

    SingleCellSimulationViewSimulation *simulation = qobject_cast<SingleCellSimulationViewSimulation *>(sender());

    if (simulation == mSimulation) {
        // Output the elapsed time, if valid, and reset our progress bar (with a
        // short delay)

        if (pElapsedTime != -1) {
            // We have a valid elapsed time, so show our simulation time

            SingleCellSimulationViewSimulationData *simulationData = mSimulation->data();
            QString solversInformation = QString();

            if (!simulationData->odeSolverName().isEmpty())
                solversInformation += simulationData->odeSolverName();
            else
                solversInformation += simulationData->daeSolverName();

            if (!simulationData->nlaSolverName().isEmpty())
                solversInformation += "+"+simulationData->nlaSolverName();

            output(QString(OutputTab+"<strong>Simulation time:</strong> <span"+OutputInfo+">"+QString::number(0.001*pElapsedTime, 'g', 3)+" s (using "+solversInformation+")</span>."+OutputBrLn));
        }

        QTimer::singleShot(ResetDelay, this, SLOT(resetProgressBar()));

        // Update our parameters and simulation mode

        mContentsWidget->informationWidget()->parametersWidget()->updateParameters();

        updateSimulationMode();

        // Allow interaction with our graph panel's plot

        mActiveGraphPanel->plot()->setInteractive(true);
    }

    // Remove our tracking of our simulation progress and let people know that
    // we should reset our file tab icon, but only if we are the active
    // simulation

    if (simulation) {
        // Stop keeping track of our simulation progress

        mProgresses.remove(simulation->fileName());

        // Reset our file tab icon (with a bit of a delay)
        // Note: we can't directly pass simulation to resetFileTabIcon(), so
        //       instead we use mStoppedSimulations which is a list of
        //       simulations in case several simulations were to stop at around
        //       the same time...

        if (simulation != mSimulation) {
            mStoppedSimulations << simulation;

            QTimer::singleShot(ResetDelay, this, SLOT(resetFileTabIcon()));
        }
    }
}

//==============================================================================

void SingleCellSimulationViewWidget::resetProgressBar()
{
    // Reset our progress bar

    mProgressBarWidget->setValue(0.0);
}

//==============================================================================

void SingleCellSimulationViewWidget::resetFileTabIcon()
{
    // Let people know that we want to reset our file tab icon

    SingleCellSimulationViewSimulation *simulation = mStoppedSimulations.first();

    mStoppedSimulations.removeFirst();

    emit updateFileTabIcon(simulation->fileName(), QIcon());
}

//==============================================================================

void SingleCellSimulationViewWidget::simulationError(const QString &pMessage,
                                                     const ErrorType &pErrorType)
{
    // Output the simulation error, but only if we are dealing with the active
    // simulation

    SingleCellSimulationViewSimulation *simulation = qobject_cast<SingleCellSimulationViewSimulation *>(sender());

    if (!simulation || (simulation == mSimulation)) {
        // Note: we test for simulation to be valid since simulationError() can
        //       also be called directly (as opposed to being called as a
        //       response to a signal) as is done in initialize() above...

        mErrorType = pErrorType;

        updateInvalidModelMessageWidget();

        output(OutputTab+"<span"+OutputBad+"><strong>"+tr("Error:")+"</strong> "+pMessage+".</span>"+OutputBrLn);
    }
}

//==============================================================================

void SingleCellSimulationViewWidget::simulationDataModified(const bool &pIsModified)
{
    // Update our refresh action, but only if we are dealing with the active
    // simulation

    if (qobject_cast<SingleCellSimulationViewSimulationData *>(sender()) == mSimulation->data())
        mGui->actionReset->setEnabled(pIsModified);
}

//==============================================================================

SingleCellSimulationViewSimulation * SingleCellSimulationViewWidget::simulation() const
{
    // Return our (current) simulation

    return mSimulation;
}

//==============================================================================

void SingleCellSimulationViewWidget::setDelayValue(const int &pDelayValue)
{
    // Set the value of our delay widget

    mDelayWidget->setValue(pDelayValue);

    updateDelayValue(pDelayValue);
}

//==============================================================================

SingleCellSimulationViewContentsWidget * SingleCellSimulationViewWidget::contentsWidget() const
{
    // Return our contents widget

    return mContentsWidget;
}

//==============================================================================

void SingleCellSimulationViewWidget::splitterWidgetMoved()
{
    // Our splitter has been moved, so keep track of its new sizes

    mSplitterWidgetSizes = mSplitterWidget->sizes();
}

//==============================================================================

void SingleCellSimulationViewWidget::simulationPropertyChanged(Core::Property *pProperty)
{
    // Update one of our simulation's properties and, if needed, update the
    // minimum or maximum value for our X axis
    // Note #1: with regards to the starting point property, we need to update
    //          it because it's can potentially have an effect on the value of
    //          our 'computed constants' and 'variables'...
    // Note #2: we don't want to waste our time checking our graph panel's
    //          plot's axes everytime we set something, hence our passing false
    //          to our various methods...

    bool needUpdating = true;

    if (pProperty == mContentsWidget->informationWidget()->simulationWidget()->startingPointProperty()) {
        mSimulation->data()->setStartingPoint(Core::PropertyEditorWidget::doublePropertyItem(pProperty->value()));
    } else if (pProperty == mContentsWidget->informationWidget()->simulationWidget()->endingPointProperty()) {
        mSimulation->data()->setEndingPoint(Core::PropertyEditorWidget::doublePropertyItem(pProperty->value()));
    } else if (pProperty == mContentsWidget->informationWidget()->simulationWidget()->pointIntervalProperty()) {
        mSimulation->data()->setPointInterval(Core::PropertyEditorWidget::doublePropertyItem(pProperty->value()));

        needUpdating = false;
    }

    // Update the minimum/maximum (local) values of our axes and replot
    // ourselves, if needed

    if (needUpdating) {
        if (mSimulation->data()->startingPoint() < mSimulation->data()->endingPoint()) {
            mActiveGraphPanel->plot()->setMinX(mSimulation->data()->startingPoint());
            mActiveGraphPanel->plot()->setMaxX(mSimulation->data()->endingPoint());

            mActiveGraphPanel->plot()->setLocalMinX(mActiveGraphPanel->plot()->minX());
            mActiveGraphPanel->plot()->setLocalMaxX(mActiveGraphPanel->plot()->maxX());
        } else {
            mActiveGraphPanel->plot()->setMinX(mSimulation->data()->endingPoint());
            mActiveGraphPanel->plot()->setMaxX(mSimulation->data()->startingPoint());

            mActiveGraphPanel->plot()->setLocalMinX(mActiveGraphPanel->plot()->minX());
            mActiveGraphPanel->plot()->setLocalMaxX(mActiveGraphPanel->plot()->maxX());
        }

        checkAxisY();

        mActiveGraphPanel->plot()->replotNow();
    }
}

//==============================================================================

void SingleCellSimulationViewWidget::solversPropertyChanged(Core::Property *pProperty)
{
    // Check whether any of our NLA solver's properties has been modified and,
    // if so, then update our simulation data object accordingly
    // Note: these are the only solvers propeties property we need to check
    //       because they are the only ones that can potentially have an effect
    //       on the value of 'computed constants' and 'variables'...

    SingleCellSimulationViewInformationSolversWidget *solversWidget = mContentsWidget->informationWidget()->solversWidget();

    if (pProperty == solversWidget->nlaSolverData()->solversListProperty())
        mSimulation->data()->setNlaSolverName(pProperty->value()->text());
    else
        foreach (Core::Property *property, solversWidget->nlaSolverData()->solversProperties().value(mSimulation->data()->nlaSolverName()))
            if (pProperty == property) {
                mSimulation->data()->addNlaSolverProperty(pProperty->name()->text(),
                                                          (pProperty->value()->type() == Core::PropertyItem::Integer)?
                                                              Core::PropertyEditorWidget::integerPropertyItem(pProperty->value()):
                                                              Core::PropertyEditorWidget::doublePropertyItem(pProperty->value()));

                break;
            }
}

//==============================================================================

QString SingleCellSimulationViewWidget::modelParameterKey(const QString pFileName,
                                                          CellMLSupport::CellmlFileRuntimeModelParameter *pModelParameter)
{
    // Return the for the given model parameter

    return pFileName+"|"+QString::number(pModelParameter->type())+"|"+QString::number(pModelParameter->index());
}

//==============================================================================

void SingleCellSimulationViewWidget::showModelParameter(const QString &pFileName,
                                                        CellMLSupport::CellmlFileRuntimeModelParameter *pModelParameter,
                                                        const bool &pShow)
{
    // Determine the key for the given parameter

    QString key = modelParameterKey(pFileName, pModelParameter);

    // Retrieve the curve data associated with the key, if any

    SingleCellSimulationViewWidgetCurveData *curveData = mCurvesData.value(key);

    // Check whether to show/hide a curve

    if (curveData) {
        // We already have a curve, so just make it visible/invisible and update
        // our curve's data, in case we are to make it visible

        if (pShow) {
            curveData->curve()->setRawSamples(mSimulation->results()->points(),
                                              curveData->yData(),
                                              mSimulation->results()->size());

            mActiveGraphPanel->plot()->attach(curveData->curve());
        } else {
            mActiveGraphPanel->plot()->detach(curveData->curve());
        }
    } else if (pShow) {
        // We don't have a curve, but we want one so create one, as well as some
        // data for it

        SingleCellSimulationViewGraphPanelPlotCurve *curve = new SingleCellSimulationViewGraphPanelPlotCurve();
        SingleCellSimulationViewWidgetCurveData *curveData = new SingleCellSimulationViewWidgetCurveData(pFileName, mSimulation, pModelParameter, curve);

        // Set some data for our curve

        curve->setRawSamples(mSimulation->results()->points(),
                             curveData->yData(),
                             mSimulation->results()->size());

        // Attach the curve to our graph panel's plot

        mActiveGraphPanel->plot()->attach(curve);

        // Keep track of our curve data

        mCurvesData.insert(key, curveData);
    }

    // Check our graph panel's plot's local axes before replotting everything
    // Note: we always want to replot, hence our passing false as an argument to
    //       resetLocalAxes()...

    mActiveGraphPanel->plot()->resetLocalAxes(false);
    mActiveGraphPanel->plot()->replotNow();
}

//==============================================================================

void SingleCellSimulationViewWidget::updateResults(SingleCellSimulationViewSimulation *pSimulation,
                                                   const qulonglong &pSize,
                                                   const bool &pReplot)
{
    // Update our curves, if any and only if actually necessary

    SingleCellSimulationViewSimulation *simulation = pSimulation?pSimulation:mSimulation;

    // Our simulation worker has made some progress, so update our progress bar,
    // but only if we are dealing with the active simulation

    if (simulation == mSimulation) {
        // We are dealing with the active simulation, so update our curves and
        // progress bar, and enable/disable the export to CSV

        // Update our curves, if any

        foreach (SingleCellSimulationViewWidgetCurveData *curveData, mCurvesData)
            // Update the curve, should it be attached

            if (curveData->curve()->plot()) {
                // Keep track of our curve's old size

                qulonglong oldDataSize = curveData->curve()->dataSize();

                // Update our curve's data

                curveData->curve()->setRawSamples(mSimulation->results()->points(),
                                                  curveData->yData(),
                                                  pSize);

                // Draw the curve's new segment, but only if there is some data to
                // plot and that we don't want to replot everything

                if (!pReplot && (pSize > 1))
                    mActiveGraphPanel->plot()->drawCurveSegment(curveData->curve(), oldDataSize?oldDataSize-1:0, pSize-1);
            }

        // Replot our active graph panel, if needed

        if (pReplot || (pSize <= 1))
            // We want to initialise the plot and/or there is no data to plot,
            // so replot our active graph panel

            mActiveGraphPanel->plot()->replotNow();

        // Update our progress bar

        mProgressBarWidget->setValue(mSimulation->progress());
    } else {
        // We are dealing with another simulation, so simply create an icon that
        // shows the other simulation's progress and let people know about it
        // Note: we need to retrieve the name of the file associated with the
        //       other simulation since we have only one simulation object at
        //       any given time, and anyone handling the updateFileTabIcon()
        //       signal (e.g. CentralWidget) won't be able to tell for which
        //       simulation the update is...

        int oldProgress = mProgresses.value(simulation->fileName(), -1);
        int newProgress = (tabBarIconSize()-2)*simulation->progress();
        // Note: tabBarIconSize()-2 because we want a one-pixel wide
        //       border...

        if (newProgress != oldProgress) {
            // The progress has changed (or we want to force the updating of a
            // specific simulation), so keep track of its new value and update
            // our file tab icon

            mProgresses.insert(simulation->fileName(), newProgress);

            // Let people know about the file tab icon to be used for the
            // model

            emit updateFileTabIcon(simulation->fileName(),
                                   fileTabIcon(simulation->fileName()));
        }
    }
}

//==============================================================================

void SingleCellSimulationViewWidget::checkResults(SingleCellSimulationViewSimulation *pSimulation)
{
    // Update our simulation results size

    qulonglong simulationResultsSize = pSimulation->results()->size();

    // Update our results, but only if needed

    if (simulationResultsSize != mOldSimulationResultsSizes.value(pSimulation)) {
        mOldSimulationResultsSizes.insert(pSimulation, simulationResultsSize);

        updateResults(pSimulation, simulationResultsSize);
    }

    // Ask to recheck our simulation's results, but only if our simulation is
    // still running

    if (   pSimulation->isRunning()
        || (simulationResultsSize != pSimulation->results()->size())) {
        // Note: we cannot ask QTimer::singleShot() to call checkResults() since
        //       it expects a pointer to a simulation as a parameter, so instead
        //       we call a method with no arguments which will make use of our
        //       list to know which simulation should be passed as an argument
        //       to checkResults()...

        mCheckResultsSimulations << pSimulation;

        QTimer::singleShot(0, this, SLOT(callCheckResults()));
    }
}

//==============================================================================

void SingleCellSimulationViewWidget::callCheckResults()
{
    // Retrieve the simulation for which we want to call checkResults() and then
    // call checkResults()

    SingleCellSimulationViewSimulation *simulation = mCheckResultsSimulations.first();

    mCheckResultsSimulations.removeFirst();

    checkResults(simulation);
}

//==============================================================================

}   // namespace SingleCellSimulationView
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
