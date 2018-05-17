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

template<typename T> class Consumer;
template<typename T> class Producer;

template<class T>
using vector_ptr = std::unique_ptr<std::vector<T>>;

//! Thread-safe multi-producer multi-consumer queue
template<typename T, typename Chunk=std::vector<T>>
class Pipe {

  protected:
    impl::pipe_handle h_;
    template<typename> friend class Consumer;
    template<typename> friend class Producer;

  public:

    Pipe(size_t limit=0): h_(pipe_new(sizeof(vector_ptr<T>), limit)) {}
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
    vector_ptr<T> chunk_ptr_;
    size_t bufsize_;

    void reset() {
	chunk_ptr_ = std::make_unique<std::vector<T>>();
	chunk_ptr_->reserve(bufsize_);
    }

  public:

    Producer(Pipe<T>& pipe, size_t bufsize):
	h_(pipe_producer_new(pipe.h_.get())),
	bufsize_(bufsize)
    {
	reset();
    }

    Producer(Producer&&) = default;
    Producer& operator=(Producer&&) = default;

    ~Producer() { if (h_ && !empty()) send(); }

    size_t capacity() const { return chunk_ptr_->capacity(); }
    size_t count() const { return chunk_ptr_->size(); } 
    bool empty() const { return count() == 0; };
    bool full() const { return count() == capacity()-1; }
    
    //template<class T_=T>
    //typename std::enable_if<std::is_trivially_copyable<T_>::value, bool>::type
    bool push(T&& val) {
	chunk_ptr_->emplace_back(std::forward<T>(val));
	if (full()) { send(); return true; }
	else return false;
    }
    
    //template<class T_=T>
    //typename std::enable_if<!std::is_trivially_copyable<T_>::value, bool>::type
    //bool push(T&& val) {
//	chunk_ptr_[pos_++] = std::move(val);
//	if (full()) { send(); return true; }
//	else return false;
 //   }

    bool send() {
      if (empty()) return false;
      auto p = chunk_ptr_.release();
      printf("sending %d size\n", p->size());
      pipe_push(h_.get(), &p, 1);
      reset();
      return true;
    }
};


template<typename T>
class Consumer {

  private:
    impl::consumer_handle h_;

  public:
    Consumer(Pipe<T>& pipe, size_t bufsize)
      : h_(pipe_consumer_new(pipe.h_.get()))
    {}

    Consumer(Consumer&& o) = default;
    Consumer& operator=(Consumer&&) = default;

    std::vector<T> recv() {
      std::vector<T>* p;
      auto popped = pipe_pop(h_.get(), &p, 1);
      if (popped) printf("popped %d size %d\n", popped, p->size());
      return popped ? std::move(*p) : std::vector<T>();
    }
};

} // hc
