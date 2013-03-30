//==============================================================================
// CVODE solver class
//==============================================================================

#ifndef CVODESOLVER_H
#define CVODESOLVER_H

//==============================================================================

#include "coreodesolver.h"

//==============================================================================

#include "nvector/nvector_serial.h"

//==============================================================================

namespace OpenCOR {
namespace CVODESolver {

//==============================================================================

static const QString MaximumStepProperty = "Maximum step";
static const QString MaximumNumberOfStepsProperty = "Maximum number of steps";
static const QString RelativeToleranceProperty = "Relative tolerance";
static const QString AbsoluteToleranceProperty = "Absolute tolerance";

//==============================================================================

// Default CVODE parameter values
// Note #1: a maximum step of 0 means that there is no maximum step as such and
//          that CVODE can use whatever step it sees fit...
// Note #2: CVODE's default maximum number of steps is 500 which ought to be big
//          enough in most cases...

static const double DefaultMaximumStep = 0.0;

enum {
    DefaultMaximumNumberOfSteps = 500
};

static const double DefaultRelativeTolerance = 1.0e-7;
static const double DefaultAbsoluteTolerance = 1.0e-7;

//==============================================================================

class CvodeSolverUserData
{
public:
    explicit CvodeSolverUserData(double *pConstants, double *pAlgebraic,
                                 CoreSolver::CoreOdeSolver::ComputeRatesFunction pComputeRates);

    double * constants() const;
    double * algebraic() const;

    CoreSolver::CoreOdeSolver::ComputeRatesFunction computeRates() const;

private:
    double *mConstants;
    double *mAlgebraic;

    CoreSolver::CoreOdeSolver::ComputeRatesFunction mComputeRates;
};

//==============================================================================

class CvodeSolver : public CoreSolver::CoreOdeSolver
{
public:
    explicit CvodeSolver();
    ~CvodeSolver();

    virtual void initialize(const double &pVoiStart, const int &pStatesCount,
                            double *pConstants, double *pStates, double *pRates,
                            double *pAlgebraic, ComputeRatesFunction pComputeRates);

    virtual void solve(double &pVoi, const double &pVoiEnd) const;

private:
    void *mSolver;
    N_Vector mStatesVector;
    CvodeSolverUserData *mUserData;

    double mMaximumStep;
    int mMaximumNumberOfSteps;
    double mRelativeTolerance;
    double mAbsoluteTolerance;
};

//==============================================================================

}   // namespace CVODESolver
}   // namespace OpenCOR

//==============================================================================

#endif

//==============================================================================
// End of file
//==============================================================================
