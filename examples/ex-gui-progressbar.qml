// Example of an interactive GUI script

import Script 1.0

ScriptDialog {
    id: root

    // Function called when the user click on the OK button
    onAccepted: {
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
}
