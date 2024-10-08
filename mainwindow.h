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
    void onSelectionChanged();
    void onMouseMoveInPlot(QMouseEvent *event);
    void onLegendDoubleClick(QCPLegend *legend, QCPAbstractLegendItem *item);
    void onMostLikelyCheckBoxToggled(bool checked);
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
    QString lastTicker;
    QVector<QVector<double>> storedSimulations;
    QVector<double> storedLikelihoods;
    int storedHistoricalDays;
protected:
    void mouseMoveEvent(QMouseEvent *event) override;
};
#endif
