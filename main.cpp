#include <bits/stdc++.h>
using namespace std;

struct Var {
    enum Type { INT, STRING } type;
    int int_val = 0;
    string str_val;
};

static inline bool is_valid_varname(const string &s) {
    if (s.empty()) return false;
    if (!(isalpha((unsigned char)s[0]) || s[0] == '_')) return false;
    for (size_t i = 1; i < s.size(); ++i) {
        unsigned char c = s[i];
        if (!(isalnum(c) || c == '_')) return false;
    }
    return true;
}

struct ScopeSimulator {
    vector<unordered_map<string, Var>> scopes;

    ScopeSimulator() { scopes.emplace_back(); }

    int find_scope_idx(const string &name) const {
        for (int i = (int)scopes.size() - 1; i >= 0; --i) {
            auto it = scopes[i].find(name);
            if (it != scopes[i].end()) return i;
        }
        return -1;
    }

    bool declare_var(const string &type_token, const string &name, const string &value_part) {
        if (!is_valid_varname(name)) return false;
        auto &cur = scopes.back();
        if (cur.find(name) != cur.end()) return false; // redeclare in same scope invalid

        Var v;
        if (type_token == "int") {
            // value_part should be an integer literal
            if (value_part.empty()) return false;
            // validate integer literal
            int sign = 1; size_t i = 0;
            if (value_part[0] == '+' || value_part[0] == '-') {
                if (value_part.size() == 1) return false;
                sign = (value_part[0] == '-') ? -1 : 1;
                i = 1;
            }
            long long acc = 0;
            for (; i < value_part.size(); ++i) {
                char c = value_part[i];
                if (!isdigit((unsigned char)c)) return false;
                acc = acc * 10 + (c - '0');
            }
            long long llv = acc * sign;
            if (llv < INT_MIN || llv > INT_MAX) return false;
            v.type = Var::INT;
            v.int_val = (int)llv;
            cur[name] = std::move(v);
            return true;
        } else if (type_token == "string") {
            // value_part should be a quoted string "..."
            if (value_part.size() < 2) return false;
            if (value_part.front() != '"' || value_part.back() != '"') return false;
            v.type = Var::STRING;
            v.str_val = value_part.substr(1, value_part.size() - 2);
            cur[name] = std::move(v);
            return true;
        } else {
            return false;
        }
    }

    bool add_to_result(const string &res, const string &v1, const string &v2) {
        if (!is_valid_varname(res) || !is_valid_varname(v1) || !is_valid_varname(v2)) return false;
        int ir = find_scope_idx(res);
        int i1 = find_scope_idx(v1);
        int i2 = find_scope_idx(v2);
        if (ir < 0 || i1 < 0 || i2 < 0) return false;
        Var &R = scopes[ir][res];
        Var &A = scopes[i1][v1];
        Var &B = scopes[i2][v2];
        if (R.type != A.type || R.type != B.type) return false;
        if (R.type == Var::INT) {
            long long tmp = (long long)A.int_val + (long long)B.int_val;
            if (tmp < INT_MIN || tmp > INT_MAX) return false; // though guaranteed
            R.int_val = (int)tmp;
        } else {
            R.str_val = A.str_val + B.str_val;
        }
        return true;
    }

    bool self_add(const string &name, const string &value_repr, bool is_string_literal) {
        if (!is_valid_varname(name)) return false;
        int idx = find_scope_idx(name);
        if (idx < 0) return false;
        Var &V = scopes[idx][name];
        if (V.type == Var::INT) {
            if (is_string_literal) return false;
            if (value_repr.empty()) return false;
            int sign = 1; size_t i = 0; long long acc = 0;
            if (value_repr[0] == '+' || value_repr[0] == '-') {
                if (value_repr.size() == 1) return false;
                sign = (value_repr[0] == '-') ? -1 : 1;
                i = 1;
            }
            for (; i < value_repr.size(); ++i) {
                char c = value_repr[i];
                if (!isdigit((unsigned char)c)) return false;
                acc = acc * 10 + (c - '0');
            }
            long long llv = (long long)V.int_val + acc * sign;
            if (llv < INT_MIN || llv > INT_MAX) return false;
            V.int_val = (int)llv;
            return true;
        } else { // STRING
            if (!is_string_literal) return false;
            if (value_repr.size() < 2) return false;
            if (value_repr.front() != '"' || value_repr.back() != '"') return false;
            string add = value_repr.substr(1, value_repr.size() - 2);
            V.str_val += add;
            return true;
        }
    }

    bool print_var(const string &name) const {
        if (!is_valid_varname(name)) return false;
        int idx = find_scope_idx(name);
        if (idx < 0) return false;
        const Var &V = scopes[idx].at(name);
        cout << name << ":";
        if (V.type == Var::INT) cout << V.int_val;
        else cout << V.str_val;
        cout << '\n';
        return true;
    }
};

static inline void ltrim_inplace(string &s) {
    size_t i = 0; while (i < s.size() && isspace((unsigned char)s[i])) ++i; s.erase(0, i);
}
static inline void rtrim_inplace(string &s) {
    size_t i = s.size(); while (i > 0 && isspace((unsigned char)s[i-1])) --i; s.erase(i);
}
static inline void trim_inplace(string &s) { rtrim_inplace(s); ltrim_inplace(s); }

// Extract next non-space token (no quotes), return token and new index
static inline bool next_token(const string &s, size_t &pos, string &out) {
    while (pos < s.size() && isspace((unsigned char)s[pos])) ++pos;
    if (pos >= s.size()) return false;
    size_t start = pos;
    while (pos < s.size() && !isspace((unsigned char)s[pos])) ++pos;
    out.assign(s.begin() + start, s.begin() + pos);
    return true;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    long long n;
    if (!(cin >> n)) return 0;
    string dummy; getline(cin, dummy); // consume rest of line

    ScopeSimulator sim;

    string line;
    line.reserve(256);
    for (long long _i = 0; _i < n; ++_i) {
        if (!std::getline(cin, line)) break;
        // Trim right for safety
        rtrim_inplace(line);
        size_t pos = 0; string cmd;
        // Fetch command token
        if (!next_token(line, pos, cmd)) {
            cout << "Invalid operation\n";
            continue;
        }
        bool ok = false;
        if (cmd == "Indent") {
            sim.scopes.emplace_back();
            ok = true;
        } else if (cmd == "Dedent") {
            if (sim.scopes.size() <= 1) ok = false; else { sim.scopes.pop_back(); ok = true; }
        } else if (cmd == "Declare") {
            string type_tok, var_name;
            if (!next_token(line, pos, type_tok) || !next_token(line, pos, var_name)) {
                ok = false;
            } else {
                // For int: next token; for string: extract quoted literal from remaining
                // Skip spaces
                while (pos < line.size() && isspace((unsigned char)line[pos])) ++pos;
                if (type_tok == "int") {
                    string val_tok;
                    if (!next_token(line, pos, val_tok)) ok = false;
                    else ok = sim.declare_var(type_tok, var_name, val_tok);
                } else if (type_tok == "string") {
                    if (pos >= line.size() || line[pos] != '"') {
                        ok = false;
                    } else {
                        size_t endq = line.find('"', pos + 1);
                        if (endq == string::npos) ok = false;
                        else {
                            string quoted = line.substr(pos, endq - pos + 1);
                            ok = sim.declare_var(type_tok, var_name, quoted);
                        }
                    }
                } else {
                    ok = false;
                }
            }
        } else if (cmd == "Add") {
            string res, v1, v2;
            if (!next_token(line, pos, res) || !next_token(line, pos, v1) || !next_token(line, pos, v2)) {
                ok = false;
            } else {
                ok = sim.add_to_result(res, v1, v2);
            }
        } else if (cmd == "SelfAdd") {
            string name;
            if (!next_token(line, pos, name)) {
                ok = false;
            } else {
                // decide whether the next value is a quoted string or an int token
                while (pos < line.size() && isspace((unsigned char)line[pos])) ++pos;
                if (pos >= line.size()) {
                    ok = false;
                } else if (line[pos] == '"') {
                    size_t endq = line.find('"', pos + 1);
                    if (endq == string::npos) ok = false;
                    else {
                        string quoted = line.substr(pos, endq - pos + 1);
                        ok = sim.self_add(name, quoted, true);
                    }
                } else {
                    string val_tok; // integer literal token
                    if (!next_token(line, pos, val_tok)) ok = false;
                    else ok = sim.self_add(name, val_tok, false);
                }
            }
        } else if (cmd == "Print") {
            string name;
            if (!next_token(line, pos, name)) ok = false;
            else ok = sim.print_var(name);
        } else {
            ok = false;
        }

        if (!ok) {
            cout << "Invalid operation\n";
        }
    }
    return 0;
}
