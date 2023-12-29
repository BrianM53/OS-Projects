// your PA3 BoundedBuffer.cpp code here

//no modifications needed

#include "BoundedBuffer.h"


using namespace std;


BoundedBuffer::BoundedBuffer (int _cap) : cap(_cap) {
    // modify as needed
}

BoundedBuffer::~BoundedBuffer () {
    // modify as needed
}

void BoundedBuffer::push (char* msg, int size) {
    // 1. Convert the incoming byte sequence given by msg and size into a vector<char>
    vector<char> data(msg, msg + size);

    unique_lock<mutex> lock(m);
    // 2. Wait until there is room in the queue (i.e., queue lengh is less than cap)
    push_cv.wait(lock, [this] {return q.size() < (size_t) cap; });

    // 3. Then push the vector at the end of the queue
    q.push(data);
    // 4. Wake up threads that were waiting for push
    lock.unlock();
    pop_cv.notify_one();
}

int BoundedBuffer::pop (char* msg, int size) {

    std::unique_lock<std::mutex> lock(m);
    // 1. Wait until the queue has at least 1 item
    pop_cv.wait(lock, [this] {return q.size() > 0; });

    // 2. Pop the front item of the queue. The popped item is a vector<char>
    std::vector<char> data = q.front();
    q.pop();
    lock.unlock();

    // 3. Convert the popped vector<char> into a char*, copy that into msg; assert that the vector<char>'s length is <= size
    if (data.size() <= static_cast<size_t>(size)) {
        std::copy(data.begin(), data.end(), msg);
    }

    // 4. Wake up threads that were waiting for pop
    // 5. Return the vector's length to the caller so that they know how many bytes were popped
    push_cv.notify_one();
    return static_cast<int>(data.size());

}

size_t BoundedBuffer::size () {
    return q.size();
}
