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
#include "MagnetronInterface.h"
#include "Magnetron.h"

FW_DEFINE_THIS_FILE("Magnetron.cpp")

namespace APP {

#undef ADD_EVT
#define ADD_EVT(e_) #e_,

static char const * const timerEvtName[] = {
    "MAGNETRON_TIMER_EVT_START",
    MAGNETRON_TIMER_EVT
};

static char const * const internalEvtName[] = {
    "MAGNETRON_INTERNAL_EVT_START",
    MAGNETRON_INTERNAL_EVT
};

static char const * const interfaceEvtName[] = {
    "MAGNETRON_INTERFACE_EVT_START",
    MAGNETRON_INTERFACE_EVT
};

Magnetron::Magnetron() :
    Active((QStateHandler)&Magnetron::InitialPseudoState, MAGNETRON, "MAGNETRON"),
    m_magnetronTimer(GetHsm().GetHsmn(), MAGNETRON_TIMER),
    m_onTime{},
    m_offTime{},
    m_remainingTime{},
    m_pipe{nullptr},
    m_history{nullptr}
    {
        SET_EVT_NAME(MAGNETRON);
    }

QState Magnetron::InitialPseudoState(Magnetron * const me, QEvt const * const e) {
    (void)e;
    return Q_TRAN(&Magnetron::Root);
}

QState Magnetron::Root(Magnetron * const me, QEvt const * const e) {
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
            EVENT(e);
            return Q_TRAN(&Magnetron::Stopped);
        }
        case MAGNETRON_START_REQ: {
            EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new MagnetronStartCfm(req.GetFrom(), GET_HSMN(), req.GetSeq(), ERROR_STATE);
            Fw::Post(evt);
            return Q_HANDLED();
        }
        case MAGNETRON_STOP_REQ: {
            EVENT(e);
            Evt const &req = EVT_CAST(*e);
            me->GetHsm().SaveInSeq(req);
            return Q_TRAN(&Magnetron::Stopped);
        }
    }
    return Q_SUPER(&QHsm::top);
}

QState Magnetron::Stopped(Magnetron * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
        case MAGNETRON_STOP_REQ: {
            EVENT(e);
            Evt const &req = EVT_CAST(*e);
            Evt *evt = new MagnetronStopCfm(req.GetFrom(), GET_HSMN(), req.GetSeq(), ERROR_SUCCESS);
            Fw::Post(evt);
            return Q_HANDLED();
        }
        case MAGNETRON_START_REQ: {
            EVENT(e);
            Evt const &req = EVT_CAST(*e);
            me->GetHsm().SaveInSeq(req);
            return Q_TRAN(&Magnetron::Started);
        }
    }
    return Q_SUPER(&Magnetron::Root);
}

QState Magnetron::Started(Magnetron * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            return Q_TRAN(&Magnetron::Off);
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
        case MAGNETRON_OFF_REQ: {
            EVENT(e);
            return Q_TRAN(&Magnetron::Off);
        }
        case MAGNETRON_ON_REQ: {
            EVENT(e);
            //get data from pipe
            MagnetronOnReq const &req = static_cast<MagnetronOnReq const &>(*e);
            me->m_pipe = req.GetPipe();
            uint32_t powerLevel{};
            uint32_t count {me->m_pipe->Read(&powerLevel, 1)};
            if(0 == count) {
                LOG("Could not read from magnetron pipe\n");
                return Q_HANDLED();
            }

            //calculate on/off times
            me->m_onTime = static_cast<float>(powerLevel/10.0f) * APP::Magnetron::CYCLE_TIME_MS;
            me->m_offTime = static_cast<uint32_t>(APP::Magnetron::CYCLE_TIME_MS) - me->m_onTime;
            LOG("\tpower level: %d\n"
                "\ton time    : %d\n"
                "\toff time   : %d\n", powerLevel, me->m_onTime, me->m_offTime);
            //start magnetron timer
            me->m_magnetronTimer.Start(me->m_onTime);
            return Q_TRAN(&Magnetron::On);
        }
    }
    return Q_SUPER(&Magnetron::Root);
}

QState Magnetron::Off(Magnetron * const me, QEvt const * const e) {
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
    return Q_SUPER(&Magnetron::Started);
}

QState Magnetron::On(Magnetron * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            return Q_TRAN(&Magnetron::Running);
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
        case MAGNETRON_PAUSE_REQ: {
            EVENT(e);
            me->m_remainingTime = me->m_magnetronTimer.currCtr();
            me->m_magnetronTimer.Stop();
            return Q_TRAN(&Magnetron::Paused);
        }
    }
    return Q_SUPER(&Magnetron::Started);
}

QState Magnetron::Running(Magnetron * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            me->m_history = &Magnetron::Running;
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
        case MAGNETRON_TIMER: {
            EVENT(e);
            me->m_magnetronTimer.Stop();
            me->m_magnetronTimer.Start(me->m_offTime);
            return Q_TRAN(&Magnetron::NotRunning);
        }
    }
    return Q_SUPER(&Magnetron::On);
}

QState Magnetron::NotRunning(Magnetron * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            me->m_history = &Magnetron::NotRunning;
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
        case MAGNETRON_TIMER: {
            EVENT(e);
            me->m_magnetronTimer.Stop();
            me->m_magnetronTimer.Start(me->m_onTime);
            return Q_TRAN(&Magnetron::Running);
        }
    }
    return Q_SUPER(&Magnetron::On);
}

QState Magnetron::Paused(Magnetron * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
        case MAGNETRON_ON_REQ: {
            EVENT(e);
            me->m_magnetronTimer.Start(me->m_remainingTime);
            return Q_TRAN(&me->m_history);
        }
    }
    return Q_SUPER(&Magnetron::Started);
}



/*
QState Magnetron::MyState(Magnetron * const me, QEvt const * const e) {
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
            return Q_TRAN(&Magnetron::SubState);
        }
    }
    return Q_SUPER(&Magnetron::SuperState);
}
*/

} // namespace APP
