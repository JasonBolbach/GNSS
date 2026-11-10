#pragma once
// UBX class 0x06: receiver configuration (legacy messages, u-blox 6 through M8; newer
// generations keep these but prefer CFG-VALSET/VALGET). One header per message.
// Polls: ToPoll(). Writes: ToSet() / ToSet*() on the settings messages, ToSaveAll() / ToReset*() on the
// CFG-CFG and CFG-RST commands (which have no poll form). Every builder returns the frame size and
// accepts a nullptr buffer to just report it.
#include "UBX/UBX_Common.h"
#include "UBX/CFG/UBX_CFG_PRT.h"
#include "UBX/CFG/UBX_CFG_MSG.h"
#include "UBX/CFG/UBX_CFG_RATE.h"
#include "UBX/CFG/UBX_CFG_NAV5.h"
#include "UBX/CFG/UBX_CFG_GNSS.h"
#include "UBX/CFG/UBX_CFG_CFG.h"
#include "UBX/CFG/UBX_CFG_RST.h"
