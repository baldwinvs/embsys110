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

#include <string.h>
#include "fw_log.h"
#include "fw_assert.h"
#include "Console.h"
#include "MicrowaveCmd.h"
#include "MicrowaveInterface.h"

FW_DEFINE_THIS_FILE("CompositeActCmd.cpp")

namespace APP {

static CmdStatus Clock(Console &console, Evt const *e) {
    switch (e->sig) {
        case Console::CONSOLE_CMD: {
            console.PutStr("clock signal entered\n\r");
            Evt *evt = new MicrowaveExtClockSig(MICROWAVE, console.GetHsmn());
            Fw::Post(evt);
            break;
        }
    }
    return CMD_DONE;
}

static CmdStatus CookTime(Console &console, Evt const *e) {
    switch (e->sig) {
        case Console::CONSOLE_CMD: {
            console.PutStr("cook time signal entered\n\r");
            Evt *evt = new MicrowaveExtCookTimeSig(MICROWAVE, console.GetHsmn());
            Fw::Post(evt);
            break;
        }
    }
    return CMD_DONE;
}

static CmdStatus PowerLevel(Console &console, Evt const *e) {
    switch (e->sig) {
        case Console::CONSOLE_CMD: {
            console.PutStr("power level signal entered\n\r");
            Evt *evt = new MicrowaveExtPowerLevelSig(MICROWAVE, console.GetHsmn());
            Fw::Post(evt);
            break;
        }
    }
    return CMD_DONE;
}

static CmdStatus KitchenTimer(Console &console, Evt const *e) {
    switch (e->sig) {
        case Console::CONSOLE_CMD: {
            console.PutStr("kitchen timer signal entered\n\r");
            Evt *evt = new MicrowaveExtKitchenTimerSig(MICROWAVE, console.GetHsmn());
            Fw::Post(evt);
            break;
        }
    }
    return CMD_DONE;
}

static CmdStatus StateRequest(Console &console, Evt const *e) {
    switch (e->sig) {
        case Console::CONSOLE_CMD: {
            console.PutStr("current state requested\n\r");
            Evt *evt = new MicrowaveExtStateReqSig(MICROWAVE, console.GetHsmn());
            Fw::Post(evt);
            break;
        }
}
    return CMD_DONE;
}

static CmdStatus Stop(Console &console, Evt const *e) {
    switch (e->sig) {
        case Console::CONSOLE_CMD: {
            console.PutStr("stop signal entered\n\r");
            Evt *evt = new MicrowaveExtStopSig(MICROWAVE, console.GetHsmn());
            Fw::Post(evt);
            break;
        }
    }
    return CMD_DONE;
}

static CmdStatus Start(Console &console, Evt const *e) {
    switch (e->sig) {
        case Console::CONSOLE_CMD: {
            console.PutStr("start signal entered\n\r");
            Evt *evt = new MicrowaveExtStartSig(MICROWAVE, console.GetHsmn());
            Fw::Post(evt);
            break;
        }
    }
    return CMD_DONE;
}

static CmdStatus Digit(Console &console, Evt const *e) {
    Hsm &hsm = console.GetHsm();
    switch (e->sig) {
        case Console::CONSOLE_CMD: {
            Console::ConsoleCmd const &ind = static_cast<Console::ConsoleCmd const &>(*e);
            if (ind.Argc() < 2) {
                console.PutStr("microwave digit <number 0-9>\n\r");
                return CMD_DONE;
            }
            uint32_t digit = STRING_TO_NUM(ind.Argv(1), 0);
            char buf[32] = {0};
            sprintf(buf, "digit %lu entered\n\r", digit);
            console.PutStr(buf);
            Evt *evt = new MicrowaveExtDigitSig(MICROWAVE, hsm.GetHsmn(), hsm.GenSeq(), digit);
            Fw::Post(evt);
            break;
        }
    }
    return CMD_DONE;
}

static CmdStatus List(Console &console, Evt const *e);
static CmdHandler const cmdHandler[] = {
    { "clock",         Clock,        "Clock Signal",         0 },
    { "cook_time",     CookTime,     "Cook Time Signal",     0 },
    { "power_level",   PowerLevel,   "Power Level Signal",   0 },
    { "kitchen_timer", KitchenTimer, "Kitchen Timer Signal", 0 },
    { "state_req",     StateRequest, "State Request",        0 },
    { "stop",          Stop,         "Stop Signal",          0 },
    { "start",         Start,        "Start Signal",         0 },
    { "digit",         Digit,        "Digit Signal",         0 },
    { "?",             List,         "List commands",        0 }
};

static CmdStatus List(Console &console, Evt const *e) {
    return console.ListCmd(e, cmdHandler, ARRAY_COUNT(cmdHandler));
}

CmdStatus MicrowaveCmd(Console &console, Evt const *e) {
    return console.HandleCmd(e, cmdHandler, ARRAY_COUNT(cmdHandler));
}

}
