# Empirical Traffic Generator
## What is this?

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
The configuration file also allows several customizations including the 
desired average receive throughput and the number of requests.

## Quick Start

- Run make
- Start server
```
./bin/server -p 5050 >> /dev/null &
```
- Start client
```
./bin/client -c exampleConfig1 -l log -s 123
```

## Build

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
./client -c exampleConfig1 -l log -s 123
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

The configuration file specifies the list of servers for the client, the 
request size and request fanout distributions, average load, and the number 
of requests.

The format is a sequence of key and value(s), one key per line. The permitted
keys are:

* **server:** ip address and port of an active server.
```
server localhost 5050
server 192.168.0.1 5000
```

* **req_size_dist:** request size distribution file path and name.
```
req_size_dist /home/jsmith/empirical-traffic-gen/DCTCP_CDF
```

There must be one request size distribution file, present at the given path, 
which specifies the CDF of the request size distribution. See "DCTCP_CDF" 
for an example with proper formatting.

* **fanout:** fanout value and weight. The fanout and weight are both 
integers.
```
fanout 1 50
fanout 2 30
fanout 8 20
```

The fanout values must be no more than the number of available servers. For 
each request, the client chooses a fanout with a probability proportional to 
the weight. For example, with the above configuration, half the requests have
fanout 1, and 20% have fanout 8.

* **load:** average RX throughput at the client in Mbps.
```
load 1000Mbps
```

There must only be one load in the configuration file. A special case is:
```
load 0
```
Here, the client makes requests back-to-back as quickly as possible.

The client generates requests to roughly match the desired average throughput. 
If the desired load is high or the CPU is busy, the timing between requests may 
become innacurate and the actual throughput can be lower than desired. The client
outputs the actual throughtput upon termination.

* **num_reqs:** the total number of requests.
```
num_reqs 1500
```


## Output

A successful run creates two output files: $pre_reqs.out and $pre_flows.out, 
where $prefix is the string provided via command line. The two files give 
the size (in bytes) and completion time (in microseconds) for all requests and 
flows, respectively. If the fanout is always 1, requests and flows (and hence 
the outputs) are identical.