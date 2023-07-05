
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "/opt/homebrew/opt/llvm/include/llvm/ADT/APFloat.h"
#include "/opt/homebrew/opt/llvm/include/llvm/ADT/STLExtras.h"
#include "/opt/homebrew/opt/llvm/include/llvm/IR/BasicBlock.h"
#include "/opt/homebrew/opt/llvm/include/llvm/IR/Constants.h"
#include "/opt/homebrew/opt/llvm/include/llvm/IR/Function.h"
#include "/opt/homebrew/opt/llvm/include/llvm/IR/IRBuilder.h"
#include "/opt/homebrew/opt/llvm/include/llvm/IR/LLVMContext.h"
#include "/opt/homebrew/opt/llvm/include/llvm/IR/Module.h"
#include "/opt/homebrew/opt/llvm/include/llvm/IR/Type.h"
#include "/opt/homebrew/opt/llvm/include/llvm/IR/Verifier.h"
#include "/opt/homebrew/opt/llvm/include/llvm/Support/raw_ostream.h"
#include "/opt/homebrew/opt/llvm/include/llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/TargetParser/Host.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetSelect.h"
#include"lexer.cc"
#include "codegen.cc"
using namespace llvm;
LexicalAnalyzer* lexer;


//===----------------------------------------------------------------------===//
// Parser
//===----------------------------------------------------------------------===//





std::unique_ptr<ExprAST> LogError(const char *Str) {
 lexer->Printtoken();
  fprintf(stderr, "\vError: %s\n", Str);
  return nullptr;
}
std::unique_ptr<PrototypeAST> LogErrorP(const char *Str) {
  LogError(Str);
  return nullptr;
}


static int GetTokPrecedence(){
    switch(lexer->CurTok){
        case '<':
        case '>': return 10;
        case '-':
        case '+': return 20;
        case '*':
        case '/': return 40;
        default: return -1;
    }
}

/// prototype
///   ::= id '(' id* ')'
static std::unique_ptr<PrototypeAST> ParsePrototype() {
  if (lexer->CurTok != tok_identifier)
    return LogErrorP("Expected function name in prototype");

  std::string FnName = lexer->IdentifierStr;
  lexer->NextToken();

  if (lexer->CurTok != '(')
    return LogErrorP("Expected '(' in prototype");

  std::vector<std::string> ArgNames;
  while (lexer->NextToken() == tok_identifier)
    ArgNames.push_back(lexer->IdentifierStr);
  if (lexer->CurTok != ')')
    return LogErrorP("Expected ')' in prototype");

  // success.
  lexer->NextToken(); // eat ')'.

  return std::make_unique<PrototypeAST>(FnName, std::move(ArgNames));
}
static std::unique_ptr<ExprAST> ParseExpression();
/// parenexpr ::= '(' expression ')'
static std::unique_ptr<ExprAST> ParseParenExpr() {
  lexer->NextToken(); // eat (.
  auto V = ParseExpression();
  if (!V)
    return nullptr;

  if (lexer->CurTok != ')')
    return LogError("expected ')'");
  lexer->NextToken(); // eat ).
  return V;
}
/// ifexpr ::= 'if' expression 'then' expression 'else' expression
static std::unique_ptr<ExprAST> ParseIfExpr() {
  lexer->NextToken();  // eat the if.

  // condition.
  auto Cond = ParseExpression();
  if (!Cond)
    return nullptr;

  if (lexer->CurTok != tok_then)
    return LogError("expected then");
  lexer->NextToken();  // eat the then

  auto Then = ParseExpression();
  if (!Then)
    return nullptr;

  if (lexer->CurTok != tok_else)
    return LogError("expected else");

  lexer->NextToken();

  auto Else = ParseExpression();
  if (!Else)
    return nullptr;

  return std::make_unique<IfExprAST>(std::move(Cond), std::move(Then),
                                      std::move(Else));
}
/// forexpr ::= 'for' identifier '=' expr ',' expr (',' expr)? 'in' expression
static std::unique_ptr<ExprAST> ParseForExpr() {
  lexer->NextToken();  // eat the for.

  if (lexer->CurTok != tok_identifier)
    return LogError("expected identifier after for");

  std::string IdName = lexer->IdentifierStr;
  lexer->NextToken();  // eat identifier.

  if (lexer->CurTok != '=')
    return LogError("expected '=' after for");
  lexer->NextToken();  // eat '='.


  auto Start = ParseExpression();
  if (!Start)
    return nullptr;
  if (lexer->CurTok != ',')
    return LogError("expected ',' after for start value");
  lexer->NextToken();

  auto End = ParseExpression();
  if (!End)
    return nullptr;

  // The step value is optional.
  std::unique_ptr<ExprAST> Step;
  if (lexer->CurTok == ',') {
    lexer->NextToken();
    Step = ParseExpression();
    if (!Step)
      return nullptr;
  }

  if (lexer->CurTok != tok_in)
    return LogError("expected 'in' after for");
  lexer->NextToken();  // eat 'in'.

  auto Body = ParseExpression();
  if (!Body)
    return nullptr;

  return std::make_unique<ForExprAST>(IdName, std::move(Start),
                                       std::move(End), std::move(Step),
                                       std::move(Body));
}
/// numberexpr ::= number
static std::unique_ptr<ExprAST> ParseNumberExpr() {
  auto Result = std::make_unique<NumberExprAST>(lexer->NumVal);
  lexer->NextToken(); // consume the number
  return std::move(Result);
}
/// identifierexpr
///   ::= identifier
///   ::= identifier '(' expression* ')'
static std::unique_ptr<ExprAST> ParseIdentifierExpr() {
  std::string IdName = lexer->IdentifierStr;

  lexer->NextToken(); // eat identifier.

  if (lexer->CurTok != '(') // Simple variable ref.
    return std::make_unique<VariableExprAST>(IdName);

  // Call.
  lexer->NextToken(); // eat (
  std::vector<std::unique_ptr<ExprAST>> Args;
  if (lexer->CurTok != ')') {
    while (true) {
      if (auto Arg = ParseExpression())
        Args.push_back(std::move(Arg));
      else
        return nullptr;

      if (lexer->CurTok ==')' )
        break;

      if (lexer->CurTok != ',')
        return LogError("Expected ')' or ',' in argument list");
      lexer->NextToken();
    }
  }

  // Eat the ')'.
  lexer->NextToken();

  return std::make_unique<CallExprAST>(IdName, std::move(Args));
}
/// primary
///   ::= identifierexpr
///   ::= numberexpr
///   ::= parenexpr
static std::unique_ptr<ExprAST> ParsePrimary() {
  switch (lexer->CurTok) {
  default:
    return LogError("unknown token when expecting an expression");
  case tok_identifier:
    return ParseIdentifierExpr();
  case tok_number:
    return ParseNumberExpr();
  case '(':
    return ParseParenExpr();
  case tok_if: 
  return ParseIfExpr();
  case tok_for:
  return ParseForExpr();
  }
}
/// binoprhs
///   ::= ('+' primary)*
static std::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec,
                                              std::unique_ptr<ExprAST> LHS) {
  // If this is a binop, find its precedence.
  while (true) {
    int TokPrec = GetTokPrecedence();

    // If this is a binop that binds at least as tightly as the current binop,
    // consume it, otherwise we are done.
    if (TokPrec < ExprPrec)
      return LHS;

    // Okay, we know this is a binop.
    int BinOp = lexer->CurTok;
    lexer->NextToken(); // eat binop

    // Parse the primary expression after the binary operator.
    auto RHS = ParsePrimary();
    if (!RHS)
      return nullptr;

    // If BinOp binds less tightly with RHS than the operator after RHS, let
    // the pending operator take RHS as its LHS.
    int NextPrec = GetTokPrecedence();
    if (TokPrec < NextPrec) {
      RHS = ParseBinOpRHS(TokPrec + 1, std::move(RHS));
      if (!RHS)
        return nullptr;
    }

    // Merge LHS/RHS.
    LHS =
        std::make_unique<BinaryExprAST>(BinOp, std::move(LHS), std::move(RHS));
  }
}


// expression
///   ::= primary binoprhs
///
static std::unique_ptr<ExprAST>  ParseExpression() {
  auto LHS = ParsePrimary();
  if (!LHS)
    return nullptr;

  return ParseBinOpRHS(0, std::move(LHS));
}
/// definition ::= 'def' prototype expression
static std::unique_ptr<FunctionAST> ParseDefinition() {
  lexer->NextToken(); // eat def.
  auto Proto = ParsePrototype();
  if (!Proto)
    return nullptr;
  if(lexer->CurTok == '{'){

    lexer->NextToken();
    if (auto F = ParseExpression()){
  
    auto M = std::make_unique<FunctionAST>(std::move(Proto), std::move(F));
    if(lexer->CurTok == ';')  
    lexer->NextToken();
    for(int i=0;i<10 && lexer->CurTok != '}' && 
                        lexer->CurTok != tok_def &&
                        lexer->CurTok != tok_extern &&
                        lexer->CurTok != tok_eof            ;i++){
    
    M->Push(std::move(ParseExpression()));
    if(lexer->CurTok == ';') 
    lexer->NextToken();

    }
    if(lexer->CurTok == '}')
    lexer->NextToken();
    return M;
  } 
  }
  else if (auto E = ParseExpression()){
    return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
  }
  return nullptr;
}


/// external ::= 'extern' prototype
static std::unique_ptr<PrototypeAST> ParseExtern() {
  lexer->NextToken(); // eat extern.
  return ParsePrototype();
}

//===----------------------------------------------------------------------===//
// Top-Level parsing and JIT Driver
//===----------------------------------------------------------------------===//
static void InitializeModule() {
  // Open a new context and module.
  TheContext = std::make_unique<LLVMContext>();
  TheModule = std::make_unique<Module>("my cool jit", *TheContext);

  // Create a new builder for the module.
  Builder = std::make_unique<IRBuilder<>>(*TheContext);

  
  // Create a new pass manager attached to it.
  TheFPM = std::make_unique<legacy::FunctionPassManager>(TheModule.get());

  //  simple "peephole" optimizations
  TheFPM->add(createInstructionCombiningPass());
  // Reassociate expressions.
  TheFPM->add(createReassociatePass());
  // Eliminate Common SubExpressions.
  TheFPM->add(createGVNPass());
  // Simplify the control flow graph 
  TheFPM->add(createCFGSimplificationPass());

  TheFPM->doInitialization();
  

}
static void HandleDefinition() {
  if (auto FnAst = ParseDefinition()) {
    
    if(auto *FnIr = FnAst->codegen()){
 
    fprintf(stderr, "Read your function definition.\n");
   // FnIr->print(errs());
    fprintf(stderr, "\n");
    }
  } else {
    // Skip token for error recovery.
    lexer->NextToken();
  }
}
static void HandleExtern() {
  if (auto ProAst = ParseExtern()){ 
    if(auto *FnIr = ProAst->codegen()){
    fprintf(stderr, "Parsed an extern\n");
    fprintf(stderr, "Read extern");
   // FnIr->print(errs());
    fprintf(stderr, "\n");
  }
  } else {
    // Skip token for error recovery.
    lexer->NextToken();
  }
}


static void ParseProgram() {
  while (true) {
    fprintf(stderr, "ready> ");
    switch (lexer->CurTok) {
    case tok_eof:
      return;
    case ';':
      lexer->NextToken();
      break;
    case tok_def:
      HandleDefinition();
      break;
    case tok_extern:
      HandleExtern();
      break;
    default:
      LogError("Syntax Error");
      lexer->NextToken();
      break;
    }
  }
}

//===----------------------------------------------------------------------===//
// Main driver code.
//===----------------------------------------------------------------------===//

int main(int argc, char* argv[]) {
 std::error_code EC;
//raw_fd_ostream outputFile("output.ll", EC, llvm::sys::fs::OF_None);
std::string Output;

if(argc == 1){
Output = "output.ll";
system("ls | grep output*.ll | wc -l > 8num.txt");
lexer = new LexicalAnalyzer(fopen("8num.txt","r"));
lexer->NextToken();
system("rm 8num.txt");
int L  = (int)lexer->NumVal + 1 + 48;
Output.insert(6,1,(char)L);
delete lexer;
}
else
Output = argv[1];
if(argc == 3){
  lexer = new LexicalAnalyzer(fopen(argv[2],"r"));
}
else
lexer = new LexicalAnalyzer();
 llvm::raw_fd_ostream outputFile(Output, EC);

  fprintf(stderr, "ready> ");
 lexer->NextToken();


  InitializeModule();


  ParseProgram();
  delete lexer;
  InitializeAllTargetInfos();
  InitializeAllTargets();
  InitializeAllTargetMCs();
  InitializeAllAsmParsers();
  InitializeAllAsmPrinters();
  auto TargetTriple = sys::getDefaultTargetTriple();
  TheModule->setTargetTriple(TargetTriple);

  std::string Error;
  auto Target = TargetRegistry::lookupTarget(TargetTriple, Error);

  if (!Target) {
    errs() << Error;
    return 1;
  }

  auto CPU = "generic";
  auto Features = "";

  TargetOptions opt;
  auto RM = std::optional<Reloc::Model>();
  auto TheTargetMachine =
      Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);

  TheModule->setDataLayout(TheTargetMachine->createDataLayout());
 

  if (!EC) {
    // Print out all of the generated code.
    TheModule->print(outputFile, nullptr);
    outputFile.close();

    if (outputFile.has_error()) {
        errs() << "Error writing to the output file\n";
        return 0;
    } else {
        outs() << "LLVM IR successfully written to " << Output << "\n";
    }
} else {
    errs() << "Error opening the output file\n";
    return 0;
}
Function* MainP = TheModule->getFunction("main");
if(!MainP){
  outs()<< "Warning: No Main Function found\n";
  return 0;
}
else{
int c;
outs()<< "Do you want to compile the code? y/n\v";
c=getchar();
if(c == 'y' || c == 'Y' || c==EOF){
std::string Klib = "lib.o";//Libraries that contain the externed code.
std::string ComCommand = "clang " + Klib + ' ' + Output + " -o " + Output + ".e";
system(ComCommand.c_str());
outs()<< "Executable file generated with name: "+Output+".e\n";
}
else
outs()<<"Code Not Compiled";
  return 0;
  
}
}

