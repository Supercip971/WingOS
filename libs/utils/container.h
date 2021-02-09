#ifndef CONTAINER_H
#define CONTAINER_H
#include <stddef.h>
#include <stdint.h>
namespace utils
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

} // namespace utils
#endif // CONTAINER_H
