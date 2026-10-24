#include "calc_core.h"
#include <QRegularExpression>
#include <algorithm>
#include <cmath>
#include <limits>
#include <numbers>
#include <stdexcept>

namespace liss {
namespace {
struct Token {
    enum Type { Number, Operator, Function, Left, Right } type;
    QString text;
    long double number = 0;
    int position = 0;
};
[[noreturn]] void invalid(const QString &text, int position = -1) {
    throw std::runtime_error(
        (position >= 0 ? QStringLiteral("第 %1 个字符：%2").arg(position + 1).arg(text) : text)
            .toUtf8()
            .constData());
}
int precedence(const QString &op) {
    if (op == QStringLiteral("+") || op == QStringLiteral("-"))
        return 1;
    if (op == QStringLiteral("*") || op == QStringLiteral("/") || op == QStringLiteral("%"))
        return 2;
    if (op == QStringLiteral("neg") || op == QStringLiteral("pos"))
        return 3;
    return 4;
}
bool rightAssociative(const QString &op) {
    return op == QStringLiteral("^") || op == QStringLiteral("neg") || op == QStringLiteral("pos");
}
bool unary(const QString &op) {
    return op == QStringLiteral("neg") || op == QStringLiteral("pos");
}
bool functionName(const QString &name) {
    static const QStringList functions{
        QStringLiteral("sin"),  QStringLiteral("cos"),  QStringLiteral("tan"),  QStringLiteral("asin"),
        QStringLiteral("acos"), QStringLiteral("atan"), QStringLiteral("sinh"), QStringLiteral("cosh"),
        QStringLiteral("tanh"), QStringLiteral("ln"),   QStringLiteral("lg"),   QStringLiteral("log"),
        QStringLiteral("sqrt"), QStringLiteral("abs"),  QStringLiteral("ceil"), QStringLiteral("floor"),
        QStringLiteral("exp")};
    return functions.contains(name);
}
long double constant(const QString &name, const CalcContext &context, bool &found) {
    found = true;
    if (name == QStringLiteral("pi"))
        return std::numbers::pi_v<long double>;
    if (name == QStringLiteral("e"))
        return std::numbers::e_v<long double>;
    if (name == QStringLiteral("phi"))
        return (std::sqrt(5.0L) + 1) / 2;
    if (name == QStringLiteral("dgold"))
        return (std::sqrt(5.0L) - 1) / 2;
    if (name == QStringLiteral("c"))
        return 299792458.0L;
    if (name == QStringLiteral("na"))
        return 6.02214076e23L;
    if (name == QStringLiteral("g"))
        return 6.67430e-11L;
    if (name == QStringLiteral("ans"))
        return context.ans;
    if (name == QStringLiteral("m"))
        return context.memory;
    if (name == QStringLiteral("x"))
        return context.variables[0];
    if (name == QStringLiteral("y"))
        return context.variables[1];
    if (name == QStringLiteral("z"))
        return context.variables[2];
    found = false;
    return 0;
}
std::vector<Token> tokenize(QString source, const CalcContext &context) {
    if (source.size() > 8192)
        invalid(QStringLiteral("表达式过长，最多 8192 个字符"));
    source.replace(QChar(u'×'), QChar(u'*'));
    source.replace(QChar(u'÷'), QChar(u'/'));
    source.replace(QChar(u'−'), QChar(u'-'));
    source.replace(QChar(u'π'), QStringLiteral("pi"));
    source.replace(QChar(u'√'), QStringLiteral("sqrt"));
    static const QRegularExpression number(QStringLiteral(R"(^(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][+-]?\d+)?)"));
    std::vector<Token> result;
    for (int i = 0; i < source.size();) {
        const QChar c = source[i];
        if (c.isSpace()) {
            ++i;
            continue;
        }
        const int start = i;
        if (c.isDigit() || c == QChar(u'.')) {
            const auto match = number.match(source.mid(i));
            if (!match.hasMatch())
                invalid(QStringLiteral("数字格式错误"), i);
            const QString text = match.captured(0);
            bool okay = false;
            const double value = text.toDouble(&okay);
            if (!okay || !std::isfinite(value))
                invalid(QStringLiteral("数字超出可表示范围"), i);
            result.push_back({Token::Number, text, value, i});
            i += text.size();
            if (i < source.size() && source[i] == QChar(u'.'))
                invalid(QStringLiteral("数字含有多余的小数点"), i);
        } else if (c.isLetter()) {
            while (i < source.size() && source[i].isLetter())
                ++i;
            const QString name = source.mid(start, i - start).toLower();
            bool found = false;
            const long double value = constant(name, context, found);
            if (found) {
                if (!std::isfinite(value) || std::abs(value) > std::numeric_limits<double>::max())
                    invalid(QStringLiteral("变量或记忆值超出浮点数范围"), start);
                result.push_back({Token::Number, name, value, start});
            } else if (functionName(name))
                result.push_back({Token::Function, name, 0, start});
            else
                invalid(QStringLiteral("未知函数或常量“%1”").arg(name), start);
        } else {
            ++i;
            if (QStringLiteral("+-*/%^~").contains(c))
                result.push_back(
                    {Token::Operator, c == QChar(u'~') ? QStringLiteral("-") : QString(c), 0, start});
            else if (c == QChar(u'(') || c == QChar(u'['))
                result.push_back({Token::Left, QString(c), 0, start});
            else if (c == QChar(u')') || c == QChar(u']'))
                result.push_back({Token::Right, QString(c), 0, start});
            else
                invalid(QStringLiteral("不支持的字符“%1”").arg(c), start);
        }
        if (result.size() > 4096)
            invalid(QStringLiteral("表达式过于复杂，最多 4096 个词元"));
    }
    return result;
}
long double single(const QString &f, long double a, const CalcContext &c) {
    const long double unit = c.degrees ? std::numbers::pi_v<long double> / 180 : 1;
    if (f == QStringLiteral("sin"))
        return std::sin(a * unit);
    if (f == QStringLiteral("cos"))
        return std::cos(a * unit);
    if (f == QStringLiteral("tan")) {
        if (std::abs(std::cos(a * unit)) < 1e-14L)
            invalid(QStringLiteral("tan 在此角度无定义"));
        return std::tan(a * unit);
    }
    if (f == QStringLiteral("asin") || f == QStringLiteral("acos")) {
        if (std::abs(a) > 1)
            invalid(QStringLiteral("反三角函数的参数必须在 [-1, 1] 内"));
        return (f == QStringLiteral("asin") ? std::asin(a) : std::acos(a)) / unit;
    }
    if (f == QStringLiteral("atan"))
        return std::atan(a) / unit;
    if (f == QStringLiteral("sinh"))
        return std::sinh(a);
    if (f == QStringLiteral("cosh"))
        return std::cosh(a);
    if (f == QStringLiteral("tanh"))
        return std::tanh(a);
    if (f == QStringLiteral("sqrt")) {
        if (a < 0)
            invalid(QStringLiteral("实数平方根的参数不能小于 0"));
        return std::sqrt(a);
    }
    if (f == QStringLiteral("ln") || f == QStringLiteral("lg") || f == QStringLiteral("log")) {
        if (a <= 0)
            invalid(QStringLiteral("对数的参数必须大于 0"));
        return f == QStringLiteral("ln") ? std::log(a) : std::log10(a);
    }
    if (f == QStringLiteral("abs"))
        return std::abs(a);
    if (f == QStringLiteral("ceil"))
        return std::ceil(a);
    if (f == QStringLiteral("floor"))
        return std::floor(a);
    if (f == QStringLiteral("exp"))
        return std::exp(a);
    invalid(QStringLiteral("未知函数"));
}
} // namespace
QString formatNumber(long double value) {
    if (value == 0)
        value = 0;
    return QString::number(static_cast<double>(value), 'g', 15);
}
CalcResult evaluate(const QString &expression, const CalcContext &context) {
    auto tokens = tokenize(expression, context);
    if (tokens.empty())
        invalid(QStringLiteral("请输入表达式"));
    // Two explicit stacks retain the original calculator's operator-precedence approach.
    std::vector<Token> output, operators;
    bool needsOperand = true;
    int depth = 0;
    auto pushBinary = [&](Token token) {
        while (!operators.empty() && operators.back().type == Token::Operator &&
               (precedence(operators.back().text) > precedence(token.text) ||
                (precedence(operators.back().text) == precedence(token.text) &&
                 !rightAssociative(token.text)))) {
            output.push_back(operators.back());
            operators.pop_back();
        }
        operators.push_back(token);
    };
    for (size_t i = 0; i < tokens.size(); ++i) {
        Token token = tokens[i];
        const bool startsValue =
            token.type == Token::Number || token.type == Token::Function || token.type == Token::Left;
        if (!needsOperand && startsValue) {
            if (i > 0 && tokens[i - 1].type == Token::Number && token.type == Token::Number &&
                (tokens[i - 1].text[0].isDigit() || tokens[i - 1].text[0] == QChar(u'.')) &&
                (token.text[0].isDigit() || token.text[0] == QChar(u'.')))
                invalid(QStringLiteral("两个数字之间需要运算符"), token.position);
            pushBinary({Token::Operator, QStringLiteral("*"), 0, token.position});
            needsOperand = true;
        }
        if (token.type == Token::Number) {
            output.push_back(token);
            needsOperand = false;
        } else if (token.type == Token::Function) {
            if (i + 1 >= tokens.size() || tokens[i + 1].type != Token::Left ||
                tokens[i + 1].text != QStringLiteral("("))
                invalid(QStringLiteral("函数后需要圆括号，例如 sin(30)"), token.position);
            operators.push_back(token);
            needsOperand = true;
        } else if (token.type == Token::Left) {
            if (++depth > 256)
                invalid(QStringLiteral("括号嵌套过深"), token.position);
            if (token.text == QStringLiteral("["))
                operators.push_back({Token::Function, QStringLiteral("abs"), 0, token.position});
            operators.push_back(token);
            needsOperand = true;
        } else if (token.type == Token::Right) {
            if (needsOperand)
                invalid(QStringLiteral("括号内缺少数值"), token.position);
            while (!operators.empty() && operators.back().type != Token::Left) {
                output.push_back(operators.back());
                operators.pop_back();
            }
            if (operators.empty())
                invalid(QStringLiteral("缺少左括号"), token.position);
            if ((operators.back().text == QStringLiteral("[")) != (token.text == QStringLiteral("]")))
                invalid(QStringLiteral("括号类型不匹配"), token.position);
            operators.pop_back();
            --depth;
            if (!operators.empty() && operators.back().type == Token::Function) {
                output.push_back(operators.back());
                operators.pop_back();
            }
            needsOperand = false;
        } else {
            if (needsOperand) {
                if (token.text != QStringLiteral("+") && token.text != QStringLiteral("-"))
                    invalid(QStringLiteral("运算符前缺少数值"), token.position);
                token.text =
                    token.text == QStringLiteral("-") ? QStringLiteral("neg") : QStringLiteral("pos");
                operators.push_back(token);
            } else {
                pushBinary(token);
                needsOperand = true;
            }
        }
    }
    if (needsOperand)
        invalid(QStringLiteral("表达式末尾缺少数值"));
    while (!operators.empty()) {
        if (operators.back().type == Token::Left)
            invalid(QStringLiteral("缺少右括号"), operators.back().position);
        output.push_back(operators.back());
        operators.pop_back();
    }
    CalcResult result;
    result.tokenCount = static_cast<int>(tokens.size());
    std::vector<long double> values;
    QStringList postfix;
    for (const Token &token : output) {
        postfix.push_back(token.text);
        if (token.type == Token::Number) {
            values.push_back(token.number);
            continue;
        }
        const bool one = token.type == Token::Function || unary(token.text);
        if (values.size() < (one ? 1u : 2u))
            invalid(QStringLiteral("操作数不足"), token.position);
        const long double b = values.back();
        values.pop_back();
        long double value = 0;
        QString step;
        if (one) {
            value = token.type == Token::Function         ? single(token.text, b, context)
                    : token.text == QStringLiteral("neg") ? -b
                                                          : b;
            step = QStringLiteral("%1(%2)").arg(token.text, formatNumber(b));
        } else {
            const long double a = values.back();
            values.pop_back();
            if (token.text == QStringLiteral("+"))
                value = a + b;
            else if (token.text == QStringLiteral("-"))
                value = a - b;
            else if (token.text == QStringLiteral("*"))
                value = a * b;
            else if (token.text == QStringLiteral("/") || token.text == QStringLiteral("%")) {
                if (b == 0)
                    invalid(QStringLiteral("除数不能为 0"), token.position);
                value = token.text == QStringLiteral("/") ? a / b : std::fmod(a, b);
            } else
                value = std::pow(a, b);
            step = QStringLiteral("%1 %2 %3").arg(formatNumber(a), token.text, formatNumber(b));
        }
        if (!std::isfinite(value) || std::abs(value) > std::numeric_limits<double>::max())
            invalid(QStringLiteral("结果超出实数范围或发生溢出"), token.position);
        values.push_back(value);
        if (result.steps.size() < 60)
            result.steps.push_back(QStringLiteral("%1 = %2").arg(step, formatNumber(value)));
    }
    if (values.size() != 1)
        invalid(QStringLiteral("表达式无法归约为一个结果"));
    result.value = values.back();
    result.postfix = postfix.join(QChar(u' '));
    return result;
}
} // namespace liss
