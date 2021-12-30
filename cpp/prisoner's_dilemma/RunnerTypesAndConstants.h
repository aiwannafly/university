#ifndef PRISONER_DILEMMA_RUNNERTYPESANDCONSTANTS_H
#define PRISONER_DILEMMA_RUNNERTYPESANDCONSTANTS_H

#include "StrategyTypesAndConstants.h"

using TStatus =  enum TStatus {
    OK, WRONG_MODE, WRONG_STEPS,
    MATRIX_FILE_NOT_OPENED, WRONG_MATRIX, NOT_ENOUGH_STRATEGIES,
    WRONG_STRATEGY_NAME, TOO_MANY_STRATEGIES, OUTPUT_STREAM_FAILURE
};

using TMode = enum TMode {
    DETAILED, FAST, TOURNAMENT
};

#endif //PRISONER_DILEMMA_RUNNERTYPESANDCONSTANTS_H