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
            //std::cout << "!FD" << std::endl;
            return;
        }

        const IdentifierInfo *II = FD->getIdentifier();
        if (!II) {
            //std::cout << "!II" << std::endl;
            return;
        }

        const StringRef funcName = II->getName();
        //std::cout << "FUNC NAME " << funcName.str() << std::endl;
        if (!isMemFunc(funcName)) {
            //std::cout << "NOT MEM FUNC" << std::endl;
            return;
        }

        for (unsigned i = 0; i < FD->getNumParams(); ++i) {
            const Expr* E = CE->getArg(i)->IgnoreCasts();
            const QualType QT = E->getType();
            //std::cout << "isPDS(" << QT.getAsString() << ")" << std::endl;
            
            //std::cout << "POINTER IS POD: " << QT.isPODType(C.getASTContext()) << std::endl;

            if (!QT->isPointerType()) {
                //std::cout << "NOT POINTER" << std::endl;
                return;
            }

            const QualType Pointee = QT->getPointeeType();
            //std::cout << "POINTEE: " << Pointee.getAsString() << std::endl;
          
            //const ASTContext& AC = C.getASTContext();
            //std::cout << "C11: " << AC.getLangOpts().CPlusPlus11 << std::endl; 

            if (QT.isPODType(C.getASTContext())) {
                //std::cout << "QT IS POD" << std::endl;
            } else {
                //std::cout << "QT !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
                if (const ExplodedNode *N = C.generateNonFatalErrorNode()) {
                    if (!BT) {
                        BT.reset(new BuiltinBug(this, "Enum confusion",
                                "compared expressions have different enum types"));
                    }
                C.emitReport(std::make_unique<PathSensitiveBugReport>(*BT, BT->getDescription(), N));
            }
            }

            /*
            CXXRecordDecl* CXXRD = QT->getAsCXXRecordDecl();
            if (!CXXRD) {
                std::cout << "NOT CXXRD" << std::endl;
                return;
            }
            
            if (!CXXRD->isPOD()) {
                std::cout << "REPORT!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1" << i << std::endl;
            } else {
                std::cout << "POD " << i << std::endl;
            }
            */
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
