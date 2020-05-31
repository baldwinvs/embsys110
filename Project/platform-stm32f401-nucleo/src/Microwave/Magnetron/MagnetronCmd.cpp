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
#include "ConsoleInterface.h"
#include "MagnetronCmd.h"
#include "MagnetronInterface.h"

FW_DEFINE_THIS_FILE("MagnetronCmd.cpp")

namespace APP {

static uint32_t powerLevelStor[1];
static bool pipeFilled{false};
MagnetronPipe pipe(powerLevelStor, 1);

static CmdStatus PowerLevel(Console &console, Evt const e) {
    switch (e->sig) {
        case Console::CONSOLE_CMD: {
            Console::ConsoleCmd const &ind = static_cast<Console::ConsoleCmd const &>(*e);
            if (ind.Argc() < 2) {
                console.PutStr("magnetron power <power level>\n\r");
                return CMD_DONE;
            }
            uint32_t powerLevel = STRING_TO_NUM(ind.Argv(1), 0);
            uint32_t count {pipe.Write(&powerLevel, 1)};
            if(count == 0) {
                console.PutStr("Pipe::Write unsuccessful\n\r");
                return CMD_DONE;
            }
            pipeFilled = true;
        }
    }
    return CMD_DONE;
}

static CmdStatus On(Console & console, Evt const e) {
    switch (e->sig) {
        case Console::CONSOLE_CMD: {
            if(pipeFilled) {
                console.PutStr("Magnetron power-on request\n\r");
                Evt *evt = new MagnetronOnReq(MAGNETRON, console.GetHsmn());
                Fw::Post(evt);
                pipeFilled = false; //assuming that it was read correctly
            }
            else {
                console.PutStr("No record of power level being set\n\r");
            }
            break;
        }
    }
    return CMD_DONE;
}

static CmdStatus Off(Console & console, Evt const e) {
    switch (e->sig) {
        case Console::CONSOLE_CMD: {
            console.PutStr("Magnetron power-off request\n\r");
            Evt *evt = new MagnetronOffReq(MAGNETRON, console.GetHsmn());
            Fw::Post(evt);
            break;
        }
    }
    return CMD_DONE;
}

static CmdStatus List(Console &console, Evt const *e);
static CmdHandler const cmdHandler[] = {
    { "?",          List,       "List commands", 0 },
    { "power",      PowerLevel, "Set Power Level", 0 },
    { "on",         On,         "Turn on", 0 },
    { "off",        Off,        "Turn off", 0 },
};

static CmdStatus List(Console &console, Evt const *e) {
    return console.ListCmd(e, cmdHandler, ARRAY_COUNT(cmdHandler));
}

CmdStatus MagnetronCmd(Console &console, Evt const *e) {
    return console.HandleCmd(e, cmdHandler, ARRAY_COUNT(cmdHandler));
}

}
