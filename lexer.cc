//===----------------------------------------------------------------------===//
// Lexer
//===----------------------------------------------------------------------===//
#include "lexer.h"

          

class LexicalAnalyzer
{
private:
  int gettok();
  FILE* f = NULL;
  int line_no = 1;
public:
  LexicalAnalyzer(){}
 LexicalAnalyzer(FILE* fin);
  ~LexicalAnalyzer(){}
  void Printtoken();
  int CurTok;
  union{
  std::string IdentifierStr; 
  double NumVal;  
  };
  int NextToken();
};
void LexicalAnalyzer::Printtoken(){
  if(CurTok==tok_eof)
  fprintf(stderr,"\nline_no: %d \vCurrent Token: End of file. ",line_no);
  else if(CurTok == tok_number)
  fprintf(stderr,"\nline_no: %d \vCurrent Token: Number = '%lf'. ",line_no,NumVal);
  else if(CurTok == tok_identifier)
  fprintf(stderr,"\nline_no: %d \vCurrent Token: Identifier = '%s'. ",line_no,IdentifierStr.c_str());
  else if(CurTok < 0)
  fprintf(stderr,"\nline_no: %d \vCurrent Token: '%s'. ",line_no,IdentifierStr.c_str());
  else
  fprintf(stderr,"\nline_no: %d \vCurrent Token '%c'. ",line_no,CurTok);

}
LexicalAnalyzer::LexicalAnalyzer(FILE* fin){
  if(fin == NULL)
  fprintf(stderr,"Error Opening File\n");
  else
  f = fin;
}

int LexicalAnalyzer::NextToken(){
  
  CurTok = gettok();
  return CurTok;
}

int LexicalAnalyzer::gettok(){
  static int LastChar = ' ';
   
  // Skip any whitespace.
  while (isspace(LastChar)){
    if(LastChar == 10)
    line_no++;
    LastChar = (f==NULL)? getchar(): getc(f);

    }
  if (isalpha(LastChar)) { // identifier: [a-zA-Z][a-zA-Z0-9]*
    IdentifierStr = LastChar;
    while (isalnum((LastChar = (f==NULL)? getchar(): getc(f))))
      IdentifierStr += LastChar;

    if (IdentifierStr == "def")
      return tok_def;
    if (IdentifierStr == "extern")
      return tok_extern;
    if (IdentifierStr == "if")
  return tok_if;
    if (IdentifierStr == "then")
  return tok_then;
    if (IdentifierStr == "else")
  return tok_else;
    if (IdentifierStr == "for")
  return tok_for;
if (IdentifierStr == "in")
  return tok_in;
    return tok_identifier;
  }

  if (isdigit(LastChar) || LastChar == '.') { // Number: [0-9.]+
    std::string NumStr;
    do {
      NumStr += LastChar;
      LastChar = (f==NULL)? getchar(): getc(f);
    } while (isdigit(LastChar) || LastChar == '.');

    NumVal = strtod(NumStr.c_str(), nullptr);
    return tok_number;
  }

  if (LastChar == '#') {//comments
    do
      LastChar = (f==NULL)? getchar(): getc(f);
    while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');

    if (LastChar != EOF)
      return gettok();
  }


  if (LastChar == EOF)
    return tok_eof;

  int ThisChar = LastChar;
  LastChar = (f==NULL)? getchar(): getc(f);
  return ThisChar;
}
