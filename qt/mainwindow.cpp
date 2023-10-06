#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <qnetwork.h>
#include <QMessageBox>
#include <QTimer>
#include <QDebug>

const char* MQTT_HOST_ADDRESS = "39.107.228.202";
const int   MQTT_HOST_PORT = 1883;
const char* MQTT_CLIENT_ID = "";
const char* MQTT_USER_NAME = "";
const char* MQTT_PASSWORD = "";
const char* MQTT_USERNAME = "";
const char* MQTT_PUBLISH_TOPIC = "smart_agriculture_client";
const char* MQTT_SUBSCRIBE_TOPIC = "smart_agriculture_ui";

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_client = new QMQTT::Client(QHostAddress(MQTT_HOST_ADDRESS),MQTT_HOST_PORT);
    m_client->setClientId(MQTT_CLIENT_ID);
    m_client->setUsername(MQTT_USER_NAME);
    m_client->setPassword(MQTT_PASSWORD);
    m_client->setAutoReconnect(true);
    m_client->connectToHost();


    // MQTT
    connect(m_client,SIGNAL(connected()),this,SLOT(onConnectedMQTT()));
    connect(m_client,SIGNAL(disconnected()),this,SLOT(onDisconnectedMQTT()));
    connect(m_client,SIGNAL(received(QMQTT::Message)),this,SLOT(onDataReceivedMQTT(QMQTT::Message)));

    // ListWidget
    connect(ui->listWidget_relay_name, SIGNAL(itemClicked(QListWidgetItem*)),this, SLOT(onClickListWidgetRelayNameClicked(QListWidgetItem*)));

    // TableWidget
    connect(ui->tableWidget_trigger, SIGNAL(itemClicked(QTableWidgetItem*)),this, SLOT(onClicktableWidgetTrigger(QTableWidgetItem*)));


    // Button
    connect(ui->pushButton_relay_name, SIGNAL(clicked()),this,SLOT(onClickPushButtonRelayName()));
    connect(ui->pushButton_relay_control_open, SIGNAL(clicked()),this,SLOT(onClickPushButtonRelayControlOpen()));
    connect(ui->pushButton_relay_control_close, SIGNAL(clicked()),this,SLOT(onClickPushButtonRelayControlClose()));
    //connect(ui->pushButton_network, SIGNAL(clicked()),this,SLOT(onClickPushButtonNetwork()));
    connect(ui->pushButton_trigger_update, SIGNAL(clicked()),this,SLOT(onClickPushButtonTriggerUpdate()));
    connect(ui->pushButton_trigger_edit, SIGNAL(clicked()),this,SLOT(onClickPushButtonTriggerEdit()));


    // 定时任务
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimer()));
    m_timer->start(3000);

    // 设置文本框限制条件
    ui->lineEdit_trigger_value->setValidator(new QRegExpValidator(QRegExp("^-?(\\d+(\\.\\d{0,2})?)$")));



    // 初始化数据

    // 初始化触发器类型
    for(int i=0;i<TRIGGER_TYPE_LEN;++i)
    {
        ui->comboBox_trigger_type->addItem(TRIGGER_TYPE[i]);
    }

    // 初始化触发器比较方式
    for(int i=0;i<TRIGGER_COMPARE_LEN;++i)
    {
        ui->comboBox_trigger_compare->addItem(TRIGGER_COMPARE[i]);
    }

    // 初始化触发器操作
    for(int i=0;i<TRIGGER_OPERATE_LEN;++i)
    {
        ui->comboBox_trigger_operate->addItem(TRIGGER_OPERATE[i]);
    }

    // 初始化继电器名称
    for(int i=0;i<RELAY_LEN;++i)
    {
        ui->comboBox_trigger_relay->addItem("继电器"+QString::number(i));
    }

    // 初始化触发器列表大小
    ui->tableWidget_trigger->setRowCount(TRIGGER_LEN);

    // 初始化触发器列表默认显示内容
    for(int i=0;i<TRIGGER_LEN;++i)
    {
        this->updateUiTriggerData(i);
    }
}

MainWindow::~MainWindow()
{
    delete m_client;
    delete m_timer;
    delete ui;
}


void MainWindow::onConnectedMQTT()
{
    this->setConnectState(ConnectState::OffLine);

    m_client->subscribe(MQTT_SUBSCRIBE_TOPIC);

    m_last_message = "#connect#";
    QMQTT::Message message(136,MQTT_PUBLISH_TOPIC,"#connect#");
    m_client->publish(message);
}

void MainWindow::onDisconnectedMQTT()
{
    this->setConnectState(ConnectState::Disconnected);
}


// 接收到单片机发来的MQTT信息
void MainWindow::onDataReceivedMQTT(const QMQTT::Message message)
{
    // 接受到服务器消息时的处理
    QString strPaylaod = message.payload();
    QString strLine = ui->textBrowser_msg->toPlainText();

    QDateTime dt = QDateTime::currentDateTime();
    QString strDate = dt.toString(Qt::SystemLocaleLongDate);//日期格式自定义
    strLine += "\n";
    strLine += strDate;
    strLine += "\n";
    strLine += strPaylaod;
    strLine += "\n";

    ui->textBrowser_msg->setText(strLine);

    // 作为返回成功信息
    if(strPaylaod.compare("ok") == 0)
    {

        // 标记连接设备成功
        this->setConnectState(ConnectState::Connected);

        // 清除上次发送指令，表示收到确认信息，阻止重复发送
        m_last_message.clear();
        ui->label_commend_state->clear();
    }

    // 继电器状态数据
    else if(strPaylaod.startsWith("#relay_data#"))
    {

        QStringList relay_data_str_list = strPaylaod.split(" ");
        for(int i=0;i<RELAY_LEN;i++){

            if(relay_data_str_list[i+1] == "1")
                m_relay_data[i] = 1;
            else if(relay_data_str_list[i+1] == "0")
                m_relay_data[i] = 0;
            else
                m_relay_data[i] = -1;

        }

        this->updateUiRelayData();
        ui->groupBox_relay->setTitle("继电器状态   更新时间:"+strDate);
    }

    // 继电器名称
    else if(strPaylaod.startsWith("#relay_name#"))
    {
        QStringList relay_data_str_list = strPaylaod.split(" ");

        for(int i=0;i<RELAY_LEN;i++){
            m_relay_name[i] = relay_data_str_list[i+1];
        }

        this->updateUiRelayName();
    }

    // 传感器数据
    else if(strPaylaod.startsWith("#sensor_data#"))
    {
        QStringList sensor_data_str_list = strPaylaod.split(" ");

        for(int i=0;i<SENSOR_LEN;i++){
            bool ok;
            m_sensor_data[i] = sensor_data_str_list[i+1].toLongLong(&ok);
            if(!ok)
            {
                qDebug() << "未能转成LongLong" << Qt::endl;
            }
        }

        this->updateUiSensorData();
        ui->groupBox_sensor->setTitle("传感器数据   更新时间:"+strDate);
    }

    // 触发器数据
    else if(strPaylaod.startsWith("#trigger_data#"))
    {
        bool ok;


        // 格式: 指令 优先级id 名称 开关 类型 触发的值 比较方式 继电器id 继电器操作方式
        QStringList trigger_data = strPaylaod.split(" ");
        int id = trigger_data[1].toInt(&ok);
        if(!ok)
        {
            qDebug() << "未能转成Int" << Qt::endl;
            return;
        }

        QString name = trigger_data[2];
        bool use = trigger_data[3].toShort(&ok);
        if(!ok)
        {
            qDebug() << "未能转成Short" << Qt::endl;
            return;
        }

        int type = trigger_data[4].toInt(&ok);
        if(!ok)
        {
            qDebug() << "未能转成Int" << Qt::endl;
            return;
        }

        long long value = trigger_data[5].toLongLong(&ok);
        if(!ok)
        {
            qDebug() << "未能转成LongLong" << Qt::endl;
            return;
        }

        int compare = trigger_data[6].toInt(&ok);
        if(!ok)
        {
            qDebug() << "未能转成Int" << Qt::endl;
            return;
        }

        uint relay_id = trigger_data[7].toUInt(&ok);
        if(!ok)
        {
            qDebug() << "未能转成UInt" << Qt::endl;
            return;
        }

        int relay_operate = trigger_data[8].toInt(&ok);
        if(!ok)
        {
            qDebug() << "未能转成Int" << Qt::endl;
            return;
        }

        m_trigger_data[id].name = name;
        m_trigger_data[id].use = use;
        m_trigger_data[id].type = type;
        m_trigger_data[id].value = value;
        m_trigger_data[id].compare = compare;
        m_trigger_data[id].relay_id = relay_id;
        m_trigger_data[id].relay_operate = relay_operate;

        this->updateUiTriggerData(id);
        ui->label_trigger_update_time->setText("更新时间:"+strDate);
    }
}

// 发送MQTT消息
bool MainWindow::sendMessageMQTT(QString msg)
{
    if(m_connect_state != ConnectState::Connected)
    {
        QMessageBox::warning(this, tr("警告"),tr("网络未连接到设备"),QMessageBox::Ok);
        m_last_message = "#connect#";
        return false;
    }

    if(!m_last_message.isEmpty())
    {
        QMessageBox::warning(this, tr("警告"),tr("操作频繁，请稍后重试"),QMessageBox::Ok);
        return false;
    }

    // 记录本次发送消息 当发送成功后清楚记录
    m_last_message = msg;
    ui->label_commend_state->setText("指令发送中...");

    QMQTT::Message message(136,MQTT_PUBLISH_TOPIC,msg.toUtf8());
    m_client->publish(message);


    QString strLine = ui->textBrowser_msg->toPlainText();

    QDateTime dt = QDateTime::currentDateTime();
    QString strDate = dt.toString(Qt::SystemLocaleLongDate); //日期格式自定义
    strLine += "\n";
    strLine += strDate;
    strLine += "\t我";
    strLine += "\n";
    strLine += msg;
    strLine += "\n";

    ui->textBrowser_msg->setText(strLine);

    return true;
}

// 设置连接状态
void MainWindow::setConnectState(const ConnectState& state)
{
    m_connect_state = state;
    if(state == ConnectState::Connected)
    {
        ui->label_connect_state->setText(tr("连接成功"));
        ui->label_connect_state->setStyleSheet("QLabel{background-color:rgb(80,200,120);}");
    }
    else if(state == ConnectState::Disconnected)
    {
        ui->label_connect_state->setText(tr("连接服务器失败，正在重新连接"));
        ui->label_connect_state->setStyleSheet("QLabel{background-color:rgb(255,0,0);}");
    }
    else if(state == ConnectState::OffLine)
    {
        ui->label_connect_state->setText(tr("设备不在线"));
        ui->label_connect_state->setStyleSheet("QLabel{background-color:rgb(255,0,0);}");
    }
    else
    {
        ui->label_connect_state->setText(tr("错误"));
    }
}

// 定时函数 每隔3秒执行一次
void MainWindow::onTimer()
{
    // 如果有未回复的消息 则定时重复发送
    if(!m_last_message.isEmpty())
    {
        QMQTT::Message message(136,MQTT_PUBLISH_TOPIC,m_last_message.toUtf8());
        m_client->publish(message);
    }
}


// 更新界面中的传感器数据
void MainWindow::updateUiSensorData()
{
    QLabel* const label_sensor_list[SENSOR_LEN] = { ui->label_sensor_temp, ui->label_sensor_moist,
                                        ui->label_sensor_soil_moist,
                                        ui->label_sensor_light, ui->label_sensor_co2};

    for(int i=0;i<SENSOR_LEN;++i)
    {
       // 修改状态页面的继电器名称
        label_sensor_list[i]->setText("<span style=\"color:#008000;\">"+QString::number(double(m_sensor_data[i])/100)+"</span>");
    }

}

// 更新界面中的继电器数据
void MainWindow::updateUiRelayData()
{
    QLabel* const label_relay_list[] = { ui->label_relay_0, ui->label_relay_1,
                                      ui->label_relay_2, ui->label_relay_3,
                                      ui->label_relay_4, ui->label_relay_5,
                                      ui->label_relay_6, ui->label_relay_7};
    for(int i=0;i<RELAY_LEN;++i)
    {
        if(m_relay_data[i] == 1)
        {
            label_relay_list[i]->setText("<span style=\"color:#ff0000;\">"+tr("开启")+"</span>");
        }
        else if(m_relay_data[i] == 0)
        {
            label_relay_list[i]->setText("<span style=\"color:#0000ff;\">"+tr("关闭")+"</span>");
        }
        else
        {
            label_relay_list[i]->setText(tr("错误"));
            ui->label_connect_state->setStyleSheet("QLabel{background-color:rgb(255,0,0);}");
        }
    }
}

// 更新界面中的继电器名称
void MainWindow::updateUiRelayName()
{
    QLabel* const label_relay_name_list[] = { ui->label_relay_name_0, ui->label_relay_name_1,
                                        ui->label_relay_name_2, ui->label_relay_name_3,
                                        ui->label_relay_name_4, ui->label_relay_name_5,
                                        ui->label_relay_name_6, ui->label_relay_name_7};
    for(int i=0;i<RELAY_LEN;++i)
    {
        QString name = m_relay_name[i]+"【"+tr("继电器")+QString::number(i)+"】";

        // 修改状态页面的继电器名称
        label_relay_name_list[i]->setText(name);

        // 修改备注页面的寄存名称
        ui->listWidget_relay_name->item(i)->setText(name);

        // 修改控制页面的寄存器名称
        ui->listWidget_relay_control->item(i)->setText(name);

        // 修改设置页面操作的继电器下列菜单
        ui->comboBox_trigger_relay->setItemText(i,name);
    }
}


// 更新界面中触发器数据       节约资源只更新一个
void MainWindow::updateUiTriggerData(int i)
{
    if(i<0||i>=TRIGGER_LEN)
    {
        qDebug() << "更新的触发器下标不合法" << Qt::endl;
        return ;
    }

    // 优先级 id
    ui->tableWidget_trigger->setItem(i,0,new QTableWidgetItem( QString::number(i) ));

    // 是否被启用
    if(m_trigger_data[i].use == 1)
        ui->tableWidget_trigger->setItem(i,1,new QTableWidgetItem( tr("启用") ));
    else if(m_trigger_data[i].use == 0)
        ui->tableWidget_trigger->setItem(i,1,new QTableWidgetItem( tr("关闭") ));
    else
        ui->tableWidget_trigger->setItem(i,1,new QTableWidgetItem( "" ));


    // 触发器类型
    ui->tableWidget_trigger->setItem(i,2,new QTableWidgetItem( TRIGGER_TYPE[m_trigger_data[i].type] ));

    // 触发器名称
    ui->tableWidget_trigger->setItem(i,3,new QTableWidgetItem( m_trigger_data[i].name ));

    // 触发的继电器
    ui->tableWidget_trigger->setItem(i,4,new QTableWidgetItem( m_relay_name[m_trigger_data[i].relay_id] ));

    // 比较方式
    ui->tableWidget_trigger->setItem(i,5,new QTableWidgetItem( TRIGGER_COMPARE[m_trigger_data[i].compare] ));

    // 触发的数值
    ui->tableWidget_trigger->setItem(i,6,new QTableWidgetItem( QString::number(double(m_trigger_data[i].value)/100) ));

    // 触发器的操作
    ui->tableWidget_trigger->setItem(i,7,new QTableWidgetItem(TRIGGER_OPERATE[m_trigger_data[i].relay_operate] ));
}


void MainWindow::onClickListWidgetRelayNameClicked(QListWidgetItem* item)
{
    int index = ui->listWidget_relay_name->row(item);
    ui->lineEdit_relay_name->setText(m_relay_name[index]);
}


void MainWindow::onClicktableWidgetTrigger(QTableWidgetItem *item)
{
    int index = ui->tableWidget_trigger->row(item);
    ui->checkBox_trigger->setChecked(m_trigger_data[index].use);
    ui->lineEdit_trigger_id->setText(QString::number(index));
    ui->lineEdit_trigger_name->setText(m_trigger_data[index].name);
    ui->comboBox_trigger_type->setCurrentIndex(m_trigger_data[index].type);
    ui->comboBox_trigger_compare->setCurrentIndex(m_trigger_data[index].compare);
    ui->lineEdit_trigger_value->setText(QString::number(double(m_trigger_data[index].value)/100));
    ui->comboBox_trigger_relay->setCurrentIndex(m_trigger_data[index].relay_id);
    ui->comboBox_trigger_operate->setCurrentIndex(m_trigger_data[index].relay_operate);
}


// 点击修改继电器名称按钮
void MainWindow::onClickPushButtonRelayName()
{
    QListWidgetItem* item = ui->listWidget_relay_name->currentItem();
    int index = ui->listWidget_relay_name->row(item);
    if(index == -1)
    {
        QMessageBox::warning(this, tr("警告"),tr("请先选择一行"),QMessageBox::Ok);
        return;
    }


    QString name = ui->lineEdit_relay_name->text().trimmed();
    if(name.size() == 0)
    {
        QMessageBox::warning(this, tr("警告"),tr("名称不能为空"),QMessageBox::Ok);
        return;
    }

    if(name.size() > 5)
    {
        QMessageBox::warning(this, tr("警告"),tr("名称最大5个字符"),QMessageBox::Ok);
        return;
    }

    if(name.contains(QRegExp("\\s")))
    {
        QMessageBox::warning(this, tr("警告"),tr("名称不能包含空白字符"),QMessageBox::Ok);
        return;
    }

    bool result = this->sendMessageMQTT("#set_relay_name# "+QString::number(index)+" "+name);
    if(result)
    {
        QMessageBox::about(this, tr("成功"),tr("指令已发送，稍后会自动刷新"));
    }
}


// 点击 控制打开继电器
void MainWindow::onClickPushButtonRelayControlOpen()
{
    QListWidgetItem* item = ui->listWidget_relay_control->currentItem();
    int index = ui->listWidget_relay_control->row(item);
    if(index == -1)
    {
        QMessageBox::warning(this, tr("警告"),tr("请先选择一行"),QMessageBox::Ok);
        return;
    }

    QMessageBox::StandardButton box;
    QString box_msg;
    box_msg += m_relay_name[index];
    box_msg += "【继电器";
    box_msg += QString::number(index);
    box_msg += "】\n确定要【开启】这个继电器吗?";
    box = QMessageBox::question(this, "提示", box_msg, QMessageBox::Yes|QMessageBox::No);
    if(box==QMessageBox::No)
    {
       return;
    }

    bool result =this->sendMessageMQTT("#set_relay_open# "+QString::number(index));

    if(result)
        QMessageBox::about(this, tr("成功"),tr("已通知设备开启继电器"));


    // 这类的控制信号 为了保证与单片机的数据一致性 所以只通过单片机获取到的消息修改界面中显示的内容
    // 在这里就进行修改显示内容
}


// 点击 控制关闭继电器
void MainWindow::onClickPushButtonRelayControlClose()
{
    QListWidgetItem* item = ui->listWidget_relay_control->currentItem();
    int index = ui->listWidget_relay_control->row(item);
    if(index == -1)
    {
        QMessageBox::warning(this, tr("警告"),tr("请先选择一行"),QMessageBox::Ok);
        return;
    }

    QMessageBox::StandardButton box;
    QString box_msg;
    box_msg += m_relay_name[index];
    box_msg += "【继电器";
    box_msg += QString::number(index);
    box_msg += "】\n确定要【关闭】这个继电器吗?";
    box = QMessageBox::question(this, "提示", box_msg, QMessageBox::Yes|QMessageBox::No);
    if(box==QMessageBox::No)
    {
       return;
    }

    bool result = this->sendMessageMQTT("#set_relay_close# "+QString::number(index));

    if(result)
        QMessageBox::about(this, tr("成功"),tr("已通知设备关闭继电器"));

    // 这类的控制信号 为了保证与单片机的数据一致性 所以只通过单片机获取到的消息修改界面中显示的内容
    // 在这里就进行修改显示内容
}

//// 点击开始配网
//void MainWindow::onClickPushButtonNetwork()
//{
//    QString wifi_name = ui->lineEdit_wifi_name->text().trimmed();
//    QString wifi_password = ui->lineEdit_wifi_password->text().trimmed();

//    if(wifi_name.size() == 0||wifi_password.size() == 0)
//    {
//        QMessageBox::warning(this, tr("警告"),tr("WiFi名称和密码不能为空"),QMessageBox::Ok);
//        return;
//    }

//    if(wifi_name.contains(QRegExp("\\s"))||wifi_password.contains(QRegExp("\\s")))
//    {
//        QMessageBox::warning(this, tr("警告"),tr("WiFi名称和密码不能包含空白字符"),QMessageBox::Ok);
//        return;
//    }

//    QMessageBox::StandardButton box;
//    QString box_msg;
//    box_msg += "WiFi名称:";
//    box_msg += wifi_name;
//    box_msg += "\nWiFi密码:";
//    box_msg += wifi_password;
//    box_msg += "\n确定要连接这个网络吗?";

//    box = QMessageBox::question(this, "提示", box_msg, QMessageBox::Yes|QMessageBox::No);
//    if(box==QMessageBox::No)
//    {
//       return;
//    }


//    bool result = this->sendMessageMQTT("#set_network# "+wifi_name+" "+wifi_password);

//    if(result)
//        QMessageBox::about(this, tr("成功"),tr("设备开始连接中，如果连接失败你还可以重新配网"));
//}


void MainWindow::onClickPushButtonTriggerUpdate()
{
    bool result = this->sendMessageMQTT("#get_trigger_data#");

    if(result)
        QMessageBox::about(this, tr("成功"),tr("发送获取触发器列表指令成功"));
}

void MainWindow::onClickPushButtonTriggerEdit()
{
    bool ok;

    int id = ui->lineEdit_trigger_id->text().toUInt(&ok);
    if(!ok)
    {
        QMessageBox::warning(this, tr("警告"),tr("优先级必须为纯数字"),QMessageBox::Ok);
        return;
    }

    QString name = ui->lineEdit_trigger_name->text();
    if(name.size() == 0)
    {
        QMessageBox::warning(this, tr("警告"),tr("名称不能为空"),QMessageBox::Ok);
        return;
    }

    if(name.contains(QRegExp("\\s")))
    {
        QMessageBox::warning(this, tr("警告"),tr("名称不能包含空白字符"),QMessageBox::Ok);
        return;
    }
    if(name.size() > 5)
    {
        QMessageBox::warning(this, tr("警告"),tr("名称最大5个字符"),QMessageBox::Ok);
        return;
    }

    int use = (ui->checkBox_trigger->checkState() == Qt::Checked);
    int type = ui->comboBox_trigger_type->currentIndex();
    int compare = ui->comboBox_trigger_compare->currentIndex();

    long long value = ui->lineEdit_trigger_value->text().toDouble(&ok)*100;
    if(!ok)
    {
        QMessageBox::warning(this, tr("警告"),tr("触发的数值必须为纯数字"),QMessageBox::Ok);
        return;
    }

    int relay_id = ui->comboBox_trigger_relay->currentIndex();
    int relay_operate =ui->comboBox_trigger_operate->currentIndex();

    QMessageBox::StandardButton box;
    QString box_msg;
    box_msg += "确定要对 ";
    box_msg += m_trigger_data[id].name;
    box_msg += "【触发器";
    box_msg += QString::number(id);
    box_msg += "】 进行修改吗?";
    box = QMessageBox::question(this, "提示", box_msg, QMessageBox::Yes|QMessageBox::No);
    if(box==QMessageBox::No)
    {
       return;
    }


    // 格式: 指令 优先级id 名称 开关 类型 触发的值 比较方式 继电器id 继电器操作方式
    bool result = this->sendMessageMQTT("#set_trigger# "+QString::number(id)+" "+name+" "+QString::number(use)+" "+QString::number(type)+" "+QString::number(value)+
                                              " "+QString::number(compare)+" "+QString::number(relay_id)+" "+QString::number(relay_operate));

    if(result)
        QMessageBox::about(this, tr("成功"),tr("发送修改指令成功"));
}
