#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    void addInformation(const QString &s);
    QString turnFileSize_toQString(float filesize_num);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_4_clicked(int control=0);

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked(int control=0);

    void on_plainTextEdit_textChanged();

    void on_pushButton_5_clicked();

    void on_action_2_triggered();

    void on_action_3_triggered();

    void on_action_4_triggered();

    void on_action_triggered();

    void on_action_8_triggered();

    void on_action_7_triggered();

    void on_checkBox_clicked(bool checked);

    void dragEnterEvent(QDragEnterEvent *event);

    void dropEvent(QDropEvent *event);

    void on_pushButton_open_clicked(int control=0);

    void on_pushButton_key_clicked();

    void on_pushButton_key_2_clicked();

    void on_action_10_triggered();

    void on_pushButton_open_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
