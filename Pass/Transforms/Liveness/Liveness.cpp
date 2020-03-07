#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Instruction.h"
#include <string>
#include <map>
#include <set>

using namespace llvm;
using namespace std;

#define DEBUG_TYPE "ValueNumbering"

using namespace llvm;

namespace {
struct ValueNumbering : public FunctionPass {
  string func_name = "test";
  static char ID;
  ValueNumbering() : FunctionPass(ID) {}

  void findVarKillAndUEVar(map<BasicBlock*, map<string, set<Value*>>> &table, BasicBlock &bb){
    for(auto& inst: bb){
      if(inst.isBinaryOp()){
        map<string, set<Value*>> *bbInfo = &table[&bb];
        set<Value*> *VarKill = &(*bbInfo)["VarKill"];
        set<Value*> *UEVar = &(*bbInfo)["UEVar"];

        Value* v1 = inst.getOperand(0);
        Value* v2 = inst.getOperand(1);

        StringRef varName = v1->getName();

        if(VarKill->find(v1) == VarKill->end()){
          UEVar->insert(v1);
        }
        if(VarKill->find(v2) == VarKill->end()){
          UEVar->insert(v2);
        }

        VarKill->insert(&inst);
      }
    }
  }

  bool runOnFunction(Function &F) override {
    errs() << "Liveness: ";
    errs() << F.getName() << "\n";
    if (F.getName() != func_name) return false;

    map<BasicBlock*, map<string, set<Value*>>> table = {};

    for(auto& basic_block : F){
      map<string, set<Value*>> blockInfo;
      set<Value*> UEVar;
      set<Value*> VarKill;
      set<Value*> LiveOut;
      blockInfo.insert(make_pair("UEVar", UEVar));
      blockInfo.insert(make_pair("VarKill", VarKill));
      blockInfo.insert(make_pair("VarKill", LiveOut));
      table.insert(make_pair(&basic_block, blockInfo));
    }

    for(auto& basic_block: F){
      findVarKillAndUEVar(table, basic_block);

      map<string, set<Value*>> bbInfo = table[&basic_block];
      set<Value*> VarKill = bbInfo["VarKill"];
      set<Value*> UEVar = bbInfo["UEVar"];
    }

    for(auto& basic_block: F){
      errs() << "Basic Block: \n";
      map<string, set<Value*>> bbInfo = table[&basic_block];
      set<Value*> VarKill = bbInfo["VarKill"];
      set<Value*> UEVar = bbInfo["UEVar"];
      for(auto it = UEVar.begin();it != UEVar.end();++ it){
        errs() << **it << "\n";
      }
    }



    
    return false;
  }
}; // end of struct ValueNumbering
}  // end of anonymous namespace

char ValueNumbering::ID = 0;
static RegisterPass<ValueNumbering> X("ValueNumbering", "ValueNumbering Pass",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);