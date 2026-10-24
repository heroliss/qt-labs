#include "calc_core.h"
#include "common.h"
#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QLineEdit>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QShortcut>
#include <QTimer>
#include <cmath>
#include <limits>

namespace liss {
namespace {
class CalcWindow : public QMainWindow {
  public:
    CalcWindow(bool showcase, const QString &scenario) {
        setWindowTitle(QStringLiteral("Liss Calculator · 科学表达式计算器"));
        setMinimumSize(1080, 790);
        resize(1380, 980);
        auto *root = new QWidget;
        root->setObjectName(QStringLiteral("root"));
        setCentralWidget(root);
        auto *layout = new QVBoxLayout(root);
        layout->setContentsMargins(24, 22, 24, 18);
        layout->setSpacing(16);
        auto *head = new QHBoxLayout;
        head->addWidget(heading(QStringLiteral("LISS QT LABS  /  EXPRESSION ENGINE"),
                                QStringLiteral("从表达式，到清晰的答案"),
                                QStringLiteral("手写双栈求值 · 科学函数 · 括号与优先级 · 可查看运算过程")),
                        1);
        angle_ = new QComboBox;
        angle_->setObjectName(QStringLiteral("angleMode"));
        angle_->addItems({QStringLiteral("DEG · 角度"), QStringLiteral("RAD · 弧度")});
        angle_->setMinimumWidth(150);
        angle_->setAccessibleName(QStringLiteral("角度单位"));
        head->addWidget(angle_);
        layout->addLayout(head);
        auto *body = new QHBoxLayout;
        body->setSpacing(16);
        auto *left = new QVBoxLayout;
        left->setSpacing(14);
        auto *right = new QVBoxLayout;
        right->setSpacing(14);
        QVBoxLayout *exprLayout;
        auto *exprPanel =
            panel(QStringLiteral("表达式"),
                  QStringLiteral("支持键盘输入、粘贴与隐式乘法，例如 2pi、2(3+4)。Enter 计算，Esc 清空。"),
                  &exprLayout);
        expression_ = new QLineEdit;
        expression_->setObjectName(QStringLiteral("expressionInput"));
        expression_->setAccessibleName(QStringLiteral("数学表达式"));
        expression_->setPlaceholderText(QStringLiteral("例如 sqrt(3^2+4^2) + sin(30)*12"));
        expression_->setMaxLength(8192);
        expression_->setMinimumHeight(48);
        expression_->setStyleSheet(QStringLiteral("font-family: Consolas; font-size: 21px; padding: 12px;"));
        exprLayout->addWidget(expression_);
        result_ = text(QStringLiteral("0"), QStringLiteral("result"), false);
        result_->setObjectName(QStringLiteral("calculationResult"));
        result_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        result_->setMinimumHeight(68);
        exprLayout->addWidget(result_);
        feedback_ =
            text(QStringLiteral("输入表达式即可预览结果；按“=”记录本次计算。"), QStringLiteral("muted"));
        feedback_->setObjectName(QStringLiteral("calculationStatus"));
        exprLayout->addWidget(feedback_);
        auto *resultActions = new QHBoxLayout;
        memoryValue_ = text(QStringLiteral("M = 0"), QStringLiteral("muted"));
        resultActions->addWidget(memoryValue_, 1);
        auto *copy = button(QStringLiteral("复制结果"), QStringLiteral("copyResult"));
        resultActions->addWidget(copy);
        exprLayout->addLayout(resultActions);
        left->addWidget(exprPanel);
        QVBoxLayout *keypadLayout;
        auto *keypadPanel = panel({}, {}, &keypadLayout);
        auto *grid = new QGridLayout;
        grid->setSpacing(9);
        const std::vector<QStringList> rows{{QStringLiteral("sin"), QStringLiteral("cos"),
                                             QStringLiteral("tan"), QStringLiteral("ln"),
                                             QStringLiteral("lg")},
                                            {QStringLiteral("sqrt"), QStringLiteral("abs"),
                                             QStringLiteral("("), QStringLiteral(")"), QStringLiteral("^")},
                                            {QStringLiteral("7"), QStringLiteral("8"), QStringLiteral("9"),
                                             QStringLiteral("÷"), QStringLiteral("退格")},
                                            {QStringLiteral("4"), QStringLiteral("5"), QStringLiteral("6"),
                                             QStringLiteral("×"), QStringLiteral("AC")},
                                            {QStringLiteral("1"), QStringLiteral("2"), QStringLiteral("3"),
                                             QStringLiteral("−"), QStringLiteral("Ans")},
                                            {QStringLiteral("0"), QStringLiteral("."), QStringLiteral("π"),
                                             QStringLiteral("+"), QStringLiteral("=")}};
        for (int row = 0; row < rows.size(); ++row)
            for (int col = 0; col < 5; ++col) {
                const QString label = rows[row][col];
                auto *key = button(label, QStringLiteral("key_%1_%2").arg(row).arg(col),
                                   label == QStringLiteral("="));
                key->setMinimumSize(65, 45);
                if (row >= 2 && col < 3)
                    key->setStyleSheet(QStringLiteral("font-size: 19px;"));
                if (label == QStringLiteral("退格"))
                    key->setAccessibleName(QStringLiteral("退格"));
                if (label == QStringLiteral("AC"))
                    key->setAccessibleName(QStringLiteral("清空表达式"));
                grid->addWidget(key, row, col);
                connect(key, &QPushButton::clicked, this, [this, label] {
                    if (label == QStringLiteral("=")) {
                        calculate(true);
                        return;
                    }
                    if (label == QStringLiteral("AC")) {
                        expression_->clear();
                        expression_->setFocus();
                        return;
                    }
                    if (label == QStringLiteral("退格")) {
                        expression_->backspace();
                        expression_->setFocus();
                        return;
                    }
                    QString token = label;
                    if (QStringList{QStringLiteral("sin"), QStringLiteral("cos"), QStringLiteral("tan"),
                                    QStringLiteral("ln"), QStringLiteral("lg"), QStringLiteral("sqrt"),
                                    QStringLiteral("abs")}
                            .contains(label))
                        token += QChar(u'(');
                    insert(token);
                });
            }
        keypadLayout->addLayout(grid);
        auto *more = new QHBoxLayout;
        functions_ = new QComboBox;
        functions_->setAccessibleName(QStringLiteral("更多科学函数"));
        functions_->addItems({QStringLiteral("更多函数…"), QStringLiteral("asin"), QStringLiteral("acos"),
                              QStringLiteral("atan"), QStringLiteral("sinh"), QStringLiteral("cosh"),
                              QStringLiteral("tanh"), QStringLiteral("ceil"), QStringLiteral("floor"),
                              QStringLiteral("exp")});
        more->addWidget(functions_, 1);
        for (const QString &key : {QStringLiteral("e"), QStringLiteral("%"), QStringLiteral("M")}) {
            auto *b = button(key, QStringLiteral("insert_%1").arg(key));
            more->addWidget(b);
            connect(b, &QPushButton::clicked, this, [this, key] { insert(key); });
        }
        keypadLayout->addLayout(more);
        left->addWidget(keypadPanel, 1);
        QVBoxLayout *varsLayout;
        auto *variables = panel(
            QStringLiteral("变量与记忆"),
            QStringLiteral("X、Y、Z 可直接用于表达式；M 为独立记忆值，Ans 为上次确认的结果。"), &varsLayout);
        auto *variableRow = new QHBoxLayout;
        for (int i = 0; i < 3; ++i) {
            auto *group = new QVBoxLayout;
            const QString label = QString(QChar(u'X' + i));
            group->addWidget(text(label, QStringLiteral("muted")));
            vars_[i] = new QDoubleSpinBox;
            vars_[i]->setRange(-1e100, 1e100);
            vars_[i]->setDecimals(5);
            vars_[i]->setValue(0);
            vars_[i]->setAccessibleName(QStringLiteral("变量 %1").arg(label));
            group->addWidget(vars_[i]);
            variableRow->addLayout(group, 1);
            connect(vars_[i], &QDoubleSpinBox::valueChanged, this, [this, i](double value) {
                context_.variables[i] = value;
                preview_.start(100);
            });
        }
        varsLayout->addLayout(variableRow);
        auto *memoryRow = new QHBoxLayout;
        for (const QString &label : {QStringLiteral("MS · 保存"), QStringLiteral("M+ · 累加"),
                                     QStringLiteral("MR · 读取"), QStringLiteral("MC · 清空")}) {
            auto *b = button(label, QStringLiteral("memory_%1").arg(label.left(2)));
            memoryRow->addWidget(b);
            connect(b, &QPushButton::clicked, this, [this, label] {
                if (label.startsWith(QStringLiteral("MR"))) {
                    insert(QStringLiteral("M"));
                    return;
                }
                if (label.startsWith(QStringLiteral("MC")))
                    context_.memory = 0;
                else if (valid_) {
                    if (label.startsWith(QStringLiteral("M+"))) {
                        const long double candidate = context_.memory + last_.value;
                        if (!std::isfinite(candidate) ||
                            std::abs(candidate) > std::numeric_limits<double>::max()) {
                            feedback_->setText(QStringLiteral("记忆累加超出浮点数范围，原记忆值已保留。"));
                            feedback_->setStyleSheet(QStringLiteral("color: #ffb6ad;"));
                            return;
                        }
                        context_.memory = candidate;
                    } else
                        context_.memory = last_.value;
                }
                memoryValue_->setText(QStringLiteral("M = %1").arg(formatNumber(context_.memory)));
                preview_.start(100);
            });
        }
        varsLayout->addLayout(memoryRow);
        left->addWidget(variables);
        QVBoxLayout *historyLayout;
        auto *historyPanel =
            panel(QStringLiteral("计算记录"),
                  QStringLiteral("双击记录，将表达式放回输入框。仅保留本次会话。"), &historyLayout);
        history_ = new QListWidget;
        history_->setObjectName(QStringLiteral("calculationHistory"));
        history_->setMinimumHeight(185);
        history_->setSpacing(5);
        historyLayout->addWidget(history_, 1);
        auto *clear = button(QStringLiteral("清空记录"), QStringLiteral("clearHistory"));
        historyLayout->addWidget(clear);
        right->addWidget(historyPanel, 2);
        QVBoxLayout *traceLayout;
        auto *tracePanel = panel(QStringLiteral("求值路径"),
                                 QStringLiteral("先转换为逆波兰式，再使用数值栈逐步归约。"), &traceLayout);
        postfix_ = text(QStringLiteral("RPN  —"), QStringLiteral("muted"));
        postfix_->setObjectName(QStringLiteral("postfixExpression"));
        postfix_->setFont(QFont(QStringLiteral("Consolas"), 10));
        traceLayout->addWidget(postfix_);
        steps_ = new QTableWidget;
        steps_->setObjectName(QStringLiteral("evaluationSteps"));
        prepareTable(steps_, {QStringLiteral("步骤"), QStringLiteral("运算")});
        steps_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        steps_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
        steps_->setMinimumHeight(200);
        traceLayout->addWidget(steps_, 1);
        right->addWidget(tracePanel, 3);
        QVBoxLayout *tipsLayout;
        auto *tips = panel(QStringLiteral("表达式约定"),
                           QStringLiteral("^ 为右结合乘方；−2^2 = −4；2^−3 = 0.125。\n[expr] "
                                          "表示绝对值；函数使用圆括号。\npi、e、phi、C、NA、G "
                                          "为内置常量。\n结果采用双精度有效范围，显示最多 15 位有效数字。"),
                           &tipsLayout);
        right->addWidget(tips);
        body->addLayout(left, 6);
        body->addLayout(right, 5);
        layout->addLayout(body, 1);
        connect(expression_, &QLineEdit::textEdited, this, [this] { preview_.start(130); });
        connect(expression_, &QLineEdit::textChanged, this, [this] { preview_.start(130); });
        connect(expression_, &QLineEdit::returnPressed, this, [this] {
            preview_.stop();
            calculate(true);
        });
        connect(&preview_, &QTimer::timeout, this, [this] { calculate(false); });
        preview_.setSingleShot(true);
        connect(angle_, &QComboBox::currentIndexChanged, this, [this](int index) {
            context_.degrees = index == 0;
            preview_.start(80);
        });
        connect(functions_, &QComboBox::activated, this, [this](int index) {
            if (index > 0) {
                insert(functions_->itemText(index) + QChar(u'('));
                functions_->setCurrentIndex(0);
            }
        });
        connect(clear, &QPushButton::clicked, history_, &QListWidget::clear);
        connect(history_, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *item) {
            expression_->setText(item->data(Qt::UserRole).toString());
            expression_->setFocus();
        });
        connect(copy, &QPushButton::clicked, this, [this] {
            if (valid_) {
                QApplication::clipboard()->setText(formatNumber(last_.value));
                feedback_->setText(QStringLiteral("结果已复制。"));
            }
        });
        auto *escape = new QShortcut(QKeySequence(Qt::Key_Escape), this);
        connect(escape, &QShortcut::activated, this, [this] {
            expression_->clear();
            expression_->setFocus();
        });
        if (showcase) {
            for (const QString &sample : {QStringLiteral("(1+0.05)^12*10000"),
                                          QStringLiteral("ln(exp(2))+2^3^2"), QStringLiteral("2pi*12.5")}) {
                expression_->setText(sample);
                calculate(true);
            }
            expression_->setText(scenario == QStringLiteral("error")
                                     ? QStringLiteral("sqrt(-1)")
                                     : QStringLiteral("sqrt(3^2+4^2)+sin(30)*12"));
            preview_.stop();
            calculate(scenario != QStringLiteral("error"));
        }
        expression_->setFocus();
    }

  private:
    void insert(const QString &value) {
        expression_->insert(value);
        expression_->setFocus();
    }
    void calculate(bool commit) {
        if (expression_->text().trimmed().isEmpty()) {
            result_->setText(QStringLiteral("0"));
            feedback_->setText(QStringLiteral("输入表达式即可预览结果；按“=”记录本次计算。"));
            steps_->setRowCount(0);
            postfix_->setText(QStringLiteral("RPN  —"));
            valid_ = false;
            return;
        }
        try {
            last_ = evaluate(expression_->text(), context_);
            valid_ = true;
            result_->setText(formatNumber(last_.value));
            feedback_->setText(QStringLiteral("%1 · %2 个词元 · %3 次运算%4")
                                   .arg(context_.degrees ? QStringLiteral("DEG") : QStringLiteral("RAD"))
                                   .arg(last_.tokenCount)
                                   .arg(last_.steps.size())
                                   .arg(commit ? QStringLiteral(" · 已记录") : QStringLiteral(" · 预览")));
            feedback_->setStyleSheet(QStringLiteral("color: #9eb8ce;"));
            postfix_->setText(QStringLiteral("RPN  %1").arg(last_.postfix));
            steps_->setRowCount(last_.steps.size());
            for (int i = 0; i < last_.steps.size(); ++i) {
                setCell(steps_, i, 0, QString::number(i + 1), QColor(QStringLiteral("#77dcd7")));
                setCell(steps_, i, 1, last_.steps[i]);
            }
            if (commit) {
                context_.ans = last_.value;
                auto *item = new QListWidgetItem(
                    QStringLiteral("%1\n= %2     [%3]")
                        .arg(expression_->text(), formatNumber(last_.value),
                             context_.degrees ? QStringLiteral("DEG") : QStringLiteral("RAD")));
                item->setData(Qt::UserRole, expression_->text());
                item->setSizeHint(QSize(0, 58));
                history_->insertItem(0, item);
                while (history_->count() > 50)
                    delete history_->takeItem(history_->count() - 1);
            }
        } catch (const std::exception &error) {
            valid_ = false;
            result_->setText(QStringLiteral("—"));
            feedback_->setText(QString::fromUtf8(error.what()));
            feedback_->setStyleSheet(QStringLiteral("color: #ffb6ad;"));
            postfix_->setText(QStringLiteral("检查表达式后即可继续计算"));
            steps_->setRowCount(0);
        }
    }
    CalcContext context_;
    CalcResult last_;
    bool valid_ = false;
    QTimer preview_;
    QLineEdit *expression_;
    QComboBox *angle_, *functions_;
    QLabel *result_, *feedback_, *memoryValue_, *postfix_;
    QListWidget *history_;
    QTableWidget *steps_;
    std::array<QDoubleSpinBox *, 3> vars_;
};
} // namespace
QMainWindow *createCalcWindow(bool showcase, const QString &scenario) {
    return new CalcWindow(showcase, scenario);
}
} // namespace liss
