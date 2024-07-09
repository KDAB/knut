import QtQuick
import Knut
import Knut.Test

/*!
 * \qmltype TestCase
 * \brief Provides a way to create unit tests as script.
 * \inqmlmodule Knut.Test
 * Run unit tests as a script, and returns the number of failed tests.
 */

QtObject {
    id: testCase
/*!
 * \qmlproperty string TestCase::name
 * This property defines the name of the unit test. **This is a mandatory property**.
 */
    property string name

    property bool hasError: false
    property string current
    property var model: ListModel {}
    property int failed: 0

/*!
 * \qmlmethod TestCase::compare(actual, expected, msg)
 * Compares `actual` vs `expected`, and display the `msg` if it's not the same.
 */
    function compare(actual, expected, msg) {
        var success = qtest_compareInternal(actual, expected)
        if (!success) {
            if (msg === undefined)
                msg = "Compared values are not the same"

            Message.error("FAIL!  : " + name + "::" + current + "() " + msg)
            Message.debug("   Actual  : " + actual)
            Message.debug("   Expected: " + expected)
            Message.debug("   " + TestUtil.callerFile() + "(" + TestUtil.callerLine() + ")")
            hasError = true
        }
    }

/*!
 * \qmlmethod TestCase::verify(value, msg)
 * Verifies that `value` is true, and display the `msg` if it's not.
 */
    function verify(value, msg) {
        if (!value) {
            if (msg === undefined)
                msg = "Verification failed"
            Message.error("FAIL!  : " + name + "::" + current + "() " + msg)
            Message.debug("   " + TestUtil.callerFile() + "(" + TestUtil.callerLine() + ")")
            hasError = true
        }
    }

    function run() {
        // Note: cannot run functions in TestCase elements
        // that lack a name.
        if (name.length === 0) {
            failed = -1
            return
        }

        Message.log("********* Start testing of " + name + " *********")
        var passed = 0
        var testList = []
        for (var prop in testCase) {
            if (prop.indexOf("test_") !== 0)
                continue
            current = prop
            hasError = false
            testCase[prop]()

            if (!hasError) {
                passed++
                Message.log("PASS   : " + name + "::" + current + "()")
            } else {
                failed++
            }
        }
        Message.log("Totals: " + passed + " passed, " + failed + " failed")
        Message.log("********* Finished testing of " + name + " *********")
        Qt.quit()
    }

    // Determine what is o.
    // Discussions and reference: http://philrathe.com/articles/equiv
    // Test suites: http://philrathe.com/tests/equiv
    // Author: Philippe Rathé <prathe@gmail.com>
    function qtest_typeof(o) {
        if (typeof o === "undefined") {
            return "undefined";

            // consider: typeof null === object
        } else if (o === null) {
            return "null";

        } else if (o.constructor === String) {
            return "string";

        } else if (o.constructor === Boolean) {
            return "boolean";

        } else if (o.constructor === Number) {

            if (isNaN(o)) {
                return "nan";
            } else {
                return "number";
            }
            // consider: typeof [] === object
        } else if (o instanceof Array) {
            return "array";

            // consider: typeof new Date() === object
        } else if (o instanceof Date) {
            return "date";

            // consider: /./ instanceof Object;
            //           /./ instanceof RegExp;
            //          typeof /./ === "function"; // => false in IE and Opera,
            //                                          true in FF and Safari
        } else if (o instanceof RegExp) {
            return "regexp";

        } else if (typeof o === "object") {
            if ("mapFromItem" in o && "mapToItem" in o) {
                return "declarativeitem";  // @todo improve detection of declarative items
            } else if ("x" in o && "y" in o && "z" in o) {
                return "vector3d"; // Qt 3D vector
            }
            return "object";
        } else if (o instanceof Function) {
            return "function";
        } else {
            return undefined;
        }
    }

    // Test for equality
    // Large parts contain sources from QUnit or http://philrathe.com
    // Discussions and reference: http://philrathe.com/articles/equiv
    // Test suites: http://philrathe.com/tests/equiv
    // Author: Philippe Rathé <prathe@gmail.com>
    function qtest_compareInternal(act, exp) {
        var success = false;
        if (act === exp) {
            success = true; // catch the most you can
        } else if (act === null || exp === null || typeof act === "undefined" || typeof exp === "undefined") {
            success = false; // don't lose time with error prone cases
        } else {
            var typeExp = qtest_typeof(exp), typeAct = qtest_typeof(act)
            if (typeExp !== typeAct) {
                // allow object vs string comparison (e.g. for colors)
                // else break on different types
                if ((typeExp === "string" && (typeAct === "object") || typeAct === "declarativeitem")
                        || ((typeExp === "object" || typeExp === "declarativeitem") && typeAct === "string")) {
                    success = (act === exp)
                }
            } else if (typeExp === "string" || typeExp === "boolean" ||
                       typeExp === "null" || typeExp === "undefined") {
                if (exp instanceof act.constructor || act instanceof exp.constructor) {
                    // to catch short annotation VS 'new' annotation of act declaration
                    // e.g. var i = 1;
                    //      var j = new Number(1);
                    success = (act === exp)
                } else {
                    success = (act === exp)
                }
            } else if (typeExp === "nan") {
                success = isNaN(act);
            } else if (typeExp === "number") {
                // Use act fuzzy compare if the two values are floats
                if (Math.abs(act - exp) <= 0.00001) {
                    success = true
                }
            } else if (typeExp === "array") {
                success = qtest_compareInternalArrays(act, exp)
            } else if (typeExp === "object") {
                success = qtest_compareInternalObjects(act, exp)
            } else if (typeExp === "declarativeitem") {
                success = qtest_compareInternalObjects(act, exp) // @todo improve comparison of declarative items
            } else if (typeExp === "vector3d") {
                success = (Math.abs(act.x - exp.x) <= 0.00001 &&
                           Math.abs(act.y - exp.y) <= 0.00001 &&
                           Math.abs(act.z - exp.z) <= 0.00001)
            } else if (typeExp === "date") {
                success = (act.valueOf() === exp.valueOf())
            } else if (typeExp === "regexp") {
                success = (act.source === exp.source && // the regex itself
                           act.global === exp.global && // and its modifiers (gmi) ...
                           act.ignoreCase === exp.ignoreCase &&
                           act.multiline === exp.multiline)
            }
        }
        return success
    }

    function qtest_compareInternalObjects(act, exp) {
        var i;
        var eq = true; // unless we can prove it
        var aProperties = [], bProperties = []; // collection of strings

        // comparing constructors is more strict than using instanceof
        if (act.constructor !== exp.constructor) {
            return false;
        }

        for (i in act) { // be strict: don't ensures hasOwnProperty and go deep
            aProperties.push(i); // collect act's properties
            if (!qtest_compareInternal(act[i], exp[i])) {
                eq = false;
                break;
            }
        }

        for (i in exp) {
            bProperties.push(i); // collect exp's properties
        }

        // Ensures identical properties name
        return eq && qtest_compareInternal(aProperties.sort(), bProperties.sort());

    }

    function qtest_compareInternalArrays(actual, expected) {
        if (actual.length !== expected.length) {
            return false
        }

        for (var i = 0, len = actual.length; i < len; i++) {
            if (!qtest_compareInternal(actual[i], expected[i])) {
                return false
            }
        }

        return true
    }
}
