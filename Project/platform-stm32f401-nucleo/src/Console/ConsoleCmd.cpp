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
#include "fw_macro.h"
#include "fw_timer.h"
#include "Console.h"
#include "ConsoleInterface.h"
#include "UartOutInterface.h"
#include "ConsoleCmd.h"
#include "LogCmd.h"
#include "SystemCmd.h"
#include "WifiStCmd.h"
#include "GpioOutCmd.h"
#include "MicrowaveCmd.h"
#include "MagnetronCmd.h"
#include <memory>

FW_DEFINE_THIS_FILE("ConsoleCmd.cpp")

namespace APP {

static CmdStatus Assert(Console &console, Evt const *e) {
    switch (e->sig) {
        case Console::CONSOLE_CMD: {
            Console::ConsoleCmd const &ind = static_cast<Console::ConsoleCmd const &>(*e);
            if ((ind.Argc() > 1) && STRING_EQUAL(ind.Argv(1), "1234")) {
                FW_ASSERT(0);
            } else {
                console.PutStr("Invalid passcode\n\r");
            }
            break;
        }
    }
    return CMD_DONE;
}

static CmdStatus Hsm(Console &console, Evt const *e) {
    uint32_t &hsmn = console.Var(0);
    switch (e->sig) {
        case Console::CONSOLE_CMD: {
            console.Print("HSMs in system:\n\r");
            console.Print("===============\n\r");
            hsmn = 0;
            break;
        }
        case UART_OUT_EMPTY_IND: {
            for (; hsmn < HSM_COUNT; hsmn++) {
                bool result = console.PrintItem(hsmn, 28, 4, "%s(%lu)", Log::GetHsmName(hsmn), hsmn);
                if (!result) {
                    return CMD_CONTINUE;
                }
            }
            console.PutStr("\n\r\n\r");
            return CMD_DONE;
        }
    }
    return CMD_CONTINUE;
}

static CmdStatus State(Console &console, Evt const *e) {
    uint32_t &hsmn = console.Var(0);
    switch (e->sig) {
        case Console::CONSOLE_CMD: {
            console.Print("HSM states:\n\r");
            console.Print("===========\n\r");
            hsmn = 0;
            break;
        }
        case UART_OUT_EMPTY_IND: {
            for (; hsmn < HSM_COUNT; hsmn++) {
                bool result = console.PrintItem(hsmn, 56, 2, "%s(%lu) - %s", Log::GetHsmName(hsmn), hsmn, Log::GetState(hsmn));
                if (!result) {
                    return CMD_CONTINUE;
                }
            }
            console.PutStr("\n\r\n\r");
            return CMD_DONE;
        }
    }
    return CMD_CONTINUE;
}

static CmdStatus List(Console &console, Evt const *e);
static CmdHandler const cmdHandler[] = {
    { "assert",     Assert,       "Trigger assert",      0 },
    { "hsm",        Hsm,          "List all HSMs",       0 },
    { "state",      State,        "List HSM states",     0 },
    { "log",        LogCmd,       "Log control",         0 },
    { "sys",        SystemCmd,    "System",              0 },
    { "wifi",       WifiStCmd,    "Wifi(stm32) control", 0 },
    { "gpio",       GpioOutCmd,   "GPIO output control", 0 },
    { "mw",         MicrowaveCmd, "Microwave",           0 },
    { "magnetron",  MagnetronCmd, "Magnetron",           0 },
    { "?",          List,         "List commands",       0 },
};

static CmdStatus List(Console &console, Evt const *e) {
    return console.ListCmd(e, cmdHandler, ARRAY_COUNT(cmdHandler));
}

CmdStatus ConsoleCmd(Console &console, Evt const *e) {
    return console.HandleCmd(e, cmdHandler, ARRAY_COUNT(cmdHandler), true);
}

}
