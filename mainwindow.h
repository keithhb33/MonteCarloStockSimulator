#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QDateTime>
#include "qcustomplot.h"
#include "montecarlo.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onSimulateButtonClicked();
    void onMostLikelyCheckBoxToggled(bool checked);
    void onSelectionChanged();
    void onMouseMoveInPlot(QMouseEvent *event);
    void onLegendDoubleClick(QCPLegend *legend, QCPAbstractLegendItem *item);
    void onXAxisRangeChanged(const QCPRange &newRange);

private:
    QLineEdit *tickerInput;
    QComboBox *periodComboBox;
    QCheckBox *mostLikelyCheckBox;
    QPushButton *simulateButton;
    QCustomPlot *customPlot;

    MonteCarlo *monteCarlo;

    void plotSimulations(const QVector<QVector<double>> &simulations, const QVector<double> &likelihoods, bool mostLikely);
    void setupUI();
    void setupPlot();

    QVector<double> prices;
    QVector<QDateTime> dates;

    QCPGraph *selectedGraph = nullptr;
    QCPItemTracer *graphTracer = nullptr;

    // Stored data for reuse
    QString lastTicker;
    QVector<QVector<double>> storedSimulations;
    QVector<double> storedLikelihoods;
    int storedHistoricalDays;

    // Variables for dynamic zooming
    QDateTime dataStartDate;
    QDateTime dataEndDate;
    int maxHistoricalDays;
    int maxSimulationDays;

    void fetchHistoricalData(const QDateTime &startDate, const QDateTime &endDate);
    void extendSimulation(int additionalDays);

protected:
    void mouseMoveEvent(QMouseEvent *event) override;
};

#endif // MAINWINDOW_H
