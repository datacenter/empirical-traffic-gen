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

## How to build?

In the main directory, run:

   make 

Two binaries, client and server, will be added under ./bin .

## Example Usage

Example usage, with both processes on the same machine:

Server:
$ ./server -p 5050

Client:
$ ./client -c configLocal -l test -s 123

