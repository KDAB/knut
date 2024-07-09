// Example script with all the different widgets managed by ScriptDialog showing how to set/get data
/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: BSD-3-Clause

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

import Knut

ScriptDialog {
    id: root

    // Function called at startup, once the dialog is setup
    function init() {
        // Initializing widget data
        data.lineEdit = "Initialization text"
        // Define checkBox as unchecked and radioButton as checked
        data.checkBox = false
        data.radioButton = true
        data.spinBox = 1
        data.doubleSpinBox = 4.2
        // Defined the possible choices for the comboBoxfine
        data.comboBoxModel = ["1", "2", "3"]
        data.comboBox = "2"
    }

    function showData() {
        Message.log("OK button is clicked")
        Message.log("LineEdit data: " + data.lineEdit)
        Message.log("CheckBox data: " + data.checkBox)
        Message.log("RadioButton data: " + data.radioButton)
        Message.log("SpinBox data: " + data.spinBox)
        Message.log("DoubleSpinBox data: " + data.doubleSpinBox)
        Message.log("ComboBox data: " + data.comboBox)
    }

    // Function called when the user click on the OK button
    onAccepted: {
        // Logging the state of each widget when OK is clicked
        showData();
    }

    // Function called when the user click on the Cancel button
    onRejected: {
         // Logging when the Cancel button is clicked
        Message.log("Cancel button is clicked")
    }

    // Function called when a button is clicked
    onClicked:(name)=>{
        if (name == "pushButton"){
            Message.log("PushButton is clicked")
        }
        else if (name == "toolButton"){
            Message.log("ToolButton is clicked")
        }
    }
    // Function to automatically test the script, useful for automated testing
    // It runs the script without user interaction
    function test() {
        showData();
        close();
    }
}
