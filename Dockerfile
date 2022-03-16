FROM gcc:11.2.0 AS builder
RUN mkdir -p /app/
COPY ./ /app/
WORKDIR /app/

RUN apt-get update && apt-get install -y --no-install-recommends cmake libpq-dev postgresql-server-dev-all
RUN cmake -S . -B build -D CMAKE_BUILD_TYPE=Release
RUN cmake --build build || :
#CMD ["/app/build/CPPinger"]

FROM scratch
COPY --from=builder /app/build/CPPinger ./
CMD ["/CPPinger"]
