%{
#include <stdlib.h>
#include <string.h>

#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

QJsonArray yydialogs; /* resulting dialogs */
QJsonArray yybitmaps;

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

static void unite(QJsonObject *obj1, QJsonObject *obj2)
{
    for (auto it = obj2->constBegin(); it != obj2->constEnd(); ++it)
        obj1->insert(it.key(), it.value());
}

%}

%code requires {
    class QJsonObject;
    class QJsonArray;
    class QString;
};

%union {
    int ival;
    QJsonObject *joval;
    QJsonArray *javal;
    QString *qsval;
}

%type<joval> control_parameters
%type<joval> control_parameters_extended
%type<joval> control_parameters_base
%type<joval> control_parameters_text
%type<joval> control_parameters_text_extended
%type<joval> auto3state_control
%type<joval> autocheckbox_control
%type<joval> autoradiobutton_control
%type<joval> checkbox_control
%type<joval> combobox_control
%type<joval> control_control
%type<joval> ctext_control
%type<joval> ctext_control_base
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
%type<joval> ctext_styles
%type<javal> optional_styles
%type<qsval> control_identifier
%type<qsval> common_identifier
%type<qsval> style_identifier
%type<qsval> class
%type<javal> controls
%type<javal> control_statements
%type<joval> dialog_statements
%type<joval> dialog_optional_statements
%type<joval> dialog
%type<joval> dialog_base
%type<joval> dialog_geometry
%type<joval> dialogex
%type<joval> dialogex_base
%type<joval> caption_statement
%type<joval> characteristics_statement
%type<joval> class_statement
%type<joval> exstyle_statement
%type<joval> font_statement
%type<joval> font_statement_base
%type<joval> icon
%type<joval> language_statement
%type<joval> menu_statement
%type<joval> style_statement
%type<joval> version_statement
%type<joval> dialog_statement
%type<ival> dialogex_helpid
%type<ival> font_statement_optional_number
%type<ival> icon_control_optional_number
%type<ival> style_optional_not

%token ACCELERATORS
%token AUTO3STATE
%token AUTOCHECKBOX
%token AUTORADIOBUTTON
%token BBEGIN
%token BITMAP
%token BUTTON
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
%token DESIGNINFO
%token DIALOG
%token DIALOGEX
%token DLGINIT
%token EDITTEXT
%token BEND
%token EXSTYLE
%token FONT
%token GROUPBOX
%token HTML
%token ICON
%token<qsval> IDENTIFIER
%token LANGUAGE
%token LISTBOX
%token LTEXT
%token MENU
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
%token<qsval> STRING_LITERAL
%token STRINGTABLE
%token STYLE
%token TEXTINCLUDE
%token TOOLBAR
%token VERSION
%token VERSIONINFO
%token WIGETS

%start rcfile

%%

/*
 * GENERAL RULE FOR RCFILE
 */

rcfile:
    rcfile resources
    | rcfile ignored_statement
    | ignored_statement
    | resources
    ;

ignored_statement:
    language_statement
    | textinclude
    ;

resources: resources resource | resource
    ;

resource:
    dialog
    | dialogex
    | bitmap
    | icon
    | toolbar
    | menu
    | accelerators
    | designinfo
    | dlginit
    | wiget
    | cursor
    ;

common_identifier:
    IDENTIFIER
    {
        $$ = $1;
    }
    | NUMBER
    {
        QString *s = new QString(QString::number($1));

        $$ = s;
    }
    | STRING_LITERAL
    {
        $$ = $1;
    }
    ;


/*
 * DIALOG and DIALOGEX
 */

dialog:
    dialog_base dialog_optional_statements control_statements
    {
        QJsonObject *o = $1;

        if ($2) {
            unite(o, $2);
            delete $2;
        }

        if ($3) {
            o->insert("children", *$3);
            delete $3;
        }

        yydialogs.append(*o);
        delete o;
    }
    ;

dialog_base:
    IDENTIFIER DIALOG dialog_geometry
    {
        QJsonObject *o = $3;
        o->insert("type", "DIALOG");
        o->insert("id", *$1);

        delete $1;

        $$ = o;
    }
    ;

dialogex:
    dialogex_base dialogex_helpid dialog_optional_statements control_statements
    {
        QJsonObject *o = $1;

        if ($2)
            o->insert("helpid", $2);

        if ($3) {
            unite(o, $3);
            delete $3;
        }

        if ($4) {
            o->insert("children", *$4);
            delete $4;
        }

        yydialogs.append(*o);
        delete o;
    }
    ;

dialogex_helpid:
    /* empty */
    {
        $$ = 0;
    }
    | COMMA NUMBER
    {
        $$ = $2;
    }
    ;

dialogex_base:
    IDENTIFIER DIALOGEX dialog_geometry
    {
        QJsonObject *o = $3;

        o->insert("type", "DIALOGEX");
        o->insert("id", *$1);

        delete $1;

        $$ = o;
    }
    ;

dialog_geometry:
    NUMBER COMMA NUMBER COMMA NUMBER COMMA NUMBER
    {
        QJsonObject *o = new QJsonObject {
            {"geometry", geometryObject($1, $3, $5, $7)}
        };

        $$ = o;
    }
    ;

dialog_optional_statements:
    /* empty */
    {
        $$ = 0;
    }
    | dialog_statements
    {
        $$ = $1;
    }
    ;

dialog_statements:
    dialog_statements dialog_statement
    {
        QJsonObject *o = $1;
        unite(o, $2);

        delete $2;

        $$ = o;
    }
    | dialog_statement
    {
        $$ = $1;
    }
    ;

dialog_statement:
    caption_statement { $$ = $1; }
    | style_statement { $$ = $1; }
    | characteristics_statement { $$ = $1; }
    | class_statement { $$ = $1; }
    | exstyle_statement { $$ = $1; }
    | font_statement { $$ = $1; }
    | language_statement { $$ = $1; }
    | menu_statement { $$ = $1; }
    | version_statement { $$ = $1; }
    ;


/*
 * Statements
 */

caption_statement:
    CAPTION STRING_LITERAL
    {
        QJsonObject *o = new QJsonObject {
            {"caption", *$2}
        };

        delete $2;

        $$ = o;
    }
    ;

characteristics_statement:
    CHARACTERISTICS NUMBER
    {
        QJsonObject *o = new QJsonObject {
            {"characteristics", $2}
        };

        $$ = o;
    }
    ;

class_statement:
    CLASS NUMBER
    {
        QJsonObject *o = new QJsonObject {
            {"class", $2}
        };

        $$ = o;
    }
    | CLASS STRING_LITERAL
    {
        QJsonObject *o = new QJsonObject {
            {"class", *$2}
        };

        delete $2;

        $$ = o;
    }
    ;

common_optional_statements:
    common_optional_statements common_optional_statement | common_optional_statement
    ;

common_optional_statement:
    /* empty */
    | characteristics_statement { delete $1; }
    | language_statement { delete $1; }
    | version_statement { delete $1; }
    ;

exstyle_statement:
    EXSTYLE styles
    {
        QJsonObject *o = new QJsonObject {
            {"exstyle", *$2}
        };

        delete $2;

        $$ = o;
    }
    ;

font_statement:
    font_statement_base font_statement_optional_number
    font_statement_optional_number font_statement_optional_number
    {
        if ($2 >= 0)
            $1->insert("weight", $2);

        if ($3 >= 0)
            $1->insert("italic", $3);

        if ($4 >= 0)
            $1->insert("charset", $4);

        QJsonObject *o = new QJsonObject {
            {"font", *$1}
        };

        delete $1;

        $$ = o;
    }
    ;

font_statement_base:
    FONT NUMBER COMMA STRING_LITERAL
    {
        QJsonObject *o = new QJsonObject {
            {"pointsize", $2},
            {"typeface", *$4}
        };

        delete $4;

        $$ = o;
    }
    ;

font_statement_optional_number:
    /* empty */
    {
        $$ = -1;
    }
    | COMMA NUMBER
    {
        $$ = $2;
    }
    ;

language_statement:
    LANGUAGE IDENTIFIER COMMA IDENTIFIER
    {
        QJsonObject *o = new QJsonObject {
            {"language", QJsonObject {
                {"language", *$2},
                {"sublanguage", *$4}}
            }
        };

        delete $2;
        delete $4;

        $$ = o;
    }
    ;

menu_statement:
    MENU STRING_LITERAL
    {
        QJsonObject *o = new QJsonObject {
            {"menu", *$2}
        };

        delete $2;

        $$ = o;
    }
    ;

style_statement:
    STYLE styles
    {
        QJsonObject *o = new QJsonObject {
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
            {"version", $2}
        };

        $$ = o;
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

optional_styles:
    /* empty */
    {
        $$ = 0;
    }
    | COMMA styles
    {
        $$ = $2;
    }
    ;


style_identifier:
    style_optional_not IDENTIFIER
    {
        if ($1)
            $2->prepend("NOT ");

        $$ = $2;
    }
    | style_optional_not NUMBER
    {
        QString *s = new QString;
        s->setNum($2);

        if ($1)
            s->prepend("NOT ");

        $$ = s;
    }
    ;

style_optional_not:
    {
        $$ = 0;
    }
    | NOT
    {
        $$ = 1;
    }
    ;


/*
 * Controls
 */

control_statements:
    BBEGIN controls BEND
    {
        $$ = $2;
    }
    ;

controls:
    /* empty */
    {
        $$ = 0;
    }
    | controls control
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
    CONTROL common_identifier COMMA control_identifier COMMA class COMMA styles
    COMMA NUMBER COMMA NUMBER COMMA NUMBER COMMA NUMBER optional_styles
    {
        QJsonObject *o = new QJsonObject {
            {"type", "CONTROL"},
            {"text", *$2},
            {"id", *$4},
            {"class", *$6},
            {"style", *$8},
            {"geometry", geometryObject($10, $12, $14, $16)}
        };

        if ($17) {
            o->insert("exstyle", *$17);

            delete $17;
        }

        delete $2;
        delete $4;
        delete $6;
        delete $8;

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
    EDITTEXT control_parameters_extended
    {
        QJsonObject *o = $2;
        o->insert("type", "EDITTEXT");

        $$ = o;
    }
    ;

ctext_control:
    ctext_control_base ctext_styles
    {
        QJsonObject *o = $1;

        if ($2) {
            unite(o, $2);

            delete $2;
        }

        $$ = o;
    }
    ;

ctext_control_base:
    CTEXT STRING_LITERAL COMMA control_parameters_base
    {
        QJsonObject *o = $4;
        o->insert("type", "CTEXT");
        o->insert("text", *$2);

        delete $2;

        $$ = o;
    }
    ;

ctext_styles:
    /* empty */
    {
        $$ = 0;
    }
    | COMMA styles optional_styles
    {
        QJsonObject *o = new QJsonObject {
            {"style", *$2}
        };

        delete $2;

        if ($3) {
            o->insert("exstyle", *$3);
            delete $3;
        }

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
    ICON IDENTIFIER COMMA IDENTIFIER COMMA NUMBER COMMA NUMBER
    icon_control_optional_number icon_control_optional_number optional_styles
    optional_styles
    {
        QJsonObject *o = new QJsonObject{
            {"type", "ICON"},
            {"text", *$2},
            {"id", *$4},
            {"geometry", geometryObject($6, $8, 0, 0)}
        };

        if ($11) {
            o->insert("style", *$11);
            delete $11;
        }

        if ($12) {
            o->insert("exstyle", *$12);
            delete $12;
        }

        delete $2;
        delete $4;

        $$ = o;
    }
    ;

icon_control_optional_number:
    /* empty */
    {
        $$ = 0;
    }
    | COMMA NUMBER
    {
        $$ = $2;
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
    LTEXT control_parameters_text_extended
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
    control_parameters_base optional_styles
    {
        QJsonObject *params = $1;

        if ($2) {
            params->insert("style", *$2);
            delete $2;
        }

        $$ = params;
    }
    ;

control_parameters_extended:
    control_parameters_base optional_styles optional_styles
    {
        QJsonObject *params = $1;

        if ($2) {
            params->insert("style", *$2);
            delete $2;
        }

        if ($3) {
            params->insert("extended_style", *$3);
            delete $3;
        }

        $$ = params;
    }
    ;

control_parameters_text:
    STRING_LITERAL COMMA control_parameters
    {
        QJsonObject *p = $3;
        p->insert("text", *$1);

        delete $1;

        $$ = p;
    };

control_parameters_text_extended:
    STRING_LITERAL COMMA control_parameters_extended
    {
        QJsonObject *p = $3;
        p->insert("text", *$1);

        delete $1;

        $$ = p;
    };

control_parameters_base:
    control_identifier COMMA NUMBER COMMA NUMBER COMMA NUMBER COMMA NUMBER
    {
        QJsonObject *params = new QJsonObject {
            {"id", *$1},
            {"geometry", geometryObject($3, $5, $7, $9)}
        };

        delete $1;

        $$ = params;
    }
    ;

control_identifier:
    IDENTIFIER
    {
        $$ = $1;
    }
    | NUMBER
    {
        QString *s = new QString(QString::number($1));

        $$ = s;
    }
    ;

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

/*
 * TEXTINCLUDE
 */

textinclude:
    /* empty */
    | NUMBER TEXTINCLUDE BBEGIN string_list BEND
    ;

string_list:
    string_list STRING_LITERAL | STRING_LITERAL;

/*
 * BITMAP
 */

bitmap:
    common_identifier BITMAP STRING_LITERAL
    {
        QJsonObject o {
            {"type", "BITMAP"},
            {"nameID", *$1},
            {"filename", *$3}
        };

        yybitmaps.append(o);

        delete $1;
        delete $3;
    }
    ;

/*
 * ICON
 */

icon:
    common_identifier ICON STRING_LITERAL
    {
        QJsonObject o {
            {"type", "ICON"},
            {"nameID", *$1},
            {"filename", *$3}
        };

        yybitmaps.append(o);

        delete $1;
        delete $3;
    }
    ;

/*
 * TOOLBAR
 */

toolbar:
    common_identifier TOOLBAR NUMBER COMMA NUMBER
    BBEGIN toolbar_statements BEND
    ;

toolbar_statements:
    toolbar_statements toolbar_statement | toolbar_statement
    ;

toolbar_statement:
    button_statement | SEPARATOR
    ;

button_statement:
    BUTTON IDENTIFIER
    ;

/*
 * MENU and POPUP
 */

menu:
    common_identifier MENU common_optional_statements
    BBEGIN menu_item_definitions BEND
    ;

menu_item_definitions:
    menu_item_definitions menu_item_definition | menu_item_definition
    ;

menu_item_definition:
    menuitem_statement | menuitem_separator_statement | popup
    ;

menuitem_statement:
    MENUITEM STRING_LITERAL COMMA common_identifier menuitem_optional_optionlist
    ;

menuitem_separator_statement:
    MENUITEM SEPARATOR
    ;

menuitem_optional_optionlist:
    /* empty */
    | menuitem_optionlist
    ;

menuitem_optionlist:
    menuitem_optionlist IDENTIFIER | IDENTIFIER
    ;

popup:
    POPUP STRING_LITERAL popup_optionlist BBEGIN menu_item_definitions BEND
    ;

popup_optionlist:
    menuitem_optional_optionlist
    ;

/*
 * ACCELERATORS
 */

accelerators:
    common_identifier ACCELERATORS common_optional_statements
    BBEGIN accelerators_list BEND
    ;

accelerators_list:
    accelerators_list accel | accel
    ;

accel:
    common_identifier COMMA common_identifier accel_type accel_options
    ;

accel_type:
    /* empty */
    | COMMA IDENTIFIER
    ;

accel_options:
    /* empty */
    | accel_options COMMA accel_option_identifier
    | COMMA accel_option_identifier
    ;

accel_option_identifier:
    IDENTIFIER | CONTROL
    ;

/*
 * DESIGNINFO
 */

designinfo:
    IDENTIFIER DESIGNINFO BBEGIN designinfo_list BEND
    ;

designinfo_list:
    /* empty */
    | designinfo_list designinfo_item
    | designinfo_item
    ;

designinfo_item:
    IDENTIFIER COMMA designinfo_item_type BBEGIN designinfo_item_properties BEND
    ;

designinfo_item_type:
    IDENTIFIER | DIALOG
    ;

designinfo_item_properties:
    designinfo_item_properties designinfo_item_property
    | designinfo_item_property
    ;

designinfo_item_property:
    IDENTIFIER COMMA NUMBER
    ;

/*
 * DLGINIT
 */

dlginit:
    IDENTIFIER DLGINIT BBEGIN dlginit_params BEND
    ;

dlginit_params:
    dlginit_params dlginit_param | dlginit_param
    ;

dlginit_param:
    dlginit_param COMMA common_identifier | common_identifier
    ;

/*
 * WIDGETS
 */

wiget:
    IDENTIFIER WIGETS STRING_LITERAL
    ;

/*
 * CURSOR
 */

cursor:
    common_identifier CURSOR STRING_LITERAL
    ;
