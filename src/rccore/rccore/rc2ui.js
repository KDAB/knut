// Property handling
////////////////////
function writeFrame(frame, writer) {
    let frameConversion = {
        "ClientEdge": {
            "frameShape": "QFrame::Panel",
            "frameShadow": "QFrame::Sunken",
            "lineWidth": 2
        },
        "StaticEdge": {
            "frameShape": "QFrame::Panel",
            "frameShadow": "QFrame::Sunken"
        },
        "ModalFrame": {
            "frameShape": "QFrame::Panel",
            "frameShadow": "QFrame::Raised",
            "lineWidth": 2
        },
        "Border": {
            "frameShape": "QFrame::Box"
        },
        "Panel": {
            "frameShape": "QFrame::Panel"
        },
        "BlackFrame": {
            "frameShape": "QFrame::Box"
        },
        "Sunken": {
            "frameShape": "QFrame::Panel",
            "frameShadow": "QFrame::Sunken"
        },
    }
    if (frame in frameConversion) {
        for (const [name, value] of Object.entries(frameConversion[frame])) {
            writer.addProperty(name, value);
        }
    }
}

function writeProperty(name, value, writer) {
    // This is the place to customize properties
    // See how the frame is handled
    if (name === "frame") {
        writeFrame(value, writer);
    } else {
        writer.addProperty(name, value);
    }
}


// Widget hanlding
//////////////////
function convertClassName(className, writer) {
    // This is the place to customize classes
    // In case you are using a custom Qt widget, make sure to call
    // writer.addCustomWidget("Foo" , "QLabel", "<foo.h>", false)
    // - Foo is the class of the widget (ie what is returned by the method)
    // - QLabel is the base class
    // - <foo.h> is the header, use "foo.h" for a local header
    // - the last boolean is true if the widget is a container
    // For example:
    // if (className === "QLabel") {
    //    writer.addCustomWidget("MyLabel", "QLabel", "\"mylabel.h\"")
    //    return "MyLabel";
    //}

    return className;
}

function writeWidget(widget, writer) {
    writer.startWidget(convertClassName(widget.className, writer), widget);

    for (const [name, value] of Object.entries(widget.properties)) {
        writeProperty(name, value, writer);
    }

    for (let child of widget.children) {
        writeWidget(child, writer);
    }

    writer.endWidget();
}

function main(dialog, writer) {
    writeWidget(dialog, writer);
}
