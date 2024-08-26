#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QTextStream>
#include <QTimer>
#include <QString>

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#include <QDateTime>



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , counter(0)  // 初始化计数器为0
{
    ui->setupUi(this);

    // 初始化定时器
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateTemperature);

    // 设置定时器每200ms触发一次
    timer->start(500);

//    /// 添加以下代码
//        QLineSeries *series = new QLineSeries();
//        *series << QPointF(11, 1)
//                << QPointF(13, 3)
//                << QPointF(17, 6)
//                << QPointF(18, 3)
//                << QPointF(20, 2);

//        QChart *chart = new QChart();
//        chart->legend()->hide();
//        chart->addSeries(series);
//        chart->createDefaultAxes();
//        chart->setTitle("Simple line chart example");

//        ui->chartView->setChart(chart);
//        ui->chartView->setRenderHint(QPainter::Antialiasing);
}

MainWindow::~MainWindow()
{
    delete ui;
}



// LED 控制路径
const QString led1Path = "/sys/class/leds/blue/brightness";
const QString led2Path = "/sys/class/leds/green/brightness";
const QString led3Path = "/sys/class/leds/red/brightness";

//void MainWindow::updateButtonText()
//{
//    counter++;
//    ui->pushButton->setText(QString::number(counter));  // 将计数值转换为字符串并设置为按钮文本
//}

void MainWindow::on_pushButton_clicked()
{
    QFile file(led1Path);
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream in(&file);
        QString currentState = in.readLine().trimmed();
        file.close();

        // 切换LED状态
        QString newState = (currentState == "1") ? "0" : "1";

        if (file.open(QIODevice::WriteOnly)) {
            QTextStream out(&file);
            out << newState;
            file.close();
        } else {
            // 处理文件写入失败的情况
        }
    } else {
        // 处理文件读取失败的情况
    }
}

void MainWindow::on_pushButton_2_clicked()
{
    QFile file(led2Path);
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream in(&file);
        QString currentState = in.readLine().trimmed();
        file.close();

        // 切换LED状态
        QString newState = (currentState == "1") ? "0" : "1";

        if (file.open(QIODevice::WriteOnly)) {
            QTextStream out(&file);
            out << newState;
            file.close();
        } else {
            // 处理文件写入失败的情况
        }
    } else {
        // 处理文件读取失败的情况
    }
}

void MainWindow::on_pushButton_3_clicked()
{
    QFile file(led3Path);
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream in(&file);
        QString currentState = in.readLine().trimmed();
        file.close();

        // 切换LED状态
        QString newState = (currentState == "1") ? "0" : "1";

        if (file.open(QIODevice::WriteOnly)) {
            QTextStream out(&file);
            out << newState;
            file.close();
        } else {
            // 处理文件写入失败的情况
        }
    } else {
        // 处理文件读取失败的情况
    }
}

// AHT20 I2C地址
#define AHT20_I2C_ADDR 0x38

bool initAHT20(int file) {
    uint8_t initCmd[] = {0xBE, 0x08, 0x00}; // 初始化命令
    if (write(file, initCmd, sizeof(initCmd)) != sizeof(initCmd)) {
        return false;
    }
    usleep(10000); // 等待10ms
    return true;
}

float readTemperature(int file) {
    uint8_t triggerCmd[] = {0xAC, 0x33, 0x00}; // 触发测量命令
    if (write(file, triggerCmd, sizeof(triggerCmd)) != sizeof(triggerCmd)) {
        return -1;
    }
    usleep(80000); // 等待80ms

    uint8_t data[6];
    if (read(file, data, sizeof(data)) != sizeof(data)) {
        return -1;
    }

    // 检查状态位
    if (data[0] & 0x80) {
        return -1;
    }

    // 计算温度
    uint32_t rawTemp = (data[3] & 0x0F) << 16 | data[4] << 8 | data[5];
    float temperature = ((float)rawTemp * 200.0 / 1048576.0) - 50.0;
    return temperature;
}

void MainWindow::on_pushButton_4_clicked()
{
    // 打开I2C设备
    int file = open("/dev/i2c-1", O_RDWR);
    if (file < 0) {
        ui->textEdit_1->setText("Error: Failed to open I2C bus.");
        return;
    }

    // 设置从设备地址
    if (ioctl(file, I2C_SLAVE, AHT20_I2C_ADDR) < 0) {
        ui->textEdit_1->setText("Error: Failed to acquire bus access and/or talk to slave.");
        ::close(file);
        return;
    }

    // 初始化AHT20传感器
    if (!initAHT20(file)) {
        ui->textEdit_1->setText("Error: Failed to initialize AHT20.");
        ::close(file);
        return;
    }

    // 读取温度
    float temperature = readTemperature(file);
    if (temperature != -1) {
        // 将结果显示在 textEdit_1 中
        ui->textEdit_1->setText(QString("%1 °C").arg(temperature));
    } else {
        ui->textEdit_1->setText("Error: Failed to read temperature from AHT20.");
    }

    // 关闭I2C设备
    ::close(file);
}

#include <QtCharts/QLineSeries>

// 假设我们只存储最近50个数据点
const int DATA_POINTS = 50;
float temperatureData[DATA_POINTS] = {0}; // 初始化为0

void MainWindow::updateTemperature()
{
    int file = open("/dev/i2c-1", O_RDWR);
    if (file < 0) {
        ui->textEdit_1->setText("Error: Failed to open I2C bus.");
        return;
    }

    if (ioctl(file, I2C_SLAVE, AHT20_I2C_ADDR) < 0) {
        ui->textEdit_1->setText("Error: Failed to acquire bus access and/or talk to slave.");
        ::close(file);
        return;
    }

    if (!initAHT20(file)) {
        ui->textEdit_1->setText("Error: Failed to initialize AHT20.");
        ::close(file);
        return;
    }

    float temperature = readTemperature(file);
    if (temperature != -1) {
        ui->textEdit_1->setText(QString(" %1 °C").arg(temperature));
    } else {
        ui->textEdit_1->setText("Error: Failed to read temperature from AHT20.");
    }

    ::close(file);

    // 移动数组中的数据，删除最旧的数据
    for (int i = 1; i < DATA_POINTS; ++i) {
        temperatureData[i - 1] = temperatureData[i];
    }
    temperatureData[DATA_POINTS - 1] = temperature; // 插入最新数据

    // 使用 << 操作符绘制全部数据
    QLineSeries *series = new QLineSeries();
    for (int i = 0; i < DATA_POINTS; ++i) {
        *series << QPointF(i, temperatureData[i]);
    }

    // 更新图表
    QChart *chart = ui->chartView->chart();
    chart->removeAllSeries(); // 清除旧的曲线
    chart->addSeries(series); // 添加新的曲线
    chart->createDefaultAxes();
    chart->axisX()->setRange(0, DATA_POINTS - 1); // 设置X轴范围
    chart->axisY()->setRange(24, 36); // 根据实际温度范围设置Y轴范围
    ui->chartView->setRenderHint(QPainter::Antialiasing);
}
