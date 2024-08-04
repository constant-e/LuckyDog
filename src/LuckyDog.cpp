#include "LuckyDog.hpp"
#include "ui_LuckyDog.h"
#include "filestream/filestream.h"
#include "sonic/sonic.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QObject>
#include <QTableWidget>
#include <random>

#define DEFAULTCFG "{\"count\":2,\"t\":100,\"nameList\":[{\"id\":1,\"name\":\"学生1\",\"w\":10},{\"id\":2,\"name\":\"学生2\",\"w\":10}]}"

using namespace sonic_json;

bool LuckyDog::loadConfig()
{
    string cfgText = openFile("config.json");
    if (cfgText.empty()) {
        saveFile("config.json", DEFAULTCFG);
        return loadConfig();
    } else {
        Document doc;
        doc.Parse(cfgText);
        if (doc.HasParseError()) {
            QMessageBox::warning(
                this, 
                "错误", 
                "加载配置文件错误：格式不正确。\n将使用默认配置文件。");
            saveFile("config.json.bak", cfgText);
            saveFile("config.json", DEFAULTCFG);
            return loadConfig();
        }

        if (!doc.HasMember("nameList")) {
            QMessageBox::warning(
                this, 
                "异常", 
                "加载配置文件异常：不存在nameList。\n将使用默认配置文件。");
            saveFile("config.json.bak", cfgText);
            saveFile("config.json", DEFAULTCFG);
            return loadConfig();
        }

        if (!doc.HasMember("count")) {
            QMessageBox::warning(
                this, 
                "异常", 
                "加载配置文件异常：不存在count。\n将使用默认配置文件。");
            saveFile("config.json.bak", cfgText);
            saveFile("config.json", DEFAULTCFG);
            return loadConfig();
        }

        if (!doc.HasMember("t")) {
            QMessageBox::warning(
                this, 
                "异常", 
                "加载配置文件异常：不存在t。\n将使用默认配置文件。");
            saveFile("config.json.bak", cfgText);
            saveFile("config.json", DEFAULTCFG);
            return loadConfig();
        }

        time = doc.FindMember("t")->value.GetInt64();
        ui->doubleSpinBox->setValue(float(time) / 1000);

        int c = doc.FindMember("count")->value.GetInt64();
        nameList.resize(c);
        ui->tableWidget->setRowCount(c);
        ui->tableWidget->setHorizontalHeaderLabels({"学号", "姓名", "权重"});
        for (int i = 0; i < c; i++) {
            if (!(
            doc.AtPointer("nameList", i)->HasMember("id") &&
            doc.AtPointer("nameList", i)->HasMember("name") &&
            doc.AtPointer("nameList", i)->HasMember("w")
            )) {
                QMessageBox::warning(
                this, 
                "异常", 
                "加载配置文件异常：学生配置错误。\n将使用默认配置文件。");
                saveFile("config.json.bak", cfgText);
                saveFile("config.json", DEFAULTCFG);
                return loadConfig();
            }
            nameList[i].id = doc.AtPointer("nameList", i, "id")->GetInt64();
            nameList[i].name = doc.AtPointer("nameList", i, "name")->GetString();
            nameList[i].w = doc.AtPointer("nameList", i, "w")->GetInt64();
            ui->tableWidget->setItem(i, 0, new QTableWidgetItem(QString(to_string(nameList[i].id).c_str())));
            ui->tableWidget->setItem(i, 1, new QTableWidgetItem(QString(nameList[i].name.c_str())));
            ui->tableWidget->setItem(i, 2, new QTableWidgetItem(QString(to_string(nameList[i].w).c_str())));
        }
        return true;
    }
}

void LuckyDog::onClickAddBtn()
{
    canSave = false;
    int c = nameList.size() + 1;
    ui->tableWidget->setRowCount(c);
    ui->tableWidget->setItem(c - 1, 0, new QTableWidgetItem("-1"));
    ui->tableWidget->setItem(c - 1, 1, new QTableWidgetItem("未命名"));
    ui->tableWidget->setItem(c - 1, 2, new QTableWidgetItem("10"));
    canSave = true;
    onConfigStu();
}

void LuckyDog::onClickCBtn()
{
    if (running) {
        running = false;
        ui->cButton->setText("开始选择");
    } else {
        running = true;
        ui->cButton->setText("停止");
        std::thread t(&LuckyDog::changeName, this, time);
        t.detach();
    }
}

void LuckyDog::onClickCBtn2()
{
    int id;
    int l = nameList.size();
    int s = 0; // 总权重
    for (int i = 0; i < l; i++) {
        s += nameList[i].w;
    }

    random_device seed;
	ranlux48 engine(seed());
    uniform_int_distribution<> distrib(1, s);
    int r = distrib(engine);

    for (int i = 0; i <= l; i++) {
        if (r <= 0) {
            id = i;
            break;
        }
        r -= nameList[i].w;
    }

    string txt = "# 结果：\n# ";
    txt.append(to_string(nameList[id - 1].id));
    txt.append("号 ");
    txt.append(nameList[id - 1].name);

    ui->label->setText(QString(txt.c_str()));
}

void LuckyDog::onClickInBtn()
{
    string path = QFileDialog::getOpenFileName(this, "", ".", "*.json").toStdString();
    if (path.empty()) return;
    saveFile("config.json", openFile(path));
    onConfigStu();
}

void LuckyDog::onClickOutBtn()
{
    string path = QFileDialog::getSaveFileName(this, "", ".", "*.json").toStdString();
    saveFile(path, openFile("config.json"));
}

void LuckyDog::onClickRmBtn()
{
    int r = ui->tableWidget->currentRow();
    ui->tableWidget->removeRow(r);
    onConfigStu();
}

void LuckyDog::onConfigSet()
{
    time = ui->doubleSpinBox->value() * 1000;
    Document doc;
    doc.Parse(openFile("config.json"));
    doc.FindMember("t")->value.SetInt64(time);
}

void LuckyDog::onConfigStu()
{
    if (!canSave) return;
    string cfgTxt = openFile("config.json");
    Document doc;
    auto& alloc = doc.GetAllocator();
    doc.Parse(cfgTxt);
    int c = ui->tableWidget->rowCount();
    int c2 = doc.FindMember("count")->value.GetInt64();
    int dc = c - c2;
    if (dc < 0) {
        for (int i = 0; i < -dc; i++) {
            doc.AtPointer("nameList")->PopBack();
        }
    } else if (dc > 0) {
        Node * n = doc.AtPointer("nameList", 0);
        for (int i = 0; i < dc; i++) {
            doc.AtPointer("nameList")->PushBack(*n, alloc);
        }
    }

    nameList.resize(c);

    for (int i = 0; i < c; i++) {
        nameList[i].id = ui->tableWidget->item(i, 0)->text().toInt();
        nameList[i].name = ui->tableWidget->item(i, 1)->text().toStdString();
        nameList[i].w = ui->tableWidget->item(i, 2)->text().toInt();
        doc.AtPointer("nameList", i, "id")->SetInt64(nameList[i].id);
        doc.AtPointer("nameList", i, "name")->SetString(nameList[i].name);
        doc.AtPointer("nameList", i, "w")->SetInt64(nameList[i].w);
    }
    doc.FindMember("count")->value.SetInt64(c);
    WriteBuffer wb;
    doc.Serialize(wb);
    saveFile("config.json.bak", cfgTxt);
    saveFile("config.json", wb.ToString());
}

LuckyDog::LuckyDog(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::LuckyDog)
{
    ui->setupUi(this);
    if (!loadConfig()) return;
    QObject::connect(ui->addButton, &QPushButton::clicked, this, &LuckyDog::onClickAddBtn);
    QObject::connect(ui->cButton, &QPushButton::clicked, this, &LuckyDog::onClickCBtn);
    QObject::connect(ui->cButton2, &QPushButton::clicked, this, &LuckyDog::onClickCBtn2);
    QObject::connect(ui->doubleSpinBox, &QDoubleSpinBox::valueChanged, this, &LuckyDog::onConfigSet);
    QObject::connect(ui->inButton, &QPushButton::clicked, this, &LuckyDog::onClickInBtn);   
    QObject::connect(ui->outButton, &QPushButton::clicked, this, &LuckyDog::onClickOutBtn);
    QObject::connect(ui->rmButton, &QPushButton::clicked, this, &LuckyDog::onClickRmBtn);
    QObject::connect(ui->tableWidget, &QTableWidget::cellChanged, this, &LuckyDog::onConfigStu);
}

LuckyDog::~LuckyDog()
{
    delete ui;
}
