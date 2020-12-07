#ifndef THREADSAFEQUEUE_HPP
#define THREADSAFEQUEUE_HPP

#include <mutex>
#include <condition_variable>
#include <queue>

template <typename T>
class ThreadsafeQueue {
    std::queue<T> data;
    std::mutex mutex;
    std::condition_variable waiter;

public:
    ThreadsafeQueue() = default;
    ~ThreadsafeQueue() = default;

    void enqueue(const T& obj) {
	const std::lock_guard<std::mutex> lock(mutex); // lock the queue
	
	data.push(obj);
	waiter.notify_one(); // tell one thread waiting for data that it's there
    }
    
    T dequeue() {
	std::unique_lock<std::mutex> lock(mutex); // lock the queue
	
	while (data.size() == 0) waiter.wait(lock);
	
	T element = data.front();
	data.pop();
	return element;
    }
};

#endif // THREADSAFEQUEUE_HPP
