#ifndef LuckyDog_H
#define LuckyDog_H

#include <chrono>
#include <iostream>
#include <QMainWindow>
#include <QObject>
#include <thread>
#include <vector>

QT_BEGIN_NAMESPACE
namespace Ui { class LuckyDog; }
QT_END_NAMESPACE

using namespace std;

class LuckyDog : public QMainWindow
{
    Q_OBJECT

public slots:
    void onClickAddBtn();
    void onClickCBtn();
    void onClickCBtn2();
    void onClickInBtn();
    void onClickOutBtn();
    void onClickRmBtn();
    void onConfigSet();
    void onConfigStu();

public:
    LuckyDog(QWidget *parent = nullptr);
    ~LuckyDog();
    static void changeName(LuckyDog * l, int t)
    {
        while (l->running) {
            l->onClickCBtn2();
            std::this_thread::sleep_for(std::chrono::milliseconds(t));
        }
    }

private:
    Ui::LuckyDog *ui;
    bool canSave = true;
    bool loadConfig();
    struct Student
    {
        int id;
        string name;
        int w = 10;
    };
    vector<Student> nameList;
    bool running = false;
    int time;
};
#endif // LuckyDog_H
