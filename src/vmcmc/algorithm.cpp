/**
 * @file
 *
 * @date 29.07.2016
 * @author marco@kleesiek.com
 */

#include <vmcmc/algorithm.h>
#include <vmcmc/logger.h>
#include <vmcmc/stringutils.h>
#include <vmcmc/stats.h>

//#include <boost/accumulators/accumulators.hpp>
//#include <boost/accumulators/statistics/mean.hpp>
//#include <boost/accumulators/statistics/variance.hpp>
//#include <boost/accumulators/statistics/stats.hpp>

using namespace std;
using namespace boost;

namespace vmcmc {

LOG_DEFINE("vmcmc.algorithm");

Algorithm::Algorithm() :
    fTotalLength( 1E6 )
{ }

Algorithm::~Algorithm()
{ }

void Algorithm::SetParameterConfig(const ParameterList& paramConfig)
{
    fParameterConfig = paramConfig;
}

bool Algorithm::Initialize()
{
    if (!(fLikelihood || fNegLogLikelihood)) {
        LOG(Error, "No target function specified.");
        return false;
    }

    // TODO: perform consistency checks on the parameter list

    return true;
}

double Algorithm::EvaluatePrior(const std::vector<double>& paramValues) const
{
    if (!fParameterConfig.IsInsideLimits( paramValues ))
        return 0.0;

    return (fPrior) ? fPrior( paramValues ) : 1.0;
}

double Algorithm::EvaluateLikelihood(const std::vector<double>& paramValues) const
{
    LOG_ASSERT(fLikelihood || fNegLogLikelihood, "No target function specified.");

    return (fLikelihood) ? fLikelihood( paramValues ) : exp( -fNegLogLikelihood( paramValues ) );
}

double Algorithm::EvaluateNegLogLikelihood(const std::vector<double>& paramValues) const
{
    LOG_ASSERT(fLikelihood || fNegLogLikelihood, "No target function specified.");

    return (fNegLogLikelihood) ? fNegLogLikelihood( paramValues ) : -log(fLikelihood( paramValues ));
}

bool Algorithm::Evaluate(Sample& sample) const
{
    LOG_ASSERT(fLikelihood || fNegLogLikelihood, "No target function specified.");

    sample.Reset();

    const std::vector<double>& paramValues = sample.Values().data();

    if (!fParameterConfig.IsInsideLimits( sample.Values() ))
        return false;

    const double prior = (fPrior) ? fPrior( paramValues ) : 1.0;
    if (prior == 0.0)
        return false;

    sample.SetPrior( prior );

    if (fLikelihood) {
        const double likelihood = fLikelihood( paramValues );
        sample.SetLikelihood( likelihood );
        sample.SetNegLogLikelihood( -log(likelihood) );
    }
    else {
        const double negLogLikelihood = fNegLogLikelihood( paramValues );
        sample.SetNegLogLikelihood( negLogLikelihood );
        sample.SetLikelihood( exp(-negLogLikelihood) );
    }

    return true;
}

void Algorithm::Run()
{
    if (!Initialize()) {
        LOG(Error, "Initialization failed, aborting.");
        return;
    }

    for (size_t iStep = 0; iStep < fTotalLength; iStep++) {
        Advance();

        if (iStep % (fTotalLength/100) == 0) {
            for (size_t iChain = 0; iChain < NChains(); iChain++) {
                const Sample& sample = GetChain(iChain).back();
                LOG(Debug, "(" << iChain << ") " << iStep << ": " << sample);
            }
        }

    }

    for (size_t iChain = 0; iChain < NChains(); iChain++) {
        const Chain& chain = GetChain(iChain);

        LOG(Info, "Diagnostics for chain " << iChain << ":");

        const double accRate = stats::accRate(chain);

        LOG(Info, "  Acceptance Rate: " << accRate);
    }

    LOG(Info, "MCMC run finished.");
}

} /* namespace vmcmc */
