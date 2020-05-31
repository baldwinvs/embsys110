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
    m_cook{false},
    m_closed{true},
    m_clockTime{},
    m_timerIndex{},
    m_secondsRemaining{},
    m_displayTimer{{},{}},
    m_fan{FAN, "FAN"},
    m_lamp{MW_LAMP, "MW_LAMP"},
    m_turntable{TURNTABLE, "TURNTABLE"},
    m_halfSecondTimer{GetHsm().GetHsmn(), IDLE_TIMER},
    m_secondTimer{GetHsm().GetHsmn(), IDLE_TIMER},
    m_pipe{m_magnetronStor, MAGNETRON_PIPE_ORDER}
    {
        SET_EVT_NAME(MICROWAVE);
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
            Evt *evt = new MicrowaveStartCfm(req.GetFrom(), GET_HSMN(), req.GetSeq(), ERROR_STATE, GET_HSMN());
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
            Evt *evt = new MicrowaveStartCfm(req.GetFrom(), GET_HSMN(), req.GetSeq(), ERROR_STATE, GET_HSMN());
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
            evt = new MWLampOffReq(MW_LAMP, GET_HSMN());
            me->PostSync(evt);
            evt = new TurntableOffReq(TURNTABLE, GET_HSMN());
            me->PostSync(evt);

            evt = new MicrowaveStopCfm(req.GetFrom(), GET_HSMN(), req.GetSeq(), ERROR_SUCCESS);
            Fw::Post(evt);
            return Q_TRAN(&Microwave::Stopped);
        }
    }
    return Q_SUPER(&Microwave::Root);
}

QState Microwave::DisplayClock(Microwave * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&Microwave::Started);
}

QState Microwave::SetClock(Microwave * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            //send Signal::MOD_LEFT_TENS
            return Q_TRAN(&Microwave::ClockSelectHourTens);
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&Microwave::DisplayClock);
}

QState Microwave::ClockSelectHourTens(Microwave * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&Microwave::SetClock);
}

QState Microwave::ClockSelectHourOness(Microwave * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&Microwave::SetClock);
}

QState Microwave::ClockSelectMinuteTens(Microwave * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&Microwave::SetClock);
}

QState Microwave::ClockSelectMinuteOnes(Microwave * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&Microwave::SetClock);
}

QState Microwave::SetCookTimer(Microwave * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&Microwave::Started);
}

QState Microwave::SetPowerLevel(Microwave * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&Microwave::Started);
}

QState Microwave::SetKitchenTimer(Microwave * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
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
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&Microwave::KitchenTimer);
}

QState Microwave::KitchenSelectHourOnes(Microwave * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&Microwave::KitchenTimer);
}

QState Microwave::KitchenSelectMinuteTens(Microwave * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&Microwave::KitchenTimer);
}

QState Microwave::KitchenSelectMinuteOnes(Microwave * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&Microwave::KitchenTimer);
}

QState Microwave::DisplayTimer(Microwave * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            me->m_secondsRemaining = me->time2Seconds(me->displayTime[me->m_timerIndex].time);
            return Q_TRAN(&Microwave::DisplayTimerRunning);
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
        case MICROWAVE_EXT_START_SIG: {
            if(me->m_cooking) {
                me->Add30SecondsToCookTime();
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

            //TODO
            //write msg to wifi module
            // send Update::DISPLAY_TIMER with me->m_displayTime[me->m_timerIndex].time

            if(me->m_cook) {
                me->m_cooking = true;
                //TODO: check the return count, handle if 0?
                //write the current power level to the magnetron pipe
                m_magnetronPipe.Write(&displayTime[timerIndex].powerLevel, 1);

                Evt* evt = new MWLampOnReq(MW_LAMP, GET_HSMN(), GEN_SEQ());
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
                evt = new MWLampOffReq(MW_LAMP, GET_HSMN(), GEN_SEQ());
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
                Evt* evt = new MWLampOnReq(MW_LAMP, GET_HSMN(), GEN_SEQ());
                me->PostSync(evt);
            }
            return Q_HANDLED();
        }
        case MICROWAVE_EXT_DOOR_CLOSED_SIG: {
            EVENT(e);
            if(!me->m_closed) {
                me->m_closed = true;
                Evt* evt = new MWLampOffReq(MW_LAMP, GET_HSMN(), GEN_SEQ());
                me->PostSync(evt);
            }
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&Microwave::DisplayTimer);
}

MicrowaveMsgFormat::Time Microwave::seconds2Time(const uint32_t seconds) const {
    static const maxSeconds = 5999; //corresponds to a time of 99:59
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

uint32_t Microwave::time2Seconds(const MicrowaveMsgFormat::Time& time) const {
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
    m_displayTime[m_timerIndex].time = seconds2Time(m_secondsRemaining);
    if(0 == secondsRemaining) {
        m_secondTimer.Stop();
        Evt* evt = new MagnetronOffReq(MAGNETRON, GET_HSMN(), GEN_SEQ());
        Fw::Post(evt);
        
        m_timerIndex = (m_timerIndex + 1) % MAX_COOK_TIMERS;

        m_secondsRemaining = time2Seconds(m_displayTime[m_timerIndex].time);
        if(0 == secondsRemaining) {
            return;
        }
        /* only cooking can have 2 times */
        else {
            m_magnetronPipe.Write(&m_displayTime[m_timerIndex].powerLevel, 1);
            m_secondTimer.Start(m_secondsRemaining);
            evt = new MagnetronOnReq(MAGNATRON, GET_HSMN(), GEN_SEQ());
            Fw::Post(evt);

            //TODO
            //write msg to wifi module
            // send Update::DISPLAY_TIMER with displayTime[timerIndex].time
        }
    }
    else {
        //TODO
        //write msg to wifi module
        // send Update::DISPLAY_TIMER with displayTime[timerIndex].time
    }
}

void Microwave::Add30SecondsToCookTime() {
    m_secondsRemaining += 30;
    m_displayTime[m_timerIndex].time = seconds2Time(m_secondsRemaining);

    //TODO
    //write msg to wifi module
    // send Update::DISPLAY_TIMER with m_displayTime[m_timerIndex].time
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
    
    //TODO
    //write msg to wifi module
    // send Update::CLOCK with m_clockTime
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
