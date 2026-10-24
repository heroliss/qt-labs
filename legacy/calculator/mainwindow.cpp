#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "calculate.h"
#include "QMessageBox"
//#include "aboutdialog.h"
//#include "ui_aboutdialog.h"
bool textChangedMark=false;
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    state="欢迎使用进阶科学计算器！";
    ui->statusBar->showMessage(state);
    mark_change=true;
    messagebox.setIcon(QMessageBox::Warning);
    messagebox.setButtonText(QMessageBox::Ok,"好的，我知道了");
    messagebox.setText("M变量自动存储计算结果，因此不能包含它自己，否则计算将进入死循环");
    messagebox.setWindowTitle("警告：此编辑框中不能输入字母M");
    ui->listWidget_2->setVisible(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    ui->plainTextEdit_input->insertPlainText("1");
}

void MainWindow::on_pushButton_clear_clicked()
{
    ui->plainTextEdit_input->clear();
}

void MainWindow::on_plainTextEdit_input_textChanged()
{

}

void MainWindow::on_pushButton_calculate_clicked()
{

    static long double result=0;
    state.clear();
    int status=0;
    QString variable[4],output;
    variable[0]=ui->plainTextEdit_Medit->toPlainText();//将变量框中的数据传入variable
    variable[1]=ui->lineEdit->text();
    variable[2]=ui->lineEdit_2->text();
    variable[3]=ui->lineEdit_3->text();

    QString s_line=ui->plainTextEdit_input->toPlainText();
    //s_line.simplifyWhiteSpace();   //清除空格类的符号

    if(!calculate(s_line,result,status,variable))//判断是否算式有错误，有则令状态信息为“错误”，无错则改变output变量为计算结果
        state+="错误！";
    else
        output.setNum((double)result,'G',15);
    ui->plainTextEdit_output->setPlainText(output);

    getStates(status);        //获取状态号对应的文字信息，送给私有变量state
    ui->statusBar->showMessage(state);

}
void MainWindow::getStates(int status)  //获取一个状态号对应的文字信息
{
    switch(status)
    {
    case 0:		state+="正常";break;
    case 1:		state+="算式开头输入错误";break;
    case 2:		state+="小数点落单（存在一个不属于任何数字的小数点）";break;
    case 3:		state+="数字错误（存在多余的小数点）";break;
    case 4:		state+="输入的数值太大（单个数值最大不能超过20位）";break;
    case 5:		state+="数字后接错误（数字或常量后面不能直接连接除根号、函数、双操作数运算符和右括号以外的字符(串)）";break;
    case 6:		state+="末尾存在无用左括号";break;
    case 7:		state+="左括号后接错误（存在错误表达式于左括号后）";break;
    case 8:		state+="末尾存在无用符号";break;
    case 9:		state+="运算符后接错误（存在错误表达式于某一运算符后）";break;
    case 10:		state+="末尾存在无用左绝对值符号";break;
    case 11:		state+="左绝对值符号后接错误（存在错误表达式于左绝对值符号后）";break;
    case 12:		state+="发现绝对值符号与括号混搭错误";break;
    case 13:		state+="除0错误（存在为0的除数)";break;
    case 14:		state+="函数未识别（函数名书写错误或无此函数）";break;
    case 15:		state+="末尾存在无用函数";break;
    case 16:		state+="函数后接错误";break;
    case 17:		state+="存在非法字符（既非字母、数字也不是代表某种运算含义的字符）";break;
    case 18:		state+="发现多余的括号（或绝对值符号）";break;
    case 19:		state+="除0错误（存在为0的除数或计算过程中产生了某个为0的除数）";break;

    case 20:		state+="末尾存在无用atanb函数";break;
    case 21:		state+="atanb函数后接错误";break;
    case 22:		state+="atanb函数前接错误";break;
    case 23:		state+="算式开头atanb函数前缺少数据";break;
    case 24:		state+="常量字母后面不能连接数字";break;
    case 25:		state+="右括号后接错误（右括号或绝对值右竖线后不能接数字或字母）";break;
    case 26:		state+="变量M后接错误（变量字母后面不能连接数字）";break;
    case 27:		state+="变量M没有初值（M存储上一次的计算结果或为M指定一个值）";break;

    case 101:		state+="输入无效或无法计算（反正弦函数asin取值范围：[-1,1]）";break;
    case 102:		state+="输入无效或无法计算（反余弦函数acos取值范围：[-1,1]）";break;
    case 103:		state+="输入无效或无法计算（对数函数ln取值范围：(0,+∞））";break;
    case 104:		state+="输入无效或无法计算（对数函数lg取值范围：(0,+∞））";break;
    case 105:		state+="输入无效或无法计算（开平方函数sqrt取值范围：[0,+∞））";break;
    case 106:		state+="输入无效或无法计算（除0错误）";break;

    default:
        state+="未知错误";
        break;
    }
}

void MainWindow::on_pushButton_2_clicked()
{
        ui->plainTextEdit_input->insertPlainText("2");
}

void MainWindow::on_pushButton_3_clicked()
{
        ui->plainTextEdit_input->insertPlainText("3");
}

void MainWindow::on_pushButton_4_clicked()
{
        ui->plainTextEdit_input->insertPlainText("4");
}

void MainWindow::on_pushButton_5_clicked()
{
        ui->plainTextEdit_input->insertPlainText("5");
}

void MainWindow::on_pushButton_6_clicked()
{
        ui->plainTextEdit_input->insertPlainText("6");
}

void MainWindow::on_pushButton_7_clicked()
{
        ui->plainTextEdit_input->insertPlainText("7");
}

void MainWindow::on_pushButton_8_clicked()
{
        ui->plainTextEdit_input->insertPlainText("8");
}

void MainWindow::on_pushButton_9_clicked()
{
        ui->plainTextEdit_input->insertPlainText("9");
}

void MainWindow::on_pushButton_0_clicked()
{
        ui->plainTextEdit_input->insertPlainText("0");
}

void MainWindow::on_pushButton_point_clicked()
{
        ui->plainTextEdit_input->insertPlainText(".");
}

void MainWindow::on_pushButton_point_2_clicked()
{
        ui->plainTextEdit_input->insertPlainText("+");
}

void MainWindow::on_pushButton_sub_clicked()
{
        ui->plainTextEdit_input->insertPlainText("-");
}

void MainWindow::on_pushButton_multiply_clicked()
{
        ui->plainTextEdit_input->insertPlainText("*");
}

void MainWindow::on_pushButton_divide_clicked()
{
        ui->plainTextEdit_input->insertPlainText("/");
}

void MainWindow::on_pushButton_add_clicked()
{
        ui->plainTextEdit_input->insertPlainText("+");
}

void MainWindow::on_pushButton_M_clicked()
{
        ui->plainTextEdit_input->insertPlainText("M");
}

void MainWindow::on_toolButton_clicked()
{
    if(ui->stackedWidget_2->currentIndex()==0)
    ui->stackedWidget_2->setCurrentIndex(1);
    else
        ui->stackedWidget_2->setCurrentIndex(0);
}

void MainWindow::on_plainTextEdit_output_textChanged()
{
    if(mark_change==true)
    ui->plainTextEdit_Medit->setPlainText(ui->plainTextEdit_output->toPlainText());
    else
        ui->plainTextEdit_Medit->setPlainText("("+ui->plainTextEdit_input->toPlainText()+")");
}

void MainWindow::on_toolButton_2_clicked()
{
    ui->plainTextEdit_Medit->clear();
}

void MainWindow::on_toolButton_3_clicked()
{
    if(mark_change==true)
   {
        mark_change=false;
        ui->plainTextEdit_Medit->setPlainText("("+ui->plainTextEdit_input->toPlainText()+")");
        ui->toolButton_3->setText("转换为数值");
    }else
    {
        mark_change=true;
        ui->plainTextEdit_Medit->setPlainText(ui->plainTextEdit_output->toPlainText());
        ui->toolButton_3->setText("转换为算式");
    }
}

void MainWindow::on_plainTextEdit_Medit_textChanged()
{
    QString s=ui->plainTextEdit_Medit->toPlainText();
    if(s.contains('M'))
    {
        messagebox.show();
        int i=0;
        while(i<s.length())
        {
            if(s[i]=='M')
                s.remove(i,1);
            i++;
        }
        ui->plainTextEdit_Medit->setPlainText(s);
    }
}

void MainWindow::on_pushButton_sub_2_clicked()
{
//多余的无用函数
}

void MainWindow::on_pushButton_parentheses_L_clicked()
{
    ui->plainTextEdit_input->insertPlainText("(");

}

void MainWindow::on_pushButton_pow_clicked()
{
    ui->plainTextEdit_input->insertPlainText("^");

}

void MainWindow::on_pushButton_parentheses_R_clicked()
{
    ui->plainTextEdit_input->insertPlainText(")");
}

void MainWindow::on_pushButton_radical_clicked()
{
    ui->plainTextEdit_input->insertPlainText("sqrt");
}

void MainWindow::on_pushButton_complement_clicked()
{
    ui->plainTextEdit_input->insertPlainText("%");
}

void MainWindow::on_pushButton_del_clicked()
{
    QTextCursor cursor=ui->plainTextEdit_input->textCursor();
    if(cursor.hasSelection())
        cursor.clearSelection(); //？
    cursor.deletePreviousChar();    //删除前一个字符
}

void MainWindow::on_pushButton_AC_clicked()
{
    ui->plainTextEdit_input->clear();
}

void MainWindow::on_pushButton_bracket_L_clicked()
{
    ui->plainTextEdit_input->insertPlainText("[");
}

void MainWindow::on_pushButton_bracket_R_clicked()
{
    ui->plainTextEdit_input->insertPlainText("]");
}

void MainWindow::on_pushButton_reverse_clicked()
{
    ui->plainTextEdit_input->insertPlainText("~");
}

void MainWindow::on_pushButton_exp_clicked()
{
    ui->plainTextEdit_input->insertPlainText("E");
}

void MainWindow::on_pushButton_up_clicked()
{
    ui->plainTextEdit_input->insertPlainText("`");
}

void MainWindow::on_pushButton_down_clicked()
{
    ui->plainTextEdit_input->insertPlainText("_");
}

void MainWindow::on_pushButton_equal_clicked()
{
    on_pushButton_calculate_clicked();
}

void MainWindow::on_listWidget_clicked(const QModelIndex &index)
{
    if(ui->listWidget->currentIndex().row()==0)
    {
        ui->listWidget->setVisible(false);
        ui->listWidget_2->setVisible(true);
    }else
    ui->plainTextEdit_input->insertPlainText(ui->listWidget->currentItem()->text());

}

void MainWindow::on_commandLinkButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::on_commandLinkButton_2_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::on_pushButton_log_clicked()
{
    ui->plainTextEdit_input->setPlainText("ln("+ui->plainTextEdit_input->toPlainText()+")/ln ");
}

void MainWindow::on_pushButton_ln_clicked()
{
    ui->plainTextEdit_input->insertPlainText("ln");
}

void MainWindow::on_pushButton_lg_clicked()
{
    ui->plainTextEdit_input->insertPlainText("lg");
}

void MainWindow::on_pushButton_1x_clicked()
{
    ui->plainTextEdit_input->setPlainText("1/("+ui->plainTextEdit_input->toPlainText()+")");
}

void MainWindow::on_pushButton_X_clicked()
{
    ui->plainTextEdit_input->insertPlainText("X");
}

void MainWindow::on_pushButton_Y_clicked()
{
    ui->plainTextEdit_input->insertPlainText("Y");
}

void MainWindow::on_pushButton_Z_clicked()
{
    ui->plainTextEdit_input->insertPlainText("Z");
}

void MainWindow::on_listWidget_2_clicked(const QModelIndex &index)
{
    if(ui->listWidget_2->currentIndex().row()==0)
    {
        ui->listWidget_2->setVisible(false);
        ui->listWidget->setVisible(true);
    }else
    ui->plainTextEdit_input->insertPlainText(ui->listWidget_2->currentItem()->text());

}

void MainWindow::on_action_5_triggered()
{
    ui->plainTextEdit_input->undo();
}

void MainWindow::on_action_6_triggered()
{
    ui->plainTextEdit_input->redo();
}





void MainWindow::on_action_2_triggered()
{
    QMessageBox::about(this,"关于","进阶计算器2.1\ncreate by ss");

}

void MainWindow::on_action_triggered()
{
    QMessageBox::about(this,"帮助","\n特性说明：\n此程序对算式表达式的输入有极大的包容性和灵活性\n使用 重叠界面 和 鼠标的悬停提示 使程序简洁易用\n有强大的错误处理机制和提示功能\n有出色的宏变量计算功能\n有非常丰富的函数和变量库\n新增特色的取整、绝对值、log等运算符");
}

void MainWindow::on_action_3_triggered(bool checked)
{
    if(checked)
    {
        ui->plainTextEdit_input->setFixedWidth(301);
        ui->plainTextEdit_input->setFixedHeight(31);
        ui->plainTextEdit_output->setGeometry(80,40,221,31);
        ui->pushButton_calculate->setGeometry(10,40,61,31);
        this->setFixedWidth(305),
        this->setFixedHeight(140);
        ui->pushButton_M->setVisible(false);
        ui->label->setVisible(false);
        ui->action_4->setChecked(false);
    }
    else
    {
        ui->action_4->setChecked(true);
        on_action_4_triggered(true);
    }
}

void MainWindow::on_action_4_triggered(bool checked)
{
    if(checked)
    {
        ui->plainTextEdit_input->setFixedWidth(591);
        ui->plainTextEdit_input->setFixedHeight(61);
        ui->plainTextEdit_output->setGeometry(40,60,221,31);
        ui->pushButton_calculate->setGeometry(0,110,91,41);
        this->setFixedWidth(586),
        this->setFixedHeight(296);
        ui->pushButton_M->setVisible(true);
        ui->label->setVisible(true);
        ui->action_3->setChecked(false);
    }
    else
    {
        ui->action_3->setChecked(true);
        on_action_3_triggered(true);
    }
}
