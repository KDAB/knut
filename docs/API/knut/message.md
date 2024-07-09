# Message

Singleton with methods to display different messages to the user. [More...](#detailed-description)

```qml
import Knut
```

## Methods

| | Name |
|-|-|
||**[debug](#debug)**(string text)|
||**[error](#error)**(string text)|
||**[log](#log)**(string text)|
||**[warning](#warning)**(string text)|

## Detailed Description

The `message` property in QML can be used to display different messages to the user, via logs.

```qml
Message.log("Hello World!")
```

## Method Documentation

#### <a name="debug"></a>**debug**(string text)

Shows the message `text` as a debug in the log.

#### <a name="error"></a>**error**(string text)

Shows the message `text` as an error in the log.

#### <a name="log"></a>**log**(string text)

Shows the message `text` as a log in the log.

#### <a name="warning"></a>**warning**(string text)

Shows the message `text` as a warning in the log.
