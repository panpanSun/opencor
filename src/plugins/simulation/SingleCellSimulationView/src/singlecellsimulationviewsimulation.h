//==============================================================================
// Single cell simulation view simulation
//==============================================================================

#ifndef SINGLECELLSIMULATIONVIEW_H
#define SINGLECELLSIMULATIONVIEW_H

//==============================================================================

#include "singlecellsimulationviewsimulationworker.h"

//==============================================================================

#include <QObject>

//==============================================================================

namespace OpenCOR {

//==============================================================================

namespace CellMLSupport {
    class CellmlFileRuntime;
}   // namespace CellMLSuppoer

//==============================================================================

namespace SingleCellSimulationView {

//==============================================================================

class SingleCellSimulationViewSimulationInformationWidget;

//==============================================================================

class SingleCellSimulationViewSimulation : public QObject
{
    Q_OBJECT

public:
    explicit SingleCellSimulationViewSimulation(const QString &pFileName);
    ~SingleCellSimulationViewSimulation();

    void updateFromGui(SingleCellSimulationViewSimulationInformationWidget *pSimulationSettings);
    void updateGui(SingleCellSimulationViewSimulationInformationWidget *pSimulationSettings);

    QString fileName() const;

    CellMLSupport::CellmlFileRuntime * cellmlFileRuntime() const;

    SingleCellSimulationViewSimulationWorker::Status workerStatus() const;
    double workerProgress() const;

    void run();
    void pause();
    void stop();

private:
    QThread *mWorkerThread;
    SingleCellSimulationViewSimulationWorker *mWorker;

    QString mFileName;

    double mStartingPoint;
    double mEndingPoint;
    double mPointInterval;

Q_SIGNALS:
    void running();
    void pausing();
    void stopped(const int &pElapsedTime);

    void progress(const double &pProgress);

    void error(const QString &pMessage);

private Q_SLOTS:
    void finished(const int &pElapsedTime);
};

//==============================================================================

}   // namespace SingleCellSimulationView
}   // namespace OpenCOR

//==============================================================================

#endif

//==============================================================================
// End of file
//==============================================================================
