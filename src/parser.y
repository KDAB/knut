%{
#include <stdlib.h>
#include <string.h>

#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

extern int yylex(void);
extern void yyerror(const char *s);

static QJsonObject geometryObject(int x, int y, int width, int height)
{
    return QJsonObject {
        {"x", x},
        {"y", y},
        {"width", width},
        {"height", height},
    };
}

%}

%code requires {
    class QJsonObject;
    class QJsonArray;
    class QString;
};

%union {
    char *sval;
    int ival;
    QJsonObject *joval;
    QJsonArray *javal;
    QString *qsval;
}

%type<joval> control_parameters
%type<joval> control_parameters_text
%type<joval> auto3state_control
%type<joval> autocheckbox_control
%type<joval> autoradiobutton_control
%type<joval> checkbox_control
%type<joval> combobox_control
%type<joval> control_control
%type<joval> ctext_control
%type<joval> defpushbutton_control
%type<joval> edittext_control
%type<joval> groupbox_control
%type<joval> icon_control
%type<joval> listbox_control
%type<joval> ltext_control
%type<joval> pushbox_control
%type<joval> pushbutton_control
%type<joval> radiobutton_control
%type<joval> rtext_control
%type<joval> scrollbar_control
%type<joval> state3_control
%type<joval> control
%type<javal> styles
%type<qsval> control_text
%type<sval> class
%type<javal> controls
%type<javal> control_statements

%token ACCELERATORS
%token AUTO3STATE
%token AUTOCHECKBOX
%token AUTORADIOBUTTON
%token BBEGIN
%token BITMAP
%token CAPTION
%token CHARACTERISTICS
%token CHECKBOX
%token CLASS
%token COMBOBOX
%token COMMA
%token CONTROL
%token CTEXT
%token CURSOR
%token DEFPUSHBUTTON
%token DIALOG
%token DIALOGEX
%token DIRECTIVE
%token EDITTEXT
%token BEND
%token EXSTYLE
%token FONT
%token GROUPBOX
%token HTML
%token ICON
%token<sval> IDENTIFIER
%token LANGUAGE
%token LISTBOX
%token LTEXT
%token MENU
%token MENUEX
%token MENUITEM
%token MESSAGETABLE
%token<ival> NUMBER
%token OPERATOR_OR
%token POPUP
%token PUSHBOX
%token PUSHBUTTON
%token RADIOBUTTON
%token RCDATA
%token RTEXT
%token SCROLLBAR
%token STATE3
%token<sval> STRING_LITERAL
%token STRINGTABLE
%token STYLE
%token VERSION
%token VERSIONINFO

%%

dialogex:
    dialogex_params control_statements;

dialogex_params:
    IDENTIFIER DIALOGEX NUMBER COMMA NUMBER COMMA NUMBER COMMA NUMBER
    | dialogex COMMA NUMBER
    | dialogex statements;

statements:
    statements statement | statement;

control_statements:
    BBEGIN controls BEND
    {
        qDebug() << QJsonDocument(*$2).toJson();
        $$ = $2;
    }
    ;

controls:
    controls control
    {
        QJsonArray *a = $1;
        a->append(*$2);

        delete $2;

        $$ = a;
    }
    | control
    {
        QJsonArray *a = new QJsonArray;
        a->append(*$1);

        delete $1;

        $$ = a;
    }
    ;

statement:
    caption_statement
    | style_statement
    | characteristics_statement
    | class_statement
    | exstyle_statement
    | font_statement
    | language_statement
    | menu_statement
    | menuitem_statement
    | version_statement
    ;

caption_statement:
    CAPTION STRING_LITERAL;

characteristics_statement:
    CHARACTERISTICS NUMBER;

class_statement:
    CLASS NUMBER | CLASS STRING_LITERAL;

exstyle_statement:
    EXSTYLE styles;

font_statement:
   FONT NUMBER COMMA STRING_LITERAL COMMA NUMBER COMMA NUMBER COMMA NUMBER;

language_statement:
    LANGUAGE IDENTIFIER COMMA IDENTIFIER;

menu_statement:
    MENU STRING_LITERAL;

menuitem_statement:
    MENUITEM STRING_LITERAL COMMA NUMBER
    | MENUITEM STRING_LITERAL COMMA NUMBER COMMA optionlist;

style_statement:
    STYLE styles;

version_statement:
    VERSION NUMBER;

optionlist:
    optionlist IDENTIFIER | IDENTIFIER;

styles:
    styles OPERATOR_OR IDENTIFIER
    {
        QJsonArray *a = $$;
        a->append(QLatin1String($3));
        free($3);

        $$ = a;
    }
    | IDENTIFIER
    {
        QJsonArray *a = new QJsonArray;
        a->append(QLatin1String($1));
        free($1);

        $$ = a;
    };

control:
    auto3state_control { $$ = $1; }
    | autocheckbox_control { $$ = $1; }
    | autoradiobutton_control { $$ = $1; }
    | checkbox_control { $$ = $1; }
    | combobox_control { $$ = $1; }
    | control_control { $$ = $1; }
    | ctext_control { $$ = $1; }
    | defpushbutton_control { $$ = $1; }
    | edittext_control { $$ = $1; }
    | groupbox_control { $$ = $1; }
    | icon_control { $$ = $1; }
    | listbox_control { $$ = $1; }
    | ltext_control { $$ = $1; }
    | pushbox_control { $$ = $1; }
    | pushbutton_control { $$ = $1; }
    | radiobutton_control { $$ = $1; }
    | rtext_control { $$ = $1; }
    | scrollbar_control { $$ = $1; }
    | state3_control { $$ = $1; }
    ;

auto3state_control:
    AUTO3STATE control_parameters_text
    {
        QJsonObject *o = $2;
        o->insert(QStringLiteral("type"), QStringLiteral("AUTO3STATE"));

        $$ = o;
    }
    ;

autocheckbox_control:
    AUTOCHECKBOX control_parameters_text
    {
        QJsonObject *o = $2;
        o->insert(QStringLiteral("type"), QStringLiteral("AUTOCHECKBOX"));

        $$ = o;
    }
    ;

autoradiobutton_control:
    AUTORADIOBUTTON control_parameters_text
    {
        QJsonObject *o = $2;
        o->insert(QStringLiteral("type"), QStringLiteral("AUTORADIOBUTTON"));

        $$ = o;
    }
    ;

checkbox_control:
    CHECKBOX control_parameters_text
    {
        QJsonObject *o = $2;
        o->insert(QStringLiteral("type"), QStringLiteral("CHECKBOX"));

        $$ = o;
    }
    ;

combobox_control:
    COMBOBOX control_parameters
    {
        QJsonObject *o = $2;
        o->insert(QStringLiteral("type"), QStringLiteral("COMBOBOX"));

        $$ = o;
    }
    ;

control_control:
    CONTROL control_text COMMA IDENTIFIER COMMA class COMMA styles COMMA NUMBER COMMA NUMBER COMMA NUMBER COMMA NUMBER
    {
        QJsonObject *o = new QJsonObject;
        o->insert(QStringLiteral("type"), QStringLiteral("CONTROL"));
        o->insert(QStringLiteral("text"), *$2);
        o->insert(QStringLiteral("id"), QLatin1String($4));
        o->insert(QStringLiteral("class"), QLatin1String($6));
        o->insert(QStringLiteral("style"), *$8);
        o->insert(QStringLiteral("geometry"),
                    geometryObject($10, $12, $14, $16));

        free($4);
        free($6);

        delete $2;
        delete $8;

        $$ = o;
    }
    | control_control styles
    {
        QJsonObject *o = $1;

        QJsonArray a = o->value(QStringLiteral("style")).toArray();
        a += *$2;
        o->insert(QStringLiteral("style"), a);

        delete $2;

        $$ = o;
    }
    ;

defpushbutton_control:
    DEFPUSHBUTTON control_parameters_text
    {
        QJsonObject *o = $2;
        o->insert(QStringLiteral("type"), QStringLiteral("DEFPUSHBUTTON"));

        $$ = o;
    }
    ;

edittext_control:
    EDITTEXT control_parameters
    {
        QJsonObject *o = $2;
        o->insert(QStringLiteral("type"), QStringLiteral("EDITTEXT"));

        $$ = o;
    }
    ;

ctext_control:
    CTEXT control_parameters_text
    {
        QJsonObject *o = $2;
        o->insert(QStringLiteral("type"), QStringLiteral("CTEXT"));

        $$ = o;
    }
    ;

groupbox_control:
    GROUPBOX control_parameters_text
    {
        QJsonObject *o = $2;
        o->insert(QStringLiteral("type"), QStringLiteral("GROUPBOX"));

        $$ = o;
    }
    ;

icon_control:
    ICON STRING_LITERAL COMMA IDENTIFIER COMMA NUMBER COMMA NUMBER
    {
        QJsonObject *o = new QJsonObject;
        o->insert(QStringLiteral("type"), QStringLiteral("ICON"));
        o->insert(QStringLiteral("text"), QLatin1String($2));
        o->insert(QStringLiteral("id"), QLatin1String($4));
        o->insert(QStringLiteral("geometry"),
                    geometryObject($6, $8, 0, 0));

        free($2);
        free($4);

        $$ = o;
    }
    | icon_control COMMA NUMBER COMMA NUMBER
    {
        QJsonObject *o = $1;

        QJsonObject geometry = o->value(QStringLiteral("geometry")).toObject();

        o->insert(QStringLiteral("geometry"),
                geometryObject(geometry.value(QStringLiteral("x")).toInt(),
                geometry.value(QStringLiteral("y")).toInt(), $3, $5));

        $$ = o;
    }
    | icon_control styles
    {
        QJsonObject *o = $1;

        o->insert(QStringLiteral("style"), *$2);

        delete $2;

        $$ = o;
    }
    ;

listbox_control:
    LISTBOX control_parameters
    {
        QJsonObject *o = $2;
        o->insert(QStringLiteral("type"), QStringLiteral("LISTBOX"));

        $$ = o;
    }
    ;

ltext_control:
    LTEXT control_parameters_text
    {
        QJsonObject *o = $2;
        o->insert(QStringLiteral("type"), QStringLiteral("LTEXT"));

        $$ = o;
    }
    ;

pushbox_control:
    PUSHBOX control_parameters_text
    {
        QJsonObject *o = $2;
        o->insert(QStringLiteral("type"), QStringLiteral("PUSHBOX"));

        $$ = o;
    }
    ;

pushbutton_control:
    PUSHBUTTON control_parameters_text
    {
        QJsonObject *o = $2;
        o->insert(QStringLiteral("type"), QStringLiteral("PUSHBUTTON"));

        $$ = o;
    }
    ;

radiobutton_control:
    RADIOBUTTON control_parameters_text
    {
        QJsonObject *o = $2;
        o->insert(QStringLiteral("type"), QStringLiteral("RADIOBUTTON"));

        $$ = o;
    }
    ;

rtext_control:
    RTEXT control_parameters_text
    {
        QJsonObject *o = $2;
        o->insert(QStringLiteral("type"), QStringLiteral("RTEXT"));

        $$ = o;
    }
    ;

scrollbar_control:
    SCROLLBAR control_parameters
    {
        QJsonObject *o = $2;
        o->insert(QStringLiteral("type"), QStringLiteral("SCROLLBAR"));

        $$ = o;
    }
    ;

state3_control:
    STATE3 control_parameters_text
    {
        QJsonObject *o = $2;
        o->insert(QStringLiteral("type"), QStringLiteral("STATE3"));

        $$ = o;
    }
    ;

control_parameters:
    IDENTIFIER COMMA NUMBER COMMA NUMBER COMMA NUMBER COMMA NUMBER
    {
        QJsonObject *params = new QJsonObject;
        params->insert(QStringLiteral("id"), QLatin1String($1));
        params->insert(QStringLiteral("geometry"),
                        geometryObject($3, $5, $7, $9));

        $$ = params;
    }
    | control_parameters COMMA styles
    {
        QJsonObject *params = $1;

        params->insert(QStringLiteral("style"), *$3);

        delete $3;

        $$ = params;
    }
    ;

control_parameters_text:
    STRING_LITERAL COMMA control_parameters
    {
        QJsonObject *p = $3;
        p->insert(QStringLiteral("text"), $1);

        free($1);

        $$ = p;
    };

class:
    IDENTIFIER
    {
        $$ = $1;
    }
    | STRING_LITERAL
    {
        $$ = $1;
    }
    ;

control_text:
    IDENTIFIER
    {
        QString *s = new QString(QLatin1String($1));
        free($1);

        $$ = s;
    }
    | NUMBER
    {
        QString *s = new QString;
        s->setNum($1);

        $$ = s;
    }
    | STRING_LITERAL
    {
        QString *s = new QString(QLatin1String($1));
        free($1);

        $$ = s;
    }
    ;
