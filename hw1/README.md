# HW 1

## Запуск

```
g++ -std=c++17 -O0 -o main main.cpp
./main

```

## Пояснение

Для размера кеша и ассоциативности использовал алгоритм из [статьи "Robust Method to Determine Cache and TLB Characteristics"](https://etd.ohiolink.edu/acprod/odb_etd/ws/send_file/send?accession=osu1308256764&disposition=inline)

Для поиска длины линейки я придумал алгоритм:

Постепенно увеличиваем `line_size`, будем проходится по определённым линейкам так, чтобы когда у нас `line_size <= `настоящей, то линейки с тегами заканчивающимися на X попадали в сет по с индексом X, а иначе и 