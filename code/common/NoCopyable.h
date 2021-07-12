#ifndef NOCOPYABLE_H
#define NOCOPYABLE_H

class NoCopyable {
public:
    NoCopyable(const NoCopyable&) = delete;
    NoCopyable(NoCopyable&&) = delete;
    NoCopyable& operator=(const NoCopyable&) = delete;
    NoCopyable& operator=(NoCopyable&&) = delete;
protected:
    NoCopyable() = default;
    ~NoCopyable() = default;
};

#endif