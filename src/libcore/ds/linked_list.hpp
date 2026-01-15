#pragma once

#include "libcore/ds/vec.hpp"
#include "libcore/funcs.hpp"
#include "libcore/optional.hpp"
#include "libcore/type-utils.hpp"
#include "libcore/type/trait.hpp"

#include <libcore/mem/view.hpp>
namespace core
{

template <typename T>
class LinkedList
{
    struct Node
    {
        Node *next;
        Storage<T> data;
    };

    Node *head = nullptr;
    Node *tail = nullptr;
    size_t _count = 0;

public:
    struct Iterator
    {
        Node *_ptr = nullptr;

        Iterator() {};
        Iterator(Node *ptr) { _ptr = ptr; };
        constexpr bool operator==(const Iterator &other)
        {
            return _ptr == other._ptr;
        }

        constexpr bool operator!=(const Iterator &other)
        {
            return _ptr != other._ptr;
        }

        ~Iterator()
        {
        }

        T &operator*()
        {
            return *_ptr->data.as_ptr();
        }

        Iterator &operator++()
        {

            if(_ptr == nullptr) [[unlikely]]
            {
                return *this;
            }
            this->_ptr = this->_ptr->next;
            return *this;
        }

        Iterator operator++(int)
        {
            if(_ptr == nullptr) [[unlikely]]
            {
                return *this;
            }
            Iterator res = *this;
            this->_ptr = this->_ptr->next;
            return res;
        }
    };

    using Type = T;
    LinkedList() = default;

    LinkedList(LinkedList &&other)
    {
        core::swap(head, other.head);
        core::swap(tail, other.tail);
        core::swap(_count, other._count);
    }

    LinkedList &operator=(LinkedList &&other)
    {
        core::swap(head, other.head);
        core::swap(tail, other.tail);
        core::swap(_count, other._count);
        return *this;
    }

    LinkedList &operator+=(LinkedList &&other)
    {
        if (tail != nullptr)
        {
            tail->next = other.head;
            _count += other._count;
            tail = other.tail;
        }
        else
        {
            head = other.head;
            tail = other.tail;
            _count = other._count;
        }

        other.head = nullptr;
        other.tail = nullptr;
        other._count = 0;

        return *this;
    }

    void release()
    {
        while (head != nullptr)
        {
            auto next = head->next;
            head->data.destruct();
            free(head);
            head = next;
        }
        head = nullptr;
        tail = nullptr;
        _count = 0;
    }

    ~LinkedList()
    {
        release();
    }

    size_t count() const
    {
        return _count;
    }

    template <IsConvertibleTo<T> F>
    T *push(F v)
    {
        auto node = (Node *)malloc(sizeof(Node));

        new (&node->data) Storage<T>(core::forward<F>(v));
       // node->data = core::forward<F>(v);
        node->next = nullptr;

        if (tail != nullptr)
        {
            tail->next = node;
        }
        tail = node;

        if (head == nullptr)
        {
            head = node;
        }
        _count++;

        return tail->data.as_ptr();
    }

    T pop_front()
    {
        if (head == nullptr) [[unlikely]]
        {
            __builtin_unreachable();
        }

        T res = core::move(head->data.take());
        auto next = head->next;

        if (tail == head)
        {
            tail = nullptr;
        }


        free(head);
        head = next;
        _count--;
        return (res);
    }

    T remove(size_t idx)
    {
        if (idx == 0)
        {
            return (pop_front());
        }
        auto prev = head;

        for (size_t i = 0; i < idx - 1; i++)
        {
            if (prev == nullptr)
            {
                return Storage<T>::empty();
            }
            prev = prev->next;
        }

        if (prev == nullptr || prev->next == nullptr)
        {
            return Storage<T>::empty();
        }

        auto node_to_delete = prev->next;
        T res = node_to_delete->data.take();

        prev->next = node_to_delete->next;


        if (node_to_delete == tail)
        {
            tail = prev;
            tail->next = nullptr;
        }

        free (node_to_delete);
        _count--;

        return (res);
    }

    LinkedList &operator=(const LinkedList &other)
    {
        if (this == &other)
        {
            return *this;
        }
        release();
        for (auto &v : other)
        {
            try$(push(v));
        }
        return *this;
    }


    auto first()
    {
        return head->data.as_ptr();
    }

    auto last()
    {
        return tail->data.as_ptr();
    }

    Iterator begin()
    {
        return Iterator(head);
    }


    Iterator end()
    {
        return Iterator(nullptr);
    }
};

static_assert(Iterable<LinkedList<int>>);

} // namespace core
