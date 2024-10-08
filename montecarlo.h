#ifndef MONTECARLO_H
#define MONTECARLO_H

#include <QObject>
#include <QVector>

class MonteCarlo : public QObject
{
    Q_OBJECT
public:
    explicit MonteCarlo(QObject *parent = nullptr);
    void setHistoricalPrices(const QVector<double> &prices);
    QVector<double> runSimulation(int days, bool mostLikely);

private:
    QVector<double> historicalPrices;
    double drift;
    double volatility;
    void calculateParameters();
};

#endif
