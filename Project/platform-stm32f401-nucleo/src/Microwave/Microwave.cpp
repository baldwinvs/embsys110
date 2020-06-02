/*******************************************************************************
 * Copyright (C) Gallium Studio LLC. All rights reserved.
 *
 * This program is open source software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Alternatively, this program may be distributed and modified under the
 * terms of Gallium Studio LLC commercial licenses, which expressly supersede
 * the GNU General Public License and are specifically designed for licensees
 * interested in retaining the proprietary status of their code.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * Contact information:
 * Website - https://www.galliumstudio.com
 * Source repository - https://github.com/galliumstudio
 * Email - admin@galliumstudio.com
 ******************************************************************************/

#include "app_hsmn.h"
#include "fw_log.h"
#include "fw_assert.h"
#include "MicrowaveInterface.h"
#include "Microwave.h"
#include "FanInterface.h"
#include "MWLampInterface.h"
#include "TurntableInterface.h"
#include "MagnetronInterface.h"
#include "WifiInterface.h"

FW_DEFINE_THIS_FILE("Microwave.cpp")

namespace APP {

#undef ADD_EVT
#define ADD_EVT(e_) #e_,

static char const * const timerEvtName[] = {
    "MICROWAVE_TIMER_EVT_START",
    MICROWAVE_TIMER_EVT
};

static char const * const internalEvtName[] = {
    "MICROWAVE_INTERNAL_EVT_START",
    MICROWAVE_INTERNAL_EVT
};

static char const * const interfaceEvtName[] = {
    "MICROWAVE_INTERFACE_EVT_START",
    MICROWAVE_INTERFACE_EVT
};

Microwave::Microwave() :
    Active((QStateHandler)&Microwave::InitialPseudoState, MICROWAVE, "MICROWAVE"),
    m_blink{false},
    m_blinkToggle{false},
    m_cook{false},
	m_cooking{false},
    m_closed{true},
    m_message{},
    m_clockTime{},
    m_proposedClockTime{},
    m_state{MicrowaveMsgFormat::State::NONE},
    m_timersUsed{},
    m_timerIndex{},
    m_secondsRemaining{},
    m_displayTime{{},{}},
    m_fan{FAN, "FAN"},
    m_lamp{MW_LAMP, "MW_LAMP"},
    m_turntable{TURNTABLE, "TURNTABLE"},
    m_halfSecondTimer{GetHsm().GetHsmn(), HALF_SECOND_TIMER},
    m_secondTimer{GetHsm().GetHsmn(), SECOND_TIMER},
    m_halfSecondCounts{},
    m_magnetronPipe{m_magnetronStor, MAGNETRON_PIPE_ORDER}
    {
        SET_EVT_NAME(MICROWAVE);

        m_message.dst = MicrowaveMsgFormat::Destination::APP;
    }

QState Microwave::InitialPseudoState(Microwave * const me, QEvt const * const e) {
    (void)e;
    return Q_TRAN(&Microwave::Root);
}

QState Microwave::Root(Microwave * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            // Initialize regions.
            me->m_fan.Init(me);
            me->m_lamp.Init(me);
            me->m_turntable.Init(me);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
        case Q_INIT_SIG: {
            return Q_TRAN(&Microwave::Stopped);
        }
        case MICROWAVE_START_REQ: {
            EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new MicrowaveStartCfm(req.GetFrom(), GET_HSMN(), req.GetSeq(), ERROR_STATE);
            Fw::Post(evt);
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&QHsm::top);
}

QState Microwave::Stopped(Microwave * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
        case MICROWAVE_STOP_REQ: {
            EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new MicrowaveStopCfm(req.GetFrom(), GET_HSMN(), req.GetSeq(), ERROR_SUCCESS);
            Fw::Post(evt);
            return Q_HANDLED();
        }
        case MICROWAVE_START_REQ: {
            EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new MicrowaveStartCfm(req.GetFrom(), GET_HSMN(), req.GetSeq(), ERROR_STATE);
            Fw::Post(evt);
            return Q_TRAN(&Microwave::Started);
        }
    }
    return Q_SUPER(&Microwave::Root);
}

QState Microwave::Started(Microwave * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
        case MICROWAVE_STOP_REQ: {
            EVENT(e);
            Evt const & req = EVT_CAST(*e);

            Evt *evt = new FanOffReq(FAN, GET_HSMN());
            me->PostSync(evt);
            evt = new MwLampOffReq(MW_LAMP, GET_HSMN());
            me->PostSync(evt);
            evt = new TurntableOffReq(TURNTABLE, GET_HSMN());
            me->PostSync(evt);
            evt = new MagnetronOffReq(MAGNETRON, GET_HSMN(), GEN_SEQ());
            Fw::Post(evt);

            evt = new MicrowaveStopCfm(req.GetFrom(), GET_HSMN(), req.GetSeq(), ERROR_SUCCESS);
            Fw::Post(evt);
            return Q_TRAN(&Microwave::Stopped);
        }
        case HALF_SECOND_TIMER: {
            //EVENT(e);
            if(++me->m_halfSecondCounts == HALF_SECOND_COUNTS_PER_MINUTE) {
                LOG("HALF_SECOND_TIMER, IncrementClock()");
                me->IncrementClock();
                me->m_halfSecondCounts = 0;
            }
            if(me->m_blink) {
                if(me->m_blinkToggle) {
                    me->SendSignal(MicrowaveMsgFormat::Signal::BLINK_ON);
                    me->m_blinkToggle = false;
                }
                else {
                    me->SendSignal(MicrowaveMsgFormat::Signal::BLINK_OFF);
                    me->m_blinkToggle = true;
                }
            }
            return Q_HANDLED();
        }
        case MICROWAVE_EXT_START_SIG: {
            EVENT(e);
            if(me->m_displayTime[me->m_timerIndex].time == MicrowaveMsgFormat::Time()) {
                me->Add30SecondsToCookTime();
            }
            return Q_TRAN(&Microwave::DisplayTimer);
        }
        case MICROWAVE_EXT_STOP_SIG: {
            EVENT(e);
            me->m_timersUsed = 0;
            return Q_TRAN(&Microwave::DisplayClock);
        }
        case MICROWAVE_EXT_DOOR_OPEN_SIG: {
            EVENT(e);
            me->m_closed = false;
            return Q_HANDLED();
        }
        case MICROWAVE_EXT_DOOR_CLOSED_SIG: {
            EVENT(e);
            me->m_closed = true;
            return Q_HANDLED();
        }
        case MICROWAVE_WIFI_CONN_REQ: {
            EVENT(e);
            char const *host = "192.168.0.27";
            char const *portStr = "60002";
            uint16_t port = STRING_TO_NUM(portStr, 0);
            Evt *evt = new WifiConnectReq(WIFI_ST, GET_HSMN(), GEN_SEQ(), host, port);
            Fw::Post(evt);
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&Microwave::Root);
}

QState Microwave::DisplayClock(Microwave * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            me->m_state = MicrowaveMsgFormat::State::DISPLAY_CLOCK;
            me->UpdateClock(me->m_clockTime);
            me->m_blink = true;
            me->m_halfSecondTimer.Start(HALF_SECOND_TIMEOUT_MS);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            me->m_blink = false;
            return Q_HANDLED();
        }
        case MICROWAVE_EXT_COOK_TIME_SIG: {
            EVENT(e);
            me->SendSignal(MicrowaveMsgFormat::Signal::COOK_TIME);
            return Q_TRAN(&Microwave::SetCookTimer);
        }
        case MICROWAVE_EXT_KITCHEN_TIMER_SIG: {
            EVENT(e);
            me->SendSignal(MicrowaveMsgFormat::Signal::KITCHEN_TIMER);
            return Q_TRAN(&Microwave::SetKitchenTimer);
        }
    }
    return Q_SUPER(&Microwave::Started);
}

QState Microwave::SetClock(Microwave * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            me->m_proposedClockTime = me->m_clockTime;
            me->m_blink = false;
            me->SendSignal(MicrowaveMsgFormat::Signal::BLINK_ON);
            me->SendSignal(MicrowaveMsgFormat::Signal::MOD_LEFT_TENS);
            me->m_blink = true;
            return Q_TRAN(&Microwave::ClockSelectHourTens);
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
        case MICROWAVE_EXT_CLOCK_SIG: {
            EVENT(e);
            if(me->m_clockTime != me->m_proposedClockTime) {
                me->m_clockTime = me->m_proposedClockTime;
                me->m_halfSecondTimer.Stop();
                me->m_halfSecondCounts = 0;
                me->m_halfSecondTimer.Start(HALF_SECOND_TIMEOUT_MS);
                me->UpdateClock(me->m_clockTime);
            }
            me->m_blink = false;
            me->SendSignal(MicrowaveMsgFormat::Signal::BLINK_ON);
            me->SendSignal(MicrowaveMsgFormat::Signal::CLOCK);
            me->m_blink = true;
            return Q_TRAN(&Microwave::DisplayClock);
        }
        case MICROWAVE_EXT_STOP_SIG: {
            EVENT(e);
            me->UpdateClock(me->m_clockTime);
            me->m_blink = false;
            me->SendSignal(MicrowaveMsgFormat::Signal::BLINK_ON);
            me->SendSignal(MicrowaveMsgFormat::Signal::CLOCK);
            me->m_blink = true;
            return Q_TRAN(&Microwave::DisplayClock);
        }
        case MICROWAVE_EXT_START_SIG: {
            EVENT(e);
            // don't do anything for this event
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&Microwave::DisplayClock);
}

QState Microwave::ClockSelectHourTens(Microwave * const me, QEvt const * const e) {
    MicrowaveMsgFormat::Time &time = me->m_proposedClockTime;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            me->m_state = MicrowaveMsgFormat::State::CLOCK_SELECT_HOUR_TENS;
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            me->UpdateClock(me->m_proposedClockTime);
            me->SendSignal(MicrowaveMsgFormat::Signal::MOD_LEFT_ONES);
            return Q_HANDLED();
        }
        case MICROWAVE_EXT_DIGIT_SIG: {
            EVENT(e);
            MicrowaveExtDigitSig const &req = static_cast<MicrowaveExtDigitSig const &>(*e);
            uint32_t digit = req.GetDigit();

            switch(digit) {
                case 0:
                case 1:
                    time.left_tens = digit;
                    return Q_TRAN(&Microwave::ClockSelectHourOnes);
                default:
                    break;
            }
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&Microwave::SetClock);
}

QState Microwave::ClockSelectHourOnes(Microwave * const me, QEvt const * const e) {
    MicrowaveMsgFormat::Time &time = me->m_proposedClockTime;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            me->m_state = MicrowaveMsgFormat::State::CLOCK_SELECT_HOUR_TENS;
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            me->UpdateClock(me->m_proposedClockTime);
            me->SendSignal(MicrowaveMsgFormat::Signal::MOD_RIGHT_TENS);
            return Q_HANDLED();
        }
        case MICROWAVE_EXT_DIGIT_SIG: {
            EVENT(e);
            MicrowaveExtDigitSig const &req = static_cast<MicrowaveExtDigitSig const &>(*e);
            uint32_t digit = req.GetDigit();

            switch(digit) {
                case 0:
                case 1:
                    time.left_ones = digit;
                    return Q_TRAN(&Microwave::ClockSelectMinuteTens);
                case 2:
                case 3:
                case 4:
                case 5:
                case 6:
                case 7:
                case 8:
                case 9:
                    if(0 == time.left_tens) {
                        time.left_ones = digit;
                        return Q_TRAN(&Microwave::ClockSelectMinuteTens);
                    }
                default:
                    break;
            }
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&Microwave::SetClock);
}

QState Microwave::ClockSelectMinuteTens(Microwave * const me, QEvt const * const e) {
    MicrowaveMsgFormat::Time &time = me->m_proposedClockTime;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            me->m_state = MicrowaveMsgFormat::State::CLOCK_SELECT_MINUTE_TENS;
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            me->UpdateClock(me->m_proposedClockTime);
            me->SendSignal(MicrowaveMsgFormat::Signal::MOD_RIGHT_ONES);
            return Q_HANDLED();
        }
        case MICROWAVE_EXT_DIGIT_SIG: {
            EVENT(e);
            MicrowaveExtDigitSig const &req = static_cast<MicrowaveExtDigitSig const &>(*e);
            uint32_t digit = req.GetDigit();

            switch(digit) {
                case 0:
                case 1:
                case 2:
                case 3:
                case 4:
                case 5:
                    time.right_tens = digit;
                    return Q_TRAN(&Microwave::ClockSelectMinuteOnes);
                default:
                    break;
            }
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&Microwave::SetClock);
}

QState Microwave::ClockSelectMinuteOnes(Microwave * const me, QEvt const * const e) {
    MicrowaveMsgFormat::Time &time = me->m_proposedClockTime;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            me->m_state = MicrowaveMsgFormat::State::CLOCK_SELECT_MINUTE_ONES;
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            me->UpdateClock(me->m_proposedClockTime);
            me->SendSignal(MicrowaveMsgFormat::Signal::MOD_LEFT_TENS);
            return Q_HANDLED();
        }
        case MICROWAVE_EXT_DIGIT_SIG: {
            EVENT(e);
            MicrowaveExtDigitSig const &req = static_cast<MicrowaveExtDigitSig const &>(*e);
            uint32_t digit = req.GetDigit();

            switch(digit) {
                case 0:
                case 1:
                case 2:
                case 3:
                case 4:
                case 5:
                case 6:
                case 7:
                case 8:
                case 9:
                    time.right_ones = digit;
                    return Q_TRAN(&Microwave::ClockSelectHourTens);
                default:
                    break;
            }
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&Microwave::SetClock);
}

QState Microwave::SetCookTimer(Microwave * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            me->m_displayTime[me->m_timerIndex].time = MicrowaveMsgFormat::Time();
            me->UpdateDisplayTime();
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
        case MICROWAVE_EXT_POWER_LEVEL_SIG: {
            EVENT(e);
            me->SendSignal(MicrowaveMsgFormat::Signal::POWER_LEVEL);
            return Q_TRAN(&Microwave::SetPowerLevel);
        }
    }
    return Q_SUPER(&Microwave::Started);
}

QState Microwave::SetCookTimerInitial(Microwave * const me, QEvt const * const e) {
    MicrowaveMsgFormat::Time &time = me->m_displayTime[me->m_timerIndex].time;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            me->m_state = MicrowaveMsgFormat::State::SET_COOK_TIMER_INITIAL;
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            me->UpdateDisplayTime();
            return Q_HANDLED();
        }
        case MICROWAVE_EXT_DIGIT_1_SIG: {
            EVENT(e);
            time.right_ones = 1;
            return Q_TRAN(&Microwave::SetCookTimerFinal);
        }
        case MICROWAVE_EXT_DIGIT_2_SIG: {
            EVENT(e);
            time.right_ones = 2;
            return Q_TRAN(&Microwave::SetCookTimerFinal);
        }
        case MICROWAVE_EXT_DIGIT_3_SIG: {
            EVENT(e);
            time.right_ones = 3;
            return Q_TRAN(&Microwave::SetCookTimerFinal);
        }
        case MICROWAVE_EXT_DIGIT_4_SIG: {
            EVENT(e);
            time.right_ones = 4;
            return Q_TRAN(&Microwave::SetCookTimerFinal);
        }
        case MICROWAVE_EXT_DIGIT_5_SIG: {
            EVENT(e);
            time.right_ones = 5;
            return Q_TRAN(&Microwave::SetCookTimerFinal);
        }
        case MICROWAVE_EXT_DIGIT_6_SIG: {
            EVENT(e);
            time.right_ones = 6;
            return Q_TRAN(&Microwave::SetCookTimerFinal);
        }
        case MICROWAVE_EXT_DIGIT_7_SIG: {
            EVENT(e);
            time.right_ones = 7;
            return Q_TRAN(&Microwave::SetCookTimerFinal);
        }
        case MICROWAVE_EXT_DIGIT_8_SIG: {
            EVENT(e);
            time.right_ones = 8;
            return Q_TRAN(&Microwave::SetCookTimerFinal);
        }
        case MICROWAVE_EXT_DIGIT_9_SIG: {
            EVENT(e);
            time.right_ones = 9;
            return Q_TRAN(&Microwave::SetCookTimerFinal);
        }
    }
    return Q_SUPER(&Microwave::SetCookTimer);
}

QState Microwave::SetCookTimerFinal(Microwave * const me, QEvt const * const e) {
    MicrowaveMsgFormat::Time &time = me->m_displayTime[me->m_timerIndex].time;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            me->m_state = MicrowaveMsgFormat::State::SET_COOK_TIMER_FINAL;
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
        case MICROWAVE_EXT_DIGIT_0_SIG: {
            EVENT(e);
            me->ShiftLeftAndInsert(time, 0);
            me->UpdateDisplayTime();
            return Q_HANDLED();
        }
        case MICROWAVE_EXT_DIGIT_1_SIG: {
            EVENT(e);
            me->ShiftLeftAndInsert(time, 1);
            me->UpdateDisplayTime();
            return Q_HANDLED();
        }
        case MICROWAVE_EXT_DIGIT_2_SIG: {
            EVENT(e);
            me->ShiftLeftAndInsert(time, 2);
            me->UpdateDisplayTime();
            return Q_HANDLED();
        }
        case MICROWAVE_EXT_DIGIT_3_SIG: {
            EVENT(e);
            me->ShiftLeftAndInsert(time, 3);
            me->UpdateDisplayTime();
            return Q_HANDLED();
        }
        case MICROWAVE_EXT_DIGIT_4_SIG: {
            EVENT(e);
            me->ShiftLeftAndInsert(time, 4);
            me->UpdateDisplayTime();
            return Q_HANDLED();
        }
        case MICROWAVE_EXT_DIGIT_5_SIG: {
            EVENT(e);
            me->ShiftLeftAndInsert(time, 5);
            me->UpdateDisplayTime();
            return Q_HANDLED();
        }
        case MICROWAVE_EXT_DIGIT_6_SIG: {
            EVENT(e);
            me->ShiftLeftAndInsert(time, 6);
            me->UpdateDisplayTime();
            return Q_HANDLED();
        }
        case MICROWAVE_EXT_DIGIT_7_SIG: {
            EVENT(e);
            me->ShiftLeftAndInsert(time, 7);
            me->UpdateDisplayTime();
            return Q_HANDLED();
        }
        case MICROWAVE_EXT_DIGIT_8_SIG: {
            EVENT(e);
            me->ShiftLeftAndInsert(time, 8);
            me->UpdateDisplayTime();
            return Q_HANDLED();
        }
        case MICROWAVE_EXT_DIGIT_9_SIG: {
            EVENT(e);
            me->ShiftLeftAndInsert(time, 9);
            me->UpdateDisplayTime();
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&Microwave::SetCookTimer);
}

QState Microwave::SetPowerLevel(Microwave * const me, QEvt const * const e) {
    uint32_t &powerLevel = me->m_displayTime[me->m_timerIndex].powerLevel;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            me->m_blink = true;
            powerLevel = MAX_POWER;
            me->UpdatePowerLevel();
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            me->m_blink = false;
            return Q_HANDLED();
        }
        case MICROWAVE_EXT_CLOCK_SIG: {
            EVENT(e);
            if(me->m_timersUsed < MAX_COOK_TIMERS) {
                me->m_timerIndex = (me->m_timerIndex + 1) % MAX_COOK_TIMERS;
                me->SendSignal(MicrowaveMsgFormat::Signal::COOK_TIME);
                return Q_TRAN(&Microwave::SetCookTimer);
            }
            return Q_HANDLED();
        }
        case MICROWAVE_EXT_DIGIT_0_SIG: {
            EVENT(e);
            if(powerLevel == 1) {
                powerLevel = 10;
            }
            else {
                powerLevel = 0;
            }
            me->UpdatePowerLevel();
            return Q_HANDLED();
        }
        case MICROWAVE_EXT_DIGIT_1_SIG: {
            EVENT(e);
            powerLevel = 1;
            me->UpdatePowerLevel();
            return Q_HANDLED();
        }
        case MICROWAVE_EXT_DIGIT_2_SIG: {
            EVENT(e);
            powerLevel = 2;
            me->UpdatePowerLevel();
            return Q_HANDLED();
        }
        case MICROWAVE_EXT_DIGIT_3_SIG: {
            EVENT(e);
            powerLevel = 3;
            me->UpdatePowerLevel();
            return Q_HANDLED();
        }
        case MICROWAVE_EXT_DIGIT_4_SIG: {
            EVENT(e);
            powerLevel = 4;
            me->UpdatePowerLevel();
            return Q_HANDLED();
        }
        case MICROWAVE_EXT_DIGIT_5_SIG: {
            EVENT(e);
            powerLevel = 5;
            me->UpdatePowerLevel();
            return Q_HANDLED();
        }
        case MICROWAVE_EXT_DIGIT_6_SIG: {
            EVENT(e);
            powerLevel = 6;
            me->UpdatePowerLevel();
            return Q_HANDLED();
        }
        case MICROWAVE_EXT_DIGIT_7_SIG: {
            EVENT(e);
            powerLevel = 7;
            me->UpdatePowerLevel();
            return Q_HANDLED();
        }
        case MICROWAVE_EXT_DIGIT_8_SIG: {
            EVENT(e);
            powerLevel = 8;
            me->UpdatePowerLevel();
            return Q_HANDLED();
        }
        case MICROWAVE_EXT_DIGIT_9_SIG: {
            EVENT(e);
            powerLevel = 9;
            me->UpdatePowerLevel();
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&Microwave::Started);
}

QState Microwave::SetKitchenTimer(Microwave * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            me->m_displayTime[me->m_timerIndex].time = MicrowaveMsgFormat::Time();
            ++me->m_timersUsed;
            
            me->UpdateDisplayTime();
            me->SendSignal(MicrowaveMsgFormat::Signal::MOD_LEFT_TENS);
            return Q_TRAN(&Microwave::KitchenSelectHourTens);
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&Microwave::Started);
}

QState Microwave::KitchenSelectHourTens(Microwave * const me, QEvt const * const e) {
    MicrowaveMsgFormat::Time &time = me->m_displayTime[me->m_timerIndex].time;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            me->m_state = MicrowaveMsgFormat::State::KITCHEN_SELECT_HOUR_TENS;
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            me->UpdateDisplayTime();
            me->SendSignal(MicrowaveMsgFormat::Signal::MOD_LEFT_ONES);
            return Q_HANDLED();
        }
        case MICROWAVE_EXT_DIGIT_0_SIG: {
            EVENT(e);
            time.left_tens = 0;
            return Q_TRAN(&Microwave::KitchenSelectHourOnes);
        }
        case MICROWAVE_EXT_DIGIT_1_SIG: {
            EVENT(e);
            time.left_tens = 1;
            return Q_TRAN(&Microwave::KitchenSelectHourOnes);
        }
        case MICROWAVE_EXT_DIGIT_2_SIG: {
            EVENT(e);
            time.left_tens = 2;
            return Q_TRAN(&Microwave::KitchenSelectHourOnes);
        }
        case MICROWAVE_EXT_DIGIT_3_SIG: {
            EVENT(e);
            time.left_tens = 3;
            return Q_TRAN(&Microwave::KitchenSelectHourOnes);
        }
        case MICROWAVE_EXT_DIGIT_4_SIG: {
            EVENT(e);
            time.left_tens = 4;
            return Q_TRAN(&Microwave::KitchenSelectHourOnes);
        }
        case MICROWAVE_EXT_DIGIT_5_SIG: {
            EVENT(e);
            time.left_tens = 5;
            return Q_TRAN(&Microwave::KitchenSelectHourOnes);
        }
        case MICROWAVE_EXT_DIGIT_6_SIG: {
            EVENT(e);
            time.left_tens = 6;
            return Q_TRAN(&Microwave::KitchenSelectHourOnes);
        }
        case MICROWAVE_EXT_DIGIT_7_SIG: {
            EVENT(e);
            time.left_tens = 7;
            return Q_TRAN(&Microwave::KitchenSelectHourOnes);
        }
        case MICROWAVE_EXT_DIGIT_8_SIG: {
            EVENT(e);
            time.left_tens = 8;
            return Q_TRAN(&Microwave::KitchenSelectHourOnes);
        }
        case MICROWAVE_EXT_DIGIT_9_SIG: {
            EVENT(e);
            time.left_tens = 9;
            return Q_TRAN(&Microwave::KitchenSelectHourOnes);
        }
    }
    return Q_SUPER(&Microwave::SetKitchenTimer);
}

QState Microwave::KitchenSelectHourOnes(Microwave * const me, QEvt const * const e) {
    MicrowaveMsgFormat::Time &time = me->m_displayTime[me->m_timerIndex].time;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            me->m_state = MicrowaveMsgFormat::State::KITCHEN_SELECT_HOUR_ONES;
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            me->UpdateDisplayTime();
            me->SendSignal(MicrowaveMsgFormat::Signal::MOD_RIGHT_TENS);
            return Q_HANDLED();
        }
        case MICROWAVE_EXT_DIGIT_0_SIG: {
            EVENT(e);
            time.left_ones = 0;
            return Q_TRAN(&Microwave::KitchenSelectMinuteTens);
        }
        case MICROWAVE_EXT_DIGIT_1_SIG: {
            EVENT(e);
            time.left_ones = 1;
            return Q_TRAN(&Microwave::KitchenSelectMinuteTens);
        }
        case MICROWAVE_EXT_DIGIT_2_SIG: {
            EVENT(e);
            time.left_ones = 2;
            return Q_TRAN(&Microwave::KitchenSelectMinuteTens);
        }
        case MICROWAVE_EXT_DIGIT_3_SIG: {
            EVENT(e);
            time.left_ones = 3;
            return Q_TRAN(&Microwave::KitchenSelectMinuteTens);
        }
        case MICROWAVE_EXT_DIGIT_4_SIG: {
            EVENT(e);
            time.left_ones = 4;
            return Q_TRAN(&Microwave::KitchenSelectMinuteTens);
        }
        case MICROWAVE_EXT_DIGIT_5_SIG: {
            EVENT(e);
            time.left_ones = 5;
            return Q_TRAN(&Microwave::KitchenSelectMinuteTens);
        }
        case MICROWAVE_EXT_DIGIT_6_SIG: {
            EVENT(e);
            time.left_ones = 6;
            return Q_TRAN(&Microwave::KitchenSelectMinuteTens);
        }
        case MICROWAVE_EXT_DIGIT_7_SIG: {
            EVENT(e);
            time.left_ones = 7;
            return Q_TRAN(&Microwave::KitchenSelectMinuteTens);
        }
        case MICROWAVE_EXT_DIGIT_8_SIG: {
            EVENT(e);
            time.left_ones = 8;
            return Q_TRAN(&Microwave::KitchenSelectMinuteTens);
        }
        case MICROWAVE_EXT_DIGIT_9_SIG: {
            EVENT(e);
            time.left_ones = 9;
            return Q_TRAN(&Microwave::KitchenSelectMinuteTens);
        }
    }
    return Q_SUPER(&Microwave::SetKitchenTimer);
}

QState Microwave::KitchenSelectMinuteTens(Microwave * const me, QEvt const * const e) {
	MicrowaveMsgFormat::Time &time = me->m_displayTime[me->m_timerIndex].time;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            me->m_state = MicrowaveMsgFormat::State::KITCHEN_SELECT_MINUTE_TENS;
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            me->UpdateDisplayTime();
            me->SendSignal(MicrowaveMsgFormat::Signal::MOD_RIGHT_ONES);
            return Q_HANDLED();
        }
        case MICROWAVE_EXT_DIGIT_0_SIG: {
            EVENT(e);
            time.right_tens = 0;
            return Q_TRAN(&Microwave::KitchenSelectMinuteOnes);
        }
        case MICROWAVE_EXT_DIGIT_1_SIG: {
            EVENT(e);
            time.right_tens = 1;
            return Q_TRAN(&Microwave::KitchenSelectMinuteOnes);
        }
        case MICROWAVE_EXT_DIGIT_2_SIG: {
            EVENT(e);
            time.right_tens = 2;
            return Q_TRAN(&Microwave::KitchenSelectMinuteOnes);
        }
        case MICROWAVE_EXT_DIGIT_3_SIG: {
            EVENT(e);
            time.right_tens = 3;
            return Q_TRAN(&Microwave::KitchenSelectMinuteOnes);
        }
        case MICROWAVE_EXT_DIGIT_4_SIG: {
            EVENT(e);
            time.right_tens = 4;
            return Q_TRAN(&Microwave::KitchenSelectMinuteOnes);
        }
        case MICROWAVE_EXT_DIGIT_5_SIG: {
            EVENT(e);
            time.right_tens = 5;
            return Q_TRAN(&Microwave::KitchenSelectMinuteOnes);
        }
        case MICROWAVE_EXT_DIGIT_6_SIG: {
            EVENT(e);
            time.right_tens = 6;
            return Q_TRAN(&Microwave::KitchenSelectMinuteOnes);
        }
        case MICROWAVE_EXT_DIGIT_7_SIG: {
            EVENT(e);
            time.right_tens = 7;
            return Q_TRAN(&Microwave::KitchenSelectMinuteOnes);
        }
        case MICROWAVE_EXT_DIGIT_8_SIG: {
            EVENT(e);
            time.right_tens = 8;
            return Q_TRAN(&Microwave::KitchenSelectMinuteOnes);
        }
        case MICROWAVE_EXT_DIGIT_9_SIG: {
            EVENT(e);
            time.right_tens = 9;
            return Q_TRAN(&Microwave::KitchenSelectMinuteOnes);
        }
    }
    return Q_SUPER(&Microwave::SetKitchenTimer);
}

QState Microwave::KitchenSelectMinuteOnes(Microwave * const me, QEvt const * const e) {
	MicrowaveMsgFormat::Time& time = me->m_displayTime[me->m_timerIndex].time;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            me->m_state = MicrowaveMsgFormat::State::KITCHEN_SELECT_MINUTE_ONES;
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            me->UpdateDisplayTime();
            me->SendSignal(MicrowaveMsgFormat::Signal::MOD_LEFT_TENS);
            return Q_HANDLED();
        }
        case MICROWAVE_EXT_DIGIT_0_SIG: {
            EVENT(e);
            time.right_ones = 0;
            return Q_TRAN(&Microwave::KitchenSelectHourTens);
        }
        case MICROWAVE_EXT_DIGIT_1_SIG: {
            EVENT(e);
            time.right_ones = 1;
            return Q_TRAN(&Microwave::KitchenSelectHourTens);
        }
        case MICROWAVE_EXT_DIGIT_2_SIG: {
            EVENT(e);
            time.right_ones = 2;
            return Q_TRAN(&Microwave::KitchenSelectHourTens);
        }
        case MICROWAVE_EXT_DIGIT_3_SIG: {
            EVENT(e);
            time.right_ones = 3;
            return Q_TRAN(&Microwave::KitchenSelectHourTens);
        }
        case MICROWAVE_EXT_DIGIT_4_SIG: {
            EVENT(e);
            time.right_ones = 4;
            return Q_TRAN(&Microwave::KitchenSelectHourTens);
        }
        case MICROWAVE_EXT_DIGIT_5_SIG: {
            EVENT(e);
            time.right_ones = 5;
            return Q_TRAN(&Microwave::KitchenSelectHourTens);
        }
        case MICROWAVE_EXT_DIGIT_6_SIG: {
            EVENT(e);
            time.right_ones = 6;
            return Q_TRAN(&Microwave::KitchenSelectHourTens);
        }
        case MICROWAVE_EXT_DIGIT_7_SIG: {
            EVENT(e);
            time.right_ones = 7;
            return Q_TRAN(&Microwave::KitchenSelectHourTens);
        }
        case MICROWAVE_EXT_DIGIT_8_SIG: {
            EVENT(e);
            time.right_ones = 8;
            return Q_TRAN(&Microwave::KitchenSelectHourTens);
        }
        case MICROWAVE_EXT_DIGIT_9_SIG: {
            EVENT(e);
            time.right_ones = 9;
            return Q_TRAN(&Microwave::KitchenSelectHourTens);
        }
    }
    return Q_SUPER(&Microwave::SetKitchenTimer);
}

QState Microwave::DisplayTimer(Microwave * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            me->m_secondsRemaining = me->Time2Seconds(me->m_displayTime[me->m_timerIndex].time);
            return Q_TRAN(&Microwave::DisplayTimerRunning);
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
        case MICROWAVE_EXT_START_SIG: {
            EVENT(e);
            if(me->m_cooking) {
                me->Add30SecondsToCookTime();    
                me->UpdateDisplayTime();
            }
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&Microwave::Started);
}

QState Microwave::DisplayTimerRunning(Microwave * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            me->m_secondTimer.Start(me->m_secondsRemaining);
            me->m_state = MicrowaveMsgFormat::State::DISPLAY_TIMER_RUNNING;
            me->UpdateDisplayTime();

            if(me->m_cook) {
                me->m_cooking = true;
                //write the current power level to the magnetron pipe
                const uint32_t count {me->m_magnetronPipe.Write(&me->m_displayTime[me->m_timerIndex].powerLevel, 1)};
                if(0 == count) {
                    LOG("Write to MagnetronPipe failed\n\r");
                }

                Evt* evt = new MwLampOnReq(MW_LAMP, GET_HSMN(), GEN_SEQ());
                me->PostSync(evt);
                evt = new FanOnReq(FAN, GET_HSMN(), GEN_SEQ());
                me->PostSync(evt);
                evt = new TurntableOnReq(TURNTABLE, GET_HSMN(), GEN_SEQ());
                me->PostSync(evt);
                evt = new MagnetronOnReq(MAGNETRON, GET_HSMN(), GEN_SEQ(), &me->m_magnetronPipe);
                Fw::Post(evt);
            }
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            me->m_cooking = false;
            return Q_HANDLED();
        }
        case SECOND_TIMER: {
            EVENT(e);
            me->DecrementTimer();

            if(me->m_secondsRemaining == 0) {
                //all timers done, transition back to DisplayClock
                return Q_TRAN(&Microwave::DisplayClock);
            }
            return Q_HANDLED();
        }
        case MICROWAVE_EXT_STOP_SIG: {
            EVENT(e);
            return Q_TRAN(&Microwave::DisplayTimerPaused);
        }
        case MICROWAVE_EXT_DOOR_OPEN_SIG: {
            EVENT(e);
            me->m_closed = false;
            return Q_TRAN(&Microwave::DisplayTimerPaused);
        }
    }
    return Q_SUPER(&Microwave::DisplayTimer);
}

QState Microwave::DisplayTimerPaused(Microwave * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            me->m_secondTimer.Stop();
            me->m_state = MicrowaveMsgFormat::State::DISPLAY_TIMER_PAUSED;

            Evt* evt = new FanOffReq(FAN, GET_HSMN(), GEN_SEQ());
            me->PostSync(evt);
            evt = new TurntableOffReq(TURNTABLE, GET_HSMN(), GEN_SEQ());
            me->PostSync(evt);
            if(me->m_closed) {
                evt = new MwLampOffReq(MW_LAMP, GET_HSMN(), GEN_SEQ());
                me->PostSync(evt);
            }
            evt = new MagnetronPauseReq(MAGNETRON, GET_HSMN(), GEN_SEQ());
            Fw::Post(evt);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
        case MICROWAVE_EXT_DOOR_OPEN_SIG: {
            EVENT(e);
            if(me->m_closed) {
                me->m_closed = false;
                Evt* evt = new MwLampOnReq(MW_LAMP, GET_HSMN(), GEN_SEQ());
                me->PostSync(evt);
            }
            return Q_HANDLED();
        }
        case MICROWAVE_EXT_DOOR_CLOSED_SIG: {
            EVENT(e);
            if(!me->m_closed) {
                me->m_closed = true;
                Evt* evt = new MwLampOffReq(MW_LAMP, GET_HSMN(), GEN_SEQ());
                me->PostSync(evt);
            }
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&Microwave::DisplayTimer);
}

void Microwave::SendState(const MicrowaveMsgFormat::State state) {
    m_message.state = state;
    SendMessage(m_message);
}

void Microwave::SendSignal(const MicrowaveMsgFormat::Signal signal) {
    m_message.signal = signal;
    SendMessage(m_message);
}

void Microwave::SendMessage(const MicrowaveMsgFormat::Message& message) {
    char buf[sizeof(MicrowaveMsgFormat::Message)];
    memcpy(buf, &message, sizeof(MicrowaveMsgFormat::Message));
    Evt* evt = new WifiSendReq(WIFI_ST, this->GetHsmn(), this->GenSeq(), buf);
    Fw::Post(evt);
}

void Microwave::UpdateClock(const MicrowaveMsgFormat::Time& clock) {
    SendUpdate(MicrowaveMsgFormat::Update::CLOCK, clock);
}

void Microwave::UpdatePowerLevel() {
    const uint32_t &powerLevel = m_displayTime[m_timerIndex].powerLevel;
    SendUpdate(MicrowaveMsgFormat::Update::POWER_LEVEL, powerLevel);
}

void Microwave::UpdateDisplayTime() {
    const MicrowaveMsgFormat::Time &time = m_displayTime[m_timerIndex].time;
    SendUpdate(MicrowaveMsgFormat::Update::DISPLAY_TIMER, time);
}

void Microwave::ShiftLeftAndInsert(MicrowaveMsgFormat::Time& time, const uint32_t digit) {
    time.left_tens = time.left_ones;
    time.left_ones = time.right_tens;
    time.right_tens = time.right_ones;
    time.right_ones = digit;
}

MicrowaveMsgFormat::Time Microwave::Seconds2Time(uint32_t seconds) const {
    static const uint32_t maxSeconds = 5999; //corresponds to a time of 99:59
    if(seconds > maxSeconds) seconds = maxSeconds;

    uint32_t min = seconds / 60;
    uint32_t sec = seconds % 60;

    MicrowaveMsgFormat::Time time;
    time.left_tens = min / 10;
    time.left_ones = min % 10;
    time.right_tens = sec / 10;
    time.right_ones = sec % 10;

    return time;
}

uint32_t Microwave::Time2Seconds(const MicrowaveMsgFormat::Time& time) const {
    uint32_t min{};
    uint32_t sec{};

    sec += time.right_ones;
    sec += (time.right_tens * 10);
    min += time.left_ones;
    min += (time.left_tens * 10);

    return (60 * min) + sec;
}

void Microwave::DecrementTimer() {
    m_secondsRemaining -= 1;
    m_displayTime[m_timerIndex].time = Seconds2Time(m_secondsRemaining);
    if(0 == m_secondsRemaining) {
        m_secondTimer.Stop();
        Evt* evt = new MagnetronOffReq(MAGNETRON, this->GetHsmn(), this->GenSeq());
        Fw::Post(evt);
        
        --m_timersUsed;

        if(m_timersUsed > 0) {
            m_timerIndex = (m_timerIndex + 1) % MAX_COOK_TIMERS;

            m_secondsRemaining = Time2Seconds(m_displayTime[m_timerIndex].time);
            m_magnetronPipe.Write(&m_displayTime[m_timerIndex].powerLevel, 1);
            m_secondTimer.Start(m_secondsRemaining);
            evt = new MagnetronOnReq(MAGNETRON, this->GetHsmn(), this->GenSeq(), &m_magnetronPipe);
            Fw::Post(evt);
            
            UpdateDisplayTime();
        }
    }
    else {
        UpdateDisplayTime();
    }
}

void Microwave::Add30SecondsToCookTime() {
    m_secondsRemaining += 30;
    m_displayTime[m_timerIndex].time = Seconds2Time(m_secondsRemaining);
}

void Microwave::IncrementClock() {
    //incrementing by 1 minute HH:MM
    ++m_clockTime.right_ones;
    if(10 == m_clockTime.right_ones) {
        m_clockTime.right_ones = 0;
        ++m_clockTime.right_tens;
        if(6 == m_clockTime.right_tens) {
            m_clockTime.right_tens = 0;
            ++m_clockTime.left_ones;
            if(1 == m_clockTime.left_tens) {
                if(3 == m_clockTime.left_ones) {
                    m_clockTime.left_tens = 0;
                    m_clockTime.left_ones = 1;
                }
            }
            else {
                if(10 == m_clockTime.left_ones) {
                    m_clockTime.left_tens = 1;
                    m_clockTime.left_ones = 0;
                }
            }
        }
    }
    
    UpdateClock(m_clockTime);
}

/*
QState Microwave::MyState(Microwave * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
        case Q_INIT_SIG: {
            return Q_TRAN(&Microwave::SubState);
        }
    }
    return Q_SUPER(&Microwave::SuperState);
}
*/

} // namespace APP
