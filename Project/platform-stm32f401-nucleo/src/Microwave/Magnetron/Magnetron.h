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

#ifndef MAGNETRON_H
#define MAGNETRON_H

#include "qpcpp.h"
#include "fw_active.h"
#include "fw_timer.h"
#include "fw_evt.h"
#include "app_hsmn.h"

using namespace QP;
using namespace FW;

namespace APP {

class Magnetron : public Active {
public:
    Magnetron();

protected:
    static QState InitialPseudoState(Magnetron * const me, QEvt const * const e);
    static QState Root(Magnetron * const me, QEvt const * const e);
        static QState Stopped(Magnetron * const me, QEvt const * const e);
        static QState Starting(Magnetron * const me, QEvt const * const e);
        static QState Stopping(Magnetron * const me, QEvt const * const e);
        static QState Started(Magnetron * const me, QEvt const * const e);
            static QState Off(Magnetron * const me, QEvt const * const e);
            static QState On(Magnetron * const me, QEvt const * const e);
                static QState Running(Magnetron * const me, QEvt const * const e);
                static QState NotRunning(Magnetron * const me, QEvt const * const e);
            static QState Paused(Magnetron * const me, QEvt const * const e);

    enum {
        CYCLE_TIME_MS = 30000,
    };
    
    Timer m_stateTimer;
    Timer m_magnetronTimer;
    uint32_t m_onTime;
    uint32_t m_offTime;
    uint32_t m_remainingTime;

    enum {
    	MIN_POWER = 0,
		MAX_POWER = 10,
    };

    uint32_t m_powerLevel;
    MagnetronPipe* m_pipe;
    
    // m_history is a function pointer used for saving the previous state.
    QState (*m_history) (Magnetron * const me, QEvt const * const e);

#define MAGNETRON_TIMER_EVT \
    ADD_EVT(STATE_TIMER) \
    ADD_EVT(MAGNETRON_TIMER)

#define MAGNETRON_INTERNAL_EVT \
	ADD_EVT(DONE) \
    ADD_EVT(FAILED)

#undef ADD_EVT
#define ADD_EVT(e_) e_,

    enum {
        MAGNETRON_TIMER_EVT_START = TIMER_EVT_START(MAGNETRON),
        MAGNETRON_TIMER_EVT
    };

    enum {
    	MAGNETRON_INTERNAL_EVT_START = INTERNAL_EVT_START(MAGNETRON),
		MAGNETRON_INTERNAL_EVT
    };

    class Failed : public ErrorEvt {
    public:
        Failed(Hsmn hsmn, Error error, Hsmn origin, Reason reason) :
            ErrorEvt(FAILED, hsmn, hsmn, 0, error, origin, reason) {}
    };
};

} // namespace APP

#endif // MAGNETRON_H
