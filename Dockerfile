FROM ubuntu as build

RUN apt-get update && \
    apt-get install -y cmake gcc make g++ && \
    apt-get install -y libev-dev

ADD . .
COPY ./httptest /var/www/html/httptest


RUN chmod +x build.sh && ./build.sh
RUN echo "install this"
RUN touch /var/log/err.log
RUN ln -sf /dev/stdout /var/log/err.log
#RUN ln -sf /dev/stdout /var/log/nginx/access.log \
#    && ln -sf /dev/stderr /var/log/nginx/error.log

WORKDIR /build

EXPOSE 80

ENTRYPOINT ["./web_server","-c","4","-r","/var/www/html"]
