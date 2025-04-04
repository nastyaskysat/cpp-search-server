# Поисковый сервер (C++)
Сервер для поиска документов по запросу с поддержкой стоп-слов, релевантности и пагинации

## Возможности
- Добавление/удаление документов
- Поиск с учётом стоп-слов
- Исключение минус-слов из результатов
- Ранжирование по TF-IDF
- Разбиение на страницы
- Параллельный поиск (многопоточность)

## Пример использования
```
#include "search_server.h"
#include <iostream>

int main() {
    SearchServer server("in the at a"s);
    
    // Добавление документов
    server.AddDocument(1, "black cat in the city"s, DocumentStatus::ACTUAL, {1, 2, 3});
    server.AddDocument(2, "white dog in the village"s, DocumentStatus::ACTUAL, {1, 2});
    
    // Поиск документов
    for (const auto& doc : server.FindTopDocuments("black cat -city"s)) {
        std::cout << "Document id: " << doc.id 
                  << ", relevance: " << doc.relevance << std::endl;
    }
}
```

## Особенности реализации
- Многопоточность для параллельного выполнения поиска
- Оптимизация производительности с помощью хеш-таблиц и предварительных вычислений
- Гибкая система ранжирования на основе TF-IDF
- Поддержка разных статусов документов (ACTUAL, IRRELEVANT, BANNED, REMOVED)
