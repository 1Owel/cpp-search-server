    #include "request_queue.h"
    
    std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentStatus status) {
        // напишите реализацию
        const auto result =  search_server_.FindTopDocuments(raw_query, status);
        CalculateRequest(result);
        return result;
    }

    std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query) {
        // напишите реализацию
        const auto result =  search_server_.FindTopDocuments(raw_query);
        CalculateRequest(result);
        return result;
    }

    int RequestQueue::GetNoResultRequests() const {
        // напишите реализацию
        return no_docs_result_count_;
    }

    void RequestQueue::CalculateRequest(const std::vector<Document>& result) {
        ++current_time_;
        if (result.empty()) {
            ++no_docs_result_count_;
        }
        requests_.push_back({current_time_, result.size()});
        if (requests_.size() > min_in_day_) {
            requests_.pop_front();
            if (!result.empty()) {
                --no_docs_result_count_;
            }
        }
    }