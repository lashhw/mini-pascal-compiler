#ifndef TRAVERSER_H
#define TRAVERSER_H

#include <cstring>
#include <stack>
#include <sstream>
#include <utility>
#include "symbol_table.h"

struct Traverser {
    std::ofstream &out_file;
    std::string basename;

    SymbolTable symbol_table;

    std::stringstream vinit;
    std::stack<std::stringstream> buffer;
    std::vector<std::string> functions;

    std::stack<std::unordered_map<int, int>> reg_map;
    std::stack<int> reg_used;
    std::stack<int> return_symbol_reg;

    IDType curr_type = IDType::VOID;
    int label_used = 0;
    
    Traverser(std::ofstream &out_file, std::string basename) : out_file(out_file), basename(std::move(basename)) {}

    TypeDescriptor* get_type_descriptor(Node *root) {
        assert(root->node_type == NodeType::TYPE);
        
        if (root->metadata.tval == IDType::INT ||
            root->metadata.tval == IDType::REAL ||
            root->metadata.tval == IDType::STRING ||
            root->metadata.tval == IDType::VOID)
            return new TypeDescriptor{root->metadata.tval};
        else if (root->metadata.tval == IDType::ARRAY)
            return new TypeDescriptor{IDType::ARRAY, root->child[0]->metadata.ival, root->child[1]->metadata.ival, 
                                      get_type_descriptor(root->child[2]), nullptr};
        
        assert(false);
    }

    void gen_expr(Node *root) {
        if (root->node_type == NodeType::LITERAL_INT) {
            buffer.top() << "    ldc " << root->metadata.ival << "\n";
            curr_type = IDType::INT;
        } else if (root->node_type == NodeType::LITERAL_DBL) {
            buffer.top() << "    ldc " << std::showpoint << root->metadata.dval << std::noshowpoint << "\n";
            curr_type = IDType::REAL;
        } else if (root->node_type == NodeType::LITERAL_STR) {
            buffer.top() << "    ldc " << root->metadata.sval << "\n";
            curr_type = IDType::STRING;
        } else if (root->node_type == NodeType::OP) {
            gen_expr(root->child[0]);
            gen_expr(root->child[1]);
            if (root->metadata.oval == OpType::AND) {
                buffer.top() << "    iand\n";
            } else if (root->metadata.oval == OpType::OR) {
                buffer.top() << "    ior\n";
            } else if (root->metadata.oval == OpType::LT || 
                       root->metadata.oval == OpType::GT ||
                       root->metadata.oval == OpType::EQ ||
                       root->metadata.oval == OpType::LET ||
                       root->metadata.oval == OpType::GET ||
                       root->metadata.oval == OpType::NEQ) {
                if (curr_type == IDType::INT) {
                    if (root->metadata.oval == OpType::LT) 
                        buffer.top() << "    if_icmplt";
                    else if (root->metadata.oval == OpType::GT) 
                        buffer.top() << "    if_icmpgt";
                    else if (root->metadata.oval == OpType::EQ) 
                        buffer.top() << "    if_icmpeq";
                    else if (root->metadata.oval == OpType::LET) 
                        buffer.top() << "    if_icmple";
                    else if (root->metadata.oval == OpType::GET) 
                        buffer.top() << "    if_icmpge";
                    else if (root->metadata.oval == OpType::NEQ) 
                        buffer.top() << "    if_icmpne";
                } else if (curr_type == IDType::REAL) {
                    buffer.top() << "    fcmpg\n";
                    if (root->metadata.oval == OpType::LT) {
                        buffer.top() << "    iconst_m1\n";
                        buffer.top() << "    if_icmpeq";
                    } else if (root->metadata.oval == OpType::GT) {
                        buffer.top() << "    iconst_1\n";
                        buffer.top() << "    if_icmpeq";
                    } else if (root->metadata.oval == OpType::EQ) {
                        buffer.top() << "    iconst_0\n";
                        buffer.top() << "    if_icmpeq";
                    } else if (root->metadata.oval == OpType::LET) {
                        buffer.top() << "    iconst_0\n";
                        buffer.top() << "    if_icmple";
                    } else if (root->metadata.oval == OpType::GET) {
                        buffer.top() << "    iconst_0\n";
                        buffer.top() << "    if_icmpge";
                    } else if (root->metadata.oval == OpType::NEQ) {
                        buffer.top() << "    iconst_0\n";
                        buffer.top() << "    if_icmpne";
                    }
                }
                int true_label = ++label_used;
                int end_label = ++label_used;
                buffer.top() << " L" << true_label << "\n";
                buffer.top() << "    iconst_0\n";
                buffer.top() << "    goto L" << end_label << "\n";
                buffer.top() << "L" << true_label << ":\n";
                buffer.top() << "    iconst_1\n";
                buffer.top() << "L" << end_label << ":\n";
                curr_type = IDType::INT;
            } else if (root->metadata.oval == OpType::ADD) {
                if (curr_type == IDType::INT)
                    buffer.top() << "    iadd\n";
                else if (curr_type == IDType::REAL)
                    buffer.top() << "    fadd\n";
                else if (curr_type == IDType::STRING) {
                    int first_reg = reg_used.top();
                    reg_used.top()++;
                    int second_reg = reg_used.top();
                    reg_used.top()++;
                    buffer.top() << "    astore " << second_reg << "\n";
                    buffer.top() << "    astore " << first_reg << "\n";
                    buffer.top() << "    new java/lang/StringBuffer\n";
                    buffer.top() << "    dup\n";
                    buffer.top() << "    invokespecial java/lang/StringBuffer/<init>()V\n";
                    buffer.top() << "    aload " << first_reg << "\n";
                    buffer.top() << "    invokevirtual java/lang/StringBuffer/append(Ljava/lang/String;)Ljava/lang/StringBuffer;\n";
                    buffer.top() << "    aload " << second_reg << "\n";
                    buffer.top() << "    invokevirtual java/lang/StringBuffer/append(Ljava/lang/String;)Ljava/lang/StringBuffer;\n";
                    buffer.top() << "    invokevirtual java/lang/StringBuffer/toString()Ljava/lang/String;\n";
                }
            } else if (root->metadata.oval == OpType::SUB) {
                if (curr_type == IDType::INT)
                    buffer.top() << "    isub\n";
                else if (curr_type == IDType::REAL)
                    buffer.top() << "    fsub\n";
            } else if (root->metadata.oval == OpType::MUL) {
                if (curr_type == IDType::INT)
                    buffer.top() << "    imul\n";
                else if (curr_type == IDType::REAL)
                    buffer.top() << "    fmul\n";
            } else if (root->metadata.oval == OpType::DIV) {
                if (curr_type == IDType::INT)
                    buffer.top() << "    idiv\n";
                else if (curr_type == IDType::REAL)
                    buffer.top() << "    fdiv\n";
            }
        } else if (root->node_type == NodeType::VAR) {
            if (strcmp(root->metadata.sval, "readlnI") == 0) {
                buffer.top() << "    invokestatic " << basename << "/readlnI()I\n";
            } else {
                char *node_name = root->metadata.sval;
                SymbolTableResult symbol_table_result = symbol_table.get(node_name);
                if (symbol_table_result.type_descriptor->id_type == IDType::SUBPROG) {
                    if (symbol_table_result.scope != 0)
                        gen_additional_vars();
                    for (Node *curr = root->child[0]; curr != nullptr; curr = curr->next)
                        gen_expr(curr->child[0]);
                    buffer.top() << "    invokestatic " << basename << "/" << node_name << get_jvm_type_str(symbol_table_result.type_descriptor) << "\n";
                    curr_type = symbol_table_result.type_descriptor->base->id_type;
                } else {
                    if (symbol_table_result.scope == 0) {
                        buffer.top() << "    getstatic " << basename << "/" << node_name << " " << get_jvm_type_str(symbol_table_result.type_descriptor) << "\n";
                    } else {
                        if (symbol_table_result.type_descriptor->id_type == IDType::INT) 
                            buffer.top() << "    iload " << reg_map.top()[symbol_table_result.timestamp] << "\n";
                        else if (symbol_table_result.type_descriptor->id_type == IDType::REAL) 
                            buffer.top() << "    fload " << reg_map.top()[symbol_table_result.timestamp] << "\n";
                        else
                            buffer.top() << "    aload " << reg_map.top()[symbol_table_result.timestamp] << "\n";
                    }
                    curr_type = symbol_table_result.type_descriptor->id_type;
                }
                if (symbol_table_result.type_descriptor->id_type == IDType::ARRAY) {
                    Node *curr_var_tail = root->child[0];
                    TypeDescriptor *curr_type_descriptor = symbol_table_result.type_descriptor;
                    for (; curr_var_tail != nullptr; curr_var_tail = curr_var_tail->next, curr_type_descriptor = curr_type_descriptor->base) {
                        gen_expr(curr_var_tail->child[0]);
                        buffer.top() << "    ldc " << curr_type_descriptor->lower_bound << "\n";
                        buffer.top() << "    isub\n";
                        if (curr_type_descriptor->base->id_type == IDType::INT)
                            buffer.top() << "    iaload\n";
                        else if (curr_type_descriptor->base->id_type == IDType::REAL)
                            buffer.top() << "    faload\n";
                        else
                            buffer.top() << "    aaload\n";
                    }
                }
            }
        } else if (root->node_type == NodeType::PROCEDURE) {
            char *node_name = root->metadata.sval;
            SymbolTableResult symbol_table_result = symbol_table.get(node_name);
            if (symbol_table_result.scope != 0)
                gen_additional_vars();
            for (Node *curr = root->child[0]; curr != nullptr; curr = curr->next)
                gen_expr(curr->child[0]);
            buffer.top() << "    invokestatic " << basename << "/" << node_name << get_jvm_type_str(symbol_table_result.type_descriptor) << "\n";
        } else if (root->node_type == NodeType::NOT) {
            gen_expr(root->child[0]);
            buffer.top() << "    iconst_1\n";
            buffer.top() << "    ixor\n";
        } else if (root->node_type == NodeType::NEGATE) {
            gen_expr(root->child[0]);
            if (curr_type == IDType::INT)
                buffer.top() << "    ineg\n";
            else if (curr_type == IDType::REAL)
                buffer.top() << "    fneg\n";
        } else {
            assert(false);
        }
    }
    
    void gen_prog(Node *root) {
        assert(root->node_type == NodeType::PROG);
        // Node *id_list_node = root->child[0];
        Node *decl_list_node = root->child[1];
        Node *subprog_decl_list_node = root->child[2];
        Node *stmt_list_node = root->child[3];

        buffer.emplace();
        symbol_table.open_scope();

        symbol_table.add(root->metadata.sval, new TypeDescriptor{IDType::VOID});
        buffer.top() << ".class public " << basename << "\n.super java/lang/Object\n\n";
        gen_decl_list_prog(decl_list_node);
        buffer.top() << '\n';
        buffer.top() << ".method public static readlnI()I\n    .limit locals 10\n    .limit stack 10\n    ldc 0\n    istore 1\nLAB1:\n    getstatic java/lang/System/in Ljava/io/InputStream;\n    invokevirtual java/io/InputStream/read()I\n    istore 2\n    iload 2\n    ldc 10\n    isub\n    ifeq LAB2\n    iload 2\n    ldc 32\n    isub\n    ifeq LAB2\n    iload 2\n    ldc 48\n    isub\n    ldc 10\n    iload 1\n    imul\n    iadd\n    istore 1\n    goto LAB1\nLAB2:\n    iload 1\n    ireturn\n.end method\n\n";
        buffer.top() << ".method public static vinit()V\n    .limit locals 100\n    .limit stack 100\n" << vinit.str() << "    return\n.end method\n\n";
        buffer.top() << ".method public <init>()V\n    aload_0\n    invokenonvirtual java/lang/Object/<init>()V\n    return\n.end method\n\n";

        gen_subprog_decl_list(subprog_decl_list_node);
        for (auto &function : functions)
            buffer.top() << function;

        buffer.top() << ".method public static main([Ljava/lang/String;)V\n    .limit locals 100\n    .limit stack 100\n    invokestatic " + basename + "/vinit()V\n";
        reg_map.emplace();
        reg_used.push(0);
        gen_stmt(stmt_list_node);
        buffer.top() << "    return\n.end method\n";

        out_file << buffer.top().str();
        buffer.pop();
        symbol_table.close_scope();
        reg_map.pop();
        reg_used.pop();
    }
    
    void gen_decl_list_prog(Node *root) {
        if (root == nullptr) return;
        assert(root->node_type == NodeType::DECL_LIST);
        for (Node *decl_list_node = root; decl_list_node != nullptr; decl_list_node = decl_list_node->next) {
            TypeDescriptor *type_descriptor = get_type_descriptor(decl_list_node->child[1]);
            for (Node *id_list_node = decl_list_node->child[0]; id_list_node != nullptr; id_list_node = id_list_node->next) {
                char *var_name = id_list_node->metadata.sval;
                symbol_table.add(var_name, type_descriptor);
                std::string jvm_type_str = get_jvm_type_str(type_descriptor);
                buffer.top() << ".field public static " << var_name << " " << jvm_type_str << "\n";
                if (type_descriptor->id_type == IDType::INT) 
                    vinit << "    ldc 0\n    putstatic " << basename << "/" << var_name << " I\n";
                else if (type_descriptor->id_type == IDType::REAL) 
                    vinit << "    ldc 0.0\n    putstatic " << basename << "/" << var_name << " F\n";
                else if (type_descriptor->id_type == IDType::STRING)
                    vinit << "    ldc \"\"\n    putstatic " << basename << "/" << var_name << " Ljava/lang/String;\n";
                else if (type_descriptor->id_type == IDType::ARRAY) {
                    int arr_dim = 0;
                    for (TypeDescriptor *curr = type_descriptor; curr->id_type == IDType::ARRAY; curr = curr->base, arr_dim++)
                        vinit << "    bipush " << curr->upper_bound - curr->lower_bound + 1 << "\n";
                    vinit << "    multianewarray " << jvm_type_str << " " << arr_dim << "\n";
                    vinit << "    putstatic " << basename << "/" << var_name << " " << jvm_type_str << "\n";
                } else
                    assert(false);
            }
        }
    }
    
    void gen_subprog_decl_list(Node *root) {
        if (root == nullptr) return;
        assert(root->node_type == NodeType::SUBPROG_DECL_LIST);
        for (Node *subprog_decl_list_node = root; subprog_decl_list_node != nullptr; subprog_decl_list_node = subprog_decl_list_node->next) {
            buffer.emplace();

            reg_map.emplace();
            reg_used.push(0);
            return_symbol_reg.push(-1);

            Node *subprog_head_node = subprog_decl_list_node->child[0];
            Node *decl_list_node = subprog_decl_list_node->child[1];
            Node *inner_subprog_decl_list_node = subprog_decl_list_node->child[2];
            Node *stmt_list_node = subprog_decl_list_node->child[3];
            gen_subprog_head(subprog_head_node);
            gen_decl_list_subprog(decl_list_node);
            gen_subprog_decl_list(inner_subprog_decl_list_node);
            gen_stmt(stmt_list_node);

            SymbolTableResult symbol_table_result = symbol_table.get(subprog_head_node->metadata.sval);
            if (symbol_table_result.type_descriptor->base->id_type == IDType::VOID) {
                buffer.top() << "    return\n";
            } else if (symbol_table_result.type_descriptor->base->id_type == IDType::INT) {
                buffer.top() << "    iload " << return_symbol_reg.top() << "\n";
                buffer.top() << "    ireturn\n";
            } else if (symbol_table_result.type_descriptor->base->id_type == IDType::REAL) {
                buffer.top() << "    fload " << return_symbol_reg.top() << "\n";
                buffer.top() << "    freturn\n";
            } else {
                buffer.top() << "    aload " << return_symbol_reg.top() << "\n";
                buffer.top() << "    areturn\n";
            }
            buffer.top() << ".end method\n\n";

            functions.push_back(buffer.top().str());
            buffer.pop();

            reg_map.pop();
            reg_used.pop();
            return_symbol_reg.pop();

            symbol_table.close_scope();
            symbol_table.close_scope();
        }
    }
    
    void gen_decl_list_subprog(Node *root) {
        if (root == nullptr) return;
        assert(root->node_type == NodeType::DECL_LIST);
        for (Node *decl_list_node = root; decl_list_node != nullptr; decl_list_node = decl_list_node->next) {
            TypeDescriptor *type_descriptor = get_type_descriptor(decl_list_node->child[1]);
            for (Node *id_list_node = decl_list_node->child[0]; id_list_node != nullptr; id_list_node = id_list_node->next) {
                char *var_name = id_list_node->metadata.sval;
                SymbolTableResult symbol_table_result = symbol_table.add(var_name, type_descriptor);
                reg_map.top()[symbol_table_result.timestamp] = reg_used.top();
                std::string jvm_type_str = get_jvm_type_str(type_descriptor);
                if (type_descriptor->id_type == IDType::INT) 
                    buffer.top() << "    ldc 0\n    istore " << reg_used.top() << "\n";
                else if (type_descriptor->id_type == IDType::REAL) 
                    buffer.top() << "    ldc 0.0\n    fstore " << reg_used.top() << "\n";
                else if (type_descriptor->id_type == IDType::ARRAY) {
                    int arr_dim = 0;
                    for (TypeDescriptor *curr = type_descriptor; curr->id_type == IDType::ARRAY; curr = curr->base, arr_dim++)
                        buffer.top() << "    bipush " << curr->upper_bound - curr->lower_bound + 1 << "\n";
                    buffer.top() << "    multianewarray " << jvm_type_str << " " << arr_dim << "\n";
                    buffer.top() << "    astore " << reg_used.top() << "\n";
                }
                reg_used.top()++;
            }
        }
    }
    
    void gen_subprog_head(Node *root) {
        assert(root->node_type == NodeType::SUBPROG_HEAD);
        auto *subprog_type_descriptor = new TypeDescriptor{IDType::SUBPROG, 0, 0, get_type_descriptor(root->child[1]), nullptr};
        TypeDescriptor *subprog_type_descriptor_tail = subprog_type_descriptor->base;

        std::vector<std::pair<std::string, TypeDescriptor*>> params_inherited;
        for (int i = 1; i <= symbol_table.curr_scope; i++) {
            for (const auto &result : symbol_table.symbol_table[i]) {
                auto *dup_type_descriptor = new TypeDescriptor(*result.second.type_descriptor);
                subprog_type_descriptor_tail = subprog_type_descriptor_tail->next = dup_type_descriptor;
                params_inherited.emplace_back(result.first, dup_type_descriptor);
            }
        }

        std::vector<std::pair<std::string, TypeDescriptor*>> params;
        for (Node *param_list_node = root->child[0]; param_list_node != nullptr; param_list_node = param_list_node->next) {
            TypeDescriptor *param_type_descriptor = get_type_descriptor(param_list_node->child[1]);
            for (Node *id_list_node = param_list_node->child[0]; id_list_node != nullptr; id_list_node = id_list_node->next) {
                auto *dup_param_type_descriptor = new TypeDescriptor(*param_type_descriptor);
                subprog_type_descriptor_tail = subprog_type_descriptor_tail->next = dup_param_type_descriptor;
                params.emplace_back(id_list_node->metadata.sval, dup_param_type_descriptor);
            }
        }

        symbol_table.add(root->metadata.sval, subprog_type_descriptor);
        buffer.top() << ".method public static " << root->metadata.sval << get_jvm_type_str(subprog_type_descriptor) << "\n";
        buffer.top() << "    .limit locals 100\n    .limit stack 100\n";

        symbol_table.open_scope();
        for (auto &p : params_inherited) {
            SymbolTableResult symbol_table_result = symbol_table.add(p.first, p.second);
            reg_map.top()[symbol_table_result.timestamp] = reg_used.top();
            reg_used.top()++;
        }

        symbol_table.open_scope();
        for (auto &p : params) {
            SymbolTableResult symbol_table_result = symbol_table.add(p.first, p.second);
            reg_map.top()[symbol_table_result.timestamp] = reg_used.top();
            reg_used.top()++;
        }
        
        reg_map.top()[symbol_table.get(root->metadata.sval).timestamp] = reg_used.top();
        return_symbol_reg.top() = reg_used.top();
        reg_used.top()++;
    }
    
    void gen_stmt(Node *root) {
        if (root == nullptr) return;
        if (root->node_type == NodeType::STMT_LIST) {
            for (Node *stmt_list_node = root; stmt_list_node != nullptr; stmt_list_node = stmt_list_node->next)
                gen_stmt(stmt_list_node->child[0]);
        } else if (root->node_type == NodeType::ASSIGN) {
            Node *var_node = root->child[0];
            char *var_name = var_node->metadata.sval;
            Node *expr_node = root->child[1];
            SymbolTableResult symbol_table_result = symbol_table.get(var_name);
            bool is_return_var = (reg_map.top().count(symbol_table_result.timestamp) != 0 && reg_map.top()[symbol_table_result.timestamp] == return_symbol_reg.top());
            bool is_global_var = symbol_table_result.scope == 0 && !is_return_var;
            if (symbol_table_result.type_descriptor->id_type == IDType::ARRAY) {
                if (is_global_var) 
                    buffer.top() << "    getstatic " << basename << "/" << var_name << " " << get_jvm_type_str(symbol_table_result.type_descriptor) << "\n";
                else
                    buffer.top() << "    aload " << reg_map.top()[symbol_table_result.timestamp] << "\n";
                Node *curr_var_tail = var_node->child[0];
                TypeDescriptor *curr_type_descriptor = symbol_table_result.type_descriptor;
                for (; curr_var_tail != nullptr; curr_var_tail = curr_var_tail->next, curr_type_descriptor = curr_type_descriptor->base) {
                    gen_expr(curr_var_tail->child[0]);
                    buffer.top() << "    ldc " << curr_type_descriptor->lower_bound << "\n";
                    buffer.top() << "    isub\n";
                    if (curr_var_tail->next != nullptr)
                        buffer.top() << "    aaload\n";
                }
                gen_expr(expr_node);
                if (curr_type_descriptor->id_type == IDType::INT) 
                    buffer.top() << "    iastore\n";
                else if (curr_type_descriptor->id_type == IDType::REAL) 
                    buffer.top() << "    fastore\n";
                else
                    buffer.top() << "    aastore\n";
            } else if (is_global_var) {
                gen_expr(expr_node);
                buffer.top() << "    putstatic " << basename << "/" << var_name << " " << get_jvm_type_str(symbol_table_result.type_descriptor) << "\n";
            } else if (is_return_var) {
                gen_expr(expr_node);
                if (symbol_table_result.type_descriptor->base->id_type == IDType::INT) 
                    buffer.top() << "    istore " << return_symbol_reg.top() << "\n";
                else if (symbol_table_result.type_descriptor->base->id_type == IDType::REAL)
                    buffer.top() << "    fstore " << return_symbol_reg.top() << "\n";
                else
                    buffer.top() << "    astore " << return_symbol_reg.top() << "\n";
            } else {
                gen_expr(expr_node);
                if (symbol_table_result.type_descriptor->id_type == IDType::INT) 
                    buffer.top() << "    istore " << reg_map.top()[symbol_table_result.timestamp] << "\n";
                else if (symbol_table_result.type_descriptor->id_type == IDType::REAL) 
                    buffer.top() << "    fstore " << reg_map.top()[symbol_table_result.timestamp] << "\n";
                else
                    buffer.top() << "    astore " << reg_map.top()[symbol_table_result.timestamp] << "\n";
            }
        } else if (root->node_type == NodeType::IF) {
            Node *expr_node = root->child[0];
            Node *stmt_1_node = root->child[1];
            Node *stmt_2_node = root->child[2];
            gen_expr(expr_node);
            int false_label = ++label_used;
            int end_label = ++label_used;
            buffer.top() << "    ifeq L" << false_label << "\n";
            gen_stmt(stmt_1_node);
            buffer.top() << "    goto L" << end_label << "\n";
            buffer.top() << "L" << false_label << ":\n";
            gen_stmt(stmt_2_node);
            buffer.top() << "L" << end_label << ":\n";
        } else if (root->node_type == NodeType::WHILE) {
            Node *expr_node = root->child[0];
            Node *stmt_node = root->child[1];
            int test_label = ++label_used;
            int end_label = ++label_used;
            buffer.top() << "L" << test_label << ":\n";
            gen_expr(expr_node);
            buffer.top() << "    ifeq L" << end_label << "\n";
            gen_stmt(stmt_node);
            buffer.top() << "    goto L" << test_label << "\n";
            buffer.top() << "L" << end_label << ":\n";
        } else if (root->node_type == NodeType::PROCEDURE) {
            if (strcmp(root->metadata.sval, "writelnI") == 0) {
                buffer.top() << "    getstatic java/lang/System/out Ljava/io/PrintStream;\n";
                gen_expr(root->child[0]->child[0]);
                buffer.top() << "    invokevirtual java/io/PrintStream/println(I)V\n";
            } else if (strcmp(root->metadata.sval, "writelnR") == 0) {
                buffer.top() << "    getstatic java/lang/System/out Ljava/io/PrintStream;\n";
                gen_expr(root->child[0]->child[0]);
                buffer.top() << "    invokevirtual java/io/PrintStream/println(F)V\n";
            } else if (strcmp(root->metadata.sval, "writelnS") == 0) {
                buffer.top() << "    getstatic java/lang/System/out Ljava/io/PrintStream;\n";
                gen_expr(root->child[0]->child[0]);
                buffer.top() << "    invokevirtual java/io/PrintStream/println(Ljava/lang/String;)V\n";
            } else {
                SymbolTableResult symbol_table_result = symbol_table.get(root->metadata.sval);
                if (symbol_table_result.scope != 0)
                    gen_additional_vars();
                for (Node *expr_list_node = root->child[0]; expr_list_node != nullptr; expr_list_node = expr_list_node->next)
                    gen_expr(expr_list_node->child[0]);
                buffer.top() << "    invokestatic " << basename << "/" << root->metadata.sval << get_jvm_type_str(symbol_table_result.type_descriptor) << "\n";
            }
        } else {
            assert(false);
        }
    }

    void gen_additional_vars() {
        for (int i = 1; i <= symbol_table.curr_scope; i++) {
            for (const auto &result : symbol_table.symbol_table[i]) {
                if (result.second.type_descriptor->id_type == IDType::INT ||
                    result.second.type_descriptor->id_type == IDType::REAL ||
                    result.second.type_descriptor->id_type == IDType::STRING ||
                    result.second.type_descriptor->id_type == IDType::ARRAY) {
                    Node *var_node = new Node{NodeType::VAR, {}, {}, {}};
                    var_node->metadata.sval = strdup(result.first.c_str());
                    gen_expr(var_node);
                }
            }
        }
    }
};

#endif
