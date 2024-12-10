#ifndef RANDOM_GENERATOR_H
#define RANDOM_GENERATOR_H
#include <cstdlib>
#include <vector>
#include <random>

namespace ns3
{

/**
 * \ingroup applications
 * Modelize a generation method.
 */
class RandomGenerator 
{
public:

    virtual ~RandomGenerator() = default;

    virtual double GetRandom() const = 0;

};

/**
 * \ingroup applications
 * Simple generator using basic statistical values (min, max, mean, stdDev).
 */
class RandomGeneratorRv : public RandomGenerator 
{
public:

    RandomGeneratorRv(double min, double max, double mean, double stdDev);

    virtual ~RandomGeneratorRv() = default;

    double GetRandom() const override;

private:
    double m_min, m_max, m_mean, m_stdDev;
};

/**
 * \ingroup applications
 * Simple generator using basic statistical values (min, max, mean, stdDev).
 */
class RandomGeneratorDist : public RandomGenerator 
{
public:

    RandomGeneratorDist(const std::vector<std::pair<double, double>>& distribution);

    virtual ~RandomGeneratorDist() = default;

    double GetRandom() const override;

private:
    // Random number generator
    mutable std::default_random_engine m_rng;

    std::vector<std::pair<double, double>> m_distribution;
    mutable std::discrete_distribution<> m_discreteDistributionObj;


};

/**
 * \ingroup applications
 * Simple generator using basic statistical values (min, max, mean, stdDev).
 */
class RandomGeneratorNormal : public RandomGenerator 
{
public:

    RandomGeneratorNormal(double min, double max, double mean, double stdDev);

    virtual ~RandomGeneratorNormal() = default;

    double GetRandom() const override;

private:
    double m_min, m_max, m_mean, m_stdDev;
};
} // namespace ns3

#endif /* RANDOM_GENERATOR_H */
