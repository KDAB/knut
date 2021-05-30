# Script

Provides a script object for writing non visual scripts. [More...](#detailed-description)

```qml
import Script 1.0
```

## Detailed Description

The Script is the base class for all creatable items in QML. It is needed as a QtObject
can't have any children in QML. It can be used as the basis for non visual QML scripts:

import Script 1.0

Script {
// ...

