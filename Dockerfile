FROM gcc:11.2.0
RUN mkdir -p /app/
COPY ./ /app/
WORKDIR /app/

RUN apt-get update && apt-get install -y --no-install-recommends cmake
RUN CFLAGS=-Wno-error
RUN cmake -S . -B build -D CMAKE_BUILD_TYPE=Release
RUN cmake --build build

FROM alpine:latest
RUN mkdir -p /app/
WORKDIR /app/
COPY --from=0 /app/build/CPPinger ./
CMD ["./CPPinger"]
