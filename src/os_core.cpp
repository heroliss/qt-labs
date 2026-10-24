#include "os_core.h"
#include <algorithm>
#include <set>
#include <stdexcept>

namespace liss {
QString stateName(State state) {
    switch (state) {
    case State::Running:
        return QStringLiteral("运行");
    case State::Blocked:
        return QStringLiteral("阻塞");
    default:
        return QStringLiteral("就绪");
    }
}
OsModel::OsModel(int blocks) {
    reset(blocks);
}
void OsModel::reset(int blocks) {
    if (blocks < 8 || blocks > 4096)
        throw std::invalid_argument("Memory must contain 8..4096 blocks");
    processes_.clear();
    memory_.assign(blocks, 0);
    ready_.clear();
    blocked_.clear();
    running_.reset();
    events_.clear();
    nextId_ = 1;
    elapsed_ = completed_ = compactions_ = 0;
    event(QStringLiteral("系统就绪 · %1 个模拟内存块").arg(blocks));
}
void OsModel::event(const QString &text) {
    events_.push_back(QStringLiteral("T+%1  %2").arg(elapsed_, 3, 10, QChar(u'0')).arg(text));
    if (events_.size() > 200)
        events_.erase(events_.begin());
}
std::vector<Region> OsModel::freeRegions() const {
    std::vector<Region> result;
    for (int i = 0; i < static_cast<int>(memory_.size());) {
        if (memory_[i] != 0) {
            ++i;
            continue;
        }
        const int start = i;
        while (i < static_cast<int>(memory_.size()) && memory_[i] == 0)
            ++i;
        result.push_back({start, i - start});
    }
    return result;
}
int OsModel::freeBlocks() const {
    return static_cast<int>(std::count(memory_.begin(), memory_.end(), 0));
}
int OsModel::largestFree() const {
    int size = 0;
    for (const auto &r : freeRegions())
        size = std::max(size, r.length);
    return size;
}
void OsModel::dispatch() {
    if (running_ || ready_.empty())
        return;
    running_ = ready_.front();
    ready_.pop_front();
    processes_.at(*running_).state = State::Running;
}
int OsModel::create(QString name, int blocks, int burst, bool autoCompact) {
    name = name.trimmed();
    if (name.isEmpty() || name.size() > 40)
        throw std::invalid_argument("Process name must contain 1..40 characters");
    if (blocks < 1 || blocks > static_cast<int>(memory_.size()))
        throw std::invalid_argument("Invalid memory block count");
    if (burst < 1 || burst > 1000000)
        throw std::invalid_argument("CPU burst must contain 1..1000000 ticks");
    if (blocks > freeBlocks())
        throw std::runtime_error("Not enough free memory");
    if (blocks > largestFree()) {
        if (!autoCompact)
            throw std::runtime_error("Memory is fragmented; compact it or enable automatic compaction");
        compact();
    }
    auto regions = freeRegions();
    // Preserve the original simulator's largest-hole (worst-fit) allocation policy.
    const auto chosen = std::max_element(
        regions.begin(), regions.end(), [](const Region &a, const Region &b) { return a.length < b.length; });
    const int id = nextId_++;
    processes_.emplace(id, Process{id, name, chosen->start, blocks, burst});
    std::fill_n(memory_.begin() + chosen->start, blocks, id);
    ready_.push_back(id);
    dispatch();
    event(QStringLiteral("创建 P%1 · %2 · %3 块 / %4 ticks").arg(id).arg(name).arg(blocks).arg(burst));
    return id;
}
bool OsModel::terminate(int id) {
    auto found = processes_.find(id);
    if (found == processes_.end())
        return false;
    std::fill_n(memory_.begin() + found->second.start, found->second.blocks, 0);
    std::erase(ready_, id);
    std::erase(blocked_, id);
    if (running_ == id)
        running_.reset();
    processes_.erase(found);
    dispatch();
    event(QStringLiteral("结束 P%1 · 释放内存").arg(id));
    return true;
}
bool OsModel::block() {
    if (!running_)
        return false;
    const int id = *running_;
    running_.reset();
    processes_.at(id).state = State::Blocked;
    blocked_.push_back(id);
    dispatch();
    event(QStringLiteral("P%1 等待 I/O → 阻塞").arg(id));
    return true;
}
bool OsModel::wake(int id) {
    if (std::find(blocked_.begin(), blocked_.end(), id) == blocked_.end())
        return false;
    std::erase(blocked_, id);
    ready_.push_back(id);
    processes_.at(id).state = State::Ready;
    dispatch();
    event(QStringLiteral("P%1 的 I/O 完成 → 就绪").arg(id));
    return true;
}
bool OsModel::step(int quantum) {
    if (quantum < 1 || quantum > 1000)
        throw std::invalid_argument("Quantum must contain 1..1000 ticks");
    dispatch();
    if (!running_)
        return false;
    const int id = *running_;
    auto &p = processes_.at(id);
    const int ticks = std::min(quantum, p.remaining);
    p.remaining -= ticks;
    p.executed += ticks;
    elapsed_ += ticks;
    if (p.remaining == 0) {
        ++completed_;
        terminate(id);
        event(QStringLiteral("P%1 已完成").arg(id));
    } else {
        p.state = State::Ready;
        ready_.push_back(id);
        running_.reset();
        dispatch();
        event(QStringLiteral("时间片结束 · P%1 执行 %2 ticks").arg(id).arg(ticks));
    }
    return true;
}
void OsModel::compact() {
    std::vector<Process *> ordered;
    for (auto &[id, p] : processes_) {
        (void)id;
        ordered.push_back(&p);
    }
    std::sort(ordered.begin(), ordered.end(),
              [](const Process *a, const Process *b) { return a->start < b->start; });
    std::fill(memory_.begin(), memory_.end(), 0);
    int cursor = 0, moved = 0;
    for (Process *p : ordered) {
        if (p->start != cursor)
            ++moved;
        p->start = cursor;
        std::fill_n(memory_.begin() + cursor, p->blocks, p->id);
        cursor += p->blocks;
    }
    ++compactions_;
    event(QStringLiteral("内存紧凑 · 移动 %1 个进程 · 连续空闲 %2 块").arg(moved).arg(freeBlocks()));
}
void OsModel::showcase() {
    reset();
    create(QStringLiteral("Render worker"), 24, 30);
    create(QStringLiteral("Texture loader"), 18, 18);
    create(QStringLiteral("Audio stream"), 16, 28);
    create(QStringLiteral("Asset index"), 22, 22);
    create(QStringLiteral("Network I/O"), 14, 16);
    step();
    block();
    terminate(3);
    create(QStringLiteral("Shader compile"), 10, 25);
    step();
}
bool OsModel::validate(QString *error) const {
    auto fail = [&](const QString &message) {
        if (error)
            *error = message;
        return false;
    };
    std::set<int> seen;
    auto checkQueue = [&](int id, State state) {
        auto found = processes_.find(id);
        return found != processes_.end() && found->second.state == state && seen.insert(id).second;
    };
    if (running_ && !checkQueue(*running_, State::Running))
        return fail(QStringLiteral("Invalid running process"));
    for (int id : ready_)
        if (!checkQueue(id, State::Ready))
            return fail(QStringLiteral("Invalid ready queue"));
    for (int id : blocked_)
        if (!checkQueue(id, State::Blocked))
            return fail(QStringLiteral("Invalid blocked queue"));
    if (seen.size() != processes_.size())
        return fail(QStringLiteral("Process is missing from state queues"));
    if (!running_ && !ready_.empty())
        return fail(QStringLiteral("CPU idle while ready work exists"));
    std::vector<int> expected(memory_.size(), 0);
    for (const auto &[id, p] : processes_) {
        if (id < 1 || p.blocks < 1 || p.remaining < 1 || p.start < 0 ||
            p.start + p.blocks > static_cast<int>(memory_.size()))
            return fail(QStringLiteral("Invalid process range"));
        for (int i = p.start; i < p.start + p.blocks; ++i) {
            if (expected[i])
                return fail(QStringLiteral("Overlapping allocations"));
            expected[i] = id;
        }
    }
    if (expected != memory_)
        return fail(QStringLiteral("Memory and PCB allocations differ"));
    return true;
}
} // namespace liss
