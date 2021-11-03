#ifndef PRISONER_DILEMMA_ALWAYSCOOPSTRATEGY_H
#define PRISONER_DILEMMA_ALWAYSCOOPSTRATEGY_H

#include "Strategy.h"

constexpr char alwaysCoopID[] = "always_coop";

class AlwaysCoopStrategy : public Strategy {
public:
    AlwaysCoopStrategy(size_t orderNumber, TChoiceMatrix &history,
    TScoreMap &scoreMap) : Strategy(orderNumber, history,
            scoreMap) {}

    TChoice getChoice() override;
};

#endif //PRISONER_DILEMMA_ALWAYSCOOPSTRATEGY_H