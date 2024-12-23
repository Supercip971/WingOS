
#pragma once

// generally used by implementation
class Cpu
{
protected:
    int _id;

    bool _present;

public:
    int id() const { return _id; };

    bool present() const { return _present; };

    Cpu(int id, bool present) : _id(id), _present(present) {};

    Cpu() : _id(-1), _present(false) {};

    static int currentId();
    static Cpu *current();

    static Cpu *get(int id);
};
