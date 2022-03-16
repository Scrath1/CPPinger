# CPPinger
A network utility written in C++ to monitor the connection to a network target using pings
that documents events such as network timeouts or high response times.

## Dependencies
 - [cpp-icmplib](https://github.com/markondej/cpp-icmplib) (automatically included in the source files)
 - [Channel](https://github.com/andreiavrammsd/cpp-channel) (automatically included via CMakeLists)
 - [yaml-cpp](https://github.com/jbeder/yaml-cpp) (automatically included via CMakeLists)
 - libpq-dev (Install using `sudo apt install libpq-dev`)

## config.yml
Each pinger container should have its own config file like this
```yaml
pinger:
  interval: 2
  target: 192.168.1.1
  pingwarning_threshold: 75
  verbosity: 1
logger:
  use_logger: true
  logfile_name: "logfile.log"
database:
  use_database: true
  host: localhost
  port: 5432
  dbname: cppinger
  user: cppinger
  password: example
```
### Parameters

| Category | Setting              | Values           | Description                                                                                                                                                      |
|----------|----------------------|------------------|------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| pinger   | interval             | 1..n             | The interval in seconds between each ping                                                                                                                        |
|          | target               | IPv4 or hostname | Address to send pings to                                                                                                                                         |
|          | pingwarning_treshold | 0..n             | The value in ms above which a high ping event is registered                                                                                                      |
|          | verbosity            | 0,1,2            | Verbosity of console output. 0 = Only control information, 1 = control information + detected events ,2 = control information + detected events + ping responses |
| logger   | use_logger           | true, false      | Enables or disabled event logging to a logfile                                                                                                                   |
|          | logfile_name         | any filename     | The logfile to write events to                                                                                                                                   |
| database | use_database         | true, false      | Enables or disables event logging to the database                                                                                                                |
|          | host                 | IP or hostname   | The IP or hostname of the database                                                                                                                               |
|          | port                 | any number       | The port used by the database                                                                                                                                    |
|          | dbname               | any string       | The database name used when connecting to the database. Make sure this is the same as the one used in the docker-compose.yml.                                    |
|          | user                 | any string       | The username used when connecting to the database. Make sure this is the same as the one used in the docker-compose.yml                                          |
|          | password             | any string       | The password used when connecting to the database. Make sure this is the same as the one used in the docker-compose.yml                                          |

## Multiple Ping Targets
If you want to monitor your connection to multiple hosts you can add additional cppinger containers in the `docker-compose.yml`.

### Example:
In this example we have 2 pinger containers each with their own config file located in the config directory.
cppinger1 uses the pinger1.yml and cppinger2 uses the pinger2.yml file.

If you don't need a logfile you can disable the logger in the containers config file and leave out the logs volume mapping.
```yaml
version: '3'
services:
  cppinger1:
    build:
      context: ./
      dockerfile: Dockerfile
    volumes:
      - ./config/pinger1.yml:/app/config/config.yml
      - ./logs:/app/logs
    restart: unless-stopped

  cppinger2:
    build:
      context: ./
      dockerfile: Dockerfile
    volumes:
      - ./config/pinger2.yml:/app/config/config.yml
      - ./logs:/app/logs
    restart: unless-stopped

  timescaledb:
    image: timescale/timescaledb:latest-pg14
    ports:
      - '5432:5432'
    volumes:
      - ./timescale-data:/var/lib/postgresql/data
    restart: unless-stopped
    environment:
      TIMESCALEDB_TELEMETRY: 'off'
      POSTGRES_USER: cppinger
      POSTGRES_PASSWORD: example
      POSTGRES_DB: cppinger
```