function main() {
    var doc = result.createDocument("CreateToolbars")

    doc.content = qsTr("QToolBar *toolbar = m_actionManager->CreateToolBar(%1, tr(\"title\")/*KDAB: need to find title*/, this);\n").arg(toolbar.id)
	doc.content += "addToolBar(toolbar);\n\n"
    for (var i = 0; i < toolbar.children.length; ++i) {
        var toolbarAction = toolbar.children[i]
        if (toolbarAction.isSeparator) {
             doc.content += "toolbar->addSeparator();\n"
        } else {
            doc.content += qsTr("toolbar->addAction(m_actionManager->GetAction(%1));\n").arg(toolbarAction.id)
        }
    }
    if (toolbar.needToGenerateActions) {
        doc = result.createDocument(qsTr("Create%1Action").arg(toolbar.id.replace("&", "")))
        doc.content += "\n"
        doc.content += qsTr("void MainWindow::Create%1Actions()\n").arg(toolbar.id.replace("&", ""))
        doc.content += "{\n"
        doc.content += "    QAction *action = nullptr;\n";
        for (var i = 0; i < toolbar.children.length; ++i) {
            var toolbarAction = toolbar.children[i]
            if (toolbarAction.iconName !== "") {
                doc.content += qsTr("    action = m_actionManager->CreateAction(%1, QString(), this);\n").arg(toolbarAction.id);
                if (toolbarAction.statusTip !== "")
                    doc.content += qsTr("    action->setStatusTip(tr(\"%1\"));\n").arg(toolbarAction.statusTip)
                if (toolbarAction.toolTip !== "")
                    doc.content += qsTr("    action->setToolTip(tr(\"%1\"));\n").arg(toolbarAction.toolTip)
                doc.content += qsTr("    action->setIcon(QIcon(QStringLiteral(\":/%1\")));\n").arg(toolbarAction.iconName)
            }
        }
        doc.content += "}\n\n"
    }
}
