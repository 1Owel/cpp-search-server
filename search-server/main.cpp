#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>



using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result = 0;
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
    int id;
    double relevance;
};

class SearchServer {
public:
    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(const int& document_id, const string& document) {
        const vector<string> words = SplitIntoWordsNoStop(document);
        for (const string& word : words) {
            word_to_document_freqs_[word][document_id] += 1.0 / words.size(); // Подсчет тф слова для каждого документа
        }
    }

    vector<Document> FindTopDocuments(const string& raw_query) const {
        const Query query_words = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query_words);

        sort(matched_documents.begin(), matched_documents.end(),
            [](const Document& lhs, const Document& rhs) {
                return lhs.relevance > rhs.relevance;
            });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

    int document_count_ = 0;

private:


    struct Query {
        set<string> min;
        set<string> plus;
    };

    map<string, map<int, double>> word_to_document_freqs_; // Ключ - Слово. Словарь из id дока и TF слова в этом доке

    set<string> stop_words_;


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

    Query ParseQuery(const string& text) const {
        Query query_words;
        for (const string& word : SplitIntoWordsNoStop(text)) {
            if (count(word.begin(), word.end(), '-') > 0) {
                const string minword = word.substr(1);
                if (stop_words_.count(minword) == 0) {
                    query_words.min.insert(minword);
                }

            }
            else {
                query_words.plus.insert(word);
            }
        }
        return query_words;
    }

    double CalculateTfIdfRelevance(const double& tf, const string& word) const {
        return tf * log((document_count_ + 0.0) / word_to_document_freqs_.at(word).size());
    }

    vector<Document> FindAllDocuments(const Query& query_words) const {
        map<int, double> document_to_relevance; // Ключ id найденного документа и релевантность документа.

        vector<Document> result;

        // map<string, map<int, double>> word_to_document_freqs_; Ключ - Слово. Словарь из id документа и TF слова в этом документе

        for (const string& word : query_words.plus) {
            if (word_to_document_freqs_.contains(word)) {
                for (const auto& [id, tf] : word_to_document_freqs_.at(word)) {
                    document_to_relevance[id] += CalculateTfIdfRelevance(tf, word);
                }
            }
        }

        for (const string& word : query_words.min) {
            if (word_to_document_freqs_.contains(word)) {
                for (const auto& [id, tf] : word_to_document_freqs_.at(word)) {
                    document_to_relevance.erase(id);
                }
            }
        }

        for (const auto& [key, relev] : document_to_relevance) {
            result.push_back({ key, relev });
        }
        return result;
    }
};

SearchServer CreateSearchServer() {
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());

    search_server.document_count_ = ReadLineWithNumber();
    for (int document_id = 0; document_id < search_server.document_count_; ++document_id) {
        search_server.AddDocument(document_id, ReadLine());
    }

    return search_server;
}

int main() {
    const SearchServer search_server = CreateSearchServer();

    const string query = ReadLine();
    for (const auto& [document_id, relevance] : search_server.FindTopDocuments(query)) {
        cout << "{ document_id = "s << document_id << ", "
            << "relevance = "s << relevance << " }"s << endl;
    }
}