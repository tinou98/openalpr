#ifndef SAFE_QUEUE_H_
#define SAFE_QUEUE_H_

#include <queue>
#include "support/tinythread.h"

template <typename T>
class SafeQueue
{
    public:
        explicit SafeQueue(size_t size = 0) : _buf(new T[size]), _max_size(size) {}

        void resize(size_t size) {
            tthread::lock_guard<tthread::mutex> mlock(_mutex);

            delete[] _buf;
            _buf = new T[size];
            _max_size = size;
            _head = 0;
            _tail = 0;
            _full = false;
        }

        T pop()
        {
            tthread::lock_guard<tthread::mutex> mlock(_mutex);
            while (this->empty()) {
                _cond.wait(_mutex);
            }
            T val = _buf[_tail];
            _tail = (_tail + 1) % _max_size;
            _full = false;
            return val;
        }

        void push(const T& item)
        {
            tthread::lock_guard<tthread::mutex> mlock(_mutex);

            _buf[_head] = item;
            if(_full) {
                _tail = (_tail + 1) % _max_size;
            }
            _head = (_head + 1) % _max_size;
            _full = _head == _tail;

            _cond.notify_one();
        }

        bool empty() const
        {
            return !_full && (_head == _tail);
        }

        size_t size() const
        {
            return _full ? _max_size : (_head >= _tail) ? (_head - _tail) : (_max_size + _head - _tail);
        }

    private:
        T *_buf;
        bool _full = false;
        size_t _head = 0;
        size_t _tail = 0;
        size_t _max_size;

        tthread::mutex _mutex;
        tthread::condition_variable _cond;
};

#endif
