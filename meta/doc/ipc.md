# IPC


WingOS ipc tries to implement the quick path of SeL4 while maintaining asynchronous messaging.

We want two use cases: 
- asynchronous messaging, for networking, fs, ...
- synchronous messaging, for devices, general ipc

## How SeL4 works ?

In SeL4 messages are sended synchronously, we have :
- `SeL4_Send` => Blocks until a receiver is able to receive 
- `SeL4_Recv` => Blocks until a message is available
- `SeL4_Reply` => Reply to a received message 
- `SeL4_Call` => seL4 Send + Recv
- `SeL4_SendNB` => Non-blocking send
- `SeL4_RecvNB` => Non-blocking recv

Thus a message is never stored in memory. We are not able to receive asynchronously.

It is great to avoid memory allocation, and is able to give us fast path IPC.

You are not able to have `SeL4_SendNB` and `SeL4_RecvNB` together, as they are blocking calls.

## How asynchronous messaging works

Generally, every message are sended in a vector of messages. The receiver is notified when a message is available.

## How wingOS works

We have two kind of message: 
- One with expected reply
- One without expected reply

When sending a message with expected reply (like `read, receive_packet, get_status`), you are forced to work like the SeL4 way. Not able to have an asynchronous send/receive.

When sending a message without any expected reply (like `write, flush, send_packet, close...`) you are able to not wait for a reply.

WingOS makes a distinction between call and sending data. 

- `wingOS_call` => Synchronous call, will block until receive 
- `wingOS_send` => Synchronous send, will block until server  
  - If the Asset is an ipc_endpoint, it will: 
    - Wait for the receiver to be ready, then send the message.
  - If the Asset is an ipc_reply_endpoint, it will: 
    - Wake up the receiver, and send the message.
- `wingOS_send_async` => Asynchronous send, will not block will either: 
  - Wake up the receiver if asleep / waiting for a message 
  - Add to the receiver queue 
- `WingOS_receive` => Synchronous receive
  - If the Asset is an ipc_endpoint, it will: 
    - Wait for a message to be available, or look into the receiver queue. 
    - If no message is available, it will block until one is received.
  

