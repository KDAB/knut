function space(depth) {
	var result = ""
    for (var i = 0; i < 2*depth; ++i)
		result += " "
	return result
}

function isEmptyMap( obj ) {
    for ( var name in obj ) {
        return false;
    }
    return true;
}

function createWidget(widget, depth, isLast) {
    var content = space(depth) + "{\n"
    content += space(depth) + qsTr("  \"className\" : %1,\n").arg(widget.className)
    content += space(depth) + qsTr("  \"id\"        : %1,\n").arg(widget.id)
    content += space(depth) + qsTr("  \"geometry\"  : %1,\n").arg(widget.geometry)

    if (!isEmptyMap(widget.properties)) {
        content += space(depth) + qsTr("  \"properties\": {\n")
        for (var key in widget.properties) {
            content += space(depth + 2) + qsTr("\"%1\": %2,\n").arg(key).arg(widget.properties[key])
        }
        content = content.slice(0, content.length - 2)
        content += "\n" + space(depth) + qsTr("  }\n")
    }

    if (widget.children.length !== 0) {
        content += space(depth) + qsTr("  \"children\"  : [\n")
        for (const childWidget of widget.children)
            content += createWidget(childWidget, depth+2)
        content = content.slice(0, content.length - 2)
        content += "\n" + space(depth) + qsTr("  ]\n")
	}
    content += space(depth) + ((depth === 0) ? "}\n" : "},\n")
    return content
}

function main() {
    for (const widget of widgets) {
        var doc = result.createDocument(widget.id)
        doc.content += createWidget(widget, 0)
    }
}
