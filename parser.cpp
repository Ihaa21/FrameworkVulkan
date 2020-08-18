
//
// NOTE: Lexer
//

inline void LexExpression(linear_arena* Arena)
{
    char* Expression = "4*(3-2) + 5";

    char* Source = Expression;
}

//
// NOTE: Parser
//

// IMPORTANT: We need these to appear in token definition order
static const parse_rule ParseRuleTable[TokenId_Count] =
{
    { TokenId_None, 0xFF, 0, 0 },
    { TokenId_Root, 0, 0, 0 },
    { TokenId_OpenBracket, 1, ParseRuleFlag_SkipPrecedenceClimb, 0 },
    { TokenId_CloseBracket, 1, ParseRuleFlag_RightAssociative, 0 },
    { TokenId_Add, 2, 0, 0 },
    { TokenId_Subtract, 2, 0, 0 },
    { TokenId_Positive, 3, ParseRuleFlag_RightAssociative, 0 },
    { TokenId_Negative, 3, ParseRuleFlag_RightAssociative, 0 },
    { TokenId_Multiply, 4, 0, 0 },
    { TokenId_Divide, 4, 0, 0 },
    { TokenId_Number, 10, 0, 0 },
};

inline u32 ParseGetNodePrecedence(expression_node* Node)
{
    u32 Result = ParseRuleTable[Node->TokenId].Precedence;
    return Result;
}

#define OPTIMIZED_LEXER 0

inline void ParseExpression(linear_arena* ParentArena)
{
    // NOTE: https://www.rhyscitlema.com/algorithms/expression-parsing-algorithm/#:~:text=%7C%20Algorithms%20%7C%20Fun%20%7C-,Expression%20parsing%20algorithm,the%20order%20of%20operations%20matters.
    linear_arena Arena = LinearSubArena(ParentArena, KiloBytes(500));
    //string Expression = String("5 - 6/2 + 3*4");
    string Expression = String("4*(3 - 2) + 5");
    //string Expression = String("4+-2");

    // NOTE: Setup loop invariants
    string CurrChar = Expression;

    expression_node* Root = PushStruct(&Arena, expression_node);
    *Root = {};
    Root->TokenId = TokenId_Root;
    expression_node* CurrNode = Root;

    token_id PrevToken = TokenId_Root;
    
    while (CurrChar.NumChars != 0)
    {
        // NOTE: Get our node from the current char + its properties
        u32 NewNodePrecedence = 0;
        u32 NewNodeFlags = 0;
        expression_node NewNode = {};
        {            
            token_id CurrToken = {};

#if OPTIMIZED_LEXER
            // TODO: Come back to this, right now I left it alone since the post wasn't super detailed and I haven't thought too much
            // about it yet. Come back to this once I add function names, and more complex stuff to the parser for actual code!
            // NOTE: https://nothings.org/computer/lexing.html
            // NOTE: https://steemit.com/programming/@drifter1/writing-a-simple-compiler-on-my-own-using-symbol-tables-in-the-lexer
            lexer_state State = LexerState_Start;
            u32 TokenLength = 0;
            do
            {
                char CurrChar = *SourcePtr++;
                lexer_equiv_class EquivClass = EquivClassTable[CurrChar];
                State = Transition[State][EquivClass];
                TokenLength += InToken[State];
            } while (state < LexerState_End);

            // TODO: Move this only to the token states that care
            char* TokenStart = SourcePtr - TokenLength;
#else
            
            // TODO: This whole section should become a hash
            if (IsUInt(CurrChar.Chars[0]))
            {
                ReadUIntAndAdvance(&CurrChar, &NewNode.Value);
                CurrToken = TokenId_Number;
            }
            else if (IsLetter(CurrChar.Chars[0]))
            {
                InvalidCodePath;
            }
            else
            {
                switch (CurrChar.Chars[0])
                {
                    case '(':
                    {
                        CurrToken = TokenId_OpenBracket;
                    } break;

                    case ')':
                    {
                        CurrToken = TokenId_CloseBracket;
                    } break;

                    case '+':
                    {
                        if ((PrevToken == TokenId_Number || PrevToken == TokenId_CloseBracket))
                        {
                            CurrToken = TokenId_Add;
                        }
                        else
                        {
                            CurrToken = TokenId_Positive;
                        }
                    } break;

                    case '-':
                    {
                        if ((PrevToken == TokenId_Number || PrevToken == TokenId_CloseBracket))
                        {
                            CurrToken = TokenId_Subtract;
                        }
                        else
                        {
                            CurrToken = TokenId_Negative;
                        }
                    } break;

                    case '*':
                    {
                        CurrToken = TokenId_Multiply;
                    } break;

                    case '/':
                    {
                        CurrToken = TokenId_Divide;
                    } break;
                }
                
                AdvanceString(&CurrChar, 1);
            }

            AdvancePastSpaces(&CurrChar);
#endif
            
            const parse_rule* Rule = ParseRuleTable + CurrToken;
            NewNodePrecedence = Rule->Precedence;
            NewNodeFlags = Rule->Flags;
            NewNode.TokenId = CurrToken;

            PrevToken = CurrToken;
        }
        
        // NOTE: Climb up the tree as long to get correct location with precedence
        NewNodePrecedence += (NewNodeFlags & ParseRuleFlag_RightAssociative) ? 1 : 0; // NOTE: This will set the strictly greater behavior we need
        while ((NewNodeFlags & ParseRuleFlag_SkipPrecedenceClimb) == 0 &&
               CurrNode &&
               ParseRuleTable[CurrNode->TokenId].Precedence >= NewNodePrecedence)
        {
            CurrNode = CurrNode->Parent;
        }
        
        if (NewNode.TokenId == TokenId_CloseBracket)
        {
            // NOTE: Remove the bracket nodes
            Assert(CurrNode->TokenId == TokenId_OpenBracket);
            expression_node* Parent = CurrNode->Parent;
            Parent->Right = CurrNode->Right;
            if (CurrNode->Right)
            {
                CurrNode->Right->Parent = Parent;
            }

            // TODO: Delete CurrNode

            // NOTE: Set the current node to parent of the brackets
            CurrNode = Parent;
        }
        else
        {
            // NOTE: Link the node to the tree so that the left child of the new node is the old right child of the curr node
            expression_node* NewNodePtr = PushStruct(&Arena, expression_node);
            *NewNodePtr = NewNode;
            
            NewNodePtr->Left = CurrNode->Right;
            if (CurrNode->Right)
            {
                CurrNode->Right->Parent = NewNodePtr;
            }
            CurrNode->Right = NewNodePtr;
            NewNodePtr->Parent = CurrNode;

            // NOTE: Set the current node to be the new node
            CurrNode = NewNodePtr;
        }
    }
    
    int i = 0;
}
