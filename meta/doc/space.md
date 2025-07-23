# Space 

This is a theoretical document that outlines the concept of a "space" and how handles are used to manage resources.

It is not a full implementation for now and is subject to change as the project evolves.

Each task is assigned in a space. Each space has a unique identifier, which is a 64-bit integer. 

Each handle is reference counted. You can send a handle to another task, meaning inserting in its own space.

You can have multiple task in a space but only one task per space.
To have multiple space, you need to create a new space, inside our space. 

To access its data, you either need to move or share something in your space to the child space, or use a syscall to create a share handle to the data you want to access. But generally it is not used. 


- For example, if you want to have a out of memory manager, it can use a syscall to parse each space, and use a specific space to parse all the physical memory region and free them.

# Syscalls 

> NOTE: a space ID of 0 means self 

- Space create (flags, rights, child space)
- Object create (type, flags, rights, space ID)
- Object destroy (handle, space ID)
- Object share (handle, space ID, target space ID)
- Object move (handle, space ID, target space ID)
- Object get (handle, space ID)

# Handles

A handle is unique to a specific space. A handle is always meaningless outside of its space.

You can have a list of all space (because they all have a unique handle) and you can have a list of all handles in a space.
