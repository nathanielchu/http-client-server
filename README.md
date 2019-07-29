# HTTP client server

## Overview

Implements a HTTP Client and Server using BSD sockets without high-level network-layer abstractions related to networking. 

## Features

- Configurable web-server hostname, port number, and directory name
- Web-server handles concurrent connections to serve multiple clients
- Implements HTTP GET method

## Installation

```shell
make
```

Builds the web-server and web-client.

## Developing

```shell
./serve.sh
```

serve.sh is a script to make the web-server and web-client and then runs them.

## Usage

```shell
./web-server [hostname] [port] [file-dir]
```

```shell
./web-client [URL] [URL]...
```