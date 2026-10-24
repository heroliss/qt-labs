#pragma once
#include <QString>
#include <QStringList>
#include <array>
#include <vector>

namespace liss {
struct CalcContext {
    bool degrees = true;
    long double ans = 0, memory = 0;
    std::array<long double, 3> variables{};
};
struct CalcResult {
    long double value = 0;
    QString postfix;
    QStringList steps;
    int tokenCount = 0;
};
CalcResult evaluate(const QString &expression, const CalcContext &context = {});
QString formatNumber(long double value);
} // namespace liss
