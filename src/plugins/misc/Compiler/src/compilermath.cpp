//==============================================================================
// Compiler mathematical functions
//==============================================================================

#include "compilermath.h"
#include "corenlasolver.h"

//==============================================================================

#include <cmath>
#include <cstdarg>
#include <cstdlib>

//==============================================================================

double factorial(double pNb)
{
    double res = 1.0;

    while (pNb > 1.0)
        res *= pNb--;

    return res;
}

//==============================================================================

double arbitrary_log(double pNb, double pBase)
{
    return std::log(pNb)/std::log(pBase);
}

//==============================================================================

double gcdPair(double pNb1, double pNb2)
{
    #define EVEN(pNb) !(pNb & 1)

    int nb1 = std::fabs(pNb1);
    int nb2 = std::fabs(pNb2);

    if (!nb1)
        return nb2;

    if (!nb2)
        return nb1;

    int shift = 0;

    while (EVEN(nb1) && EVEN(nb2)) {
        ++shift;

        nb1 >>= 1;
        nb2 >>= 1;
    }

    do {
        if (EVEN(nb1))
          nb1 >>= 1;
        else if (EVEN(nb2))
            nb2 >>= 1;
        else if (nb1 >= nb2)
            nb1 = (nb1-nb2) >> 1;
        else
            nb2 = (nb2-nb1) >> 1;
    } while (nb1);

    return nb2 << shift;
}

//==============================================================================

double lcmPair(double pNb1, double pNb2)
{
    return (pNb1*pNb2)/gcdPair(pNb1, pNb2);
}

//==============================================================================

double gcd_multi(int pCount, ...)
{
    if (!pCount)
        return 1.0;

    va_list parameters;

    va_start(parameters, pCount);
        double res = va_arg(parameters, double);

        while (--pCount)
            res = gcdPair(res, va_arg(parameters, double));
    va_end(parameters);

    return res;
}

//==============================================================================

double lcm_multi(int pCount, ...)
{
    if (!pCount)
        return 1.0;

    va_list parameters;

    va_start(parameters, pCount);
        double res = va_arg(parameters, double);

        while (--pCount)
            res = lcmPair(res, va_arg(parameters, double));
    va_end(parameters);

    return res;
}

//==============================================================================

double multi_min(int pCount, ...)
{
    if (!pCount)
        return strtod("NAN", 0);

    va_list parameters;

    va_start(parameters, pCount);
        double res = va_arg(parameters, double);
        double otherParameter;

        while (--pCount) {
            otherParameter = va_arg(parameters, double);

            if (otherParameter < res)
                res = otherParameter;
        }
    va_end(parameters);

    return res;
}

//==============================================================================

double multi_max(int pCount, ...)
{
    if (!pCount)
        return strtod("NAN", 0);

    va_list parameters;

    va_start(parameters, pCount);
        double res = va_arg(parameters, double);
        double otherParameter;

        while (--pCount) {
            otherParameter = va_arg(parameters, double);

            if (otherParameter > res)
                res = otherParameter;
        }
    va_end(parameters);

    return res;
}

//==============================================================================

#ifdef Q_OS_WIN
double asinh(double pNb)
{
    return log(pNb+sqrt(pNb*pNb+1));
}
#endif

//==============================================================================

#ifdef Q_OS_WIN
double acosh(double pNb)
{
    return log(pNb+sqrt(pNb*pNb-1));
}
#endif

//==============================================================================

#ifdef Q_OS_WIN
double atanh(double pNb)
{
    return 0.5*(log(1+pNb)-log(1-pNb));
}
#endif

//==============================================================================

void doNonLinearSolve(char *pRuntime,
                      void (*pFunction)(double *, double *, void *),
                      double *pParameters, int *pRes, int pSize,
                      void *pUserData)
{
    // Retrieve the NLA solver which we should use

    OpenCOR::CoreSolver::CoreNlaSolver *nlaSolver = OpenCOR::CoreSolver::nlaSolver(pRuntime);

    if (nlaSolver) {
        // We have found our NLA solver, so initialise it

        nlaSolver->initialize(pFunction, pParameters, pSize, pUserData);

        // Now, we can solve our NLA system

        nlaSolver->solve();

        // Everything went fine, so...

        *pRes = 1;
    } else {
        // We couldn't retrieve an NLA solver, so...
        // Note: this should never happen, but better be safe than sorry, so...

        *pRes = 0;
    }
}

//==============================================================================
// End of file
//==============================================================================
