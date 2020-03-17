//===-- ZeroCharPtrAssignChecker.cpp -----------------------------------------*- C++ -*--//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Defines a checker for char pointer assignment.
//     If a '\0' is assigned to a variable or type 'char *', it may have been
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

class ZeroCharPtrAssignChecker : public Checker<check::PreStmt<BinaryOperator>> {

  mutable std::unique_ptr<BuiltinBug> BT;
  
public:
    
    void checkPreStmt(const BinaryOperator *B, CheckerContext &C) const {
        if (B->getOpcode() != BO_Assign) {
            return;
        }

        Expr *lhs = B->getLHS();
        Expr *rhs = B->getRHS();

        QualType ltype = lhs->getType();
        QualType rtype = rhs->getType();

        if (!ltype.getTypePtr()->isPointerType() || !rtype.getTypePtr()->isCharType()) {
            return;
        }

        //const EnumDecl *lED = ltype->castAs<EnumType>()->getDecl();
        //const EnumDecl *rED = rtype->castAs<EnumType>()->getDecl();

        //bool match = lED->getNameAsString() == rED->getNameAsString();

        //if (!match) {
            if (const ExplodedNode *N = C.generateNonFatalErrorNode()) {
                    if (!BT)
                        BT.reset(new BuiltinBug(this, "Enum confusion",
                                "Zero is assigned to char pointer variable"));
                C.emitReport(std::make_unique<PathSensitiveBugReport>(*BT, BT->getDescription(), N));
            }
        //}
    }

};


}

void ento::registerZeroCharPtrAssignChecker(CheckerManager &mgr) {
    mgr.registerChecker<ZeroCharPtrAssignChecker>();
}

bool ento::shouldRegisterZeroCharPtrAssignChecker(const LangOptions &LO) {
  return true;
}
