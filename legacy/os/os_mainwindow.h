#ifndef OS_MAINWINDOW_H
#define OS_MAINWINDOW_H
#include "memorygraphdialog.h"
#include "processtabledialog.h"
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QMainWindow>
#include "os_basic.h"
#include "QLabel"

namespace Ui {
class OS_MainWindow;
}

class OS_MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit OS_MainWindow(QWidget *parent = 0);
    ~OS_MainWindow();

    OS_Basic *getOs() const;
    void setOs(OS_Basic *value);

private slots:
    void on_action_triggered();

    void on_action_4_triggered();

    void on_action_5_triggered();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_6_clicked();

    void on_toolButton_2_clicked();

    void on_toolButton_clicked();

    void on_listWidget_3_doubleClicked(const QModelIndex &index);

    void on_listWidget_doubleClicked(const QModelIndex &index);

    void on_listWidget_2_doubleClicked(const QModelIndex &index);

    void on_pushButton_7_clicked();

    void on_action_6_triggered();

    void on_action_3_triggered();

    void on_action_8_triggered();

    void on_action_2_triggered();

    void on_pushButton_8_clicked();

    void on_action_7_triggered();

    void on_action_QT_triggered();

private:
    Ui::OS_MainWindow *ui;
    QLabel *statusLabel;
    MemoryGraphDialog *memoryDialog;
    ProcessTableDialog *processDialog;
    OS_Basic *os;
    char colorIndex;
    int blockSize;
    int blockNum;
    int sleepTime;
    void closeEvent(QCloseEvent *);
    void paintEvent(QPaintEvent *);
    void reloadStateList();
    void resetMemoryLable();
    bool timeStoped;
    QElapsedTimer t;
    void AutomaticDemonstration();//自动演示
    vector<QString> nameList;
    int sizeMax;
    int sizeMin;
    void addFileToRandomNameList(QString path);
    void flashAllWindows();

};

#endif // OS_MAINWINDOW_H
