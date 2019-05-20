function main() {
	var doc = result.createDocument("CreateActions")
	
    doc.content = "QAction *action = nullptr;\n\n"
	for (var i = 0; i < actions.length; ++i) {
		var action = actions[i]
        doc.content += qsTr("action = m_actionManager->CreateAction(%1, tr(\"%2\"), this);\n").arg(action.id).arg(action.title)
		if (action.checked) {
			doc.content += "action->setCheckable(true);\n"
			doc.content += "action->setChecked(true);\n"
		}
        if (action.toolTip !== "")
            doc.content += qsTr("action->setToolTip(tr(\"%1\"));\n").arg(action.toolTip)
        if (action.statusTip !== "")
            doc.content += qsTr("action->setStatusTip(tr(\"%1\"));\n").arg(action.statusTip)
	}
}
