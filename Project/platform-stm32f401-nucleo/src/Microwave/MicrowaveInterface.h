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

#ifndef MICROWAVE_INTERFACE_H
#define MICROWAVE_INTERFACE_H

#include "fw_def.h"
#include "fw_evt.h"
#include "fw_pipe.h"
#include "app_hsmn.h"

using namespace QP;
using namespace FW;

namespace APP {

#define MICROWAVE_INTERFACE_EVT \
    ADD_EVT(MICROWAVE_START_REQ) \
    ADD_EVT(MICROWAVE_START_CFM) \
    ADD_EVT(MICROWAVE_STOP_REQ) \
    ADD_EVT(MICROWAVE_STOP_CFM) \
    ADD_EVT(MICROWAVE_EXT_STOP_SIG) \
    ADD_EVT(MICROWAVE_EXT_START_SIG) \
    ADD_EVT(MICROWAVE_EXT_COOK_TIME_SIG) \
    ADD_EVT(MICROWAVE_EXT_POWER_LEVEL_SIG) \
    ADD_EVT(MICROWAVE_EXT_KITCHEN_TIMER_SIG) \
    ADD_EVT(MICROWAVE_EXT_DOOR_OPEN_SIG) \
    ADD_EVT(MICROWAVE_EXT_DOOR_CLOSED_SIG) \
    ADD_EVT(MICROWAVE_EXT_DIGIT_0_SIG) \
    ADD_EVT(MICROWAVE_EXT_DIGIT_1_SIG) \
    ADD_EVT(MICROWAVE_EXT_DIGIT_2_SIG) \
    ADD_EVT(MICROWAVE_EXT_DIGIT_3_SIG) \
    ADD_EVT(MICROWAVE_EXT_DIGIT_4_SIG) \
    ADD_EVT(MICROWAVE_EXT_DIGIT_5_SIG) \
    ADD_EVT(MICROWAVE_EXT_DIGIT_6_SIG) \
    ADD_EVT(MICROWAVE_EXT_DIGIT_7_SIG) \
    ADD_EVT(MICROWAVE_EXT_DIGIT_8_SIG) \
    ADD_EVT(MICROWAVE_EXT_DIGIT_9_SIG) \
    ADD_EVT(MICROWAVE_EXT_DIGIT_SIG) \
    ADD_EVT(MICROWAVE_WIFI_CONN_REQ)

#undef ADD_EVT
#define ADD_EVT(e_) e_,

enum {
    MICROWAVE_INTERFACE_EVT_START = INTERFACE_EVT_START(MICROWAVE),
    MICROWAVE_INTERFACE_EVT
};

enum {
    MICROWAVE_REASON_UNSPEC = 0,
};

class MicrowaveStartReq : public Evt {
public:
    enum {
        TIMEOUT_MS = 200
    };
    MicrowaveStartReq(Hsmn to, Hsmn from, Sequence seq) :
        Evt(MICROWAVE_START_REQ, to, from, seq) {}
};

class MicrowaveStartCfm : public ErrorEvt {
public:
    MicrowaveStartCfm(Hsmn to, Hsmn from, Sequence seq,
                   Error error, Hsmn origin = HSM_UNDEF, Reason reason = 0) :
        ErrorEvt(MICROWAVE_START_CFM, to, from, seq, error, origin, reason) {}
};

class MicrowaveStopReq : public Evt {
public:
    enum {
        TIMEOUT_MS = 200
    };
    MicrowaveStopReq(Hsmn to, Hsmn from, Sequence seq) :
        Evt(MICROWAVE_STOP_REQ, to, from, seq) {}
};

class MicrowaveStopCfm : public ErrorEvt {
public:
    MicrowaveStopCfm(Hsmn to, Hsmn from, Sequence seq,
                   Error error, Hsmn origin = HSM_UNDEF, Reason reason = 0) :
        ErrorEvt(MICROWAVE_STOP_CFM, to, from, seq, error, origin, reason) {}
};

class MicrowaveExtStopSig : public Evt {
public:
    MicrowaveExtStopSig(Hsmn to, Hsmn from, Sequence seq = 0) :
        Evt(MICROWAVE_EXT_STOP_SIG, to, from, seq) {}
};

class MicrowaveExtStartSig : public Evt {
public:
    MicrowaveExtStartSig(Hsmn to, Hsmn from, Sequence seq = 0) :
        Evt(MICROWAVE_EXT_START_SIG, to, from, seq) {}
};

class MicrowaveExtCookTimeSig : public Evt {
public:
    MicrowaveExtCookTimeSig(Hsmn to, Hsmn from, Sequence seq = 0) :
        Evt(MICROWAVE_EXT_COOK_TIME_SIG, to, from, seq) {}
};

class MicrowaveExtPowerLevelSig : public Evt {
public:
    MicrowaveExtPowerLevelSig(Hsmn to, Hsmn from, Sequence seq = 0) :
        Evt(MICROWAVE_EXT_POWER_LEVEL_SIG, to, from, seq) {}
};

class MicrowaveExtKitchenTimerSig : public Evt {
public:
    MicrowaveExtKitchenTimerSig(Hsmn to, Hsmn from, Sequence seq = 0) :
        Evt(MICROWAVE_EXT_KITCHEN_TIMER_SIG, to, from, seq) {}
};

class MicrowaveExtDoorOpenSig : public Evt {
public:
    MicrowaveExtDoorOpenSig(Hsmn to, Hsmn from, Sequence seq = 0) :
        Evt(MICROWAVE_EXT_DOOR_OPEN_SIG, to, from, seq) {}
};

class MicrowaveExtDoorClosedSig : public Evt {
public:
    MicrowaveExtDoorClosedSig(Hsmn to, Hsmn from, Sequence seq = 0) :
        Evt(MICROWAVE_EXT_DOOR_CLOSED_SIG, to, from, seq) {}
};

class MicrowaveExtDigit0Sig : public Evt {
public:
    MicrowaveExtDigit0Sig(Hsmn to, Hsmn from, Sequence seq = 0) :
        Evt(MICROWAVE_EXT_DIGIT_0_SIG, to, from, seq) {}
};

class MicrowaveExtDigit1Sig : public Evt {
public:
    MicrowaveExtDigit1Sig(Hsmn to, Hsmn from, Sequence seq = 0) :
        Evt(MICROWAVE_EXT_DIGIT_1_SIG, to, from, seq) {}
};

class MicrowaveExtDigit2Sig : public Evt {
public:
    MicrowaveExtDigit2Sig(Hsmn to, Hsmn from, Sequence seq = 0) :
        Evt(MICROWAVE_EXT_DIGIT_2_SIG, to, from, seq) {}
};

class MicrowaveExtDigit3Sig : public Evt {
public:
    MicrowaveExtDigit3Sig(Hsmn to, Hsmn from, Sequence seq = 0) :
        Evt(MICROWAVE_EXT_DIGIT_3_SIG, to, from, seq) {}
};

class MicrowaveExtDigit4Sig : public Evt {
public:
    MicrowaveExtDigit4Sig(Hsmn to, Hsmn from, Sequence seq = 0) :
        Evt(MICROWAVE_EXT_DIGIT_4_SIG, to, from, seq) {}
};

class MicrowaveExtDigit5Sig : public Evt {
public:
    MicrowaveExtDigit5Sig(Hsmn to, Hsmn from, Sequence seq = 0) :
        Evt(MICROWAVE_EXT_DIGIT_5_SIG, to, from, seq) {}
};

class MicrowaveExtDigit6Sig : public Evt {
public:
    MicrowaveExtDigit6Sig(Hsmn to, Hsmn from, Sequence seq = 0) :
        Evt(MICROWAVE_EXT_DIGIT_6_SIG, to, from, seq) {}
};

class MicrowaveExtDigit7Sig : public Evt {
public:
    MicrowaveExtDigit7Sig(Hsmn to, Hsmn from, Sequence seq = 0) :
        Evt(MICROWAVE_EXT_DIGIT_7_SIG, to, from, seq) {}
};

class MicrowaveExtDigit8Sig : public Evt {
public:
    MicrowaveExtDigit8Sig(Hsmn to, Hsmn from, Sequence seq = 0) :
        Evt(MICROWAVE_EXT_DIGIT_8_SIG, to, from, seq) {}
};

class MicrowaveExtDigit9Sig : public Evt {
public:
    MicrowaveExtDigit9Sig(Hsmn to, Hsmn from, Sequence seq = 0) :
        Evt(MICROWAVE_EXT_DIGIT_9_SIG, to, from, seq) {}
};

class MicrowaveExtDigitSig : public Evt {
public:
    MicrowaveExtDigitiSig(Hsmn to, Hsmn from, Sequence seq, uint32_t digit) :
        Evt(MICROWAVE_EXT_DIGIT_SIG, to, from, seq), m_digit(digit)) {}
    uint32_t GetDigit() const { return m_digit; }
private:
    uint32_t m_digit;
}

class MicrowaveWifiConnReq : public Evt {
public:
    MicrowaveWifiConnReq(Hsmn to, Hsmn from, Sequence seq = 0) :
        Evt(MICROWAVE_WIFI_CONN_REQ, to, from, seq) {}
};

} // namespace APP

#endif // MICROWAVE_INTERFACE_H
