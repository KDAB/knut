# Script

Script object for writing non visual scripts. [More...](#detailed-description)

```qml
import Knut
```

## Detailed Description

The `Script` is the base class for all creatable items in QML. It is needed as a `QtObject`
can't have any children in QML. It can be used as the basis for non visual QML scripts:

```qml
import Knut

Script {
// ...
}
```
