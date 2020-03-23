//===-- ZeroCharPtrCompareChecker.cpp -----------------------------------------*- C -*--//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Defines a checker for comparison between char pointer and null terminator.
//     If a char pointer variable is compared to null terminator, it may have been
//     written accidently and different behaviour is probably expected.
//
//===----------------------------------------------------------------------===//


#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"

using namespace clang;
using namespace ento;

namespace {

class ZeroCharPtrCompareChecker : public Checker<check::PreStmt<BinaryOperator>> {

  mutable std::unique_ptr<BuiltinBug> BT;
  
public:
    
    void checkPreStmt(const BinaryOperator *B, CheckerContext &C) const {
        if (B->getOpcode() != BO_EQ) {
            return;
        }

        Expr *lhs = B->getLHS()->IgnoreCasts();
        Expr *rhs = B->getRHS()->IgnoreCasts();

        QualType ltype = lhs->getType();

        if (!ltype->isPointerType() || !isa<CharacterLiteral>(rhs)) {
            return;
        }

        if (const ExplodedNode *N = C.generateNonFatalErrorNode()) {
                if (!BT) {
                    BT.reset(new BuiltinBug(this, "Zero char pointer comparison",
                            "a char pointer is compared to the null terminator"));
                }
            C.emitReport(std::make_unique<PathSensitiveBugReport>(*BT, BT->getDescription(), N));
        }
    }

};


}

void ento::registerZeroCharPtrCompareChecker(CheckerManager &mgr) {
    mgr.registerChecker<ZeroCharPtrCompareChecker>();
}

bool ento::shouldRegisterZeroCharPtrCompareChecker(const LangOptions &LO) {
  return true;
}
