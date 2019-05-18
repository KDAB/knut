function main() {
    var doc = result.createDocument("CreateToolbars")

    doc.content = qsTr("QToolBar *toolbar = m_actionManager->CreateToolBar(%1);\n").arg(toolbar.id)
	doc.content += "addToolBar(toolbar);\n\n"
    for (var i = 0; i < toolbar.children.length; ++i) {
        var toolbarAction = toolbar.children[i]
        if (toolbarAction.isSeparator) {
             doc.content += "toolbar->addSeparator();\n"
        } else {
            doc.content += qsTr("toolbar->addAction(m_actionManager->GetAction(%1));\n").arg(toolbarAction.id)
        }
    }
}
