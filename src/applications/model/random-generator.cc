#include "random-generator.h"

namespace ns3 
{

RandomGeneratorUniform::RandomGeneratorUniform(double min, double max)
    : m_min(min), m_max(max)
{
}

double
RandomGeneratorUniform::GetRandom() const
{
    double random = m_min + (rand() / (double)RAND_MAX) * (m_max - m_min);
    return random;
}

RandomGeneratorDist::RandomGeneratorDist(
    const std::vector<std::pair<double, double>>& distribution)
    : m_distribution(distribution)
{
    // Initialize packet size distribution
    std::vector<double> probabilities;
    for (const auto& pair : distribution) {
        probabilities.push_back(pair.second);
    }
    m_discreteDistributionObj = std::discrete_distribution<>(
        probabilities.begin(), probabilities.end());

}

double
RandomGeneratorDist::GetRandom() const
{
    int index = m_discreteDistributionObj(m_rng);
    return m_distribution[index].first;
}

RandomGeneratorNormal::RandomGeneratorNormal(double min, double max, double mean, double stdDev)
    : m_min(min), m_max(max), m_mean(mean), m_stdDev(stdDev)
{
}

double
RandomGeneratorNormal::GetRandom() const
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> d(m_mean, m_stdDev);

    double randomValue = d(gen);

    if (randomValue < m_min) return m_min;
    if (randomValue > m_max) return m_max;

    return randomValue;
}

} // namespace ns3
