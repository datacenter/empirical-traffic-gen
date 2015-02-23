# What is this?

This is a simple client/server application for generating user defined 
traffic patterns.

The server is simple: It listens for incoming requests on the specified 
ports, and replies with a *flow* with the requested size for each request.

The client connects to a list of servers (given in the input configuration 
file), and generates requests to randomly chosen servers. For each request, 
it samples from the input request size and fanout distributions (also specified 
in the configuration file) to determine the request size and how many flows 
to generate in parallel for the request. The fanout generates synchronized 
*incast-like* flows from a random subset of the servers towards the client.
The configuration file also allows several customizations, importantly, the 
desired average receive throughput and the number of requests.

## Building

In the main directory, run:

```
make 
```

Two binaries, **client** and **server**, will be added under ./bin . The build
has been tested with g++ 4.6.3 on Ubuntu, but the code has minimal dependencies 
and should compile on any Linux system. It does need pthread and tr1 libraries.

## Command Line Arguments

### Server
Example: 
```
./server -p 5050
```
   **-p : port** on which the server should listen *(optional)*<br>
          *default: 5000*

   **-h :** print commnad line usage information and exit

### Client
Example: 
```
./client -c exampleFile1 -l log -s 123
```
   **-c : config** file name *(required)*<br>
          The configuration file specifies the workload characteristics, and 
          must be formatted as described below.

   **-l : log** file name prefix *(optional)*<br>
          The prefix is used for the two output files with flow and request
          completion times.<br>
          *default: log*

   **-s : seed** value *(optional)*<br>
          The seed is used for various random number generators. It is 
          recommended that different clients use different seeds to avoid 
          synchronization.<br> 
          *default: set seed based on current machine time (not repeatable)*

   **-h :** print commnad line usage information and exit

## Client Configuration File

The client configuration file specifies the list of servers to connect to, 
the request size and request fanout distributions, average load, and the 
number of requests.

The format is a sequence of key and value(s), one key per line. The permitted
keys are:

* **server:** ip address and port of an active server; e.g., 
```
server localhost 5050
server 192.168.0.1 5000
```

* **req_size_dist:** request size distribution file path and name; e.g.,
```
req_size_dist /home/jsmith/empirical-traffic-gen/DCTCP_CDF
```
The file must exist at the given path and specifies the CDF of the request 
size distribution. See "DCTCP_CDF" for an example with proper formatting.

