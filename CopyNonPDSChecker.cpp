//===-- CopyNonPDSChecker.cpp ----------------------------*- C++ -*--//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Defines a checker for incorrect using of memory manipulating functions.
//     If a function like memcpy is used for non-POD structures, 
//     it will work unexpectedly.
//
//===----------------------------------------------------------------------===//


#include "clang/AST/StmtVisitor.h"
#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/AnalysisManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "llvm/Support/raw_ostream.h"
#include <iostream>
#include <string>

using namespace clang;
using namespace ento;

namespace {

class CopyNonPDSChecker : public Checker<check::PreStmt<CallExpr>> {

    mutable std::unique_ptr<BuiltinBug> BT;

    bool isMemFunc(const StringRef& funcName) const {
        if (funcName == "memset" ||
                funcName == "memcpy" ||
                funcName == "memchr" ||
                funcName == "memcmp" ||
                funcName == "memmove") {
            return true;
        }
        return false;
    }

public:

    void checkPreStmt(const CallExpr *CE, CheckerContext& C) const {
        const FunctionDecl* FD = CE->getDirectCallee();
        if (!FD) {
            return;
        }

        const IdentifierInfo *II = FD->getIdentifier();
        if (!II) {
            return;
        }

        const StringRef funcName = II->getName();
        if (!isMemFunc(funcName)) {
            return;
        }

        for (unsigned i = 0; i < FD->getNumParams(); ++i) {
            const Expr* E = CE->getArg(i)->IgnoreCasts();
            const CXXRecordDecl* CXXRD = E->getBestDynamicClassType();

            if (!CXXRD) {
                return;
            }

            if (!CXXRD->isPOD()) {
                if (const ExplodedNode *N = C.generateNonFatalErrorNode()) {
                    if (!BT) {
                        BT.reset(new BuiltinBug(this, "Memory manipulation function ",
                                "is used on non-POD structure"));
                    }
                    C.emitReport(std::make_unique<PathSensitiveBugReport>(*BT, BT->getDescription(), N));
                }
            }
        }
    }
};

}

void ento::registerCopyNonPDSChecker(CheckerManager &mgr) {
    mgr.registerChecker<CopyNonPDSChecker>();
}

bool ento::shouldRegisterCopyNonPDSChecker(const LangOptions &LO) {
  return true;
}
