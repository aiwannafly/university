#include "Strategy.h"

TChoiceMatrix Strategy::getHistory() const{
    return history_;
}

TScoreMap Strategy::getScoreMap() const {
    return scoreMap_;
}

size_t Strategy::getOrderNumber() const{
    return orderNumber_;
}

TConfigs Strategy::getConfigs() const {
    return configs_;
}
