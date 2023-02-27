#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <algorithm>
#include <cassert>
#include <string>
#include <unordered_map>
#include <vector>
#include "ast.h"
#include "info.h"

struct TypeDescriptor {
    IDType id_type;
    int lower_bound;
    int upper_bound;
    TypeDescriptor *base;
    TypeDescriptor *next;
};

struct SymbolTableEntry {
    int timestamp;
    TypeDescriptor *type_descriptor;
};

struct SymbolTableResult {
    int timestamp;
    TypeDescriptor *type_descriptor;
    int scope;
};

static std::string get_jvm_type_str(TypeDescriptor *type_descriptor) {
    switch (type_descriptor->id_type) {
        case IDType::INT: 
            return "I";
        case IDType::REAL: 
            return "F";
        case IDType::STRING:
            return "Ljava/lang/String;";
        case IDType::VOID: 
            return "V";
        case IDType::ARRAY: 
            return "[" + get_jvm_type_str(type_descriptor->base);
        case IDType::SUBPROG: {
            std::string s = "(";
            for (TypeDescriptor *p = type_descriptor->base->next; p != nullptr; p = p->next)
                s += get_jvm_type_str(p);
            s += ")";
            s += get_jvm_type_str(type_descriptor->base);
            return s;
        }
    }
    return "UNKNOWN";
}

struct SymbolTable {
    int curr_timestamp = 0;
    int curr_scope = -1;
    std::vector<std::unordered_map<std::string, SymbolTableEntry>> symbol_table;

    SymbolTableResult add(const std::string &identifier, TypeDescriptor *type_descriptor) {
        if (symbol_table[curr_scope].count(identifier)) {
            assert(false);
        } else {
            SHOW_NEWSYM(identifier.c_str());
            curr_timestamp++;
            symbol_table[curr_scope][identifier] = SymbolTableEntry{curr_timestamp, type_descriptor};
            return {curr_timestamp, type_descriptor, curr_scope};
        }
    }
    
    SymbolTableResult get(const std::string &identifier) {
        for (int i = curr_scope; i >= 0; i--) {
            if (symbol_table[i].count(identifier)) {
                SymbolTableEntry symbol_table_entry = symbol_table[i][identifier];
                return {symbol_table_entry.timestamp, symbol_table_entry.type_descriptor, i};
            }
        }
        assert(false);
    }

    void open_scope() {
        SHOW_NEWSCP();
        curr_scope++;
        symbol_table.emplace_back();
    }

    void close_scope() {
        SHOW_CLSSCP();
        curr_scope--;
        symbol_table.pop_back();
    }
};

#endif
