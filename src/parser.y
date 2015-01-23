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
%type<qsval> style_identifier
%type<sval> class
%type<javal> controls
%type<javal> control_statements
%type<javal> optionlist
%type<javal> statements
%type<javal> dialogs
%type<joval> dialogex
%type<joval> dialogex_params
%type<joval> caption_statement
%type<joval> characteristics_statement
%type<joval> class_statement
%type<joval> exstyle_statement
%type<joval> font_statement
%type<joval> language_statement
%type<joval> menu_statement
%type<joval> menuitem_statement
%type<joval> menuitem_separator_statement
%type<joval> style_statement
%type<joval> version_statement
%type<joval> statement

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
%token NOT
%token<ival> NUMBER
%token OPERATOR_OR
%token POPUP
%token PUSHBOX
%token PUSHBUTTON
%token RADIOBUTTON
%token RCDATA
%token RTEXT
%token SCROLLBAR
%token SEPARATOR
%token STATE3
%token<sval> STRING_LITERAL
%token STRINGTABLE
%token STYLE
%token VERSION
%token VERSIONINFO

%%

rcfile:
    dialogs
    {
        qDebug() << QJsonDocument(*$1).toJson();
    }
    ;

dialogs:
    dialogs dialogex
    {
        QJsonArray *a = $1;
        a->append(*$2);

        delete $2;

        $$ = a;
    }
    | dialogex
    {
        QJsonArray *a = new QJsonArray {*$1};

        delete $1;

        $$ = a;
    }
    ;

dialogex:
    dialogex_params control_statements
    {
        QJsonObject *o = $1;

        QJsonArray *controls = $2;

        QJsonObject children;

        Q_FOREACH (const QJsonValue &obj, *controls) {
            QJsonObject control = obj.toObject();
            children.insert(control.value("id").toString(), control);
        }

        o->insert("children", children);

        delete $2;
    }

dialogex_params:
    IDENTIFIER DIALOGEX NUMBER COMMA NUMBER COMMA NUMBER COMMA NUMBER
    {
        QJsonObject *o = new QJsonObject {
            {"id", $1},
            {"type", "DIALOGEX"},
            {"geometry", geometryObject($3, $5, $7, $9)}
        };

        free($1);

        $$ = o;
    }
    | dialogex_params COMMA NUMBER
    {
        QJsonObject *o = $1;
        o->insert("helpid", $3);

        $$ = o;
    }
    | dialogex_params statements
    {
        QJsonObject *o = $1;

        Q_FOREACH (const QJsonValue &value, *$2) {
            QJsonObject s = value.toObject();
            o->insert(s.value("type").toString(), s);
        }

        delete $2;
        $$ = o;
    }
    ;

statements:
    statements statement
    {
        QJsonArray *a = $1;
        a->append(*$2);

        delete $2;

        $$ = a;
    }
    | statement
    {
        QJsonArray *a = new QJsonArray { *$1 };

        delete $1;

        $$ = a;
    }
    ;

control_statements:
    BBEGIN controls BEND
    {
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
    caption_statement { $$ = $1; }
    | style_statement { $$ = $1; }
    | characteristics_statement { $$ = $1; }
    | class_statement { $$ = $1; }
    | exstyle_statement { $$ = $1; }
    | font_statement { $$ = $1; }
    | language_statement { $$ = $1; }
    | menu_statement { $$ = $1; }
    | menuitem_statement { $$ = $1; }
    | menuitem_separator_statement { $$ = $1; }
    | version_statement { $$ = $1; }
    ;

caption_statement:
    CAPTION STRING_LITERAL
    {
        QJsonObject *o = new QJsonObject {
            {"type", "CAPTION"},
            {"text", $2}
        };

        free($2);

        $$ = o;
    }
    ;

characteristics_statement:
    CHARACTERISTICS NUMBER
    {
        QJsonObject *o = new QJsonObject {
            {"type", "CHARACTERISTICS"},
            {"dword", $2}
        };

        $$ = o;
    }
    ;

class_statement:
    CLASS NUMBER
    {
        QJsonObject *o = new QJsonObject {
            {"type", "CLASS"},
            {"class", $2}
        };

        $$ = o;
    }
    | CLASS STRING_LITERAL
    {
        QJsonObject *o = new QJsonObject {
            {"type", "CLASS"},
            {"class", $2}
        };

        free($2);

        $$ = o;
    }
    ;

exstyle_statement:
    EXSTYLE styles
    {
        QJsonObject *o = new QJsonObject {
            {"type", "EXSTYLE"},
            {"style", *$2}
        };

        delete $2;

        $$ = o;
    }
    ;

font_statement:
    FONT NUMBER COMMA STRING_LITERAL COMMA NUMBER COMMA NUMBER COMMA NUMBER
    {
        QJsonObject *o = new QJsonObject {
            {"type", "FONT"},
            {"pointsize", $2},
            {"typeface", $4},
            {"weight", $6},
            {"italic", static_cast<bool>($6)},
            {"charset", $8}
        };

        free($4);

        $$ = o;
    }
    ;

language_statement:
    LANGUAGE IDENTIFIER COMMA IDENTIFIER
    {
        QJsonObject *o = new QJsonObject {
            {"type", "LANGUAGE"},
            {"language", $2},
            {"sublanguage", $4}
        };

        free($2);
        free($4);

        $$ = o;
    }
    ;

menu_statement:
    MENU STRING_LITERAL
    {
        QJsonObject *o = new QJsonObject {
            {"type", "MENU"},
            {"menuname", $2}
        };

        free($2);

        $$ = o;
    }
    ;

menuitem_statement:
    MENUITEM STRING_LITERAL COMMA NUMBER
    {
        QJsonObject *o = new QJsonObject {
            {"type", "MENUITEM"},
            {"text", $2},
            {"result", $4}
        };

        free($2);

        $$ = o;
    }
    | menuitem_statement optionlist
    {
        QJsonObject *o = $1;
        o->insert("optionlist", *$2);

        delete $2;

        $$ = o;
    }
    ;

menuitem_separator_statement:
    MENUITEM SEPARATOR
    {
        QJsonObject *o = new QJsonObject {
            {"type", "MENUITEM_SEPARATOR"}
        };

        $$ = o;
    }
    ;

style_statement:
    STYLE styles
    {
        QJsonObject *o = new QJsonObject {
            {"type", "STYLE"},
            {"style", *$2}
        };

        delete $2;

        $$ = o;
    }
    ;

version_statement:
    VERSION NUMBER
    {
        QJsonObject *o = new QJsonObject {
            {"type", "VERSION"},
            {"dword", $2}
        };

        $$ = o;
    }
    ;

optionlist:
    optionlist IDENTIFIER
    {
        QJsonArray *a = $1;
        a->append($2);

        free($2);

        $$ = a;
    }
    | IDENTIFIER
    {
        QJsonArray *a = new QJsonArray {$1};

        free($1);

        $$ = a;
    }
    ;

styles:
    styles OPERATOR_OR style_identifier
    {
        QJsonArray *a = $$;
        a->append(*$3);

        delete $3;

        $$ = a;
    }
    | style_identifier
    {
        QJsonArray *a = new QJsonArray {*$1};

        delete $1;

        $$ = a;
    }
    ;

style_identifier:
    IDENTIFIER
    {
        QString *s = new QString {$1};

        free($1);

        $$ = s;
    }
    | NOT style_identifier
    {
        QString *s = $2;
        s->prepend("NOT ");

        $$ = s;
    }
    ;


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
        o->insert("type", "AUTO3STATE");

        $$ = o;
    }
    ;

autocheckbox_control:
    AUTOCHECKBOX control_parameters_text
    {
        QJsonObject *o = $2;
        o->insert("type", "AUTOCHECKBOX");

        $$ = o;
    }
    ;

autoradiobutton_control:
    AUTORADIOBUTTON control_parameters_text
    {
        QJsonObject *o = $2;
        o->insert("type", "AUTORADIOBUTTON");

        $$ = o;
    }
    ;

checkbox_control:
    CHECKBOX control_parameters_text
    {
        QJsonObject *o = $2;
        o->insert("type", "CHECKBOX");

        $$ = o;
    }
    ;

combobox_control:
    COMBOBOX control_parameters
    {
        QJsonObject *o = $2;
        o->insert("type", "COMBOBOX");

        $$ = o;
    }
    ;

control_control:
    CONTROL control_text COMMA IDENTIFIER COMMA class COMMA styles COMMA NUMBER COMMA NUMBER COMMA NUMBER COMMA NUMBER
    {
        QJsonObject *o = new QJsonObject {
            {"type", "CONTROL"},
            {"text", *$2},
            {"id", $4},
            {"class", $6},
            {"style", *$8},
            {"geometry", geometryObject($10, $12, $14, $16)}
        };

        free($4);
        free($6);

        delete $2;
        delete $8;

        $$ = o;
    }
    | control_control styles
    {
        QJsonObject *o = $1;

        QJsonArray a = o->value("style").toArray();
        a += *$2;
        o->insert("style", a);

        delete $2;

        $$ = o;
    }
    ;

defpushbutton_control:
    DEFPUSHBUTTON control_parameters_text
    {
        QJsonObject *o = $2;
        o->insert("type", "DEFPUSHBUTTON");

        $$ = o;
    }
    ;

edittext_control:
    EDITTEXT control_parameters
    {
        QJsonObject *o = $2;
        o->insert("type", "EDITTEXT");

        $$ = o;
    }
    ;

ctext_control:
    CTEXT control_parameters_text
    {
        QJsonObject *o = $2;
        o->insert("type", "CTEXT");

        $$ = o;
    }
    ;

groupbox_control:
    GROUPBOX control_parameters_text
    {
        QJsonObject *o = $2;
        o->insert("type", "GROUPBOX");

        $$ = o;
    }
    ;

icon_control:
    ICON STRING_LITERAL COMMA IDENTIFIER COMMA NUMBER COMMA NUMBER
    {
        QJsonObject *o = new QJsonObject{
            {"type", "ICON"},
            {"text", $2},
            {"id", $4},
            {"geometry", geometryObject($6, $8, 0, 0)}
        };

        free($2);
        free($4);

        $$ = o;
    }
    | icon_control COMMA NUMBER COMMA NUMBER
    {
        QJsonObject *o = $1;

        QJsonObject geometry = o->value("geometry").toObject();

        o->insert("geometry",
                geometryObject(geometry.value("x").toInt(),
                geometry.value("y").toInt(), $3, $5));

        $$ = o;
    }
    | icon_control styles
    {
        QJsonObject *o = $1;

        o->insert("style", *$2);

        delete $2;

        $$ = o;
    }
    ;

listbox_control:
    LISTBOX control_parameters
    {
        QJsonObject *o = $2;
        o->insert("type", "LISTBOX");

        $$ = o;
    }
    ;

ltext_control:
    LTEXT control_parameters_text
    {
        QJsonObject *o = $2;
        o->insert("type", "LTEXT");

        $$ = o;
    }
    ;

pushbox_control:
    PUSHBOX control_parameters_text
    {
        QJsonObject *o = $2;
        o->insert("type", "PUSHBOX");

        $$ = o;
    }
    ;

pushbutton_control:
    PUSHBUTTON control_parameters_text
    {
        QJsonObject *o = $2;
        o->insert("type", "PUSHBUTTON");

        $$ = o;
    }
    ;

radiobutton_control:
    RADIOBUTTON control_parameters_text
    {
        QJsonObject *o = $2;
        o->insert("type", "RADIOBUTTON");

        $$ = o;
    }
    ;

rtext_control:
    RTEXT control_parameters_text
    {
        QJsonObject *o = $2;
        o->insert("type", "RTEXT");

        $$ = o;
    }
    ;

scrollbar_control:
    SCROLLBAR control_parameters
    {
        QJsonObject *o = $2;
        o->insert("type", "SCROLLBAR");

        $$ = o;
    }
    ;

state3_control:
    STATE3 control_parameters_text
    {
        QJsonObject *o = $2;
        o->insert("type", "STATE3");

        $$ = o;
    }
    ;

control_parameters:
    IDENTIFIER COMMA NUMBER COMMA NUMBER COMMA NUMBER COMMA NUMBER
    {
        QJsonObject *params = new QJsonObject {
            {"id", $1},
            {"geometry", geometryObject($3, $5, $7, $9)}
        };

        $$ = params;
    }
    | control_parameters COMMA styles
    {
        QJsonObject *params = $1;

        params->insert("style", *$3);

        delete $3;

        $$ = params;
    }
    ;

control_parameters_text:
    STRING_LITERAL COMMA control_parameters
    {
        QJsonObject *p = $3;
        p->insert("text", $1);

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
        QString *s = new QString($1);
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
        QString *s = new QString($1);
        free($1);

        $$ = s;
    }
    ;
