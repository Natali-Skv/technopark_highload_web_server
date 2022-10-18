# Веб-сервер для отдачи статики с диска
### Индивидуальное задание по курсу Highload: [[требования+тесты]](https://github.com/init/http-test-suite)
* **разработать веб-сервер для отдачи статики с диска**  
* **проверить стабильность и корректность работы**   
* **провести сравнительное нагрузочное тестирование с nginx**  

### Используемые технологии:
- архитектура: **prefork**
- язык программирования: **C**
- библиотека, реализующая event loop **[libev](https://github.com/enki/libev)**
- метод мониторинга сетевых событий: **epoll**

### Использование:
```
./web_server [OPTIONS]

опция                   аргумент   значение по умолчанию                   описание
-h  --help                         -                                       Посмотреть возможные опции   
-c, --cpu_limit         [NUM]      _SC_NPROCESSORS_ONLN                    Установить ограничение   
-d, --required_argument [PATH]     "/home/ns/tp/hl/tests_for_web_server/"  Установить путь к корневой директории  
-s, --server_log        [PATH]     "s_log_"                                Установить префикс пути к лог-файлу сервера, полный путь до файла: server_log + process_pid  
-a, --access_log        [PATH]     "a_log_"                                Установить префикс пути к access-лог-файлу сервера, полный путь до файла: access_log + process_pid   

[--help|--document_root <path>|--cpu_limit <num>|access_log <path>|server_log <path>] [-r <path>|-c <num>|-a <path>|-s <path>]
```
  
При запуске сервера создается *cpu_limit* процессов, которые начинают обрабатывать запросы.  
При запуске сервера создаются файлы логов, по два на каждый процесс.   
  
Имя файла -- конкатенация префикса пути, переданного в аргументах и PID данного процесса.  
Например, при запуске с помощью команды `./web_server -с 1 --access_log "/var/log/access_" --server_log "/var/log/server_"` будет создано 2 лог файла, например таких: "/var/log/access_12345","/var/log/server_log_12345".  
   
**Структура server-log:**  
В сервер-лог файл записываются: информационные логи(старт, завершение процесса), ошибки выделения памяти, информация о внутренних ошибках сервера (500) при выполнении запроса пользователя.  

```
log-type date:                     PID:   message
[INFO]   Tue Oct 18 19:29:54 2022: 85951: process with pid 85951, root directory /home/ns/tp/hl/tests_for_web_server/ started

log-type  PID-request_id: date:                     message:                errno message  
[R_ERROR] 75706-1243:     Tue Oct 18 17:51:03 2022: error sending response: Error range 
```
   
**Структура access-log:**  
В access-лог файл записывается информация о запросах пользователей.    
Access-лог файл заполняется через буфер, то есть последние логи могут не появиться, пока либо не заполнится буфер, то есть придет еще несколько запросов, либо не завершится процесс.  
```
PID-request_id: date:                     first line of http request:                   response status code: ms for processing request: ms for receiving, processing, getting request 
75706-1243:     Tue Oct 18 17:51:03 2022: GET /httptest/wikipedia_russia.html HTTP/1.0: 200:                  100:                       230
```

### Запуск через docker:
``` asm
$ sudo docker build -t web_server .
$ sudo docker run -p 80:80 web_server
```

### Запуск из директории проекта:
``` asm
$ mkdir build
$ cd build
$ cmake ..
$ make
$ sudo ./web_server -c 4
```

### Результаты нагрузочного тестирования написанного веб-сервера.
Сервер был запущен командой `sudo docker run --cpus=4 -p 80:80 web_server`, с cpu_limit=4  
<details>
  <summary>:purple_circle: Развернуть результаты нагрузочного тестирования написанного веб-сервера!</summary>   
     
``` asm
$ ab -n 100000 127.0.0.1:80/httptest/wikipedia_russia.html
This is ApacheBench, Version 2.3 <$Revision: 1879490 $>
Copyright 1996 Adam Twiss, Zeus Technology Ltd, http://www.zeustech.net/
Licensed to The Apache Software Foundation, http://www.apache.org/

Benchmarking 127.0.0.1 (be patient)
Completed 10000 requests
Completed 20000 requests
Completed 30000 requests
Completed 40000 requests
Completed 50000 requests
Completed 60000 requests
Completed 70000 requests
Completed 80000 requests
Completed 90000 requests
Completed 100000 requests
Finished 100000 requests


Server Software:        web
Server Hostname:        127.0.0.1
Server Port:            80

Document Path:          /httptest/wikipedia_russia.html
Document Length:        954824 bytes

Concurrency Level:      1
Time taken for tests:   94.832 seconds
Complete requests:      100000
Failed requests:        0
Total transferred:      95495600000 bytes
HTML transferred:       95482400000 bytes
Requests per second:    1054.49 [#/sec] (mean)
Time per request:       0.948 [ms] (mean)
Time per request:       0.948 [ms] (mean, across all concurrent requests)
Transfer rate:          983393.57 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   0.0      0       1
Processing:     1    1   0.1      1       8
Waiting:        0    0   0.1      0       7
Total:          1    1   0.1      1       8

Percentage of the requests served within a certain time (ms)
  50%      1
  66%      1
  75%      1
  80%      1
  90%      1
  95%      1
  98%      1
  99%      1
 100%      8 (longest request)

```   
</details>

### Результаты нагрузочного тестирования nginx.
Nginx был запущен командой `sudo docker run --cpus=4 -p 8888:8888 nginx_server` с worker_processes=4 и access_log=off  

<details>
  <summary>:purple_circle: Развернуть результаты нагрузочного тестирования написанного веб-сервера!</summary> 
  
``` asm
$ ab -n 100000 127.0.0.1:8888/httptest/wikipedia_russia.html
This is ApacheBench, Version 2.3 <$Revision: 1879490 $>
Copyright 1996 Adam Twiss, Zeus Technology Ltd, http://www.zeustech.net/
Licensed to The Apache Software Foundation, http://www.apache.org/

Benchmarking 127.0.0.1 (be patient)
Completed 10000 requests
Completed 20000 requests
Completed 30000 requests
Completed 40000 requests
Completed 50000 requests
Completed 60000 requests
Completed 70000 requests
Completed 80000 requests
Completed 90000 requests
Completed 100000 requests
Finished 100000 requests


Server Software:        nginx/1.23.1
Server Hostname:        127.0.0.1
Server Port:            8888

Document Path:          /httptest/wikipedia_russia.html
Document Length:        954824 bytes

Concurrency Level:      1
Time taken for tests:   94.448 seconds
Complete requests:      100000
Failed requests:        0
Total transferred:      95506200000 bytes
HTML transferred:       95482400000 bytes
Requests per second:    1058.78 [#/sec] (mean)
Time per request:       0.944 [ms] (mean)
Time per request:       0.944 [ms] (mean, across all concurrent requests)
Transfer rate:          987500.68 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   0.0      0       1
Processing:     1    1   0.7      1     206
Waiting:        0    0   0.1      0      14
Total:          1    1   0.7      1     206

Percentage of the requests served within a certain time (ms)
  50%      1
  66%      1
  75%      1
  80%      1
  90%      1
  95%      1
  98%      1
  99%      1
 100%    206 (longest request)

```

</details>

| метрика                      | nginx   | написанный веб-сервер |
|------------------------------|---------|-----------------------|
| rps                          | 1058.78 | 1054.49               |
| Time per request [ms] (mean) | 0.944   | 0.948                 |
