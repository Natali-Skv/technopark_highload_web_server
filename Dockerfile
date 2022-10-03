FROM ubuntu as build

RUN apt-get update && \
    apt-get install -y cmake gcc make g++ && \
    apt-get install -y libev-dev

ADD . .
COPY ./httptest /var/www/html/httptest


RUN chmod +x build.sh && ./build.sh

WORKDIR /build

EXPOSE 80

ENTRYPOINT ["./web_server","-c","4","-r","/var/www/html"]
