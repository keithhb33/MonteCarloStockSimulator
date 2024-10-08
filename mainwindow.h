#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
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

private:
    QLineEdit *tickerInput;
    QComboBox *periodComboBox;
    QCheckBox *mostLikelyCheckBox;
    QPushButton *simulateButton;
    QCustomPlot *customPlot;

    MonteCarlo *monteCarlo;

    void plotSimulation(const QVector<double> &simulationPrices);
    void setupUI();
    void setupPlot();

    QVector<double> prices;
    QVector<QDateTime> dates;

protected:
    void mouseMoveEvent(QMouseEvent *event) override;
};

#endif
