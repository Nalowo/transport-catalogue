# Учебный проект
# Транспортный справочник
Хранит список остановок и маршрутов, проходящих через остановки. 
Ввод и вывод - данные в формате json. 
Способен сериализовать и десериализовать собранную базу, строить маршрут от остановки А до остановки Б по графу, с расчетом времени в пути, выводить карту маршрутов в svg.

 * make_base – построение базы, ожидается base_requests.
 * process_requests – обработка запроса, так же можно передать массив с base_requests, база будет собрана и в с ней будет обработан stat_requests.

* Заполнение базы начинается с массива base_requests, который содержит в себе описания маршрутов – Bus и остановок – Stop.

```json
    {
        "is_roundtrip": true, // обозначение что маршрут кольцевой
        "name": "297",
        "stops": [ // через какие остановки проходит маршрут
            "Biryulyovo Zapadnoye",
            "Biryulyovo Tovarnaya",
            "Universam",
            "Biryulyovo Zapadnoye"
        ],
        "type": "Bus"
    },
    {
        "latitude": 55.574371,
        "longitude": 37.6517,
        "name": "Biryulyovo Zapadnoye",
        "road_distances": { // дистанция до соседних остановок
            "Biryulyovo Tovarnaya": 2600
        },
        "type": "Stop"
    },
```

* Node serialization_settings содержит в себе настройку для сериализации базы

```json
    "serialization_settings": {
        "file": "transport_catalogue.db" // имя файла куда будет сохранена база, если файла нет, он будет создан
    },
```

* Node render_settings содержит в себе настройки svg рендера

```json
    "render_settings": {
        "bus_label_font_size": 20,
        "bus_label_offset": [
            7,
            15
        ],
        "color_palette": [
            "green",
            [
                255,
                160,
                0
            ],
            "red"
        ],
        "height": 200,
        "line_width": 14,
        "padding": 30,
        "stop_label_font_size": 20,
        "stop_label_offset": [
            7,
            -3
        ],
        "stop_radius": 5,
        "underlayer_color": [
            255,
            255,
            255,
            0.85
        ],
        "underlayer_width": 3,
        "width": 200
    },
```

* Node routing_settings содержит настройки для графа вычисляющего маршруты.

```json
    "routing_settings": {
        "bus_velocity": 40, // скорость автобуса, считаю, что она неизменная на всем маршруте
        "bus_wait_time": 6 // время ожидания на остановке
    },
```

* Node stat_requests содержит в себе запросы к базе.

```json
    { // запрос на получение информации об маршруте
        "id": 1,
        "name": "297",
        "type": "Bus"
    },
    { // запрос на получение информации об остановке
        "id": 3,
        "name": "Universam",
        "type": "Stop"
    },
    { // запрос на построение маршрута от остановки А и до остановки Б
        "from": "Biryulyovo Zapadnoye",
        "id": 4,
        "to": "Universam",
        "type": "Route"
    },
    {  // запрос на построение карты в svg
      "id": 1,
      "type": "Map"
    }
```

* На вывод будет получен json массив в котором:

```json
    { // вывод информации об маршруте 
        "curvature": 1.42963, // извилистость, то есть отношение фактической длины маршрута к географическому расстоянию
        "request_id": 1,
        "route_length": 5990,
        "stop_count": 4,
        "unique_stop_count": 3
    },
    { // вывод ответа на запрос информации об остановке
        "buses": [ // список маршрутов следующих через остановку
            "297",
            "635"
        ],
        "request_id": 3
    },
    { // ответ на запрос построение маршрута от А до Б
        "items": [
            {
                "stop_name": "Biryulyovo Zapadnoye",
                "time": 6,
                "type": "Wait" // ожидание на остановке
            },
            {
                "bus": "297",
                "span_count": 2, // сколько остановок нужно проехать на этом маршруте
                "time": 5.235, // время в пути
                "type": "Bus"
            }
        ],
        "request_id": 4,
        "total_time": 11.235
    },
    { // ответ на запрос построения карты
        "map": "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n<svg…
```

Полные варианты входных данных в папке example.

# Компиляция

Для компиляции вам понадобится собранная библиотека Protobuf 3.14 и новей.

Нужно в директории src создать директорию build и выполнить в ней команды:

cmake .. -DCMAKE_PREFIX_PATH=<абсолютный путь до директории>\Protobuf -DCMAKE_BUILD_TYPE=Debug/Release -G "MinGW Makefiles"

cmake --build .

* Собиралось на:
g++.exe (MinGW-W64 x86_64-ucrt-posix-seh, built by Brecht Sanders) 12.2.0
