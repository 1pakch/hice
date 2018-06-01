#pragma once

#include <memory>
#include <utility>
#include <vector>


#include <pipe/pipe.h>

#include <hice/unique_ptr.hpp>


namespace sunk {

namespace impl {

using pipe_handle = hc::unique_ptr<pipe_t, pipe_free>;
using producer_handle = hc::unique_ptr<pipe_producer_t, pipe_producer_free>;
using consumer_handle = hc::unique_ptr<pipe_consumer_t, pipe_consumer_free>;

}

template<typename T> class Src;
template<typename T> class Dst;

//! Thread-safe multi-producer multi-consumer queue
template<typename T>
class Queue {

  protected:
    impl::pipe_handle h_;
    template<typename> friend class Src;
    template<typename> friend class Dst;

  public:

    Queue(size_t limit=0): h_(pipe_new(sizeof(T), limit)) {}
    Queue(Queue&&) = default;
    Queue& operator=(Queue&&) = default;

    void close() { h_.reset();  }

    Src<T> get_src() {
	return std::move(Src<T>(*this));
    }

    Dst<T> get_dst() {
	return std::move(Dst<T>(*this));
    }
};


template<typename T>
class Src {

  private:

    impl::producer_handle h_;

  public:

    Src(Queue<T>& q): h_(pipe_producer_new(q.h_.get())) {}
    Src(Src&&) = default;
    Src& operator=(Src&&) = default;

    bool push_move(T&& val) {
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
      //printf("sending %d size\n", p->size());
      pipe_push(h_.get(), &p, 1);
      reset();
      return true;
    }
};

/*
template<typename T>
class ChunkedIn {

  private:
    impl::consumer_handle h_;
    bool end_ = false;

  public:
    ChunkedIn(Pipe<T>& pipe, size_t bufsize)
      : h_(pipe_consumer_new(pipe.h_.get()))
    {}

    ChunkedIn(ChunkedIn&& o) = default;
    ChunkedIn& operator=(ChunkedIn&&) = default;

    operator bool() const { return !end_; }

    std::vector<T> recv() {
      std::vector<T>* p = nullptr;
      if (!end_ && _recv(&p)) return std::move(*p);
      else return std::vector<T>();
    }

  private:
    // pop pointer to vector with data from pipe
    bool _recv(std::vector<T>** p) {
      end_ = pipe_pop(h_.get(), p, 1);
      return end_;
    }
};


template<typename T, typename Chunk=std::vector<T>>
class Pipe {

  protected:
    impl::pipe_handle h_;
    template<typename> friend class ChunkedIn;
    template<typename> friend class ChunkedOut;

  public:

    Pipe(size_t limit=0): h_(pipe_new(sizeof(vector_ptr<T>), limit)) {}
    Pipe(Pipe&&) = default;
    Pipe& operator=(Pipe&&) = default;

    void close() { h_.reset();  }

    ChunkedIn<T> create_consumer(size_t bufsize) {
	return std::move(ChunkedIn<T>(*this, bufsize));
    }

    ChunkedOut<T> create_producer(size_t bufsize) {
	return std::move(ChunkedOut<T>(*this, bufsize));
    }
};

template<class T>
using vector_ptr = std::unique_ptr<std::vector<T>>;

template<typename T>
class ChunkedOut {

  private:

    impl::producer_handle h_;
    vector_ptr<T> chunk_ptr_;
    size_t bufsize_;

    void reset() {
	chunk_ptr_ = std::make_unique<std::vector<T>>();
	chunk_ptr_->reserve(bufsize_);
    }

  public:

    ChunkedOut(Pipe<T>& pipe, size_t bufsize):
	h_(pipe_producer_new(pipe.h_.get())),
	bufsize_(bufsize)
    {
	reset();
    }

    ChunkedOut(ChunkedOut&&) = default;
    ChunkedOut& operator=(ChunkedOut&&) = default;

    ~ChunkedOut() { if (h_ && !empty()) send(); }

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
      //printf("sending %d size\n", p->size());
      pipe_push(h_.get(), &p, 1);
      reset();
      return true;
    }
};


template<typename T>
class ChunkedIn {

  private:
    impl::consumer_handle h_;
    bool end_ = false;

  public:
    ChunkedIn(Pipe<T>& pipe, size_t bufsize)
      : h_(pipe_consumer_new(pipe.h_.get()))
    {}

    ChunkedIn(ChunkedIn&& o) = default;
    ChunkedIn& operator=(ChunkedIn&&) = default;

    operator bool() const { return !end_; }

    std::vector<T> recv() {
      std::vector<T>* p = nullptr;
      if (!end_ && _recv(&p)) return std::move(*p);
      else return std::vector<T>();
    }

  private:
    // pop pointer to vector with data from pipe
    bool _recv(std::vector<T>** p) {
      end_ = pipe_pop(h_.get(), p, 1);
      return end_;
    }
};

*/

} // hc
