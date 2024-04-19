#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include <deque>

#include "document.h"
#include "search_server.h"

class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& search_server) : search_server_(search_server) {}
    // сделаем "обёртки" для всех методов поиска, чтобы сохранять результаты для нашей статистики
    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
        // напишите реализацию
        const std::vector<Document> result = search_server_.FindTopDocuments(raw_query, document_predicate);
        CalculateRequest(result);
        return result;
    }
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);
    
    std::vector<Document> AddFindRequest(const std::string& raw_query);

    int GetNoResultRequests() const;

private:
    struct QueryResult;

    std::deque<QueryResult> requests_;
    const static int min_in_day_ = 1440;
    const SearchServer& search_server_;
    // возможно, здесь вам понадобится что-то ещё

    uint64_t current_time_ = 0;
    uint64_t no_docs_result_count_ = 0;

    void CalculateRequest(const std::vector<Document>& result);

};