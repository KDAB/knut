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

    // 1) Create a Javascript generator to run the different steps
    // See https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Generator
    function *stepGeneratorFunc() {
        // 2) Defines the number of steps in the script
        // This can be done using the `stepCount` property of ScriptDialog
        // Note: if you don't set the step count, a moving scrollbar will be displayed
        setStepCount(5);

        // 3) Initialize the first step using `firstStep` and run the different commands for this step
        firstStep("Step 1...");
        Message.log("Step 1 is in progress, cooking something good...");
        Utils.sleep(1000);

        // 4) Use `yield` to start the following steps
        yield "Step 2...";
        Message.log("Step 2 is in progress, cooking something good...");
        Utils.sleep(1000);

        yield "Step 3...";
        Message.log("Step 3 is in progress, cooking something good...");
        Utils.sleep(1000);

        yield "Step 4...";
        Message.log("Step 4 is in progress, cooking something good...");
        Utils.sleep(1000);

        yield "Step 5...";
        Message.log("Step 5 is in progress, cooking something good...");
        Utils.sleep(1000);
    }

    // 5) On Ok, start the generator to show the different steps
    onAccepted: {
        runSteps(stepGeneratorFunc());
    }

    // This is to test the script automatically, to be used with `knut --test ex-gui-interactive.qml`
    // It will run the script automatically without interaction, even if it's the same method as `onAccepted`
    function test() {
        runSteps(stepGeneratorFunc());
        close();
    }
}
