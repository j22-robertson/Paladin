//
// Created by jalr on 17-02-2026.
//

///https://www.geeksforgeeks.org/cpp/implement-circular-buffer-using-std-vector-in-cpp/
#ifndef PALADIN_RINGBUFFER_H
#define PALADIN_RINGBUFFER_H
#include <optional>
#include <span>
#include <vector>
template<typename T>
class RingBuffer {
public:
    template<typename N>
    requires std::unsigned_integral<N>
    RingBuffer(N _capacity) {
        capacity = _capacity+1;
        front = back = 0;
        buffer.resize(capacity);
    }

    bool Empty() const {
        return front == back;
    }
    // Maximum capacity
    [[nodiscard]] std::uint32_t Capacity() const {
        return capacity-1;
    }
    // Num allocated elements
    [[nodiscard]] std::uint32_t Count() const{
        return (back+capacity-front)%capacity;
    }

    bool Full() const {
        PALADIN_LOG(INFO, "Tail+1%capacity = " +std::to_string(back+1%capacity))
        PALADIN_LOG(INFO, "Head = "+ std::to_string(front))
        return ((back+1) % capacity)==front;
    }

    void PopFront() {
        if (Empty()) return;
        front = (front+1)%capacity;
    }

    std::optional<T> GetFront() {
        if (Empty()) {
            return std::nullopt;
        }
        return buffer[front];
    }

    std::optional<T> GetBack() {
        if (Empty()) {
            return std::nullopt;
        }
        return (back==0) ? buffer[capacity-1] : buffer[back-1];
    }

    void PushBack(T value) {
        if (Full()) {
            front = (front+1)%capacity;
        }
        buffer[back] = value;
        back = (back+1)%capacity;
    }

    std::vector<T> GetForPrint() {
        std::vector<T> buffer_data;
        PALADIN_LOG(INFO, "Front:"+std::to_string(front)+ " back:"+std::to_string(back))
        if (front<back) {
            buffer_data.insert(buffer_data.end(), buffer.begin()+front, buffer.begin()+back);
        }
        else {
            buffer_data.insert(buffer_data.end(), buffer.begin()+front,buffer.end());
            buffer_data.insert(buffer_data.end(), buffer.begin(), buffer.begin()+back);
        }
        return buffer_data;
    }

private:
    std::vector<T> buffer;

    std::uint32_t front = 0;
    std::uint32_t back = 0;
    // Maximum size of the buffer
    std::uint32_t capacity = 0;
    std::uint32_t count = 0;
};

#endif //PALADIN_RINGBUFFER_H