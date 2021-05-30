# Message

Provides methods to display different messages to the user. [More...](#detailed-description)

```qml
import Script 1.0
```

## Methods

- **[error](#error)**(string text)
- **[log](#log)**(string text)
- **[debug](#debug)**(string text)
- **[warning](#warning)**(string text)

## Detailed Description

The \c message property is used to display different messages to the user, via logs.

message.log("Hello World!") // In the Script Output panel

## Method Documentation

#### <a name="error"></a>**error**(string text)

Shows the message \a text as an error in the log.

#### <a name="log"></a>**log**(string text)

Shows the message \a text as a log in the log.

#### <a name="debug"></a>**debug**(string text)

Shows the message \a text as a debug in the log.

#### <a name="warning"></a>**warning**(string text)

Shows the message \a text as a warning in the log.
