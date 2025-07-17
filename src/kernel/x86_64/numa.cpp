#include "numa.hpp"

core::Result<CpuTreeNode *> _initialize_cpu_tree_impl()
{
    return core::Result<CpuTreeNode *>::error("not implemented, as QEMU don't export an SRAT structure");
}

core::Result<CpuTreeNode *> initialize_cpu_tree()
{
    auto root = _initialize_cpu_tree_impl();

    if (root.is_error())
    {
        log::err$("unable to initialize cpu tree, falling back to guessed NUMA structure");
        log::err$(root.error());

        return fallback_use_guessed();
    }

    return root;
}

core::Result<CpuTreeNode *> fallback_use_guessed()
{
    auto root = new CpuTreeNode();

    for (size_t i = 0; i < Cpu::count(); i++)
    {
        auto node = new CpuTreeNode();
        node->_children = {};
        node->_cpu = Cpu::get(i);
        root->_children.push(node);
    }

    return root;
}
