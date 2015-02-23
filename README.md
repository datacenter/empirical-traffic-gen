# What is this?

This is a simple client/server application for generating user defined 
traffic patterns.

The server is simple: It listens for incoming requests on the specified 
ports, and echos a 'message' with the requested flow size for each request.

The client connects to a list of servers (given in the input configuration 
file), and generates requests to randomly chosen servers. For each request, 
it samples from the input flow size distribution (also given in the configuration 
file) to get the flow size for the request. The configuration file also allows 
several customizations, importantly the desired average receive throughput, 
and a fanout distribution for generating incast-type requests simultaneously 
to multiple servers.

## Building

In the main directory, run:

```
make 
```

Two binaries, **client** and **server**, will be added under ./bin . The build
has been tested with g++ 4.6.3 on Ubuntu, but the code has minimal dependencies 
and should build on any Linux system. It does need pthread and tr1 libraries.

## Command Line Arguments

#### Server
Example: ./server -p 5050

   **-p  :  port** on which the server should listen (optional)
           default: 5000

#### Client
Example: ./client -c exampleFile1 -o out -s 123

   **-c : config** file 
