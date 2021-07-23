#ifndef __buffer_hh__
#define __buffer_hh__

#include <cstddef>
#include <string>
#include <memory>
#include <tuple>

#include "noncopyable.hh"

namespace kkit {

class buffer : public noncopyable 
{
public:
    typedef std::shared_ptr<buffer> ptr;
    buffer(){};
    virtual ~buffer(){};
    //返回数据长度
    virtual char *data() const = 0 ;
    virtual std::tuple<char *, uint32_t> data_z() const = 0;
    virtual uint32_t size() const = 0;

    virtual std::string string() const {
        return std::string(data(),size());
    }

    virtual uint32_t capacity() const{
        return size();
    }
};

template <typename T>
class bufferOffset : public buffer
{
public:
    typedef std::shared_ptr<bufferOffset> ptr;

    bufferOffset(T data, size_t offset = 0, size_t len = 0) : _data(std::move(data)) {
        setup(offset, len);
    }

    ~bufferOffset() {}

    char *data() const override {
        return const_cast<char *>(_data.data()) + _offset;
    }

    std::tuple<char *, uint32_t> data_z() const override {
        return { data(), size() };
    }

    uint32_t size() const override{
        return _size;
    }

    std::string string() const override {
        return std::string(data(),size());
    }

private:
    void setup(uint32_t offset = 0,uint32_t len = 0){
        _offset = offset;
        _size = len;
        if(_size <= 0 || _size > _data.size()){
            _size = _data.size();
        }
    }

private:
    T _data;
    uint32_t _offset = 0;
    uint32_t _size = 0;
};

}

#endif