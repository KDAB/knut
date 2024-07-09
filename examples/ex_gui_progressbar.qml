// Example of an interactive GUI script
/*
  This file is part of Knut.

  SPDX-FileCopyrightText: 2024 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: BSD-3-Clause

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

import Knut

ScriptDialog {
    id: root

    function showProgress() {
        // 1) Defines the number of steps in the script
        // This can be done using the `stepCount` property of ScriptDialog
        // Note: if you don't set the step count, a moving scrollbar will be displayed
        setStepCount(5);

        // 2) Initialize the first step using `firstStep` and run the different commands for this step
        firstStep("Step 1...");
        Message.log("Step 1 is in progress, cooking something good...");
        Utils.sleep(1000);

        // 3) Use `nextStep` for the following steps
        nextStep("Step 2...");
        Message.log("Step 2 is in progress, cooking something good...");
        Utils.sleep(1000);

        nextStep("Step 3...");
        Message.log("Step 3 is in progress, cooking something good...");
        Utils.sleep(1000);

        nextStep("Step 4...");
        Message.log("Step 4 is in progress, cooking something good...");
        Utils.sleep(1000);

        nextStep("Step 5...");
        Message.log("Step 5 is in progress, cooking something good...");
        Utils.sleep(1000);
    }

    // Function called when the user click on the OK button
    onAccepted: {
        showProgress();
    }

    // This is to test the script automatically, to be used with `knut --test ex-gui-interactive.qml`
    // It will run the script automatically without interaction
    function test() {
        showProgress();
        close();
    }
}
