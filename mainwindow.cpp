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

    // Connect selection change signal
    connect(customPlot, &QCustomPlot::selectionChangedByUser, this, &MainWindow::onSelectionChanged);

    // Connect mouse move signal
    connect(customPlot, &QCustomPlot::mouseMove, this, &MainWindow::onMouseMoveInPlot);

    // Connect legend interactions
    connect(customPlot, &QCustomPlot::legendDoubleClick, this, &MainWindow::onLegendDoubleClick);

    // Connect the checkbox toggled signal
    connect(mostLikelyCheckBox, &QCheckBox::toggled, this, &MainWindow::onMostLikelyCheckBoxToggled);

    // Corrected connect statement for xAxis rangeChanged signal
    connect(customPlot->xAxis, static_cast<void(QCPAxis::*)(const QCPRange&)>(&QCPAxis::rangeChanged),
            this, &MainWindow::onXAxisRangeChanged);

    setMouseTracking(true);
    customPlot->setMouseTracking(true);

    // Initialize variables
    dataStartDate = QDateTime();
    dataEndDate = QDateTime();
    maxHistoricalDays = 0;
    maxSimulationDays = 0;
    lastTicker = "";
    storedSimulations.clear();
    storedLikelihoods.clear();
    storedHistoricalDays = 0;
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
    customPlot->legend->setBrush(QBrush(QColor(255, 255, 255, 230)));

    // Move the legend to the top-left corner
    customPlot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop | Qt::AlignLeft);

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
                continue;
            }

            bool ok;
            double closePrice = fields.at(4).toDouble(&ok);
            if (!ok) {
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

    // Plot historical data
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

    // Run multiple simulations
    monteCarlo->runSimulations(days, numSimulations, simulations, likelihoods);

    // Store simulations and relevant data
    storedSimulations = simulations;
    storedLikelihoods = likelihoods;
    lastTicker = ticker;
    storedHistoricalDays = historicalDays;

    this->dates = limitedDates;
    this->prices = limitedPrices;

    dataStartDate = dates.first();
    dataEndDate = dates.last();
    maxHistoricalDays = dates.size();
    maxSimulationDays = days;

    // Plot simulations
    plotSimulations(simulations, likelihoods, mostLikely);

    customPlot->rescaleAxes();
    customPlot->replot();
}

void MainWindow::plotSimulations(const QVector<QVector<double>> &simulations, const QVector<double> &likelihoods, bool mostLikely)
{
    // Remove existing simulation graphs (graphs from index 1 onwards)
    int graphCount = customPlot->graphCount();
    // Assuming the historical data graph is at index 0
    for (int i = graphCount - 1; i > 0; --i)
    {
        customPlot->removeGraph(i);
    }

    QDateTime lastDate = dates.last();

    if (mostLikely)
    {
        // Find the simulation with the highest likelihood
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
        // Plot all simulations
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

    customPlot->xAxis->setRange(dataStartDate.toSecsSinceEpoch(), lastDate.addDays(maxSimulationDays).toSecsSinceEpoch());
    customPlot->yAxis->rescale();
    customPlot->replot();
}

void MainWindow::onMostLikelyCheckBoxToggled(bool checked)
{
    Q_UNUSED(checked);

    QString ticker = tickerInput->text().trimmed();

    // Check if simulations are available for the current ticker
    if (!storedSimulations.isEmpty() && ticker == lastTicker)
    {
        customPlot->clearGraphs();

        // Plot historical data
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

        // Re-plot simulations based on the new checkbox state
        bool mostLikely = mostLikelyCheckBox->isChecked();
        plotSimulations(storedSimulations, storedLikelihoods, mostLikely);

        customPlot->rescaleAxes();
        customPlot->replot();
    }
}

void MainWindow::onSelectionChanged()
{
    // Check if a graph is selected
    bool graphSelected = false;
    for (int i = 0; i < customPlot->graphCount(); ++i)
    {
        QCPGraph *graph = customPlot->graph(i);
        if (!graph->selection().isEmpty())
        {
            graphSelected = true;
            selectedGraph = graph;
            break;
        }
    }

    if (!graphSelected)
    {
        // No graph is selected
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
        // A graph is selected
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

        // Set the tracer's position
        graphTracer->setGraphKey(x);

        // Get the tracer's position in plot coordinates
        double y = graphTracer->position->value();

        // Convert x back to QDateTime
        QDateTime date = QDateTime::fromSecsSinceEpoch(static_cast<qint64>(x));
        QString dateStr = date.toString("MMM dd yyyy");
        QString tooltipText = QString("Date: %1\nPrice: $%2")
                                  .arg(dateStr)
                                  .arg(y, 0, 'f', 2);

        // Show the tooltip near the cursor
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
                // Select the graph by selecting all its data points
                customPlot->deselectAll();
                graph->setSelection(QCPDataSelection(graph->data()->dataRange()));
                onSelectionChanged();
                customPlot->replot();
            }
        }
    }
}

void MainWindow::onXAxisRangeChanged(const QCPRange &newRange)
{
    // Convert axis range to dates
    QDateTime newStartDate = QDateTime::fromSecsSinceEpoch(static_cast<qint64>(newRange.lower));
    QDateTime newEndDate = QDateTime::fromSecsSinceEpoch(static_cast<qint64>(newRange.upper));

    bool dataUpdated = false;

    // Ensure the new start date is valid and before the end date
    if (newStartDate < dataStartDate)
    {
        if (newStartDate >= dataEndDate)
        {
            qDebug() << "Invalid date range: Start date is after end date.";
            return;
        }

        // Fetch more historical data
        fetchHistoricalData(newStartDate, dataStartDate);
        dataStartDate = newStartDate;
        dataUpdated = true;
    }

    // Check if we need to extend the simulation
    QDateTime simulationEndDate = dataEndDate.addDays(maxSimulationDays);
    if (newEndDate > simulationEndDate)
    {
        int additionalDays = simulationEndDate.daysTo(newEndDate);
        if (additionalDays > 0)
        {
            extendSimulation(additionalDays);
            dataUpdated = true;
        }
    }

    if (dataUpdated)
    {
        customPlot->rescaleAxes();
        customPlot->replot();
    }
}

void MainWindow::fetchHistoricalData(const QDateTime &startDate, const QDateTime &endDate)
{
    QString ticker = tickerInput->text().trimmed();
    if (ticker.isEmpty())
        return;

    // Adjust startDate to next trading day if it's a weekend
    QDateTime adjustedStartDate = startDate;
    while (adjustedStartDate.date().dayOfWeek() > 5) // Saturday=6, Sunday=7
    {
        adjustedStartDate = adjustedStartDate.addDays(1);
    }

    // Adjust endDate to previous trading day if it's a weekend
    QDateTime adjustedEndDate = endDate;
    while (adjustedEndDate.date().dayOfWeek() > 5)
    {
        adjustedEndDate = adjustedEndDate.addDays(-1);
    }

    // Ensure start date is before end date
    if (adjustedStartDate >= adjustedEndDate)
    {
        qDebug() << "Adjusted dates are invalid. Start date is after end date.";
        return;
    }

    // Format dates
    QString startDateStr = adjustedStartDate.toString("yyyy-MM-dd");
    QString endDateStr = adjustedEndDate.toString("yyyy-MM-dd");

    // Call Python script
    QString scriptPath = QDir::currentPath() + "/fetch_data.py";
    QStringList arguments;
    arguments << scriptPath << ticker << startDateStr << endDateStr;

    QProcess process;
    process.start("python3", arguments);
    if (!process.waitForStarted()) {
        QMessageBox::critical(this, "Error", "Failed to start the Python script.");
        return;
    }

    process.waitForFinished(-1);

    QString output = process.readAllStandardOutput();
    QString errorOutput = process.readAllStandardError();

    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        QMessageBox::critical(this, "Error", "Failed to fetch data.\n" + errorOutput);
        return;
    }

    // Read new data
    QFile file("stock_data.csv");
    if (!file.exists()) {
        QMessageBox::critical(this, "Error", "Data file not found.");
        return;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Failed to open data file.");
        return;
    }

    QVector<double> newPrices;
    QVector<QDateTime> newDates;
    QTextStream in(&file);
    QString header = in.readLine();
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList fields = line.split(',');

        if (fields.size() > 5) {
            QString dateString = fields.at(0);
            QDateTime date = QDateTime::fromString(dateString, "yyyy-MM-dd");
            if (!date.isValid()) {
                continue;
            }

            bool ok;
            double closePrice = fields.at(4).toDouble(&ok);
            if (!ok) {
                continue;
            }

            newDates.append(date);
            newPrices.append(closePrice);
        }
    }
    file.close();

    // Prepend new data, avoiding duplicates
    if (!dates.isEmpty() && !newDates.isEmpty())
    {
        // Remove overlapping dates
        while (!newDates.isEmpty() && newDates.last() >= dates.first())
        {
            newDates.removeLast();
            newPrices.removeLast();
        }
    }

    dates = newDates + dates;
    prices = newPrices + prices;

    // Update plot data
    QVector<double> timeValues;
    for (const QDateTime &date : dates) {
        timeValues.append(date.toSecsSinceEpoch());
    }

    customPlot->graph(0)->setData(timeValues, prices);

    dataStartDate = dates.first();
}

void MainWindow::extendSimulation(int additionalDays)
{
    int numSimulations = 10;
    QVector<QVector<double>> simulations;
    QVector<double> likelihoods;

    monteCarlo->setHistoricalPrices(prices);

    // Run simulations for the extended period
    monteCarlo->runSimulations(maxSimulationDays + additionalDays, numSimulations, simulations, likelihoods);

    // Update stored simulations
    storedSimulations = simulations;
    storedLikelihoods = likelihoods;

    maxSimulationDays += additionalDays;

    // Re-plot simulations
    bool mostLikely = mostLikelyCheckBox->isChecked();
    plotSimulations(storedSimulations, storedLikelihoods, mostLikely);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    QMainWindow::mouseMoveEvent(event);
}
