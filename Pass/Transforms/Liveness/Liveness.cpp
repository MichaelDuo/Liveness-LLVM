#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/BasicBlock.h"
#include <string>
#include <vector>
#include <set>
#include <unordered_map>
#include <algorithm>

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
      map<string, set<Value*>> *bbInfo = &table[&bb];
      set<Value*> *VarKill = &(*bbInfo)["VarKill"];
      set<Value*> *UEVar = &(*bbInfo)["UEVar"];

      if(inst.getOpcode()==31){
        Value* v = inst.getOperand(0);
        if(VarKill->find(v) == VarKill->end()){
          UEVar->insert(v);
        }
      }

      if(inst.getOpcode()==32){
        VarKill->insert(inst.getOperand(1));
      }
    }

  }

  void findLiveout(map<BasicBlock*, map<string, set<Value*>>> &table, Function &F){
    bool flag = true;
    while(flag){
      flag = false;
      for(auto& bb: F){

        set<Value*> dest, dest1;
        
        map<string, set<Value*>> *bbInfo = &table[&bb];
        set<Value*> prev = (*bbInfo)["LiveOut"];
        set<Value*> *VarKill = &(*bbInfo)["VarKill"];
        set<Value*> *UEVar = &(*bbInfo)["UEVar"];
        set<Value*> *LiveOut = &(*bbInfo)["LiveOut"];

        for(BasicBlock* Succ: successors(&bb)){
          map<string, set<Value*>> *SuccBbInfo = &table[Succ];
          set<Value*> *SuccVarKill = &(*SuccBbInfo)["VarKill"];
          set<Value*> *SuccUEVar = &(*SuccBbInfo)["UEVar"];
          set<Value*> *SuccLiveOut = &(*SuccBbInfo)["LiveOut"];

          set_difference(SuccLiveOut->begin(), SuccLiveOut->end(), SuccVarKill->begin(), SuccVarKill->end(), inserter(dest, dest.begin()));
          set_union(dest.begin(), dest.end(), SuccUEVar->begin(), SuccUEVar->end(), inserter(dest1, dest1.begin()));
          set_union(dest1.begin(), dest1.end(), LiveOut->begin(), LiveOut->end(), inserter(*LiveOut, LiveOut->begin()));
          dest.clear(); dest1.clear();
        }
        if(!std::equal(prev.begin(), prev.end(),LiveOut->begin(), LiveOut->end())){
          flag = true;
        }
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
    }

    findLiveout(table, F);

    for(auto& basic_block: F){
      errs() << "----- ";
      basic_block.printAsOperand(errs(), false);
      errs() << " -----\n";
      map<string, set<Value*>> bbInfo = table[&basic_block];
      set<Value*> VarKill = bbInfo["VarKill"];
      set<Value*> UEVar = bbInfo["UEVar"];
      set<Value*> LiveOut = bbInfo["LiveOut"];

      errs() << "UEVar: ";
      for(auto it = UEVar.begin();it != UEVar.end();++ it){
        errs() << (**it).getName() << " ";
      }
      errs() << "\n";

      errs() << "VarKill: ";
      for(auto it = VarKill.begin();it != VarKill.end();++ it){
        errs() << (**it).getName() << " ";
      }
      errs() << "\n";


      errs() << "LiveOut: ";
      for(auto it = LiveOut.begin();it != LiveOut.end();++ it){
        errs() << (**it).getName() << " ";
      }
      errs() << "\n";

    }
    
    return false;
  }
}; // end of struct ValueNumbering
}  // end of anonymous namespace

char ValueNumbering::ID = 0;
static RegisterPass<ValueNumbering> X("ValueNumbering", "ValueNumbering Pass",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);