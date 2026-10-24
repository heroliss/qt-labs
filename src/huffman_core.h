#pragma once
#include <QByteArray>
#include <QString>
#include <array>
#include <atomic>
#include <functional>
#include <vector>

namespace liss {
struct HuffmanStats {
    quint64 inputBytes = 0, outputBytes = 0, payloadBits = 0;
    std::array<quint64, 256> frequency{};
    std::array<QString, 256> codes{};
    QByteArray sha256;
    double entropy = 0, meanBits = 0;
};
struct WorkControl {
    std::atomic<bool> cancel{false};
    std::atomic<int> progress{0};
    std::atomic<int> phase{0}; // 0 idle, 1 statistics, 2 encoding, 3 decoding, 4 verified
};
class HuffmanCodec {
  public:
    static HuffmanStats compress(const QString &input, const QString &output, WorkControl *control = nullptr);
    static HuffmanStats decompress(const QString &input, const QString &output,
                                   WorkControl *control = nullptr);
};
QString humanBytes(quint64 bytes);
} // namespace liss
