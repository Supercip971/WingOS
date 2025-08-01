#pragma once 



#include "libcore/io/reader.hpp"
#include "libcore/mem/view.hpp"
namespace core 
{

    template<typename Base = char>
    class Scanner 
    {

        MemView<Base> _buffer;
        size_t _cursor = 0;
        public:

        Scanner(MemView<Base> buffer) : _buffer(buffer) {}

        size_t tell() const
        {
            return _cursor;
        }

        size_t size() const
        {
            return _buffer.len();
        }

        size_t ended() const
        {
            return _cursor >= _buffer.len();
        }

        size_t remaining() const
        {
            return _buffer.len() - _cursor;
        }

        size_t seek(size_t offset)
        {
            if (offset > _buffer.len())
            {
                return _cursor;
            }
            _cursor = offset;
            return _cursor;
        }

        core::Result<Base> current() const
        {
            if (ended())
            {
                return core::Result<Base>("End of buffer reached");
            }
            return _buffer[_cursor];
        }

        core::Result<Base> peek(size_t offset = 0) const
        {
            if (offset + _cursor >= _buffer.len())
            {
                return core::Result<Base>("End of buffer reached");
            }
            return _buffer[_cursor + offset];
        }

        core::Result<Base> next()
        {
            if (ended())
            {
                return core::Result<Base>("End of buffer reached");
            }
            return _buffer[_cursor++];
        }

        core::Result<void> skip(size_t count)
        {
            if (_cursor + count >= _buffer.len())
            {
                return core::Result<void>("End of buffer reached");
            }
            _cursor += count;
            return {};
        }

        core::Result<void> rewind(size_t count)
        {
            if (_cursor < count)
            {
                return core::Result<void>("Cannot rewind beyond the start of the buffer");
            }
            _cursor -= count;
            return {};
        }

        core::Result<bool> skip(Base value)
        {
            if (ended())
            {
                return core::Result<bool>("End of buffer reached");
            }
            if (_buffer[_cursor] == value)
            {
                _cursor++;
                return true;
            }
            return false;
        }

        core::Result<bool> skip_string(MemView<Base> str)
        {
            if (_cursor + str.len() > _buffer.len())
            {
                return core::Result<bool>("End of buffer reached");
            }
            for (size_t i = 0; i < str.len(); i++)
            {
                if (_buffer[_cursor + i] != str[i])
                {
                    return false;
                }
            }
            _cursor += str.len();
            return true;
        }

        core::Result<bool> skip_any_of(MemView<Base> chars)
        {
            if (ended())
            {
                return core::Result<bool>("End of buffer reached");
            }
            for (size_t i = 0; i < chars.len(); i++)
            {
                if (_buffer[_cursor] == chars[i])
                {
                    _cursor++;
                    return true;
                }
            }
            return false;
        }

        core::Result<bool> skip_spaces()
        {
            if (ended())
            {
                return core::Result<bool>("End of buffer reached");
            }
            while (_cursor < _buffer.len() && (_buffer[_cursor] == ' ' || _buffer[_cursor] == '\t' || _buffer[_cursor] == '\n' || _buffer[_cursor] == '\r'))
            {
                _cursor++;
            }
            return true;
        }

        core::Result<MemView<Base>> read(size_t count)
        {
            if (_cursor + count > _buffer.len())
            {
                return core::Result<MemView<Base>>("End of buffer reached");
            }
            MemView<Base> result(_buffer.data() + _cursor, count);
            _cursor += count;
            return result;
        }

        core::Result<MemView<Base>> read_until(Base delimiter)
        {
            size_t start = _cursor;
            while (_cursor < _buffer.len() && _buffer[_cursor] != delimiter)
            {
                _cursor++;
            }
            if (_cursor >= _buffer.len() && delimiter != '\0')
            {
                return core::Result<MemView<Base>>("End of buffer reached");
            }
            MemView<Base> result(_buffer.data() + start, _cursor - start);
            return result;

        }
        
    };
};