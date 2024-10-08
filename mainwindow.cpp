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
    connect(customPlot, &QCustomPlot::selectionChangedByUser, this, &MainWindow::onSelectionChanged);
    connect(customPlot, &QCustomPlot::mouseMove, this, &MainWindow::onMouseMoveInPlot);
    connect(customPlot, &QCustomPlot::legendDoubleClick, this, &MainWindow::onLegendDoubleClick);
    connect(mostLikelyCheckBox, &QCheckBox::toggled, this, &MainWindow::onMostLikelyCheckBoxToggled);
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
    lastTicker = "";
    storedSimulations.clear();
    storedLikelihoods.clear();
    storedHistoricalDays = 0;
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
    customPlot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop|Qt::AlignLeft);
    customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables | QCP::iSelectLegend);
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
    customPlot->graph(0)->setSelectable(QCP::stSingleData);
    customPlot->graph(0)->setSelectionDecorator(new QCPSelectionDecorator());
    monteCarlo->setHistoricalPrices(limitedPrices);
    int days = historicalDays;
    int numSimulations = 10;
    QVector<QVector<double>> simulations;
    QVector<double> likelihoods;
    monteCarlo->runSimulations(days, numSimulations, simulations, likelihoods);
    storedSimulations = simulations;
    storedLikelihoods = likelihoods;
    lastTicker = ticker;
    storedHistoricalDays = historicalDays;
    this->dates = limitedDates;
    this->prices = limitedPrices;
    plotSimulations(simulations, likelihoods, mostLikely);
    customPlot->rescaleAxes();
    customPlot->replot();
}
void MainWindow::plotSimulations(const QVector<QVector<double>> &simulations, const QVector<double> &likelihoods, bool mostLikely)
{
    QDateTime lastDate = dates.last();
    if (mostLikely)
    {
        int maxIndex = std::distance(likelihoods.begin(), std::max_element(likelihoods.begin(), likelihoods.end()));
        QVector<double> mostLikelySimulation = simulations[maxIndex];
        QVector<double> simTimeValues;
        for (int i = 0; i < mostLikelySimulation.size(); ++i) {
            QDateTime simDate = lastDate.addDays(i + 1);
            simTimeValues.append(simDate.toSecsSinceEpoch());
        }
        customPlot->addGraph();
        customPlot->graph()->setPen(QPen(Qt::red));
        customPlot->graph()->setName("Most Likely Simulation");
        customPlot->graph()->setData(simTimeValues, mostLikelySimulation);
        customPlot->graph()->setSelectable(QCP::stSingleData);
        customPlot->graph()->setSelectionDecorator(new QCPSelectionDecorator());
    }
    else
    {
        for (int n = 0; n < simulations.size(); ++n)
        {
            QVector<double> simTimeValues;
            QVector<double> simulationPrices = simulations[n];
            for (int i = 0; i < simulationPrices.size(); ++i) {
                QDateTime simDate = lastDate.addDays(i + 1);
                simTimeValues.append(simDate.toSecsSinceEpoch());
            }
            customPlot->addGraph();
            QPen pen;
            pen.setColor(QColor::fromHsv((n * 255) / simulations.size(), 255, 200));
            customPlot->graph()->setPen(pen);
            customPlot->graph()->setName(QString("Simulation %1").arg(n + 1));
            customPlot->graph()->setData(simTimeValues, simulationPrices);
            customPlot->graph()->setSelectable(QCP::stSingleData);
            customPlot->graph()->setSelectionDecorator(new QCPSelectionDecorator());
        }
    }
    customPlot->xAxis->setRange(dates.first().toSecsSinceEpoch(), lastDate.addDays(simulations[0].size()).toSecsSinceEpoch());
    customPlot->yAxis->rescale();
    customPlot->replot();
}
void MainWindow::onMostLikelyCheckBoxToggled(bool checked)
{
    Q_UNUSED(checked);
    QString ticker = tickerInput->text().trimmed();
    if (!storedSimulations.isEmpty() && ticker == lastTicker)
    {
        customPlot->clearGraphs();
        QVector<double> timeValues;
        for (const QDateTime &date : dates) {
            timeValues.append(date.toSecsSinceEpoch());
        }
        customPlot->addGraph();
        customPlot->graph(0)->setPen(QPen(Qt::blue));
        customPlot->graph(0)->setName("Historical Data");
        customPlot->graph(0)->setData(timeValues, prices);
        customPlot->graph(0)->setSelectable(QCP::stSingleData);
        customPlot->graph(0)->setSelectionDecorator(new QCPSelectionDecorator());
        bool mostLikely = mostLikelyCheckBox->isChecked();
        plotSimulations(storedSimulations, storedLikelihoods, mostLikely);
        customPlot->rescaleAxes();
        customPlot->replot();
    }
    else
    {
    }
}
void MainWindow::onSelectionChanged()
{
    bool graphSelected = false;
    for (int i = 0; i < customPlot->graphCount(); ++i)
    {
        QCPGraph *graph = customPlot->graph(i);
        if (!graph->selection().isEmpty()) // Corrected line
        {
            graphSelected = true;
            selectedGraph = graph;
            break;
        }
    }
    if (!graphSelected)
    {
        selectedGraph = nullptr;
        if (graphTracer)
        {
            customPlot->removeItem(graphTracer);
            graphTracer = nullptr;
            customPlot->replot();
        }
    }
    else
    {
        if (!graphTracer)
        {
            graphTracer = new QCPItemTracer(customPlot);
            graphTracer->setGraph(selectedGraph);
            graphTracer->setInterpolating(true);
            graphTracer->setStyle(QCPItemTracer::tsCircle);
            graphTracer->setPen(QPen(Qt::red));
            graphTracer->setBrush(Qt::red);
            graphTracer->setSize(7);
        }
        else
        {
            graphTracer->setGraph(selectedGraph);
        }
    }
}
void MainWindow::onMouseMoveInPlot(QMouseEvent *event)
{
    if (selectedGraph && graphTracer)
    {
        double x = customPlot->xAxis->pixelToCoord(event->pos().x());
        graphTracer->setGraphKey(x);
        double y = graphTracer->position->value();
        QDateTime date = QDateTime::fromSecsSinceEpoch(static_cast<qint64>(x));
        QString dateStr = date.toString("MMM dd yyyy");
        QString tooltipText = QString("Date: %1\nPrice: $%2")
                                  .arg(dateStr)
                                  .arg(y, 0, 'f', 2);
        QToolTip::showText(customPlot->mapToGlobal(event->pos()), tooltipText, customPlot);
        customPlot->replot();
    }
    else
    {
        QToolTip::hideText();
    }
}
void MainWindow::onLegendDoubleClick(QCPLegend *legend, QCPAbstractLegendItem *item)
{
    Q_UNUSED(legend)
    if (item)
    {
        QCPPlottableLegendItem *plItem = qobject_cast<QCPPlottableLegendItem *>(item);
        if (plItem)
        {
            QCPGraph *graph = qobject_cast<QCPGraph *>(plItem->plottable());
            if (graph)
            {
                customPlot->deselectAll();
                graph->setSelection(QCPDataSelection(graph->data()->dataRange())); // Corrected line
                onSelectionChanged();
                customPlot->replot();
            }
        }
    }
}
void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    QMainWindow::mouseMoveEvent(event);
}
