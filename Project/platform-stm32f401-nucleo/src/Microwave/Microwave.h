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

#ifndef MICROWAVE_H
#define MICROWAVE_H

#include "qpcpp.h"
#include "fw_active.h"
#include "fw_timer.h"
#include "fw_evt.h"
#include "app_hsmn.h"
#include "Fan.h"
#include "MWLamp.h"
#include "Turntable.h"
#include "MicrowaveMsgFormat.h"
#include "MagnetronInterface.h"

using namespace QP;
using namespace FW;

namespace APP {

class Microwave : public Active {
public:
    Microwave();

protected:
    static QState InitialPseudoState(Microwave * const me, QEvt const * const e);
    static QState Root(Microwave * const me, QEvt const * const e);
        static QState Stopped(Microwave * const me, QEvt const * const e);
        static QState Started(Microwave * const me, QEvt const * const e);
            static QState DisplayClock(Microwave * const me, QEvt const * const e);
                static QState SetClock(Microwave * const me, QEvt const * const e);
                    static QState ClockSelectHourTens(Microwave * const me, QEvt const * const e);
                    static QState ClockSelectHourOnes(Microwave * const me, QEvt const * const e);
                    static QState ClockSelectMinuteTens(Microwave * const me, QEvt const * const e);
                    static QState ClockSelectMinuteOnes(Microwave * const me, QEvt const * const e);
            static QState SetCookTimer(Microwave * const me, QEvt const * const e);
                static QState SetCookTimerInitial(Microwave * const me, QEvt const * const e);
                static QState SetCookTimerFinal(Microwave * const me, QEvt const * const e);
            static QState SetPowerLevel(Microwave * const me, QEvt const * const e);
            static QState SetKitchenTimer(Microwave * const me, QEvt const * const e);
                static QState KitchenSelectHourTens(Microwave * const me, QEvt const * const me);
                static QState KitchenSelectHourOnes(Microwave * const me, QEvt const * const me);
                static QState KitchenSelectMinuteTens(Microwave * const me, QEvt const * const me);
                static QState KitchenSelectMinuteOnes(Microwave * const me, QEvt const * const me);
            static QState DisplayTimer(Microwave * const me, QEvt const * const e);
                static QState DisplayTimerRunning(Microwave * const me, QEvt const * const e);
                static QState DisplayTimerPaused(Microwave * const me, QEvt const * const e);

    struct DisplayTime {
        MicrowaveMsgFormat::Time time;
        uint32_t powerLevel;
    };

    template<typename Data>
    void SendUpdate(const MicrowaveMsgFormat::Update update, const Data& data) {
        m_message.update = update;
        memcpy(m_message.data, &data, sizeof(data));
        SendMessage(m_message);
    }
    void SendState(const MicrowaveMsgFormat::State state);
    void SendSignal(const MicrowaveMsgFormat::Signal signal);
    void SendMessage(const MicrowaveMsgFormat::Message& message);

    void UpdateClock();
    void UpdatePowerLevel();
    void UpdateDisplayTime();

    void ShiftLeftAndInsert(MicrowaveMsgFormat::Time& time, const uint32_t digit);
    MicrowaveMsgFormat::Time Seconds2Time(const uint32_t seconds) const;
    uint32_t Time2Seconds(const MicrowaveMsgFormat::Time& time) const;

    void DecrementTimer();
    void Add30SecondsToCookTime();
    void IncrementClock();

    bool m_blink;
    bool m_blinkToggle;
    bool m_cook;
    bool m_closed;
    MicrowaveMsgFormat::Message m_message;
    MicrowaveMsgFormat::Time m_clockTime;
    MicrowaveMsgFormat::Time m_proposedClockTime;
    MicrowaveMsgFormat::State m_state;

    enum {
        MAX_COOK_TIMERS = 2,
    };

    enum {
        MAX_POWER = 10,
    }
    
    uint32_t m_timersUsed;
    uint32_t m_timerIndex;
    uint32_t m_secondsRemaining;
    DisplayTime m_displayTime[MAX_COOK_TIMERS];

    Fan m_fan;              // Orthogonal region for the Fan
    MWLamp m_lamp;          // Orthogonal region for the Microwave Lamp
    Turntable m_turntable;  // Orthogonal region for the Turntable

    enum {
        HALF_SECOND_TIMEOUT_MS = 500,
        SECOND_TIMEOUT_MS      = 1000,
    };

    Timer m_halfSecondTimer;
    Timer m_secondTimer;

    enum {
        HALF_SECOND_COUNTS_PER_MINUTE = 120,
    };

    uint32_t m_halfSecondCounts;

    enum {
        MAGNETRON_PIPE_ORDER = 1
    };

    uint32_t m_magnetronStor[1 << MAGNETRON_PIPE_ORDER];
    MagnetronPipe m_magnetronPipe;

#define MICROWAVE_TIMER_EVT \
    ADD_EVT(HALF_SECOND_TIMER) \
    ADD_EVT(SECOND_TIMER)

#define MICROWAVE_INTERNAL_EVT \
    ADD_EVT(DONE) \
    ADD_EVT(FAILED)

#undef ADD_EVT
#define ADD_EVT(e_) e_,

    enum {
        MICROWAVE_TIMER_EVT_START = TIMER_EVT_START(MICROWAVE),
        MICROWAVE_TIMER_EVT
    };

    enum {
        MICROWAVE_INTERNAL_EVT_START = INTERNAL_EVT_START(MICROWAVE),
        MICROWAVE_INTERNAL_EVT
    };

    class Failed : public ErrorEvt {
    public:
        Failed(Hsmn hsmn, Error error, Hsmn origin, Reason reason) :
            ErrorEvt(FAILED, hsmn, hsmn, 0, error, origin, reason) {}
    };
};

} // namespace APP

#endif // MICROWAVE_H
