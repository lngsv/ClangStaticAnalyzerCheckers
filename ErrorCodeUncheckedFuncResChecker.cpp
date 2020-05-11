//===-- ErrorCodeUncheckedFuncResChecker.cpp ----------------------------*- C++ -*--//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Defines a checker for unchecked function result.
//     If a function return value of std::error_code type is not checked, 
//     errors may not be noticed.
//
//===----------------------------------------------------------------------===//


#include "clang/AST/StmtVisitor.h"
#include "clang/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/AnalysisManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugReporter.h"
#include "llvm/Support/raw_ostream.h"
#include <iostream>
#include <string>

using namespace clang;
using namespace ento;

namespace {

class WalkAST : public StmtVisitor<WalkAST> {
    BugReporter &BR;
    AnalysisDeclContext* AC;
    const CheckerBase* Checker;

public:
    WalkAST(BugReporter &br, AnalysisDeclContext* ac, const CheckerBase* c) : BR(br), AC(ac), Checker(c) {}

    void VisitCompoundStmt (CompoundStmt *S) {
        for (Stmt *Child : S->children()) {
            if (Child) {
                if (CallExpr *CE = dyn_cast<CallExpr>(Child)) {
                    QualType ret_type = CE->getCallReturnType(AC->getASTContext());
                    bool isErrorCode = ret_type.getAsString() == "std::error_code";
                    if (isErrorCode) {
                        checkUncheckedReturnValue(CE);
                    }
                }
                Visit(Child);
            }
        }
    }

    void VisitStmt(Stmt *S) { 
        VisitChildren(S); 
    }

    void VisitChildren(Stmt *S) {
        for (Stmt *Child : S->children()) {
            if (Child) {
                Visit(Child);
            }
        }
    }

    void checkUncheckedReturnValue(CallExpr *CE) {
        const FunctionDecl *FD = CE->getDirectCallee();
        if (!FD) {
            return;
        }

        SmallString<256> buf;
        llvm::raw_svector_ostream os(buf);
        os << "The return value from the call to '" << *FD
            << "' is not checked. As its type is std::error_code, "
            << "you may have missed an error";

        PathDiagnosticLocation CELoc =
          PathDiagnosticLocation::createBegin(CE, BR.getSourceManager(), AC);
        BR.EmitBasicReport(AC->getDecl(), Checker, "Unchecked function result of std::error_code type",
                         "ErrorCodeUncheckedFuncRes", os.str(), CELoc,
                         CE->getCallee()->getSourceRange());
    }
};


class ErrorCodeUncheckedFuncResChecker : public Checker<check::ASTCodeBody> {
public:

    void checkASTCodeBody(const Decl *D, AnalysisManager& mgr, BugReporter &BR) const {
        WalkAST walker(BR, mgr.getAnalysisDeclContext(D), this);
        walker.Visit(D->getBody());
    }

};


}

void ento::registerErrorCodeUncheckedFuncResChecker(CheckerManager &mgr) {
    mgr.registerChecker<ErrorCodeUncheckedFuncResChecker>();
}

bool ento::shouldRegisterErrorCodeUncheckedFuncResChecker(const LangOptions &LO) {
  return true;
}
