#pragma once

#include <cstddef>
#include <cassert>

template <typename node_t, size_t max_capacity>
struct priority_queue {

    void insert(node_t node) {
        if (empty()) {
            _begin = _end = 0;
        }
        else if (_end == max_capacity) {
            size_t current_size = size();
            assert(current_size < max_capacity);
            std::copy(begin(), end(), _nodes);
            _begin = 0;
            _end = current_size;
        }

        if (size() > 0) {
            int last = _end;

            while (_nodes[last - 1] > node && last >= _begin) {
                _nodes[last] = _nodes[last - 1];
                last--;
            }

            _nodes[last] = node;
        }
        else {
            _nodes[_end] = node;
        }
        _end++;
    }


    node_t pop() {
        return _nodes[_begin++];
    }

    auto begin() {
        return _nodes + _begin;
    }

    auto begin() const {
        return _nodes + _begin;
    }

    auto end() {
        return _nodes + _end;
    }

    auto end() const {
        return _nodes + _end;
    }

    int size() const {
        return _end - _begin;
    }

    bool empty() const {
        return _begin == _end;
    }

private:
    int _begin{ 0 };
    int _end{ 0 };
    node_t _nodes[max_capacity]{};
};