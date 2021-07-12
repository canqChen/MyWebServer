#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#include<chrono>

using std::chrono::system_clock;

typedef std::chrono::nanoseconds   Nanosecond;
typedef std::chrono::microseconds  Microsecond;
typedef std::chrono::milliseconds  Millisecond;
typedef std::chrono::seconds       Second;
typedef std::chrono::minutes       Minute;
typedef std::chrono::hours         Hour;
typedef std::chrono::time_point<system_clock, Nanosecond> Timestamp;

struct GetTimestamp {
    static inline Timestamp now() {
        return system_clock::now();
    }

    static inline Timestamp nowAfter(Nanosecond interval) { 
        return now() + interval; 
    }

    static inline Timestamp nowBefore(Nanosecond interval) { 
        return now() - interval; 
    }
};


#endif