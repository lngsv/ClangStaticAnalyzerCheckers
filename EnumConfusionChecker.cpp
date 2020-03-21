//===-- EnumConfusionChecker.cpp -----------------------------------------*- C++ -*--//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Defines a checker for enum comparison.
//     If a 'enum A' variable is compared to 'enum B' value, it may have been
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

class EnumConfusionChecker : public Checker<check::PreStmt<BinaryOperator>> {

  mutable std::unique_ptr<BuiltinBug> BT;
  
public:
    
    void checkPreStmt(const BinaryOperator *B, CheckerContext &C) const {
        if (B->getOpcode() != BO_EQ) {
            return;
        }

        Expr *lhs = B->getLHS();
        Expr *rhs = B->getRHS();

        QualType ltype = lhs->IgnoreCasts()->getType();
        QualType rtype = rhs->IgnoreCasts()->getType();

        if (!ltype->isEnumeralType() || !rtype->isEnumeralType()) {
            return;
        }

        const EnumDecl *lED = ltype->castAs<EnumType>()->getDecl();
        const EnumDecl *rED = rtype->castAs<EnumType>()->getDecl();

        bool match = lED->getNameAsString() == rED->getNameAsString();

        if (!match) {
            if (const ExplodedNode *N = C.generateNonFatalErrorNode()) {
                    if (!BT)
                        BT.reset(new BuiltinBug(this, "Enum confusion",
                                "Enum variable has a type different from "
                                "the type of the value it is compared to"));
                C.emitReport(std::make_unique<PathSensitiveBugReport>(*BT, BT->getDescription(), N));
            }
        }
    }

};


}

void ento::registerEnumConfusionChecker(CheckerManager &mgr) {
    mgr.registerChecker<EnumConfusionChecker>();
}

bool ento::shouldRegisterEnumConfusionChecker(const LangOptions &LO) {
  return true;
}
