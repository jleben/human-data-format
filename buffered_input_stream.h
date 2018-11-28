#include <iostream>
#include <vector>

namespace human_data {

using std::istream;
using std::vector;

class Buffered_Input_Stream
{
public:
    using Position = std::ptrdiff_t;

    class Iterator
    {
    public:
        using value_type = char;
        using difference_type = Position;
        using pointer = char*;
        using reference = char&;
        using iterator_category = std::bidirectional_iterator_tag;

        Iterator() {}

        Iterator(Buffered_Input_Stream * stream, Position position):
            stream(stream), pos(position)
        {
            if (position >= 0)
                stream->extend_to(position);
        }

        Position position() const { return pos; }

        char operator*() const
        {
            return stream->d_buffer[pos];
        }

        bool operator==(const Iterator & other) const
        {
            return pos == other.pos;
        }

        bool operator !=(const Iterator & other) const
        {
            return !(*this == other);
        }

        Iterator & operator++()
        {
            bool ok = stream->extend_to(pos + 1);
            if (ok)
                ++pos;
            else
                // Make this an end iterator.
                pos = -1;
            return *this;
        }

        Iterator operator++(int)
        {
            auto t = *this;
            ++(*this);
            return t;
        }

        Iterator & operator--()
        {
            // In case this is an end iterator,
            // this implementation conforms to BidirectionalIterator spec,
            // although it may still not have sensible semantics.
            // E.g. it is not decrementable in the sense that --i == i,
            // although it is decrementable according to standard:
            // there is an iterator j such that i = ++j.
            if (pos > 0)
                --pos;
            return *this;
        }

        Iterator operator--(int)
        {
            auto t = *this;
            --(*this);
            return t;
        }
    private:
        Buffered_Input_Stream * stream = nullptr;
        Position pos = -1;
    };

    Buffered_Input_Stream(istream & stream):
        d_stream(stream)
    {
        d_start_pos = stream.tellg();
    }

    // Clears buffer. Invalidates all iterators.
    // Leaves stream at the last read position.
    void reset()
    {
        d_buffer.clear();
        d_start_pos = d_stream.tellg();
    }

    // Clears buffer. Invalidates all iterators.
    // Leaves stream at the position indicated by the iterator.
    // Iterator must be valid and not end.
    void reset(const Iterator & iter)
    {
        d_buffer.clear();
        d_stream.seekg(position(iter));
        d_start_pos = d_stream.tellg();
        // FIXME: Throw an exception if seek fails.
    }

    istream::pos_type start_position() const { return d_start_pos; }

    // If 'iter' is not end, return position in stream that it represents.
    istream::pos_type position(const Iterator & iter)
    {
        return d_start_pos + istream::pos_type(iter.position());
    }

    Iterator begin()
    {
        return Iterator(this, 0);
    }

    Iterator end()
    {
        return Iterator(this, -1);
    }

private:
    friend class Iterator;

    bool extend_to(Position pos)
    {
        auto current_size = d_buffer.size();
        auto required_size = pos+1;
        if (required_size > current_size)
        {
            d_buffer.resize(required_size);
            auto to_read_size = required_size - current_size;
            d_stream.read(&d_buffer[current_size], to_read_size);
            if (d_stream.fail())
                d_buffer.resize(current_size + d_stream.gcount());
            return (!d_stream.fail());
        }
        return true;
    }

    istream & d_stream;
    istream::pos_type d_start_pos;
    vector<char> d_buffer;
};

}
