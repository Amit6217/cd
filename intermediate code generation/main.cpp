#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <cctype>
#include <stack>
#include <iomanip>
#include <algorithm>

using namespace std;

// Data Structure for Quadruple: (op, arg1, arg2, result)
struct Quadruple {
    int index;
    string op;
    string arg1;
    string arg2;
    string result;
};

// Data Structure for Triple: (op, arg1, arg2)
struct Triple {
    int index;
    string op;
    string arg1;
    string arg2;
};

// Data Structure for Three-Address Code (TAC)
struct TAC {
    string code;
};

class IntermediateCodeGenerator {
public:
    vector<TAC> tacList;
    vector<Quadruple> quadList;
    vector<Triple> tripleList;
    vector<int> indirectPointers;

    int tempCount = 1;

    void clear() {
        tacList.clear();
        quadList.clear();
        tripleList.clear();
        indirectPointers.clear();
        tempCount = 1;
    }

    // Check operator precedence
    int precedence(const string& op) {
        if (op == "uminus") return 3;
        if (op == "*" || op == "/") return 2;
        if (op == "+" || op == "-") return 1;
        return 0;
    }

    bool isOperator(char c) {
        return c == '+' || c == '-' || c == '*' || c == '/';
    }

    // Tokenize expression and distinguish binary vs unary minus
    vector<string> tokenize(const string& expr) {
        vector<string> tokens;
        string current = "";

        for (size_t i = 0; i < expr.length(); ++i) {
            char c = expr[i];
            if (isspace(c)) continue;

            if (isalnum(c) || c == '_') {
                current += c;
            } else {
                if (!current.empty()) {
                    tokens.push_back(current);
                    current = "";
                }
                if (c == '-') {
                    // Check if unary minus
                    if (tokens.empty() || tokens.back() == "(" || tokens.back() == "=" ||
                        tokens.back() == "+" || tokens.back() == "-" || tokens.back() == "*" || tokens.back() == "/") {
                        tokens.push_back("uminus");
                    } else {
                        tokens.push_back("-");
                    }
                } else {
                    tokens.push_back(string(1, c));
                }
            }
        }
        if (!current.empty()) tokens.push_back(current);
        return tokens;
    }

    // Convert infix token list to postfix (Shunting-Yard algorithm)
    vector<string> infixToPostfix(const vector<string>& tokens) {
        vector<string> postfix;
        stack<string> opStack;

        for (const auto& tok : tokens) {
            if (tok == "(") {
                opStack.push(tok);
            } else if (tok == ")") {
                while (!opStack.empty() && opStack.top() != "(") {
                    postfix.push_back(opStack.top());
                    opStack.pop();
                }
                if (!opStack.empty()) opStack.pop(); // Pop '('
            } else if (tok == "uminus" || tok == "+" || tok == "-" || tok == "*" || tok == "/") {
                while (!opStack.empty() && opStack.top() != "(" &&
                       (precedence(opStack.top()) > precedence(tok) ||
                        (precedence(opStack.top()) == precedence(tok) && tok != "uminus"))) {
                    postfix.push_back(opStack.top());
                    opStack.pop();
                }
                opStack.push(tok);
            } else {
                postfix.push_back(tok); // Operand
            }
        }

        while (!opStack.empty()) {
            postfix.push_back(opStack.top());
            opStack.pop();
        }
        return postfix;
    }

    // Generate TAC, Quadruples, Triples, and Indirect Triples
    void generate(const string& expr) {
        clear();
        string lhs = "";
        string rhs = expr;

        size_t eqPos = expr.find('=');
        if (eqPos != string::npos) {
            lhs = expr.substr(0, eqPos);
            rhs = expr.substr(eqPos + 1);
            // Trim whitespace
            auto trim = [](string& s) {
                while (!s.empty() && isspace(s.front())) s.erase(s.begin());
                while (!s.empty() && isspace(s.back())) s.pop_back();
            };
            trim(lhs);
            trim(rhs);
        }

        vector<string> tokens = tokenize(rhs);
        vector<string> postfix = infixToPostfix(tokens);

        // Stacks for code generation: (value/temp name, triple reference)
        stack<string> valStack;
        stack<string> tripleStack;

        int stepIdx = 0;

        for (const auto& tok : postfix) {
            if (tok == "uminus") {
                string arg1 = valStack.top(); valStack.pop();
                string tArg1 = tripleStack.top(); tripleStack.pop();

                string temp = "t" + to_string(tempCount++);
                string tripRef = "(" + to_string(stepIdx) + ")";

                // TAC
                tacList.push_back({ temp + " = - " + arg1 });
                // Quadruple: (uminus, arg1, -, temp)
                quadList.push_back({ stepIdx, "uminus", arg1, "-", temp });
                // Triple: (uminus, tArg1, -)
                tripleList.push_back({ stepIdx, "uminus", tArg1, "-" });
                indirectPointers.push_back(stepIdx);

                valStack.push(temp);
                tripleStack.push(tripRef);
                stepIdx++;
            } else if (tok == "+" || tok == "-" || tok == "*" || tok == "/") {
                string arg2 = valStack.top(); valStack.pop();
                string arg1 = valStack.top(); valStack.pop();
                string tArg2 = tripleStack.top(); tripleStack.pop();
                string tArg1 = tripleStack.top(); tripleStack.pop();

                string temp = "t" + to_string(tempCount++);
                string tripRef = "(" + to_string(stepIdx) + ")";

                // TAC
                tacList.push_back({ temp + " = " + arg1 + " " + tok + " " + arg2 });
                // Quadruple: (op, arg1, arg2, temp)
                quadList.push_back({ stepIdx, tok, arg1, arg2, temp });
                // Triple: (op, tArg1, tArg2)
                tripleList.push_back({ stepIdx, tok, tArg1, tArg2 });
                indirectPointers.push_back(stepIdx);

                valStack.push(temp);
                tripleStack.push(tripRef);
                stepIdx++;
            } else {
                // Operand
                valStack.push(tok);
                tripleStack.push(tok);
            }
        }

        // Final Assignment
        if (!lhs.empty() && !valStack.empty()) {
            string finalVal = valStack.top();
            string finalTrip = tripleStack.top();

            tacList.push_back({ lhs + " = " + finalVal });
            quadList.push_back({ stepIdx, "=", finalVal, "-", lhs });
            tripleList.push_back({ stepIdx, "=", lhs, finalTrip });
            indirectPointers.push_back(stepIdx);
        }
    }

    // Print Three-Address Code (TAC)
    void printTAC() const {
        cout << "\n[1] THREE-ADDRESS CODE (TAC):\n";
        cout << "-------------------------------------------\n";
        for (size_t i = 0; i < tacList.size(); ++i) {
            cout << "  (" << i + 1 << ")  " << tacList[i].code << "\n";
        }
    }

    // Print Quadruple Table
    void printQuadruples() const {
        cout << "\n[2] QUADRUPLE TABLE:\n";
        cout << "-----------------------------------------------------------\n";
        cout << left << setw(10) << "Index" 
             << setw(12) << "Op" 
             << setw(14) << "Arg1" 
             << setw(14) << "Arg2" 
             << setw(14) << "Result" << "\n";
        cout << "-----------------------------------------------------------\n";
        for (const auto& q : quadList) {
            cout << left << setw(10) << ("(" + to_string(q.index) + ")")
                 << setw(12) << q.op 
                 << setw(14) << q.arg1 
                 << setw(14) << q.arg2 
                 << setw(14) << q.result << "\n";
        }
    }

    // Print Triple Table
    void printTriples() const {
        cout << "\n[3] TRIPLE TABLE:\n";
        cout << "---------------------------------------------------\n";
        cout << left << setw(10) << "Index" 
             << setw(12) << "Op" 
             << setw(16) << "Arg1" 
             << setw(16) << "Arg2" << "\n";
        cout << "---------------------------------------------------\n";
        for (const auto& t : tripleList) {
            cout << left << setw(10) << ("(" + to_string(t.index) + ")")
                 << setw(12) << t.op 
                 << setw(16) << t.arg1 
                 << setw(16) << t.arg2 << "\n";
        }
    }

    // Print Indirect Triple Table
    void printIndirectTriples() const {
        cout << "\n[4] INDIRECT TRIPLE TABLE:\n";
        cout << "-------------------------------------\n";
        cout << "  Statement Pointers Table:\n";
        cout << "  " << left << setw(12) << "Statement" << setw(16) << "Triple Address" << "\n";
        cout << "  -----------------------------------\n";
        for (size_t i = 0; i < indirectPointers.size(); ++i) {
            cout << "  " << left << setw(12) << ("[" + to_string(i) + "]")
                 << setw(16) << ("(" + to_string(indirectPointers[i]) + ")") << "\n";
        }

        cout << "\n  Indirect Triples Table:\n";
        cout << "  ---------------------------------------------------\n";
        cout << "  " << left << setw(10) << "Address" 
             << setw(12) << "Op" 
             << setw(16) << "Arg1" 
             << setw(16) << "Arg2" << "\n";
        cout << "  ---------------------------------------------------\n";
        for (const auto& t : tripleList) {
            cout << "  " << left << setw(10) << ("(" + to_string(t.index) + ")")
                 << setw(12) << t.op 
                 << setw(16) << t.arg1 
                 << setw(16) << t.arg2 << "\n";
        }
    }

    void displayAll(const string& expr) {
        cout << "\n======================================================================\n";
        cout << " SOURCE EXPRESSION: " << expr << "\n";
        cout << "======================================================================\n";
        generate(expr);
        printTAC();
        printQuadruples();
        printTriples();
        printIndirectTriples();
    }
};

int main() {
    cout << "======================================================================\n";
    cout << "             INTERMEDIATE CODE GENERATION (ICG) SUITE                 \n";
    cout << "======================================================================\n";

    IntermediateCodeGenerator icg;

    // Expression 1: Classic arithmetic with parentheses
    icg.displayAll("x = (a + b) * (c - d)");

    // Expression 2: Expression with unary minus and addition
    icg.displayAll("a = -b * (c + d)");

    // Expression 3: Expression with multiple operator precedence
    icg.displayAll("w = a + b * c / d - e");

    cout << "\n======================================================================\n";
    cout << "            INTERMEDIATE CODE GENERATED SUCCESSFULLY                  \n";
    cout << "======================================================================\n";
    return 0;
}
