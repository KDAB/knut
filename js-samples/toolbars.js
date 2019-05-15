function main() {
    var doc = result.createDocument("CreateToolbars")
	
    doc.content = "QToolbar *toolbar = nullptr;\n\n"
    for (var i = 0; i < toolbars.children.length; ++i) {
        var toolbarAction = toolbars.children[i]
        if (toolbarAction.isSeparator) {
             doc.content += "toolbar->addSeparator();\n"
        } else {
            doc.content += qsTr("toolbar->addAction(m_actionManager->GetAction(%1));\n").arg(toolbarAction.id)
        }
    }
}
