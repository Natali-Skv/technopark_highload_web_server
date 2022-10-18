# technopark_highload_web_server
### индивидуальное задание по курсу Highload: [[требования+тесты]](https://github.com/init/http-test-suite)
* **Разработать веб-сервер для отдачи статики с диска,**  
* **проверить стабильность и корректность работы,**   
* **провести сравнительное нагрузочное тестирование с nginx.**  

    

``` asm
$ sudo docker build -t web_server .
$ sudo docker run -p 80:80 web_server
```
``` asm
$ mkdir build
$ cd build
$ cmake ..
$ make
$ sudo ./web_server -c 4 -r /home/ns/tp/hl/tests_for_web_server/
```

### Результаты нагрузочного тестирования написанного веб-сервера.
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

### Результаты нагрузочного тестирования nginx.
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
Time taken for tests:   164.636 seconds
Complete requests:      100000
Failed requests:        0
Total transferred:      95506200000 bytes
HTML transferred:       95482400000 bytes
Requests per second:    607.40 [#/sec] (mean)
Time per request:       1.646 [ms] (mean)
Time per request:       1.646 [ms] (mean, across all concurrent requests)
Transfer rate:          566508.62 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   0.2      0      25
Processing:     1    1   1.2      1     211
Waiting:        0    1   0.7      0      55
Total:          1    2   1.2      1     211
WARNING: The median and mean for the waiting time are not within a normal deviation
        These results are probably not that reliable.

Percentage of the requests served within a certain time (ms)
  50%      1
  66%      1
  75%      2
  80%      2
  90%      2
  95%      3
  98%      4
  99%      5
 100%    211 (longest request)

```

| метрика                      | nginx   | написанный веб-сервер |
|------------------------------|---------|-----------------------|
| rps                          | 1305.54 | 1956.68               |
| Time per request [ms] (mean) | 7.660   | 5.111                 |
