    #include "document.h"

    std::ostream& operator<<(std::ostream& out, const Document& document) {
        using namespace std;
        out << "{ document_id = "s << document.id << ", relevance = "s << document.relevance << ", rating = "s << document.rating << " }"s;
        return out;
    }
    
    Document::Document() = default;

    Document::Document(int id, double relevance, int rating)
        : id(id)
        , relevance(relevance)
        , rating(rating) {
    }