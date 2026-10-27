#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <map>
#include <iomanip>
#include <algorithm>

using namespace std;

// Representation: '#' denotes epsilon, '$' denotes end-of-input
const string EPSILON = "#";
const string END_MARKER = "$";

class LL1Parser {
public:
    string startSymbol;
    vector<string> nonTerminals;
    set<string> nonTerminalsSet;
    set<string> terminals;
    map<string, vector<vector<string>>> rules; // NT -> list of RHS alternatives
    map<string, set<string>> first;
    map<string, set<string>> follow;
    map<pair<string, string>, vector<string>> table; // (NT, Terminal) -> RHS

    bool isNT(const string& s) const { return nonTerminalsSet.count(s) > 0; }

    void addNT(const string& nt) {
        if (!nonTerminalsSet.count(nt)) {
            nonTerminals.push_back(nt);
            nonTerminalsSet.insert(nt);
        }
    }

    void addRule(const string& lhs, const vector<string>& rhs) {
        addNT(lhs);
        if (startSymbol.empty()) startSymbol = lhs;
        rules[lhs].push_back(rhs);
        for (const auto& s : rhs) {
            if (s != EPSILON && !isNT(s)) terminals.insert(s);
        }
    }

    void refreshTerminals() {
        terminals.clear();
        for (const auto& kv : rules) {
            for (const auto& rhs : kv.second) {
                for (const auto& s : rhs) {
                    if (s != EPSILON && !isNT(s)) terminals.insert(s);
                }
            }
        }
    }

    void printGrammar(const string& title) const {
        cout << "\n--- " << title << " ---\n";
        for (const auto& nt : nonTerminals) {
            if (!rules.count(nt) || rules.at(nt).empty()) continue;
            cout << "  " << setw(4) << left << nt << " -> ";
            const auto& alts = rules.at(nt);
            for (size_t i = 0; i < alts.size(); ++i) {
                for (size_t j = 0; j < alts[i].size(); ++j) {
                    cout << alts[i][j] << (j + 1 < alts[i].size() ? " " : "");
                }
                if (i + 1 < alts.size()) cout << " | ";
            }
            cout << "\n";
        }
    }

    // 1. Remove Left Recursion (Direct and Indirect)
    void removeLeftRecursion() {
        vector<string> nts = nonTerminals;
        for (size_t i = 0; i < nts.size(); ++i) {
            string Ai = nts[i];
            for (size_t j = 0; j < i; ++j) {
                string Aj = nts[j];
                vector<vector<string>> newRules;
                for (auto& rhs : rules[Ai]) {
                    if (!rhs.empty() && rhs[0] == Aj) {
                        vector<string> gamma(rhs.begin() + 1, rhs.end());
                        for (auto& ajRhs : rules[Aj]) {
                            vector<string> exp = (ajRhs.size() == 1 && ajRhs[0] == EPSILON) ? vector<string>{} : ajRhs;
                            exp.insert(exp.end(), gamma.begin(), gamma.end());
                            if (exp.empty()) exp.push_back(EPSILON);
                            newRules.push_back(exp);
                        }
                    } else {
                        newRules.push_back(rhs);
                    }
                }
                rules[Ai] = newRules;
            }

            vector<vector<string>> alphas, betas;
            for (auto& rhs : rules[Ai]) {
                if (!rhs.empty() && rhs[0] == Ai) {
                    alphas.push_back(vector<string>(rhs.begin() + 1, rhs.end()));
                } else {
                    betas.push_back(rhs);
                }
            }

            if (!alphas.empty()) {
                string AiPrime = Ai + "'";
                while (isNT(AiPrime)) AiPrime += "'";
                addNT(AiPrime);
                nts.push_back(AiPrime);

                vector<vector<string>> newAi, newPrime;
                for (auto& beta : betas) {
                    vector<string> b = (beta.size() == 1 && beta[0] == EPSILON) ? vector<string>{} : beta;
                    b.push_back(AiPrime);
                    newAi.push_back(b);
                }
                for (auto& alpha : alphas) {
                    vector<string> a = (alpha.size() == 1 && alpha[0] == EPSILON) ? vector<string>{} : alpha;
                    a.push_back(AiPrime);
                    newPrime.push_back(a);
                }
                newPrime.push_back({ EPSILON });
                rules[Ai] = newAi;
                rules[AiPrime] = newPrime;
            }
        }
        refreshTerminals();
    }

    // 2. Left Factoring
    void leftFactor() {
        bool changed = true;
        while (changed) {
            changed = false;
            for (const auto& nt : nonTerminals) {
                auto& alts = rules[nt];
                if (alts.size() < 2) continue;

                size_t maxLen = 0;
                vector<string> bestPrefix;
                for (size_t i = 0; i < alts.size(); ++i) {
                    for (size_t j = i + 1; j < alts.size(); ++j) {
                        size_t k = 0;
                        while (k < alts[i].size() && k < alts[j].size() && alts[i][k] == alts[j][k]) k++;
                        if (k > maxLen) {
                            maxLen = k;
                            bestPrefix = vector<string>(alts[i].begin(), alts[i].begin() + k);
                        }
                    }
                }

                if (maxLen > 0) {
                    changed = true;
                    string ntPrime = nt + "'";
                    while (isNT(ntPrime)) ntPrime += "'";
                    addNT(ntPrime);

                    vector<vector<string>> newNt, newPrime;
                    vector<string> factored = bestPrefix;
                    factored.push_back(ntPrime);
                    newNt.push_back(factored);

                    for (auto& alt : alts) {
                        if (alt.size() >= maxLen && vector<string>(alt.begin(), alt.begin() + maxLen) == bestPrefix) {
                            vector<string> rem(alt.begin() + maxLen, alt.end());
                            newPrime.push_back(rem.empty() ? vector<string>{ EPSILON } : rem);
                        } else {
                            newNt.push_back(alt);
                        }
                    }
                    rules[nt] = newNt;
                    rules[ntPrime] = newPrime;
                    break;
                }
            }
        }
        refreshTerminals();
    }

    // 3. FIRST and FOLLOW Computation
    set<string> firstOfString(const vector<string>& str) {
        if (str.empty()) return { EPSILON };
        set<string> res;
        bool allEps = true;
        for (const auto& sym : str) {
            if (!isNT(sym)) {
                res.insert(sym);
                allEps = false;
                break;
            }
            for (const auto& f : first[sym]) {
                if (f != EPSILON) res.insert(f);
            }
            if (!first[sym].count(EPSILON)) {
                allEps = false;
                break;
            }
        }
        if (allEps) res.insert(EPSILON);
        return res;
    }

    void computeFirstAndFollow() {
        for (const auto& t : terminals) first[t] = { t };
        first[EPSILON] = { EPSILON };

        bool changed = true;
        while (changed) {
            changed = false;
            for (const auto& nt : nonTerminals) {
                for (const auto& rhs : rules[nt]) {
                    for (const auto& f : firstOfString(rhs)) {
                        if (first[nt].insert(f).second) changed = true;
                    }
                }
            }
        }

        follow[startSymbol].insert(END_MARKER);
        changed = true;
        while (changed) {
            changed = false;
            for (const auto& nt : nonTerminals) {
                for (const auto& rhs : rules[nt]) {
                    for (size_t i = 0; i < rhs.size(); ++i) {
                        string B = rhs[i];
                        if (!isNT(B)) continue;
                        vector<string> beta(rhs.begin() + i + 1, rhs.end());
                        set<string> fBeta = firstOfString(beta);
                        for (const auto& f : fBeta) {
                            if (f != EPSILON && follow[B].insert(f).second) changed = true;
                        }
                        if (fBeta.count(EPSILON)) {
                            for (const auto& f : follow[nt]) {
                                if (follow[B].insert(f).second) changed = true;
                            }
                        }
                    }
                }
            }
        }

        cout << "\n--- FIRST and FOLLOW Sets ---\n";
        cout << left << setw(10) << "NT" << setw(24) << "FIRST" << setw(24) << "FOLLOW" << "\n";
        cout << string(58, '-') << "\n";
        for (const auto& nt : nonTerminals) {
            string fStr = "{ ", folStr = "{ ";
            for (auto& s : first[nt]) fStr += s + " "; fStr += "}";
            for (auto& s : follow[nt]) folStr += s + " "; folStr += "}";
            cout << left << setw(10) << nt << setw(24) << fStr << setw(24) << folStr << "\n";
        }
    }

    // 4. Construct Parsing Table
    void constructTable() {
        table.clear();
        for (const auto& nt : nonTerminals) {
            for (const auto& rhs : rules[nt]) {
                set<string> f = firstOfString(rhs);
                for (const auto& a : f) {
                    if (a != EPSILON) table[{nt, a}] = rhs;
                }
                if (f.count(EPSILON)) {
                    for (const auto& b : follow[nt]) table[{nt, b}] = rhs;
                }
            }
        }

        vector<string> cols(terminals.begin(), terminals.end());
        sort(cols.begin(), cols.end());
        cols.push_back(END_MARKER);

        cout << "\n--- LL(1) Parsing Table ---\n";
        cout << left << setw(8) << "NT";
        for (const auto& c : cols) cout << setw(16) << c;
        cout << "\n" << string(8 + cols.size() * 16, '-') << "\n";

        for (const auto& nt : nonTerminals) {
            cout << left << setw(8) << nt;
            for (const auto& c : cols) {
                if (table.count({nt, c})) {
                    string prod = nt + "->";
                    for (auto& s : table[{nt, c}]) prod += s;
                    cout << setw(16) << prod;
                } else {
                    cout << setw(16) << "-";
                }
            }
            cout << "\n";
        }
    }

    // 5. Parse Input String (Pushdown Stack Simulation)
    bool parseString(const vector<string>& tokens) {
        vector<string> st = { END_MARKER, startSymbol };
        vector<string> input = tokens;
        input.push_back(END_MARKER);
        size_t ip = 0;
        int step = 1;

        cout << "\n--- Parsing Input: \"";
        for (size_t i = 0; i < tokens.size(); ++i) cout << tokens[i] << (i + 1 < tokens.size() ? " " : "");
        cout << "\" ---\n";
        cout << left << setw(6) << "Step" << setw(24) << "Stack" << setw(22) << "Input Buffer" << "Action\n";
        cout << string(72, '-') << "\n";

        while (!st.empty()) {
            string top = st.back();
            string curr = input[ip];

            string sStr = ""; for (const auto& s : st) sStr += s + " ";
            string inStr = ""; for (size_t i = ip; i < input.size(); ++i) inStr += input[i] + " ";

            cout << left << setw(6) << step++ << setw(24) << sStr << setw(22) << inStr;

            if (top == END_MARKER && curr == END_MARKER) {
                cout << "ACCEPT\n" << string(72, '-') << "\n";
                cout << ">> Result: ACCEPTED (String successfully parsed)\n";
                return true;
            }

            if (top == curr) {
                cout << "Match terminal '" << top << "'\n";
                st.pop_back();
                ip++;
            } else if (isNT(top)) {
                if (!table.count({top, curr})) {
                    cout << "ERROR (No rule for [" << top << ", " << curr << "])\n" << string(72, '-') << "\n";
                    cout << ">> Result: REJECTED (Syntax Error)\n";
                    return false;
                }
                auto rhs = table[{top, curr}];
                st.pop_back();
                if (!(rhs.size() == 1 && rhs[0] == EPSILON)) {
                    for (int i = (int)rhs.size() - 1; i >= 0; --i) st.push_back(rhs[i]);
                }
                cout << "Output " << top << " -> ";
                for (const auto& s : rhs) cout << s << " ";
                cout << "\n";
            } else {
                cout << "ERROR (Mismatch, expected '" << top << "')\n" << string(72, '-') << "\n";
                cout << ">> Result: REJECTED (Syntax Error)\n";
                return false;
            }
        }
        cout << ">> Result: REJECTED\n";
        return false;
    }
};

int main() {
    cout << "======================================================================\n";
    cout << "                     LL(1) PARSER IMPLEMENTATION                      \n";
    cout << "======================================================================\n";

    LL1Parser parser;

    // Grammar with Left Recursion: E -> E + T | T,  T -> T * F | F,  F -> ( E ) | id
    parser.addRule("E", { "E", "+", "T" });
    parser.addRule("E", { "T" });
    parser.addRule("T", { "T", "*", "F" });
    parser.addRule("T", { "F" });
    parser.addRule("F", { "(", "E", ")" });
    parser.addRule("F", { "id" });

    parser.printGrammar("Original Grammar (with Left Recursion)");

    // Sub-Task 1: Remove Left Recursion
    parser.removeLeftRecursion();
    parser.printGrammar("Grammar after Removing Left Recursion");

    // Sub-Task 2: Left Factoring
    parser.leftFactor();
    parser.printGrammar("Grammar after Left Factoring");

    // Sub-Task 3: Compute FIRST & FOLLOW Sets, then Build Table
    parser.computeFirstAndFollow();
    parser.constructTable();

    // Sub-Task 4: Parse Strings (Step-by-Step Trace & Accept/Reject)
    // Test 1: Valid expression
    parser.parseString({ "id", "+", "id", "*", "id" });

    // Test 2: Valid expression with parentheses
    parser.parseString({ "(", "id", "+", "id", ")", "*", "id" });

    // Test 3: Invalid expression (Syntax Error)
    parser.parseString({ "id", "+", "*", "id" });

    cout << "\n======================================================================\n";
    cout << "                   LL(1) PARSER WORKED SUCCESSFULLY                   \n";
    cout << "======================================================================\n";
    return 0;
}
