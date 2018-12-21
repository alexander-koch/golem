FROM frolvlad/alpine-gcc
COPY . /app
WORKDIR /app
RUN apk add --update make
RUN make install