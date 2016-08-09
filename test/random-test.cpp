/*
 * random-test.cpp
 *
 *  Created on: 24.07.2016
 *      Author: marco@kleesiek.com
 */

#include <fmcmc/random.h>
#include <fmcmc/ublas.h>
#include <fmcmc/stringutils.h>

#include <thread>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>

#include <gtest/gtest.h>

using namespace std;
using namespace fmcmc;
using namespace boost;
using namespace boost::accumulators;

TEST(random_test, singleton_initialization) {
    Random& rand = Random::Instance();
    rand.SetSeed(1);

    ASSERT_EQ(rand.Uniform(0, 100), 42) << "Deterministic random"
            "number generator with unexpected result.";

    ASSERT_DOUBLE_EQ(rand.Uniform(-99.0, +99.0), 43.624248925001041);

    ublas::vector<double> mean(100, 0.0);
}

TEST(random_test, uniform_multithreaded) {
    Random::Instance().SetSeed(1);

    constexpr size_t sNThreads = 4;

    thread myThreads[sNThreads];
    for (int t = 0; t < sNThreads; ++t) {
        myThreads[t] = thread( []() {

            accumulator_set<double, stats<tag::mean> > acc;

            for (int i = 0; i < 100000; ++i)
                acc( Random::Instance().Uniform(0.0, 10.0) );

            ASSERT_NEAR(mean(acc), 5.0, 0.05);
        });
    }

    for (int t = 0; t < sNThreads; ++t) {
        myThreads[t].join();
    }
}

TEST(random_test, multivariate_normal) {
    Random& rand = Random::Instance();
    rand.SetSeed(1);

    constexpr size_t N = 5;

    ublas::triangular_matrix<double> cov(N, N);
    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < N; ++j) {
            if (i == j)
                cov(i, j) = 1.0 * (double) (i+1);
            else if (i > j)
                cov(i, j) = 0.0;
        }
    }
    cov(1, 0) = 0.8;
    cov(3, 2) = -1.0;
    cov(4, 3) = 3.0;

    ublas::triangular_matrix<double> cholesky(N, N);
    ASSERT_EQ( choleskyDecompose(cov, cholesky), 0) << "Cholesky decomposition failed.";

    // TODO: replace boost accumulators by own statistics tools
    accumulator_set<double, stats<tag::covariance<double, tag::covariate1> > > acc01, acc12, acc23, acc34;

    const ublas::vector<double> mean(N, 5.0);

    for (int i = 0; i < 1000; ++i) {
        ublas::vector<double> rVector = rand.GaussianMultiVariate(mean, cholesky);
        acc01(rVector(0), covariate1 = rVector(1));
        acc12(rVector(1), covariate1 = rVector(2));
        acc23(rVector(2), covariate1 = rVector(3));
        acc34(rVector(3), covariate1 = rVector(4));
    }

    ASSERT_NEAR(covariance(acc01), 0.8, 0.16);
    ASSERT_NEAR(covariance(acc12), 0.0, 0.1);
    ASSERT_NEAR(covariance(acc23), -1.0, 0.2);
    ASSERT_NEAR(covariance(acc34), 3.0, 0.6);

}