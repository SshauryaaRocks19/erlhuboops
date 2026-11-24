#pragma once

#include "Supplies.h"

#include <algorithm>
#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

namespace ReliefOps::Logistics {

template <typename T, typename Compare = std::less<T>>
class PriorityDispatchQueue {
public:
    explicit PriorityDispatchQueue(Compare cmp = Compare()) : cmp_(cmp) {}

    void push(const T& item)
    {
        items_.push_back(item);
        std::push_heap(items_.begin(), items_.end(), cmp_);
    }

    T pop()
    {
        if (items_.empty()) {
            throw std::out_of_range("dispatch queue is empty");
        }
        std::pop_heap(items_.begin(), items_.end(), cmp_);
        T top = items_.back();
        items_.pop_back();
        return top;
    }

    const T& top() const
    {
        if (items_.empty()) {
            throw std::out_of_range("dispatch queue is empty");
        }
        return items_.front();
    }

    bool empty() const { return items_.empty(); }
    std::size_t size() const { return items_.size(); }

private:
    std::vector<T> items_;
    Compare cmp_;
};

struct DispatchRequest {
    std::string id;
    std::string sector;
    Inventory::TriageLevel triage;
    int casualties;
    bool medical;
};

// Heap comparator: returns true when a should be served after b.
struct UrgentMedicalFirst {
    bool operator()(const DispatchRequest& a, const DispatchRequest& b) const
    {
        if (a.medical != b.medical) {
            return !a.medical;
        }
        if (a.triage != b.triage) {
            return a.triage < b.triage;
        }
        return a.casualties < b.casualties;
    }
};

}  // namespace ReliefOps::Logistics
