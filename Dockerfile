FROM alpine:latest

RUN apk add --no-cache wget\
    git\
    make\
    gcc\
    libc-dev

RUN mkdir -p /app
COPY src /app/src
COPY include /app/include
COPY t /app/t
COPY Makefile /app/Makefile

RUN cd /app; make
CMD ["/app/unit_test"]
