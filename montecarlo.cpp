#include "montecarlo.h"
#include <cmath>
#include <random>

MonteCarlo::MonteCarlo(QObject *parent) : QObject(parent)
{
}

void MonteCarlo::setHistoricalPrices(const QVector<double> &prices)
{
    historicalPrices = prices;
    calculateParameters();
}

void MonteCarlo::calculateParameters()
{
    QVector<double> logReturns;
    for (int i = 1; i < historicalPrices.size(); ++i) {
        double ret = log(historicalPrices[i] / historicalPrices[i - 1]);
        logReturns.append(ret);
    }

    double sum = 0;
    for (double val : logReturns) sum += val;
    double mean = sum / logReturns.size();

    double variance = 0;
    for (double val : logReturns) variance += pow(val - mean, 2);
    variance /= logReturns.size();

    drift = mean - (variance / 2);
    volatility = sqrt(variance);
}

QVector<double> MonteCarlo::runSimulation(int days, bool mostLikely)
{
    QVector<double> simulation;
    simulation.append(historicalPrices.last());
    std::default_random_engine generator;
    std::normal_distribution<double> distribution(0.0, 1.0);

    for (int i = 1; i < days; ++i) {
        double randomShock = mostLikely ? 0 : distribution(generator);
        double price = simulation.last() * exp(drift + volatility * randomShock);
        simulation.append(price);
    }

    return simulation;
}
