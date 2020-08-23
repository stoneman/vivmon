# Vivmon

Environmental monitoring for a vivarium

![Screenshot](https://raw.githubusercontent.com/stoneman/vivmon/main/screenshot.jpg)

## Architecture

- An [Arduino sketch](/esp8266-temp-humidity-uv-service.ino) designed to run on an ESP8266 based board (developed with an ESP-12F).

- A [Grafana Dashboard](/graphana-dashboard.json) to display current and historical environmental statistics graphically.

- Something to collect JSON data from the ESP board's web service and to act as a data source for the Grafana Dashboard. InfluxDB can be used as the data source with Telegraf retriving the stats and inserting them into InfluxDB. Telegraph can gather the data using the `httpjson` input plugin:


```
[[inputs.httpjson]]
    name = "climatestats"
    servers = [
      "http://192.168.1.49"
    ]
    response_timeout = 5
    method = "GET"
```

## Roadmap

- Add alerting when undesired environmental values are measured.
