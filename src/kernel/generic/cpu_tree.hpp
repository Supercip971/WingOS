
#pragma once 


#include "kernel/generic/cpu.hpp"
#include "libcore/ds/vec.hpp"

class CpuTreeNode 
{
    public:
    bool is_leaf() const { return _children.len() == 0; }

    size_t count() const {
        size_t c = is_leaf() ? 1 : 0;

        for (auto child : _children)
        {
            c += child->count();
        }

        return c;
    }

    // may be changed for x86 for a faster algorithm
    bool are_they_sibling(CoreId a, CoreId b) const {
        bool has_a = false;
        bool has_b = false;
        for (auto child : _children)
        {
            if(child->is_leaf())
            {
                if(child->_cpu->id() == a) {
                    has_a = true;

                    if (has_b) {
                        return true;
                    }
                }
                if(child->_cpu->id() == b) {
                    has_b = true;

                    if (has_a) {
                        return true;
                    }
                }
            }
        }

        for (auto child : _children)
        {
            if (child->are_they_sibling(a, b))
            {
                return true;
            }
        }

        return false;
    }

    core::Vec<CpuTreeNode *> _children;
    Cpu *_cpu;

    static CpuTreeNode const* root(); 

    static void assign_root(CpuTreeNode* node); 
};




