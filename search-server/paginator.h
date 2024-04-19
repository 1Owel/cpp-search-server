#pragma once
#include <vector>


template <typename Container, typename Iterator>
class Paginator {
    public:
    Paginator(const Container& c, const size_t& page_size, const Iterator& begin) {
        size_t num_on_page = 0;
        auto page_begin = begin;
        size_t count = 0;
        auto it = c.begin();
        do
        {
            if (it == c.end() || num_on_page == page_size) {
                page_ranges_.push_back({page_begin, it});
                page_begin = it;
                num_on_page = 0;
            }
            ++it;
            ++num_on_page;
            ++count;
        } while (count != (c.size() + 1));

    }

    auto GetPagesContainer() {
        return page_ranges_;
    }
    private:
    std::vector<std::pair<Iterator, Iterator>> page_ranges_;
};