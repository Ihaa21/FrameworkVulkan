#pragma once

//
// NOTE: Lexer
//

enum lexer_equiv_class : u32
{
};

enum lexer_state : u32
{
    LexerState_Start,

    LexerState_End,
};


//
// NOTE: Parser
//

enum parse_rule_flags
{
    ParseRuleFlag_RightAssociative = 1 << 0,
    ParseRuleFlag_SkipPrecedenceClimb = 1 << 1,
};

enum token_id : u8
{
    TokenId_None = 0,
    
    TokenId_Root,
    TokenId_OpenBracket,
    TokenId_CloseBracket,
    TokenId_Add,
    TokenId_Subtract,
    TokenId_Positive,
    TokenId_Negative,
    TokenId_Multiply,
    TokenId_Divide,
    TokenId_Number,

    TokenId_Count,
};

struct parse_rule
{
    token_id TokenId;
    u8 Precedence;
    u8 Flags;
    u8 Pad;
};

struct expression_node
{
    // NOTE: If expression is 0, this is a number node
    token_id TokenId;
    u32 Value;

    // TODO: Remake to ids
    expression_node* Parent;
    expression_node* Left;
    expression_node* Right;
};
