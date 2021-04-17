#pragma once

enum class syscall_codes
{
    NULL_SYSCALL = 0, // don't use >:^(

    // 1 2 & 3 are free for use rn

    GET_PROCESS_GLOBAL_DATA = 4, // get process global data, if arg1 (target) is nullptr, return self global data, else return a process global data return -1 if there is an error

    MEMORY_ALLOC = 6, // pmm alloc
    MEMORY_FREE = 7,  // pmm free

    FILE_OPEN = 8,   // open a file
    FILE_CLOSE = 9,  // close a file
    FILE_READ = 10,  // read file
    FILE_WRITE = 11, // write file
    FILE_SEEK = 12,  // seek a file

    NANO_SLEEP = 13, // sleep the current process

    GET_PID = 14, // get current pid

    EXEC = 15, // execute a new programm

    EXIT = 16, // current process exit

    IPC_SERV_EXIST = 17,         // check if a server exist
    CREATE_SERVER = 18,          // try to create a new server
    CONNECT_SERVER = 19,         // (client) try to connect to a server
    ACCEPT_CONNECTION = 20,      // (server) try to accept a new connection
    IS_CONNECTION_ACCEPTED = 21, // (client) is the connection accepted by the server
    DECONNECT = 22,              // (client) deconnect from a server
    SEND = 23,                   // send data to a client/server
    RECEIVE = 24,                // receive data from a client/server
};

#define SYS_ALLOC_SHARED 0x2
