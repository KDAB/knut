%{

#include <stdlib.h>
#include <string.h>

#include <QString>

#include "rcparser.h"

int yylinenum = 1;

%}

%option noyywrap

%%

ACCELERATORS            { return ACCELERATORS; };
AUTO3STATE              { return AUTO3STATE; };
AUTOCHECKBOX            { return AUTOCHECKBOX; };
AUTORADIOBUTTON         { return AUTORADIOBUTTON; };
BEGIN                   { return BBEGIN; };
BITMAP                  { return BITMAP; };
BUTTON                  { return BUTTON; };
CAPTION                 { return CAPTION; };
CHARACTERISTICS         { return CHARACTERISTICS; };
CHECKBOX                { return CHECKBOX; };
CLASS                   { return CLASS; };
COMBOBOX                { return COMBOBOX; };
CONTROL                 { return CONTROL; };
CTEXT                   { return CTEXT; };
CURSOR                  { return CURSOR; };
DEFPUSHBUTTON           { return DEFPUSHBUTTON; };
DESIGNINFO              { return DESIGNINFO; };
DIALOG                  { return DIALOG; };
DIALOGEX                { return DIALOGEX; };
DLGINIT                 { return DLGINIT; };
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
MENUITEM                { return MENUITEM; };
NOT                     { return NOT; };
MESSAGETABLE            { return MESSAGETABLE; };
PNG                     { return PNG; };
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
TEXTINCLUDE             { return TEXTINCLUDE; };
TOOLBAR                 { return TOOLBAR; };
VERSION                 { return VERSION; };
VERSIONINFO             { return VERSIONINFO; };
WIGETS                  { return WIGETS; };
[A-Za-z_][A-Za-z0-9_]*  { yylval.qsval = new QString(yytext);
                          return IDENTIFIER; };
\"(\\.|\"\"|[^"])*\"    { QString text(yytext);
                          yylval.qsval = new QString(text.mid(1, text.count()-2));
                          return STRING_LITERAL; };
[+|-]?(0x)?[0-9A-Fa-f]+ { bool ok;
                          yylval.ival = QString(yytext).toInt(&ok, 0);
                          return NUMBER; };
[ \t]                   {};
\n                      { ++yylinenum; };
\|                      { return OPERATOR_OR; };
\/\/.*                  {};
^#.*                    { /* ignore directives*/ };
,                       { return COMMA; };

%%
