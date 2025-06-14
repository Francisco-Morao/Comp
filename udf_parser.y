%{
//-- don't change *any* of these: if you do, you'll break the compiler.
#include <algorithm>
#include <memory>
#include <cstring>
#include <cdk/compiler.h>
#include <cdk/types/types.h>
#include ".auto/all_nodes.h"
#define LINE                         compiler->scanner()->lineno()
#define yylex()                      compiler->scanner()->scan()
#define yyerror(compiler, s)         compiler->scanner()->error(s)
//-- don't change *any* of these --- END!
#define NIL (new cdk::nil_node(LINE))
%}

%parse-param {std::shared_ptr<cdk::compiler> compiler}

%union {
  //--- don't change *any* of these: if you do, you'll break the compiler.
  YYSTYPE() : type(cdk::primitive_type::create(0, cdk::TYPE_VOID)) {}
  ~YYSTYPE() {}
  YYSTYPE(const YYSTYPE &other) { *this = other; }
  YYSTYPE& operator=(const YYSTYPE &other) { type = other.type; return *this; }

  std::shared_ptr<cdk::basic_type> type;        /* expression type */
  //-- don't change *any* of these --- END!

  int                   i;          /* integer value */
  double                d;           /* double value */
  std::string          *s;          /* symbol name or string literal */
  cdk::basic_node      *node;       /* node pointer */
  cdk::sequence_node   *sequence;
  cdk::expression_node *expression; /* expression nodes */
  cdk::lvalue_node     *lvalue;
  udf::block_node       *block;    /* block node */
  std::vector<size_t>  *dims;       /* dimensions of a tensor */
};

%token <i> tINTEGER
%token <d> tREAL
%token <s> tIDENTIFIER tSTRING
%token <expression> tNULLPTR

%token tTYPE_INT tTYPE_REAL tTYPE_PTR tTYPE_STRING tTYPE_VOID tTYPE_TENSOR tTYPE_AUTO
%token tSIZEOF tOBJECTS
%token tGE tLE tEQ tNE tAND tOR tINPUT
%token tPUBLIC tFORWARD tPRIVATE
%token tIF tELIF tELSE
%token tFOR tBREAK tCONTINUE
%token tRETURN tWRITE tWRITELN
%token tCAPACITY tRANK tDIMS tDIM tRESHAPE tCONTRACTION

%type <node> instruction declaration fundec fundef variable argdec ifelif fordec
%type <sequence> file instructions expressions declarations variables opt_variables opt_instructions argdecs 
%type <sequence> for_init fordecs opt_expressions
%type <expression> expression literal opt_initializer tensor_expressions identifier
%type <lvalue> lval
%type<block> block

%type<s> string
%type<type> data_type void_type
%type<dims> dims

%nonassoc tIFX
%nonassoc tELIF 
%nonassoc tELSE

%right '='
%left tOR
%left tAND
%right '~'
%left tNE tEQ
%left '<' tLE tGE '>'
%left '+' '-'
%left '*' '/' '%'
%left tCONTRACTION
%nonassoc '@'
%nonassoc tUNARY
%nonassoc '[' '('

%{
//-- The rules below will be included in yyparse, the main parsing function.
%}
%%

file : /* empty */  { compiler->ast($$ = new cdk::sequence_node(LINE)); }
     | declarations { compiler->ast($$ = $1); }
     ;

declarations : declaration            { $$ = new cdk::sequence_node(LINE, $1); }
           | declarations declaration { $$ = new cdk::sequence_node(LINE, $2, $1); }
           ;

declaration : variable ';' { $$ = $1; }
          | fundec     { $$ = $1; }
          | fundef     { $$ = $1; }
          ;

variables : variable ';'          { $$ = new cdk::sequence_node(LINE, $1); }
         | variables variable ';' { $$ = new cdk::sequence_node(LINE, $2, $1); }
         ;

variable : tFORWARD data_type tIDENTIFIER           
          { 
               $$ = new udf::var_declaration_node(LINE, tPUBLIC, $2, *$3, nullptr); 
               delete $3;
          }
        | tPUBLIC data_type tIDENTIFIER opt_initializer 
          {      
               $$ = new udf::var_declaration_node(LINE, tPUBLIC, $2, *$3, $4);
               delete $3;
          }
        | data_type tIDENTIFIER opt_initializer         
          { 
               $$ = new udf::var_declaration_node(LINE, tPRIVATE, $1, *$2, $3); 
               delete $2;
          }
        | tPUBLIC tTYPE_AUTO tIDENTIFIER '=' expression
          { 
               $$ = new udf::var_declaration_node(LINE, tPUBLIC, nullptr, *$3, $5);
               delete $3;
          }
        | tTYPE_AUTO tIDENTIFIER '=' expression
          {
               $$ = new udf::var_declaration_node(LINE, tPRIVATE, nullptr, *$2, $4);
               delete $2;
          }
        ;

opt_variables : /* empty */   { $$ = NULL; }
            | variables     { $$ = $1; }
            ;

opt_initializer : /* empty */      { $$ = nullptr; }
            | '=' expression      { $$ = $2; }
            ;

void_type : tTYPE_VOID      { $$ = cdk::primitive_type::create(0, cdk::TYPE_VOID); }
         ;

data_type : tTYPE_INT                     { $$ = cdk::primitive_type::create(4, cdk::TYPE_INT); }
         | tTYPE_REAL                   { $$ = cdk::primitive_type::create(8, cdk::TYPE_DOUBLE); }
         | tTYPE_STRING                 { $$ = cdk::primitive_type::create(4, cdk::TYPE_STRING); }
         | tTYPE_TENSOR '<' dims '>'      { $$ = cdk::tensor_type::create(*$3); }
         | tTYPE_PTR '<' data_type '>'     { $$ = cdk::reference_type::create(4, $3); }
         | tTYPE_PTR '<' tTYPE_AUTO '>'  { $$ = cdk::reference_type::create(4, nullptr); }
         | tTYPE_PTR '<' void_type '>'     { $$ = cdk::reference_type::create(4, $3); }
         ;

fundec: data_type tIDENTIFIER '(' argdecs ')' 
          { 
               $$ = new udf::function_declaration_node(LINE, tPRIVATE, $1, *$2, $4); 
               delete $2;
          }
     | tFORWARD data_type tIDENTIFIER '(' argdecs ')' 
          { 
               $$ = new udf::function_declaration_node(LINE, tPUBLIC, $2, *$3, $5); 
               delete $3;
          }
     | tPUBLIC data_type tIDENTIFIER '(' argdecs ')' 
          { 
               $$ = new udf::function_declaration_node(LINE, tPUBLIC, $2, *$3, $5); 
               delete $3;
          }
     | tTYPE_AUTO tIDENTIFIER '(' argdecs ')' 
          { 
               $$ = new udf::function_declaration_node(LINE, tPRIVATE, nullptr, *$2, $4); 
               delete $2;
          }
     | tFORWARD tTYPE_AUTO tIDENTIFIER '(' argdecs ')' 
          { 
               $$ = new udf::function_declaration_node(LINE, tPUBLIC, nullptr, *$3, $5); 
               delete $3;
          }
     | tPUBLIC tTYPE_AUTO tIDENTIFIER '(' argdecs ')' 
          { 
               $$ = new udf::function_declaration_node(LINE, tPUBLIC, nullptr, *$3, $5); 
               delete $3;
          }
     | void_type tIDENTIFIER '(' argdecs ')' 
          {
               $$ = new udf::function_declaration_node(LINE, tPRIVATE, $1, *$2, $4); 
               delete $2;
          }
     | tFORWARD void_type tIDENTIFIER '(' argdecs ')' 
          { 
               $$ = new udf::function_declaration_node(LINE, tPUBLIC,  $2, *$3, $5); 
               delete $3;
          }
     | tPUBLIC void_type tIDENTIFIER '(' argdecs ')' 
          {
               $$ = new udf::function_declaration_node(LINE, tPUBLIC,  $2, *$3, $5); 
               delete $3;
          }
     ;

fundef: data_type tIDENTIFIER '(' argdecs ')' block
          { 
               $$ = new udf::function_definition_node(LINE, tPRIVATE, $1, *$2, $4, $6);
               delete $2;
          }
     | tPUBLIC data_type tIDENTIFIER '(' argdecs ')' block
          { 
               $$ = new udf::function_definition_node(LINE, tPUBLIC, $2, *$3, $5, $7); 
               delete $3;
          }
     | tTYPE_AUTO tIDENTIFIER '(' argdecs ')' block
          { 
               $$ = new udf::function_definition_node(LINE, tPRIVATE, nullptr, *$2, $4, $6); 
               delete $2;
          }
     | tPUBLIC tTYPE_AUTO tIDENTIFIER '(' argdecs ')' block
          { 
               $$ = new udf::function_definition_node(LINE, tPUBLIC, nullptr, *$3, $5, $7); 
               delete $3;
          }
     | void_type tIDENTIFIER '(' argdecs ')' block 
          { 
               $$ = new udf::function_definition_node(LINE, tPRIVATE, $1, *$2, $4, $6); 
               delete $2;
          }
     | tPUBLIC void_type tIDENTIFIER '(' argdecs ')' block 
          {
               $$ = new udf::function_definition_node(LINE, tPUBLIC,  $2, *$3, $5, $7); 
               delete $3;
          }
     ;

argdecs : /* empty */        { $$ = new cdk::sequence_node(LINE); }
       | argdec             { $$ = new cdk::sequence_node(LINE, $1); }
       | argdecs ',' argdec   { $$ = new cdk::sequence_node(LINE, $3, $1); }
       ;

argdec : data_type tIDENTIFIER { $$ = new udf::var_declaration_node(LINE, tPRIVATE, $1, *$2, nullptr); }
       ;

block : '{' opt_variables opt_instructions '}' { $$ = new udf::block_node(LINE, $2, $3); }
     ;

instructions : instruction              { $$ = new cdk::sequence_node(LINE, $1); }
           | instructions instruction   { $$ = new cdk::sequence_node(LINE, $2, $1); }
           ;

instruction : tIF '(' expression ')' instruction %prec tIFX  { $$ = new udf::if_node(LINE, $3, $5); }
          | tIF '(' expression ')' instruction ifelif        { $$ = new udf::if_else_node(LINE, $3, $5, $6); }
          | expression ';'                             { $$ = new udf::evaluation_node(LINE, $1); }
          | tWRITE expressions ';'                     { $$ = new udf::print_node(LINE, $2, false); }
          | tWRITELN expressions ';'                  { $$ = new udf::print_node(LINE, $2, true); }
          | tBREAK                                { $$ = new udf::break_node(LINE); }
          | tCONTINUE                             { $$ = new udf::continue_node(LINE); }
          | tRETURN ';'                             { $$ = new udf::return_node(LINE, nullptr); }
          | tRETURN expression ';'                   { $$ = new udf::return_node(LINE, $2); }
          | tFOR '(' for_init ';' opt_expressions ';' opt_expressions ')' instruction    { $$ = new udf::for_node(LINE, $3, $5, $7, $9); }
          | block                                   { $$ = $1; }
          ;

ifelif : tELSE instruction                             { $$ = $2; }
     | tELIF '(' expression ')' instruction %prec tIFX    { $$ = new udf::if_node(LINE, $3, $5); }
     | tELIF '(' expression ')' instruction ifelif         { $$ = new udf::if_else_node(LINE, $3, $5, $6); }
     ;

fordec : data_type tIDENTIFIER '=' expression { $$ = new udf::var_declaration_node(LINE, tPRIVATE, $1, *$2, $4);  delete $2;}
      ;

fordecs : fordec { $$ = new cdk::sequence_node(LINE, $1); }
       | fordecs ',' fordec { $$ = new cdk::sequence_node(LINE, $3, $1); }
       ;

for_init : /* empty */   { $$ = new cdk::sequence_node(LINE, NIL); }
       | fordecs       { $$ = $1; }
       | tTYPE_AUTO tIDENTIFIER '=' expression 
       { 
          $$ = new cdk::sequence_node(LINE, new udf::var_declaration_node(LINE, tPRIVATE, nullptr, *$2, $4));
          delete $2;
       }
       | expressions { $$ = $1; }
       ;

opt_expressions : /* empty */  { $$ = new cdk::sequence_node(LINE); }
             | expressions    { $$ = $1; }
             ;

opt_instructions: /* empty */ { $$ = new cdk::sequence_node(LINE); }
             | instructions { $$ = $1; }
             ;

lval : tIDENTIFIER               { $$ = new cdk::variable_node(LINE, *$1); delete $1; }
     | expression '@' '(' expressions ')' { $$ = new udf::tensor_index_node(LINE, $1, $4); }
     | expression '[' expression ']' { $$ = new udf::index_node(LINE, $1, $3); }
     ;

expression : string                 { $$ = new cdk::string_node(LINE, *$1); delete $1; }
          | literal                { $$ = $1; }
          /* LEFT VALUES */
          | lval                  { $$ = new cdk::rvalue_node(LINE, $1); }
          /* ASSIGNMENTS */
          | lval '=' expression         { $$ = new cdk::assignment_node(LINE, $1, $3); }
          /* UNARY EXPRESSION */
          | '-' expression %prec tUNARY { $$ = new cdk::unary_minus_node(LINE, $2); }
          | '+' expression %prec tUNARY { $$ = new cdk::unary_plus_node(LINE, $2); }
          | '~' expression              { $$ = new cdk::not_node(LINE, $2); }
          /* ARITHMETIC EXPRESSIONS */
          | expression '+' expression         { $$ = new cdk::add_node(LINE, $1, $3); }
          | expression '-' expression         { $$ = new cdk::sub_node(LINE, $1, $3); }
          | expression '*' expression         { $$ = new cdk::mul_node(LINE, $1, $3); }
          | expression '/' expression         { $$ = new cdk::div_node(LINE, $1, $3); }
          | expression '%' expression         { $$ = new cdk::mod_node(LINE, $1, $3); }
          /* LOGICAL EXPRESSIONS */
          | expression '<' expression         { $$ = new cdk::lt_node(LINE, $1, $3); }
          | expression '>' expression         { $$ = new cdk::gt_node(LINE, $1, $3); }
          | expression tGE expression         { $$ = new cdk::ge_node(LINE, $1, $3); }
          | expression tLE expression         { $$ = new cdk::le_node(LINE, $1, $3); }
          | expression tNE expression         { $$ = new cdk::ne_node(LINE, $1, $3); }
          | expression tEQ expression         { $$ = new cdk::eq_node(LINE, $1, $3); }
          /* LOGICAL EXPRESSIONS */
          | expression tAND expression        { $$ = new cdk::and_node(LINE, $1, $3); }
          | expression tOR expression         { $$ = new cdk::or_node(LINE, $1, $3); }
          /* OTHER EXPRESSION */
          | tINPUT                           { $$ = new udf::input_node(LINE); }
          | tOBJECTS '(' expression ')'           { $$ = new udf::alloc_node(LINE, $3); } 
          | tIDENTIFIER '(' opt_expressions ')'   { $$ = new udf::function_call_node(LINE, *$1, $3); delete $1; }
          | tSIZEOF '(' expression ')'            { $$ = new udf::sizeof_node(LINE, $3); }
          | lval '?'                           { $$ = new udf::address_of_node(LINE, $1); }
          | '(' expression ')'                   { $$ = $2; }
          /* TENSOR */
          | tensor_expressions                 { $$ = $1; }
          ;

tensor_expressions : '[' expressions ']'                      { $$ = new udf::tensor_node(LINE, $2); }
               | identifier '.' tCAPACITY                { $$ = new udf::tensor_capacity_node(LINE, $1); }
               | identifier '.' tRANK                   { $$ = new udf::tensor_rank_node(LINE, $1); }
               | identifier '.' tDIMS                    { $$ = new udf::tensor_dims_node(LINE, $1); }
               | identifier '.' tDIM '(' expression ')'       { $$ = new udf::tensor_dim_node(LINE, $1, $5); }
               | identifier '.' tRESHAPE '(' expressions ')' { $$ = new udf::tensor_reshape_node(LINE, $1, $5); }
               | identifier tCONTRACTION identifier     { $$ = new udf::tensor_contraction_node(LINE, $1, $3); }
               ;

identifier : tIDENTIFIER { $$ = new cdk::rvalue_node(LINE, new cdk::variable_node(LINE, *$1)); delete $1; }
        ;

expressions : expression                 { $$ = new cdk::sequence_node(LINE, $1); }
          | expressions ',' expression     { $$ = new cdk::sequence_node(LINE, $3, $1); }
          ;

literal : tINTEGER                          { $$ = new cdk::integer_node(LINE, $1); }
      | tREAL                             { $$ = new cdk::double_node(LINE, $1); }
      | tNULLPTR                         { $$ = new udf::nullptr_node(LINE); }
      ;

string : tSTRING          { $$ = $1; }
      | string tSTRING    { $$ = $1; $$->append(*$2); delete $2; }
      ;

dims : tINTEGER               { $$ = new std::vector<size_t>(); $$->push_back($1); }
     | dims ',' tINTEGER        { $$ = $1; $$->push_back($3); }
     ;

%%
