#ifndef MICROWAVE_MESSAGE_FORMAT_H
#define MICROWAVE_MESSAGE_FORMAT_H

#include <cstring>
#include <cstdint>

namespace MicrowaveMsgFormat {

enum class Destination : uint32_t {
    APP = 0x4D617070,
    DEV = 0x4D646576
};

enum class Type : uint8_t {
    STATE   = 0x10,
    SIGNAL  = 0x20,
    UPDATE  = 0x40,
};

enum class State : uint32_t {
    NONE                        = 0x10000000,
    DISPLAY_CLOCK               = 0x10000001,
    CLOCK_SELECT_HOUR_TENS      = 0x10000002,
    CLOCK_SELECT_HOUR_ONES      = 0x10000004,
    CLOCK_SELECT_MINUTE_TENS    = 0x10000008,
    CLOCK_SELECT_MINUTE_ONES    = 0x10000010,
    SET_COOK_TIMER_INITIAL      = 0x10000020,
    SET_COOK_TIMER_FINAL        = 0x10000040,
    SET_POWER_LEVEL             = 0x10000080,
    KITCHEN_SELECT_HOUR_TENS    = 0x10000100,
    KITCHEN_SELECT_HOUR_ONES    = 0x10000200,
    KITCHEN_SELECT_MINUTE_TENS  = 0x10000400,
    KITCHEN_SELECT_MINUTE_ONES  = 0x10000800,
    DISPLAY_TIMER_RUNNING       = 0x10000100,
    DISPLAY_TIMER_PAUSED        = 0x10000200,
};

enum class Signal : uint32_t {
    NONE            = 0x20000000,
    CLOCK           = 0x20000001,
    COOK_TIME       = 0x20000002,
    POWER_LEVEL     = 0x20000004,
    KITCHEN_TIMER   = 0x20000008,
    STOP            = 0x20000010,
    START           = 0x20000020,
    DIGIT_0         = 0x20000040,
    DIGIT_1         = 0x20000080,
    DIGIT_2         = 0x20000100,
    DIGIT_3         = 0x20000200,
    DIGIT_4         = 0x20000400,
    DIGIT_5         = 0x20000800,
    DIGIT_6         = 0x20001000,
    DIGIT_7         = 0x20002000,
    DIGIT_8         = 0x20004000,
    DIGIT_9         = 0x20008000,
    BLINK_ON        = 0x20010000,
    BLINK_OFF       = 0x20020000,
    MOD_LEFT_TENS   = 0x20040000,
    MOD_LEFT_ONES   = 0x20080000,
    MOD_RIGHT_TENS  = 0x20100000,
    MOD_RIGHT_ONES  = 0x20200000,
};

enum class Update : uint32_t {
    NONE            = 0x40000000,
    CLOCK           = 0x40000001,
    DISPLAY_TIMER   = 0x40000002,
    POWER_LEVEL     = 0x40000004,
};

class Time {
public:
    Time() = default;
    ~Time() = default;

    Time(const Time&) = default;
    Time& operator=(const Time&) = default;

    Time(Time&&) = default;
    Time& operator=(Time&&) = default;

    uint32_t left_tens;
    uint32_t left_ones;
    uint32_t right_tens;
    uint32_t right_ones;
};

class Message
{
public:
    Message() = default;
    Message(const char* rxData)
        : data{}
    {
        memcpy(this, rxData, sizeof(Message));
    }
    
    ~Message() = default;

    Message(const Message&) = default;
    Message& operator=(const Message&) = default;

    Message(Message&&) = default;
    Message& operator=(Message&&) = default;

    bool operator==(const Message& rhs)
    {
        return 0 == memcmp(this, &rhs, sizeof(Message));
    }
    bool operator!=(const Message& rhs)
    {
        return 0 != memcmp(this, &rhs, sizeof(Message));
    }

    Destination dst;
    union {
        State state;
        Signal signal;
        Update update;
    };
    char data[sizeof(Time)];
};

} // namespace MicrowaveMsgFormat

#endif // MICROWAVE_MESSAGE_FORMAT_H
