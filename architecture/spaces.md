
# Interfacing the userspace and the kernel 


## The SPace 

Each treads has its own space. 


A space is constructed of 256 entries of 16 bytes. 
The first 8 bytes are the access bytes and the type descriptor. The second byte depends on the type, it could be: 
- the address of page
- a pointer to a task structure
- a pointer to an endpoint 
- a pointer to another space


The space is addressed through a single uint64_t address:

| Address type byte | byte 1 | byte 2 | byte 3 | byte 4 | byte 5 | byte 6 | byte 7 |
| ----------------- | ------ | ------ | ------ | ------ | ------ | ------ | ------ |
| 0[access_check]   | Index  | ...    | ...    | ...    | ...    | ....   | ...    |
| 1[0000000]        | rootcd | rootcd | rootcd | rootcd | Index  | Index  | Index  |


- The first way to access the space is with the first bit to 0. This is the fastest way to access the space.
  And it is the recommended way to access the space.

- The second way is not implemented, but may be used in the future to access child space.
  For example, if you create a child task with a specific space at the 7th index, you will be able to access it with the second way.
  You will only be able to use it if you have the `rootcd` space already in the space.

> Note: a space index of 255 means 'use this space', so it is not a valid index.
> So if we want to index the `root.child[0].child[0]`, we would have an index: 0x0000FF0000000000


The userspace manage the space by itself. It can create a new space, and add it to the current space.

For example, a program could easily create 4 child space and use them as a way to allocate memory. So it could look like: 



```c
root {
 // used for allocation 
 [0] = space("memory space") {
 }
}
```


The app would then be able to store page allocated in this space.
We would be able to have 5 space deep of space, which would give us: $255 ^{6} \simeq 274941996890625$ page allocation.

Using a fixed size of 8 bit per space, instead of SeL4 variable size space, the userspace doesn't have to manage complex data structure to allocate an object.  

### Issues: 

- The fact that we are using multiple indirect byte, make the cache less efficient. Maybe the userspace could 
  use a shallow space when needed. 
- Each space is $16*256= 4096$ bytes long, which means that if we want to use a 5 space deep tree, we would need $4096 * 5 = 20480$ bytes of memory.  This is a lot of memory, and we may want to use a smaller space. Or, the first allocations are automatically done in the root space, and then the user can create a new space to store more data.
