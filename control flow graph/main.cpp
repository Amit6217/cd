#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <set>
#include <map>
#include <iomanip>
#include <algorithm>

using namespace std;

struct Instruction {
    int lineNo;
    string text;
    bool isLeader = false;
    string reason = "";
    bool isConditional = false;
    bool isUnconditional = false;
    bool isHalt = false;
    int targetLine = -1;
};

struct Edge {
    int fromBlock;
    int toBlock;
    string type; // "Fall-through", "True Branch", "False Branch", "Unconditional"
};

struct BasicBlock {
    int id; // B1, B2, ...
    int startLine;
    int endLine;
    vector<Instruction> instructions;
    vector<int> successors;
    vector<int> predecessors;
    vector<Edge> edges;
};

class ControlFlowGraph {
public:
    vector<Instruction> code;
    vector<BasicBlock> blocks;
    map<int, int> lineToBlock; // Line number -> Block ID
    vector<Edge> allEdges;

    void parseInstructions(const vector<string>& rawCode) {
        code.clear();
        blocks.clear();
        lineToBlock.clear();
        allEdges.clear();

        for (size_t i = 0; i < rawCode.size(); ++i) {
            Instruction inst;
            inst.lineNo = (int)i + 1;
            inst.text = rawCode[i];

            // Detect goto and target
            size_t gotoPos = inst.text.find("goto");
            if (gotoPos != string::npos) {
                string afterGoto = inst.text.substr(gotoPos + 4);
                stringstream ss(afterGoto);
                int target;
                if (ss >> target) {
                    inst.targetLine = target;
                }
                if (inst.text.find("if") != string::npos) {
                    inst.isConditional = true;
                } else {
                    inst.isUnconditional = true;
                }
            } else if (inst.text.find("halt") != string::npos || inst.text.find("return") != string::npos) {
                inst.isHalt = true;
            }

            code.push_back(inst);
        }
    }

    // Sub-Task 1: Identify Leaders
    void identifyLeaders() {
        if (code.empty()) return;

        // Rule 1: The first statement is a leader
        code[0].isLeader = true;
        code[0].reason = "Rule 1: First statement of program";

        for (size_t i = 0; i < code.size(); ++i) {
            // Rule 2: Target of any conditional or unconditional goto is a leader
            if (code[i].targetLine != -1) {
                int targetIdx = code[i].targetLine - 1;
                if (targetIdx >= 0 && targetIdx < (int)code.size()) {
                    code[targetIdx].isLeader = true;
                    string r = "Rule 2: Target of goto at line " + to_string(code[i].lineNo);
                    if (code[targetIdx].reason.empty()) {
                        code[targetIdx].reason = r;
                    } else if (code[targetIdx].reason.find("Rule 2") == string::npos) {
                        code[targetIdx].reason += " & " + r;
                    }
                }
            }

            // Rule 3: Statement immediately following a goto is a leader
            if (code[i].isConditional || code[i].isUnconditional) {
                if (i + 1 < code.size()) {
                    code[i + 1].isLeader = true;
                    string r = "Rule 3: Follows goto at line " + to_string(code[i].lineNo);
                    if (code[i + 1].reason.empty()) {
                        code[i + 1].reason = r;
                    } else if (code[i + 1].reason.find("Rule 3") == string::npos) {
                        code[i + 1].reason += " & " + r;
                    }
                }
            }
        }
    }

    // Sub-Task 2: Construct Basic Blocks
    void constructBasicBlocks() {
        blocks.clear();
        int blockCount = 1;
        BasicBlock currBlock;
        currBlock.id = blockCount;
        currBlock.startLine = code[0].lineNo;

        for (size_t i = 0; i < code.size(); ++i) {
            // If this is a leader and not the very first statement, close previous block
            if (code[i].isLeader && i != 0) {
                currBlock.endLine = code[i - 1].lineNo;
                blocks.push_back(currBlock);

                blockCount++;
                currBlock = BasicBlock();
                currBlock.id = blockCount;
                currBlock.startLine = code[i].lineNo;
            }

            currBlock.instructions.push_back(code[i]);
            lineToBlock[code[i].lineNo] = blockCount;
        }

        // Push final block
        if (!currBlock.instructions.empty()) {
            currBlock.endLine = code.back().lineNo;
            blocks.push_back(currBlock);
        }
    }

    // Sub-Task 3: Generate CFG (Directed Edges between Basic Blocks)
    void generateCFG() {
        allEdges.clear();

        for (size_t i = 0; i < blocks.size(); ++i) {
            auto& b = blocks[i];
            const auto& lastInst = b.instructions.back();

            if (lastInst.isConditional) {
                // Conditional: Branch edge to target block
                int targetBlock = lineToBlock[lastInst.targetLine];
                Edge branchEdge = { b.id, targetBlock, "Branch (True)" };
                b.edges.push_back(branchEdge);
                b.successors.push_back(targetBlock);
                allEdges.push_back(branchEdge);

                // Fall-through edge to next sequential block
                if (i + 1 < blocks.size()) {
                    int nextBlock = blocks[i + 1].id;
                    Edge fallEdge = { b.id, nextBlock, "Fall-through (False)" };
                    b.edges.push_back(fallEdge);
                    b.successors.push_back(nextBlock);
                    allEdges.push_back(fallEdge);
                }
            } else if (lastInst.isUnconditional) {
                // Unconditional: only jump edge to target block
                int targetBlock = lineToBlock[lastInst.targetLine];
                Edge jumpEdge = { b.id, targetBlock, "Unconditional Jump" };
                b.edges.push_back(jumpEdge);
                b.successors.push_back(targetBlock);
                allEdges.push_back(jumpEdge);
            } else if (!lastInst.isHalt) {
                // Normal instruction: fall-through to next block
                if (i + 1 < blocks.size()) {
                    int nextBlock = blocks[i + 1].id;
                    Edge fallEdge = { b.id, nextBlock, "Fall-through" };
                    b.edges.push_back(fallEdge);
                    b.successors.push_back(nextBlock);
                    allEdges.push_back(fallEdge);
                }
            }
        }

        // Build predecessors
        for (const auto& e : allEdges) {
            int toIdx = e.toBlock - 1;
            if (toIdx >= 0 && toIdx < (int)blocks.size()) {
                blocks[toIdx].predecessors.push_back(e.fromBlock);
            }
        }
    }

    // Sub-Task 4: Display Graph & Structure
    void displayLeaders() const {
        cout << "\n[1] LEADER IDENTIFICATION TABLE:\n";
        cout << "----------------------------------------------------------------------------------------\n";
        cout << left << setw(8) << "Line" 
             << setw(26) << "Three-Address Code" 
             << setw(12) << "Is Leader?" 
             << "Reason\n";
        cout << "----------------------------------------------------------------------------------------\n";
        for (const auto& inst : code) {
            cout << left << setw(8) << inst.lineNo 
                 << setw(26) << inst.text 
                 << setw(12) << (inst.isLeader ? "YES" : "No") 
                 << (inst.isLeader ? inst.reason : "-") << "\n";
        }
    }

    void displayBasicBlocks() const {
        cout << "\n[2] BASIC BLOCKS PARTITION:\n";
        cout << "----------------------------------------------------------------------------------------\n";
        for (const auto& b : blocks) {
            cout << "  +--- [ Block B" << b.id << " ] (Lines " << b.startLine << " to " << b.endLine << ") ---\n";
            for (const auto& inst : b.instructions) {
                cout << "  |  (" << inst.lineNo << ")  " << inst.text << "\n";
            }
            cout << "  +-------------------------------------------------\n\n";
        }
    }

    void displayCFGStructure() const {
        cout << "[3] CONTROL FLOW GRAPH EDGES & ADJACENCY:\n";
        cout << "----------------------------------------------------------------------------------------\n";
        cout << left << setw(10) << "Source" 
             << setw(12) << "Target" 
             << setw(24) << "Edge Type" 
             << "Description\n";
        cout << "----------------------------------------------------------------------------------------\n";
        for (const auto& e : allEdges) {
            cout << left << setw(10) << ("B" + to_string(e.fromBlock)) 
                 << setw(12) << ("B" + to_string(e.toBlock)) 
                 << setw(24) << e.type 
                 << "Control transfers from B" << e.fromBlock << " -> B" << e.toBlock << "\n";
        }

        cout << "\n[4] PREDECESSOR & SUCCESSOR SETS:\n";
        cout << "----------------------------------------------------------------------------------------\n";
        cout << left << setw(10) << "Block" 
             << setw(28) << "Predecessors" 
             << setw(28) << "Successors" << "\n";
        cout << "----------------------------------------------------------------------------------------\n";
        for (const auto& b : blocks) {
            string pred = "{ ";
            for (size_t i = 0; i < b.predecessors.size(); ++i) {
                pred += "B" + to_string(b.predecessors[i]) + (i + 1 < b.predecessors.size() ? ", " : " ");
            }
            pred += "}";

            string succ = "{ ";
            for (size_t i = 0; i < b.successors.size(); ++i) {
                succ += "B" + to_string(b.successors[i]) + (i + 1 < b.successors.size() ? ", " : " ");
            }
            succ += "}";

            cout << left << setw(10) << ("B" + to_string(b.id)) 
                 << setw(28) << pred 
                 << setw(28) << succ << "\n";
        }
    }

    void displayASCIIGraph() const {
        cout << "\n[5] ASCII CONTROL FLOW GRAPH REPRESENTATION:\n";
        cout << "----------------------------------------------------------------------------------------\n";
        cout << "  [ENTRY]\n";
        cout << "     |\n";
        cout << "     v\n";
        for (size_t i = 0; i < blocks.size(); ++i) {
            const auto& b = blocks[i];
            cout << "  +-----------------------------+\n";
            cout << "  |          BLOCK B" << b.id << "           |\n";
            for (const auto& inst : b.instructions) {
                string line = "  |  " + to_string(inst.lineNo) + ": " + inst.text;
                while (line.length() < 32) line += " ";
                line += "|\n";
                cout << line;
            }
            cout << "  +-----------------------------+\n";

            if (b.edges.empty()) {
                cout << "     |\n";
                cout << "     v\n";
                cout << "  [EXIT]\n";
            } else {
                for (const auto& e : b.edges) {
                    if (e.toBlock == b.id) {
                        cout << "     |--- Loop back to B" << b.id << " (" << e.type << ")\n";
                    } else if (e.toBlock < b.id) {
                        cout << "     |--- Branch backward to B" << e.toBlock << " (" << e.type << ")\n";
                    } else if (e.toBlock == b.id + 1) {
                        cout << "     | (to B" << e.toBlock << ": " << e.type << ")\n";
                        cout << "     v\n";
                    } else {
                        cout << "     |---> Branch forward to B" << e.toBlock << " (" << e.type << ")\n";
                    }
                }
            }
        }
    }

    void displayDOTGraph() const {
        cout << "\n[6] GRAPHVIZ (DOT) GRAPH SPECIFICATION:\n";
        cout << "----------------------------------------------------------------------------------------\n";
        cout << "digraph CFG {\n";
        cout << "  node [shape=box, fontname=\"Courier\"];\n";
        cout << "  ENTRY -> B1;\n";
        for (const auto& b : blocks) {
            string label = "B" + to_string(b.id) + "\\n";
            for (const auto& inst : b.instructions) {
                label += to_string(inst.lineNo) + ": " + inst.text + "\\n";
            }
            cout << "  B" << b.id << " [label=\"" << label << "\"];\n";
        }
        for (const auto& e : allEdges) {
            cout << "  B" << e.fromBlock << " -> B" << e.toBlock 
                 << " [label=\"" << e.type << "\"];\n";
        }
        cout << "}\n";
    }

    void processProgram(const string& title, const vector<string>& rawCode) {
        cout << "\n========================================================================================\n";
        cout << " PROGRAM: " << title << "\n";
        cout << "========================================================================================\n";
        parseInstructions(rawCode);
        identifyLeaders();
        constructBasicBlocks();
        generateCFG();

        displayLeaders();
        displayBasicBlocks();
        displayCFGStructure();
        displayASCIIGraph();
        displayDOTGraph();
    }
};

int main() {
    cout << "========================================================================================\n";
    cout << "                  CONTROL FLOW GRAPH (CFG) GENERATION SUITE                             \n";
    cout << "========================================================================================\n";

    ControlFlowGraph cfg;

    // Test Program 1: Loop with Conditional Branch (Sum of 1 to 10)
    vector<string> program1 = {
        "i = 1",
        "sum = 0",
        "t1 = i * i",
        "sum = sum + t1",
        "i = i + 1",
        "if i <= 10 goto 3",
        "write sum",
        "halt"
    };
    cfg.processProgram("Loop with Conditional Branch (Sum of Squares)", program1);

    // Test Program 2: If-Else Conditional Branch
    vector<string> program2 = {
        "read x",
        "if x > 0 goto 5",
        "y = -x",
        "goto 6",
        "y = x",
        "write y",
        "halt"
    };
    cfg.processProgram("If-Else Branching Structure", program2);

    cout << "\n========================================================================================\n";
    cout << "                         CFG GENERATED SUCCESSFULLY                                     \n";
    cout << "========================================================================================\n";
    return 0;
}
