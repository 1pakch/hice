#pragma once

#include <memory>
#include <utility>
#include <vector>

#include <pipe/pipe.h>

#include <hice/unique_ptr.hpp>


namespace hc {

namespace impl {

using pipe_handle = hc::unique_ptr<pipe_t, pipe_free>;
using producer_handle = hc::unique_ptr<pipe_producer_t, pipe_producer_free>;
using consumer_handle = hc::unique_ptr<pipe_consumer_t, pipe_consumer_free>;

}

template<typename T> class Pipe;
template<typename T> class Consumer;
template<typename T> class Producer;


template<typename T>
class Pipe {

  protected:
    impl::pipe_handle h_;
    template<typename> friend class Consumer;
    template<typename> friend class Producer;

  public:

    Pipe(size_t limit=0): h_(pipe_new(sizeof(T), limit)) {}
    Pipe(Pipe&&) = default;
    Pipe& operator=(Pipe&&) = default;

    void close() { h_.reset();  }

    Consumer<T> create_consumer(size_t bufsize) {
	return std::move(Consumer<T>(*this, bufsize));
    }

    Producer<T> create_producer(size_t bufsize) {
	return std::move(Producer<T>(*this, bufsize));
    }
};


template<typename T>
class Producer {

  private:

    impl::producer_handle h_;
    std::vector<T> buf_;
    size_t pos_ = 0;

  public:

    Producer(Pipe<T>& pipe, size_t bufsize):
	h_(pipe_producer_new(pipe.h_.get())),
	buf_(bufsize)
    {}
    
    size_t capacity() const { return buf_.size(); }
    size_t count() const { return pos_; } 
    bool empty() const { return count() == 0; };
    bool full() const { return count() == capacity()-1; }
    
    void clear() { pos_ = 0; } 

    template<class T_=T>
    typename std::enable_if<std::is_trivially_copyable<T_>::value, bool>::type
    push(T val) {
	buf_[pos_++] = val;
	if (full()) { send(); return true; }
	else return false;
    }
    
    template<class T_=T>
    typename std::enable_if<!std::is_trivially_copyable<T_>::value, void>::type
    push(T&& val) {
	buf_[pos_++] = std::move(val);
	if (full()) { send(); return true; }
	else return false;
    }

    bool send() {
      if (empty()) return false;
      pipe_push(h_.get(), buf_.data(), count());
      pos_ = 0;
      return true;
    }
};


template<typename T>
class Consumer {

  private:
    impl::consumer_handle h_;
    std::vector<T> buf_;

  public:
    Consumer(Pipe<T>& pipe, size_t bufsize)
      : h_(pipe_consumer_new(pipe.h_.get())),
	buf_(bufsize)
    {}

    Consumer(Consumer&& o) = default;
    Consumer& operator=(Consumer&&) = default;

    template<class T_=T>
    typename std::enable_if<std::is_trivially_copyable<T_>::value, T>::type
    pop(size_t i) { return buf_[i]; }

    template<class T_=T>
    typename std::enable_if<!std::is_trivially_copyable<T_>::value, T&&>::type
    pop(size_t i) { return std::move(buf_[i]); }

    size_t recv() {
      return pipe_pop(h_.get(), buf_.data(), buf_.size());
    }
};

} // hc
