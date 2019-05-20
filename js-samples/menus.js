function createMenu(currentMenu, parentMenu, doc) {
	for (var i = 0; i < currentMenu.children.length; ++i) {
		var childMenu = currentMenu.children[i]
		if (childMenu.isAction) {
			doc.content += qsTr("menu->addAction(m_actionManager->GetAction(%1));\n").arg(childMenu.id)
		} else if (childMenu.isSeparator) {
			result += "menu->addSeparator();\n"
		} else {
			doc.content += "{\n";
			doc.content += "parentMenu = menu;\n"
			doc.content += qsTr("menu = m_actionManager->CreateMenu(tr(\"%1\"), %2;\n\n").arg(childMenu.title).arg(parentMenu);
			doc.content += createMenu(childMenu, "parentMenu", doc);
			doc.content += "menu = parentMenu;\n"
			doc.content += "}\n";
		}
	}
	return result;
}

function main() {
    var doc = result.createDocument("CreateMenus")

	doc.content = "QMenu *menu = nullptr;\n"
	doc.content += "QMenu *parentMenu = nullptr;\n"
	doc.content += createMenu(menu, "menuBar()", doc)
}
