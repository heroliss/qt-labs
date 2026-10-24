#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QMessageBox>
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    void getStates(int status);
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:

    void on_pushButton_clicked();

    void on_pushButton_clear_clicked();

    void on_plainTextEdit_input_textChanged();

    void on_pushButton_calculate_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_6_clicked();

    void on_pushButton_7_clicked();

    void on_pushButton_8_clicked();

    void on_pushButton_9_clicked();

    void on_pushButton_0_clicked();

    void on_pushButton_point_clicked();

    void on_pushButton_point_2_clicked();

    void on_pushButton_sub_clicked();

    void on_pushButton_multiply_clicked();

    void on_pushButton_divide_clicked();

    void on_pushButton_add_clicked();

    void on_pushButton_M_clicked();

    void on_toolButton_clicked();

    void on_plainTextEdit_output_textChanged();

    void on_toolButton_2_clicked();

    void on_toolButton_3_clicked();

    void on_plainTextEdit_Medit_textChanged();

    void on_pushButton_sub_2_clicked();

    void on_pushButton_parentheses_L_clicked();

    void on_pushButton_pow_clicked();

    void on_pushButton_parentheses_R_clicked();

    void on_pushButton_radical_clicked();

    void on_pushButton_complement_clicked();

    void on_pushButton_del_clicked();

    void on_pushButton_AC_clicked();

    void on_pushButton_bracket_L_clicked();

    void on_pushButton_bracket_R_clicked();

    void on_pushButton_reverse_clicked();

    void on_pushButton_exp_clicked();

    void on_pushButton_up_clicked();

    void on_pushButton_down_clicked();

    void on_pushButton_equal_clicked();

    void on_listWidget_clicked(const QModelIndex &index);

    void on_commandLinkButton_clicked();

    void on_commandLinkButton_2_clicked();

    void on_pushButton_log_clicked();

    void on_pushButton_ln_clicked();

    void on_pushButton_lg_clicked();

    void on_pushButton_1x_clicked();

    void on_pushButton_X_clicked();

    void on_pushButton_Y_clicked();

    void on_pushButton_Z_clicked();

    void on_listWidget_2_clicked(const QModelIndex &index);

    void on_action_5_triggered();

    void on_action_6_triggered();



    void on_action_2_triggered();

    void on_action_triggered();

    void on_action_3_triggered(bool checked);

    void on_action_4_triggered(bool checked);

private:

    QMessageBox messagebox;
    bool mark_change;
    QString state;
    Ui::MainWindow *ui;
};


#endif // MAINWINDOW_H
