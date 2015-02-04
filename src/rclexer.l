%{

#include <stdlib.h>
#include <string.h>

#include <QString>

#include "rcparser.h"

extern int linenum;

%}

%option noyywrap

%%

ACCELERATORS            { return ACCELERATORS; };
AUTO3STATE              { return AUTO3STATE; };
AUTOCHECKBOX            { return AUTOCHECKBOX; };
AUTORADIOBUTTON         { return AUTORADIOBUTTON; };
BEGIN                   { return BBEGIN; };
BITMAP                  { return BITMAP; };
CAPTION                 { return CAPTION; };
CHARACTERISTICS         { return CHARACTERISTICS; };
CHECKBOX                { return CHECKBOX; };
CLASS                   { return CLASS; };
COMBOBOX                { return COMBOBOX; };
CONTROL                 { return CONTROL; };
CTEXT                   { return CTEXT; };
CURSOR                  { return CURSOR; };
DEFPUSHBUTTON           { return DEFPUSHBUTTON; };
DIALOG                  { return DIALOG; };
DIALOGEX                { return DIALOGEX; };
EDITTEXT                { return EDITTEXT; };
END                     { return BEND; };
EXSTYLE                 { return EXSTYLE; };
FONT                    { return FONT; };
GROUPBOX                { return GROUPBOX; };
HTML                    { return HTML; };
ICON                    { return ICON; };
LANGUAGE                { return LANGUAGE; };
LISTBOX                 { return LISTBOX; };
LTEXT                   { return LTEXT; };
MENU                    { return MENU; };
NOT                     { return NOT; };
MESSAGETABLE            { return MESSAGETABLE; };
POPUP                   { return POPUP; };
PUSHBOX                 { return PUSHBOX; };
PUSHBUTTON              { return PUSHBUTTON; };
RADIOBUTTON             { return RADIOBUTTON; };
RCDATA                  { return RCDATA; };
RTEXT                   { return RTEXT; };
SCROLLBAR               { return SCROLLBAR; };
SEPARATOR               { return SEPARATOR; };
STATE3                  { return STATE3; };
STRINGTABLE             { return STRINGTABLE; };
STYLE                   { return STYLE; };
VERSION                 { return VERSION; };
VERSIONINFO             { return VERSIONINFO; };
[A-Za-z_][A-Za-z0-9_]*  { yylval.qsval = new QString(yytext);
                          return IDENTIFIER; };
\"(\\.|[^"])*\"         { QString text(yytext);
                          yylval.qsval = new QString(text.mid(1, text.count()-2));
                          return STRING_LITERAL; };
[+|-]?(0x)?[0-9]+       { bool ok;
                          yylval.ival = QString(yytext).toInt(&ok, 0);
                          return NUMBER; };
[ \t]                   {};
\n                      { ++linenum; };
\|                      { return OPERATOR_OR; };
\/\/.*                  {};
^#.*                    { /* ignore directives*/ };
,                       { return COMMA; };

%%
