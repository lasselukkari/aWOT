#pragma once

#ifndef typeof
#define typeof __typeof__
#endif

#include "Compare.h"

#define arduinoCITestBehaviorExp(die, desc, pass) \
  do                                     \
  {                                      \
    if (!assertion(__FILE__, __LINE__,   \
                   desc, pass))          \
    {                                    \
      if (die) return;                   \
    }                                    \
  } while (0)

#define arduinoCITestBehaviorOp(die, desc, rel1, arg1, op, op_name, rel2, arg2) \
  do                                                                   \
  {                                                                    \
    if (!assertion<typeof(arg1), typeof(arg2)>(__FILE__, __LINE__,     \
                                               desc,                   \
                                               rel1, #arg1, (arg1),    \
                                               op_name, op,            \
                                               rel2, #arg2, (arg2)))   \
    {                                                                  \
      if (die) return;                                                 \
    }                                                                  \
  } while (0)



// helper define for the operators below
#define arduinoCIAssertOp(desc, rel1, arg1, op, op_name, rel2, arg2) \
  arduinoCITestBehaviorOp(false, "assert" desc, rel1, arg1, op, op_name, rel2, arg2)

#define arduinoCIAssureOp(desc, rel1, arg1, op, op_name, rel2, arg2) \
  arduinoCITestBehaviorOp(true, "assure" desc, rel1, arg1, op, op_name, rel2, arg2)


/** macro generates optional output and calls fail() but does not return if false. */
#define assertTrue(arg)                           arduinoCITestBehaviorExp(false, "True " #arg, (arg))
#define assertFalse(arg)                          arduinoCITestBehaviorExp(false, "False " #arg, !(arg))
#define assertNull(arg)                           arduinoCITestBehaviorExp(false, "Null " #arg, ((void*)NULL == (void*)(arg)))
#define assertNotNull(arg)                        arduinoCITestBehaviorExp(false, "NotNull " #arg, ((void*)NULL != (void*)(arg)))
#define assertEqual(arg1,arg2)                    arduinoCIAssertOp("Equal","expected",arg1,compareEqual,"==","actual",arg2)
#define assertNotEqual(arg1,arg2)                 arduinoCIAssertOp("NotEqual","unwanted",arg1,compareNotEqual,"!=","actual",arg2)
#define assertComparativeEquivalent(arg1,arg2)    arduinoCIAssertOp("ComparativeEquivalent","expected",arg1,compareEquivalent,"!<>","actual",arg2)
#define assertComparativeNotEquivalent(arg1,arg2) arduinoCIAssertOp("ComparativeNotEquivalent","unwanted",arg1,compareNotEquivalent,"<>","actual",arg2)
#define assertLess(arg1,arg2)                     arduinoCIAssertOp("Less","lowerBound",arg1,compareLess,"<","actual",arg2)
#define assertMore(arg1,arg2)                     arduinoCIAssertOp("More","upperBound",arg1,compareMore,">","actual",arg2)
#define assertLessOrEqual(arg1,arg2)              arduinoCIAssertOp("LessOrEqual","lowerBound",arg1,compareLessOrEqual,"<=","actual",arg2)
#define assertMoreOrEqual(arg1,arg2)              arduinoCIAssertOp("MoreOrEqual","upperBound",arg1,compareMoreOrEqual,">=","actual",arg2)

#define assertEqualFloat(arg1, arg2, arg3)    arduinoCIAssertOp("EqualFloat", "epsilon", arg3, compareMoreOrEqual, ">=", "actualDifference", fabs(arg1 - arg2))
#define assertNotEqualFloat(arg1, arg2, arg3) arduinoCIAssertOp("NotEqualFloat", "epsilon", arg3, compareLessOrEqual, "<=", "insufficientDifference", fabs(arg1 - arg2))
#define assertInfinity(arg)                   arduinoCITestBehaviorExp(false, "Infinity " #arg, isinf(arg))
#define assertNotInfinity(arg)                arduinoCITestBehaviorExp(false, "NotInfinity " #arg, !isinf(arg))
#define assertNAN(arg)                        arduinoCITestBehaviorExp(false, "NAN " #arg, isnan(arg))
#define assertNotNAN(arg)                     arduinoCITestBehaviorExp(false, "NotNAN " #arg, !isnan(arg))


/** macro generates optional output and calls fail() followed by a return if false. */
#define assureTrue(arg)                           arduinoCITestBehaviorExp(true, "True " #arg, (arg))
#define assureFalse(arg)                          arduinoCITestBehaviorExp(true, "False " #arg, !(arg))
#define assureNull(arg)                           arduinoCITestBehaviorExp(true, "Null " #arg, ((void*)NULL == (void*)(arg)))
#define assureNotNull(arg)                        arduinoCITestBehaviorExp(true, "NotNull " #arg, ((void*)NULL != (void*)(arg)))
#define assureEqual(arg1,arg2)                    arduinoCIAssureOp("Equal","expected",arg1,compareEqual,"==","actual",arg2)
#define assureNotEqual(arg1,arg2)                 arduinoCIAssureOp("NotEqual","unwanted",arg1,compareNotEqual,"!=","actual",arg2)
#define assureComparativeEquivalent(arg1,arg2)    arduinoCIAssureOp("ComparativeEquivalent","expected",arg1,compareEquivalent,"!<>","actual",arg2)
#define assureComparativeNotEquivalent(arg1,arg2) arduinoCIAssureOp("ComparativeNotEquivalent","unwanted",arg1,compareNotEquivalent,"<>","actual",arg2)
#define assureLess(arg1,arg2)                     arduinoCIAssureOp("Less","lowerBound",arg1,compareLess,"<","actual",arg2)
#define assureMore(arg1,arg2)                     arduinoCIAssureOp("More","upperBound",arg1,compareMore,">","actual",arg2)
#define assureLessOrEqual(arg1,arg2)              arduinoCIAssureOp("LessOrEqual","lowerBound",arg1,compareLessOrEqual,"<=","actual",arg2)
#define assureMoreOrEqual(arg1,arg2)              arduinoCIAssureOp("MoreOrEqual","upperBound",arg1,compareMoreOrEqual,">=","actual",arg2)

#define assureEqualFloat(arg1, arg2, arg3)    arduinoCIAssureOp("EqualFloat", "epsilon", arg3, compareMoreOrEqual, ">=", "actualDifference", fabs(arg1 - arg2))
#define assureNotEqualFloat(arg1, arg2, arg3) arduinoCIAssureOp("NotEqualFloat", "epsilon", arg3, compareLessOrEqual, "<=", "insufficientDifference", fabs(arg1 - arg2))
#define assureInfinity(arg)                   arduinoCITestBehaviorExp(true, "Infinity " #arg, isinf(arg))
#define assureNotInfinity(arg)                arduinoCITestBehaviorExp(true, "NotInfinity " #arg, !isinf(arg))
#define assureNAN(arg)                        arduinoCITestBehaviorExp(true, "NAN " #arg, isnan(arg))
#define assureNotNAN(arg)                     arduinoCITestBehaviorExp(true, "NotNAN " #arg, !isnan(arg))
