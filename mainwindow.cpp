#include "mainwindow.h"
#include "montecarlo.h"
#include "qcustomplot.h"

#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDir>
#include <QProcess>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QDebug>
#include <algorithm>
#include <QDateTime>
#include <QMouseEvent>
#include <QToolTip>
#include <limits>
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    monteCarlo = new MonteCarlo(this);

    setupUI();
    setupPlot();

    connect(simulateButton, &QPushButton::clicked, this, &MainWindow::onSimulateButtonClicked);

    setMouseTracking(true);
    customPlot->setMouseTracking(true);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    tickerInput = new QLineEdit(this);
    tickerInput->setPlaceholderText("Enter Stock Ticker");

    periodComboBox = new QComboBox(this);
    periodComboBox->addItems({"1 Month", "6 Months", "1 Year", "2 Years"});

    mostLikelyCheckBox = new QCheckBox("Most Likely Outcome", this);

    simulateButton = new QPushButton("Simulate", this);

    QHBoxLayout *inputLayout = new QHBoxLayout();
    inputLayout->addWidget(tickerInput);
    inputLayout->addWidget(periodComboBox);
    inputLayout->addWidget(mostLikelyCheckBox);
    inputLayout->addWidget(simulateButton);

    customPlot = new QCustomPlot(this);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addLayout(inputLayout);
    mainLayout->addWidget(customPlot);

    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);
}

void MainWindow::setupPlot()
{
    QSharedPointer<QCPAxisTickerDateTime> dateTicker(new QCPAxisTickerDateTime);
    dateTicker->setDateTimeFormat("MMM dd yyyy");
    customPlot->xAxis->setTicker(dateTicker);

    customPlot->xAxis->setLabel("Date");
    customPlot->yAxis->setLabel("Stock Price");
    customPlot->legend->setVisible(true);
    customPlot->legend->setBrush(QBrush(QColor(255,255,255,230)));

    customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
}

void MainWindow::onSimulateButtonClicked()
{
    QString ticker = tickerInput->text().trimmed();
    QString period = periodComboBox->currentText();
    bool mostLikely = mostLikelyCheckBox->isChecked();

    if (ticker.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please enter a stock ticker.");
        return;
    }

    QString scriptPath = QDir::currentPath() + "/fetch_data.py";
    QStringList arguments;
    arguments << scriptPath << ticker;

    QProcess process;
    process.start("python3", arguments);
    if (!process.waitForStarted()) {
        QMessageBox::critical(this, "Error", "Failed to start the Python script.");
        return;
    }

    process.waitForFinished(-1);

    QString output = process.readAllStandardOutput();
    QString errorOutput = process.readAllStandardError();

    qDebug() << "Python script output:" << output;
    qDebug() << "Python script error output:" << errorOutput;

    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        QMessageBox::critical(this, "Error", "Failed to fetch data.\n" + errorOutput);
        return;
    }

    QFile file("stock_data.csv");
    if (!file.exists()) {
        QMessageBox::critical(this, "Error", "Data file not found.");
        return;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Failed to open data file.");
        return;
    }

    prices.clear();
    dates.clear();
    QTextStream in(&file);
    QString header = in.readLine();
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList fields = line.split(',');

        if (fields.size() > 5) {
            QString dateString = fields.at(0);
            QDateTime date = QDateTime::fromString(dateString, "yyyy-MM-dd");
            if (!date.isValid()) {
                qDebug() << "Invalid date format:" << dateString;
                continue;
            }

            bool ok;
            double closePrice = fields.at(4).toDouble(&ok);
            if (!ok) {
                qDebug() << "Failed to convert price to double:" << fields.at(4);
                continue;
            }

            dates.append(date);
            prices.append(closePrice);
        }
    }
    file.close();

    if (prices.isEmpty() || dates.isEmpty()) {
        QMessageBox::warning(this, "Data Error", "No price data available.");
        return;
    }

    QList<QPair<QDateTime, double>> dataList;
    for (int i = 0; i < dates.size(); ++i) {
        dataList.append(qMakePair(dates[i], prices[i]));
    }

    std::sort(dataList.begin(), dataList.end(), [](const QPair<QDateTime, double> &a, const QPair<QDateTime, double> &b) {
        return a.first < b.first;
    });

    dates.clear();
    prices.clear();
    for (const auto &pair : dataList) {
        dates.append(pair.first);
        prices.append(pair.second);
    }

    int historicalDays = 30;
    if (period == "1 Month")
        historicalDays = 30;
    else if (period == "6 Months")
        historicalDays = 180;
    else if (period == "1 Year")
        historicalDays = 365;
    else if (period == "2 Years")
        historicalDays = 730;

    QVector<QDateTime> limitedDates;
    QVector<double> limitedPrices;

    QDateTime cutoffDate = dates.last().addDays(-historicalDays);
    for (int i = dates.size() - 1; i >= 0; --i)
    {
        if (dates[i] >= cutoffDate)
        {
            limitedDates.prepend(dates[i]);
            limitedPrices.prepend(prices[i]);
        }
        else
        {
            break;
        }
    }

    if (limitedPrices.isEmpty() || limitedDates.isEmpty()) {
        QMessageBox::warning(this, "Data Error", "Not enough historical data available.");
        return;
    }

    QVector<double> timeValues;
    for (const QDateTime &date : limitedDates) {
        timeValues.append(date.toSecsSinceEpoch());
    }

    customPlot->clearGraphs();

    customPlot->addGraph();
    customPlot->graph(0)->setPen(QPen(Qt::blue));
    customPlot->graph(0)->setName("Historical Data");
    customPlot->graph(0)->setData(timeValues, limitedPrices);

    monteCarlo->setHistoricalPrices(limitedPrices);

    int days = historicalDays;

    QVector<double> simulation = monteCarlo->runSimulation(days, mostLikely);

    plotSimulation(simulation);

    this->dates = dates;
    this->prices = prices;

    customPlot->rescaleAxes();
    customPlot->replot();
}

void MainWindow::plotSimulation(const QVector<double> &simulationPrices)
{
    QVector<double> simTimeValues;
    QDateTime lastDate = dates.last();

    for (int i = 0; i < simulationPrices.size(); ++i) {
        QDateTime simDate = lastDate.addDays(i + 1);
        simTimeValues.append(simDate.toSecsSinceEpoch());
    }

    customPlot->addGraph();
    customPlot->graph(1)->setPen(QPen(Qt::red));
    customPlot->graph(1)->setName("Simulation");

    customPlot->graph(1)->setData(simTimeValues, simulationPrices);

    customPlot->xAxis->setRange(simTimeValues.first(), simTimeValues.last());

    customPlot->yAxis->rescale();

    customPlot->replot();
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    QPoint mousePos = event->pos();

    if (customPlot->rect().contains(customPlot->mapFromParent(mousePos)))
    {
        double x = customPlot->xAxis->pixelToCoord(customPlot->mapFromParent(mousePos).x());
        double y = customPlot->yAxis->pixelToCoord(customPlot->mapFromParent(mousePos).y());

        bool foundDataPoint = false;
        double closestDistance = std::numeric_limits<double>::max();
        double closestX = 0, closestY = 0;
        QString dataLabel;

        for (int i = 0; i < customPlot->graphCount(); ++i)
        {
            QCPGraph *graph = customPlot->graph(i);
            if (!graph)
                continue;

            QSharedPointer<QCPGraphDataContainer> dataContainer = graph->data();
            for (auto it = dataContainer->constBegin(); it != dataContainer->constEnd(); ++it)
            {
                double dataX = it->key;
                double dataY = it->value;
                double dx = dataX - x;
                double dy = dataY - y;
                double distance = std::sqrt(dx * dx + dy * dy);

                if (distance < closestDistance)
                {
                    closestDistance = distance;
                    closestX = dataX;
                    closestY = dataY;
                    foundDataPoint = true;

                    if (i == 0)
                        dataLabel = "Historical Data";
                    else if (i == 1)
                        dataLabel = "Simulation";
                }
            }
        }

        if (foundDataPoint && closestDistance < 10)
        {
            QDateTime date = QDateTime::fromSecsSinceEpoch(static_cast<qint64>(closestX));
            QString dateStr = date.toString("MMM dd yyyy");
            QString tooltipText = QString("%1\nDate: %2\nPrice: $%3")
                                      .arg(dataLabel)
                                      .arg(dateStr)
                                      .arg(closestY, 0, 'f', 2);

            QToolTip::showText(event->globalPos(), tooltipText, customPlot);
        }
        else
        {
            QToolTip::hideText();
        }
    }
    else
    {
        QToolTip::hideText();
    }

    QMainWindow::mouseMoveEvent(event);
}
