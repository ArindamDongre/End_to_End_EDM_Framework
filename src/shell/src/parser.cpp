#include "../include/parser.hpp"
#include <vector>
#include <cctype>    // isspace
using namespace std;

namespace{
    enum class TokenType{
        WORD,
        input_redir,
        output_redir,
        PIPE,
        BACKGROUND
    };

    struct Token{
        TokenType type;
        string value;
    };

    bool tokenize(const string &line, vector<Token> &tokens, string &error){
        tokens.clear();
        string current;
        bool in_single=false;
        bool in_double=false;

        auto push_word_if_any=[&](){
            if(!current.empty()){
                tokens.push_back({TokenType::WORD, current});
                current.clear();
            }
        };

        for(size_t i=0; i<line.size();i++){
            char c=line[i];
            
            if (in_single) {
                if (c == '\'') {
                    // end of '...'
                    in_single = false;
                } else {
                    current.push_back(c);
                }
                continue;
            }

            if (in_double) {
                if (c == '"') {
                    // end of "..."
                    in_double = false;
                } else {
                    current.push_back(c);
                }
                continue;
            }

            // not currently inside any quotes
            if (c == '\'') {
                in_single = true;
                continue;
            }

            if (c == '"') {
                in_double = true;
                continue;
            }

            if (isspace(static_cast<unsigned char>(c))) {
                // whitespace ends current word
                push_word_if_any();
                continue;
            }

            if (c == '<' || c == '>' || c == '|' || c == '&') {
                // first, finish any pending word
                push_word_if_any();
                
                TokenType ttype;
                if (c == '<')      ttype = TokenType::input_redir;
                else if (c == '>') ttype = TokenType::output_redir;
                else if (c == '|') ttype = TokenType::PIPE;
                else               ttype = TokenType::BACKGROUND; // &

                tokens.push_back(Token{ttype, string(1, c)});
                continue;
            }

            // normal character, part of a WORD
            current.push_back(c);
        }

        // end of line
        if (in_single || in_double) {
            error = "Unterminated quote";
            return false;
        }

        // flush last word
        if (!current.empty()) {
            tokens.push_back(Token{TokenType::WORD, current});
        }

        return true;
    }

    bool parse_simple_command(const vector<Token> &tokens, size_t begin, size_t end, SimpleCommand &out, string &error){
        out = SimpleCommand{}; // reset

        bool expect_word_for_redir = false;
        enum class RedirSide { NONE, IN, OUT };
        RedirSide redir_side = RedirSide::NONE;

        for (size_t i = begin; i < end; ++i) {
            const Token &tok = tokens[i];

            if (expect_word_for_redir) {
                if (tok.type != TokenType::WORD) {
                    error = "Redirection operator must be followed by a filename";
                    return false;
                }

                if (redir_side == RedirSide::IN) {
                    if (!out.input_redirection.empty()) {
                        error = "Multiple input redirections not allowed";
                        return false;
                    }
                    out.input_redirection = tok.value;
                } else if (redir_side == RedirSide::OUT) {
                    if (!out.output_redirection.empty()) {
                        error = "Multiple output redirections not allowed";
                        return false;
                    }
                    out.output_redirection = tok.value;
                }

                expect_word_for_redir = false;
                redir_side = RedirSide::NONE;
                continue;
            }

            // not currently expecting a filename for redirection
            switch (tok.type) {
                case TokenType::WORD:
                    out.argv.push_back(tok.value);
                    break;

                case TokenType::input_redir:
                    expect_word_for_redir = true;
                    redir_side = RedirSide::IN;
                    break;

                case TokenType::output_redir:
                    expect_word_for_redir = true;
                    redir_side = RedirSide::OUT;
                    break;

                case TokenType::PIPE:
                case TokenType::BACKGROUND:
                    // these should be handled outside this function
                    error = "Unexpected token inside simple command";
                    return false;
            }
        }

        if (expect_word_for_redir) {
            error = "Redirection operator without filename";
            return false;
        }

        if (out.argv.empty()) {
            error = "Empty command";
            return false;
        }

        return true;
    }

    
}

// Top-level parse_command
bool parse_command(const string &line, Command &out, string &error){

    out = Command{}; // reset
    out.original_line = line;

    vector<Token> tokens;
    if (!tokenize(line, tokens, error)) {
        return false;
    }

    if (tokens.empty()) {
        error = "Empty input";
        return false;
    }

    // Handle background '&' at the end
    bool background = false;
    if (tokens.back().type == TokenType::BACKGROUND) {
        background = true;
        tokens.pop_back();
        if (tokens.empty()) {
            error = "Background '&' with no command";
            return false;
        }
    }

    size_t segment_start = 0;
    const size_t n = tokens.size();
    for (size_t i = 0; i <= n; ++i) {
        const bool at_end = (i == n);
        const bool is_pipe = (!at_end && tokens[i].type == TokenType::PIPE);
        if (!at_end && !is_pipe) {
            continue;
        }

        if (i == segment_start) {
            error = "Pipe '|' cannot be at the beginning or end";
            return false;
        }

        SimpleCommand simple;
        if (!parse_simple_command(tokens, segment_start, i, simple, error)) {
            return false;
        }
        out.commands.push_back(simple);
        segment_start = i + 1;
    }

    if (out.commands.empty()) {
        error = "Empty command";
        return false;
    }

    out.background = background;
    return true;
}
