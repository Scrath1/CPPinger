FROM gcc:11.2.0
RUN mkdir -p /app/config
RUN mkdir -p /app/logs
COPY ./ /app/
WORKDIR /app/
RUN apt-get update && apt-get install -y --no-install-recommends cmake libpq-dev postgresql-server-dev-all
RUN cmake -S . -B build -D CMAKE_BUILD_TYPE=Release
RUN cmake --build build || :
RUN cp /app/build/CPPinger /app/
RUN rm -r build
CMD ["/app/CPPinger"]