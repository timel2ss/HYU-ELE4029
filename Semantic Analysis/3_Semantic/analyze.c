/****************************************************/
/* File: analyze.c                                  */
/* Semantic analyzer implementation                 */
/* for the TINY compiler                            */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "analyze.h"
#include "globals.h"
#include "symtab.h"
#include "util.h"

// Global Variables
static ScopeRec *globalScope = NULL;
static ScopeRec *currentScope = NULL;

// Error Handlers
static void RedefinitionError(char *name, int lineno, SymbolList symbol)
{
	Error = TRUE;
	fprintf(listing, "Error: Symbol \"%s\" is redefined at line %d (already defined at line ", name, lineno);
	int First = TRUE;
	while (symbol != NULL)
	{
		if (strcmp(name, symbol->name) == 0)
		{
			symbol->state = STATE_REDEFINED;
			if (symbol->node->scope != NULL) symbol->node->scope->state = STATE_REDEFINED;
			if (First != TRUE) fprintf(listing, " ");
			First = FALSE;
			fprintf(listing, "%d", symbol->lineList->lineno);
		}
		symbol = symbol->next;
	}
	fprintf(listing, ")\n");
}

static SymbolRec *UndeclaredFunctionError(ScopeRec *currentScope, TreeNode *node)
{
	fprintf(listing, "Error: undeclared function \"%s\" is called at line %d\n", node->name, node->lineno);
	Error = TRUE;
	return insertSymbol(currentScope, node->name, Undetermined, FunctionSym, node->lineno, NULL);
}

static SymbolRec *UndeclaredVariableError(ScopeRec *currentScope, TreeNode *node)
{
	Error = TRUE;
	fprintf(listing, "Error: undeclared variable \"%s\" is used at line %d\n", node->name, node->lineno);
	return insertSymbol(currentScope, node->name, Undetermined, VariableSym, node->lineno, NULL);
}

static void VoidTypeVariableError(char *name, int lineno)
{
	fprintf(listing, "Error: The void-type variable is declared at line %d (name : \"%s\")\n", lineno, name);
	Error = TRUE;
}

static void ArrayIndexingError(char *name, int lineno)
{
	fprintf(listing, "Error: Invalid array indexing at line %d (name : \"%s\"). indicies should be integer\n", lineno, name);
	Error = TRUE;
}

static void ArrayIndexingError2(char *name, int lineno)
{
	fprintf(listing, "Error: Invalid array indexing at line %d (name : \"%s\"). indexing can only allowed for int[] variables\n", lineno, name);
	Error = TRUE;
}

static void InvalidFunctionCallError(char *name, int lineno)
{
	fprintf(listing, "Error: Invalid function call at line %d (name : \"%s\")\n", lineno, name);
	Error = TRUE;
}

static void InvalidReturnError(int lineno)
{
	fprintf(listing, "Error: Invalid return at line %d\n", lineno);
	Error = TRUE;
}

static void InvalidAssignmentError(int lineno)
{
	fprintf(listing, "Error: invalid assignment at line %d\n", lineno);
	Error = TRUE;
}

static void InvalidOperationError(int lineno)
{
	fprintf(listing, "Error: invalid operation at line %d\n", lineno);
	Error = TRUE;
}

static void InvalidConditionError(int lineno)
{
	fprintf(listing, "Error: invalid condition at line %d\n", lineno);
	Error = TRUE;
}


/* Procedure traverse is a generic recursive
 * syntax tree traversal routine:
 * it applies preProc in preorder and postProc
 * in postorder to tree pointed to by t
 */
static void traverse(TreeNode *t, void (*preProc)(TreeNode *), void (*postProc)(TreeNode *))
{
	if (t != NULL)
	{
		// pre-order process
		preProc(t);
		// traverse childs
		{
			int i;
			for (i = 0; i < MAXCHILDREN; i++) traverse(t->child[i], preProc, postProc);
		}

		// post-order process
		postProc(t);

		// traverse siblings
		traverse(t->sibling, preProc, postProc);
	}
}

// nullProc: do-nothing
static void nullProc(TreeNode *t)
{
	if (t == NULL) return;
	else
		return;
}
// scopeIn: preprocess traverse functions to scope-in
static void scopeIn(TreeNode *t)
{
	if (t->scope != NULL) currentScope = t->scope;
}
// scopeOut: postprocess traverse functions to scope-out
static void scopeOut(TreeNode *t)
{
	if (t->scope != NULL) currentScope = t->scope->parent;
}

static void insertNode(TreeNode *t)
{
	switch (t->kind)
	{
		// Variable Declaration
		case VariableDecl:
		{
			// Semantic Error: Void-Type Variables
			if (t->type == Void || t->type == VoidArray) VoidTypeVariableError(t->name, t->lineno);
			// Semantic Error: Redefined Variables
			SymbolRec *symbol = lookupSymbolInCurrentScope(currentScope, t->name);
			if (symbol != NULL) RedefinitionError(t->name, t->lineno, symbol);
			// Insert New Variable Symbol to Symbol Table
			insertSymbol(currentScope, t->name, t->type, VariableSym, t->lineno, t);
			// Break
			break;
		}
		// Function Declaration
		case FunctionDecl:
		{
			// Error Check: currentScope is not global
			ERROR_CHECK(currentScope == globalScope);
			// Semantic Error: Redefined Variables
			SymbolRec *symbol = lookupSymbolInCurrentScope(globalScope, t->name);
			if (symbol != NULL) RedefinitionError(t->name, t->lineno, symbol);
			// Insert New Function Symbol to Symbol Table
			insertSymbol(currentScope, t->name, t->type, FunctionSym, t->lineno, t);
			// Change Current Scope
			currentScope = t->scope = insertScope(t->name, currentScope, t);
			// Break
			break;
		}
		// Parameters
		case Params:
		{
			// Void Parameters: Do Nothing
			if (t->flag == TRUE) break;

            // Semantic Error: Void-Type Parameters
			// Semantic Error: Redefined Variables
			// Insert New Variable Symbol to Symbol Table
            // *********************Fill the Code*************************
            // Semantic Error: Void-Type Parameters
            if (t->type == Void || t->type == VoidArray) {
                VoidTypeVariableError(t->name, t->lineno);
                break;
            }

            // Semantic Error: Redefined Variables
            SymbolRec *symbol = lookupSymbolInCurrentScope(currentScope, t->name);
            if (symbol != NULL) {
                RedefinitionError(t->name, t->lineno, symbol);
                break;
            }

            // Insert New Variable Symbol to Symbol Table
            insertSymbol(currentScope, t->name, t->type, VariableSym, t->lineno, t);
			// ************************************************************/
			
			// Break
			break;
		}
		// Compound Statements
		case CompoundStmt:
		{
			// Insert New Scope If The Compound Statement is not for Function Body
			if (t->flag != TRUE) t->scope = currentScope = insertScope(NULL, currentScope, currentScope->func);
			// Break
			break;
		}
		// Call Function
		case CallExpr:
		{
			// Semantic Error: Undeclared Functions
			SymbolRec *func = lookupSymbolWithKind(globalScope, t->name, FunctionSym);
			if (func == NULL) func = UndeclaredFunctionError(globalScope, t);
			// Update Symbol Table Entry
			else
				appendSymbol(globalScope, t->name, t->lineno);
			// Break
			break;
		}
		// Variable Access
		case VarAccessExpr:
		{
			// Semantic Error: Undeclared Variables
			// Update Symbol Table Entry
			// *********************Fill the Code*************************
            // Semantic Error: Undeclared Variables
            SymbolRec *var = lookupSymbolWithKind(currentScope, t->name, VariableSym);
            if (var == NULL) {
                var = UndeclaredVariableError(currentScope, t);
                break;
            }
            // Update Symbol Table Entry
            t->type = var->type;
            appendSymbol(currentScope, t->name, t->lineno);
			// ************************************************************/
			
			// Break
			break;
		}
		// If/If-Else, While, Return Statements
		// Assign, Binary Operator, Constant Expression
		case IfStmt:
		case WhileStmt:
		case ReturnStmt:
		case AssignExpr:
		case BinOpExpr:
		case ConstExpr:
			// Do Nothing
			break;
		default: fprintf(stderr, "[%s:%d] Undefined Error Occurs\n", __FILE__, __LINE__); exit(-1);
	}
}

void declareBuiltInFunction(void)
{
	TreeNode *inputFuncNode = newTreeNode(FunctionDecl);
	inputFuncNode->lineno = 0;
	inputFuncNode->type = Integer;
	inputFuncNode->name = copyString("input");
	inputFuncNode->child[0] = newTreeNode(Params);
	inputFuncNode->child[0]->lineno = 0;
	inputFuncNode->child[0]->type = Void;
	inputFuncNode->child[0]->flag = TRUE;

	TreeNode *outputFuncNode = newTreeNode(FunctionDecl);
	outputFuncNode->lineno = 0;
	outputFuncNode->type = Void;
	outputFuncNode->name = copyString("output");
	TreeNode *outputFuncParamNode = newTreeNode(Params);
	outputFuncParamNode->lineno = 0;
	outputFuncParamNode->type = Integer;
	outputFuncParamNode->name = copyString("value");
	outputFuncNode->child[0] = outputFuncParamNode;

	insertSymbol(globalScope, inputFuncNode->name, inputFuncNode->type, FunctionSym, inputFuncNode->lineno, inputFuncNode);
	insertSymbol(globalScope, outputFuncNode->name, outputFuncNode->type, FunctionSym, outputFuncNode->lineno, outputFuncNode);
	ScopeRec *outputFuncScope = insertScope("output", globalScope, outputFuncNode);
	insertSymbol(
		outputFuncScope, outputFuncParamNode->name, outputFuncParamNode->type, VariableSym, outputFuncParamNode->lineno, outputFuncParamNode);
}

void buildSymtab(TreeNode *syntaxTree)
{
	// Initialize Global Variables
	globalScope = insertScope("global", NULL, NULL);
	currentScope = globalScope;

	declareBuiltInFunction();

	// insert node all
	traverse(syntaxTree, insertNode, scopeOut);

	// trace
	if (TraceAnalyze)
	{
		fprintf(listing, "\n\n");
		fprintf(listing, "< Symbol Table >\n");
		printSymbolTable(listing);

		fprintf(listing, "\n\n");
		fprintf(listing, "< Functions >\n");
		printFunction(listing);

		fprintf(listing, "\n\n");
		fprintf(listing, "< Global Symbols >\n");
		printGlobal(listing, globalScope);

		fprintf(listing, "\n\n");
		fprintf(listing, "< Scopes >\n");
		printScope(listing, globalScope);
	}
}

static void checkNode(TreeNode *t)
{
	switch (t->kind)
	{
		// If/If-Else, While Statement
		case IfStmt:
		case WhileStmt:
		{
			// Error Check
			ERROR_CHECK(t->child[0] != NULL);
			// Semantic Error: Invalid Condition in If/If-Else, While Statement
			// *********************Fill the Code*************************
            TreeNode *condition = t->child[0];
            if (condition->type != Integer) {
                InvalidConditionError(t->lineno);
            }
			// ************************************************************/
			
			// Break
			break;
		}
		// Return Statement
		case ReturnStmt:
		{
			// Error Check
			ERROR_CHECK(currentScope->func != NULL);
			// Semantic Error: Invalid Return
			// *********************Fill the Code*************************
			TreeNode *function = currentScope->func;
            if (t->child[0]) {
                if (function->type != t->child[0]->type) {
                    InvalidReturnError(t->lineno);
                    break;
                }
            }
            else {
                if (function->type != Void && function->type != VoidArray) {
                    InvalidReturnError(t->lineno);
                    break;
                }
            }
			// ************************************************************/
			// Break
			break;
		}
		// Assignment, Binary Operator Expression
		case AssignExpr:
		case BinOpExpr:
		{
			// Error Check
			ERROR_CHECK(t->child[0] != NULL && t->child[1] != NULL);
			// Semantic Error: Invalid Assignment / Operation
			// *********************Fill the Code*************************
            TreeNode *lhs = t->child[0];
            TreeNode *rhs = t->child[1];
            if (lhs->type != rhs->type) {
                if (t->kind == AssignExpr) {
                    InvalidAssignmentError(t->lineno);
                    break;
                }
                if (t->kind == BinOpExpr) {
                    InvalidOperationError(t->lineno);
                    break;
                }
            }
			// ************************************************************/
			// Update Node Type
			t->type = t->child[0]->type;
			// Break
			break;
		}
		// Call Expression
		case CallExpr:
		{
			SymbolRec *calleeSymbol = lookupSymbolWithKind(globalScope, t->name, FunctionSym);
			// Error Check
			ERROR_CHECK(calleeSymbol != NULL);
			// Semantic Error: Call Undeclared Function - Already Caused
			if (calleeSymbol->state == STATE_UNDECLARED)
			{
				t->type = calleeSymbol->type;
				break;
			}
			// Semantic Error: Invalid Arguments
			TreeNode *paramNode = calleeSymbol->node->child[0];
			TreeNode *argNode = t->child[0];
			// *********************Fill the Code*************************
            int flag = TRUE;
            while (paramNode && paramNode->type != Void) {
                if (!argNode) {
                    InvalidFunctionCallError(t->name, t->lineno);
                    flag = FALSE;
                    break;
                }
                if (paramNode->type != argNode->type) {
                    InvalidFunctionCallError(t->name, t->lineno);
                    flag = FALSE;
                    break;
                }
                paramNode = paramNode->sibling;
                argNode = argNode->sibling;
            }

            if (flag && argNode) {
                InvalidFunctionCallError(t->name, t->lineno);
            }
			// ************************************************************/
			// Update Node Type
			t->type = calleeSymbol->type;
			// Break
			break;
		}
		// Variable Access
		case VarAccessExpr:
		{
			SymbolRec *symbol = lookupSymbolWithKind(currentScope, t->name, VariableSym);
			// Error Check
			ERROR_CHECK(symbol != NULL);
			// Semantic Error: Access Undeclared Variable - Already Caused
			if (symbol->state == STATE_UNDECLARED)
			{
				t->type = symbol->type;
				break;
			}
			// Array Access or Not
			if (t->child[0] != NULL)
			{
				// Semantic Error: Index to Not Array				
				// Semantic Error: Index is not Integer in Array Indexing
				// *********************Fill the Code*************************
                if (t->type != IntegerArray) {
                    ArrayIndexingError2(t->name, t->lineno);
                    break;
                }
                TreeNode *index = t->child[0];
                if (index->type != Integer) {
                    ArrayIndexingError(t->name, t->lineno);
                    break;
                }
				// ************************************************************/
				// Update Node Type
				t->type = Integer;
			}
			// Update Node Type
			else
				t->type = symbol->type;
			// Break
			break;
		}
		// Constant Expression
		case ConstExpr:
		{
			// Update Node Type
			t->type = Integer;
			// Break
			break;
		}
		// Variable Declaration, Function Declaration, Compound Statement, Parameters
		case FunctionDecl:
		case VariableDecl:
		case Params:
		case CompoundStmt:
			// Do Nothing
			break;
		default: fprintf(stderr, "[%s:%d] Undefined Error Occurs\n", __FILE__, __LINE__); exit(-1);
	}
}

void typeCheck(TreeNode *syntaxTree) {
	traverse(syntaxTree, scopeIn, checkNode);
}
