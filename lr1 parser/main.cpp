#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <map>
#include <iomanip>
#include <algorithm>

using namespace std;

const string END_MARKER = "$";

struct Production {
    int id;
    string lhs;
    vector<string> rhs;

    string toString() const {
        string res = lhs + " -> ";
        if (rhs.empty()) res += "#";
        else {
            for (size_t i = 0; i < rhs.size(); ++i) {
                res += rhs[i] + (i + 1 < rhs.size() ? " " : "");
            }
        }
        return res;
    }
};

// LR(1) Item: [A -> alpha . beta, lookahead]
struct Item {
    int prodId;
    int dot;
    string lookahead;

    bool operator<(const Item& o) const {
        if (prodId != o.prodId) return prodId < o.prodId;
        if (dot != o.dot) return dot < o.dot;
        return lookahead < o.lookahead;
    }
    bool operator==(const Item& o) const {
        return prodId == o.prodId && dot == o.dot && lookahead == o.lookahead;
    }
};

class LR1Parser {
public:
    string startSymbol;
    string augmentedStart;
    vector<Production> prods;
    set<string> nonTerminals;
    set<string> terminals;
    map<string, set<string>> first;

    vector<set<Item>> canonicalStates;
    map<pair<int, string>, string> actionTable; // (state, terminal) -> "s2", "r1", "acc"
    map<pair<int, string>, int> gotoTable;      // (state, NT) -> state

    LR1Parser(const string& start) : startSymbol(start) {
        augmentedStart = start + "'";
        nonTerminals.insert(augmentedStart);
        nonTerminals.insert(startSymbol);
        // Rule 0: Augmented production S' -> S
        prods.push_back({ 0, augmentedStart, { startSymbol } });
    }

    void addProduction(const string& lhs, const vector<string>& rhs) {
        nonTerminals.insert(lhs);
        for (const auto& s : rhs) {
            if (s != "#" && !isupper(s[0])) terminals.insert(s);
            else if (isupper(s[0])) nonTerminals.insert(s);
        }
        prods.push_back({ (int)prods.size(), lhs, rhs });
    }

    bool isNT(const string& s) const { return nonTerminals.count(s) > 0; }

    // Compute FIRST sets for all symbols
    void computeFirst() {
        for (const auto& t : terminals) first[t] = { t };
        first[END_MARKER] = { END_MARKER };

        bool changed = true;
        while (changed) {
            changed = false;
            for (const auto& p : prods) {
                if (p.id == 0) continue;
                bool allEps = true;
                for (const auto& sym : p.rhs) {
                    for (const auto& f : first[sym]) {
                        if (f != "#" && first[p.lhs].insert(f).second) changed = true;
                    }
                    if (!first[sym].count("#")) {
                        allEps = false;
                        break;
                    }
                }
                if (allEps && first[p.lhs].insert("#").second) changed = true;
            }
        }
    }

    set<string> firstOfString(const vector<string>& symbols, const string& lookahead) {
        set<string> res;
        bool allEps = true;
        for (const auto& sym : symbols) {
            for (const auto& f : first[sym]) {
                if (f != "#") res.insert(f);
            }
            if (!first[sym].count("#")) {
                allEps = false;
                break;
            }
        }
        if (allEps) res.insert(lookahead);
        return res;
    }

    // Sub-Task 1: Construct LR(1) Items & CLOSURE Function
    set<Item> closure(const set<Item>& I) {
        set<Item> J = I;
        bool changed = true;
        while (changed) {
            changed = false;
            set<Item> toAdd;
            for (const auto& it : J) {
                const auto& p = prods[it.prodId];
                if (it.dot < (int)p.rhs.size()) {
                    string B = p.rhs[it.dot];
                    if (isNT(B)) {
                        // beta is symbols after B: p.rhs[dot + 1 ...]
                        vector<string> beta(p.rhs.begin() + it.dot + 1, p.rhs.end());
                        set<string> lookaheads = firstOfString(beta, it.lookahead);

                        for (const auto& prod : prods) {
                            if (prod.lhs == B) {
                                for (const auto& b : lookaheads) {
                                    Item newItem = { prod.id, 0, b };
                                    if (!J.count(newItem) && !toAdd.count(newItem)) {
                                        toAdd.insert(newItem);
                                        changed = true;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            J.insert(toAdd.begin(), toAdd.end());
        }
        return J;
    }

    // GOTO Function: GOTO(I, X)
    set<Item> goTo(const set<Item>& I, const string& X) {
        set<Item> nextItems;
        for (const auto& it : I) {
            const auto& p = prods[it.prodId];
            if (it.dot < (int)p.rhs.size() && p.rhs[it.dot] == X) {
                nextItems.insert({ it.prodId, it.dot + 1, it.lookahead });
            }
        }
        return closure(nextItems);
    }

    // Sub-Task 2: Generate Canonical Collection of LR(1) Items
    void generateCanonicalCollection() {
        computeFirst();
        Item initialItem = { 0, 0, END_MARKER }; // [S' -> .S, $]
        set<Item> I0 = closure({ initialItem });
        canonicalStates.push_back(I0);

        vector<string> allSymbols;
        allSymbols.insert(allSymbols.end(), terminals.begin(), terminals.end());
        for (const auto& nt : nonTerminals) {
            if (nt != augmentedStart) allSymbols.push_back(nt);
        }

        bool changed = true;
        while (changed) {
            changed = false;
            for (size_t i = 0; i < canonicalStates.size(); ++i) {
                for (const auto& X : allSymbols) {
                    set<Item> target = goTo(canonicalStates[i], X);
                    if (target.empty()) continue;

                    int targetIdx = -1;
                    for (size_t k = 0; k < canonicalStates.size(); ++k) {
                        if (canonicalStates[k] == target) {
                            targetIdx = (int)k;
                            break;
                        }
                    }

                    if (targetIdx == -1) {
                        targetIdx = (int)canonicalStates.size();
                        canonicalStates.push_back(target);
                        changed = true;
                    }

                    if (isNT(X)) gotoTable[{ (int)i, X }] = targetIdx;
                    else actionTable[{ (int)i, X }] = "s" + to_string(targetIdx);
                }
            }
        }
    }

    // Sub-Task 3: Build Parsing Tables (ACTION & GOTO)
    void buildParsingTables() {
        generateCanonicalCollection();

        for (size_t i = 0; i < canonicalStates.size(); ++i) {
            for (const auto& it : canonicalStates[i]) {
                const auto& p = prods[it.prodId];
                // Case: Dot at the end (Reduce or Accept)
                if (it.dot == (int)p.rhs.size()) {
                    if (p.lhs == augmentedStart && it.lookahead == END_MARKER) {
                        actionTable[{ (int)i, END_MARKER }] = "acc";
                    } else if (p.lhs != augmentedStart) {
                        actionTable[{ (int)i, it.lookahead }] = "r" + to_string(p.id);
                    }
                }
            }
        }
    }

    // Display Canonical Items
    void printCanonicalCollection() const {
        cout << "\n--- Canonical Collection of LR(1) Item Sets ---\n";
        for (size_t i = 0; i < canonicalStates.size(); ++i) {
            cout << "I" << i << ":\n";
            for (const auto& it : canonicalStates[i]) {
                const auto& p = prods[it.prodId];
                cout << "    [" << p.lhs << " -> ";
                for (int j = 0; j < (int)p.rhs.size(); ++j) {
                    if (j == it.dot) cout << ". ";
                    cout << p.rhs[j] << " ";
                }
                if (it.dot == (int)p.rhs.size()) cout << ". ";
                cout << ", " << it.lookahead << "]\n";
            }
            cout << "\n";
        }
    }

    // Display ACTION and GOTO Tables
    void printTables() const {
        vector<string> actionCols(terminals.begin(), terminals.end());
        sort(actionCols.begin(), actionCols.end());
        actionCols.push_back(END_MARKER);

        vector<string> gotoCols;
        for (const auto& nt : nonTerminals) {
            if (nt != augmentedStart) gotoCols.push_back(nt);
        }
        sort(gotoCols.begin(), gotoCols.end());

        cout << "--- LR(1) Parsing Table (ACTION & GOTO) ---\n";
        cout << left << setw(8) << "State";
        cout << "| ";
        for (const auto& t : actionCols) cout << setw(8) << t;
        cout << "| ";
        for (const auto& nt : gotoCols) cout << setw(8) << nt;
        cout << "\n" << string(12 + (actionCols.size() + gotoCols.size()) * 8, '-') << "\n";

        for (size_t i = 0; i < canonicalStates.size(); ++i) {
            cout << left << setw(8) << i << "| ";
            // ACTION columns
            for (const auto& t : actionCols) {
                if (actionTable.count({ (int)i, t })) {
                    cout << setw(8) << actionTable.at({ (int)i, t });
                } else {
                    cout << setw(8) << "-";
                }
            }
            cout << "| ";
            // GOTO columns
            for (const auto& nt : gotoCols) {
                if (gotoTable.count({ (int)i, nt })) {
                    cout << setw(8) << gotoTable.at({ (int)i, nt });
                } else {
                    cout << setw(8) << "-";
                }
            }
            cout << "\n";
        }
    }

    // Sub-Task 4: Parse String (Shift-Reduce Automaton)
    bool parseString(const vector<string>& tokens) {
        vector<int> stateStack = { 0 };
        vector<string> symbolStack = { END_MARKER };
        vector<string> input = tokens;
        input.push_back(END_MARKER);
        size_t ip = 0;
        int step = 1;

        cout << "\n--- Parsing Input: \"";
        for (size_t i = 0; i < tokens.size(); ++i) cout << tokens[i] << (i + 1 < tokens.size() ? " " : "");
        cout << "\" ---\n";
        cout << left << setw(6) << "Step" 
             << setw(20) << "State Stack" 
             << setw(18) << "Symbol Stack" 
             << setw(20) << "Input Buffer" 
             << "Action\n";
        cout << string(76, '-') << "\n";

        while (true) {
            int s = stateStack.back();
            string a = input[ip];

            string statesStr = "";
            for (auto st : stateStack) statesStr += to_string(st) + " ";
            string symbolsStr = "";
            for (auto& sym : symbolStack) symbolsStr += sym + " ";
            string inputStr = "";
            for (size_t i = ip; i < input.size(); ++i) inputStr += input[i] + " ";

            cout << left << setw(6) << step++ 
                 << setw(20) << statesStr 
                 << setw(18) << symbolsStr 
                 << setw(20) << inputStr;

            if (!actionTable.count({ s, a })) {
                cout << "ERROR (No action for [" << s << ", " << a << "])\n";
                cout << string(76, '-') << "\n>> Result: REJECTED (Syntax Error)\n";
                return false;
            }

            string act = actionTable.at({ s, a });

            if (act == "acc") {
                cout << "ACCEPT\n";
                cout << string(76, '-') << "\n>> Result: ACCEPTED (String successfully parsed)\n";
                return true;
            }

            if (act[0] == 's') { // Shift
                int nextState = stoi(act.substr(1));
                symbolStack.push_back(a);
                stateStack.push_back(nextState);
                ip++;
                cout << "Shift to State " << nextState << "\n";
            } else if (act[0] == 'r') { // Reduce
                int ruleId = stoi(act.substr(1));
                const auto& p = prods[ruleId];
                int popCount = (p.rhs.size() == 1 && p.rhs[0] == "#") ? 0 : (int)p.rhs.size();
                for (int k = 0; k < popCount; ++k) {
                    stateStack.pop_back();
                    symbolStack.pop_back();
                }
                int topState = stateStack.back();
                if (!gotoTable.count({ topState, p.lhs })) {
                    cout << "ERROR (GOTO missing for [" << topState << ", " << p.lhs << "])\n";
                    cout << string(76, '-') << "\n>> Result: REJECTED\n";
                    return false;
                }
                int nextState = gotoTable.at({ topState, p.lhs });
                symbolStack.push_back(p.lhs);
                stateStack.push_back(nextState);
                cout << "Reduce by " << p.toString() << "\n";
            }
        }
    }
};

int main() {
    cout << "======================================================================\n";
    cout << "                     LR(1) PARSER IMPLEMENTATION                      \n";
    cout << "======================================================================\n";

    // Canonical LR(1) Textbook Grammar: S -> C C,  C -> c C | d
    LR1Parser parser("S");
    parser.addProduction("S", { "C", "C" });
    parser.addProduction("C", { "c", "C" });
    parser.addProduction("C", { "d" });

    cout << "\n--- Grammar Productions ---\n";
    for (const auto& p : parser.prods) {
        cout << "  (" << p.id << ") " << p.toString() << "\n";
    }

    // Sub-Tasks 1, 2, 3: Construct Items, Canonical Collection & Tables
    parser.buildParsingTables();
    parser.printCanonicalCollection();
    parser.printTables();

    // Sub-Task 4: Parse Strings
    // Test 1: Valid input "c d c d"
    parser.parseString({ "c", "d", "c", "d" });

    // Test 2: Valid input "d d"
    parser.parseString({ "d", "d" });

    // Test 3: Invalid input "c d c" (Syntax Error)
    parser.parseString({ "c", "d", "c" });

    cout << "\n======================================================================\n";
    cout << "                   LR(1) PARSER WORKED SUCCESSFULLY                   \n";
    cout << "======================================================================\n";
    return 0;
}
