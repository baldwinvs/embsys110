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
#include "TurntableInterface.h"
#include "Turntable.h"

FW_DEFINE_THIS_FILE("Turntable.cpp")

namespace APP {

#undef ADD_EVT
#define ADD_EVT(e_) #e_,

static char const * const timerEvtName[] = {
	"TURNTABLE_TIMER_EVT_START",
	TURNTABLE_TIMER_EVT
};

static char const * const internalEvtName[] = {
	"TURNTABLE_INTERNAL_EVT_START",
	TURNTABLE_INTERNAL_EVT
};

static char const * const interfaceEvtName[] = {
	"TURNTABLE_INTERFACE_EVT_START",
	TURNTABLE_INTERFACE_EVT
};

Turntable::Turntable(Hsmn hsmn, const char *name) :
    Region((QStateHandler)&Turntable::InitialPseudoState, hsmn, name) {
    SET_EVT_NAME(TURNTABLE);
}

QState Turntable::InitialPseudoState(Turntable * const me, QEvt const * const e) {
    (void)e;
    return Q_TRAN(&Turntable::Root);
}

QState Turntable::Root(Turntable * const me, QEvt const * const e) {
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
            return Q_TRAN(&Turntable::Off);
        }
        case TURNTABLE_ON_REQ: {
            EVENT(e);
            return Q_TRAN(&Turntable::On);
        }
        case TURNTABLE_OFF_REQ: {
            EVENT(e);
            return Q_TRAN(&Turntable::Off);
        }
    }
    return Q_SUPER(&QHsm::top);
}

QState Turntable::On(Turntable * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
        case TURNTABLE_OFF_REQ: {
            EVENT(e);
            return Q_TRAN(&Turntable::Off);
        }
    }
    return Q_SUPER(&Turntable::Root);
}

QState Turntable::Off(Turntable * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
        case TURNTABLE_ON_REQ: {
            EVENT(e);
            return Q_TRAN(&Turntable::On);
        }
    }
    return Q_SUPER(&Turntable::Root);
}

/*
QState Turntable::MyState(Turntable * const me, QEvt const * const e) {
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
            return Q_TRAN(&Turntable::SubState);
        }
    }
    return Q_SUPER(&Turntable::SuperState);
}
*/

} // namespace APP

