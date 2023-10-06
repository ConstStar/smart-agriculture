#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QTableWidget>
#include <qtimer.h>
#include "qmqtt.h"


#define SENSOR_LEN              5

#define RELAY_LEN               8
#define TRIGGER_TYPE_LEN        SENSOR_LEN
#define TRIGGER_LEN             50
#define TRIGGER_COMPARE_LEN     5
#define TRIGGER_OPERATE_LEN     2

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

enum ConnectState
{
    Connected = 0,          // 连接成功
    Disconnected = -1,      // 连接失败
    OffLine = -2            // 设备不在线
};


// 触发器类型
const char* const TRIGGER_TYPE[TRIGGER_TYPE_LEN] = {"环境温度","环境湿度","土壤湿度","光线强度","二氧化碳浓度"};

// 触发器比较方式
const char* const TRIGGER_COMPARE[TRIGGER_COMPARE_LEN] = {"等于","大于等于","小于等于","大于","小于"};

// 触发后继电器的操作
const char* const TRIGGER_OPERATE[TRIGGER_OPERATE_LEN] = {"关闭继电器","开启继电器"};


// 触发器结构体
struct Trigger
{
    QString 		name;		// 触发器名称	名称大小不能超过7个字符
    int8_t			use;			// 触发器开关 开启、关闭
    uint8_t 		type;			// 触发类型 温度、湿度、光照强度等
    int64_t			value;			// 触发数值	因为有小数的原因 所以每个数据都会扩大100倍来存储
    int8_t			compare;		// 比较方式 大于、小于等
    uint8_t			relay_id;		// 触发的继电器序号
    int8_t			relay_operate;	// 触发后继电器操作 闭合、断开

    Trigger()
    {
        name = "暂未获取的数据";
        use = 0;
        type = 0;
        value = 0;
        compare = 0;
        relay_id = 0;
        relay_operate = 0;
    }

};


class MainWindow : public QMainWindow
{
    Q_OBJECT

private slots:
    void onConnectedMQTT();
    void onDisconnectedMQTT();
    void onDataReceivedMQTT(const QMQTT::Message msg);


    void onClickListWidgetRelayNameClicked(QListWidgetItem* item);

    void onClicktableWidgetTrigger(QTableWidgetItem *item);

    void onClickPushButtonRelayName();
    void onClickPushButtonRelayControlOpen();
    void onClickPushButtonRelayControlClose();
    //void onClickPushButtonNetwork();
    void onClickPushButtonTriggerUpdate();
    void onClickPushButtonTriggerEdit();

    void onTimer();

private:
    void setConnectState(const ConnectState& state);
    bool sendMessageMQTT(QString Message);              // 发送MQTT消息
    void updateUiSensorData();                          // 更新界面中的传感器数据
    void updateUiRelayData();                           // 更新界面中的继电器数据
    void updateUiRelayName();                           // 更新界面中的继电器名称
    void updateUiTriggerData(int i);                    // 更新界面中的触发器数据

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    QMQTT::Client *m_client;
    QTimer *m_timer;

    ConnectState m_connect_state;               // 记录当前连接状态
    QString m_last_message;                     // 记录上一次发送的指令 直到接收到反馈内容

    int m_relay_data[RELAY_LEN];                // 存放继电器的数据 也就是每个继电器的开或者关
    QString m_relay_name[RELAY_LEN];            // 存放每个继电器的名称

    long long m_sensor_data[SENSOR_LEN];        // 传感器获取的数据 因为有小数的原因 所以每个数据都会扩大100倍来存储

    Trigger m_trigger_data[TRIGGER_LEN];
};
#endif // MAINWINDOW_H
