#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <cmath>
#include <cassert>
#include <optional>

 
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
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }
 
    return words;
}
enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};
 
struct Document {
    Document() = default;
 
    Document(int id, double relevance, int rating): id(id), relevance(relevance), rating(rating) {}
 
    int id = 0;
    double relevance = 0.0;
    int rating = 0;
};
 
template <typename StringContainer>
set<string> MakeUniqueNonEmptyStrings(const StringContainer& strings) {
    set<string> non_empty_strings;
    for (const string& str : strings) {
        if (!str.empty()) {
            non_empty_strings.insert(str);
        }
    }
    return non_empty_strings;
}
 

                                                //CLASS//
class SearchServer {
                                                    //PUBLCiC//
public:
    inline static constexpr int INVALID_DOCUMENT_ID = -1;
    SearchServer() = default;
    
    template <typename StringContainer>
   explicit SearchServer(const StringContainer& stop_words) :stop_words_ (MakeUniqueNonEmptyStrings(stop_words)){
   for (const auto& word : stop_words) {
        if (!IsValidWord(word)) {
            throw invalid_argument("Недопустимые символы");
        }
    }
    
    }
 
    explicit SearchServer(const string& stop_words_text) : SearchServer(SplitIntoWords(stop_words_text)){}
    
void AddDocument(int document_id, const string& document, DocumentStatus status, const vector<int>& ratings) {
        if (document_id < 0) {
            throw invalid_argument("Отрицательный индекс"s);
        }
        if (documents_.count(document_id)) {
            throw invalid_argument("Такой индекс уже существует"s);
        }
        if (!IsValidWord(document)) {
            throw invalid_argument("Недопустимые символы"s);
        }
 
        const vector<string> words = SplitIntoWordsNoStop(document);
        for (auto& word : words) {
            word_to_document_freqs_[word][document_id] += 1.0 / words.size();
        }
        documents_.emplace(document_id, DocumentData { ComputeAverageRating(ratings), status });
        
        documents_index_.push_back(document_id);
 
    }
    
            
 
    
 
    template <typename DocumentPredicate>
    vector<Document> FindTopDocuments(const string& raw_query, DocumentPredicate document_predicate) const {
        Query query = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query, document_predicate);
 
        sort(matched_documents.begin(), matched_documents.end(),
            [](const Document& lhs, const Document& rhs) {
                const double EPSILON = 1e-6;
                if (abs(lhs.relevance - rhs.relevance) < EPSILON) {
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
        
    vector<Document> FindTopDocuments(const string& raw_query, DocumentStatus status) const {
        return FindTopDocuments(raw_query,
            [&status](int document_id, DocumentStatus new_status, int rating) {
                return new_status == status;
            });
        }
 
    vector<Document> FindTopDocuments(const string& raw_query) const {
        return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
        }  
 
    int GetDocumentCount() const {
        return static_cast<int>(documents_.size());
        }
       
    
    int GetDocumentId(int index) const {
          
      if ((index >= 0) && (index < GetDocumentCount())) { 
            return documents_index_.at(index);
        } else { 
            throw out_of_range("Недопустимый размер индекса"s); 
        } 
        }
 
 
   tuple<vector<string>, DocumentStatus> MatchDocument(const string& raw_query, int document_id) const {
        Query query = ParseQuery(raw_query);
 
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
 
        return { matched_words, documents_.at(document_id).status };
    }
 
  
 
   
    
                                                    //PRiVATE//
private:
    
      struct QueryWord {
        string data;
        bool is_minus;
        bool is_stop;
    };
 
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };

    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };
    
    const set<string> stop_words_;
    map<string, map<int, double>> word_to_document_freqs_;
    map<int, DocumentData> documents_;
    vector<int> documents_index_;
 
    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }
    

    static bool IsValidWord(const string& word) {
       return none_of(word.begin(), word.end(), [](char c) {
            return c >= '\0' && c < ' ';
        });
    }
     //static bool IsValidWord(const std::string& word) {
    //for (char ch : word) {
      //  if (ch >= '0' && ch < ' ') {
         //   return false; 
        //}
  //  }
  //  return true; 
//}
         
 
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
        int rating_sum = 0;
        for (const int rating : ratings) {
            rating_sum += rating;
        }
        return rating_sum / static_cast<int>(ratings.size());
    }
 
 
 
QueryWord ParseQueryWord(string text) const {
        QueryWord result;
  bool is_minus = false;
        if (text.empty()) {
            throw invalid_argument("Пустая строка"s);
        }
       
        if (text[0] == '-') {
            is_minus = true;
            text = text.substr(1);
        }
         if (text.empty()) {
            throw invalid_argument("Пробел или нет текста после знака \"-\""s);
        } else if (text[0] == '-') {
            throw invalid_argument("Удвоенное \"-\" в минус-слове"s);
        } else if (!IsValidWord(text)) {
            throw invalid_argument("Слово содержит недопустимый символ"s);
        }
 
        return { text, is_minus, IsStopWord(text) };
    }
    
       Query ParseQuery(const string& text) const {
        Query result;
 
        for (const string& word : SplitIntoWords(text)) {
            QueryWord query_word = ParseQueryWord(word);
            if (!query_word.is_stop) {
                if (query_word.is_minus) {
                    result.minus_words.insert(query_word.data);
                }
                else {
                    result.plus_words.insert(query_word.data);
                }
            }
        }
        return result;
    }
    
 
    // Existence required
    double ComputeWordInverseDocumentFreq(const string& word) const {
        return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
    }
 
 
    template <typename DocumentPredicate>
    vector<Document> FindAllDocuments(const Query& query,
                                      DocumentPredicate document_predicate) const {
        map<int, double> document_to_relevance;
        for (const string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
            for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {
                const auto& document_data = documents_.at(document_id);
                if (document_predicate(document_id, document_data.status, document_data.rating)) {
                    document_to_relevance[document_id] += term_freq * inverse_document_freq;
                }
            }
        }
 
        for (const string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
                document_to_relevance.erase(document_id);
            }
        }
 
        vector<Document> matched_documents;
        for (const auto [document_id, relevance] : document_to_relevance) {
            matched_documents.push_back(
                {document_id, relevance, documents_.at(document_id).rating});
        }
        return matched_documents;
    }
};
 
// ==================== для примера =========================
 
void PrintDocument(const Document& document) {
    cout << "{ "s
         << "document_id = "s << document.id << ", "s
         << "relevance = "s << document.relevance << ", "s
         << "rating = "s << document.rating << " }"s << endl;
}

int main() {
    SearchServer search_server("и в на"s);

    search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, {8, -3});
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, {7, 2, 7});
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, {5, -12, 2, 1});
    search_server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, {9});

    cout << "ACTUAL by default:"s << endl;
    for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s)) {
        PrintDocument(document);
    }

    cout << "BANNED:"s << endl;
    for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::BANNED)) {
        PrintDocument(document);
    }

    cout << "Even ids:"s << endl;
    for (const Document &document :
         search_server.FindTopDocuments("пушистый ухоженный кот"s,
                                        [](int document_id, DocumentStatus status, int rating) {
                                            return document_id % 2 == 0;
                                        }))  //
    {
        PrintDocument(document);
    }
}