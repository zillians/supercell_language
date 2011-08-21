/**
 * Zillians MMO
 * Copyright (C) 2007-2010 Zillians.com, Inc.
 * For more information see http://www.zillians.com
 *
 * Zillians MMO is the library and runtime for massive multiplayer online game
 * development in utility computing model, which runs as a service for every
 * developer to build their virtual world running on our GPU-assisted machines.
 *
 * This is a close source library intended to be used solely within Zillians.com
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
/**
 * @date Jul 17, 2011 sdk - Initial version created.
 */

#ifndef ZILLIANS_LANGUAGE_GRAMMAR_THORSCRIPT_H_
#define ZILLIANS_LANGUAGE_GRAMMAR_THORSCRIPT_H_

#define BOOST_SPIRIT_UNICODE

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_numeric.hpp>
#include <boost/spirit/include/support_multi_pass.hpp>
#include <boost/spirit/include/classic_position_iterator.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/qi_no_case.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/repository/include/qi_distinct.hpp>
#include <boost/spirit/repository/include/qi_iter_pos.hpp>
#include <boost/regex/pending/unicode_iterator.hpp>
#include "utility/UnicodeUtil.h"
#include "language/tree/ASTNodeFactory.h"
#include "language/action/detail/ParserState.h"

#define DISTINCT_IDENTIFIER(x)   distinct(unicode::alnum | L'_')[x]
#define DISTINCT_NONASSIGN_OP(x) distinct(L'=')[x]

namespace qi = boost::spirit::qi;
namespace unicode = boost::spirit::unicode;
namespace tree = zillians::language::tree;
using boost::spirit::repository::distinct;
using boost::spirit::ascii::no_case;
using boost::spirit::repository::qi::iter_pos;

namespace zillians { namespace language { namespace grammar {

///////////////////////////////////////////////////////////////////////////////////////////////////
// detail
///////////////////////////////////////////////////////////////////////////////////////////////////

namespace detail {

template <typename Iterator>
struct WhiteSpace : qi::grammar<Iterator>
{
	WhiteSpace() : WhiteSpace::base_type(start)
	{
		comment_c_style = qi::lexeme[L"/*" >> *(unicode::char_ - L"*/") >> L"*/"];
		comment_c_style.name("comment_in_c_style");

		comment_cpp_style = qi::lexeme[L"//" >> *(unicode::char_ - qi::eol) >> qi::eol];
		comment_cpp_style.name("comment_in_cpp_style");

		start
			= unicode::space    // tab/space/cr/lf
			| comment_c_style   // c-style comment "/* */"
			| comment_cpp_style // cpp-style comment "//"
			;

		start.name("WHITESPACE");
	}

	qi::rule<Iterator> start;
	qi::rule<Iterator> comment_c_style;
	qi::rule<Iterator> comment_cpp_style;
};

template <typename Iterator, typename SA>
struct Identifier : qi::grammar<Iterator, typename SA::identifier::attribute_type, typename SA::identifier::local_type>
{
	Identifier() : Identifier::base_type(start_augmented)
	{
		keyword_sym =
#if 1
			L"void",
			L"int8", L"uint8", L"int16", L"uint16", L"int32", L"uint32", L"int64", L"uint64",
			L"float32", L"float64",
#endif
			L"true", L"false", L"null",
			L"const", L"static",
			L"typedef", L"class", L"interface", L"enum",
			L"public", L"protected", L"private",
			L"var", L"function",
			L"if", L"elif", L"else",
			L"switch", L"case", L"default",
			L"foreach", L"in", L"do", L"while",
			L"break", L"continue", L"return",
			L"new", L"as", L"instanceof",
			L"package", L"import",
			L"extends", L"implements";
		keyword = DISTINCT_IDENTIFIER(keyword_sym);

		start %= qi::lexeme[ ((unicode::alpha | L'_') >> *(unicode::alnum | L'_')) - keyword ];

		start_augmented = start [ typename SA::identifier::init() ];

		start_augmented.name("IDENTIFIER");
		if(action::ParserState::instance()->enable_debug_parser)
			debug(start_augmented);
	}

	qi::symbols<wchar_t const> keyword_sym;
	qi::rule<Iterator, std::wstring()> keyword, start;
	qi::rule<Iterator, typename SA::identifier::attribute_type, typename SA::identifier::local_type> start_augmented;
};

template <typename Iterator, typename SA>
struct TypeName : qi::grammar<Iterator, typename SA::identifier::attribute_type, typename SA::identifier::local_type>
{
	TypeName() : TypeName::base_type(start_augmented)
	{
		keyword_sym =
#if 0
			L"void",
			L"int8", L"uint8", L"int16", L"uint16", L"int32", L"uint32", L"int64", L"uint64",
			L"float32", L"float64",
#endif
			L"true", L"false", L"null",
			L"const", L"static",
			L"typedef", L"class", L"interface", L"enum",
			L"public", L"protected", L"private",
			L"var", L"function",
			L"if", L"elif", L"else",
			L"switch", L"case", L"default",
			L"foreach", L"in", L"do", L"while",
			L"break", L"continue", L"return",
			L"new", L"as", L"instanceof",
			L"package", L"import",
			L"extends", L"implements";
		keyword = DISTINCT_IDENTIFIER(keyword_sym);

		start %= qi::lexeme[ ((unicode::alpha | L'_') >> *(unicode::alnum | L'_')) - keyword ];

		start_augmented = start [ typename SA::identifier::init() ];

		start_augmented.name("TYPENAME");
		if(action::ParserState::instance()->enable_debug_parser)
			debug(start_augmented);
	}

	qi::symbols<wchar_t const> keyword_sym;
	qi::rule<Iterator, std::wstring()> keyword, start;
	qi::rule<Iterator, typename SA::identifier::attribute_type, typename SA::identifier::local_type> start_augmented;
};

template <typename Iterator, typename SA>
struct IntegerLiteral : qi::grammar<Iterator, typename SA::integer_literal::attribute_type, typename SA::integer_literal::local_type>
{
	IntegerLiteral() : IntegerLiteral::base_type(start_augmented)
	{
		start
			%= distinct(qi::lit(L'.') | L'x' | no_case[L'e'])[qi::int_]
			| qi::lit(L"0x") >> qi::hex
			;

		start_augmented = start [ typename SA::integer_literal::init() ];

		start_augmented.name("INTEGER_LITERAL");
		if(action::ParserState::instance()->enable_debug_parser)
			debug(start_augmented);
	}

	qi::rule<Iterator, uint64()> start;
	qi::rule<Iterator, typename SA::integer_literal::attribute_type, typename SA::integer_literal::local_type> start_augmented;
};

template <typename Iterator, typename SA>
struct FloatLiteral : qi::grammar<Iterator, typename SA::float_literal::attribute_type, typename SA::float_literal::local_type>
{
	FloatLiteral() : FloatLiteral::base_type(start_augmented)
	{
		start
			%=	( builtin_float_parser
				| (qi::int_ | builtin_float_parser) >> no_case[L'e'] >> -qi::lit(L'-') >> qi::int_
				) > -no_case[L'f']
			;

		start_augmented = start [ typename SA::float_literal::init() ];

		start_augmented.name("FLOAT_LITERAL");
		if(action::ParserState::instance()->enable_debug_parser)
			debug(start_augmented);
	}

	qi::real_parser<double, qi::strict_ureal_policies<double> > builtin_float_parser;
	qi::rule<Iterator, double()> start;
	qi::rule<Iterator, typename SA::float_literal::attribute_type, typename SA::float_literal::local_type> start_augmented;
};

template <typename Iterator, typename SA>
struct StringLiteral : qi::grammar<Iterator, typename SA::string_literal::attribute_type, typename SA::string_literal::local_type>
{
	StringLiteral() : StringLiteral::base_type(start_augmented)
	{
		unescaped_char_sym.add
			(L"\\a", L'\a')
			(L"\\b", L'\b')
			(L"\\f", L'\f')
			(L"\\n", L'\n')
			(L"\\r", L'\r')
			(L"\\t", L'\t')
			(L"\\v", L'\v')
			(L"\\\\", L'\\')
			(L"\\\'", L'\'')
			(L"\\\"", L'\"');

		start
			%= qi::lit(L'\"')
				>>	*( ( ( unicode::char_ - L'\"' ) - L'\\' )
					| unescaped_char_sym
					| L"\\x" >> qi::hex
					)
				>> L'\"'
			;

		start_augmented = start [ typename SA::string_literal::init() ];

		start_augmented.name("STRING_LITERAL");
		if(action::ParserState::instance()->enable_debug_parser)
			debug(start_augmented);
	}

	qi::symbols<wchar_t const, wchar_t const> unescaped_char_sym;
	qi::rule<Iterator, std::wstring()> start;
	qi::rule<Iterator, typename SA::string_literal::attribute_type, typename SA::string_literal::local_type> start_augmented;
};

}

///////////////////////////////////////////////////////////////////////////////////////////////////
// ThorScript
///////////////////////////////////////////////////////////////////////////////////////////////////

template<typename Iterator, typename SA>
struct ThorScript : qi::grammar<Iterator, typename SA::start::attribute_type, detail::WhiteSpace<Iterator>, typename SA::start::local_type >
{
	ThorScript() : ThorScript::base_type(start)
	{
		///////////////////////////////////////////////////////////////////////////////
		// Operators
		///////////////////////////////////////////////////////////////////////////////

		// operators & scope
		{
			// miscellaneous
			{
				ELLIPSIS  = qi::lit(L"...");
				DOT       = qi::lit(L'.');
				COLON     = qi::lit(L":");
				SEMICOLON = qi::lit(L';');
				COMMA     = qi::lit(L',');
				AT_SYMBOL = qi::lit(L'@');
				Q_MARK    = qi::lit(L'?');
			}

			// assignments
			{
				ASSIGN        = distinct(L'=')[qi::lit(L'=')];
				RSHIFT_ASSIGN = qi::lit(L">>=");
				LSHIFT_ASSIGN = qi::lit(L"<<=");
				PLUS_ASSIGN   = qi::lit(L"+=");
				MINUS_ASSIGN  = qi::lit(L"-=");
				MUL_ASSIGN    = qi::lit(L"*=");
				DIV_ASSIGN    = qi::lit(L"/=");
				MOD_ASSIGN    = qi::lit(L"%=");
				AND_ASSIGN    = qi::lit(L"&=");
				OR_ASSIGN     = qi::lit(L"|=");
				XOR_ASSIGN    = qi::lit(L"^=");
			}

			// unary operator
			{
				INCREMENT = qi::lit(L"++");
				DECREMENT = qi::lit(L"--");
			}

			// arithmetic operators
			{
				ARITHMETIC_PLUS  = DISTINCT_NONASSIGN_OP(qi::lit(L'+'));
				ARITHMETIC_MINUS = DISTINCT_NONASSIGN_OP(qi::lit(L'-'));
				ARITHMETIC_MUL   = DISTINCT_NONASSIGN_OP(qi::lit(L'*'));
				ARITHMETIC_DIV   = DISTINCT_NONASSIGN_OP(qi::lit(L'/'));
				ARITHMETIC_MOD   = DISTINCT_NONASSIGN_OP(qi::lit(L'%'));
			}

			// binary operators
			{
				BINARY_AND = distinct(qi::lit(L'&') | L'=')[qi::lit(L'&')];
				BINARY_OR  = distinct(qi::lit(L'|') | L'=')[qi::lit(L'|')];
				BINARY_XOR = DISTINCT_NONASSIGN_OP(qi::lit(L'^'));
				BINARY_NOT = qi::lit(L'~');
			}

			// shift operators
			{
				RSHIFT = DISTINCT_NONASSIGN_OP(qi::lit(L">>"));
				LSHIFT = DISTINCT_NONASSIGN_OP(qi::lit(L"<<"));
			}

			// logical operators
			{
				LOGICAL_AND = qi::lit(L"&&");
				LOGICAL_OR  = qi::lit(L"||");
				LOGICAL_NOT = DISTINCT_NONASSIGN_OP(qi::lit(L'!'));
			}

			// comparison
			{
				COMPARE_EQ = qi::lit(L"==");
				COMPARE_NE = qi::lit(L"!=");
				COMPARE_GT = distinct(qi::lit(L'>') | L'=')[qi::lit(L'>')];
				COMPARE_LT = distinct(qi::lit(L'<') | L'=')[qi::lit(L'<')];
				COMPARE_GE = qi::lit(L">=");
				COMPARE_LE = qi::lit(L"<=");
			}

			// scope
			{
				LEFT_BRACE    = qi::lit(L'{');
				RIGHT_BRACE   = qi::lit(L'}');
				LEFT_BRACKET  = qi::lit(L'[');
				RIGHT_BRACKET = qi::lit(L']');
				LEFT_PAREN    = qi::lit(L'(');
				RIGHT_PAREN   = qi::lit(L')');
			}
		}

		///////////////////////////////////////////////////////////////////////////////
		// Keywords
		///////////////////////////////////////////////////////////////////////////////

#define DECL_TOKEN(token, str) \
	token = DISTINCT_IDENTIFIER(qi::lit(str));

		// tokens
		{
			DECL_TOKEN(_TRUE, L"true");
			DECL_TOKEN(_FALSE, L"false");
			DECL_TOKEN(_NULL, L"null");

			DECL_TOKEN(CONST, L"const");
			DECL_TOKEN(STATIC, L"static");

			DECL_TOKEN(INT8, L"int8");
			DECL_TOKEN(UINT8, L"uint8");
			DECL_TOKEN(INT16, L"int16");
			DECL_TOKEN(UINT16, L"uint16");
			DECL_TOKEN(INT32, L"int32");
			DECL_TOKEN(UINT32, L"uint32");
			DECL_TOKEN(INT64, L"int64");
			DECL_TOKEN(UINT64, L"uint64");
			DECL_TOKEN(FLOAT32, L"float32");
			DECL_TOKEN(FLOAT64, L"float64");
			DECL_TOKEN(VOID, L"void");

			DECL_TOKEN(TYPEDEF, L"typedef");
			DECL_TOKEN(CLASS, L"class");
			DECL_TOKEN(INTERFACE, L"interface");
			DECL_TOKEN(ENUM, L"enum");

			DECL_TOKEN(PUBLIC, L"public");
			DECL_TOKEN(PROTECTED, L"protected");
			DECL_TOKEN(PRIVATE, L"private");

			DECL_TOKEN(VAR, L"var");
			DECL_TOKEN(FUNCTION, L"function");

			DECL_TOKEN(IF, L"if");
			DECL_TOKEN(ELIF, L"elif");
			DECL_TOKEN(ELSE, L"else");

			DECL_TOKEN(SWITCH, L"switch");
			DECL_TOKEN(CASE, L"case");
			DECL_TOKEN(DEFAULT, L"default");

			DECL_TOKEN(FOREACH, L"foreach");
			DECL_TOKEN(IN, L"in");
			DECL_TOKEN(DO, L"do");
			DECL_TOKEN(WHILE, L"while");

			DECL_TOKEN(RETURN, L"return");
			DECL_TOKEN(BREAK, L"break");
			DECL_TOKEN(CONTINUE, L"continue");

			DECL_TOKEN(NEW, L"new");
			DECL_TOKEN(AS, L"as");
			DECL_TOKEN(INSTANCEOF, L"instanceof");

			DECL_TOKEN(PACKAGE, L"package");
			DECL_TOKEN(IMPORT, L"import");

			DECL_TOKEN(EXTENDS, L"extends");
			DECL_TOKEN(IMPLEMENTS, L"implements");
		}

		///////////////////////////////////////////////////////////////////////////////
		// Rule Define
		///////////////////////////////////////////////////////////////////////////////

		///////////////////////////////////////////////////////////
		// BEGIN BASIC
		//

		typed_parameter_list
			= (IDENTIFIER > -colon_type_specifier) % COMMA
			;

		colon_type_specifier
			= COLON > type_specifier [ typename SA::colon_type_specifier::init() ]
			;

		type_specifier
			= (TYPENAME > -template_arg_specifier)                                                 [ typename SA::type_specifier::init_type() ]
			| (FUNCTION > LEFT_PAREN > -type_list_specifier > RIGHT_PAREN > -colon_type_specifier) [ typename SA::type_specifier::init_function_type() ]
			| ELLIPSIS                                                                             [ typename SA::type_specifier::init_ellipsis() ]
			;

		template_param_identifier
			= IDENTIFIER > -(COMPARE_LT >> ((IDENTIFIER | ELLIPSIS) % COMMA) > COMPARE_GT)
			;

		template_arg_identifier
			= (IDENTIFIER > -template_arg_specifier) [ typename SA::template_arg_identifier::init() ]
			;

		template_arg_specifier
			= (COMPARE_LT >> type_list_specifier > COMPARE_GT) [ typename SA::template_arg_specifier::init() ]
			;

		type_list_specifier
			= (type_specifier % COMMA) [ typename SA::type_list_specifier::init() ]
			;

		storage_specifier
			= STATIC [ typename SA::storage_specifier::init_static() ]
			| CONST  [ typename SA::storage_specifier::init_const() ]
			;

		visibility_specifier
			= PUBLIC    [ typename SA::visibility_specifier::init_public() ]
			| PROTECTED [ typename SA::visibility_specifier::init_protected() ]
			| PRIVATE   [ typename SA::visibility_specifier::init_private() ]
			;

		annotation_specifiers
			= qi::eps                  [ typename SA::annotation_specifiers::init() ]
			>> *( annotation_specifier [ typename SA::annotation_specifiers::append_annotation() ] )
			;

		annotation_specifier
			= AT_SYMBOL >> IDENTIFIER [ typename SA::annotation_specifier::init() ]
				>>	-(LEFT_BRACE >> (
					(IDENTIFIER > ASSIGN > primary_expression) [ typename SA::annotation_specifier::append_keyvalue() ] % COMMA
					) >> RIGHT_BRACE)
			;

		nested_identifier
			= qi::eps                   [ typename SA::nested_identifier::init() ]
				>> IDENTIFIER           [ typename SA::nested_identifier::append_identifier() ]
				>> *( DOT >> IDENTIFIER [ typename SA::nested_identifier::append_identifier() ] )
			| qi::eps                   [ typename SA::nested_identifier::abort() ]
			;

		//
		// END BASIC
		///////////////////////////////////////////////////////////

		///////////////////////////////////////////////////////////
		// BEGIN EXPRESSION
		//
		// 0.  POSTFIX_STEP
		// 1.  UNARY_SIGN, PREFIX_STEP
		// 2.  MULTIPLICATIVE
		// 3.  ADDITIVE
		// 4.  BITWISE_SHIFT
		// 5.  RELATIONAL
		// 6.  EQUALITY
		// 7.  BITWISE_AND
		// 8.  BITWISE_XOR
		// 9.  BITWISE_OR
		// 10. LOGICAL_AND
		// 11. LOGICAL_OR
		// 12. RANGE
		// 13. TERNARY
		// 14. ASSIGNMENT, MODIFY_ASSIGN
		// 15. COMMA
		//

		// primary expression
		primary_expression
			=	(template_arg_identifier
				| INTEGER_LITERAL
				| FLOAT_LITERAL
				| STRING_LITERAL
				| lambda_expression
				)                                       [ typename SA::primary_expression::init() ]
			| _TRUE                                     [ typename SA::primary_expression::init_true() ]
			| _FALSE                                    [ typename SA::primary_expression::init_false() ]
			| _NULL                                     [ typename SA::primary_expression::init_null() ]
			| (LEFT_PAREN >> expression >> RIGHT_PAREN) [ typename SA::primary_expression::init_paren_expression() ]
			;

		lambda_expression
			= (FUNCTION > LEFT_PAREN > -typed_parameter_list > RIGHT_PAREN > -colon_type_specifier
				> compound_statement) [ typename SA::lambda_expression::init() ]
			;

		// postfix expression
		// associativity: left-to-right
		// rank: 0
		postfix_expression
			= primary_expression                                           [ typename SA::postfix_expression::init_primary_expression() ]
				>>	*( (LEFT_BRACKET >> expression >> RIGHT_BRACKET)       [ typename SA::postfix_expression::init_postfix_array() ]
					| (LEFT_PAREN >> -(expression % COMMA) >> RIGHT_PAREN) [ typename SA::postfix_expression::init_postfix_call() ]
					| (DOT >> template_arg_identifier)                     [ typename SA::postfix_expression::init_postfix_member() ]
					| INCREMENT                                            [ typename SA::postfix_expression::init_postfix_inc() ]
					| DECREMENT                                            [ typename SA::postfix_expression::init_postfix_dec() ]
					)
			;

		// prefix expression
		// associativity: right-to-left
		// rank: 1
		prefix_expression
			= postfix_expression [ typename SA::prefix_expression::init_postfix_expression() ]
			|	qi::eps [ qi::_a = tree::UnaryExpr::OpCode::INVALID ]
				>>	(
						( INCREMENT        [ qi::_a = tree::UnaryExpr::OpCode::PREFIX_INCREMENT ]
						| DECREMENT        [ qi::_a = tree::UnaryExpr::OpCode::PREFIX_DECREMENT ]
						| BINARY_NOT       [ qi::_a = tree::UnaryExpr::OpCode::BINARY_NOT ]
						| LOGICAL_NOT      [ qi::_a = tree::UnaryExpr::OpCode::LOGICAL_NOT ]
						| ARITHMETIC_MINUS [ qi::_a = tree::UnaryExpr::OpCode::ARITHMETIC_NEGATE ]
						| NEW              [ qi::_a = tree::UnaryExpr::OpCode::NEW ]
						) >> prefix_expression
					) [ typename SA::prefix_expression::init() ]
			;

		// multiplicative expression
		// associativity: left-to-right
		// rank: 2
		multiplicative_expression
			= qi::eps [ qi::_a = tree::BinaryExpr::OpCode::INVALID ]
				>>	(prefix_expression
					%	( ARITHMETIC_MUL [ qi::_a = tree::BinaryExpr::OpCode::ARITHMETIC_MUL ]
						| ARITHMETIC_DIV [ qi::_a = tree::BinaryExpr::OpCode::ARITHMETIC_DIV ]
						| ARITHMETIC_MOD [ qi::_a = tree::BinaryExpr::OpCode::ARITHMETIC_MOD ]
						)
					) [ typename SA::multiplicative_expression::init() ]
			;

		// additive expression
		// associativity: left-to-right
		// rank: 3
		additive_expression
			= qi::eps [ qi::_a = tree::BinaryExpr::OpCode::INVALID ]
				>>	(multiplicative_expression
					%	( ARITHMETIC_PLUS  [ qi::_a = tree::BinaryExpr::OpCode::ARITHMETIC_ADD ]
						| ARITHMETIC_MINUS [ qi::_a = tree::BinaryExpr::OpCode::ARITHMETIC_SUB ]
						)
					) [ typename SA::additive_expression::init() ]
			;

		// shift expression
		// associativity: left-to-right
		// rank: 4
		shift_expression
			= qi::eps [ qi::_a = tree::BinaryExpr::OpCode::INVALID ]
				>>	(additive_expression
					%	( RSHIFT [ qi::_a = tree::BinaryExpr::OpCode::BINARY_RSHIFT ]
						| LSHIFT [ qi::_a = tree::BinaryExpr::OpCode::BINARY_LSHIFT ]
						)
					) [ typename SA::shift_expression::init() ]
			;

		// rational expression
		// associativity: left-to-right
		// rank: 5
		relational_expression
			= qi::eps [ qi::_a = tree::BinaryExpr::OpCode::INVALID ]
				>>	(shift_expression
					%	( COMPARE_GT [ qi::_a = tree::BinaryExpr::OpCode::COMPARE_GT ]
						| COMPARE_LT [ qi::_a = tree::BinaryExpr::OpCode::COMPARE_LT ]
						| COMPARE_GE [ qi::_a = tree::BinaryExpr::OpCode::COMPARE_GE ]
						| COMPARE_LE [ qi::_a = tree::BinaryExpr::OpCode::COMPARE_LE ]
						| INSTANCEOF [ qi::_a = tree::BinaryExpr::OpCode::INSTANCEOF ]
						)
					) [ typename SA::relational_expression::init() ]
			;

		// equality expression
		// associativity: left-to-right
		// rank: 6
		equality_expression
			= qi::eps [ qi::_a = tree::BinaryExpr::OpCode::INVALID ]
				>>	(relational_expression
					%	( COMPARE_EQ [ qi::_a = tree::BinaryExpr::OpCode::COMPARE_EQ ]
						| COMPARE_NE [ qi::_a = tree::BinaryExpr::OpCode::COMPARE_NE ]
						)
					) [ typename SA::equality_expression::init() ]
			;

		// and expression
		// associativity: left-to-right
		// rank: 7
		and_expression
			= (equality_expression % BINARY_AND) [ typename SA::and_expression::init() ]
			;

		// xor expression
		// associativity: left-to-right
		// rank: 8
		xor_expression
			= (and_expression % BINARY_XOR) [ typename SA::xor_expression::init() ]
			;

		// or expression
		// associativity: left-to-right
		// rank: 9
		or_expression
			= (xor_expression % BINARY_OR) [ typename SA::or_expression::init() ]
			;

		// logical and expression
		// associativity: left-to-right
		// rank: 10
		logical_and_expression
			= (or_expression % LOGICAL_AND) [ typename SA::logical_and_expression::init() ]
			;

		// logical or expression
		// associativity: left-to-right
		// rank: 11
		logical_or_expression
			= (logical_and_expression % LOGICAL_OR) [ typename SA::logical_or_expression::init() ]
			;

		// range expression
		// associativity: left-to-right
		// rank: 12
		range_expression
			= (logical_or_expression > -(ELLIPSIS > logical_or_expression)
				) [ typename SA::range_expression::init() ]
			;

		// ternary expression
		// associativity: right-to-left
		// rank: 13
		ternary_expression
			= (range_expression > -(Q_MARK > range_expression > COLON > range_expression)
				) [ typename SA::ternary_expression::init() ]
			;

		// assignment_expression
		// associativity: right-to-left
		// rank: 14
		expression
			= qi::eps [ qi::_a = tree::BinaryExpr::OpCode::INVALID ]
			    >>	(ternary_expression
					%	( ASSIGN        [ qi::_a = tree::BinaryExpr::OpCode::ASSIGN ]
						| RSHIFT_ASSIGN [ qi::_a = tree::BinaryExpr::OpCode::RSHIFT_ASSIGN ]
						| LSHIFT_ASSIGN [ qi::_a = tree::BinaryExpr::OpCode::LSHIFT_ASSIGN ]
						| PLUS_ASSIGN   [ qi::_a = tree::BinaryExpr::OpCode::ADD_ASSIGN ]
						| MINUS_ASSIGN  [ qi::_a = tree::BinaryExpr::OpCode::SUB_ASSIGN ]
						| MUL_ASSIGN    [ qi::_a = tree::BinaryExpr::OpCode::MUL_ASSIGN ]
						| DIV_ASSIGN    [ qi::_a = tree::BinaryExpr::OpCode::DIV_ASSIGN ]
						| MOD_ASSIGN    [ qi::_a = tree::BinaryExpr::OpCode::MOD_ASSIGN ]
						| AND_ASSIGN    [ qi::_a = tree::BinaryExpr::OpCode::AND_ASSIGN ]
						| OR_ASSIGN     [ qi::_a = tree::BinaryExpr::OpCode::OR_ASSIGN ]
						| XOR_ASSIGN    [ qi::_a = tree::BinaryExpr::OpCode::XOR_ASSIGN ]
						)
					) [ typename SA::expression::init() ]
			;

		//
		// END EXPRESSION
		///////////////////////////////////////////////////////////

		///////////////////////////////////////////////////////////
		// BEGIN STATEMENT
		//

		// statement
		statement
			= -annotation_specifiers
				>>	( compound_statement
					| -CONST >> variable_decl
					| expression_statement
					| selection_statement
					| iteration_statement
					| branch_statement
					)
			;

		// compound_statement
		compound_statement
			= LEFT_BRACE
			> *statement
			> RIGHT_BRACE
			;

		// expression_statement
		expression_statement
			= -expression >> SEMICOLON
			;

		// selection_statement
		selection_statement
			= IF > LEFT_PAREN > expression > RIGHT_PAREN > statement
				> *(ELIF > LEFT_PAREN > expression > RIGHT_PAREN > statement)
				> -(ELSE > statement)
			| SWITCH > LEFT_PAREN > expression > RIGHT_PAREN
				> LEFT_BRACE
				>	*( CASE > expression > COLON > statement
					| DEFAULT > COLON > statement
					)
				> RIGHT_BRACE
			;

		// iteration_statement
		iteration_statement
			= WHILE > LEFT_PAREN > expression > RIGHT_PAREN > statement
			| DO > statement > WHILE > LEFT_PAREN > expression > RIGHT_PAREN > SEMICOLON
			| FOREACH > LEFT_PAREN > -VAR > IDENTIFIER > -colon_type_specifier > IN > (range_expression | IDENTIFIER) > RIGHT_PAREN > statement
			;

		// branch_statement
		branch_statement
			= RETURN > -expression > SEMICOLON
			| BREAK > SEMICOLON
			| CONTINUE > SEMICOLON
			;

		//
		// END STATEMENT
		///////////////////////////////////////////////////////////

		///////////////////////////////////////////////////////////
		// BEGIN DECLARATION
		//

		// global declaration
		declaration
			= (-annotation_specifiers
				>>	( -CONST >> variable_decl
					| function_decl
					| typedef_decl
					| class_decl
					| interface_decl
					| enum_decl
					)
				) [ typename SA::declaration::init() ]
			;

		// variable declaration
		variable_decl
			= (VAR > IDENTIFIER > -colon_type_specifier > -(ASSIGN > expression) > SEMICOLON) [ typename SA::variable_decl::init() ]
			;

		// function declaration
		function_decl
			= (FUNCTION > (template_param_identifier | NEW) > LEFT_PAREN > -typed_parameter_list > RIGHT_PAREN > -colon_type_specifier
				> -compound_statement
				) [ typename SA::function_decl::init() ]
			;

		// typedef declaration
		typedef_decl
			= (TYPEDEF >> type_specifier > IDENTIFIER > SEMICOLON) [ typename SA::typedef_decl::init() ]
			;

		// class declaration
		class_decl
			= (CLASS > template_param_identifier > -(EXTENDS > nested_identifier) > -((IMPLEMENTS > nested_identifier) % COMMA)
				> LEFT_BRACE
				> *class_member_decl
				> RIGHT_BRACE
				) [ typename SA::class_decl::init() ]
			;

		// member declaration
		class_member_decl
			= -annotation_specifiers >> -visibility_specifier >> -storage_specifier
				>>	( variable_decl
					| function_decl
					)
			;

		// interface declaration
		interface_decl
			= (INTERFACE > IDENTIFIER
				> LEFT_BRACE
				> *member_function_decl_without_body
				> RIGHT_BRACE
				) [ typename SA::interface_decl::init() ]
			;

		// function declaration without body (used in interface declaration)
		member_function_decl_without_body
			= FUNCTION > IDENTIFIER > LEFT_PAREN > -typed_parameter_list > RIGHT_PAREN > colon_type_specifier >> SEMICOLON
			;

		// enum declaration
		enum_decl
			= (ENUM > IDENTIFIER
				> LEFT_BRACE
				> (-annotation_specifiers >> IDENTIFIER > -(ASSIGN > INTEGER_LITERAL)) % COMMA
				> RIGHT_BRACE
				) [ typename SA::enum_decl::init() ]
			;

		//
		// END DECLARATION
		///////////////////////////////////////////////////////////

		///////////////////////////////////////////////////////////
		// BEGIN MODULE
		//

		// basic translation unit (file)
		program
			= qi::eps              [ typename SA::program::init() ]
				>> -( package_decl [ typename SA::program::append_package_decl() ] )
				>> *( import_decl  [ typename SA::program::append_import_decl() ] )
				>> *( declaration  [ typename SA::program::append_declaration() ] )
			;

		// package declaration
		package_decl
			= PACKAGE
				> nested_identifier [ typename SA::package_decl::init() ]
				> SEMICOLON
			;

		// import declaration
		import_decl
			= IMPORT > nested_identifier [ typename SA::import_decl::init() ] > SEMICOLON
			;

		//
		// END MODULE
		///////////////////////////////////////////////////////////

		// starting symbol
		start
			= program
			;

		///////////////////////////////////////////////////////////////////////////////
		// Debugging Names / Handlers
		///////////////////////////////////////////////////////////////////////////////

		// keywords
		_TRUE.name("TRUE_LITERAL"); _FALSE.name("FALSE_LITERAL"); _NULL.name("NULL_LITERAL");

		// terminals
		IDENTIFIER.name("IDENTIFIER.grammar");
		TYPENAME.name("TYPENAME.grammar");
		INTEGER_LITERAL.name("INTEGER_LITERAL.grammar");
		FLOAT_LITERAL.name("FLOAT_LITERAL.grammar");
		STRING_LITERAL.name("STRING_LITERAL.grammar");

		// non-terminals
		typed_parameter_list.name("typed_parameter_list"); colon_type_specifier.name("colon_type_specifier"); type_specifier.name("type_specifier"); template_param_identifier.name("template_param_identifier"); template_arg_identifier.name("template_arg_identifier"); template_arg_specifier.name("template_arg_specifier"); type_list_specifier.name("type_list_specifier");
			storage_specifier.name("storage_specifier");
			visibility_specifier.name("visibility_specifier");
			annotation_specifiers.name("annotation_specifiers");
			annotation_specifier.name("annotation_specifier");
			nested_identifier.name("nested_identifier");
		expression.name("expression");
			primary_expression.name("primary_expression");
				lambda_expression.name("lambda_expression");
			postfix_expression.name("postfix_expression");
			prefix_expression.name("prefix_expression");
			multiplicative_expression.name("multiplicative_expression");
			additive_expression.name("additive_expression");
			shift_expression.name("shift_expression");
			relational_expression.name("relational_expression");
			equality_expression.name("equality_expression");
			and_expression.name("and_expression");
			xor_expression.name("xor_expression");
			or_expression.name("or_expression");
			logical_and_expression.name("logical_and_expression");
			logical_or_expression.name("logical_or_expression");
			ternary_expression.name("ternary_expression");
		statement.name("statement");
			compound_statement.name("compound_statement");
			expression_statement.name("expression_statement");
			selection_statement.name("selection_statement");
			iteration_statement.name("iteration_statement");
			branch_statement.name("branch_statement");
		declaration.name("declaration");
			variable_decl.name("variable_decl");
			function_decl.name("function_decl");
			typedef_decl.name("typedef_decl");
			class_decl.name("class_decl"); class_member_decl.name("class_member_decl");
			interface_decl.name("interface_decl"); member_function_decl_without_body.name("member_function_decl_without_body");
			enum_decl.name("enum_decl");
		program.name("program"); package_decl.name("package_decl"); import_decl.name("import_decl");
		start.name("start");

		if(action::ParserState::instance()->enable_debug_parser)
		{
			// keywords
			debug(_TRUE); debug(_FALSE); debug(_NULL);

			// terminals
#if 0 // NOTE: grammars cannot have debug_handlers, only rules can
			debug(IDENTIFIER);
			debug(TYPENAME);
			debug(INTEGER_LITERAL);
			debug(FLOAT_LITERAL);
			debug(STRING_LITERAL);
#endif

			// non-terminals
			debug(typed_parameter_list); debug(colon_type_specifier); debug(type_specifier); debug(template_param_identifier); debug(template_arg_identifier); debug(template_arg_specifier); debug(type_list_specifier);
				debug(storage_specifier);
				debug(visibility_specifier);
				debug(annotation_specifiers);
				debug(annotation_specifier);
				debug(nested_identifier);
			debug(expression);
				debug(primary_expression);
					debug(lambda_expression);
				debug(postfix_expression);
				debug(prefix_expression);
				debug(multiplicative_expression);
				debug(additive_expression);
				debug(shift_expression);
				debug(relational_expression);
				debug(equality_expression);
				debug(and_expression);
				debug(xor_expression);
				debug(or_expression);
				debug(logical_and_expression);
				debug(logical_or_expression);
				debug(ternary_expression);
			debug(statement);
				debug(compound_statement);
				debug(expression_statement);
				debug(selection_statement);
				debug(iteration_statement);
				debug(branch_statement);
			debug(declaration);
				debug(variable_decl);
				debug(function_decl);
				debug(typedef_decl);
				debug(class_decl); debug(class_member_decl);
				debug(interface_decl); debug(member_function_decl_without_body);
				debug(enum_decl);
			debug(program); debug(package_decl); debug(import_decl);
			debug(start);
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// Rule Declare
	///////////////////////////////////////////////////////////////////////////////

	// operators
	qi::rule<Iterator, detail::WhiteSpace<Iterator> >
		ELLIPSIS, DOT, COLON, SEMICOLON, COMMA, AT_SYMBOL, Q_MARK,
		ASSIGN, RSHIFT_ASSIGN, LSHIFT_ASSIGN, PLUS_ASSIGN, MINUS_ASSIGN, MUL_ASSIGN, DIV_ASSIGN, MOD_ASSIGN, AND_ASSIGN, OR_ASSIGN, XOR_ASSIGN,
		INCREMENT, DECREMENT,
		ARITHMETIC_PLUS, ARITHMETIC_MINUS, ARITHMETIC_MUL, ARITHMETIC_DIV, ARITHMETIC_MOD,
		BINARY_AND, BINARY_OR, BINARY_XOR, BINARY_NOT,
		RSHIFT, LSHIFT,
		LOGICAL_AND, LOGICAL_OR, LOGICAL_NOT,
		COMPARE_EQ, COMPARE_NE, COMPARE_GT, COMPARE_LT, COMPARE_GE, COMPARE_LE,
		LEFT_BRACE, RIGHT_BRACE, LEFT_BRACKET, RIGHT_BRACKET, LEFT_PAREN, RIGHT_PAREN;

	// keywords
	qi::rule<Iterator, detail::WhiteSpace<Iterator> >
		_TRUE, _FALSE, _NULL,
		CONST, STATIC,
		INT8, UINT8, INT16, UINT16, INT32, UINT32, INT64, UINT64, FLOAT32, FLOAT64, VOID,
		TYPEDEF, CLASS, INTERFACE, ENUM,
		PUBLIC, PROTECTED, PRIVATE,
		VAR, FUNCTION,
		IF, ELIF, ELSE,
		SWITCH, CASE, DEFAULT,
		FOREACH, IN, DO, WHILE,
		RETURN, BREAK, CONTINUE,
		NEW, AS, INSTANCEOF,
		PACKAGE, IMPORT,
		EXTENDS, IMPLEMENTS;

	// terminals
	detail::Identifier<Iterator, SA>     IDENTIFIER;
	detail::TypeName<Iterator, SA>       TYPENAME;
	detail::IntegerLiteral<Iterator, SA> INTEGER_LITERAL;
	detail::FloatLiteral<Iterator, SA>   FLOAT_LITERAL;
	detail::StringLiteral<Iterator, SA>  STRING_LITERAL;

	// non-terminals
	qi::rule<Iterator, detail::WhiteSpace<Iterator> >
		///////////////////////////////////////
		// BEGIN BASIC
		typed_parameter_list, /*colon_type_specifier, type_specifier,*/ template_param_identifier, /*template_arg_identifier, template_arg_specifier, type_list_specifier,*/
//			storage_specifier,
//			visibility_specifier,
//			annotation_specifiers,
//			annotation_specifier,
//			nested_identifier,
		// END BASIC
		///////////////////////////////////////
		///////////////////////////////////////
		// BEGIN EXPRESSION
//		expression,
//			primary_expression,
//				lambda_expression,
//			postfix_expression,
//			prefix_expression,
//			multiplicative_expression,
//			additive_expression,
//			shift_expression,
//			relational_expression,
//			equality_expression,
//			and_expression,
//			xor_expression,
//			or_expression,
//			logical_and_expression,
//			logical_or_expression,
//			range_expression,
//			ternary_expression,
		// END EXPRESSION
		///////////////////////////////////////
		///////////////////////////////////////
		// BEGIN STATEMENT
		statement,
			compound_statement,
			expression_statement,
			selection_statement,
			iteration_statement,
			branch_statement,
		// END STATEMENT
		///////////////////////////////////////
		///////////////////////////////////////
		// BEGIN DECLARATION
//		declaration,
//			variable_decl,
//			function_decl,
//			typedef_decl,
			/*class_decl,*/ class_member_decl,
			/*interface_decl,*/ member_function_decl_without_body;
//			enum_decl,
		// END DECLARATION
		///////////////////////////////////////
		///////////////////////////////////////
		// BEGIN MODULE
//		program, package_decl, import_decl;
		// END MODULE
		///////////////////////////////////////

	// basic
	qi::rule<Iterator, typename SA::nested_identifier::attribute_type,       detail::WhiteSpace<Iterator>, typename SA::nested_identifier::local_type>       nested_identifier;
	qi::rule<Iterator, typename SA::colon_type_specifier::attribute_type,    detail::WhiteSpace<Iterator>, typename SA::colon_type_specifier::local_type>    colon_type_specifier;
	qi::rule<Iterator, typename SA::type_specifier::attribute_type,          detail::WhiteSpace<Iterator>, typename SA::type_specifier::local_type>          type_specifier;
	qi::rule<Iterator, typename SA::template_arg_identifier::attribute_type, detail::WhiteSpace<Iterator>, typename SA::template_arg_identifier::local_type> template_arg_identifier;
	qi::rule<Iterator, typename SA::template_arg_specifier::attribute_type,  detail::WhiteSpace<Iterator>, typename SA::template_arg_specifier::local_type>  template_arg_specifier;
	qi::rule<Iterator, typename SA::type_list_specifier::attribute_type,     detail::WhiteSpace<Iterator>, typename SA::type_list_specifier::local_type>     type_list_specifier;
	qi::rule<Iterator, typename SA::storage_specifier::attribute_type,       detail::WhiteSpace<Iterator>, typename SA::storage_specifier::local_type>       storage_specifier;
	qi::rule<Iterator, typename SA::visibility_specifier::attribute_type,    detail::WhiteSpace<Iterator>, typename SA::visibility_specifier::local_type>    visibility_specifier;
	qi::rule<Iterator, typename SA::annotation_specifiers::attribute_type,   detail::WhiteSpace<Iterator>, typename SA::annotation_specifiers::local_type>   annotation_specifiers;
	qi::rule<Iterator, typename SA::annotation_specifier::attribute_type,    detail::WhiteSpace<Iterator>, typename SA::annotation_specifier::local_type>    annotation_specifier;

	// declaration
	qi::rule<Iterator, typename SA::declaration::attribute_type,    detail::WhiteSpace<Iterator>, typename SA::declaration::local_type>    declaration;
	qi::rule<Iterator, typename SA::variable_decl::attribute_type,  detail::WhiteSpace<Iterator>, typename SA::variable_decl::local_type>  variable_decl;
	qi::rule<Iterator, typename SA::function_decl::attribute_type,  detail::WhiteSpace<Iterator>, typename SA::function_decl::local_type>  function_decl;
	qi::rule<Iterator, typename SA::typedef_decl::attribute_type,   detail::WhiteSpace<Iterator>, typename SA::typedef_decl::local_type>   typedef_decl;
	qi::rule<Iterator, typename SA::class_decl::attribute_type,     detail::WhiteSpace<Iterator>, typename SA::class_decl::local_type>     class_decl;
	qi::rule<Iterator, typename SA::interface_decl::attribute_type, detail::WhiteSpace<Iterator>, typename SA::interface_decl::local_type> interface_decl;
	qi::rule<Iterator, typename SA::enum_decl::attribute_type,      detail::WhiteSpace<Iterator>, typename SA::enum_decl::local_type>      enum_decl;

	// expression
	qi::rule<Iterator, typename SA::expression::attribute_type,                detail::WhiteSpace<Iterator>, typename SA::expression::local_type>                expression;
	qi::rule<Iterator, typename SA::primary_expression::attribute_type,        detail::WhiteSpace<Iterator>, typename SA::primary_expression::local_type>        primary_expression;
	qi::rule<Iterator, typename SA::lambda_expression::attribute_type,         detail::WhiteSpace<Iterator>, typename SA::lambda_expression::local_type>         lambda_expression;
	qi::rule<Iterator, typename SA::postfix_expression::attribute_type,        detail::WhiteSpace<Iterator>, typename SA::postfix_expression::local_type>        postfix_expression;
	qi::rule<Iterator, typename SA::prefix_expression::attribute_type,         detail::WhiteSpace<Iterator>, typename SA::prefix_expression::local_type>         prefix_expression;
	qi::rule<Iterator, typename SA::multiplicative_expression::attribute_type, detail::WhiteSpace<Iterator>, typename SA::multiplicative_expression::local_type> multiplicative_expression;
	qi::rule<Iterator, typename SA::additive_expression::attribute_type,       detail::WhiteSpace<Iterator>, typename SA::additive_expression::local_type>       additive_expression;
	qi::rule<Iterator, typename SA::shift_expression::attribute_type,          detail::WhiteSpace<Iterator>, typename SA::shift_expression::local_type>          shift_expression;
	qi::rule<Iterator, typename SA::relational_expression::attribute_type,     detail::WhiteSpace<Iterator>, typename SA::relational_expression::local_type>     relational_expression;
	qi::rule<Iterator, typename SA::equality_expression::attribute_type,       detail::WhiteSpace<Iterator>, typename SA::equality_expression::local_type>       equality_expression;
	qi::rule<Iterator, typename SA::and_expression::attribute_type,            detail::WhiteSpace<Iterator>, typename SA::and_expression::local_type>            and_expression;
	qi::rule<Iterator, typename SA::xor_expression::attribute_type,            detail::WhiteSpace<Iterator>, typename SA::xor_expression::local_type>            xor_expression;
	qi::rule<Iterator, typename SA::or_expression::attribute_type,             detail::WhiteSpace<Iterator>, typename SA::or_expression::local_type>             or_expression;
	qi::rule<Iterator, typename SA::logical_and_expression::attribute_type,    detail::WhiteSpace<Iterator>, typename SA::logical_and_expression::local_type>    logical_and_expression;
	qi::rule<Iterator, typename SA::logical_or_expression::attribute_type,     detail::WhiteSpace<Iterator>, typename SA::logical_or_expression::local_type>     logical_or_expression;
	qi::rule<Iterator, typename SA::range_expression::attribute_type,          detail::WhiteSpace<Iterator>, typename SA::range_expression::local_type>          range_expression;
	qi::rule<Iterator, typename SA::ternary_expression::attribute_type,        detail::WhiteSpace<Iterator>, typename SA::ternary_expression::local_type>        ternary_expression;

	// module
	qi::rule<Iterator, typename SA::program::attribute_type,      detail::WhiteSpace<Iterator>, typename SA::program::local_type>      program;
	qi::rule<Iterator, typename SA::package_decl::attribute_type, detail::WhiteSpace<Iterator>, typename SA::package_decl::local_type> package_decl;
	qi::rule<Iterator, typename SA::import_decl::attribute_type,  detail::WhiteSpace<Iterator>, typename SA::import_decl::local_type>  import_decl;

	// start
	qi::rule<Iterator, typename SA::start::attribute_type, detail::WhiteSpace<Iterator>, typename SA::start::local_type> start;
};

} } }

#endif /* ZILLIANS_LANGUAGE_GRAMMAR_THORSCRIPT_H_ */
