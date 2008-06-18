header {
  package org.apache.yoko.tools.processors.idl;

  import java.io.*;
  import java.util.Vector;
  import java.util.Hashtable;
 }

/**
 *  This is a complete parser for the IDL language as defined
 *  by the CORBA 3.0.2 specification.  It will allow those who
 *  need an IDL parser to get up-and-running very quickly.
 *  Though IDL's syntax is very similar to C++, it is also
 *  much simpler, due in large part to the fact that it is
 *  a declarative-only language.
 *
 *  Some things that are not included are: Symbol table construction
 *  (it is not necessary for parsing, btw) and preprocessing (for
 *  IDL compiler #pragma directives). You can use just about any
 *  C or C++ preprocessor, but there is an interesting semantic
 *  issue if you are going to generate code: In C, #include is
 *  a literal include, in IDL, #include is more like Java's import:
 *  It adds definitions to the scope of the parse, but included
 *  definitions are not generated.
 *
 *  Jim Coker, jcoker@magelang.com
 *  Gary Duzan, gduzan@bbn.com
 *  Modified by Edell Nolan May 3, 2007:
 *    We originally used the corba grammar supplied on your site
 *    but it doesn't support forward declaration support for interfaces
 *    we have actually modified the grammar and fixed it. 
 */
class IDLParser extends Parser;
options {
	exportVocab=IDL;
	buildAST=true;
	k=4;
}

specification
	:   (import_dcl)* (definition)+
	;


definition
	:   (   type_dcl SEMI!
	    |   const_dcl SEMI!
	    |   except_dcl SEMI!
	    |   (("abstract" | "local")? "interface") => interf SEMI!
	    |   module SEMI!
	    |   (("abstract" | "custom")? "valuetype") => value SEMI!
	    |   type_id_dcl SEMI!
	    |   type_prefix_dcl SEMI!
	    |   (("abstract" | "custom")? "eventtype") => event SEMI!
	    |   component SEMI!
	    |   home_dcl SEMI!
	    )
	;

module
	:    "module"^
	     identifier	    
	     LCURLY! d:definition_list RCURLY!
	;

definition_list
	:   (definition)+
	;

interf
    : ( interface_dcl
      | forward_dcl
      )
    ;
 
// Grammar changed to differentiate between 
// forward declared interfaces and empty interfaces
interface_dcl
	:   (( "abstract" | "local" )?
 	    "interface"^
 	    identifier
	    ( interface_inheritance_spec )?	   
	    LCURLY interface_body RCURLY) 
	;
	
forward_dcl
 	:    ( "abstract" | "local" )?
 	    "interface"^
 	    identifier
 	;


interface_body
	:   ( export )*
	;

export
	:   (   type_dcl SEMI!
	    |   const_dcl SEMI!
	    |   except_dcl SEMI!
	    |   attr_dcl SEMI!
	    |   op_dcl SEMI!
	    |   type_id_dcl SEMI!
	    |   type_prefix_dcl SEMI!
	    )
	;


interface_inheritance_spec
	:   COLON^ scoped_name_list
	;

interface_name
	:   scoped_name
	;

scoped_name_list
	:    scoped_name (COMMA! scoped_name)*
	;


scoped_name
	:   ( SCOPEOP^ )? IDENT^ /* identifier */ (SCOPEOP! identifier)*
	;

value
	:   ( value_dcl
	    | value_abs_dcl
	    | value_box_dcl
	    | value_custom_dcl
	    | value_forward_dcl
	    )
	;

value_forward_dcl
	:   "valuetype"^
	    identifier
	;

value_box_dcl
	:   "valuetype"^
	    identifier
	    type_spec
	;

value_abs_dcl
	:   "abstract"
	    "valuetype"^
	    identifier
	    ( value_abs_full_dcl
	    | // value_abs_forward_dcl
	    )
	;

value_abs_full_dcl
	:   value_inheritance_spec
	    LCURLY! ( export )* RCURLY!
	;

// value_abs_forward_dcl
// 	:
// 	;

value_dcl
	:   value_header
	    LCURLY! ( value_element )* RCURLY!
	;

value_custom_dcl
	:   "custom"^
	    value_dcl
	;

value_header
	:   "valuetype"^
	    identifier
	    value_inheritance_spec
	;

value_inheritance_spec
/*
	:   ( COLON ( "truncatable" )?
	      value_name ( COMMA! value_name )*
	    )?
	    ( "supports" interface_name ( COMMA! interface_name )* )?
	;
*/
	:   ( value_value_inheritance_spec )?
	    ( value_interface_inheritance_spec )?
	;

value_value_inheritance_spec
	:   COLON^ ( "truncatable" )?
	    value_name ( COMMA! value_name )*
	;

value_interface_inheritance_spec
	:   "supports"^ interface_name ( COMMA! interface_name )*
	;

value_name
	:   scoped_name
	;

value_element
	:   ( export
	    | state_member
	    | init_dcl
	    )
	;

state_member
	:   ( "public" | "private" )
	    type_spec declarators SEMI!
	;

init_dcl
	:   "factory"^ identifier
	    LPAREN! (init_param_decls)? RPAREN!
	    (raises_expr)?
	    SEMI!
	;

init_param_decls
	:   init_param_decl ( COMMA! init_param_decl )*
	;

init_param_decl
	:   init_param_attribute
	    param_type_spec
	    simple_declarator
	;

init_param_attribute
	:   "in"
	;

const_dcl
	:   "const"^ const_type identifier ASSIGN! const_exp
	;

const_type
	:   (integer_type) => integer_type
	|   char_type
	|   wide_char_type
	|   boolean_type
	|   floating_pt_type
	|   string_type
	|   wide_string_type
	|   fixed_pt_const_type
	|   scoped_name
	|   octet_type
	;


/*   EXPRESSIONS   */

const_exp
	:   or_expr
	;

or_expr
	:   xor_expr
	    ( OR^ // or_op
	      xor_expr
	    )*
	;

// or_op
// 	:    OR
// 	;


xor_expr
	:   and_expr
	    ( XOR^ // xor_op
	      and_expr
	    )*
	;

// xor_op
// 	:    XOR
// 	;

and_expr
	:   shift_expr
	    ( AND^ // and_op
	      shift_expr
	    )*
	;

// and_op
// 	:    AND
// 	;


shift_expr
	:   add_expr
	    ( ( LSHIFT^
	      | RSHIFT^
	      ) // shift_op
	    add_expr
	    )*
	;

// shift_op
// 	:    LSHIFT
// 	|    RSHIFT
// 	;


add_expr
	:   mult_expr
	    ( ( PLUS^
	      | MINUS^
	      ) // add_op
	      mult_expr
	    )*
	;

// add_op
// 	:    PLUS
// 	|    MINUS
// 	;

mult_expr
	:   unary_expr
	    ( ( STAR^
	      | DIV^
	      | MOD^
	      ) // mult_op
	      unary_expr
	    )*
	;

// mult_op
// 	:    STAR
// 	|    DIV
// 	|    MOD
// 	;

unary_expr
	:   ( MINUS^
	    | PLUS^
	    | TILDE^
	    ) // unary_operator
	    primary_expr
	|   primary_expr
	;

// unary_operator
// 	:   MINUS
// 	|   PLUS
// 	|   TILDE
// 	;

// Node of type TPrimaryExp serves to avoid inf. recursion on tree parse
primary_expr
	:   scoped_name
	|   literal
	|   LPAREN^ const_exp RPAREN
	;

literal
	:   integer_literal
	|   string_literal
	|   wide_string_literal
	|   character_literal
	|   wide_character_literal
	|   fixed_pt_literal
	|   floating_pt_literal
	|   boolean_literal
	;

boolean_literal
	:   "TRUE"
	|   "FALSE"
	;

positive_int_const
	:    const_exp
	;


type_dcl
	:   "typedef"^ type_declarator
	|   (struct_type) => struct_type
	|   (union_type) => union_type
	|   enum_type
	|   "native"^ simple_declarator
	|   constr_forward_decl
	;

type_declarator
	:   type_spec declarators
	;

type_spec
	:   simple_type_spec
	|   constr_type_spec
	;

simple_type_spec
	:   base_type_spec
	|   template_type_spec
	|   scoped_name
	;

base_type_spec
	:   (floating_pt_type) => floating_pt_type	
	|   integer_type	
	|   char_type		
	|   wide_char_type		
	|   boolean_type	
	|   octet_type
	|   any_type
	|   object_type
	|   value_base_type
	;

template_type_spec
	:   sequence_type
	|   string_type
	|   wide_string_type
	|   fixed_pt_type
	;

constr_type_spec
	:   struct_type
	|   union_type
	|   enum_type
	;

declarators
	:   declarator (COMMA! declarator)*
	;

declarator
	:   simple_declarator
	|   complex_declarator
	;

simple_declarator
	:   identifier
	;

complex_declarator
	:   array_declarator
	;

floating_pt_type
	:   "float"
	|   "double"
	|   "long"^ "double"
	;

integer_type
	:  signed_int
	|  unsigned_int
	;

signed_int
	:  signed_short_int
	|  signed_long_int
	|  signed_longlong_int
	;

signed_short_int
	:  "short"
	;

signed_long_int
	:  "long"
	;

signed_longlong_int
	:  "long" "long"
	;

unsigned_int
	:  unsigned_short_int
	|  unsigned_long_int
	|  unsigned_longlong_int
	;

unsigned_short_int
	:  "unsigned" "short"
	;

unsigned_long_int
	:  "unsigned" "long"
	;

unsigned_longlong_int
	:  "unsigned" "long" "long"
	;

char_type
	:   "char"
	;

wide_char_type
	:   "wchar"
	;

boolean_type
	:   "boolean"
	;

octet_type
	:   "octet"
	;

any_type
	:   "any"
	;

object_type
	:   "Object"
	;

struct_type
	:   "struct"^
	    identifier
	    LCURLY! member_list RCURLY!
	;

member_list
	:   (member)+
	;

member
	:   type_spec declarators SEMI!
	;

union_type
	:   "union"^
	    identifier
	    "switch"! LPAREN! switch_type_spec RPAREN!
	    LCURLY! switch_body RCURLY!
	;

switch_type_spec
	:   integer_type
	|   char_type
	|   boolean_type
	|   enum_type
	|   scoped_name
	;

switch_body
	:   case_stmt_list
	;

case_stmt_list
	:  (case_stmt)+
	;

case_stmt
	:   // case_label_list
	    ( "case"^ const_exp COLON!
	    | "default"^ COLON!
	    )+
	    element_spec SEMI!
	;

// case_label_list
// 	:   (case_label)+
// 	;


// case_label
// 	:   "case"^ const_exp COLON!
// 	|   "default"^ COLON!
// 	;

element_spec
	:   type_spec declarator
	;

enum_type
	:   "enum"^ identifier LCURLY! enumerator_list RCURLY!
	;

enumerator_list
	:    enumerator (COMMA! enumerator)*
	;

enumerator
	:   identifier
	;

sequence_type
	:   "sequence"^
	     LT! simple_type_spec opt_pos_int GT!
	;

opt_pos_int
	:    (COMMA! positive_int_const)?
	;

string_type
	:   "string"^ (LT! positive_int_const GT!)?
	;

wide_string_type
	:   "wstring"^ (LT! positive_int_const GT!)?
	;

array_declarator
	:   IDENT^					// identifier
	    (fixed_array_size)+
	;

fixed_array_size
	:   LBRACK! positive_int_const RBRACK!
	;

attr_dcl
	:   readonly_attr_spec
	|   attr_spec
	;

except_dcl
	:   "exception"^
	    identifier
	    LCURLY! opt_member_list RCURLY!
	;


opt_member_list
	:    (member)*
	;

op_dcl
	:   (op_attribute)?
	    op_type_spec
	    IDENT^				// identifier
	    parameter_dcls
	    (raises_expr)?
	    (context_expr)?
	;

op_attribute
	:   "oneway"
	;

op_type_spec
	:   param_type_spec
	|   "void"
	;

parameter_dcls
	:   LPAREN! (param_dcl_list)? RPAREN!
	;

param_dcl_list
	:   param_dcl (COMMA! param_dcl)*
	;

param_dcl
	:   ("in"^ | "out"^ | "inout"^)		// param_attribute
	    param_type_spec simple_declarator
	;

// param_attribute
// 	:   "in"
// 	|   "out"
// 	|   "inout"
// 	;

raises_expr
	:   "raises"^ LPAREN! scoped_name_list RPAREN!
	;

context_expr
	:   "context"^ LPAREN! string_literal_list RPAREN!
	;

string_literal_list
	:    string_literal (COMMA! string_literal)*
	;

param_type_spec
	:   base_type_spec
	|   string_type
	|   wide_string_type
	|   scoped_name
	;

fixed_pt_type
	:   "fixed"^ LT! positive_int_const COMMA! positive_int_const GT!
	;

fixed_pt_const_type
	:   "fixed"
	;

value_base_type
	:   "ValueBase"
	;

constr_forward_decl
	:   "struct"^ identifier
	|   "union"^ identifier
	;

import_dcl
	:   "import"^ imported_scope SEMI!
	;

imported_scope
	:   scoped_name
	|   string_literal
	;

type_id_dcl
	:   "typeid"^
	    scoped_name
	    string_literal
	;

type_prefix_dcl
	:   "typeprefix"^
	    scoped_name
	    string_literal
	;

readonly_attr_spec
	:   "readonly" "attribute"^
	    param_type_spec
	    readonly_attr_declarator
	;

readonly_attr_declarator
	:   simple_declarator
	    ( raises_expr
	    | (COMMA! simple_declarator)*
	    )
	;

attr_spec
	:   "attribute"^ param_type_spec attr_declarator
	;

attr_declarator
	:   simple_declarator
	    ( ("getraises" | "setraises") => attr_raises_expr
	    | (COMMA! simple_declarator)*
	    )
	;

attr_raises_expr
	:   (get_excep_expr)?
	    (set_excep_expr)?
	;

get_excep_expr
	:   "getraises"^ exception_list
	;

set_excep_expr
	:   "setraises"^ exception_list
	;

exception_list
	:   LPAREN! scoped_name (COMMA! scoped_name)* RPAREN!
	;

// Component Stuff

component
	:   "component"^
	    identifier
	    (component_dcl)?
	;

component_dcl
	:   (component_inheritance_spec)?
	    (supported_interface_spec)?
	    LCURLY! component_body RCURLY!
	;

supported_interface_spec
	:   "supports"^ scoped_name ( COMMA! scoped_name )*
	;

component_inheritance_spec
	:   COLON^ scoped_name
	;

component_body
	:   (component_export)*
	;

component_export
	:   ( provides_dcl SEMI!
	    | uses_dcl SEMI!
	    | emits_dcl SEMI!
	    | publishes_dcl SEMI!
	    | consumes_dcl SEMI!
	    | attr_dcl SEMI!
	    )
	;

provides_dcl
	:   "provides"^ interface_type identifier
	;

interface_type
	:   ( scoped_name
	    | "Object"
	    )
	;

uses_dcl
	:   "uses"^ ("multiple")? interface_type identifier
	;

emits_dcl
	:   "emits"^ scoped_name identifier
	;

publishes_dcl
	:   "publishes"^ scoped_name identifier
	;

consumes_dcl
	:   "consumes"^ scoped_name identifier
	;

home_dcl
	:   home_header home_body
	;

home_header
	:   "home"^ identifier
	    (home_inheritance_spec)?
	    (supported_interface_spec)?
	    "manages"! scoped_name
	    (primary_key_spec)?
	;

home_inheritance_spec
	:   COLON^ scoped_name
	;

primary_key_spec
	:   "primarykey"^ scoped_name
	;

home_body
	:   LCURLY! (home_export)* RCURLY!
	;

home_export
	:   ( export
	    | factory_dcl SEMI!
	    | finder_dcl SEMI!
	    )
	;

factory_dcl
	:   "factory"^ identifier
	    LPAREN! init_param_decls RPAREN!
	    (raises_expr)?
	;

finder_dcl
	:   "finder"^ identifier
	    LPAREN! init_param_decls RPAREN!
	    (raises_expr)?
	;

event
	:   ( event_abs
	    | event_custom
	    | event_dcl
	    )
	;

event_header
	:   "eventtype"^
	    identifier
	;

event_abs
	:   "abstract"^
	    event_header
	    (event_abs_dcl)?
	;

event_abs_dcl
	:   value_inheritance_spec
	    LCURLY! (export)* RCURLY!
	;

event_custom
	:   "custom"^
	    event_header
	    event_elem_dcl
	;

event_dcl
	:   event_header
	    ( event_elem_dcl
	    | // event_forward_dcl
	    )
	;

event_elem_dcl
	:   value_inheritance_spec
	    LCURLY! (export)* RCURLY!
	;

// event_forward_dcl
// 	:
// 	;

/* literals */
integer_literal
	:   INT
	|   OCTAL
	|   HEX
	;

string_literal
	:  (STRING_LITERAL)+
	;

wide_string_literal
	:  (WIDE_STRING_LITERAL)+
	;

character_literal
	:  CHAR_LITERAL
	;

wide_character_literal
	:  WIDE_CHAR_LITERAL
	;

fixed_pt_literal
	:  FIXED
	;

floating_pt_literal
	:   f:FLOAT
	;

identifier
	:   IDENT
	;

/* IDL LEXICAL RULES  */
class IDLLexer extends Lexer;
options {
	exportVocab=IDL;
	charVocabulary='\u0000'..'\uFFFE';
	k=4;
}

SEMI
options {
  paraphrase = ";";
}
	:	';'
	;

QUESTION
options {
  paraphrase = "?";
}
	:	'?'
	;

LPAREN
options {
  paraphrase = "(";
}
	:	'('
	;

RPAREN
options {
  paraphrase = ")";
}
	:	')'
	;

LBRACK
options {
  paraphrase = "[";
}
	:	'['
	;

RBRACK
options {
  paraphrase = "]";
}
	:	']'
	;

LCURLY
options {
  paraphrase = "{";
}
	:	'{'
	;

RCURLY
options {
  paraphrase = "}";
}
	:	'}'
	;

OR
options {
  paraphrase = "|";
}
	:	'|'
	;

XOR
options {
  paraphrase = "^";
}
	:	'^'
	;

AND
options {
  paraphrase = "&";
}
	:	'&'
	;

COLON
options {
  paraphrase = ":";
}
	:	':'
	;

COMMA
options {
  paraphrase = ",";
}
	:	','
	;

DOT
options {
  paraphrase = ".";
}
	:	'.'
	;

ASSIGN
options {
  paraphrase = "=";
}
	:	'='
	;

NOT
options {
  paraphrase = "!";
}
	:	'!'
	;

LT
options {
  paraphrase = "<";
}
	:	'<'
	;

LSHIFT
options {
  paraphrase = "<<";
}
	: "<<"
	;

GT
options {
  paraphrase = ">";
}
	:	'>'
	;

RSHIFT
options {
  paraphrase = ">>";
}
	: ">>"
	;

DIV
options {
  paraphrase = "/";
}
	:	'/'
	;

PLUS
options {
  paraphrase = "+";
}
	:	'+'
	;

MINUS
options {
  paraphrase = "-";
}
	:	'-'
	;

TILDE
options {
  paraphrase = "~";
}
	:	'~'
	;

STAR
options {
  paraphrase = "*";
}
	:	'*'
	;

MOD
options {
  paraphrase = "%";
}
	:	'%'
	;


SCOPEOP
options {
  paraphrase = "::";
}
	:  	"::"
	;

WS
options {
  paraphrase = "white space";
}
	:	(' '
	|	'\t'
	|	'\n'  { newline(); }
	|	'\r')
		{ $setType(Token.SKIP); }
	;


PREPROC_DIRECTIVE
options {
  paraphrase = "a preprocessor directive";
}

	:
	'#'!
	(~'\n')* '\n'!
	{ $setType(Token.SKIP); newline(); }
	;


SL_COMMENT
options {
  paraphrase = "a comment";
}

	:
	"//"!
	(~'\n')* '\n'
	{ $setType(Token.SKIP); newline(); }
	;

ML_COMMENT
options {
  paraphrase = "a comment";
}
	:
	"/*"!
	(
			'\n' { newline(); }
		|	('*')+
			(	'\n' { newline(); }
			|	~('*' | '/' | '\n')
			)
		|	~('*' | '\n')
	)*
	"*/"!
	{ $setType(Token.SKIP);  }
	;

CHAR_LITERAL
options {
  paraphrase = "a character literal";
}
	:
	'\''!
	( ESC | ~'\'' )
	'\''!
	;

WIDE_CHAR_LITERAL
options {
  paraphrase = "a wide character literal";
}
	: 'L'! CHAR_LITERAL
	;

STRING_LITERAL
options {
  paraphrase = "a string literal";
}
	:
	'"'!
	(ESC|~'"')*
	'"'!
	;


WIDE_STRING_LITERAL
options {
  paraphrase = "a wide string literal";
}
	:
	'L'! STRING_LITERAL
	;

protected
ESC
options {
  paraphrase = "an escape sequence";
}
	:	'\\'!
		(	'n'		{$setText("\n");}
		|	't'		{$setText("\t");}
		|	'v'		{$setText("\013");}
		|	'b'		{$setText("\b");}
		|	'r'		{$setText("\r");}
		|	'f'		{$setText("\r");}
		|	'a'  		{$setText("\007");}
		|	'\\'		{$setText("\\");}
		|	'?'     	{$setText("?");}
		|	'\''		{$setText("'");}
		|	'"'		{$setText("\"");}
		|	OCTDIGIT
			(options {greedy=true;}:OCTDIGIT
			  (options {greedy=true;}:OCTDIGIT)?
			)?
			{char realc = (char) Integer.valueOf($getText, 8).intValue(); $setText(realc);}
		|       'x'! HEXDIGIT
			(options {greedy=true;}:HEXDIGIT)?
			{char realc = (char) Integer.valueOf($getText, 16).intValue(); $setText(realc);}
		|	'u'!
			HEXDIGIT
			(options {greedy=true;}:HEXDIGIT
			  (options {greedy=true;}:HEXDIGIT
			    (options {greedy=true;}:HEXDIGIT)?
			  )?
			)?
			{char realc = (char) Integer.valueOf($getText, 16).intValue(); $setText(realc);}
		)
	;

protected
VOCAB
options {
  paraphrase = "an escaped character value";
}
	:	'\3'..'\377'
	;

protected
DIGIT
options {
  paraphrase = "a digit";
}
	:	'0'..'9'
	;

protected
NONZERODIGIT
options {
  paraphrase = "a non-zero digit";
}
	:	'1'..'9'
	;

protected
OCTDIGIT
options {
  paraphrase = "an octal digit";
}
	:	'0'..'7'
	;

protected
HEXDIGIT
options {
  paraphrase = "a hexadecimal digit";
}
	:	('0'..'9' | 'a'..'f' | 'A'..'F')
	;

HEX
options {
  paraphrase = "a hexadecimal value value";
}

	:    ("0x" | "0X") (HEXDIGIT)+
	;

INT
options {
  paraphrase = "an integer value";
}
	:    NONZERODIGIT (DIGIT)*                  // base-10
	     (  '.' (DIGIT)*
		 ( (('e' | 'E') ('+' | '-')? (DIGIT)+)	{$setType(FLOAT);}
		 | ('d' | 'D')!				{$setType(FIXED);}
		 |					{$setType(FLOAT);}
		 )
	     |   ('e' | 'E') ('+' | '-')? (DIGIT)+   	{$setType(FLOAT);}
	     |   ('d' | 'D')!				{$setType(FIXED);}
	     )?
	;

OCTAL
options {
  paraphrase = "an octal value";
}
	:    '0'
	     ( (DIGIT)+
	     | FLOAT					{$setType(FLOAT);}
	     | ('d' | 'D')!				{$setType(FIXED);}
	     |						{$setType(INT);}
	     )
	;


FLOAT
options {
  paraphrase = "a floating point value";
}

	:    '.' (DIGIT)+
	     ( ('e' | 'E') ('+' | '-')? (DIGIT)+
	     | ('d' | 'D')!				{$setType(FIXED);}
	     )?
	;

IDENT
options {
  paraphrase = "an identifer";
  testLiterals = true;
}

	:   ('a'..'z'|'A'..'Z') ('a'..'z'|'A'..'Z'|'_'|'0'..'9')*
	;

ESCAPED_IDENT
options {
  paraphrase = "an escaped identifer";
  testLiterals = false;			// redundant, but explicit is good.
}
    // NOTE: Adding a ! to the '_' doesn't seem to work,
    //       so we adjust _begin manually.

	:   '_' ('a'..'z'|'A'..'Z') ('a'..'z'|'A'..'Z'|'_'|'0'..'9')*
							{_begin++;$setType(IDENT);}
	;


