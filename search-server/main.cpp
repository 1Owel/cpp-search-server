#include <algorithm>
#include <cmath>
#include <iostream>
#include <optional>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <numeric>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        }
        else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}

struct Document {
    explicit Document() {
        id = 0;
        relevance = 0.;
        rating = 0;
    }
    Document(int doc_id, double doc_relev, int doc_rating) :
        id(doc_id),
        relevance(doc_relev),
        rating(doc_rating)
        {
        }
    
    int id;
    double relevance;
    int rating;
};

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

#define COMPARE_DOUBLE(a, b) abs((a) - (b)) < 1e-6

class SearchServer {
public:

    inline static constexpr int INVALID_DOCUMENT_ID = -1;

    template <typename StringCollection>
    explicit SearchServer(const StringCollection& stop_words) {
            for (const string& word : stop_words) {
                if (!CheckSpecialSymbols(word)) {
                    throw invalid_argument("Спецсимвол в строке для создания стоп слов"s);
                } else if (!word.empty() && !stop_words_.count(word)) {
                    stop_words_.insert(word);
                }
        }
    }

    explicit SearchServer(const string& text) {
        if (!CheckSpecialSymbols(text)) {
            throw invalid_argument("Спецсимвол в строке для создания стоп слов"s);
        }
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string& document, DocumentStatus status,
                                   const vector<int>& ratings) {
        if (document_id < 0 || documents_.count(document_id)) {
            throw invalid_argument("ID документа отрицательный, либо документ с таким ID уже существует"s);
        }
        if (!CheckSpecialSymbols(document)) {
            throw invalid_argument("Спецсимвол в строке добавляемого документа"s);
        }
        const vector<string> words = SplitIntoWordsNoStop(document);
        const double inv_word_count = 1.0 / words.size();
        for (const string& word : words) {
            word_to_document_freqs_[word][document_id] += inv_word_count;
        }
        documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status });
    }

    template<typename SortBy>
    vector<Document> FindTopDocuments(const string& raw_query,
        const SortBy sortby) const {
        const auto query = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query, sortby);

        sort(matched_documents.begin(), matched_documents.end(),
            [](const Document& lhs, const Document& rhs) {
                if (COMPARE_DOUBLE(lhs.relevance, rhs.relevance)) {
                    return lhs.rating > rhs.rating;
                }
                else {
                    return lhs.relevance > rhs.relevance;
                }
            });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

    vector<Document> FindTopDocuments(const string& raw_query) const {
        return FindTopDocuments(raw_query, [](int document_id, DocumentStatus status, int rating) { return status == DocumentStatus::ACTUAL; });
    }

    vector<Document> FindTopDocuments(const string& raw_query, DocumentStatus correct_status) const {
        return FindTopDocuments(raw_query, [correct_status](int document_id, DocumentStatus status, int rating) { return status == correct_status; });
    }

    int GetDocumentCount() const {
        return documents_.size();
    }

    tuple<vector<string>, DocumentStatus> MatchDocument(const string& raw_query,
        int document_id) const {
        auto query = ParseQuery(raw_query);
        vector<string> matched_words;
        for (const string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            if (word_to_document_freqs_.at(word).count(document_id)) {
                matched_words.push_back(word);
            }
        }
        for (const string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            if (word_to_document_freqs_.at(word).count(document_id)) {
                matched_words.clear();
                break;
            }
        }
        tuple<vector<string>, DocumentStatus> result = { matched_words, documents_.at(document_id).status };
        return result;
    }

    int GetDocumentId(int index) const {
        if (index >= 0 && index < documents_.size()) {
            int current_index = 0;
            for (auto [i, doc_data] : documents_) {
                if (index != current_index) {
                    ++current_index;
                } else {
                    return i;
                }
            }        
        }
        throw out_of_range("Индекс выходит за пределы имеющихся документов"s);
    }

private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };

    set<string> stop_words_;
    map<string, map<int, double>> word_to_document_freqs_;
    map<int, DocumentData> documents_;

    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }

    static int ComputeAverageRating(const vector<int>& ratings) {
        if (ratings.empty()) {
            return 0;
        }
        int rating_sum = accumulate(ratings.begin(), ratings.end(), 0);
        return rating_sum / static_cast<int>(ratings.size());
    }

    struct QueryWord {
        string data;
        bool is_minus;
        bool is_stop;
    };

    bool CheckSpecialSymbols(const string& text) const {
        for (char i : text) {
            const int code = i + 0;
            if (code >= 0 && code <= 31) {
                return false;
            }
        }
        return true;
    }

    QueryWord ParseQueryWord(string text) const {
        bool is_minus = false;
        // Word shouldn't be empty
        if (text[0] == '-') {
            if (text.size() == 1 || text[1] == '-') {
                throw invalid_argument("Двойной минус перед словом или отсутсвие слова после минуса"s);
            }
            is_minus = true;
            text = text.substr(1);
        }
        const QueryWord result = { text, is_minus, IsStopWord(text) };
        return result;
    }

    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };

    Query ParseQuery(const string& text) const {
        Query query;
        for (const string& word : SplitIntoWords(text)) {
            auto query_word = ParseQueryWord(word);
            {
            if (!CheckSpecialSymbols(word)) {
                throw invalid_argument("Обнаружены спецсимволы в запросе"s);
            }
            }
            if (!query_word.is_stop) {
                if (query_word.is_minus) {
                    query.minus_words.insert(query_word.data);
                }
                else {
                    query.plus_words.insert(query_word.data);
                }
            }
        }
        return query;
    }

    // Existence required
    double ComputeWordInverseDocumentFreq(const string& word) const {
        return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
    }

    template<typename SortBy>
    vector<Document> FindAllDocuments(const Query& query, SortBy sortby) const {
        map<int, double> document_to_relevance;
        for (const string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
            for (const auto& [document_id, term_freq] : word_to_document_freqs_.at(word)) {
                if (sortby(document_id, documents_.at(document_id).status, documents_.at(document_id).rating)) {
                    document_to_relevance[document_id] += term_freq * inverse_document_freq;
                }
            }
        }

        for (const string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            for (const auto& [document_id, _] : word_to_document_freqs_.at(word)) {
                document_to_relevance.erase(document_id);
            }
        }

        vector<Document> matched_documents;
        for (const auto& [document_id, relevance] : document_to_relevance) {
            matched_documents.push_back(
                { document_id, relevance, documents_.at(document_id).rating });
        }
        return matched_documents;
    }
};

