/**
 * @file
 *
 * @copyright Copyright 2016 Marco Kleesiek.
 * Released under the GNU Lesser General Public License v3.
 *
 * @date 29.07.2016
 * @author marco@kleesiek.com
 *
 * @brief Contains the implementation of the popular Metropolis-Hastings
 * sampling algorithm.
 */

#ifndef VMCMC_METROPOLIS_H_
#define VMCMC_METROPOLIS_H_

#include <vmcmc/algorithm.hpp>

#include <memory>

namespace vmcmc
{

class Proposal;

/**
 * Implementation of the Metropolis-Hastings algorithm.
 *
 * The transition kernel alias proposal function can be defined by the user
 * according to the interface of #Proposal. By default, a multivariate
 * Gaussian is used, based on the parameter configuration.
 *
 * Parallel tempering (useful for multimodal likelihoods) can be activated by
 * specifying additional values for beta < 1 (reciprocal temperature),
 * thus constructing flatter distributions. For each value of beta, an
 * individual sampling chain, together with it's own parameter configuration
 * and proposal function is set up.
 */
class MetropolisHastings: public Algorithm
{
public:
    static double CalculateMHRatio(const Sample& prevState, const Sample& nextState,
        double proposalAsymmetry = 1.0, double beta = 1.0);

public:
    MetropolisHastings();
    virtual ~MetropolisHastings();

    virtual void Initialize() override;

    virtual void Advance(size_t nSteps = 1) override;

    virtual void Finalize() override;

    virtual size_t NumberOfChains() override { return fChainConfigs.size(); }
    virtual const Chain& GetChain(size_t cIndex = 0) override;

    void SetNumberOfChains(size_t nChains);

    template <typename ContainerT = std::initializer_list<double>>
    void SetBetas(ContainerT betas);
    const std::vector<double>& GetBetas() const { return fBetas; }

    template <typename ProposalT, typename... ArgsT>
    void SetProposalFunction(ArgsT&&... args);
    void SetProposalFunction(std::shared_ptr<Proposal> proposalFunction) { fProposalFunction = proposalFunction; }
    std::shared_ptr<Proposal> GetProposalFunction() { return fProposalFunction; };
    std::shared_ptr<const Proposal> GetProposalFunction() const { return fProposalFunction; }

    void SetRandomizeStartPoint(bool randomizeStartPoint) { fRandomizeStartPoint = randomizeStartPoint; }
    bool IsRandomizeStartPoint() const { return fRandomizeStartPoint; }

    void SetMultiThreading(bool enable);
    bool IsMultiThreading() const { return fMultiThreading; }

    /**
     * Get the fraction of accepted swaps between tempered chains.
     * @param iChain Index of the chain set.
     * @param iBeta Index of the tempered chain pair. If < 0, average over all pairs.
     * @return
     */
    double GetSwapAcceptanceRate(size_t iChain, ptrdiff_t iBeta = -1) const;

protected:
    void AdvanceChainConfig(size_t iChainConfig, size_t iBeta, size_t nSteps = 1);
    void ProposePtSwapping(size_t iChainConfig);

    bool fRandomizeStartPoint;

    std::vector<double> fBetas;
    std::shared_ptr<Proposal> fProposalFunction;

    size_t fPtFrequency;

    struct ChainConfig;
    std::vector<std::unique_ptr<ChainConfig>> fChainConfigs;

private:
    bool fMultiThreading;
};

template <typename ProposalT, typename... ArgsT>
inline void MetropolisHastings::SetProposalFunction(ArgsT&&... args)
{
    fProposalFunction = std::make_shared<ProposalT>(
        std::forward<ArgsT>(args)...
    );
}

template <typename ContainerT>
inline void MetropolisHastings::SetBetas(ContainerT betas)
{
    // make sure, that the default chain (0) has nominal temperature
    fBetas = { 1.0 };

    // only add betas < 1.0 (higher temperature)
    std::copy_if( betas.begin(), betas.end(),
        std::back_inserter(fBetas), [](double beta){ return beta < 1.0 && beta > 0.0; } );

    // and sort in descending order
    std::sort(fBetas.begin(), fBetas.end(), std::greater<double>());
}

} /* namespace vmcmc */

#endif /* VMCMC_METROPOLIS_H_ */
