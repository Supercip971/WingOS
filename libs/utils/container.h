#ifndef CONTAINER_H
#define CONTAINER_H
#include <stddef.h>
#include <stdint.h>
namespace wos
{
    template <class T>
    class container
    {

    public:
        using type = T;

        virtual T &get(size_t idx) = 0;
        virtual const T get(size_t idx) const = 0;
        virtual size_t size() const = 0;
    };

} // namespace wos
#endif // CONTAINER_H
