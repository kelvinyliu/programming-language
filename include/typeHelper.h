#pragma once
#include "../include/tokeniser.h"
#include "../include/evaluator.h"
#include <stdbool.h>

bool doesDataTypeMatchesData(enum ValueType valType, enum TokenType nodeType) {
    switch (nodeType) {
    case TEXT_TYPE:
        return valType == VALUE_TEXT;
    case NUMBER_TYPE:
        return valType == VALUE_NUMBER;
    case BOOLEAN_TYPE:
        return valType == VALUE_BOOL;
    default:
        return false;
    }
}