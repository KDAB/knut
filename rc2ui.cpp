/**********************************************************************
**
** Copyright (C) 2005-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "rc2ui.h"

#include <QDir>

/// some little helpers ///

void RC2UI::wi()
{
    for (int i = 0; i < indentation; i++)
        *out << "    ";
}

void RC2UI::indent()
{
    indentation++;
}

void RC2UI::undent()
{
    indentation--;
}

QString RC2UI::stripQM(const QString &string)
{
    return string.mid(1, string.length()-2);
}

QStringList RC2UI::splitStyles(const QString &styles, char sep)
{
    QString s = styles;
    QString style;
    QStringList l;
    while (s.indexOf(sep) > -1) {
        style = s.left(s.indexOf(sep));
        l << style.trimmed();
        s = s.right(s.length() - style.length() -1);
    }
    if (!s.isEmpty())
        l << s.trimmed();
    return l;
}

QString RC2UI::parseNext(QString &arg, char sep)
{
    QString next = arg.left(arg.indexOf(sep));
    arg = arg.right(arg.length() - next.length() - 1);
    return next.trimmed();
}

void RC2UI::writeClass(const QString &name)
{
    wi(); *out << "<class>" << name << "</class>" << endl;
}

void RC2UI::writeWidget(const QString &className, const QString &name)
{
    wi(); *out << "<widget class=\""
        << className << "\" name=\""
        << name << "\">"
        << endl;
}

void RC2UI::writeCString(const QString &name, const QString &value)
{
    wi(); *out << "<property name=\"" << name << "\">" << endl; indent();
    wi(); *out << "<cstring>" << value << "</cstring>" << endl; undent();
    wi(); *out << "</property>" << endl;
}

void RC2UI::writeString(const QString &name, const QString &value)
{
    wi(); *out << "<property name=\"" << name << "\">" << endl; indent();
    wi(); *out << "<string>" << value << "</string>" << endl; undent();
    wi(); *out << "</property>" << endl;
}

void RC2UI::writeRect(const QString &name, int x, int y, int w, int h)
{
    wi(); *out << "<property name=\"" << name << "\">" << endl; indent();
    wi(); *out << "<rect>" << endl; indent();
    wi(); *out << "<x>" << x << "</x>" << endl;
    wi(); *out << "<y>" << y << "</y>" << endl;
    wi(); *out << "<width>" << w << "</width>" << endl;
    wi(); *out << "<height>" << h << "</height>" << endl; undent();
    wi(); *out << "</rect>" << endl; undent();
    wi(); *out << "</property>" << endl;
}

void RC2UI::writeFont(const QString &family, int pointsize)
{
    wi(); *out << "<property name=\"font\">" << endl; indent();
    wi(); *out << "<font>" << endl; indent();
    wi(); *out << "<family>" << family << "</family>" << endl;
    wi(); *out << "<pointsize>" << pointsize << "</pointsize>" << endl; undent();
    wi(); *out << "</font>" << endl; undent();
    wi(); *out << "</property>" << endl;
}

void RC2UI::writeBool(const QString &name, bool value)
{
    wi(); *out << "<property name=\"" << name << "\">" << endl; indent();
    wi(); *out << "<bool>" << (value ? "true" : "false") << "</bool>" << endl; undent();
    wi(); *out << "</property>" << endl;
}

void RC2UI::writeNumber(const QString &name, int value)
{
    wi(); *out << "<property name=\"" << name << "\">" << endl; indent();
    wi(); *out << "<number>" << value << "</number>" << endl; undent();
    wi(); *out << "</property>" << endl;
}

void RC2UI::writeEnum(const QString &name, const QString &value)
{
    wi(); *out << "<property name=\"" << name << "\">" << endl; indent();
    wi(); *out << "<enum>" << value << "</enum>" << endl; undent();
    wi(); *out << "</property>" << endl;
}

void RC2UI::writeSet(const QString &name, const QString &value)
{
    wi(); *out << "<property name=\"" << name << "\">" << endl; indent();
    wi(); *out << "<set>" << value << "</set>" << endl; undent();
    wi(); *out << "</property>" << endl;
}

void RC2UI::writeStyles(const QStringList styles, bool isFrame)
{
    if (isFrame) {
        bool defineFrame = false;
        QString shadow = "QFrame::Plain";
        QString shape = "QFrame::StyledPanel";
        int width = 2;
        if (styles.contains( "WS_EX_STATICEDGE")) {
            shadow = "QFrame::Sunken";
            width = 2;
            defineFrame = true;
            shape = "QFrame::StyledPanel";
        }
        if (styles.contains( "WS_EX_CLIENTEDGE")) {
            shadow = "QFrame::Sunken";
            width = 2;
            defineFrame = true;
            shape = "QFrame::Panel";
        }
        if (styles.contains( "WS_EX_DLGMODALFRAME")) {
            shadow = "QFrame::Raised";
            width = 2;
            defineFrame = true;
            shape = "QFrame::Panel";
        }
        if (styles.contains( "WS_BORDER")) {
            shadow = "QFrame::Sunken";
            width = 1;
            defineFrame = true;
            shape = "QFrame::Panel";
        }

        if (defineFrame) {
            writeEnum("frameShape", shape);
            writeEnum("frameShadow", shadow);
            writeNumber("lineWidth", width);
        }
    }

    if (styles.contains("WS_DISABLED"))
        writeBool("enabled", false);
    if (styles.contains("WS_EX_ACCEPTFILES"))
        writeBool("acceptDrops", true);
    if (styles.contains("WS_EX_TRANSPARENT"))
        writeBool("autoMask", true);
    if (!styles.contains("WS_TABSTOP"))
        writeEnum("focusPolicy", "Qt::NoFocus");
}

/*!
  Constructs a RC2UI object
  */

RC2UI::RC2UI(QTextStream *input)
    : blockStart1("/////////////////////////////////////////////////////////////////////////////"),
        blockStart2("//")
{
    writeToFile = true;
    in = input;
    indentation = 0;
    out = 0;
}

/*!
  Destructs the RC2UI object
  */

RC2UI::~RC2UI()
{
}

/*!
  Parses the input stream and writes the output to files.
  */

bool RC2UI::parse()
{
    while (!in->atEnd()) {
        while (line != blockStart1 && !in->atEnd())
            line = in->readLine();
        if (in->atEnd())
            return false;
        while (line != blockStart2 && !in->atEnd())
            line = in->readLine();
        if (in->atEnd())
            return false;

        line = in->readLine();

        if (line.left(3) == "// " && !in->atEnd()) {
            QString type = line.right(line.length() - 3);
            if (in->readLine() == "//" && in->readLine().isEmpty() && !in->atEnd()) {
                if (type == "Dialog") {
                    if (!makeDialog())
                        return false;
                }
                /*
                   else if (type == "Bitmap") {
                   if (!makeBitmap())
                   return false;
                   } else if (type == "String Table") {
                   if (!makeStringTable())
                   return false;
                   } else if (type == "Accelerator") {
                   if (!makeAccelerator())
                   return false;
                   } else if (type == "Cursor") {
                   if (!makeCursor())
                   return false;
                   } else if (type == "HTML") {
                   if (!makeHTML())
                   return false;
                   } else if (type == "Icon") {
                   if (!makeIcon())
                   return false;
                   } else if (type == "Version") {
                   if (!makeVersion())
                   return false;
                   }
                   */
            }
        } else
            return false;
    }
    return true;
}

/*!
  Parses the input stream and writes the output in \a get.
  */

bool RC2UI::parse(QStringList &get)
{
    writeToFile = false;
    bool result = parse();
    get = target;
    return result;
}

/*!
  Retrieves a unique name starting with \a start
  */
QString RC2UI::useName(const QString &start)
{
    QString name = start;
    int id = 1;

    while (usedNames.contains( name)) {
        name = start + QString("%1").arg( id);
        id++;
    }

    usedNames.append(name);

    return name;
}


/*!
  Builds a number of UI dialog out of the current input stream
  */

bool RC2UI::makeDialog()
{
    line = in->readLine();
    do {
        QFile fileOut;
        QString buffer;
        int count;
        QString className;
        uint x, y, w, h;
        int endDesc;
        bool space = false;
        for (endDesc = 0; endDesc < line.length() ; endDesc++) {
            char c = line.at(endDesc).toLatin1();
            if (space && (c >= '0') && (c <= '9'))
                break;
            space = c == ' ';
        }

        QString desc = line.left(endDesc-1);
        line = line.right(line.length() - endDesc);

        className = parseNext(desc, ' ');

        count = sscanf(line.toLatin1().constData(), "%u, %u, %u, %u", &x, &y, &w, &h);

        if (!count && count == EOF)
            return false;

        char property[256];
        QStringList styles;
        QStringList extendedStyles;
        QString caption = "";
        QString baseClass = "";
        QString widgetType;
        QString widgetName;
        QString arguments;
        int pointsize = 10;
        QString fontname;
        do {
            line = "";
            do {
                if (in->atEnd())
                    return true;
                line += in->readLine();
            } while ( line[(int)line.length()-1] == '|' ||
                    line[(int)line.length()-1] == ',');
            count = sscanf(line.toLatin1().constData(), "%s", property);
            line = line.right(line.length() - line.indexOf(" ") -1);
            if (QString(property) == "STYLE") {
                styles = splitStyles(line);
                if (styles.contains( "WS_CAPTION"))
                    baseClass = "QDialog";
                else
                    baseClass = "QWidget";
            } else if (QString(property) == "CAPTION") {
                caption = stripQM(line);
            } else if (QString(property) == "FONT") {
                QString pt = line.left(line.indexOf(","));
                pointsize = pt.toInt();
                fontname = stripQM(line.right( line.length() - line.indexOf(",") - 2));
            }
        } while (line != "BEGIN");

        if (writeToFile) {

            QString outputFile = QString(className) + ".ui";
            fileOut.setFileName(outputFile);
            if (!fileOut.open(QIODevice::WriteOnly))
                qFatal("rc2ui: Could not open output file '%s'", outputFile.toLatin1().constData());
            out = new QTextStream(&fileOut);
            targetFiles.append(outputFile);
        } else {
            out = new QTextStream(&buffer, QIODevice::WriteOnly);
        }

        *out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
        *out << "<ui version=\"4.0\">" << endl;
        writeClass(className);
        writeWidget(baseClass, className);
        indent();
        writeRect("geometry", x, y, w, h);
        writeString("caption", caption);
        writeFont(fontname, pointsize);

        do {
            if (in->atEnd())
                return true;

            line = in->readLine().trimmed();
            if (line == "END")
                continue;

            widgetType = parseNext(line, ' ');
            arguments = line.trimmed();
            while ( arguments[(int)arguments.length()-1] == ',' ||
                    arguments[(int)arguments.length()-1] == '|')
                arguments += " "+in->readLine().trimmed();

            indent();

            WidgetType ID = IDUnknown;
            QString controlType;
            QString widgetID;
            QString widgetText;
            bool hasText = false;
            bool isControl = false;
            bool isFrame = false;

            if (widgetType == "PUSHBUTTON") {
                ID = IDPushButton;
                hasText = true;
            } else if (widgetType == "DEFPUSHBUTTON") {
                ID = IDPushButton;
                hasText = true;
            } else if (widgetType == "LTEXT") {
                ID = IDLabel;
                hasText = true;
            } else if (widgetType == "CTEXT") {
                ID = IDLabel;
                hasText = true;
            } else if (widgetType == "RTEXT") {
                ID = IDLabel;
                hasText = true;
            } else if (widgetType == "EDITTEXT") {
                ID = IDLineEdit;
            } else if (widgetType == "GROUPBOX") {
                ID = IDGroupBox;
                hasText = true;
            } else if (widgetType == "COMBOBOX") {
                ID = IDComboBox;
            } else if (widgetType == "LISTBOX") {
                ID = IDListBox;
            } else if (widgetType == "SCROLLBAR") {
                ID = IDScrollBar;
            } else if (widgetType == "CHECKBOX") {
                ID = IDCheckBox;
                hasText = true;
            } else if (widgetType == "RADIOBUTTON") {
                ID = IDRadioButton;
                hasText = true;
            } else if (widgetType == "CONTROL") {
                isControl = true;
                widgetText = stripQM(parseNext( arguments));
                widgetID = parseNext(arguments);
                controlType = stripQM(parseNext( arguments));
                styles = splitStyles(parseNext( arguments));

                if (controlType == "Static") {
                    ID = IDLabel;
                } else if (controlType == "Button") {
                    if ( styles.contains("BS_AUTOCHECKBOX") ||
                            styles.contains("BS_3STATE"))
                        ID = IDCheckBox;
                    else if (styles.contains("BS_AUTORADIOBUTTON"))
                        ID = IDRadioButton;
                } else if (controlType == "msctls_updown32") {
                    ID = IDSpinBox;
                } else if (controlType == "msctls_progress32") {
                    ID = IDProgressBar;
                } else if (controlType == "msctls_trackbar32") {
                    ID = IDSlider;
                } else if (controlType == "SysListView32") {
                    ID = IDIconView;
                } else if (controlType == "SysTreeView32") {
                    ID = IDListView;
                } else if (controlType == "SysTabControl32") {
                    ID = IDTabWidget;
                } else if (controlType == "SysAnimate32") {
                    ID = IDLabel;
                } else if (controlType == "RICHEDIT") {
                    ID = IDMultiLineEdit;
                } else if (controlType == "ComboBoxEx32") {
                    ID = IDComboBox;
                } else if (controlType == "") {
                    ID = IDCustom;
                } else {
                    ID = IDUnknown;
                }
            } else
                ID = IDUnknown;

            if (hasText)
                widgetText = stripQM(parseNext( arguments));

            if (isControl) {
                x = parseNext(arguments).toInt();
                y = parseNext(arguments).toInt();
                w = parseNext(arguments).toInt();
                h = parseNext(arguments).toInt();
            } else {
                widgetID = parseNext(arguments);
                x = parseNext(arguments).toInt();
                y = parseNext(arguments).toInt();
                w = parseNext(arguments).toInt();
                h = parseNext(arguments).toInt();
                styles.clear();
            }

            do {
                extendedStyles = splitStyles(parseNext(arguments));
                styles << extendedStyles;
            } while (arguments.indexOf(',') > -1);

            switch (ID) {
                case IDWidget:
                    break;
                case IDPushButton:
                    {
                        writeWidget("QPushButton", useName("PushButton_" + widgetID));
                        writeRect("geometry", x, y, w, h);
                        writeString("text", widgetText);
                        if (widgetType == "DEFPUSHBUTTON")
                            writeBool("default", true);
                    }
                    break;
                case IDLabel:
                    {
                        isFrame = true;
                        writeWidget("QLabel", useName("Label_" + widgetID));
                        writeRect("geometry", x,y,w,h);
                        writeString("text", widgetText);
                        QString align;
                        if (styles.contains("SS_BITMAP") && !styles.contains("SS_CENTERIMAGE"))
                            align += "Qt::AlignTop";
                        else
                            align += "Qt::AlignVCenter";
                        if (widgetType == "LTEXT") {
                            align += "|Qt::AlignLeft";
                        } else if ( widgetType == "CTEXT") {
                            align += "|Qt::AlignHCenter";
                        } else if ( widgetType == "RTEXT") {
                            align += "|Qt::AlignRight";
                        }
                        writeSet("alignment", align);
                    }
                    break;
                case IDCheckBox:
                    {
                        writeWidget("QCheckBox", useName("CheckBox_" + widgetID));
                        writeRect("geometry", x,y,w,h);
                        writeString("text", widgetText);
                        if (styles.contains( "BS_3STATE"))
                            writeBool("tristate", true);
                    }
                    break;
                case IDRadioButton:
                    {
                        writeWidget("QRadioButton", useName("RadioButton_" + widgetID));
                        writeRect("geometry", x,y,w,h);
                        writeString("text", widgetText);
                    }
                    break;
                case IDGroupBox:
                    {
                        writeWidget("QGroupBox", useName("GroupBox_" + widgetID));
                        writeRect("geometry", x,y,w,h);
                        writeString("title", widgetText);
                    }
                    break;
                case IDLineEdit:
                    {
                        if (!styles.contains("ES_MULTILINE")) {
                            writeWidget("QLineEdit", useName("LineEdit_" + widgetID));
                        } else {
                            writeWidget("QMultiLineEdit", useName("MultiLineEdit_" + widgetID));
                        }
                        writeRect("geometry", x,y,w,h);
                        QString align = "AlignTop";
                        if (styles.contains("ES_CENTER"))
                            align+="|AlignHCenter";
                        else if (styles.contains("ES_RIGHT"))
                            align+="|AlignRight";
                        else
                            align+="|AlignLeft";
                        writeSet("alignment", align);
                    }
                    break;
                case IDMultiLineEdit:
                    {
                        writeWidget("QMultiLineEdit", useName("MultiLineEdit_" + widgetID));
                        writeRect("geometry", x,y,w,h);
                    }
                    break;
                case IDIconView:
                    {
                        isFrame = true;
                        writeWidget("QIconView", useName("IconView_" + widgetID));
                        writeRect("geometry", x,y,w,h);
                        if (!styles.contains( "LVS_SINGLESEL"))
                            writeEnum("selectionMode", "Extended");
                        if (styles.contains( "LVS_NOLABELWRAP"))
                            writeBool("wordWrapIconText", false);
                    }
                    break;
                case IDListView:
                    {
                        isFrame = true;
                        writeWidget("QListView", useName("ListView_" + widgetID));
                        writeRect("geometry", x,y,w,h);
                        if (styles.contains( "TVS_LINESATROOT"))
                            writeBool("rootIsDecorated", true);
                        if (styles.contains( "TVS_FULLROWSELECT"))
                            writeBool("allColumnsShowFocus", true);
                    }
                    break;
                case IDProgressBar:
                    {
                        isFrame = true;
                        writeWidget("QProgressBar", useName("ProgressBar_" + widgetID));
                        writeRect("geometry", x,y,w,h);
                        if (styles.contains("TBS_VERT"))
                            writeEnum("orientation", "Vertical");
                        else
                            writeEnum("orientation", "Horizontal");
                    }
                    break;
                case IDTabWidget:
                    {
                        writeWidget("QTabWidget", useName("TabWidget_" + widgetID));
                        writeRect("geometry", x,y,w,h);
                        wi(); writeWidget("QWidget", "title"); indent();
                        wi(); *out << "<attribute>" << endl; indent();
                        wi(); *out << "<string>Tab1</string>" << endl; undent();
                        wi(); *out << "</attribute>" << endl; undent();
                        wi(); *out << "</widget>" << endl;
                    }
                    break;
                case IDSpinBox:
                    {
                        isFrame = true;
                        writeWidget("QSpinBox", useName("SpinBox_" + widgetID));
                        writeRect("geometry", x,y,w,h);
                    }
                    break;
                case IDSlider:
                    {
                        writeWidget("QSlider", useName("Slider_" + widgetID));
                        writeRect("geometry", x,y,w,h);
                        if (styles.contains("TBS_VERT"))
                            writeEnum("orientation", "Vertical");
                        else
                            writeEnum("orientation", "Horizontal");
                        if (!styles.contains("TBS_NOTICKS"))
                            writeEnum("tickmarks", "Left");
                    }
                    break;
                case IDComboBox:
                    {
                        writeWidget("QComboBox", useName("ComboBox_" + widgetID));
                        if (isControl)
                            writeRect("geometry", x,y,w,14);
                        else
                            writeRect("geometry", x,y,w,h);
                    }
                    break;
                case IDListBox:
                    {
                        isFrame = true;
                        writeWidget("QListBox", useName("ListBox_" + widgetID));
                        writeRect("geometry", x,y,w,h);
                        if (styles.contains("WS_HSCROLL"))
                            writeEnum("hScrollBarMode", "Auto");
                        else
                            writeEnum("hScrollBarMode", "AlwaysOff");
                        if (styles.contains("WS_VSCROLL"))
                            writeEnum("vScrollBarMode", "Auto");
                        else
                            writeEnum("vScrollBarMode", "AlwaysOff");
                        if (styles.contains("LBS_EXTENDEDSEL"))
                            writeEnum("selectionMode", "Extended");
                        else if (styles.contains("LBS_MULTIPLESEL"))
                            writeEnum("selectionMode", "Multi");
                        else if (styles.contains("LBS_NOSEL"))
                            writeEnum("selectionMode", "NoSelection");
                        else
                            writeEnum("selectionMode", "Single");
                        if (!styles.contains( "NO WS_BORDER"))
                            styles.append("WS_BORDER");
                    }
                    break;
                case IDScrollBar:
                    {
                        writeWidget("QScrollBar", useName("ScrollBar_" + widgetID));
                        writeRect("geometry", x,y,w,h);
                        if (styles.contains("SBS_VERT"))
                            writeEnum("orientation", "Vertical");
                        else
                            writeEnum("orientation", "Horizontal");
                    }
                    break;
                case IDCustom:
                    {
                        writeWidget("QLabel", useName("Custom_" + widgetID));
                        writeRect("geometry", x,y,w,h);
                        writeString("text", "Create a custom widget and place it here.");
                    }
                default:
                    {
                        writeWidget("QLabel", useName("Unknown_" + widgetID));
                        writeRect("geometry", x,y,w,h);
                        writeString("text", QString("No support for %1.").arg(controlType));
                    }
                    break;
            }

            writeStyles(styles, isFrame);

            styles.clear();

            undent();
            wi(); *out << "</widget>" << endl;
        } while (line != "END");

        undent();
        wi(); *out << "</widget>" << endl;
        *out << "</ui>" << endl;

        do {
            line = in->readLine();
        } while (line.isEmpty());

        if (!writeToFile)
            target.append(buffer);

        if (out) {
            delete out;
            out = 0;
        }
        fileOut.close();

    } while (line != blockStart1);

    return true;
}

/*! Not yet implemented
*/

bool RC2UI::makeBitmap()
{
    return true;
}

/*! Not yet implemented
*/

bool RC2UI::makeAccelerator()
{
    return true;
}

/*! Not yet implemented
*/

bool RC2UI::makeCursor()
{
    return true;
}

/*! Not yet implemented
*/

bool RC2UI::makeHTML()
{
    return true;
}

/*! Not yet implemented
*/

bool RC2UI::makeIcon()
{
    return true;
}

/*!
  Writes a stringtable from the input stream to a c++ header file.
  All strings are assigned using QT_TR_NOOP to enable easy translation.
  */

bool RC2UI::makeStringTable()
{
    if (!writeToFile)
        return true;

    QFile fileOut;
    line = in->readLine();
    do {
        char stringtable[256];
        char discard[12];
        sscanf(line.toLatin1().constData(), "%s %s", stringtable, discard);
        if (QString(stringtable) != "STRINGTABLE")
            return true;
        do {
            line = in->readLine();
        } while (line != "BEGIN");

        QString outputFile = QString(stringtable).toLower() + ".h";
        //FIXME if (outputFile) {
            fileOut.setFileName(outputFile);
            if (!fileOut.open(QIODevice::WriteOnly))
                qFatal("rc2ui: Could not open output file '%s'", outputFile.toLatin1().constData());
            out = new QTextStream(&fileOut);
        //}

        *out << "#ifndef STRINGTABLE_H" << endl;
        *out << "#define STRINGTABLE_H" << endl;
        *out << endl;
        *out << "#include <qstring.h>" << endl;
        *out << "#include <qobject.h>" << endl;
        *out << endl;

        QString ID;
        QString value;
        do {
            line = in->readLine().trimmed();
            if (line == "END")
                continue;

            ID = parseNext(line, ' ');
            value = parseNext(line).trimmed();

            *out << "static const QString " << ID << "= QT_TR_NOOP(" << value << ");" << endl;

        } while (line != "END");

        *out << endl;
        *out << "#endif // STRINGTABLE_H" << endl;

        do {
            line = in->readLine();
        } while (line.isEmpty());

        if (out) {
            delete out;
            out = 0;
        }
    } while (line != blockStart1);

    return true;
}

/*! Not yet implemented
*/

bool RC2UI::makeVersion()
{
    return true;
}
