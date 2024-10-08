#include "LuckyDog.hpp"
#include "ui_LuckyDog.h"
#include "filestream/filestream.h"
#include "sonic/sonic.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QObject>
#include <QTableWidget>
#include <random>

#define DEFAULTCFG "{\"t\":100,\"nameList\":[{\"id\":1,\"name\":\"学生1\",\"w\":10},{\"id\":2,\"name\":\"学生2\",\"w\":10}]}"

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
        
        ui->tableWidget->setHorizontalHeaderLabels({"学号", "姓名", "权重"});
        for (int i = 0; doc.AtPointer("nameList", i) != nullptr; i++) {
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
            Student stu;
            stu.id = doc.AtPointer("nameList", i, "id")->GetInt64();
            stu.name = doc.AtPointer("nameList", i, "name")->GetString();
            stu.w = doc.AtPointer("nameList", i, "w")->GetInt64();
            ui->tableWidget->setRowCount(i + 1);
            ui->tableWidget->setItem(i, 0, new QTableWidgetItem(QString(to_string(stu.id).c_str())));
            ui->tableWidget->setItem(i, 1, new QTableWidgetItem(QString(stu.name.c_str())));
            ui->tableWidget->setItem(i, 2, new QTableWidgetItem(QString(to_string(stu.w).c_str())));
            nameList.push_back(stu);
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
    int s = 0; // 总权重
    vector<Student> realNameList; // 去除无效数据后的学生名单

    for (int i = 0; i < nameList.size(); i++) {
        if (nameList[i].w > 0) {
            realNameList.push_back(nameList[i]);
            s += nameList[i].w;
        }
    }

    int l = realNameList.size();

    random_device seed;
	ranlux48 engine(seed());
    uniform_int_distribution<> distrib(1, s);
    int r = distrib(engine);

    for (int i = 0; i <= l; i++) {
        if (r <= 0) {
            id = i;
            break;
        }
        r -= realNameList[i].w;
    }

    string txt = "# 结果：\n# ";
    txt.append(to_string(realNameList[id - 1].id));
    txt.append("号 ");
    txt.append(realNameList[id - 1].name);

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
    int dc = c - nameList.size();
    if (dc < 0) {
        for (int i = 0; i < -dc; i++) {
            doc.AtPointer("nameList")->PopBack();
        }
    } else if (dc > 0) {
        Document node;
        node.Parse("{\"id\":-1,\"name\":\"未命名\",\"w\":10}");
        for (int i = 0; i < dc; i++) {    
            auto *n = node.AtPointer();
            doc.AtPointer("nameList")->PushBack(std::move(*n), alloc);
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
