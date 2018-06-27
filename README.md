# quintet
An implementation of the Raft consensus protocol.

## Feature

1. Raft is a consensus algorithm for managing a replicated log, which means keeping the contents consistent on each server of a cluster by synchronizing the operations between each server. 
2. Quintet is very suitable for preventing single point of failure for some large distributed system like GFS. By slightly modifying the code of the single point, and running it on a cluster of several servers, as long as more than half of the cluster are still working, the content, no matter it stores in disk or memory, in each server would maintain consistent and correct.
3. In our implementation, we provide a library with a few interfaces and hide almost all of the details in the background, so that the usage would be quite easy.
4. Equiped by c++, boost and grpc, quintet is relatively efficient. And quintet can output pretty logs for debugging.

## Usage

There are many situation suitable for this implementation.

1. GFS (Google File System) : In GFS, there is a single server, called master, storing the meta data for the whole distributed system and responding to all of the request from the cluster. The backup of the master will be very important for the system, and this is where quintet comes in.
2. DLM (Distributed Lock Manager): In many distributed system sharing resources, a distributed lock manager runs in every machine in a cluster, with a identical copy of cluster-wide lock. And the quintet can help the designer of DLM to keep locks on each server consistent.
3. Distributed Chat Room: In our example, we implement a distributed chat room, in which the users holds the service of the chat room by themselves, and no external server is needed.


## Get started

### Prerequest

* c++14+
* [cmake 3.5+](https://cmake.org/)
* [boost 1.67.0+](https://github.com/grpc/grpc)
* [grpc](https://github.com/grpc/grpc) 

### How to use
Include the [Interface.h](./include/quintet/Interface.h) header file into the source code, and assume the operations you want to synchronize among the servers are some functions:
```c++
    int foo(int a);
    void addEntry(string s, int b);
```
The following are the steps to do:

1. Declare a interface
    ```c++
        quintet::Interface interface;
    ```
2. Bind the operating functions to the interface at the begining of the code  
    ```c++
        interface.bind("foo", [](int a) -> int {...});
        interface.bind("addEntry", [](string s, int b) -> void {...});
    ```
3. Now whenever you want to call the operating function, you wish to execute on every server, use the call function:
    ```c++
        interface.call("foo", 15);     // 15 is the arguement for the foo function
        interface.call("addEntry", "hello, world", 2);
    ```
4. Everything will be taken care of by the quintet library, and you can now write the code as you want. 

On each server, start the same code (or you can customize the operating function on each server and do something interesting), with different config file, the whole system will work properly. A specific example can be found in [chatRoom.cpp](./example/chatRoom.cpp). With a really brief code, a distributed chat room is built.
