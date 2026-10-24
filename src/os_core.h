#pragma once
#include <QString>
#include <deque>
#include <map>
#include <optional>
#include <vector>

namespace liss {
enum class State { Ready, Running, Blocked };
struct Process {
    int id;
    QString name;
    int start;
    int blocks;
    int remaining;
    int executed = 0;
    State state = State::Ready;
};
struct Region {
    int start;
    int length;
};
class OsModel {
  public:
    explicit OsModel(int blocks = 128);
    int create(QString name, int blocks, int burst, bool autoCompact = true);
    bool terminate(int id);
    bool block();
    bool wake(int id);
    bool step(int quantum = 3);
    void compact();
    void reset(int blocks = 128);
    void showcase();
    const std::map<int, Process> &processes() const {
        return processes_;
    }
    const std::vector<int> &memory() const {
        return memory_;
    }
    const std::deque<int> &ready() const {
        return ready_;
    }
    const std::deque<int> &blocked() const {
        return blocked_;
    }
    std::optional<int> running() const {
        return running_;
    }
    const std::vector<QString> &events() const {
        return events_;
    }
    std::vector<Region> freeRegions() const;
    int freeBlocks() const;
    int largestFree() const;
    int elapsed() const {
        return elapsed_;
    }
    int completed() const {
        return completed_;
    }
    int compactions() const {
        return compactions_;
    }
    bool validate(QString *error = nullptr) const;

  private:
    void dispatch();
    void event(const QString &text);
    std::map<int, Process> processes_;
    std::vector<int> memory_;
    std::deque<int> ready_, blocked_;
    std::optional<int> running_;
    std::vector<QString> events_;
    int nextId_ = 1, elapsed_ = 0, completed_ = 0, compactions_ = 0;
};
QString stateName(State state);
} // namespace liss
