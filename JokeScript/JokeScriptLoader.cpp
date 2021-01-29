/*
    JokeScript

    Copyright (c) 2021 on-keyday

    This software is released under the MIT License.
    http://opensource.org/licenses/mit-license.php

*/

#include"JokeScriptLoader.h"
#include<iostream>

using namespace jokescript;

jokescript::JokeFile::~JokeFile() {
    void (*tmp)(char*);
    tmp = (void(*)(char*))free;
    lines.remove_each(tmp);
    loglines.remove_each(tmp);
}

JokeFile* CCNV jokescript::LoadJoke(const char* filename, JokeLogger* log) {
    if (!filename)return nullptr;

    JokeFile* ret = nullptr;
    FILE* file = nullptr;
    EasyVector<char> nowline;
    EasyVector<char*> lines;


    fopen_s(&file, filename, "r");
    if (!file) {
        AddJokeSysErr(log, "file \"*\" can't open", filename);
        return nullptr;
    }


    char* file_copy = nullptr;
    file_copy = StringFilter() = filename;
    if (!file_copy) {
        AddJokeSysErr(log, "memory is full", nullptr);
        fclose(file);
        return nullptr;
    }

    while (1) {
        int add = fgetc(file);

        if (add == EOF) {
            break;
        }
        if (add == '\n') {
            lines.add_nz(nowline.get_raw_z());
            nowline.init();
            continue;
        }
        nowline.add(add);
    }

    fclose(file);

    try {
        ret = new JokeFile;
    }
    catch (...) {
        void (*tmp)(char*);
        tmp = (void(*)(char*))free;
        lines.remove_each(tmp);
        free(file_copy);
        AddJokeSysErr(log, "memory is full", nullptr);
        return nullptr;
    }
    ret->filename = file_copy;
    ret->lines = std::move(lines);

    return ret;
}

bool CCNV jokescript::ParseJoke(JokeFile* file, JokeLogger* log) {
    if (!file)return false;
    unsigned long long line = 0;
    unsigned long long pos = 0;
    bool spaced = false;
    char past_not_sp = 0;
    EasyVector<char*>& lines = file->lines;
    EasyVector<char*> loglines;
    EasyVector<char> nowlogline;
    enum class commentf {
        none,
        c,
        str_d,
        str_s
    }phase = commentf::none;
    while (1) {
        auto nowline = lines[line];
        if (!nowline)break;
        while (nowline[pos]) {
            char c = nowline[pos];
            if (phase == commentf::none) {
                if (c == ' ' || c == '\t') {
                    pos++;
                    spaced = true;
                    continue;
                }
                if (spaced && pos && (isalnum((unsigned char)past_not_sp) && (isalnum((unsigned char)c) || c == '\"' || c == '\''))) {
                    nowlogline.add(' ');
                }
                past_not_sp = c;
                spaced = false;
                if (c == '/' && nowline[pos + 1] == '/') {
                    break;
                }
                if (c == '/' && nowline[pos + 1] == '*') {
                    phase = commentf::c;
                    pos += 2;
                    continue;
                }
                if (c == '\"') {
                    phase = commentf::str_d;
                }
                if (c == '\'') {
                    phase = commentf::str_s;
                }
                if (c == '{') {
                    loglines.add_nz(nowlogline.get_raw_z());
                    nowlogline.init();
                }
                if (!nowlogline[0]) {
                    file->logtoreal_line.add(line+1);
                    file->logtoreal_pos.add(pos+1);
                }
                nowlogline.add(c);
                if (c == ';' || c == '{' || c == '}') {
                    loglines.add_nz(nowlogline.get_raw_z());
                    nowlogline.init();
                }
            }
            else if (phase == commentf::c) {
                if (c == '*' && nowline[pos + 1] == '/') {
                    phase = commentf::none;
                    pos += 2;
                    continue;
                }
            }
            else if (phase == commentf::str_d) {
                if (c == '\"' && nowline[pos - 1] != '\\') {
                    phase = commentf::none;
                }
                nowlogline.add(c);
            }
            else if (phase == commentf::str_s) {
                if (c == '\'' && nowline[pos - 1] != '\\') {
                    phase = commentf::none;
                }
                nowlogline.add(c);
            }
            pos++;
        }
        line++;
        pos = 0;
    }
    if (nowlogline[0] != '\0') {
        void (*tmp)(char*);
        tmp = (void(*)(char*))free;
        loglines.remove_each(tmp);
        const char* msg = nullptr;
        if (phase == commentf::c) {
            msg = "on comment.";
        }
        else if (phase == commentf::none) {
            msg = "on statement or expression. statement and expression have to end with \';\'.";
        }
        else if (phase == commentf::str_d) {
            msg = "without end quotation mark \".";
        }
        else {
            msg = "without end quotation mark \'.";
        }
        AddJokeSynErr(log, "unexpected end of file *", msg, 1, 1);
        return false;
    }
    file->loglines = std::move(loglines);
    return true;
}
